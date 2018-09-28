/************************************************************************************************************
* Copyright (c) 2018, Dolby Laboratories Inc.
* All rights reserved.

* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:

* 1. Redistributions of source code must retain the above copyright notice, this list of conditions
*    and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
*    and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
*    promote products derived from this software without specific prior written permission.

* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************/

/****************************************************************************
;	File:	spectrum_NFFT.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fio.h"
#include "debug.h"
#include "parse_args.h"
#include "spect_NFFT.h"
#include "Utilities.h"
#include "version.h"
#include "wavelib.h"

fstruct fst;

extern void print_usage( void );

char *chart = "title, Averaged peak power spectrum,\nxlabel, Frequency (Hz),\nylabel, Amplitude (dBFS),\nxlog,\n"; /* store the name of the tool */

int main( int argc, char *argv[] )
{
    double vld_blk_sizes[11] =
    { 0.0, 512.0, 1024.0, 2048.0, 4096.0, 8192.0, 10240.0, 16384.0, 32000.0, 44100.0, 48000.0 }; /* window sizes */
    int i, isVldBlkSize, block_size, fft_output_size, data_sz = 0; /* variables to store the counter, valid block size, & fft output size */
    double *po = NULL;
    int ch;
    int startCh;
    int endCh;
	double freq;
	double difference;
	int freq_int;

    setlocale(LC_NUMERIC,"C");

    if (parse_args("spectrum_NFFT", argc, argv, &fst) != 0)
    {
        exit(1);
    }

    isVldBlkSize = 0;

    for (i = 0; i < 11; i++)
    {
      if (fst.nfft == vld_blk_sizes[i])
        isVldBlkSize = 1;
    }

    if (isVldBlkSize == 0)
    {
      error("Invalid FFT size, It must be either 512, 1024, 2048, 4096, 8192, 10240, 16384, 32000, 44100 or 48000\n");
      return -10;
    }

    if (fst.nfft == 0.0)
    {
      block_size = fst.fs;
    }
    else
    {
      block_size = (int) fst.nfft;
    }

    fft_output_size = (block_size / 2) + 1;
    if (fst.allChannels)
    {
        startCh = 0;
        endCh = fst.channels;
    }
    else
    {
        startCh = fst.channel;
        endCh = fst.channel + 1;
    }
    debugInit();
    fio_init(&fst, "spectrum_NFFT", "Frequency", "Amplitude", NULL);
    /*-- allocating memory for average power values --*/
    po = (double *) calloc(fft_output_size, sizeof(double));

    if (po == NULL)
    {
      error("Failed to allocate memory in spectrum_NFFT.\n");
      return -1;
    }
    /*-- returns the size of output array--*/
    if (fft_avg_NFFT(&fst, po, &data_sz) == 0)
      return (-1);

    /* for each channel calculate the values for the fft */
    for (ch = startCh; ch < endCh; ch++) /* for starting channel, till the given channel number */
    {
        check( fio_next_chunk(&fst, ch) );
        fio_resetpos(&fst);

        /* calculate the n point fft of the given signal for the given channel */

        fft_avg_NFFT(&fst, po, &data_sz);

        /*-- prints out to standard output or text file--*/


        for (i = 0; i < fft_output_size; i++)
        {
			freq = (fst.fs/(double)block_size) * i * 10.0;
			freq_int=(int)floor(freq);
			difference = freq - freq_int;
			if (difference == 0.5) /*to find the ambiguous number of rounding for the format %5.1f ,for example 281.25*/
			{
				/*these steps in order to realize the round to even is to make linux and windows have the same rounding behavior in printf format %5.1f*/
                freq = floor((fst.fs/(double)block_size) * i * 10.0);
			    if ((int)freq%2 == 0)
			    {
			        freq = freq/10.0;
			    }
			    else
			    {
			        freq = ceil((fst.fs/(double)block_size) * i * 10.0) / 10.0;
			    }
			}
			else /* which is not the ambiguous number for rounding */
			{
				freq = (fst.fs/(double)block_size) * i;
			}
			check( sdf_writer_add_data_double_double(fst.sdf_out,
                                                   (double) freq,
                                                   (double) po[i],
                                                   "%5.1f,\t%3.2f\n") );
        }
    }
    if (fst.navg == 0)
    {
      fprintf(stderr, "warning: Total number of input samples which were "
      "rejected due to end of file, for each channel: %d \n",data_sz );
    }

    fio_cleanup(&fst);
    debugCleanup();

    free (po);
    return 0;

}

void print_usage( void )
{
    fprintf(stderr, "Spectrum of NFFT Points tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr, "Usage: spectrum_NFFT [OPTION]... -i WAVFILE\n\n");
    fprintf(stderr, "-c <chan>,          selects channel in given Multi-channel file\n");
    fprintf(stderr, "                    0 = first channel, 1= Second channel and so on\n");
    fprintf(stderr, "-s,                 turn off stripping lead silence\n" );
	fprintf(stderr, "-thr_db <thr>,      select the power threshold for silence stripping in db\n");
	fprintf(stderr, "-thr_s <thr>,       select the sample threshold for silence stripping in bit\n");
    fprintf(stderr, "-t,                 selects text output mode\n");
    fprintf(stderr, "-to <name>,         create a text file\n");
    fprintf(stderr, "-powermin <lim>,    select dB level to which very low power values will be clipped\n");
    fprintf(stderr, "                    (default: lowest representable dB level by the given bitdepth)\n");
    fprintf(stderr, "-n <NFFT>,          selects the FFT size of spectrum\n");
    fprintf(stderr, "                    can be: 512, 1024, 2048, 4096, 8192\n");
    fprintf(stderr, "                    10240, 16384, 32000, 44100 or 48000\n");
    fprintf(stderr, "-z <NAVGs>,         selects the number of spectrum\n");
    fprintf(stderr, "                    averages to compute (if not provided\n");
    fprintf(stderr, "                    as many averages based on file length\n");
    fprintf(stderr, "                    will be computed\n");
    fprintf(stderr, "-window <Number>,   selects the window for the spectrogram\n");
    fprintf(stderr, "                    1: bartlet\n" );
    fprintf(stderr, "                    2: bartlet hann window\n" );
    fprintf(stderr, "                    3: blackmanharris window (This is the default Window.)\n" );
    fprintf(stderr, "                    4: rectangular window\n" );
    fprintf(stderr, "                    5: triangle window\n" );
    fprintf(stderr, "                    6: Hann window\n" );

}

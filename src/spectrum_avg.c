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
;	File:	spectrum_avg.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "fio.h"
#include "spect_avg.h" /*include the custom defined header for the tool*/
#include "debug.h"
#include "fft_avg.h"
#include "parse_args.h"
#include "version.h"
#include "Utilities.h"

extern void print_usage( void );

char *chart = "title, Spectrum Average,\nxlabel, Frequency (Hz),\nylabel, Amplitude (dBFS),\nxlog,\n";
fstruct fst; 

int main( int argc, char *argv[] )
{
    int res; /*variable to store temporary value while argument parsing*/
    int startCh, endCh; 
    int ch; /*intermediate channel number*/
    double *po;
    int n;
    int nc;
    int i;
    char *format = "%5.1lf,\t%3.2lf\n";

    setlocale(LC_NUMERIC,"C");

    res = parse_args("spectrum_avg", argc, argv, &fst);
    if (res)
        exit(1);

    n = (fst.fs / 2) + 1;
    po = (double *) calloc(n, sizeof(double));

    if (po == NULL)
    {
        error("%s: couldn't malloc, exiting\n", argv[0]);
        exit(-1);
    }

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

    debugInit(); /* initialize the debug function */
    fio_init(&fst, "spectrum_avg", "Frequency", "Amplitude", NULL); /* initialize the fio_init to do the file reading */

    for (ch = startCh; ch < endCh; ch++) /* loop over number of channels specified */
    {
        check( fio_next_chunk(&fst, ch) );

        fio_resetpos(&fst);

        nc = spectrum_averaging((&fst),po,n);

        /* loop to print out values */
        for (i = 0; i < nc; i++)
        {
            check( sdf_writer_add_data_float_float(fst.sdf_out, (float) i, (float) po[i], format) );
        }
    }

    fio_cleanup(&fst);
    debugCleanup();

    return (0);
} 

void print_usage( void )
{
    fprintf(stderr, "Spectrum Average Tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr, "Usage: spectrum_avg [OPTION]... -i WAVEFILE\n\n");
    fprintf(stderr, "-c <chan>,          selects channel in multi-channel file\n");
    fprintf(stderr, "                    0 = first channel\n");
    fprintf(stderr, "                    -c a for all channels\n");
    fprintf(stderr, "-s,                 turn off stripping lead silence\n" );
	fprintf(stderr, "-thr_db <thr>,      select the power threshold for silence stripping in db\n");
	fprintf(stderr, "-thr_s <thr>,       select the sample threshold for silence stripping in bit\n");
    fprintf(stderr, "-t,                 send text output to standard output (default)\n");
    fprintf(stderr, "-to <name>,         create a text file\n");
    fprintf(stderr, "-powermin <lim>,    select dB level to which very low power values will be clipped\n");
    fprintf(stderr, "                    (default: lowest representable dB level by the given bitdepth)\n");
}

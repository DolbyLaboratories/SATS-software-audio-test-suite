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
;	File:	spect_NFFT.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "fio.h"
#include "power.h"
#include "window.h"
#include "spect_NFFT.h"
#include "debug.h"

#include "SATS_fft.h"

//All buffers are aligned to 32 bytes in memory.
#define BUFFER_ALLIGN 32

#include <stdint.h>
#include <assert.h>
#define N_MEL_BANDS 40


static unsigned int *mel_break = 0;
static double  *mel_a = 0;

/*initialization function for mel scale transform*/
/*using the formula mel_break = a * (exp(b*(i+1))-1.0) to initialize the vector mel_break. the each three consecutive values in mel_break can be used to decide the triangle filter for mel filtering*/
static void
mel_init(unsigned int n_bins, unsigned int n_mel)
{
    const double warp = 3.0;    /* fiddle with this! */
    const double foo = n_bins/expm1(warp), bar = warp/n_mel;
    unsigned int i;

    if (0 == mel_break)
    {
        unsigned int prev_break = 0;
        mel_break = (unsigned int*)malloc(n_mel * sizeof *mel_break);
        mel_a = (double*)malloc(n_mel * sizeof *mel_a);
        for (i = 0; i < n_mel; i++)
        {
            mel_break[i] = (unsigned int)lrint(
                foo * expm1(bar * (i+1))
                );
            assert(0 < mel_break[i]         );
            assert(    mel_break[i] <= n_bins);
            assert(i == 0 || mel_break[i] > mel_break[i-1]); /* strictly monotonic */
            mel_a[i] = 1.0/(mel_break[i] - prev_break);
            prev_break = mel_break[i];
        }
    }
}

/*function for mel scale transform*/
static void
mel_spec(double  *dst, double *src)
{
    double LIMIT = -990.0;//-11.036224223;
    unsigned int i, j;
    double  e0 = 0, e1 = 0; /* e0 accumulates upward slope, e1 downward slope */
    double  a = mel_a[0], b0 = a, b1;  /* a = slope, b0 = upward gain, b1 = downward gain */
    double pre_sum = 0.0; /*sumation of downward slop coefficietns of previous window which is equal to the sumation of upward slop coefficients of current window*/
    double mel;
    /* first special because we only have upward slope and fixed anchor */
    for (j = 1; j < mel_break[0]; j++)  /* not 0 because coef of 0 is always 0 */
    {
        e0 = e0 + src[j] * b0;
        pre_sum = b0;
        b0 += a;
    }

    for (i = 1; i < N_MEL_BANDS; i++)
    {
        double normaliz = 0;
        double cur_sum = 0; /*sumation of downward slop coefficietns*/
        a = mel_a[i];
        b0 = a;
        b1 = 1.0 - a;
        j = mel_break[i - 1];
        e1 = e0 + src[j];
        e0 = 0;

        for (j = j + 1; j < mel_break[i]; j++)
        {
            double  e = src[j];
            e0 = e0 + e * b0;
            e1 = e1 + e * b1;
            cur_sum =cur_sum + b1;
            b0 += a;
            b1 -= a;
        }
        normaliz = 1.0 + pre_sum + cur_sum;
        pre_sum=cur_sum;
        mel = e1 / normaliz;
        //mel = e1;

        dst[i-1] = (10*log10(mel) > LIMIT) ? 10*log10(mel) : LIMIT;
    }
    dst[N_MEL_BANDS-1] = dst[i-1] = (10*log10(mel) > LIMIT) ? 10*log10(mel): LIMIT;    /* might as well keep the last upward slope values (?) */
}


static void
copy_doubles( double *pf, double *pt, int n )
{
    int i;

    for ( i = 0; i < n; i++ )
        *pt++ = *pf++;
}
/*fft_avg_NFFT function in order to do fft's with different block sizes and number of averages --*/
int fft_avg_NFFT( pfstruct pfs, double *po, int *data_sz )
{
	long block_size = 0;    // one second, 1 Hz resolution
	long block_count = 0;
	long total_block = 0;
	SATS_FFT_Complex *fft_out;
	SATS_FFT_HANDLE fft_handle;
	SATS_FFT_Status fft_status;
	int i;
	double mag = 0.0;
	double *pi, *pw, *pd, *pfft;
	double wf;
	double re, im;
	int fft_output_size = 0;
	double min_rms_db;
	long overlaping_samples = 0;
	long start_pos = 0;

	if (pfs->minPowerSet == 1)
	{
		min_rms_db = pfs->minPower;
	}
	else
	{
		// Computing minimum representable dB level for the bit depth of the signal
		min_rms_db = 0.0 - floor(20*log10(pow(2.0, pfs->bitspersamp)));
	}


	if ( pfs->nfft == 0.0 )
	{
		block_size = pfs->fs;
	}
	else
	{
		block_size = pfs->nfft;
	}

	if ( pfs->navg == 0.0)
	{
		total_block = LONG_MAX; /*Run till end of file ; No limit on number of averaging blocks */
	}
	else
	{
		total_block = pfs->navg;
	}



	fft_output_size = (block_size / 2) + 1;

	/*-- Storage buffer for input data (raw input samples) --*/
	pi = (double *)calloc( block_size , sizeof( double ) );
	if ( pi == NULL ) exit( -22 );

	/*-- Storage buffer for window coefficients --*/
	pw = (double *)calloc( block_size , sizeof( double ) );
	if ( pw == NULL ) exit( -23 );
	pfft = (double *)calloc(fft_output_size, sizeof( double ) );
	if ( pfft == NULL ) exit( -27 );
	/*-- Obtaining coefficients (pw now points to coefficients) --*/
	/* add a switch case for window type */
	switch (pfs->windowtype)
	{
	case 1:
		bartlett( pw, block_size );
		break;

	case 2:
		barthannwin( pw, block_size );
		break;

	case 3:
	default:
		blackmanharris( pw, block_size );
		break;

	case 4:
		rectwin( pw, block_size );
		break;

	case 5:
		triang( pw, block_size );
		break;

	case 6:
		hannwin( pw, block_size);
		break;
	}

	/*-- Computing window compensation --*/
	wf = compute_window_comp( pw, block_size );

	/*-- storage buffer for windowed samples --*/
	pd = (double *)SATS_FFT_malloc( block_size * sizeof( double ), BUFFER_ALLIGN );
	if ( pd == NULL ) exit( -24 );

	/*-- zeroing out windowed samples buffer --*/
	for (i=0;i<(int)block_size;i++) pd[i]=0.0;

	/*-- storage buffer for fft output data --*/
	fft_out = SATS_FFT_malloc ( sizeof ( SATS_FFT_Complex ) * fft_output_size, BUFFER_ALLIGN );
	if ( fft_out == NULL ) exit( -25 );

	/*-- zeroing out output data buffer --*/
	for (i=0;i<fft_output_size;i++)
	{
		SATS_FFT_REAL(fft_out[i])=0.0;
		SATS_FFT_IMAG(fft_out[i])=0.0;
	}

	if (strcmp(pfs->tool,"mel_scale") == 0)
	{
		for ( i = 0; i < N_MEL_BANDS ; i++ ) po[i] = 0.0;
	}
	else
	{
		for ( i = 0; i < fft_output_size ; i++ ) po[i] = 0.0;
	}

	/*--Function to Stripping silence in input data --*/
	/*checks if the value of noSilence in pfs = 0*/
	/*if it equals 0 then call the strip_lead_silence function */
	if ( pfs->noSilence==0 )
		strip_lead_silence( pfs );

	fft_status = SATS_FFT_Create(&fft_handle, block_size);

	/*-- this can be consoldated into two lines just need to get block_size worth of data --*/
	/* Reading an entire block of data by breaking it into 2 chunks*/
	fio_read( pfs, block_size / 2 );
	copy_doubles( pfs->data, pi, pfs->data_size );
	fio_read( pfs, block_size / 2 );
	copy_doubles( pfs->data, &pi[block_size/2], pfs->data_size );

	/*-- initializing block counter --*/
	block_count = 0;

	//while( pfs->data_size == block_size / 2 )
	while( (pfs->data_size == (block_size / 2) ) &&  ((long)block_count < total_block ) )
	{
		/*-- windowing input samples --*/
		window_array( pi, pw, pd, block_size );

		/*-- executing the fft on the windowed input samples --*/
		fft_status = SATS_FFT_ComputeForward( fft_handle, pd, fft_out);

		if (strcmp(pfs->tool,"mel_scale") == 0)
		{
			for ( i = 0; i < fft_output_size; i++ )
			{
				re = SATS_FFT_REAL(fft_out[i]);
				im = SATS_FFT_IMAG(fft_out[i]);
				mag = ( re * re ) + ( im * im );    // calculate power
				mag = mag * 2.0;                    // compensate for one sided fft
				mag = mag / ( wf );
				//if (mag > 1.0 )
				//pfft[i] = 1.0;
				//else if (mag < -1.0)
				//pfft[i] = -1.0;
				//else
				pfft[i] = mag ;
			}
			mel_init(fft_output_size, N_MEL_BANDS);
			mel_spec(po, pfft);
		}
		else
		{
			for ( i = 0; i < fft_output_size; i++ )
			{
				re = SATS_FFT_REAL(fft_out[i]);
				im = SATS_FFT_IMAG(fft_out[i]);
				mag = ( re * re ) + ( im * im );    // calculate power
				mag = mag * 2.0;                    // compensate for one sided fft
				mag = mag / ( pfs->fs * wf * block_size );    //    window compensation
				po[i] = po[i] + mag;
			}
		}

		block_count++;
		/*Decide the next position to begin file reading only to be done for spectrogram tool.*/
		if ((strcmp(pfs->tool, "spectrogram") == 0)||( strcmp(pfs->tool, "mel_scale") == 0) )
		{
			overlaping_samples = (long) (pfs->nfft - pfs->stride);
			start_pos = pfs->position - overlaping_samples;
			fio_resetpos(pfs);
			fio_setpos(pfs, start_pos);
		}

		if (strcmp(pfs->tool,"spectrum_NFFT") == 0)
		{
			fio_read( pfs, block_size / 2 );
			/*-- checking if we ran out of input sampls --*/
			if ( pfs->data_size != block_size / 2 )
				break;


			/*-- copying old input data back into input buffer in order to OVERLAP input data --*/
			copy_doubles( &pi[block_size/2], pi, block_size/2 );

			/*-- Get new data from wav file to put at tail end of input buffer --*/
			copy_doubles( pfs->data, &pi[block_size/2], block_size/2 );
		}

	}//for the while loop

	if ((strcmp(pfs->tool,"spectrum_NFFT") == 0)||(strcmp(pfs->tool,"spectrogram") == 0))
	{
		/*-- averaging results over each run and convert to dB --*/
		for ( i = 0; i < fft_output_size; i++ )
		{
			// compensate for window and rms -> peak
			if ( i == 0 || i == fft_output_size - 1 )
				po[i] = ( 10.0 * log10( po[i] / (double) (block_count) ) ) + 3.02;
			else
				po[i] = ( 10.0 * log10( po[i] / (double) (block_count) ) ) + 3.010299957 + 3.02;
			if (po[i] < min_rms_db)
			{
				po[i] = min_rms_db;
			}
		}
	}

	fft_status = SATS_FFT_Destroy(&fft_handle);
	SATS_FFT_free ( fft_out );
	SATS_FFT_free( pd );

	free( pi );
	free( pw );

	/*-- Making sure that the input data size is not smaller than the fft block size --*/
	if ( block_count == 0 )
	{
		error("Error: File too small to perform FFT because input samples less than fft block size\n");
		return( -1 );
	}
	*data_sz = pfs->data_size;
	//returning the number of samples which were not considered due to end of file

	/* Debug printing */
#if 0
	for (i = 0; i < (int)((block_size / 2) + 1); i++ )
	{
		debug("%5.1f,\t%3.2f\n" , (((double)pfs->fs)/(double)block_size)*i , po[i] );
	}
#endif
	fft_status = fft_status ;
	return( fft_output_size);
}

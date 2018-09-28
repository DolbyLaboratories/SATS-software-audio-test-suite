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
;	File:	settling.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include "settling.h"
#include "debug.h"
#include "fio.h"
#include "thd_freq.h"
#include "Utilities.h"

#include "SATS_fft.h" 

/* Note that the +1's below derives from the original MATLAB -> C conversion.
 * TODO: Determine whether the +1 is ever used, or just defensive.
 */
#define NUMBLOCKS_MAX           ( 20 )
#define FS_MAX                  ( 48000 )
#define FFT_SIZE_MAX            ( 8192 )            /* FFT_SIZE_MAX = 2^(round(log2(FS_MAX) - 3)) */
#define BLK_SIZE_MAX            ( 8192 + 1 )        /* BLK_SIZE_MAX = 2^(round(log2(FS_MAX) - 3)) + 1 */
#define NEW_BLK_SIZE_MAX        ( 2048 + 1 )        /* NEW_BLK_SIZE_MAX = BLK_SIZE_MAX / 4 + 1 */


static void settling_debug( char *fmt, ... );

static void settling_debug( char *fmt, ... )
{
	fmt = fmt;
#ifdef DEBUG
#ifdef CHECK_SETTLING_DEBUG
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "        CHECK_SETTLING: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
#endif
#endif
    return;
}

/**
 * @brief   Perform amplitude and/or frequency settling
 *
 * @details Frequency / amplitude settling is selected via the input enum analysis_type.
 *          An amplitude "gradient" mode can also be enabled via the analysis_type enum.
 *
 *          The output settle_point is the number of samples (+1) required for settling.
 *          Settle_point is set to zero when settling does not occur, or when an error
 *          occurs.
 *
 *          Input can be supplied in two ways: via an fstruct or supplied raw.  The input
 *          selection is indicated via the input_type enum.  If input is provided via an
 *          fstruct (pfs), settling is performed from the current data_position.  The
 *          data_position is advanced to the settle_point before exiting.  If input is
 *          provided raw, a pointer, length and sampling rate are all expected to be provided.
 *
 * @param   analysis_type   IN: enum to enable frequency and/or amplitude setting
 * @param   input_type      IN: indicates if input is provided raw or via file struct
 * @param   pfs             IN: pointer to file structure (ignored if input_type != FSTRUCT)
 * @param   praw_data       IN: pointer to raw input samples of length raw_data_length (ignored if input_type != RAW)
 * @param   raw_data_length IN: length of raw input samples (ignored if input_type != RAW)
 * @param   raw_data_fs     IN: sampling rate (in Hz) raw input samples (ignored if input_type != RAW)
 * @param   c_freq          IN: center frequency in Hz
 * @param   thres_db        IN: settling threshold in dB
 * @param   alpha           IN: alpha factor (time constant) for leaky integrator
 * @param   numblocks       IN: number of blocks to settle over (max 20)
 * @param   limit           IN: maximum length to search over (in samples)
 * @param   settle_point    OUT: number of samples required for settling + 1
 *
 * @return  Returns 0 if settling is performed.  -1 otherwise.
 */
int check_settling( check_settling_analysis_type analysis_type,
                    check_settling_input_type input_type,
                    pfstruct pfs,
                    double *praw_data,
                    unsigned long long raw_data_length,
                    unsigned int raw_data_fs,
                    double c_freq,
                    double thres_db,
                    double alpha,
                    int numblocks,
                    int limit,
                    int *settle_point)
{
    int i = 0;

    int datai = 0;                  /* index into input array data */
    int total_blkcount = 0;         /* initial total block count */
    double curr_freq = 0;           /* current frequency */
    unsigned long fs = 0;           /* sampling rate */
    unsigned long startpoint = 0;   /* initial starting point in samples to start testing for settling */
    int blklimit = 0;               /* block limit */

    /* variables for FFT */
    double fftsize;                 /* fft size */
    int blksize;                    /* block size */
    int stepsize;                   /* Step Size */
    SATS_FFT_Complex blk_out[FFT_SIZE_MAX];     /* Complex output datatype from the FFT */
    double blk[BLK_SIZE_MAX];                /* fft input Array */
    double new_blk[NEW_BLK_SIZE_MAX];        /* fft input Array */
    //SATS_FFT_HANDLE fft_handle;      /* FFT handle for configuring the FFT. */
    SATS_FFT_Status fft_status = 0;        /* Status variable for all FFT function returns. */

    double re = 0.0, img = 0.0;     /* real and imaginary values */
    double new_fft_value = 0.0;

    /* flags and variable for frequency settling testing */
    int freq_settling_on;           /* flag for testing freq settling */
    int freq_settled;               /* flag to test if frequency has settled */
    int freq_sett_limit = 0;        /* */
    int freq_sett_count = 0;        /* count for how many times it encounter similar frequency values */
    int freq_settle_blk = 0;        /* frequency settled block */

    /* flags and variable for amp settling testing */
    double pwr_levels[NUMBLOCKS_MAX];   /* this is array of power calculation for amp settling testing */
    double last_output = 0;         /* last output */
    int amp_settling_on;            /* flag for testing amp settling */
    int amp_settled;                /* flag to test if amplitude has settled */
    int use_gradient = 0;           /* flag to use gradient method */
    int amp_settle_blk = 0;         /* amplitude settled block */

    /* Miscellaneous Variables */
    int while_count;                /* Counter for the main while loop */
    int settle_blk = 0;
    double new_freq = 0.0;
    double freq_error = 0.0;
    double pwr_mean_value = 0.0;
    double pwr_level = 0.0;
    double max_fft_value = 0.0;
    int index = 0;
    double std_pwr;
    double grade;
    double start_idx, stop_idx;     /* Indicies used for debug printing */

    settling_debug("START----\n");

    /* Input argument checking */
    assert(numblocks <= NUMBLOCKS_MAX);
    if (input_type == FSTRUCT)
    {
        assert(pfs != NULL);
        assert(pfs->fs <= FS_MAX);
    }
    else
    {
        assert(raw_data_fs <= FS_MAX);
    }

    /* Set sampling rate and clamp limits */
    if (input_type == FSTRUCT)
    {
        fs = pfs->fs;
        startpoint = pfs->data_position;
        limit = imin(limit, (pfs->data_size - pfs->data_position));
    }
    else
    {
        fs = raw_data_fs;
        limit = (int) imin64(limit, raw_data_length);
    }

    /* checks what type of settling we are doing */
    if ((analysis_type == FREQ) || (analysis_type == FREQ_AMP_NO_GRADIENT) || (analysis_type == FREQ_AMP_GRADIENT))
    {
        settling_debug("Performing frequency settling\n");

        /* Setting flags and values for frequency settling testing (frequency settling testing = ON) */
        freq_settling_on = 1;

        /* starts with whatever is passed in */
        curr_freq = c_freq;
        freq_settled = 0;

        /* This minus one is here because the freq check is a > test */
        freq_sett_limit = numblocks - 1;

        /* first do freq settling */
        freq_sett_count = 0;

        /* Setting FFT variables */
        fftsize = pow(2, (round(log(fs) / log(2)) - 3));
        blksize = (int) fftsize;
        stepsize = (int) (blksize * .25);
    }
    else
    {
        /* Not doing frequency settling so make it look like freq settling happened immediately */
        freq_settling_on = 0;
        freq_settled = 1;
        freq_settle_blk = 0;

        /* if only doing amplitude settling we can use a much smaller block size. */
        /* This provides much faster settling use nearest power of two to 31.25ms */
        fftsize = pow(2, (round(log(fs) / log(2)) - 5));
        blksize = (int) fftsize;
        stepsize = (int) (blksize * .25);
    }

    settling_debug("Block and step sizes: %d %d\n", blksize, stepsize);

    /* Checking if there is enough data to detect settling */
    if (limit < blksize)
    {
        *settle_point = 0;
        settling_debug("Not enough data to detect settling.\n");
        settling_debug("FAIL.\n\n");
        return(-1);
    }

    /* Setting the blklimit for the while loop */
    blklimit = (int) floor((limit - blksize) / stepsize) + 1;

    /* Setting Amplitude Settling variables and array */
    if ((analysis_type == AMP_NO_GRADIENT) || (analysis_type == AMP_GRADIENT) || (analysis_type == FREQ_AMP_NO_GRADIENT) || (analysis_type == FREQ_AMP_GRADIENT))
    {
        if ((analysis_type == AMP_NO_GRADIENT) || (analysis_type == FREQ_AMP_NO_GRADIENT))
        {
            settling_debug("Performing amplitude settling without gradient\n");
        }
        else
        {
            settling_debug("Performing amplitude settling with gradient\n");
        }

        /* Setting variables and flags for amplitude setting (Amplitude setting testing = ON) */
        for (i = 0; i < numblocks; i++)
        {
            pwr_levels[i] = 999;
        }

        last_output = 999;
        amp_settling_on = 1;
        amp_settled = 0;

        // Checking weather gradient method will be used
        if ((analysis_type == AMP_GRADIENT) || (analysis_type == FREQ_AMP_GRADIENT))
        {
            use_gradient = 1;
        }
        else
        {
            use_gradient = 0;
        }
    }
    else
    {
        /* not doing frequency  settling so make it look like amp settling happened immediately */
        amp_settling_on = 0;
        amp_settled = 1;
        amp_settle_blk = 0;
    }

    /* Using sliding window approach  to get analysis window big enough to get frequecy resolution. */
    /* The sliding window means we don't run out of data before getting a few results. */
    /* First read in enough data to do first fft with new data */

    /* Fill in Array with data that is going to be processed, also updating audio_data_position, audio_data_chunk_position */
    if (input_type == FSTRUCT)
    {
        /* Read data into array blk */
        memcpy(blk, &pfs->data[pfs->data_position], (blksize - stepsize) * sizeof(double));
        pfs->data_position += (blksize - stepsize);
    }
    else
    {
        /* Transferring data from data */
        memcpy(blk, &praw_data[datai], (blksize - stepsize) * sizeof(double));
        datai += (blksize - stepsize);
    }

    /* Variable for counting number of iteration before AMP and FREQ Settling are successful */
    while_count = 0;

    /* This while is used to walk through the audio data and search for frequency and amplitude settlement blocks */
    /* Exit conditions are when frequency AND amp have settled */
    while ((total_blkcount < blklimit) && ((freq_settled == 0) || (amp_settled == 0)))
    {
        while_count++;

        /* Note that we don't bother amplitude scaling here because we are only concerned with frequency */
        if (input_type == FSTRUCT)
        {
            /* Getting new set of audio data of size=stepsize in samples, located at new_blk */
            memcpy(new_blk, &pfs->data[pfs->data_position], stepsize * sizeof(double));
            pfs->data_position += stepsize;

            /* Print analysis window for debug */
            start_idx = (double) (pfs->data_position - blksize);
            stop_idx = (double) (pfs->data_position);
            settling_debug("Analyzing from %5.2fs [%.0f]  to  %5.2fs [%.0f]\n",
                            start_idx / ((double) pfs->fs), start_idx,
                            stop_idx / ((double) pfs->fs), stop_idx);
        }
        else
        {
            memcpy(new_blk, &praw_data[datai], stepsize * sizeof(double));
            datai += stepsize;

            /* Print analysis window for debug */
            start_idx = (double) (datai - blksize);
            stop_idx = (double) (datai);
            settling_debug("Analyzing from %5.2fs [%.0f]  to  %5.2fs [%.0f]\n",
                            start_idx / ((double) raw_data_fs), start_idx,
                            stop_idx / ((double) raw_data_fs), stop_idx);
        }

        /* join new data with old data to make sliding window */
        memcpy(&blk[blksize - stepsize], new_blk, stepsize * sizeof(double));

        /* Incrementing the number of blocks that analysis has been done */
        total_blkcount++;

        /* Checking for Frequency Settling */
        /* freq_settled variable will be set to 1 when frequency has settled */
        if (freq_settling_on == 1)
        {

            /* Pad with zeros to fftsize */
            for (i = 0; i < (fftsize - blksize); i++)
            {
                blk[blksize + i] = 0.0;
            }

            /* Executing forward FFT for audio data in blk array */
            fft_status = SATS_FFT_ComputeForward2(blk, blk_out, (int)fftsize);

            max_fft_value = 0.0;
            index = 0;

            /* Looking for the max absolute value and index */
            for (i = 0; i < (fftsize / 2); i++)
            {
                /* Coping all the real values to fft_data array */
                re = SATS_FFT_REAL(blk_out[i]);
                img = SATS_FFT_IMAG(blk_out[i]);

                new_fft_value = sqrt(((re * re) + (img * img)));

                /* Checking for Max value and corresponding index */
                if ((double) max_fft_value < (double) new_fft_value)
                {
                    max_fft_value = (double) new_fft_value;
                    index = i;
                }
            }

            /* Calculate frequency as integer , allow frequency error of one bin in case we are straddling a bin */
            new_freq = round((index * fs) / fftsize);
            freq_error = fabs(curr_freq - new_freq);

            settling_debug("New Freq: %i\t Current Freq: %i\t Freq Error: %i\n", (int) new_freq, (int) curr_freq, (int) freq_error);

            /* Checking if new_frequency found is valid using the difference between the last frequency value and current value */
            if ((freq_error <= ceil(fs / fftsize)) && (((int) new_freq >= 14) && ((int) new_freq <= 20100)))
            {
                freq_sett_count++;
            }
            else
            {
                settling_debug("Frequency settle counter reset\n");

                if ((int) c_freq == 0)
                {
                    /* if real frequency was passed stick with it otherwise go with a new one */
                    curr_freq = new_freq;
                }

                /* if found new frequency then reset amplitude filter to speed up convergence */
                freq_sett_count = 1;
                freq_settled = 0;
            }

            settling_debug("Freq_Settle Count:       %i\n", freq_sett_count);

            if (freq_sett_count > freq_sett_limit)
            {
                freq_settled = 1;
                freq_settle_blk = total_blkcount - freq_sett_count;

                settling_debug("Frequency settled at blk %d\n", freq_settle_blk);
            }

        } /* ends frequency settling testing */

        /* Amplitude Setting Testing */
        if (amp_settling_on == 1)
        {

            /* calculate rms power */
            pwr_mean_value = pwr_mean(blk, blksize);

            if (pwr_mean_value == 0)
            {
                pwr_level = -999;
            }
            else
            {
                pwr_level = 10 * log10f((float)pwr_mean_value);
            }

            /* if power has changed so dramatically that settling is impossible then reset filter */
            if (fabs(pwr_level - last_output) > (thres_db * numblocks))
            {
                last_output = pwr_level;
            }
            else
            {
                /* In Matlab code pwr_levels(2) but must take into account index starts at zero for C */
                last_output = pwr_levels[1];
            }

            /* use moving average of power levels */
            pwr_levels[0] = ((alpha * pwr_level) + ((1 - alpha) * (last_output)));

            std_pwr = std(pwr_levels, numblocks);
            grade = -1 * gradient_mean(pwr_levels, numblocks) * numblocks;

#if 0
            for (i = 0; i < numblocks; i++)
            {
                settling_debug("pwr_levels[ %i ]: %.4f\n", i, pwr_levels[i]);
            }
            settling_debug("pwr_mean_value: %.4f\n", pwr_mean_value);
            settling_debug("pwr_level:      %.4f\n", pwr_level);
#endif

            if (use_gradient)
            {
                settling_debug("Grade:          %.4f\n", grade);
                settling_debug("Grade thres:    %.4f\n", -1 * thres_db);
            }
            else
            {
                settling_debug("std dev power:  %.4f\n", std_pwr);
                settling_debug("std dev thres:  %.4f\n", thres_db);
            }

            /* select either std devaition based settling or gradient based */
            if (((use_gradient == 0) && (std_pwr < thres_db)) || ((use_gradient == 1) && (grade > (-1 * thres_db))))
            {
                amp_settled = 1;
                amp_settle_blk = total_blkcount - numblocks;

                settling_debug("Amplitude settled at blk %d\n", amp_settle_blk);
            }
            else
            {
                amp_settled = 0;
            }

            memmove(&pwr_levels[1], pwr_levels, ((numblocks - 1) * sizeof(double)));

        } /* end of amplitude settling */

        /* stripping of oldest data ready for next time */
        memmove(blk, &blk[stepsize], (blksize - stepsize) * sizeof(double));

        settling_debug("Loop counter:            %d\n", while_count);
        settling_debug("Settling flags:          %d %d\n\n", freq_settled, amp_settled);

    } /* end of while loop */

    /* if we reach the end of the data before settling then return failed indicator */
    if (total_blkcount == blklimit)
    {
        *settle_point = 0;
        settling_debug("Search loop expired.\n");
        settling_debug("FAIL.\n\n");
        return(-1);
    }

    /* Find the settle point */
    settle_blk = imax(freq_settle_blk, amp_settle_blk);
    *settle_point = settle_blk * (int) stepsize;

    /* compensate for the first frame where we needed to fill the window */
    if (*settle_point > 0)
    {
        *settle_point = *settle_point + blksize - stepsize;
    }

    settling_debug("freq_settle_blk : %i\n", (int) freq_settle_blk);
    settling_debug("amp_settle_blk  : %i\n", (int) amp_settle_blk);
    settling_debug("settle_blk      : %i\n", (int) settle_blk);
    settling_debug("settle_point    : %i\n", (int) *settle_point + 1);

    if (input_type == FSTRUCT)
    {

        if (pfs->data_position < pfs->data_size)
        {
            pfs->data_position = startpoint + *settle_point;
        }
        else
        {
            settling_debug("Settled but insufficient space in file struct.\n");
            settling_debug("FAIL.\n\n");
            *settle_point = 0;
            return(-1);
        }
    }

    *settle_point = *settle_point + 1;

    settling_debug("SUCCESS.\n\n");
	fft_status = fft_status;
    return(0);
}

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
;	File:	fchange.c
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
#include "fchange.h"
#include "settling.h"
#include "debug.h"
#include "fio.h"
#include "thd_freq.h"
#include "Utilities.h"

#include "SATS_fft.h"


/* FFT size */
#define FFTSIZE     ( 65536 )

static void fchange_debug( char *fmt, ... );

static void fchange_debug( char *fmt, ... )
{
#ifdef DEBUG
#ifdef NEXT_F_CHANGE_DEBUG
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "    NEXT_F_CHANGE: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
#endif
#endif
    (void)fmt;
    return;
}

/**
 * @brief   Find the next frequency change
 *
 * @details Takes a normal file structure as an input parameter.  Finds absolute positions for
 *          the dwell end, start of next dwell and settle point.  See diagram below.  The
 *          frequency of the dwell (in Hz) is also returned.
 *
 *          File position is restored.
 *
 *          If dwell end == settle_point then nothing found so move to settle next time.
 *
 * @verbatim
 * |                                                     |
 * |  noise   <----   tone                  --------->   | <--- new tone
 *  --~~~~~~~~-----------------------------------------------------------
 *  ^         ^                                      ^      ^
 *  origin    settle                         dwell end      dwell start
 * @endverbatim
 *
 * @param   pfs             IN: Pointer to file structure
 * @param   settle_point    OUT: Pointer to calculated settle point
 * @param   dwell_end       OUT: Pointer to calculated dwell end
 * @param   dwell_start     OUT: Pointer to calculated dwell end
 * @param   ref_freq        OUT: Pointer to calculated frequency in Hz
 *
 * @return  Returns 0 if a new frequency is found.  -1 otherwise.
 */
int find_next_fchange( pfstruct pfs, int *settle_point, double *dwell_end, double *dwell_start, double *ref_freq )
{
    int i = 0;

    double c_freq = 0;                  /* center frequency */
    double thres_db = 0.1;              /* threshold in dB */
    double alpha = 0.1;                 /* alpha for leaky integrator */
    int numblocks = 10;                 /* initial number of blocks */
    unsigned long fs = 0;               /* sampling frequency */
    int sliding_window_step = 0;        /* sliding window step, in samples */

    /* Define pulse shape based on analysis window size
     * use triangular windows designed to reject pulses 2Hz away
     */
    double pulse[] = { -0.5, 0.5, 1.0, 0.5, -0.5 };
    double pulse_off = 2.0;             /* Pulse offset */
    int length_of_pulse = 5;

    long init_analysis = 0;             /* initial analysis time, This is the same length of time for settling */
    double ref_val = -1.0;              /* set reference value to bad value for initialization on the first block */
    int thres = 0;                      /* threshold in time (0.05 sec) */
    int found_new_tone_count = 0;       /* found new tone count */
    double new_tone_ratio = 0.0;        /* new tone ratio */

    double fft_data_in[FFTSIZE];        /* buffer for FFT data */
    SATS_FFT_Complex fft_data_out[FFTSIZE];/* Complex output datatype from FFT. */
    SATS_FFT_Status fft_status;         /* Status variable for all FFT function returns. */
    int number_fft_samples;             /* number of samples supplied to the FFT */

    double re = 0.0, im = 0.0;          /* real and imaginary numbers */
    double mag = 0.0;                   /* magnitude */
    int ref_ind = 0;                    /* reference index */
    int found_new_tone = 0;             /* flag to indicate if a new tone has been found */
    int sliding_window_length = 16384;  /* length of the sliding window */
    double val = 0.0;
    int ind = 0;
    int found_freq_count;
    double amp, freq;                   /* amplitude and frequency */
    double new_val, pulse_start, pulse_end;
    double fftdata_start, fftdata_end;
    int diff_index_fftdata, diff_index_pulse;
    long original_data_position;
    long old_pos;
    double start_idx, stop_idx;         /* indices used for debug printing */

    /* Input argument checking */
    assert(pfs != NULL);
    assert(settle_point != NULL);
    assert(dwell_start != NULL);
    assert(dwell_end != NULL);
    assert(ref_freq != NULL);

    /* Configure parameters based on input arguments */
    fs = pfs->fs;
    init_analysis = (unsigned long) round(( (double) (fs / 3) ));
    thres = fs / 20;
    original_data_position = pfs->data_position;

    fchange_debug("START----\n");

    /* Checking for settling in frequency and amplitude.  Will update *settle_point pointer. */
    if (check_settling( FREQ_AMP_NO_GRADIENT, FSTRUCT, pfs, NULL, 0, 0, c_freq, thres_db, alpha, numblocks, INT_MAX, settle_point ) != 0)
    {
        fchange_debug("Check settling failed.\n");
    }

    fchange_debug("Initial settling point: %5.3f   [%ld]\n", ((double) *settle_point)/((double) pfs->fs) , *settle_point);

    /* check if no settling point was found */
    if (*settle_point == 0)
    {
        *dwell_end = 0.0;
        *dwell_start = 0.0;
        *ref_freq = 0.0;
        fchange_debug("No Settling point was found\n");
        return(-1);
    }

    found_new_tone_count = 0;
    new_tone_ratio = 0.0;

    *settle_point += original_data_position;
    pfs->data_position = *settle_point;

    /* Zero the input and output buffers.  Output buffer is complex - hence factor of 2. */
    for (i = 0; i < FFTSIZE; i++)
    {
        fft_data_in[i] = 0.0;
        SATS_FFT_REAL(fft_data_out[i]) = 0.0;
        SATS_FFT_IMAG(fft_data_out[i]) = 0.0;
    }

    /* Fill the into FFT input array from the settling point */
    //printf("%d\n", pfs->data_position);
    if ((pfs->data_size - pfs->data_position) > init_analysis)
    {
        memcpy(fft_data_in, &pfs->data[pfs->data_position], init_analysis * sizeof(fft_data_in[0]));
        memset(&fft_data_in[init_analysis], 0, (FFTSIZE - init_analysis) * sizeof(fft_data_in[0]));
        pfs->data_position += init_analysis;

        /* Log the analysis window boundaries */
        start_idx = (double) (pfs->data_position - init_analysis);
        stop_idx = (double) (pfs->data_position);
        fchange_debug("Analyzing from %5.2fs [%.0f]  to  %5.2fs [%.0f]\n",
                    start_idx / ((double) pfs->fs), start_idx,
                    stop_idx / ((double) pfs->fs), stop_idx);
    }
    else
    {
        *dwell_end = 0.0;
        *dwell_start = 0.0;
        *ref_freq = 0.0;
        fchange_debug("Insufficient data when filling FFT input array\n");
        fchange_debug("FAIL.\n\n");
        return(-1);
    }

    /* Forward FFT for audio data in blk array */
    fft_status = SATS_FFT_ComputeForward2(fft_data_in, fft_data_out, FFTSIZE);

    /* Resetting the index and initial max value */
    val = 0.0;
    ind = 0;

    /* Find the maximum magnitude, and associated index */
    for (i = 0; i < FFTSIZE / 2; i++)
    {
        re = SATS_FFT_REAL(fft_data_out[i]);
        im = SATS_FFT_IMAG(fft_data_out[i]);
        mag = (re * re) + (im * im);

        if ((double) val < (double) fabs(mag))
        {
            val = (double) fabs(mag);
            ind = i;
        }
    }

    /* Calculating the corresponding frequency value from sample index */
    *ref_freq = round((double) (ind * fs) / FFTSIZE);

    fchange_debug("New reference frequency: %dHz\n", (int) *ref_freq);

    /* Quit if we have a reference frequency that is clearly bogus  */
    if (*ref_freq > 21000 || *ref_freq < 15)
    {
        *dwell_end = 0;
        *dwell_start = 0;
        *ref_freq = 0;
        fchange_debug("Bogus reference frequency: %dHz\n", (int) *ref_freq);
        fchange_debug("FAIL.\n\n");
        return(-1);
    }

    /* Set initially to 1 sec.  This will find boundary quickly even if dwell is long */
    sliding_window_step = fs;
    pfs->data_position = *settle_point;

    found_freq_count = 0;

    /* Fill the into FFT input array from the settling point */
    if ((pfs->data_size - pfs->data_position) > sliding_window_length)
    {
        number_fft_samples = sliding_window_length;
    }
    else
    {
        number_fft_samples = pfs->data_size - pfs->data_position;
    }

    /* Copy into fft_data_in and zero pad to the end */
    memcpy(fft_data_in, &pfs->data[pfs->data_position], number_fft_samples * sizeof(fft_data_in[0]));
    memset(&fft_data_in[number_fft_samples], 0, (FFTSIZE - number_fft_samples) * sizeof(fft_data_in[0]));
    pfs->data_position += number_fft_samples;

    /* Log the analysis window boundaries */
    start_idx = (double) (pfs->data_position - number_fft_samples);
    stop_idx = (double) (pfs->data_position);
    fchange_debug("Analyzing from %5.2fs [%.0f]  to  %5.2fs [%.0f]\n",
                start_idx / ((double) pfs->fs), start_idx,
                stop_idx / ((double) pfs->fs), stop_idx);


    /* Main search loop */
    while (number_fft_samples == sliding_window_length)
    {
        fft_status = SATS_FFT_ComputeForward2(fft_data_in, fft_data_out, FFTSIZE);

        /* Search for the max absolute value and index */
        new_val = 0.0;
        ind = 0;
        for (i = 0; i < (FFTSIZE / 2); i++)
        {
            /* Calculating power of the new data for normalization */
            re = SATS_FFT_REAL(fft_data_out[i]);
            im = SATS_FFT_IMAG(fft_data_out[i]);
            mag = ((re * re) + (im * im)) * 2;

            if (new_val < fabs(mag))
            {
                new_val = fabs(mag);
                ind = i;
            }
        }

        /* Debug Mode prints out Magnitude value and corresponding index */
        fchange_debug("Max power: %.1f\t at time: %.2fs [%ld]\n", new_val, ((double) ind) / ((double) pfs->fs), ind);

        /* Clamp pulse start and end values in case ind is bogus.  +1 for the conversion from MATLAB to C indexing. */
        pulse_start = max_d(1, ((ind + 1) - pulse_off));
        pulse_end = min_d(((ind + 1) + pulse_off), (FFTSIZE / 2));

        /* +1 to match the Matlab array offset */
        fftdata_start = (int) ((1 + pulse_start - ((ind + 1) - pulse_off)));
        fftdata_end = (int) (length_of_pulse + pulse_end - ((ind + 1) + pulse_off));

        diff_index_fftdata = (int) pulse_end - (int) pulse_start;
        diff_index_pulse = (int) fftdata_end - (int) fftdata_start;

        /* Find the power sum, new_val */
        if (diff_index_fftdata == diff_index_pulse)
        {
            double mult = 0.0;
            new_val = 0.0;
            for (i = 0; i <= diff_index_fftdata; i++)
            {
                re = SATS_FFT_REAL(fft_data_out[(int) (i + pulse_start) - 1]);
                im = SATS_FFT_IMAG(fft_data_out[(int) (i + pulse_start) - 1]);
                amp = sqrt((re * re) + (im * im));
                mult = amp * (double) pulse[i];
                new_val = new_val + mult;
            }
        }

        /* converts the sample index into frequency value */
        freq = round((double) (ind * fs) / ((double) FFTSIZE));

        /* prints out the new frequency value */
        fchange_debug("Detected frequency : %.0fHz\n", freq);

        if (ref_val == -1)
        {
            ref_ind = ind;
        }

        /* clamp pulse start and end values in case ind is bogus */
        /* the plus one is there in order to match the Matlab version */
        pulse_start = max_d(1, ((ref_ind + 1) - pulse_off));
        pulse_end = min_d(((ref_ind + 1) + pulse_off), (FFTSIZE / 2));

        /* minus one to match the Matlab array offset */
        fftdata_start = (int) ((1 + pulse_start - ((ref_ind + 1) - pulse_off)));
        fftdata_end = (int) (length_of_pulse + pulse_end - ((ref_ind + 1) + pulse_off));

        diff_index_fftdata = (int) pulse_end - (int) pulse_start;
        diff_index_pulse = (int) fftdata_end - (int) fftdata_start;

        /* Getting the power sum, val  */
        if (diff_index_fftdata == diff_index_pulse)
        {
            double mult = 0.0;
            amp = 0.0;

            val = 0.0;

            for (i = 0; i <= diff_index_fftdata; i++)
            {
                re = SATS_FFT_REAL(fft_data_out[(int) (i + pulse_start) - 1]);
                im = SATS_FFT_IMAG(fft_data_out[(int) (i + pulse_start) - 1]);
                amp = sqrt((re * re) + (im * im));
                mult = amp * (double) pulse[i];
                val = val + mult;
            }
        }

        if (ref_val == -1)
        {
            ref_val = val;
        }

        /* this checks to see if new tone is found unlike the loss of the */
        /* current tone we don't know the position of the change because */
        /* the new tone could be much bigger or smaller */
        if ((freq < (*ref_freq * 0.9)) || (freq > (*ref_freq * 1.1)))
        {
            found_new_tone = 1;
            fchange_debug("Found new tone\n");
        }
        else
        {
            found_new_tone = 0;
        }
        /* Incrementing tone count and finding new tone ratio (tone ratio is used for calculation new dwell start) */
        if ((found_new_tone == 1) && (new_val > ref_val))
        {
            found_new_tone_count++;

            /* calculate how much bigger new tone is then current */
            new_tone_ratio = max_d(new_tone_ratio, (new_val / ref_val));
        }

        /* Threshold is -6dB down  calculate next step in samples  */
        if ((val < (ref_val / 2)) || (found_new_tone == 1))
        {
            /* Slide the window backward */
            sliding_window_step = (int) (-1.0 * fabs(round(sliding_window_step / 2)));
        }
        else
        {
            /* Slide the window forward */
            if (sliding_window_step != (int) fs)
            {
                sliding_window_step = (int) fabs(round(sliding_window_step / 2));
            }

            found_freq_count++;
        }

        /* Update the window */
        old_pos = pfs->data_position - sliding_window_length;
        pfs->data_position += sliding_window_step - sliding_window_length;

        /* Handle the case where the updated window runs past the end of the buffer */
        if ((pfs->data_size - pfs->data_position) < sliding_window_length)
        {
            /* Move one block back from end of file */
            pfs->data_position = pfs->data_size - sliding_window_length;

            /* Set window step to match */
            sliding_window_step = pfs->data_position - old_pos;
        }

        fchange_debug("Sliding window step: %d\n", sliding_window_step);

        /* if correction has dropped by half then found new freq */
        if (abs(sliding_window_step) < thres)
        {

            /* if we haven't found frequency change we are probably in noise skip over settling and try again */
            if (found_freq_count == 0)
            {
                /* if settling was OK but we couldn't find freq then the settling figure was bad. */
                /* In this case bail, this is possible as settling is much better at detecting low level stuff than this algorithm */
                if (*settle_point == 0)
                {
                    /* We scanned one second, so try again one second on */
                    *dwell_end = (pfs->data_size < (original_data_position + pfs->fs)) ? pfs->data_size : (original_data_position + pfs->fs);
                    *settle_point = (int) *dwell_end;
                    *dwell_start = *dwell_end;
                }
                else
                {
                    /* otherwise start from the settle point */
                    *dwell_start = *settle_point + original_data_position;

                    /* This indicates its an empty block and to move to settle point */
                    *dwell_end = *settle_point;
                }

                fchange_debug("In noise.  Frequency not found.\n");
                fchange_debug("FAIL.\n\n");
                return(-1);
            }

            /* new dwell start is at end of the block. The current position is the start of the new block so the length of the block must be added as an uncertainty */
            /* The accuracy threshold is also added */
            *dwell_start = pfs->data_position + (2 * thres) + round(sliding_window_length / 2);
            *dwell_end = pfs->data_position + round(sliding_window_length / 2) - (2 * thres);

            /* if we are hard up against a tone then can move forward half an analysis block */
            if (found_new_tone_count > 0)
            {
                /* convert new tone ratio to fraction */
                new_tone_ratio = max_d(0, (1 - (1 / new_tone_ratio)));
                *dwell_start = *dwell_start + round((sliding_window_length / 2) * new_tone_ratio);
            }

            *dwell_end = max_d(*settle_point, *dwell_end);
            *dwell_start = max_d(*dwell_end, *dwell_start);

            /* if somehow out start position got too low then bail this shouldn't be possible */
            if (*dwell_start <= *settle_point)
            {
                *ref_freq = 0;
                *dwell_end = *settle_point;
                *dwell_start = pfs->data_size;
            }

            /* Reset data_position */
            pfs->data_position = original_data_position;

            fchange_debug("Frequency found.\n");
            fchange_debug("Frequency   : %dHz\n", (int) *ref_freq);
            fchange_debug("Settle point: %5.2fs [%ld]\n", ((double) *settle_point) / ((double) pfs->fs), *settle_point);
            fchange_debug("Dwell start : %5.2fs [%ld]\n", *dwell_start / ((double) pfs->fs), (int) *dwell_start);
            fchange_debug("Dwell end   : %5.2fs [%ld]\n", *dwell_end / ((double) pfs->fs), (int) *dwell_end);
            fchange_debug("SUCCESS.\n\n");
            return (0);
        }

        /* Fill the into FFT input array from the settling point */
        if ((pfs->data_size - pfs->data_position) > sliding_window_length)
        {
            number_fft_samples = sliding_window_length;
        }
        else
        {
            number_fft_samples = pfs->data_size - pfs->data_position;
        }

        /* Copy into fft_data_in and zero pad to the end */
        debug("0x%x 0x%x %d %d\n", fft_data_in, &pfs->data[pfs->data_position], pfs->data_position, number_fft_samples);
        memcpy(fft_data_in, &pfs->data[pfs->data_position], number_fft_samples * sizeof(double));
        memset(&fft_data_in[number_fft_samples], 0, (FFTSIZE - number_fft_samples) * sizeof(double));
        pfs->data_position += number_fft_samples;

        /* Log the analysis window boundaries */
        start_idx = (double) (pfs->data_position - number_fft_samples);
        stop_idx = (double) (pfs->data_position);
        fchange_debug("Analyzing from %5.2fs [%.0f]  to  %5.2fs [%.0f]\n",
                    start_idx / ((double) pfs->fs), start_idx,
                    stop_idx / ((double) pfs->fs), stop_idx);

    } /* End of While loop */

    /* Frequency not found */

    *ref_freq = 0;
    *dwell_end = *settle_point;
    *dwell_start = (double) pfs->data_size;
    fchange_debug("Frequency not found.  Search terminated.\n");

    /* Reset data_position */
    pfs->data_position = original_data_position;

    fchange_debug("FAIL.\n\n");

    fft_status = fft_status;

    return(-1);
}

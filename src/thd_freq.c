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
;	File:	thd_freq.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include "fio.h"
#include "thd_freq.h"
#include "fchange.h"
#include "debug.h"
#include "power.h"
#include "sos_filter.h"
#include "dr_filters.h"
#include "debug.h"
#include "Utilities.h"
#include "settling.h"

#define MIN_REAL    -HUGE_VAL
#define MAX_REAL	+HUGE_VAL
#define PI			3.141592653589793

static void thdfreq_debug( char *fmt, ... );
static void thdfilt_debug( char *fmt, ... );

static void thdfreq_debug( char *fmt, ... )
{
	fmt = fmt;
#ifdef DEBUG
#ifdef THD_FREQ_DEBUG
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "THD_FREQ: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
#endif
#endif
    return;
}

static void thdfilt_debug( char *fmt, ... )
{
	fmt = fmt;
#ifdef DEBUG
#ifdef THD_FILT_DEBUG
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "THD_FILT: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
#endif
#endif
    return;
}

/**
 * @brief   Entry point for the THD Vs Freq, and Freq resp tools
 *
 * @details These two tools share a common processing engine, hence the common entry point.
 *          This function is responsible for finding (via find_next_fchange()) and sanity
 *          checking frequency dwells.  This function will also call the core measurement
 *          functions, thd_filt() and freq_resp(), on the appropriate chunks of audio.
 *
 *          Input is passed via a file structure.  This function will detect/skip silence
 *          at the beginning of the test signal.
 *
 *          The processing function (THD Vs freq or freq resp) is determined by a mode enum
 *          that's passed in.
 *
 *          Results are returned via arrays.  Pointers to the results and "bad dwells"
 *          arrays are passed in.
 *
 * @param   pfs                 IN: Pointer to file structure
 * @param   mode                IN: Selects either THD Vs Freq, or Freq Resp tool
 * @param   points              OUT: Pointer to results array
 * @param   nrows               OUT: Pointer to length of results array
 * @param   bad_dwells          OUT: Pointer to "bad dwells" results array
 * @param   bad_dwells_nrows    OUT: Pointer to length of "bad dwells" results array
 *
 * @return  Returns 0 if successful.  -1 otherwise.
 */
int thd_freq( pfstruct pfs, thd_freq_mode mode, double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, double bad_dwells[DWELLS_MAX], int *bad_dwells_nrows )
{
    int settle_point = 0;               /* Initial value for settling point */
    double block_end = 0.0;             /* Dwell end */
    double new_block_start = 0.0;       /* Dwell start */
    double c_freq = 0.0;                /* Reference frequency */
    double reading_db = 0.0;            /* Reading in dB */
    double pred_freq = 0.0;             /* Predicted frequency */
    int data_len = 0;                   /* Length of audio chunk being processed */
    int success = 0;                    /* Flag for find a center frequency in dwell */
    double lower_limit = 0.0;           /* Threshold Lower Limit for difference between predicted and actual center frequency */
    double upper_limit = 0.0;           /* Threshold Upper Limit for difference between predicted and actual center frequency */
    double min_size = 0.0;
    double min_rms_db = 0.0;

    thdfreq_debug("START----\n");

    /* Initalizing filter coefficients */
    init_dr_filters();

    /* Stripping lead silence of input .wav file.  strip_lead_silence() will only
     * leave pfs->position set to the index following the end of the silence.
     */
    thdfreq_debug("Stripping lead silence\n");
    strip_lead_silence(pfs);

    /* Checking if reached the end of the file */
    if (fio_eof(pfs))
    {
        thdfreq_debug("Reached end of file after removing silence.  Silent file?\n");
        return(-1);
    }

    if (pfs->minPowerSet == 1)
    {
      min_rms_db = pfs->minPower;
    }
    else
    {
     /*Computing minimum representable dB level for the bit depth of the signal*/
      min_rms_db = 0.0 - floor(20*log10(pow(2, pfs->bitspersamp)));
    }
    /* fio_read() copies audio data from the file into memory pointed to by
     * pfs->data.  Audio is copied from pfs->position (i.e. following silence)
     * through to the end of the file.  From here on, pfs->data_position is used
     * to index into the "current position" of the audio.  pfs->position is not
     * used from here on.
     */
    fio_read(pfs, pfs->size - pfs->position);

    /* Set the minimum size (half a second) */
    min_size = pfs->fs * .5;

    /* Finding all the dwells by searching for change in frequencies */
    find_next_fchange(pfs, &settle_point, &block_end, &new_block_start, &c_freq);

    while (c_freq > 0)
    {
        /* After two valid points are gathered then can start using existing points for predicting the next center frequency */

        if (*nrows > 2)
        {
            /* Piecewise Hermite Interpolation function that will predict the next fundamental frequency */
            pred_freq = pchip_interp(points, *nrows, *nrows);

            /* Setting the lower and upper limits so they can be used to gauge the validity of the next center frequency */
            if (pred_freq > points[*nrows][0])
            {
                lower_limit = max_d(pred_freq * 0.9, points[*nrows - 1][0]);
                upper_limit = min_d(pred_freq * 1.1, 20000);
            }
            else
            {
                lower_limit = max_d(pred_freq * 0.9, 20);
                upper_limit = min_d(pred_freq * 1.1, points[*nrows - 1][0]);
            }
            thdfreq_debug("Next predicted frequency:\t %.0fHz \t (%.0f - %.0fHz)\n", pred_freq, lower_limit, upper_limit);

            /* Actual Center Frequency is not within limits */
            if ((c_freq < lower_limit) || (c_freq > upper_limit))
            {
                settle_point = (int) block_end;
                thdfreq_debug("Rejected Frequency %.0fHz\n", c_freq);
            }
        }

        thdfreq_debug("...Settling point:      %.2fs   [%d]\n", ((double) settle_point)/((double) pfs->fs) , settle_point);
        thdfreq_debug("...End of block:        %.2fs   [%d]\n", ((double) block_end)/((double) pfs->fs) , (int) block_end);
        thdfreq_debug("...Start of next block: %.2fs   [%d]\n", ((double) new_block_start)/((double) pfs->fs) , (int) new_block_start);
        thdfreq_debug("...Next frequency:      %.0fHz\n", c_freq);

        /* Checks to see if there is next dwell has enough data if not then reset file position */
        if (( block_end - ( (int) settle_point )) < ( (int) min_size ))
        {
            thdfreq_debug("Out of data!!!\n");
        }
        else
        {
            /* Determine the length of the audio chunk to process */
            data_len = ((int) block_end) - settle_point;

            /* Checks weather if we are doing thd_vs_freq or freq_resp test */
            if (mode == THD_VS_FREQ)
            {
                success = thd_filt(&pfs->data[settle_point], data_len, c_freq, pfs->fs, &reading_db);
                thdfreq_debug("thd_filt() returned %d\n\n", success);
            }
            else
            {
                success = freq_resp(&pfs->data[settle_point], data_len, &reading_db);
                thdfreq_debug("freq_resp() returned %d\n\n", success);
            }

            /* Found a valid center frequency and now is adding to points array else adds result to invalid array (bad_dwells) */
            if (success == 0)
            {
                if (reading_db < min_rms_db)
                 {
                	reading_db = min_rms_db;
                 }
                add_points(points, nrows, c_freq, reading_db);
            }
            else
            {
                add_bad_dwells(bad_dwells, bad_dwells_nrows, c_freq);
            }

        }

        /* Advance data_position for the next block */
        pfs->data_position = (int) new_block_start;

        /* Finds next frequency change */
        find_next_fchange(pfs, &settle_point, &block_end, &new_block_start, &c_freq);

    } /* End of while loop */

    /* prints out all elements in point array ( these elements are the content of a .res file ) */
    write_to_file(points, nrows, pfs);

    /* Free the dynamic range filters */
    free_dr_filters();

    thdfreq_debug("END----\n");
    return(0);
}

/**
 * @brief   Calculate the "frequency response" on a segment of audio
 *
 * @details This function calculates the RMS power in dB across a chunk of audio.  When
 *          called consecutively on different frequency dwells, this function effectively
 *          performs a power spectrum calculation.
 *
 *          Input is passed via a pointer and a length value.  The output is returned
 *          via a pointer.
 *
 * @param   data                IN: Pointer to audio samples
 * @param   len_data            IN: Length of audio data
 * @param   reading_db          OUT: Pointer to measured power in dB

 * @return  Returns 0 if successful.  -1 otherwise.
 */
int freq_resp( double *data, int len_data, double *reading_db )
{
    double rms = 0.0;
    double rms_db = 0.0;

    rms = sqrt(pwr_mean(data, (int) len_data));

    /* Convert to dB, sanity checking first */
    if (rms == 0)
    {
        rms_db = -999.0;
    }
    else
    {
        rms_db =  20 * log10f((float)rms);
    }
    /* find peak by adding 3dB */
    *reading_db = rms_db + 3.010299957;

    return(0);
}

/**
 * @brief   Calculate the THD on a segment of audio
 *
 * @details This function uses a notch filter to calculate the THD (in dB) across the defined
 *          audio segment.  When called consecutively on different frequency dwells,
 *          this function effectively performs a THD Vs frequency calculation.
 *
 *          Input is passed via a pointer and a length value.  The output is returned
 *          via a pointer.
 *
 * @param   data                IN: Pointer to audio samples
 * @param   len_data            IN: Length of audio data
 * @param   c_freq              IN: Center frequency of tone during segment (notched out)
 * @param   fs                  IN: Sampling rate in Hz
 * @param   reading_db          OUT: Pointer to measured power in dB

 * @return  Returns 0 if successful.  -1 otherwise.
 */
int thd_filt( double *data, int len_data, double c_freq, double fs, double *reading_db )
{
    double *y = NULL;
    double *res_filt = NULL;
    double bw = c_freq * .00002;
    double thres_db = 1.0;
    double alpha = 0.25;
    int numblocks = 20;
    int filt_settle_point;
    int new_len_data;
    double total_sum;
    int i;
    double rms;

    /* Debug printing */
    thdfilt_debug("Entering thd_filt()\n");
#if 0
    thdfilt_debug("-----------BEFORE NOTCH FILTER----------------\n");
    for (j = 0; j < 5; j++)
    {
        thdfilt_debug("data[%i]: %.16f\n", j, data[j]);
    }
    thdfilt_debug("...\n");
    for (j = len_data - 5; j < len_data; j++)
    {
        thdfilt_debug("data[%i]: %.16f\n", j, data[j]);
    }
#endif

    bw = min_d(0.125, bw);

    /* Allocate temporary buffers */
    if ((y = (double *) calloc((len_data), sizeof(double))) == NULL)
    {
        error("malloc failed\n");
        return(-1);
    }

    if ((res_filt = (double *) calloc((len_data), sizeof(double))) == NULL)
    {
        free(y);
        error("malloc failed\n");
        return(-1);
    }

    reset_sos_filter(&on_fly_notch);

    notch2ndOrder((c_freq / (fs / 2)), bw);

#if 0
    thdfilt_debug("on_fly_notch_b[0][0] :  %.16f\n", (double) (on_fly_notch.b[0][0]));
    thdfilt_debug("on_fly_notch_b[0][1] :  %.16f\n", (double) (on_fly_notch.b[0][1]));
    thdfilt_debug("on_fly_notch_b[0][2] :  %.16f\n", (double) (on_fly_notch.b[0][2]));
    thdfilt_debug("on_fly_notch_a[0][0] :  %.16f\n", (double) (on_fly_notch.a[0][0]));
    thdfilt_debug("on_fly_notch_a[0][1] :  %.16f\n", (double) (on_fly_notch.a[0][1]));
    thdfilt_debug("on_fly_notch_a[0][2] :  %.16f\n", (double) (on_fly_notch.a[0][2]));
#endif

    sos_filter_array(data, &on_fly_notch, y, len_data);

#if 0
    thdfilt_debug("-----------AFTER NOTCH FILTER----------------\n");
    for (j = 0; j < 5; j++)
    {
        thdfilt_debug("y[ %i ]: %.12f\n", j, y[j]);
    }
    thdfilt_debug("-----------AFTER NOTCH FILTER----------------\n");
#endif

    if (fs > 42000)
    {

        if ((int) fs == 44100)
        {
            reset_sos_filter(&dr_lp_44100);
            sos_filter_array(y, &dr_lp_44100, res_filt, len_data);
        }
        else
        {
            reset_sos_filter(&dr_lp_48000);
            sos_filter_array(y, &dr_lp_48000, res_filt, len_data);
        }
    }
    else
    {
        memcpy(res_filt, y, len_data * sizeof(double));
    }

#if 0
    thdfilt_debug("----------------------RES FILTER ---------------------------------\n");
    for (j = 0; j < 5; j++)
    {
        thdfilt_debug("RES_FILTER[ %i ]: %.12f\n", j, res_filt[j]);
    }
    thdfilt_debug("...\n");
    for (j = len_data - 6; j < len_data; j++)
    {

        thdfilt_debug("RES_FILTER[ %i ]: %.12f\n", j, res_filt[j]);
    }
#endif

    if (check_settling(AMP_GRADIENT, RAW, NULL, res_filt, len_data, (int) fs, c_freq, thres_db, alpha, numblocks, len_data, &filt_settle_point ) != 0)
    {
        free(y);
        free(res_filt);
        debug("check_settling failed\n");
        return(-1);
    }

    new_len_data = len_data - filt_settle_point + 1;

    if (filt_settle_point == 0)
    {
        *reading_db = -999;
        debug("check_settling returned a settling_point of 0\n");
        free(y);
        free(res_filt);
        return(-1);
    }

    total_sum = 0.0;
    for (i = filt_settle_point - 1; i < len_data; i++)
    {
        total_sum += res_filt[i] * res_filt[i];
    }

    rms = sqrt((total_sum) / (double) new_len_data);

    if (rms == 0)
    {
        *reading_db = -999;
    }
    else
    {
        *reading_db = (20 * log10f((float)rms) + 3.01);
    }

    free(res_filt);
    free(y);

    return(0);
}

double pwr_mean( double *blk, int blk_size )
{
    double mean = 0.0;
    double re = 0.0;
    double SQ_value = 0.0;
    double mean_result = 0.0;

    int i;

    for (i = 0; i < blk_size; i++)
    {
        /* Getting real value */
        re = blk[i];

        /* Squaring each element */
        SQ_value = pow(re, 2);

        /* Adding all squared values */
        mean = mean + SQ_value;
    }

    mean_result = mean / (blk_size);

    return (mean_result);
}

/* Standard deviation*/
double std( double *pwr_levels, int pwr_levels_size )
{
    double total_sum = 0;
    double diff = 0;
    double SQ = 0;
    double std_value = 0;

    int i;

    for (i = 0; i < pwr_levels_size; i++)
    {
        total_sum = total_sum + pwr_levels[i];
    }

    total_sum = total_sum / pwr_levels_size;

    for (i = 0; i < pwr_levels_size; i++)
    {
        diff = pwr_levels[i] - total_sum;
        SQ = SQ + pow(diff, 2);
    }

    std_value = SQ / (pwr_levels_size - 1);

    std_value = pow(std_value, .5);

    return (std_value);

}

double gradient_mean( double *pwr_levels, int pwr_levels_size )
{

    double mean_dx = 0.0;
    double dx = 0.0;
    int i = 0;

    dx = pwr_levels[i + 1] - pwr_levels[i];
    mean_dx = mean_dx + dx;

    /* Skipps first element */
    for (i = 1; i < (pwr_levels_size - 1); i++)
    {
        dx = ((pwr_levels[i] - (pwr_levels[i - 1])) + (pwr_levels[i + 1] - (pwr_levels[i]))) * .50;
        mean_dx = mean_dx + dx;
    }

    /* takes care of the last element */
    i = pwr_levels_size - 1;
    dx = pwr_levels[i] - pwr_levels[i - 1];
    mean_dx = mean_dx + dx;

    /* Taking the average */
    mean_dx = mean_dx / (pwr_levels_size);

    return (mean_dx);
}

void write_to_file( double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, pfstruct pfs )
{

    char *format = "%0.0f,\t%0.2f\n";
    int i = 0;

    assert(*nrows < DWELLS_MAX);

    for (i = 0; i < *nrows; i++)
    {
        check( sdf_writer_add_data_float_float(pfs->sdf_out, (float) points[i][0], (float) points[i][1], format) );
    }

}

int add_points( double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, double c_freq, double reading_db )
{
    points[*nrows][0] = c_freq;
    points[*nrows][1] = reading_db;
    (*nrows)++;
    assert(*nrows < DWELLS_MAX);
    return (0);
}

int add_bad_dwells( double bad_dwells[DWELLS_MAX], int *bad_dwells_nrows, double c_freq )
{

    bad_dwells[(int) *bad_dwells_nrows] = c_freq;
    (*bad_dwells_nrows)++;
    assert(*bad_dwells_nrows < DWELLS_MAX);
    return (0);
}

int notch2ndOrder( double Wo, double BW )
{
    double Gb = 0.707945784;
    double beta, gain;
    BW = BW * PI;
    Wo = Wo * PI;

    beta = (sqrt(1 - pow(Gb, 2)) / Gb) * tan((BW / 2));
    gain = 1 / (1 + beta);

    write_coef(&on_fly_notch, 1, (-2 * gain * cos(Wo)), ((2 * gain) - 1), gain, (-2 * gain * cos(Wo)), gain);

    return (0);
}

double pchip_interp( double y[DWELLS_MAX][THDFREQ_POINTS], int length_y, int u )
{

    int i;
    int length_x = length_y;
    int dnm1 = 0;

    int *x = NULL;
    double *h = NULL;
    double *del = NULL;
    double *slopes = NULL;
    double *dzzdx = NULL;
    double *dzdxdx = NULL;
    double *reshape_1 = NULL;
    double *reshape_2 = NULL;
    double **coefs = NULL;
    int *sort = NULL;

    double hs = 0.0;
    int k = 0;
    double w1 = 0.0;
    double w2 = 0.0;
    double dmax = 0.0;
    double dmin = 0.0;
    int numel_x = 0;
    int n;
    int dim = 0;

    int dlk = 0;
    int l = 0;
    int dl = 0;
    const double eps = 0.00000000000002220446049250313;

    //int ixexist = 0;
    int index = 0;
    int xs = 0;
    //int d = 0;
    //int dd = 0;
    double v = 0;
    int length_del;

    /* Increment to maintain compatibility with MATLAB */
    length_x++;
    length_y++;
    u++;

    if ((x = (int *) calloc((length_x), sizeof(int))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < length_x - 1; i++)
    {
        x[i] = i + 1;
    }

    n = length_x - 1;

    if ((h = (double *) calloc((length_x - 1), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < length_x - 2; i++)
    {
        h[i] = (double) (x[i + 1] - x[i]);
    }

    if ((del = (double *) calloc((length_y), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    length_del = length_y;

    for (i = 0; i < length_y - 2; i++)
    {
        del[i] = (double) (y[i + 1][0] - y[i][0]);
    }

    if ((slopes = (double *) calloc((length_y), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < length_del - 1; i++)
    {
        if (sign(del[i]) * sign(del[i + 1]) > 0)
        {

            k = i;
            hs = h[k] + h[k + 1];
            w1 = (h[k] + hs) / (3 * hs);
            w2 = (hs + h[k + 1]) / (3 * hs);
            dmax = max_d(fabs(del[k]), fabs(del[k + 1]));
            dmin = min_d(fabs(del[k]), fabs(del[k + 1]));
            slopes[k + 1] = dmin / ((w1 * (del[k] / dmax)) + (w2 * (del[k + 1] / dmax)));
        }
    }

    slopes[0] = ((2 * h[0] + h[1]) * del[0] - (h[0] * del[1])) / (h[0] + h[1]);

    if (sign(slopes[0]) != sign(del[0]))
    {
        slopes[0] = 0;
    }
    else if ((sign(del[0]) != sign(del[1])) && (fabs(slopes[0]) > fabs(3 * del[0])))
    {
        slopes[0] = 3 * del[0];
    }

    slopes[n - 1] = (((2 * h[n - 2]) + h[n - 3]) * del[n - 2] - (h[n - 2] * del[n - 3])) / (h[n - 2] + h[n - 3]);

    if (sign(slopes[n - 1]) != sign(del[n - 2]))
    {
        slopes[n - 1] = 0;
    }
    else if ((sign(del[n - 2]) != sign(del[n - 3])) && (fabs(slopes[n - 1]) > fabs(3 * del[n - 2])))
    {
        slopes[n - 1] = 3 * del[n - 2];
    }

    numel_x = length_x - 1;

    if ((dzzdx = (double *) calloc((numel_x - 1), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < numel_x - 1; i++)
    {
        dzzdx[i] = (del[i] - slopes[i]) / h[i];
    }

    if ((dzdxdx = (double *) calloc((numel_x - 1), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < numel_x - 1; i++)
    {
        dzdxdx[i] = (slopes[i + 1] - del[i]) / h[i];
    }

    dnm1 = numel_x - 1;

    if ((reshape_1 = (double *) calloc((numel_x - 1), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < numel_x - 1; i++)
    {
        reshape_1[i] = (dzdxdx[i] - dzzdx[i]) / h[i];
    }

    if ((reshape_2 = (double *) calloc((numel_x - 1), sizeof(double))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < numel_x - 1; i++)
    {
        reshape_2[i] = (2 * dzzdx[i] - dzdxdx[i]);
    }

    dim = 1;

    dlk = dnm1 * 4;

    l = length_x - 2;

    dl = dim * l;

    k = (int) ((dlk / dl) + (100 * eps));

    if ((coefs = (double **) calloc(dl, sizeof(double *))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < dl; i++)
    {
        if ((coefs[i] = (double *) calloc(k, sizeof(double))) == NULL)
        {
            error("couldn't malloc, exiting\n");
            exit(-1);
        }
    }

    for (i = 0; i < dl; i++)
    {
        coefs[i][0] = (double) reshape_1[i];
        coefs[i][1] = (double) reshape_2[i];
        coefs[i][2] = (double) slopes[i];
        coefs[i][3] = (double) y[i][0];
    }

    //ixexist = 0;

    if ((sort = (int *) calloc((length_x - 1), sizeof(int))) == NULL)
    {
        error("couldn't malloc, exiting\n");
        exit(-1);
    }

    for (i = 0; i < length_x - 1; i++)
    {
        if (i < length_x - 2)
        {
            sort[i] = (int) x[i];
        }
        else
        {
            sort[i] = (int) u;
        }
    }

    for (i = 0; i < length_x - 1; i++)
    {

    	if (i > 1)
        {
        index = i;
        //printf ("the value of index is : %d \n", index);
        }

    }

    xs = u - sort[index - 1];

    //d = 1;

    //dd = 1;

    v = coefs[index - 1][0];

    for (i = 1; i < k; i++)
    {
        v = xs * v + coefs[index - 1][i];
    }

    free(sort);

    for (i = 0; i < dl; i++)
    {
        free(coefs[i]);
    }

    free(coefs);
    free(reshape_2);
    free(reshape_1);
    free(dzdxdx);
    free(dzzdx);
    free(slopes);
    free(del);
    free(x);
    free(h);

    return (v);
}

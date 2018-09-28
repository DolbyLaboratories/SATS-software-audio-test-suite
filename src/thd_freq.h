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
;	File:	thd_freq.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#ifndef __THD_FREQ_H__
#define __THD_FREQ_H__

#define DWELLS_MAX      (5000)
#define THDFREQ_POINTS  (2)
#include "fio.h"
/*enum to define modes of the function*/
typedef enum
{
    THD_VS_FREQ = 0,
    FREQ_RESP = 1
} thd_freq_mode;

/*function to calculate the thd+N vs freq*/
int thd_freq( pfstruct pfs, thd_freq_mode mode, double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, double bad_dwells[DWELLS_MAX], int *bad_dwells_nrows );
double pwr_mean( double *blk, int blk_size );/*function to calculate the mean power*/
double std( double *pwr_levels, int pwr_levels_size );
double gradient_mean( double *pwr_levels, int pwr_levels_size );/*calculate the gradient*/
int add_points( double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, double c_freq, double reading_db );
void write_to_file( double points[DWELLS_MAX][THDFREQ_POINTS], int *nrows, pfstruct pfs );
int add_bad_dwells( double bad_dwells[DWELLS_MAX], int *bad_dwells_nrows, double c_freq );/*adding the bad dwells*/
int freq_resp( double *data, int len_data, double *reading_db ); /*function for frequency response*/
int thd_filt( double *data, int len_data, double c_freq, double fs, double *reading_db );/*filter for the THD tool*/
int notch2ndOrder( double Wo, double BW ); /*2nd order notch filter*/
double pchip_interp( double y[DWELLS_MAX][THDFREQ_POINTS], int length_y, int u );/*Hermit interpolation*/

#endif /* __THD_FREQ_H__ */

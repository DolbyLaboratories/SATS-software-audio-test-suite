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
;	File:	fft_avg.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fio.h"
#include "power.h"
#include "fft_avg.h"
#include "window.h"
#include "spect_NFFT.h"
#include "debug.h"

#include "SATS_fft.h"

//All buffers are aligned to 32 bytes in memory.
#define BUFFER_ALLIGN 32

static void
copy_doubles( double *pf, double *pt, int n )
{
    int i;

    for ( i = 0; i < n; i++ )
        *pt++ = *pf++;
}

int fft_avg( pfstruct pfs, double *po, int n )
{
    long block_size = pfs->fs;    // one second, 1 Hz resolution
    long block = 0;
    SATS_FFT_Complex *fft_out;   //Complex output datatype from MKL.
    SATS_FFT_HANDLE fft_handle;  // FFT handle for configuring the FFT.
    SATS_FFT_Status fft_status;  //Status variable for all FFT function returns.
    int i, nc;
    double mag = 0.0;
    double *pi, *pw, *pd;
    double wf;
    double re, im;
    double min_rms_db;
    
    if (pfs->minPowerSet == 1)
    {
        min_rms_db = pfs->minPower;
    }
    else
    {
        // Computing minimum representable dB level for the bit depth of the signal
        min_rms_db = 0.0 - floor(20*log10(pow(2, pfs->bitspersamp)));
    }
    
    //    output length
    nc = ( block_size / 2 ) + 1;

    if ( n < nc )
        return( -2 );

    // storage for raw samples
    pi = (double *)calloc( block_size , sizeof( double ) );
    if ( pi == NULL ) exit( -22 );

    //    create window
    pw = (double *)calloc( block_size , sizeof( double ) );
    if ( pw == NULL ) exit( -23 );
    blackmanharris( pw, block_size );
    //    compute window compensation
    wf = compute_window_comp( pw, block_size );

    // storage for windowed samples
    pd = (double *)SATS_FFT_malloc( block_size * sizeof( double ), BUFFER_ALLIGN );
    if ( pd == NULL ) exit( -24 );

    for (i=0;i<block_size;i++) pd[i]=0.0;
        
    //    storage for fft output
    fft_out = SATS_FFT_malloc ( sizeof ( SATS_FFT_Complex ) * nc, BUFFER_ALLIGN );
    
    if ( fft_out == NULL ) exit( -25 );
    
    for (i=0;i<nc;i++)
    {
        SATS_FFT_REAL(fft_out[i])=0.0;
        SATS_FFT_IMAG(fft_out[i])=0.0;
    }
         
    //    zero out output array
    for ( i = 0; i < nc; i++ )
        po[i] = 0.0;

    /*strip the leading silence if the -s option is mentioned by the user*/
    if ( !pfs->noSilence )
        strip_lead_silence( pfs );

    fft_status = SATS_FFT_Create(&fft_handle, block_size);

    fio_read( pfs, block_size / 2 );
    copy_doubles( pfs->data, pi, pfs->data_size );
    fio_read( pfs, block_size / 2 );
    copy_doubles( pfs->data, &pi[block_size/2], pfs->data_size );
    
    while( pfs->data_size == block_size / 2 )
        {
        window_array( pi, pw, pd, block_size );
        fft_status = SATS_FFT_ComputeForward(fft_handle, pd, fft_out);
        for ( i = 0; i < nc; i++ )
            {
            re = SATS_FFT_REAL(fft_out[i]);
            im = SATS_FFT_IMAG(fft_out[i]);
            mag = ( re * re ) + ( im * im );    // calculate power
            mag = mag * 2.0;                    // compensate for one sided fft
            mag = mag / ( pfs->fs * wf * block_size );    //    normalize for block size and window
            po[i] = po[i] + mag;
            }
        block++;
        fio_read( pfs, block_size / 2 );
        if ( pfs->data_size != block_size / 2 )
            break;
        copy_doubles( &pi[block_size/2], pi, block_size/2 );
        copy_doubles( pfs->data, &pi[block_size/2], block_size/2 );
        
        }
    
    for ( i = 0; i < nc; i++ )
    {
        // compensate for window and rms -> peak
        if ( i == 0 || i == nc - 1 )
            po[i] = ( 10.0 * log10( po[i] / (double) (block) ) ) + 3.02;
        else
            po[i] = ( 10.0 * log10( po[i] / (double) (block) ) ) + 3.010299957 + 3.02;
        if (po[i] < min_rms_db)
            {
            po[i] = min_rms_db;
            }
    }

    fft_status = SATS_FFT_Destroy(&fft_handle);

    SATS_FFT_free ( fft_out );
    SATS_FFT_free( pd );

    free( pi );
    free( pw );

    if ( block == 0 )
    {
        error("File too small to perform FFT\n");
        return( -1 );
    }
    fft_status = fft_status;
    return( nc );
}

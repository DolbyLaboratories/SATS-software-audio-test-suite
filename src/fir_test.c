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
;	File:	fir_test.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fio.h"
#include "fir_test.h"
#include "fir_filter.h"
#include "bandstop.h"
#include "debug.h"

FIR_FILTER pbs;

int fir_test( pfstruct pfs )
{
    unsigned long block_size = pfs->fs / 10;
    double *pd;
    //double rms_db;
    unsigned long block = 0;

    pbs.b = &B[0];
    pbs.bl = BL;
    if (init_fir_filter(&pbs) != 0)
    {
        error("Could not create FIR filter\n");
        return(-1);
    }

    strip_lead_silence(pfs);

    fio_read(pfs, block_size);

    pd = (double *) calloc(block_size, sizeof(double));

    while (pfs->data_size == (long) block_size)
    {
        //rms_db = 0.0;
        switch (pfs->fs)
        {
            case 32000:
                break;
            case 44100:
                break;
            case 48000:
                fir_filter_array(pfs->data, &pbs, pd, pfs->data_size);
                break;
        }
        block++;

        fio_read(pfs, block_size);
    }

    if (block == 0)
    {
        error("File too small to perform FFT\n");
        cleanup_fir_filter(&pbs);
        return (-1);
    }

    cleanup_fir_filter(&pbs);

    return (0);
}

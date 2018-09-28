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
;	File:	thd_ampl.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fio.h"
#include "thd_ampl.h"
#include "power.h"
#include "sos_filter.h"
#include "ta_filters.h"
#include "debug.h"
#include "Utilities.h"

// values returned are backwards from normal to allow a descending sort
static int compare_rev( const void *p1, const void *p2 )
{
    pDUPLE pd1, pd2;

    pd1 = (pDUPLE) p1;
    pd2 = (pDUPLE) p2;

    if (pd1->x > pd2->x)
        return (-1);
    if (pd1->x < pd2->x)
        return (1);

    return (0);
}

int thd_ampl( pfstruct pfs, int stop ) /*stop is 1 for stopband (THD) and 0 for passband (noise_mod)*/
{
    unsigned long block_size = (long) ((double) pfs->fs * 0.05);
    double *pd;
    double unfilt_db;
    double filt_db;
    unsigned long block = 0;
    pDUPLE pdum, pdu;
    int i, outsize;
    char *format = "%3.2lf,\t%3.2lf\n";

    init_ta_filters();

    strip_lead_silence(pfs);

    fio_read(pfs, block_size);

    //	allocate buffer for samples
    pd = (double *) calloc(pfs->fs, sizeof(double)); // ultimate size of array

    // 	allocate buffer for results
    pdum = (pDUPLE) calloc(((pfs->size / pfs->fs) + 2), sizeof(DUPLE));
    pdu = pdum;

    while (pfs->data_size == (long) block_size)
    {
        //	find the level of the tone
        unfilt_db = compute_power(pfs->data, pfs->data_size);
        if (unfilt_db != -999.0)
            unfilt_db = unfilt_db + 3.01;
        //	filter
        switch (pfs->fs)
        {
            case 32000:
                if (stop)
                    sos_filter_array(pfs->data, &no_4k_32000, pd, pfs->data_size);
                else
                    sos_filter_array(pfs->data, &bp_4k_32000, pd, pfs->data_size);
                break;
            case 44100:
                if (stop)
                    sos_filter_array(pfs->data, &no_4k_lp_44100, pd, pfs->data_size);
                else
                    sos_filter_array(pfs->data, &bp_4k_lp_44100, pd, pfs->data_size);
                break;
            case 48000:
                if (stop)
                    sos_filter_array(pfs->data, &no_4k_lp_48000, pd, pfs->data_size);
                else
                    sos_filter_array(pfs->data, &bp_4k_lp_48000, pd, pfs->data_size);
                break;
        }
        filt_db = compute_power(pd, pfs->data_size);
        if (filt_db != -999.0)
            filt_db = filt_db + 3.01;
        if (block == 0)
        {
            block_size = pfs->fs;
        }
        else
        {
            if (unfilt_db > -300.0 && filt_db > -300.0)
            {
                pdu->x = unfilt_db;
                pdu->y = filt_db;
                pdu++;
            }
        }

        fio_read(pfs, block_size);
        block++;
    }

    outsize = (int) (pdu - pdum);
    qsort(pdum, outsize, sizeof(DUPLE), compare_rev); // descending sort, because of compare_rev

    for (i = 0; i < outsize; i++)
    {
        check( sdf_writer_add_data_float_float(pfs->sdf_out, (float) pdum[i].x, (float) pdum[i].y, format) );
    }

    free_ta_filters();
    free(pdum);
    free(pd);

    if (block == 0)
    {
        error("File too small\n");
        return (-1);
    }

    return (0);
}

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
;	File:	amplitude_vs_time.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fio.h"
#include "debug.h"
#include "amplitude_vs_time.h"
#include "Utilities.h"

#define Max_secs 32 /*limit the results in each channel into a length of [32] seconds long*/

int amplitude_vs_time( pfstruct pfs )
{
    int i;
    unsigned long block_size = pfs->fs / 10;
    double time;
    unsigned long block = 0;
    char *format = "%3.5f,\t%3.5f\n";
    double endposition;

    double max_secsused = (double) Max_secs;

    //max_secsused= pfs->size/pfs->fs;

    //if (pfs->allChannels)
    //{
    //    max_secsused =  max_secsused / pfs->channels;
    //}
    if (pfs->xmaxSet)
        endposition =  ((double) pfs->xmax * pfs->fs);
    else
    {
        pfs->xmax = pfs->xmin + max_secsused;
        endposition =  ((double) pfs->xmax * pfs->fs);
    }
    time = (double) pfs->position / pfs->fs;

    if ((pfs->xmax - pfs->xmin) > (max_secsused)) /* view greater than max_secsused seconds not supported */
    {
        if (pfs->xmaxSet)
            info("Please select up to %.2f seconds of range, truncating view.\n", max_secsused);
        endposition = pfs->position + (max_secsused) * pfs->fs;
        pfs->xmax = pfs->xmin + max_secsused;
    }
    if (endposition > pfs->size)
        endposition = pfs->size;

    fio_read(pfs, block_size);

    while ((pfs->position < endposition) && (pfs->data_size == (long) block_size))
    {
        block++;

        for (i = 0; i < pfs->data_size; i++)
        {
            check( sdf_writer_add_data_double_double(pfs->sdf_out, time, pfs->data[i], format) );

            time = time + ((double) 1 / pfs->fs);
        }

        fio_read(pfs, block_size);

    }
    /*if ((pfs->position - endposition)>0) fio_read( pfs, pfs->position - endposition ); // To read left overs*/
    if (pfs->data_size > 0)
    {

        block++;
        for (i = 0; i < pfs->data_size; i++)
        {
            check( sdf_writer_add_data_double_double(pfs->sdf_out, time, pfs->data[i], format) );

            time = time + ((double) 1 / pfs->fs);
        }

    }
    if (block == 0)
    {
        error("File too small\n");
        return (-1);
    }
    return (0);
}


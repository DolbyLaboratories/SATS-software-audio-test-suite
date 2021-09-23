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
;	File:	power_vs_time.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h> 
#include "fio.h"
#include "power_vs_time.h"
#include "power.h"
#include "sos_filter.h"
#include "dr_filters.h"
#include "debug.h"
#include "Utilities.h"

#define MIN_BLOCK_SIZE 1

int power_vs_time( pfstruct pfs, int dnr )
{
    /*Apply chosen block size or use default*/

    double block_size_time = 100.0;
    unsigned long block_size;   
    unsigned long block = 0;

    double time;
    double *pd;
    double rms_db;
    double min_rms_db;

    char format[] = "%3.6lf,\t%3.2lf\n";
    

   /* Check if block size was set in samples ...*/
    if ( pfs->blksz_sSet == 1 )
    { 
    block_size = (unsigned long) pfs->blksz_s;
    }
    /* ... or in ms */
    else if ( pfs->blksz_tSet == 1 )
    {
    block_size_time = pfs->blksz_t;
    block_size = (unsigned long) floor((pfs->fs / 1000.0) * block_size_time);
    }
    else
    {
    block_size = (unsigned long) floor((pfs->fs / 1000.0) * block_size_time);
    }

    /* Check if block size < MIN_BLOCK_SIZE*/
    if (block_size < MIN_BLOCK_SIZE)
    {
    error("Chosen block size too small (less than 1 sample)\n");
    return(-1);
    }
   
    /* Check if block size > MAX sensible value, value of DBL_MAX defined in float.h */

    if (block_size > DBL_MAX)
    {
    error("Chosen block size is greater than maximum possible value.\n");
    return(-1);
    }

    /* Check if block size > total duration of the signal*/
    if ((long) block_size > pfs->size)
    {
    error("Chosen block size is greater than total signal duration. Choose a value less than total signal duration.\n");
    return(-1);
    }

    time = (double) block_size / (2.0 * pfs->fs);

    if (pfs->minPowerSet == 1)
    {
    min_rms_db = pfs->minPower;
    }
    else
    {
    /* Computing minimum representable dB level for the bit depth of the signal */
    min_rms_db = 0.0 - floor(20*log10(pow(2, pfs->bitspersamp)));
    }
            
    block = 0;
    init_dr_filters();

    if (!pfs->noSilence)
    {
      strip_lead_silence(pfs);
    }

    fio_read(pfs, block_size);

    pd = (double *) calloc(block_size, sizeof(double));
    if (pd == NULL)
    {
    error("malloc failed in power_vs_time\n");
    return (-1);
    }

    while (pfs->data_size == (long) block_size)
    {
      if (dnr)
      {
          rms_db = 0.0;
          switch (pfs->fs) /* switch case for diff sample sizes */
          {
            case 32000:
               sos_filter_array(pfs->data, &dr_32000, pd, pfs->data_size);
               break;
            case 44100:
               sos_filter_array(pfs->data, &dr_44100, pd, pfs->data_size);
               break;
            case 48000:
               sos_filter_array(pfs->data, &dr_48000, pd, pfs->data_size);
               break;
          } /*end of switch case*/
          rms_db = compute_power(pd, pfs->data_size);
      }
      else
      {
          rms_db = compute_power(pfs->data, pfs->data_size);
          block++;
      }

      if (dnr)
      {
        /* rms_db = rms_db - 5.629 + 3.010299957; */
	// matlab tools use 3.01
        rms_db = rms_db - 5.629 + 3.01;

        if (rms_db < min_rms_db)
        {
          rms_db = min_rms_db;
        }

        if (block++ != 0) // Skip first data block in case of DNR 
        {
            check( sdf_writer_add_data_double_double(pfs->sdf_out, time, rms_db, format) );
        }
      }
      else
      {
      //rms_db = rms_db + 3.010299957;	// matlab tools use 3.01
        rms_db = rms_db + 3.01;

        if (rms_db < min_rms_db)
        {
          rms_db = min_rms_db;
        }

        check( sdf_writer_add_data_double_double(pfs->sdf_out, time, rms_db, format) );
      }

      fio_read(pfs, block_size);
      time = time + ((double) block_size / pfs->fs);
    }

    free_dr_filters();
    free(pd);
    if (block == 0)
    {
      error("File too small\n");
      return (-1);
    }
    return (0);
}

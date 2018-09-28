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
;	File:	dr_filters.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "sos_filter.h"
#include "dr_filters.h"
#include "dr_coef.h"
#include "lp_coef.h"
#include "tmwtypes.h"

extern int lp_44100_bl[];
extern real64_T lp_44100_b[][3];
extern int lp_44100_al[];
extern real64_T lp_44100_a[][3];

extern int lp_48000_bl[];
extern real64_T lp_48000_b[][3];
extern int lp_48000_al[];
extern real64_T lp_48000_a[][3];

/* dynamic range 200 Hz notch filter for 32000 kHz sample rate */
SOS_FILTER dr_no_32000 =
	{
	DR_NO_32000_NSEC,
	dr_no_32000_bl,
	dr_no_32000_b,
	dr_no_32000_al,
	dr_no_32000_a,
	z1,
	z2
	};

/* dynamic range ccir468 filter for 32000 kHz sample rate */
SOS_FILTER ccir468_32000 =
	{
	CCIR468_32000_NSEC,
	ccir468_32000_bl,
	ccir468_32000_b,
	ccir468_32000_al,
	ccir468_32000_a,
	z1,
	z2
	};

/* dynamic range filters concatenated for 32000 kHz sample rate */
SOS_FILTER dr_32000 =
	{
	DR_32000_NSEC,
	dr_32000_bl,
	dr_32000_b,
	dr_32000_al,
	dr_32000_a,
	z1,
	z2
	};

/* dynamic range 200 Hz notch filter for 44100 kHz sample rate */
SOS_FILTER dr_no_44100 =
	{
	DR_NO_44100_NSEC,
	dr_no_44100_bl,
	dr_no_44100_b,
	dr_no_44100_al,
	dr_no_44100_a,
	z1,
	z2
	};

/* dynamic range low pass filter for 44100 kHz sample rate */
SOS_FILTER dr_lp_44100 =
	{
	LP_44100_NSEC,
	lp_44100_bl,
	lp_44100_b,
	lp_44100_al,
	lp_44100_a,
	z1,
	z2
	};

/* dynamic range ccir468 filter for 44100 kHz sample rate */
SOS_FILTER ccir468_44100 =
	{
	CCIR468_44100_NSEC,
	ccir468_44100_bl,
	ccir468_44100_b,
	ccir468_44100_al,
	ccir468_44100_a,
	z1,
	z2
	};

/* dynamic range filters concatenated for 44100 kHz sample rate */
SOS_FILTER dr_44100 =
	{
	DR_44100_NSEC,
	dr_44100_bl,
	dr_44100_b,
	dr_44100_al,
	dr_44100_a,
	z1,
	z2
	};

/* dynamic range 200 Hz notch filter for 48000 kHz sample rate */
SOS_FILTER dr_no_48000 =
	{
	DR_NO_48000_NSEC,
	dr_no_48000_bl,
	dr_no_48000_b,
	dr_no_48000_al,
	dr_no_48000_a,
	z1,
	z2
	};

/* dynamic range low pass filter for 48000 kHz sample rate */
SOS_FILTER dr_lp_48000 =
	{
	LP_48000_NSEC,
	lp_48000_bl,
	lp_48000_b,
	lp_48000_al,
	lp_48000_a,
	z1,
	z2
	};

/* dynamic range ccir468 filter for 48000 kHz sample rate */
SOS_FILTER ccir468_48000 =
	{
	CCIR468_48000_NSEC,
	ccir468_48000_bl,
	ccir468_48000_b,
	ccir468_48000_al,
	ccir468_48000_a,
	z1,
	z2
	};

/* dynamic range filters concatenated for 48000 kHz sample rate */
SOS_FILTER dr_48000 =
	{
	DR_48000_NSEC,
	dr_48000_bl,
	dr_48000_b,
	dr_48000_al,
	dr_48000_a,
	z1,
	z2
	};

SOS_FILTER on_fly_notch =
	{
	ON_FLY_NOTCH_NSEC,
	on_fly_notch_bl,
	on_fly_notch_b,
	on_fly_notch_al,
	on_fly_notch_a,
	z1,
	z2
	};
/*
SOS_FILTER ellip_44100 =
	{
	ELLIP_44100_NSEC,
	ellip_44100_bl,
	ellip_44100_b,
	ellip_44100_al,
	ellip_44100_a
	};

SOS_FILTER ellip_48000 =
	{
	ELLIP_48000_NSEC,
	ellip_48000_bl,
	ellip_48000_b,
	ellip_48000_al,
	ellip_48000_a
	};			
*/	

void init_dr_filters( void )
{
    if (init_sos_filter(&dr_no_32000))
        exit(-10);
    if (init_sos_filter(&ccir468_32000))
        exit(-10);
    if (init_sos_filter(&dr_32000))
        exit(-10);

    if (init_sos_filter(&dr_no_44100))
        exit(-10);
    if (init_sos_filter(&dr_lp_44100))
        exit(-10);
    if (init_sos_filter(&ccir468_44100))
        exit(-10);
    if (init_sos_filter(&dr_44100))
        exit(-10);

    if (init_sos_filter(&dr_no_48000))
        exit(-10);
    if (init_sos_filter(&dr_lp_48000))
        exit(-10);
    if (init_sos_filter(&ccir468_48000))
        exit(-10);
    if (init_sos_filter(&dr_48000))
        exit(-10);

    if (init_sos_filter(&on_fly_notch))
        exit(-10);

}
void free_dr_filters( void )
{
    if (free_sos_filter(&dr_no_32000))
        exit(-10);
    if (free_sos_filter(&ccir468_32000))
        exit(-10);
    if (free_sos_filter(&dr_32000))
        exit(-10);

    if (free_sos_filter(&dr_no_44100))
        exit(-10);
    if (free_sos_filter(&dr_lp_44100))
        exit(-10);
    if (free_sos_filter(&ccir468_44100))
        exit(-10);
    if (free_sos_filter(&dr_44100))
        exit(-10);

    if (free_sos_filter(&dr_no_48000))
        exit(-10);
    if (free_sos_filter(&dr_lp_48000))
        exit(-10);
    if (free_sos_filter(&ccir468_48000))
        exit(-10);
    if (free_sos_filter(&dr_48000))
        exit(-10);

    if (free_sos_filter(&on_fly_notch))
        exit(-10);

}

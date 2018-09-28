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
;	File:	ta_filters.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "sos_filter.h"
#include "ta_filters.h"
#include "ta_coef.h"
#include "lp_coef.h"

//	notch filter for 4kHz for 32kHz sample rate
SOS_FILTER no_4k_32000 =
	{
	NO_4K_32000_NSEC,
	no_4k_32000_bl,
	no_4k_32000_b,
	no_4k_32000_al,
	no_4k_32000_a,
	z1,
	z2
	};

// bandpass filter for 32kHz sample rate
SOS_FILTER bp_4k_32000 =
	{
	BP_4K_32000_NSEC,
	bp_4k_32000_bl,
	bp_4k_32000_b,
	bp_4k_32000_al,
	bp_4k_32000_a,
	z1,
	z2
	};

//	notch filter for 4kHz for 44.1kHz sample rate
SOS_FILTER no_4k_44100 =
	{
	NO_4K_44100_NSEC,
	no_4k_44100_bl,
	no_4k_44100_b,
	no_4k_44100_al,
	no_4k_44100_a,
	z1,
	z2
	};

// bandpass filter for 44.1kHz sample rate
SOS_FILTER bp_4k_44100 =
	{
	BP_4K_44100_NSEC,
	bp_4k_44100_bl,
	bp_4k_44100_b,
	bp_4k_44100_al,
	bp_4k_44100_a,
	z1,
	z2
	};

//	notch filter for 4kHz, then lowpass for 44.1kHz sample rate
SOS_FILTER no_4k_lp_44100 =
	{
	NO_4K_LP_44100_NSEC,
	no_4k_lp_44100_bl,
	no_4k_lp_44100_b,
	no_4k_lp_44100_al,
	no_4k_lp_44100_a,
	z1,
	z2
	};

// bandpass filter, then lowpass for 44.1kHz sample rate
SOS_FILTER bp_4k_lp_44100 =
	{
	BP_4K_LP_44100_NSEC,
	bp_4k_lp_44100_bl,
	bp_4k_lp_44100_b,
	bp_4k_lp_44100_al,
	bp_4k_lp_44100_a,
	z1,
	z2
	};

//	notch filter for 4kHz
SOS_FILTER no_4k_48000 =
	{
	NO_4K_48000_NSEC,
	no_4k_48000_bl,
	no_4k_48000_b,
	no_4k_48000_al,
	no_4k_48000_a,
	z1,
	z2
	};

// bandpass filter for 4kHz
SOS_FILTER bp_4k_48000 =
	{
	BP_4K_48000_NSEC,
	bp_4k_48000_bl,
	bp_4k_48000_b,
	bp_4k_48000_al,
	bp_4k_48000_a,
	z1,
	z2
	};

//	notch filter for 4kHz, then low pass
SOS_FILTER no_4k_lp_48000 =
	{
	NO_4K_LP_48000_NSEC,
	no_4k_lp_48000_bl,
	no_4k_lp_48000_b,
	no_4k_lp_48000_al,
	no_4k_lp_48000_a,
	z1,
	z2
	};

// bandpass filter for 4kHz, then low pass
SOS_FILTER bp_4k_lp_48000 =
	{
	BP_4K_LP_48000_NSEC,
	bp_4k_lp_48000_bl,
	bp_4k_lp_48000_b,
	bp_4k_lp_48000_al,
	bp_4k_lp_48000_a,
	z1,
	z2
	};


void init_ta_filters( void )
{
    if (init_sos_filter(&no_4k_32000))
        exit(-10);
    if (init_sos_filter(&bp_4k_32000))
        exit(-10);

    if (init_sos_filter(&no_4k_44100))
        exit(-10);
    if (init_sos_filter(&bp_4k_44100))
        exit(-10);

    if (init_sos_filter(&no_4k_lp_44100))
        exit(-10);
    if (init_sos_filter(&bp_4k_lp_44100))
        exit(-10);

    if (init_sos_filter(&no_4k_48000))
        exit(-10);
    if (init_sos_filter(&bp_4k_48000))
        exit(-10);

    if (init_sos_filter(&no_4k_lp_48000))
        exit(-10);
    if (init_sos_filter(&bp_4k_lp_48000))
        exit(-10);
}

void free_ta_filters( void )
{
    if (free_sos_filter(&no_4k_32000))
        exit(-10);
    if (free_sos_filter(&bp_4k_32000))
        exit(-10);

    if (free_sos_filter(&no_4k_44100))
        exit(-10);
    if (free_sos_filter(&bp_4k_44100))
        exit(-10);

    if (free_sos_filter(&no_4k_lp_44100))
        exit(-10);
    if (free_sos_filter(&bp_4k_lp_44100))
        exit(-10);

    if (free_sos_filter(&no_4k_48000))
        exit(-10);
    if (free_sos_filter(&bp_4k_48000))
        exit(-10);

    if (free_sos_filter(&no_4k_lp_48000))
        exit(-10);
    if (free_sos_filter(&bp_4k_lp_48000))
        exit(-10);
}

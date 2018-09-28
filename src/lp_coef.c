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
;	File:	lp_coef.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include "tmwtypes.h"
#include "lp_coef.h"

/* Low Pass filters */

/* 44.1kHz Sample Rate Filters */


int lp_44100_bl[LP_44100_NSEC] = { 1,3,1,3,1,3,1,3,1,3,1,3,1 };
real64_T lp_44100_b[LP_44100_NSEC][3] = {
  { 0.3264435464712,                 0,                 0 },
  { 1,    1.998757475239,   0.9999999999997 },
  { 1,                 0,                 0 },
  { 1,    1.989993373019,   0.9999999999998 },
  { 1,                 0,                 0 },
  { 1,    1.977254757142,                 1 },
  { 1,                 0,                 0 },
  { 1,     1.96560453861,   0.9999999999995 },
  { 1,                 0,                 0 },
  { 1,    1.957564737001,                 1 },
  { 1,                 0,                 0 },
  { 1,    1.953621726441,   0.9999999999999 },
  { 1,                 0,                 0 }
};
int lp_44100_al[LP_44100_NSEC] = { 1,3,1,3,1,3,1,3,1,3,1,3,1 };
real64_T lp_44100_a[LP_44100_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   0.8321624415298,   0.2336661152884 },
  { 1,                 0,                 0 },
  { 1,    1.443215547293,   0.6571177836971 },
  { 1,                 0,                 0 },
  { 1,    1.723522054025,   0.8517567075263 },
  { 1,                 0,                 0 },
  { 1,    1.836701633172,   0.9310881176054 },
  { 1,                 0,                 0 },
  { 1,      1.8886071568,   0.9687780372787 },
  { 1,                 0,                 0 },
  { 1,    1.915942311091,   0.9909283814552 },
  { 1,                 0,                 0 }
};

 /* 48kHz Sample Rate Filters */
int lp_48000_bl[LP_48000_NSEC] = { 1,3,1,3,1,3,1,3,1,3,1,3,1 };
real64_T lp_48000_b[LP_48000_NSEC][3] = {
  { 0.1275989262439,                 0,                 0 },
  { 1,    1.995675171563,    1.000000000006 },
  { 1,                 0,                 0 },
  { 1,    1.965358497856,   0.9999999999984 },
  { 1,                 0,                 0 },
  { 1,    1.921873305357,    1.000000000002 },
  { 1,                 0,                 0 },
  { 1,    1.882693109174,   0.9999999999987 },
  { 1,                 0,                 0 },
  { 1,    1.855976944139,    1.000000000001 },
  { 1,                 0,                 0 },
  { 1,    1.842969114423,   0.9999999999998 },
  { 1,                 0,                 0 }
};
int lp_48000_al[LP_48000_NSEC] = { 1,3,1,3,1,3,1,3,1,3,1,3,1 };
real64_T lp_48000_a[LP_48000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   0.2109884926187,  0.09547522466767 },
  { 1,                 0,                 0 },
  { 1,   0.9190570840754,   0.5005607119249 },
  { 1,                 0,                 0 },
  { 1,    1.367593671118,   0.7581128988096 },
  { 1,                 0,                 0 },
  { 1,    1.579771612679,   0.8818361401189 },
  { 1,                 0,                 0 },
  { 1,    1.682557369684,   0.9451969883445 },
  { 1,                 0,                 0 },
  { 1,    1.735231834136,   0.9838826125212 },
  { 1,                 0,                 0 }
};


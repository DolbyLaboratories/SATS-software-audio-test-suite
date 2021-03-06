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
;	File:	ta_coef.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#ifndef __TA_COEF_H__
#define __TA_COEF_H__

#include "tmwtypes.h"

//
//	32kHz Sample Rate Filters
//
#define z1 NULL
#define z2 NULL

#define NO_4K_32000_NSEC 9
 int no_4k_32000_bl[NO_4K_32000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_32000_b[NO_4K_32000_NSEC][3] = {
  { 0.8721885623254,                 0,                 0 },
  { 1,   -1.414213562373,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.414213562373,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.414213562373,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.414213562373,                 1 },
  { 1,                 0,                 0 }
};

 int no_4k_32000_al[NO_4K_32000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_32000_a[NO_4K_32000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.366682377855,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.366682377855,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.366682377855,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.366682377855,   0.9327807542182 },
  { 1,                 0,                 0 }
};

#define BP_4K_32000_NSEC 9
 int bp_4k_32000_bl[BP_4K_32000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_32000_b[BP_4K_32000_NSEC][3] = {
  { 1.557727862934e-006,                 0,                 0 },
  { 1, -0.05889419444987,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.879015468291,                 1 },
  { 1,                 0,                 0 },
  { 1,  -0.9143784524301,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.707346209925,   0.9999999999999 },
  { 1,                 0,                 0 }
};

 int bp_4k_32000_al[BP_4K_32000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_32000_a[BP_4K_32000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.393763966652,   0.9867406367536 },
  { 1,                 0,                 0 },
  { 1,    -1.41642220176,   0.9869501238575 },
  { 1,                 0,                 0 },
  { 1,   -1.382799673538,   0.9944463132202 },
  { 1,                 0,                 0 },
  { 1,   -1.437416461937,   0.9946562156666 },
  { 1,                 0,                 0 }
};

//
//	44.1kHz Sample Rate Filters
//

// 4kHz notch filter for THD+N
#define NO_4K_44100_NSEC 9
 int no_4k_44100_bl[NO_4K_44100_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_44100_b[NO_4K_44100_NSEC][3] = {
  { 0.8721885623254,                 0,                 0 },
  { 1,   -1.683906167846,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.683906167846,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.683906167846,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.683906167846,                 1 },
  { 1,                 0,                 0 }
};
 int no_4k_44100_al[NO_4K_44100_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_44100_a[NO_4K_44100_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,                 0,                 0 }
};

// 4kHz notch filter followed by 20100Hz low pass
#define NO_4K_LP_44100_NSEC 12
 int no_4k_lp_44100_bl[NO_4K_LP_44100_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T no_4k_lp_44100_b[NO_4K_LP_44100_NSEC][3] = {
  { 0.8721885623254,                 0,                 0 },
  { 1,   -1.683906167846,                 1 },
  { 1,   -1.683906167846,                 1 },
  { 1,   -1.683906167846,                 1 },
  { 1,   -1.683906167846,                 1 },
  //	lowpass
  { 0.3264435464712,                 0,                 0 },
  { 1,    1.998757475239,   0.9999999999997 },
  { 1,    1.989993373019,   0.9999999999998 },
  { 1,    1.977254757142,                 1 },
  { 1,     1.96560453861,   0.9999999999995 },
  { 1,    1.957564737001,                 1 },
  { 1,    1.953621726441,   0.9999999999999 }
};
 int no_4k_lp_44100_al[NO_4K_LP_44100_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T no_4k_lp_44100_a[NO_4K_LP_44100_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,   -1.627310716561,   0.9327807542182 },
  { 1,   -1.627310716561,   0.9327807542182 },
  // 	lowpass
  { 1,                 0,                 0 },
  { 1,   0.8321624415298,   0.2336661152884 },
  { 1,    1.443215547293,   0.6571177836971 },
  { 1,    1.723522054025,   0.8517567075263 },
  { 1,    1.836701633172,   0.9310881176054 },
  { 1,      1.8886071568,   0.9687780372787 },
  { 1,    1.915942311091,   0.9909283814552 }
};

#define BP_4K_44100_NSEC 9
 int bp_4k_44100_bl[BP_4K_44100_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_44100_b[BP_4K_44100_NSEC][3] = {
  { 1.286228000225e-006,                 0,                 0 },
  { 1,   -0.777449756023,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.934272257186,   0.9999999999999 },
  { 1,                 0,                 0 },
  { 1,   -1.390893175439,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.842688442774,                 1 },
  { 1,                 0,                 0 }
};
 int bp_4k_44100_al[BP_4K_44100_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_44100_a[BP_4K_44100_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.669653837785,   0.9903513630323 },
  { 1,                 0,                 0 },
  { 1,   -1.682283998073,   0.9905241026682 },
  { 1,                 0,                 0 },
  { 1,   -1.665252282229,   0.9959564894443 },
  { 1,                 0,                 0 },
  { 1,   -1.695579053616,   0.9961292130839 },
  { 1,                 0,                 0 }
};

#define BP_4K_LP_44100_NSEC 12
 int bp_4k_lp_44100_bl[BP_4K_LP_44100_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T bp_4k_lp_44100_b[BP_4K_LP_44100_NSEC][3] = {
  { 1.286228000225e-006,                 0,                 0 },
  { 1,   -0.777449756023,                 1 },
  { 1,   -1.934272257186,   0.9999999999999 },
  { 1,   -1.390893175439,                 1 },
  { 1,   -1.842688442774,                 1 },
  //	lowpass
  { 0.3264435464712,                 0,                 0 },
  { 1,    1.998757475239,   0.9999999999997 },
  { 1,    1.989993373019,   0.9999999999998 },
  { 1,    1.977254757142,                 1 },
  { 1,     1.96560453861,   0.9999999999995 },
  { 1,    1.957564737001,                 1 },
  { 1,    1.953621726441,   0.9999999999999 }
};
 int bp_4k_lp_44100_al[BP_4K_LP_44100_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T bp_4k_lp_44100_a[BP_4K_LP_44100_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.669653837785,   0.9903513630323 },
  { 1,   -1.682283998073,   0.9905241026682 },
  { 1,   -1.665252282229,   0.9959564894443 },
  { 1,   -1.695579053616,   0.9961292130839 },
  // 	lowpass
  { 1,                 0,                 0 },
  { 1,   0.8321624415298,   0.2336661152884 },
  { 1,    1.443215547293,   0.6571177836971 },
  { 1,    1.723522054025,   0.8517567075263 },
  { 1,    1.836701633172,   0.9310881176054 },
  { 1,      1.8886071568,   0.9687780372787 },
  { 1,    1.915942311091,   0.9909283814552 }
};
//
//	48kHz Sample Rate Filters
//

#define NO_4K_48000_NSEC 9
 int no_4k_48000_bl[NO_4K_48000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_48000_b[NO_4K_48000_NSEC][3] = {
  { 0.8721885623254,                 0,                 0 },
  { 1,   -1.732050807569,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.732050807569,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.732050807569,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.732050807569,                 1 },
  { 1,                 0,                 0 }
};

 int no_4k_48000_al[NO_4K_48000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T no_4k_48000_a[NO_4K_48000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,                 0,                 0 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,                 0,                 0 }
};

#define NO_4K_LP_48000_NSEC 12
 int no_4k_lp_48000_bl[NO_4K_LP_48000_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T no_4k_lp_48000_b[NO_4K_LP_48000_NSEC][3] = {
  { 0.8721885623254,                 0,                 0 },
  { 1,   -1.732050807569,                 1 },
  { 1,   -1.732050807569,                 1 },
  { 1,   -1.732050807569,                 1 },
  { 1,   -1.732050807569,                 1 },
  //	low pass
  { 0.1275989262439,                 0,                 0 },
  { 1,    1.995675171563,    1.000000000006 },
  { 1,    1.965358497856,   0.9999999999984 },
  { 1,    1.921873305357,    1.000000000002 },
  { 1,    1.882693109174,   0.9999999999987 },
  { 1,    1.855976944139,    1.000000000001 },
  { 1,    1.842969114423,   0.9999999999998 },
};

 int no_4k_lp_48000_al[NO_4K_LP_48000_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T no_4k_lp_48000_a[NO_4K_LP_48000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,   -1.673837233099,   0.9327807542182 },
  { 1,   -1.673837233099,   0.9327807542182 },
  //	low pass
  { 1,                 0,                 0 },
  { 1,   0.2109884926187,  0.09547522466767 },
  { 1,   0.9190570840754,   0.5005607119249 },
  { 1,    1.367593671118,   0.7581128988096 },
  { 1,    1.579771612679,   0.8818361401189 },
  { 1,    1.682557369684,   0.9451969883445 },
  { 1,    1.735231834136,   0.9838826125212 },
};

#define BP_4K_48000_NSEC 9
 int bp_4k_48000_bl[BP_4K_48000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_48000_b[BP_4K_48000_NSEC][3] = {
  { 1.240780983352e-006,                 0,                 0 },
  { 1,  -0.9323999174998,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.944221102086,    1.000000000001 },
  { 1,                 0,                 0 },
  { 1,    -1.48009018957,                 1 },
  { 1,                 0,                 0 },
  { 1,   -1.866739488078,   0.9999999999991 },
  { 1,                 0,                 0 }
};
 int bp_4k_48000_al[BP_4K_48000_NSEC] = { 1,3,1,3,1,3,1,3,1 };
 real64_T bp_4k_48000_a[BP_4K_48000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.719151456744,   0.9911303232335 },
  { 1,                 0,                 0 },
  { 1,   -1.729925201271,   0.9912922673717 },
  { 1,                 0,                 0 },
  { 1,   -1.715843320582,   0.9962827713976 },
  { 1,                 0,                 0 },
  { 1,   -1.741684237502,   0.9964446600911 },
  { 1,                 0,                 0 }
};

#define BP_4K_LP_48000_NSEC 12
 int bp_4k_lp_48000_bl[BP_4K_LP_48000_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T bp_4k_lp_48000_b[BP_4K_LP_48000_NSEC][3] = {
  { 1.240780983352e-006,                 0,                 0 },
  { 1,  -0.9323999174998,                 1 },
  { 1,   -1.944221102086,    1.000000000001 },
  { 1,    -1.48009018957,                 1 },
  { 1,   -1.866739488078,   0.9999999999991 },
  //	low pass
  { 0.1275989262439,                 0,                 0 },
  { 1,    1.995675171563,    1.000000000006 },
  { 1,    1.965358497856,   0.9999999999984 },
  { 1,    1.921873305357,    1.000000000002 },
  { 1,    1.882693109174,   0.9999999999987 },
  { 1,    1.855976944139,    1.000000000001 },
  { 1,    1.842969114423,   0.9999999999998 },
};
 int bp_4k_lp_48000_al[BP_4K_LP_48000_NSEC] = { 1,3,3,3,3, 1,3,3,3,3,3,3 };
 real64_T bp_4k_lp_48000_a[BP_4K_LP_48000_NSEC][3] = {
  { 1,                 0,                 0 },
  { 1,   -1.719151456744,   0.9911303232335 },
  { 1,   -1.729925201271,   0.9912922673717 },
  { 1,   -1.715843320582,   0.9962827713976 },
  { 1,   -1.741684237502,   0.9964446600911 },
  //	low pass
  { 1,                 0,                 0 },
  { 1,   0.2109884926187,  0.09547522466767 },
  { 1,   0.9190570840754,   0.5005607119249 },
  { 1,    1.367593671118,   0.7581128988096 },
  { 1,    1.579771612679,   0.8818361401189 },
  { 1,    1.682557369684,   0.9451969883445 },
  { 1,    1.735231834136,   0.9838826125212 },
};
#endif // __TA_COEF_H__

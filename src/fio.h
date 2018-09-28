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
;	File:	fio.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#ifndef __FSTRUCT_H__
#define __FSTRUCT_H__

#include "sdf_writer.h"
#include "wavelib.h"
#include <stdio.h>

typedef struct
{
    char filename[1024];
    char OptionInputFile[1024]; /* Optional input file mainly used for "Mult_freq_resp.exe" */
    FILE *fp;                   /* input file */
    int channel;                /* channel to test */
    short channels;               /* total number of channels */
    short allChannels;            /* true if -ca option used */
    long position;              /* in samples */
    long size;                  /* size of the entire file in samples */
    long fs;                    /* in Hertz */

    double *data;               /* pointer to data */
    long data_position;         /* in samples */
    long data_size;             /* in samples */
    unsigned long malloc_size;  /* size of "data" in bytes */


    unsigned long header_size;  /* wave file header size in bytes */
    short bitspersamp;
    short bytespersamp;
    int audio_type;             /* WAV audio format (1: PCM, 3: FLOAT) */
    short format_tag;           /* Flag set if wav file type is PCM type   */

    int time;                   /*used as a counter for the number of frames in spectrogram*/
    short channelMap;           /* true if -m option used */
    short stdoutFlag;           /* -t option, data written to standard output if true */
    char dataOutputFile[1024];  /* result of -to option, create a text file */

    axis_t axes[4];             /* 1st + 2nd data axis, channel axis, channel_name axis */
    axis_t spect_time_axes[5];  /*3rd axes for spectrogram tool*/
    sdf_config_t header;        /* binary output configuration (axes etc.) */
    sdf_writer_t sdf_out;       /* binary output handle, or NULL */
    short binary_out;           /* true iff binary output is requested (-w option) */

    short blksz_tSet;           /* flag, -blksz_t option used */
    double blksz_t;             /* value, block size in ms*/
    short blksz_sSet;           /* flag, -blksz_s option used */
    unsigned long blksz_s;      /* value, block size in samples*/

    short minPowerSet;          /* flag, -powermin option used */
    double minPower;            /* value, all powers below this value will be clipped */

    short xminSet;				/* flag, -xmin option used */
    double xmin;				/* value, undefined if xminSet false */
    short xmaxSet;				/* flag, -xmax option used */
    double xmax;				/* value, undefined if xmaxSet false */

    short noSilence;            /* turn off strip_lead_silence */
    
    short windowtype;           /* Window type for spectrogram tool */
    short top;                  /* output top envelope data, mainly used for "res_envelope" */
    short bottom;               /* output bottom envelope data, mainly used for "res_envelope" */
    long nfft;                /* variable for fft block size, added and mainly used for spectrum_NFFT */
    long navg;                /* variable for number of averages, added and mainly used for spectrum_NFFT */
    double stride;              /*variable for hop size */
    short hopsize_set;          /*Flag set when hop size is set by user*/
    char *tool;                 /* variable to hold the tools name, added and used for spectrum_NFFT; */
    double sample_thr;
    double block_thr;
    short  thr_dbSet;           /* flag for block_thr*/ 
    short  thr_sSet;            /* flag for sample_thr*/ 
    
} fstruct, *pfstruct; /*declare a structure and define it to store the values for file i/p and o/p operations*/

/* This is the function that initializes the files to be read/written
 */
int fio_init( pfstruct pfs, 
              const char *tool,  /* name of SATS tool */
              const char *axis1, /* name of 1st data axis */
              const char *axis2, /* name of 2nd data axis */
              const char *axis3  /* name of 3rd data axis*/
    );

/* Start to write data for the next channel */
int fio_next_chunk( pfstruct pfs, int ch );

int fio_eof( pfstruct pfs ); /*function that checks for the end of file */

int fio_read( pfstruct pfs, unsigned long num ); /*Read the file*/

int fio_setpos( pfstruct pfs, long new_pos ); /*set the position to start reading the file*/

int fio_resetpos( pfstruct pfs ); /*reset the starting postion*/

int fio_cleanup( pfstruct pfs ); /*cleanup function*/

void strip_lead_silence( pfstruct pfs ); /*function to strip the leading silence*/

void getChannelLabel( pfstruct pfs, int channel, char *cp );/*get the channel label*/

#endif /*__FSTRUCT_H__*/

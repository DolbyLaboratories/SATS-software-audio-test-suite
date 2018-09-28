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
;	File:	mult_freq_resp.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include "fio.h"
#include "mult_freq_resp.h"
#include "wavelib.h"
#include "fft_avg.h"
#include "parse_args.h"
#include "Utilities.h"
#include "debug.h"
#include "version.h"

extern void print_usage( void );

char *chart = "title, Multiple Tone Frequency Response,\nxlabel, Frequency (Hz),\nylabel, Amplitude (dBFS),\nxlog,\n";

fstruct fst; 

int main( int argc, char *argv[] )
{
    int startCh, endCh;
    int ch;
	int res;

    if (parse_args("mult_freq_resp", argc, argv, &fst) != 0)
    {
        return 1;
    }

    if (fst.allChannels)
    {
        startCh = 0;
        endCh = fst.channels;
    }
    else
    {
        startCh = fst.channel;
        endCh = fst.channel + 1;
    }

    debugInit();
    check( fio_init(&fst, "mult_freq_resp", "Frequency", "Amplitude", NULL) );

    for (ch = startCh; ch < endCh; ch++)
    {
        check( fio_next_chunk(&fst, ch) );
        fio_resetpos(&fst);
        strip_lead_silence(&fst);
        res=multiple_frequency_response(&fst,0);
		if (res)
		{
			return (1);
		}
    }
    fio_cleanup(&fst);
    debugCleanup();

    return 0;
}

void print_usage( void )
{
    fprintf(stderr,"Multitone Frequency Response Tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr,"Usage: mult_freq_resp [OPTION]... -f <.txt file containing list of multitone frequencies> -i WAVFILE\n\n");
    fprintf(stderr,"-c <chan>,           selects channel in multichannel file\n");
    fprintf(stderr,"                     0 = first channel\n");
    fprintf(stderr, "-s,                 turn off stripping lead silence\n" );
	fprintf(stderr, "-thr_db <thr>,      select the power threshold for silence stripping in db\n");
	fprintf(stderr, "-thr_s <thr>,       select the sample threshold for silence stripping in bit\n");
    fprintf(stderr,"-t,                  selects text output mode\n");
    fprintf(stderr,"-to <name>,          create a text file\n");
    fprintf(stderr,"-f <name>,           selects file containing list of\n");
    fprintf(stderr,"                     multitone frequencies (must be provided)\n");

}

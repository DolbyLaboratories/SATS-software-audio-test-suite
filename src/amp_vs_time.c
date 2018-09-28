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
;	File:	amp_vs_time.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "fio.h"
#include "parse_args.h"
#include "amplitude_vs_time.h"
#include "debug.h"
#include "version.h"
#include "Utilities.h"

extern void print_usage( void );
char *chart = "title, Amplitude vs. Time,\nxlabel, Time (s),\nylabel, Amplitude (normalized values)\n";

fstruct fst;

int main( int argc, char *argv[] )
{
    int res;
    int startCh, endCh;
    int ch;
    
    res = parse_args("amp_vs_time", argc, argv, &fst);
    if (res)
        exit(1);

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

    fio_init(&fst, "amp_vs_time", "Time", "Amplitude", NULL);

    for (ch = startCh; ch < endCh; ch++)
    {
        check( fio_next_chunk(&fst, ch) );

        fio_resetpos(&fst);
        if (fst.xminSet)
            fio_setpos(&fst, (long) ((double) fst.xmin * fst.fs));
        res=amplitude_vs_time(&fst);
		if (res)
		{
			return (1);
		}
    }
    fio_cleanup(&fst);
    debugCleanup();
    return (0);
}

void print_usage( void )
{
    fprintf(stderr, "Amplitude vs Time Tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr, "Usage: amp_vs_time [OPTION]... -i WAVEFILE\n\n");
    fprintf(stderr, "-c <chan>,          selects channel in multichannel file\n");
    fprintf(stderr, "                    0 = first channel\n");
    fprintf( stderr, "                   -c a for all channels\n" );
    fprintf(stderr, "-t,                 send text output to standard output (default)\n");
    fprintf(stderr, "-to <name>,         create a text file\n");
    fprintf(stderr, "-xmin <lim>,        selects minimum x-axis limit\n");
    fprintf(stderr, "-xmax <lim>,        selects maximum x-axis limit\n");
}

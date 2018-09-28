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
;	File:	dyn_rng.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "fio.h"
#include "parse_args.h"
#include "power_vs_time.h"
#include "debug.h"
#include "Utilities.h"
#include "version.h"

extern void print_usage( void );

char *chart = "title, Dynamic Range,\nxlabel, Time (s),\nylabel, Dynamic Range (dB)\n";

fstruct fst;

int main( int argc, char *argv[] )
{
    int res;
    int startCh, endCh;
    int ch;

    setlocale(LC_NUMERIC,"C");

    res = parse_args("dyn_range", argc, argv, &fst);
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

    fio_init(&fst, "dyn_rng", "Time", "Dynamic range", NULL);

    /*loop over the channels to calculate the values and put into a text file*/
    for (ch = startCh; ch < endCh; ch++)
    {
        check( fio_next_chunk(&fst, ch) );

        fio_resetpos(&fst);

        res=power_vs_time(&fst, 1); /* dynamic range calculation is done by power_vs_time */
        if (res)
		{
			return (1);
		}
    }
    fio_cleanup(&fst);
    debugCleanup();

    return (0);
}

/*usage and help function for the tool*/
void print_usage( void )
{
    fprintf(stderr, "Dynamic Range Tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr, "Usage: dyn_rng [OPTION]... -i WAVEFILE\n\n");
    fprintf(stderr, "-c <chan>,          selects channel in multichannel file\n");
    fprintf(stderr, "                    0 = first channel\n");
    fprintf(stderr, "                    -c a for all channels\n");
    fprintf(stderr, "-s,                 turn off stripping lead silence\n" );
	fprintf(stderr, "-thr_db <thr>,      select the power threshold for silence stripping in db\n");
	fprintf(stderr, "-thr_s <thr>,       select the sample threshold for silence stripping in bit\n");
    fprintf(stderr, "-t,                 send text output to standard output (default)\n");
    fprintf(stderr, "-to <name>,         create a text file\n");
}

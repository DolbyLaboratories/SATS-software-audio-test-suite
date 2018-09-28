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
;	File:	thd_vs_freq.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fio.h"
#include "parse_args.h"
#include "thd_freq.h"
#include "debug.h"
#include "version.h"
#include "Utilities.h"

extern void print_usage( void );

char *chart = "title, THD+N vs. Frequency,\nxlabel, Frequency (Hz),\nylabel, THD+N (dB)\nxlog,\n";
fstruct fst; 

int main( int argc, char *argv[] )
{
    int res;
    int startCh, endCh;
    int ch;
    int nrows = 1;
    int bad_dwells_nrows = 1;
    double points[DWELLS_MAX][THDFREQ_POINTS] = { {0.0} };
    double bad_dwells[DWELLS_MAX] = {0.0};
    int i = 0;

    res = parse_args("thd_vs_freq", argc, argv, &fst);

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

    /*initialize the debug and fio functions for the reading of the file*/
    debugInit();
    fio_init(&fst, "thd_vs_freq", "Frequency", "THD+N", NULL);

    for (ch = startCh; ch < endCh; ch++)
    {
        check( fio_next_chunk(&fst, ch) );

        fio_resetpos(&fst);

        for (i = 0; i < nrows; i++)
        {
            points[i][0] = 0.0;
            points[i][1] = 0.0;
        }

        nrows = 0;

        bad_dwells_nrows = 0;

        res=thd_freq(&fst, THD_VS_FREQ, points, &nrows, bad_dwells, &bad_dwells_nrows);
		if (res)
		{
			return (1);
		}

        if (nrows == 0)
        {
            //error("Bad Input File\n");
            error("Could not find any frequency dwell\n");
        }

    } /* end of for loop to calculate the values for all the channels */

    /* call the cleanup functions */
    fio_cleanup(&fst);
    debugCleanup();

    return (0);
}

void print_usage( void )
{
    fprintf(stderr, "THD+N vs Frequency Tool\n");
    fprintf(stderr, "%s\n", COPYRIGHT_STRING);
    fprintf(stderr, "%s\n", VERSION_STRING);
    fprintf(stderr, "Usage: thd_vs_freq [OPTION]... -i WAVEFILE\n\n");
    fprintf(stderr, "-c <chan>,          selects channel in multichannel file\n");
    fprintf(stderr, "                    0 = first channel\n");
    fprintf(stderr, "                    -c a for all channels\n");
    fprintf(stderr, "-s,                 turn off stripping lead silence\n" );
	fprintf(stderr, "-thr_db <thr>,      select the power threshold for silence stripping in db\n");
	fprintf(stderr, "-thr_s <thr>,       select the sample threshold for silence stripping in bit\n");
    fprintf(stderr, "-t,                 send text output to standard output (default)\n");
    fprintf(stderr, "-to <name>,         create a text file\n");
    fprintf(stderr, "-powermin <lim>,    select dB level to which very low power values will be clipped\n");
    fprintf(stderr, "                    (default: lowest representable dB level by the given bitdepth)\n");
}

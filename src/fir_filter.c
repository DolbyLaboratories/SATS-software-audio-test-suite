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
;	File:	fir_filter.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fir_filter.h"
#include "debug.h"

extern double fir_filter( double , pFIR_FILTER  );

int init_fir_filter( pFIR_FILTER pf )
{

    assert(pf->input == NULL);

    pf->input = (double *) calloc(sizeof(double), pf->bl);
    if (pf->input == NULL)
    {
        error("init_fir_filter: couldn't malloc input\n");
        return(-1);
    }

    reset_fir_filter(pf);
    pf->new = &pf->input[pf->bl - 1];

    return(0);
}

void reset_fir_filter( pFIR_FILTER pf )
{
    int i;

    assert(pf->input != NULL);

    for (i = 0; i < pf->bl; i++)
    {
        pf->input[i] = 0.0;
    }
}

double fir_filter( double x, pFIR_FILTER pf )
{
    int i, j;
    const double *pc;
    double *pi;
    double sum = 0.0;

    assert(pf->input != NULL);

    pc = pf->b;
    pi = pf->new;
    *pf->new = x;

    j = (int) (pf->new - pf->input);

    for (i = j; i < pf->bl; i++)
    {
        sum = sum + (*pi * *pc);
        pi++;
        pc++;
    }

    pi = pf->input;
    for (i = 0; i < j; i++)
    {
        sum = sum + (*pi * *pc);
        pi++;
        pc++;
    }

    pf->new--;
    if (pf->new < pf->input)
    {
        pf->new = &pf->input[pf->bl - 1];
    }

    return (sum);
}

void fir_filter_array( double *in, pFIR_FILTER pf, double *out, unsigned long l )
{
    unsigned long i;

    for (i = 0; i < l; i++)
    {
        *out = fir_filter(*in++, pf);
#ifdef DEBUG
        debugSample( *out );
#endif
        out++;
    }
}

void cleanup_fir_filter( pFIR_FILTER pf )
{
    assert(pf->input != NULL);

    free(pf->input);
    pf->input = NULL;
}

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
;	File:	sos_filter.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sos_filter.h"
#include "debug.h"

extern double biquad( double , const double [], const double [] , double * , double * );
extern double sos_filter( double , pSOS_FILTER );


int init_sos_filter( pSOS_FILTER pf )
{
    assert(pf->z1 == NULL);
    assert(pf->z2 == NULL);


    pf->z1 = (double *) calloc(pf->nsec, sizeof(double));
    if (pf->z1 == NULL)
    {
        error("init_sos_filter: couldn't malloc z1 delay\n");
        return (-1);
    }

    pf->z2 = (double *) calloc(pf->nsec, sizeof(double));
    if (pf->z2 == NULL)
    {
        free(pf->z1);
        pf->z1 = NULL;
        error("init_sos_filter: couldn't malloc z2 delay\n");
        return (-1);
    }

    reset_sos_filter(pf);

    return (0); // OK
}

void reset_sos_filter( pSOS_FILTER pf )
{
    int i;

    assert(pf->z1 != NULL);
    assert(pf->z2 != NULL);

    for (i = 0; i < pf->nsec; i++)
    {
        pf->z1[i] = 0.0;
        pf->z2[i] = 0.0;
    }
    return;
}

int free_sos_filter( pSOS_FILTER pf )
{

    assert(pf->z1 != NULL);
    assert(pf->z2 != NULL);

    free(pf->z1);
    free(pf->z2);
    pf->z1 = NULL;
    pf->z2 = NULL;

    return (0);
}

//
//	Direct-Form II, SOS
//
double biquad( double x, const double b[], const double a[], double *pz1, double *pz2 )
{
    double z;
    double d;

    d = x - (*pz1 * a[1]) - (*pz2 * a[2]);

    //	d = d / a[0];	// not needed, a[0] always 1.0

    z = d;

    d = d * b[0];

    d = d + (*pz1 * b[1]) + (*pz2 * b[2]);

    *pz2 = *pz1;
    *pz1 = z;

    return (d);
}

double sos_filter( double x, pSOS_FILTER pf )
{
    int i;

    assert(pf->z1 != NULL);
    assert(pf->z2 != NULL);

    for (i = 0; i < pf->nsec; i++)
    {
        if (pf->bl[i] == 1)
        {
            x = x * pf->b[i][0];
        }
        else if (pf->bl[i] == 3)
        {
            x = biquad(x, pf->b[i], pf->a[i], &pf->z1[i], &pf->z2[i]);
        }
    }
    return (x);
}

void sos_filter_array( double *in, pSOS_FILTER pfilt, double *out, unsigned long l )
{
    unsigned long i;

    for (i = 0; i < l; i++)
    {
        *out = sos_filter(*in++, pfilt);
#ifdef DEBUG
        debugSample( *out );
#endif
        out++;
    }
}

void write_coef( pSOS_FILTER pf, double a0, double a1, double a2, double b0, double b1, double b2 )
{
    //	int i;

    //debug("\n\n");


    pf->a[0][0] = a0;
    pf->a[0][1] = a1;
    pf->a[0][2] = a2;
    pf->b[0][0] = b0;
    pf->b[0][1] = b1;
    pf->b[0][2] = b2;

    /*
     for (i = 0; i < pf->nsec; i++ )
     {
     debug("i:       %i\n" ,  i);
     debug("a[][0]:  %.4f\n" ,  pf->a[i][0]);
     debug("a[][1]:  %.4f\n" ,  pf->a[i][1]);
     debug("a[][2]:  %.4f\n" ,  pf->a[i][2]);
     debug("\n");
     debug("b[][0]:  %.4f\n" ,  pf->b[i][0]);
     debug("b[][1]:  %.4f\n" ,  pf->b[i][1]);
     debug("b[][2]:  %.4f\n" ,  pf->b[i][2]);
     debug("\n");
     debug("al[][0]: %i\n" ,  (int)pf->al[0]);
     debug("bl[][0]: %i\n" ,  (int)pf->bl[0]);
     }

     debug("\n\n");
     */
}

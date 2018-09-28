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
;	File:	window.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "window.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//
//	These routines implement window functions (usually used before an FFT).
//	They are named the same as the MatLab routines and have been tested to match them.
//

void blackmanharris( double *pd, int n )
{
    double a0 = 0.35875;
    double a1 = 0.48829;
    double a2 = 0.14128;
    double a3 = 0.01168;
    double d;
    int i;

    if (n <= 0)
        return;

    for (i = 0; i < n; i++)
    {
        d = a0 - (a1 * cos((2.0 * M_PI) * ((double) i / (n - 1))));
        d = d + (a2 * cos((4.0 * M_PI) * ((double) i / (n - 1))));
        d = d - (a3 * cos((6.0 * M_PI) * ((double) i / (n - 1))));
        //		printf( "%0.14f\n", d );
        *pd++ = d;
    }
}

void hannwin( double *pd, int n )
{
	int i;
	double a0 = 0.5;
	double a1 = 0.5;

	if (n <= 0)
		return;

	for (i = 0; i < n; i++)
	{
		*pd++ = a0 - a1 * cos((2.0 * M_PI) * ((double)i/(double)(n-1)));
	}
}

void barthannwin( double *pd, int n )
{
    double d;
    int i;

    if (n <= 0)
        return;

    for (i = 0; i < n; i++)
    {
        d = 0.62 - (0.48 * fabs(((double) i / (double) (n - 1)) - 0.5));
        d = d + (0.38 * cos((2.0 * M_PI) * (((double) i / (double) (n - 1)) - 0.5)));
        //		printf( "%0.14f\n", d );
        *pd++ = d;
    }
}

void triang( double *pd, int n )
{
    int i;

    if (n <= 0)
        return;

    if (n % 2 == 1) // n is odd
    {
        for (i = 0; i < n; i++)
        {
            if (i < (double) n / 2)
                *pd = 2.0 * ((double) i + 1) / (n + 1);
            else *pd = 2.0 * (n - (double) i) / (n + 1);
            pd++;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (i < (double) n / 2)
                *pd = ((2.0 * (double) i) + 1) / n;
            else *pd = ((2.0 * (n - (double) i)) - 1) / n;
            pd++;
        }
    }
}

void bartlett( double *pd, int n )
{
    int i;

    if (n <= 0)
        return;
    if (n == 1)
    {
        *pd = 1.0;
        return;
    }

    if (n % 2 == 1) // n is odd
    {
        for (i = 0; i < n; i++)
        {
            if (i <= (n - 1) / 2)
                *pd = 2.0 * (double) i / (n - 1);
            else *pd = 2.0 - ((2.0 * (double) i) / (n - 1));
            pd++;
        }
    }
    else
    {
        for (i = 0; i < n; i++)
        {
            if (i <= ((n / 2) - 1))
                *pd = 2.0 * (double) i / (n - 1);
            else *pd = (2.0 * ((n - i) - 1)) / (double) (n - 1);
            pd++;
        }
    }
}

void rectwin( double *pd, int n )
{
    int i;

    for (i = 0; i < n; i++)
        *pd++ = 1.0;
}

//
//	compute_window_comp
//
//	compute a gain to apply to compensate for the windowing
//
//	this is suggested by the MatLab help page for "periodogram", see the algorithm section
//
//	pd is a pointer to an array to store the window values
//	n is the size of the window

double compute_window_comp( double *pd, int n )
{
    int i;
    double sum = 0.0;

    for (i = 0; i < n; i++)
    {
        sum = sum + (*pd * *pd);
        pd++;
    }
    sum = sum / (double) n;

    return (sum);
}

//
//	window_array
//
//	apply a window to an array of samples
//
//	pi is a pointer to an array of samples
//	pw is a pointer to the window array
//	po is a point to the output array
//	n is the size of all three arrays
//

void window_array( double *pi, double *pw, double *po, int n )
{
    double d;
    int i;

    for (i = 0; i < n; i++)
    {
        d = *pi++;
        d = d * *pw++;
        *po++ = d;
    }
}

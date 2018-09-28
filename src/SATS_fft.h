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
;	File:	SATS_fft.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#ifndef SATS_FFT_H_
#define SATS_FFT_H_


#if defined(_MSC_VER) 
#  if !defined(inline)
#    define inline __inline
#  endif
#  define snprintf _snprintf
#  define lrint(x) (((x) >= 0.0) ? floor((x)+0.5) : ceil((x)-0.5))
    /* macro written here is original, but reflects algorithm
     * found here https://www.johndcook.com/blog/cpp_expm1/
     * under BSD license: https://www.johndcook.com/bsd.html */
#  define expm1(x) ((fabs(x) < 1e-5) ? ((x) + 0.5*(x)*(x)) : (exp(x) - 1.0))
#endif

#ifdef KISS_FFT

/* ------------------- KISS FFT Implementation ------------------------------ */
#include "tools/kiss_fftr.h"

typedef kiss_fftr_cfg SATS_FFT_HANDLE;
typedef kiss_fft_cpx SATS_FFT_Complex;
typedef int SATS_FFT_Status;

#define SATS_FFT_REAL(c) (c).r
#define SATS_FFT_IMAG(c) (c).i

static inline
SATS_FFT_Status
SATS_FFT_Create
    (SATS_FFT_HANDLE *h
    ,int fft_size
    )
{
    *h = kiss_fftr_alloc(fft_size, 0, NULL, NULL);
    return *h != 0;
}


static inline
SATS_FFT_Status
SATS_FFT_ComputeForward
    (SATS_FFT_HANDLE h
    ,double *in
    ,SATS_FFT_Complex *out
    )
{
    kiss_fftr(h, in, out);
    return 1;
}


static inline
SATS_FFT_Status
SATS_FFT_Destroy
    (SATS_FFT_HANDLE *h
    )
{
    kiss_fftr_free(*h);
    return 1;
}


#else
/* ------------------- Intel MKL FFT Implementation ------------------------------ */

//FFT interface of Intel MKL.
#include "mkl_dfti.h" 
//Memory managing interface of Intel MKL.
#include "mkl_service.h"

typedef DFTI_DESCRIPTOR_HANDLE SATS_FFT_HANDLE;
typedef MKL_Complex16 SATS_FFT_Complex;
typedef MKL_LONG SATS_FFT_Status;

#define SATS_FFT_REAL(c) (c).real
#define SATS_FFT_IMAG(c) (c).imag


static inline
SATS_FFT_Status
SATS_FFT_Create
    (SATS_FFT_HANDLE *h_in
    ,int fft_size
    )
{
    SATS_FFT_HANDLE h;
//	Configuring FFT:
//		* Double precision
//		* Real input
//		* One-​dimensional
//		* Size=​FFTSIZE

    (void)DftiCreateDescriptor(&h, DFTI_DOUBLE, DFTI_REAL, 1, fft_size);
    (void)DftiSetValue(h, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
    (void)DftiSetValue(h, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);
    *h_in = h;
    return DftiCommitDescriptor(h);
}


static inline
SATS_FFT_Status
SATS_FFT_ComputeForward
    (SATS_FFT_HANDLE h
    ,double *in
    ,SATS_FFT_Complex *out
    )
{
    return DftiComputeForward(h, in, out);
}


static inline
SATS_FFT_Status
SATS_FFT_Destroy
    (SATS_FFT_HANDLE *h
    )
{
    return DftiFreeDescriptor(h);
}


#endif

/** --------------------------- common defs ------------------------- */

static inline
void*
SATS_FFT_malloc
    (size_t sz
    ,size_t alignment
    )
{
    uintptr_t *startptr;
    uintptr_t addr;
    uintptr_t addr2;

    if (alignment < sizeof(void*))
    {
        alignment = sizeof(void*);
    }

    addr = (uintptr_t)malloc(sz + alignment + sizeof(uintptr_t));
    addr2 = ((addr + alignment - 1) / alignment) * alignment;
    if (addr)
    {
        if ((addr2 - addr) < sizeof(void*))
        {
            addr2 += alignment;
        }
        startptr = (uintptr_t*)(addr2 - sizeof(uintptr_t));
        *startptr = addr;
    }
    return (void*)addr2;
}


static inline
void
SATS_FFT_free
    (void *mem
    )
{
    /* walk backwards one pointer's worth to find true start of
     * allocated region */
    uintptr_t addr = (uintptr_t)mem;
    addr -= sizeof(void*);
    mem = *(void**)addr;
    free(mem);
}


static inline
SATS_FFT_Status
SATS_FFT_ComputeForward2
    (double *in
    ,SATS_FFT_Complex *out
    ,int fft_size
    )
{
    SATS_FFT_HANDLE h;

    (void)SATS_FFT_Create(&h, fft_size);
    (void)SATS_FFT_ComputeForward(h, in, out);
    return SATS_FFT_Destroy(&h);
}


#endif /* SATS_FFT_H_ */


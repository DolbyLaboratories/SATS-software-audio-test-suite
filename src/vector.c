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
;	File:	vector.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include "vector.h"

struct vector_t_
{
    void **elements;
    size_t alloc;
    size_t size;
    size_t current;
};

vector_t
vector_new(void)
{
    vector_t l = malloc(sizeof *l);

    l->alloc = 1;
    l->size = 0;

    l->elements = malloc(l->alloc * sizeof(void *));

    return l;
}

vector_t
vector_copy(const vector_t v, vector_element_copy_t element_copy)
{
    vector_t v2 = malloc(sizeof *v2);
    size_t i;

    v2->alloc   = v->alloc;
    v2->size    = v->size;
    v2->current = v->current;
    v2->elements = malloc(v2->alloc * sizeof(void *));

    for (i = 0; i < v2->size; i++)
    {
        if (element_copy != NULL)
        {
            v2->elements[i] = element_copy(v->elements[i]);
        }
        else
        {
            v2->elements[i] = v->elements[i];
        }
    }

    return v2;
}

size_t
vector_size(const vector_t l)
{
    return l->size;
}

void
vector_add(vector_t l, const void *e)
{
    if (l->size == l->alloc)
    {
        l->alloc *= 2;
        l->elements = realloc(l->elements, l->alloc * sizeof(*(l->elements)));
    }

    l->elements[l->size] = (void *)e;   /* elements are never changed */
    l->size += 1;
}

void *
vector_next(vector_t l)
{
    if (l->current == l->size) 
    {
        return NULL;
    }
    return l->elements[l->current++];
}

void *
vector_first(vector_t l)
{
    l->current = 0;
    return vector_next(l);
}

void
vector_delete(vector_t l, void (*element_delete)(void *))
{
    if (l == NULL)
    {
        return;
    }

    if (element_delete != NULL)
    {
        size_t i;
        for (i = 0; i < l->size; i++)
        {
            element_delete(l->elements[i]);
        }
    }
    free(l->elements);
    free(l);
}

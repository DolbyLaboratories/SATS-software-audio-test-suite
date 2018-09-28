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
;	File:	sdf_writer.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

/* SATS Data Format Writer 

   How to use this API: 

   - Create an sdf_config (file header)
       - define keywords
       - define axes
   - Create an sdf_writer, using the config
       - define the data sinks (e.g. stdout and/or a local file)
   - For each chunk to write:
       - Create an sdf_chunk_config (chunk header)
       - Pass it to sdf_writer_next_chunk()
       - Dump chunk data ad lib
   - Do call sdf_writer_delete(). Apart from freeing memory,
     the the size of the last chunk is written

   See sdf_test.c for example code.

*/
#ifndef SDF_WRITER_H
#define SDF_WRITER_H

#include <stddef.h>

#ifdef _MSC_VER
#define PRId64 "I64d"
#define PRIu64 "I64u"
#define PRIu32 "I32u"
#define PRIu16 "hu"
#define PRIu8 "u"
#define PRIz "Iu"
#else
#include <inttypes.h>
#define PRIz "zu"
#endif

#ifdef _MSC_VER
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#endif

typedef struct sdf_config_t_ * sdf_config_t;
typedef struct sdf_writer_t_ * sdf_writer_t;
typedef struct sdf_chunk_config_t_ *sdf_chunk_config_t;
typedef struct axis_t_ *axis_t;

typedef enum { SDF_STRING, SDF_UINT32, SDF_UINT64, SDF_FLOAT32 } value_type_t;

typedef int error_t;

/*
 * file config 
 */
error_t
sdf_config_new(sdf_config_t *);

error_t
sdf_config_delete(sdf_config_t *);

error_t
sdf_config_key_string(sdf_config_t, 
                      const char *name,
                      const char *value,
                      const char *comment);

error_t
sdf_config_key_uint32(sdf_config_t, 
                      const char *name,
                      uint32_t value,
                      const char *comment);

error_t
sdf_config_key_uint64(sdf_config_t, 
                      const char *name,
                      uint64_t value,
                      const char *comment);

error_t
sdf_config_axis(sdf_config_t c,
                const char *name,
                value_type_t type,
                const char *unit,
                const char *description,
                axis_t *axis  /* out, owned by the sdf_config object */);


/*
 * sdf_writer
 */
error_t
sdf_writer_new(const sdf_config_t c, /* Defines axes and keys binary output */
               int format,           /* zero: write data as text, non-zero: write data as binary */
               sdf_writer_t *w       /* out */
    );

/*
 *  Define data sink.
 *  Maximum one sink is supported.
 */
error_t
sdf_writer_add_sink(sdf_writer_t w,
                    const char *path   /* path to output file, or "-": standard output */
    );

error_t
sdf_writer_delete(sdf_writer_t *);

error_t
sdf_writer_next_chunk(sdf_writer_t, sdf_chunk_config_t);

/* Generic data writing. Not implemented.
error_t
sdf_writer_add_data(sdf_writer_t, void *, size_t);
*/

/* Write data, special cases */
error_t
sdf_writer_add_data_float_float(sdf_writer_t, 
                                float, 
                                float,
                                const char *format  /* printf format string. Only used for text output */
    );

/* This function does not write double values as SDF data! Unlike sdf_writer_add_data_float_float, 
   it accepts double values and does a typecast to float internally before writing. */
error_t
sdf_writer_add_data_double_double(sdf_writer_t, 
                                double, 
                                double,
                                const char *format  /* printf format string. Only used for text output */
    );

error_t
sdf_writer_add_data_uint32(sdf_writer_t, uint32_t);


/*
 * chunk_config
 * Defines which axes are constant/variable for a chunk, including the order of variable axes.
 */

error_t
sdf_chunk_config_new(sdf_chunk_config_t *);

error_t
sdf_chunk_config_delete(sdf_chunk_config_t *);

/* Declare a variable axis for this chunk.
   The axis_t object is returned from sdf_config_axis(). */
error_t
sdf_chunk_config_variable(sdf_chunk_config_t, axis_t);

/* Declare a constant axis for this chunk, including its value */
error_t
sdf_chunk_config_constant_uint32(sdf_chunk_config_t, axis_t, uint32_t);

error_t
sdf_chunk_config_constant_float32(sdf_chunk_config_t, axis_t, float);

error_t
sdf_chunk_config_constant_string(sdf_chunk_config_t, axis_t, const char *);

#endif

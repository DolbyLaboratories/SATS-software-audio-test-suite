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
;	File:	sdf_writer.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include "sdf_writer.h"

#include "vector.h"
#include "Utilities.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
	#include "stdio64.h"
#endif
#include <stdarg.h>

#ifdef _MSC_VER   /* _setmode */
#include <io.h>
#include <fcntl.h>
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!(FALSE))
#endif

/*
 * Internal types 
 */
typedef struct header_key_t_ *header_key_t;
typedef struct constant_axis_t_ *constant_axis_t;
typedef struct file_t_ file_t;

/* Abstraction of FILE*, to allow writing (directly) to a file or though a buffer to a file.
   This is used to hold the current chunk in memory when writing to stdout. stdout is not seekable,
   but the current chunk size has to be written to the chunk header and is only known at the
   end of the chunk.
*/
struct file_t_
{
    size_t (*write)(const void *ptr, size_t size, size_t nmemb, file_t *);
    int (*printf)(file_t *, const char *format, ...);
    int64_t (*tell)(file_t *); /* equivalent to ftell, or  NULL if not implemented */
    int (*seek)(file_t *, uint64_t offset);   /* offset relative to beginning of file (i.e. SEEK_SET is implied) */
    int (*flush)(file_t *);  /* non-NULL only for buffered output */
    int (*close)(file_t *);
};

/* Value types */
typedef struct value_t_
{
    value_type_t type;
    union
    {
        const char * string;
        uint32_t uint32;
        uint64_t uint64;
        float float32;
    } value;
} value_t;

/* SDF configuration */
struct sdf_config_t_
{
    vector_t keys;
    vector_t axes;
};

/* SDF writer */
struct sdf_writer_t_
{
    const char *path;             /* For messaging */
    file_t *sink;                 /* out file, or temp buffer */
    int binary_format;            /* boolean */
    sdf_config_t config;          /* reference, not owned */
    uint64_t current_chunk_size_pos;  /* Pointer to chunk header field */
    uint64_t current_chunk_size;
    sdf_chunk_config_t cc;          /* Current chunk config (owned) */

    uint32_t dimension;
};

/* SDF chunk configuration */
struct sdf_chunk_config_t_
{
    vector_t variables;  /* of axis_t, reference only */
    vector_t constants;  /* of consant_axis, owned */
};

struct constant_axis_t_
{
    axis_t axis;        /* reference, not owned */
    value_t value;      /* owned */
};


/* Header key */
struct header_key_t_
{
    const char * name;
    const char * comment;
    value_t value;
};

/* Axis */
struct axis_t_
{
    const char *name;
    const char *unit;
    value_type_t type;
    const char *description;
    uint32_t id;   /* Order, as defined in file header, counting from zero */
};

const size_t CHUNK_SIZE = 33554432  ;  /* When writing to stdout, 
                                       when memory buffer exceeds this size [33554432], flush it */

/* Returns true iff little endian */
static int
is_little_endian(void)
{
    int i = 1;
    return *((char *)&i) == 1;
}

static const char *
string_dup(const char *s)
{
    char *c = malloc(sizeof(*c) * (strlen(s) + 1));
    memcpy(c, s, strlen(s) + 1);

    return c;
}

static void
string_delete(const char *s)
{
    free((char *)s);
}

/* copy constructor */
static value_t
value_copy(value_t v)
{
    value_t w;
    w.type = v.type;

    switch(v.type)
    {
    case SDF_STRING: w.value.string = string_dup(v.value.string);     break;
    case SDF_UINT32: w.value.uint32 = v.value.uint32;                 break;
    case SDF_UINT64: w.value.uint64 = v.value.uint64;                 break;
    case SDF_FLOAT32: w.value.float32 = v.value.float32;              break;
    default:
        assert(FALSE);
    }
    
    return w;
}

static void
value_delete(value_t v)
{
    if (v.type == SDF_STRING)
    {
        string_delete(v.value.string);
    }
}

static void
header_key_delete(header_key_t h)
{
    string_delete(h->name);
    string_delete(h->comment);
    value_delete(h->value);
    free(h);
}

static void
axis_delete(axis_t a)
{
    string_delete(a->name);
    string_delete(a->unit);
    string_delete(a->description);
    free(a);
}

/* copy constructor */
static constant_axis_t
constant_axis_copy(constant_axis_t c)
{
    constant_axis_t d = malloc(sizeof *d);

    assure( d != NULL, ("Allocation failure") );

    d->axis = c->axis;
    d->value = value_copy(c->value);

    return d;
}


static void
constant_axis_delete(constant_axis_t c)
{
    value_delete(c->value);
    free(c);
}

typedef struct
{
    file_t base;
    FILE *f;
} FILE_t;

static size_t
file_write(const void *ptr, size_t size, size_t nmemb, file_t *f)
{
    return fwrite(ptr, size, nmemb, ((FILE_t *)f)->f);
}

static int
file_printf(file_t *f, const char *format, ...)
{
    FILE *stream = ((FILE_t *)f)->f;
    va_list vl;
    int n;

    va_start(vl, format);
    n = vfprintf(stream, format, vl);
    va_end(vl);

    return n;
}

/* same as ftell but supports 64-bit offset */
static int64_t
file_tell(file_t *f)
{
#ifdef _MSC_VER
    return _ftelli64(((FILE_t *)f)->f);
#else  /* posix */
    return ftello(((FILE_t *)f)->f);
#endif
}

/* fseek, but supports 64-bit offsets */
static int
file_seek(file_t *f, 
          uint64_t offset   /* offset relative to beginning of file (i.e. SEEK_SET is implied) */
    )
{
    FILE *stream = ((FILE_t *)f)->f;
    uint32_t m = (uint32_t) -1;  /* Maximum that fseek supports */
    int first = 1;  /* First SEEK_SET, then SEEK_CUR */

    while (offset > 0 || first)
    {
        uint32_t off;

        if (offset > m)
        {
            off = m;
        }
        else
        {
            off = (uint32_t) offset;
        }
        
        {
            int err = fseek(stream, off, first ? SEEK_SET : SEEK_CUR);
            if (err != 0)
            {
                return err;
            }
        }
        offset -= off;
        first = 0;
    }
    return 0;
}

    

static int
file_close(file_t *f)
{
    int err = fclose(((FILE_t *)f)->f);
    free(f);
    return err;
}

static file_t *
file_new_FILE(const char *path)
{
    FILE_t *f = malloc(sizeof *f);

    f->base.write = file_write;
    f->base.printf = file_printf;
    f->base.tell = file_tell;
    f->base.seek = file_seek;
    f->base.flush = NULL;
    f->base.close = file_close;

    f->f = fopen(path, "wb");
    assure(f->f != NULL, ("Failed to open %s", path));

    return &f->base;
}

typedef struct
{
    file_t base;
    char *buffer;
    size_t size;   /* allocated buffer */
    size_t pos;    /* bytes used */
    FILE *out;     /* real output (non-seekable) */
    const char *path; /* for messaging */
    uint64_t offset; /* offset (of buffer start) since beginning of stream */
} buffer_t;

static size_t
buffer_write(const void *ptr, size_t size, size_t nmemb, file_t *f)
{
    buffer_t *b = (buffer_t *)f;

    if (b->pos + size * nmemb > b->size)
    {
        b->size *= 2;
        b->buffer = realloc(b->buffer, b->size);

        assure( b->buffer != NULL, ("Allocation failure") );
    }

    memcpy(b->buffer + b->pos, ptr, size * nmemb);
    b->pos += size * nmemb;

    return nmemb;
}

static int
buffer_printf(file_t *f,
              const char *format, ...)
{
    buffer_t *b = (buffer_t *)f;
    va_list vl;
    int n;
    size_t space;

    do
    {
        space = b->size - b->pos;
        
        va_start(vl, format);
        n = vsnprintf(b->buffer + b->pos, space, format, vl);
        va_end(vl);

        if (n >= (int) space)
        {
            b->size *= 2;
            b->buffer = realloc(b->buffer, b->size);

            assure( b->buffer != NULL, ("Allocation failure") );
        }
    } while (n >= (int) space);

    b->pos += n;

    return n;
}

/* offset: since beginning of stream */
static int64_t
buffer_tell(file_t *f)
{
    return ((buffer_t *)f)->offset + ((buffer_t *)f)->pos;
}

/* offset: since beginning of stream */
static int
buffer_seek(file_t * f, uint64_t offset)
{
    buffer_t *b = (buffer_t *) f;

    assure( offset < b->offset + b->size, 
            ("Cannot buffer_seek to an offset >= allocated size (%" PRIu64 " >= %" PRIu64"; buffer offset = %" PRIu64 ")",
             offset, b->offset + b->size, b->offset) );

    b->pos = (size_t) (offset - b->offset);

    return 0;
}

static int
buffer_flush(file_t *f)
{
    buffer_t *b = (buffer_t *) f;
	if (b->pos > 0)
    {
		int i = 0;
		int chunksize = 65535; /* we write at most this many bytes at a time, since writing [65536] or more bytes at a time doesn't work [in Windows] */
		for (i = 0; i < (int) b->pos; i += chunksize) 
		{
			int check = i + chunksize;
			if (check > (int) b->pos) 
			{
				chunksize = chunksize - (check - (int) b->pos);
			    assure( fwrite(b->buffer + i, chunksize, 1, b->out) == 1, ("Failed to flush %d bytes to %s", b->pos, b->path) );
		    }
			else
			{
			    assure( fwrite(b->buffer + i, chunksize, 1, b->out) == 1, ("Failed to flush %d bytes to %s", b->pos, b->path) );
			}
		}
	}
    b->offset += b->pos;
    b->pos = 0;

    return fflush(b->out);
}

static int
buffer_close(file_t *f)
{
    buffer_t *b = (buffer_t *)f;

    b->base.flush(&b->base);

    free(b->buffer);
    free(f);

    return 0;
}

static file_t *
file_new_buffer(size_t size, FILE *out, const char *path)
{
    buffer_t * b = malloc(sizeof *b);

    b->base.write = buffer_write;
    b->base.printf = buffer_printf;
    b->base.tell = buffer_tell;
    b->base.seek = buffer_seek;
    b->base.flush = buffer_flush;
    b->base.close = buffer_close;
    
    b->offset = 0;
    b->size = size;
    b->pos = 0;
    b->buffer = malloc(b->size);
    assure(b->buffer != NULL, ("Allocation error"));

    b->out = out;
    b->path = path;

    return &b->base;
}

/*
 *   chunk config
 */
error_t
sdf_chunk_config_new(sdf_chunk_config_t *c)
{
    assure(c != NULL, ("Null input"));
    
    *c = malloc(sizeof(**c));

    assure(*c != NULL, ("Allocation failure"));

    (*c)->variables = vector_new();
    (*c)->constants = vector_new();

    assure((*c)->variables != NULL, ("Allocation failure"));
    assure((*c)->constants != NULL, ("Allocation failure"));

    return 0;
}

/*
 *  copy constructor
 */
static sdf_chunk_config_t
sdf_chunk_config_copy(sdf_chunk_config_t c)
{
    sdf_chunk_config_t c2 = malloc(sizeof(*c2));

    assure( c != NULL, ("Null input") );
    
    c2->variables = vector_copy(c->variables, NULL);
    c2->constants = vector_copy(c->constants, (vector_element_copy_t) constant_axis_copy);

    return c2;    
}

error_t
sdf_chunk_config_delete(sdf_chunk_config_t *c)
{
    if (c != NULL)
    {
        vector_delete((*c)->variables, NULL);
        vector_delete((*c)->constants, (vector_element_delete_t) constant_axis_delete);
        free(*c);
    }

    return 0;
}

error_t
sdf_chunk_config_variable(sdf_chunk_config_t c, axis_t axis)
{
    assure( c != NULL, ("Null input") );

    vector_add(c->variables, axis);

    return 0;
}

error_t
sdf_chunk_config_constant_uint32(sdf_chunk_config_t cc, axis_t axis, uint32_t u)
{
    struct constant_axis_t_ *c = malloc(sizeof(*c));
    
    assure( axis->type == SDF_UINT32, ("Axis '%s' is not of type SDF_UINT32", axis->name));
    c->axis = axis;
    c->value.type = SDF_UINT32;
    c->value.value.uint32 = u;

    vector_add(cc->constants, c);

    return 0;
}

error_t
sdf_chunk_config_constant_float32(sdf_chunk_config_t cc, axis_t axis, float f)
{
    struct constant_axis_t_ *c = malloc(sizeof(*c));

    assure( axis->type == SDF_FLOAT32, ("Axis '%s' is not of type SDF_FLOAT32", axis->name));
    
    c->axis = axis;
    c->value.type = SDF_FLOAT32;
    c->value.value.float32 = f;

    vector_add(cc->constants, c);

    return 0;
}

error_t
sdf_chunk_config_constant_string(sdf_chunk_config_t cc, axis_t axis, const char *s)
{
    struct constant_axis_t_ *c = malloc(sizeof(*c));
    
    assure( axis->type == SDF_STRING, ("Axis '%s' is not of type SDF_STRING", axis->name));
    c->axis = axis;
    c->value.type = SDF_STRING;
    c->value.value.string = string_dup(s);

    vector_add(cc->constants, c);

    return 0;
}


/*
 *   file config
 */
error_t
sdf_config_new(sdf_config_t *c_p)
{
    sdf_config_t c;

    assure(c_p != NULL, ("Null input"));

    c = malloc(sizeof(*c));
    assure(c != NULL, ("Allocation failure"));

    c->keys = vector_new();
    c->axes = vector_new();

    *c_p = c;

    return 0;
}

error_t
sdf_config_delete(sdf_config_t *c_p)
{
    if (c_p != NULL && *c_p != NULL)
    {
        sdf_config_t c = *c_p;
        vector_delete(c->keys, (vector_element_delete_t) header_key_delete);
        vector_delete(c->axes, (vector_element_delete_t) axis_delete);
        free(c);
    }
    return 0;
}

error_t
sdf_config_key_string(sdf_config_t c,
                      const char *name,
                      const char *value,
                      const char *comment)
{
    header_key_t key = malloc(sizeof *key);

    assure(c != NULL, ("Null input"));
    assure(name != NULL, ("Null input"));
    assure(value != NULL, ("Null input"));
    assure(comment != NULL, ("Null input"));

    key->name = string_dup(name);
    key->comment = string_dup(comment);
    key->value.type = SDF_STRING;
    key->value.value.string = string_dup(value);

    vector_add(c->keys, key);

    return 0;
}

error_t
sdf_config_key_uint32(sdf_config_t c,
                      const char *name,
                      uint32_t value,
                      const char *comment)
{
    header_key_t key = malloc(sizeof *key);

    assure(c != NULL, ("Null input"));
    assure(name != NULL, ("Null input"));
    assure(comment != NULL, ("Null input"));

    key->name = string_dup(name);
    key->comment = string_dup(comment);
    key->value.type = SDF_UINT32;
    key->value.value.uint32 = value;

    vector_add(c->keys, key);

    return 0;
}

error_t
sdf_config_key_uint64(sdf_config_t c,
                      const char *name,
                      uint64_t value,
                      const char *comment)
{
    header_key_t key = malloc(sizeof *key);

    assure(c != NULL, ("Null input"));
    assure(name != NULL, ("Null input"));
    assure(comment != NULL, ("Null input"));

    key->name = string_dup(name);
    key->comment = string_dup(comment);
    key->value.type = SDF_UINT64;
    key->value.value.uint64 = value;

    vector_add(c->keys, key);

    return 0;
}

error_t
sdf_config_axis(sdf_config_t c,
                const char *name,
                value_type_t type,
                const char *unit,
                const char *description,
                axis_t * axis)
{
    axis_t a = malloc(sizeof *a);

    assure(c != NULL, ("Null input"));
    assure(name != NULL, ("Null input"));
    assure(unit != NULL, ("Null input"));
    assure(description != NULL, ("Null input"));
    assure(axis != NULL, ("Null input"));

    a->name = string_dup(name);
    a->unit = string_dup(unit);
    a->type = type;
    a->description = string_dup(description);
    a->id = (uint32_t) vector_size(c->axes);

    vector_add(c->axes, a);

    *axis = a;

    return 0;
}



static error_t
write_string(sdf_writer_t w, const char *s)
{
    if (!w->binary_format) return 0;

    assure(s != NULL, ("Null input"));

    if (strlen(s) > 0)
    {
        assure( w->sink->write(s, strlen(s), 1, w->sink) == 1,
                ("Error writing %s to %s", s, w->path));
    }

    {
        char zero = '\0';

        assure( w->sink->write(&zero, 1, 1, w->sink) == 1,
                ("Error writing string terminator to %s", w->path));
    }

    return 0;
}

static error_t
write_uint32(sdf_writer_t w, uint32_t u)
{
    if (!w->binary_format) return 0;

    assure( w->sink->write(&u, sizeof(u), 1, w->sink) == 1,
            ("Error writing %u to %s", u, w->path));

    return 0;
}

static error_t
write_uint64(sdf_writer_t w, uint64_t u)
{
    if (!w->binary_format) return 0;

    assure( w->sink->write(&u, sizeof(u), 1, w->sink) == 1,
            ("Error writing %" PRIu64 " to %s", u, w->path));

    return 0;
}

static error_t
write_float32(sdf_writer_t w, double f)
{
    if (!w->binary_format) return 0;

    assure( w->sink->write(&f, sizeof(f), 1, w->sink) == 1,
            ("Error writing %f to %s", f, w->path));
    return 0;
}

static error_t
write_type(sdf_writer_t w, value_type_t t)
{
    if (!w->binary_format) return 0;

    switch(t)
    {
    case SDF_STRING:
        write_string(w, "STRING");
        break;
    case SDF_UINT32:
        write_string(w, "UINT32");
        break;
    case SDF_UINT64:
        write_string(w, "UINT64");
        break;
    case SDF_FLOAT32:
        write_string(w, "FLOAT32");
        break;
    default:
        assert(FALSE);
        break;
    }

    return 0;
}

static error_t
write_value(sdf_writer_t w, value_t v)
{
    if (!w->binary_format) return 0;

    switch(v.type)
    {
    case SDF_STRING:
        write_string(w, v.value.string);
        break;
    case SDF_UINT32:
        write_uint32(w, v.value.uint32);
        break;
    case SDF_UINT64:
        write_uint64(w, v.value.uint64);
        break;
    case SDF_FLOAT32:
        write_float32(w, v.value.float32);
        break;
    default:
        assert(FALSE);
    }

    return 0;
}


static error_t
write_signature(sdf_writer_t w)
{
    static const char *s = "SATS";
    uint32_t major = 1;
    uint32_t minor = 0;

    if (!w->binary_format) return 0;

    assure(w->sink->write(s, 4, 1, w->sink) == 1, ("Error writing to %s", w->path));

    assure(w->sink->write(&major, sizeof(major), 1, w->sink) == 1, ("Error writing to %s", w->path));
    assure(w->sink->write(&minor, sizeof(minor), 1, w->sink) == 1, ("Error writing to %s", w->path));

    return 0;
}

static error_t
write_keys(sdf_writer_t w, vector_t keys)
{
    header_key_t key;

    if (!w->binary_format) return 0;

    for (key = vector_first(keys); key != NULL; key = vector_next(keys))
    {
        write_string(w, key->name);
        write_type(w, key->value.type);
        write_value(w, key->value);
        write_string(w, key->comment);
    }

    write_string(w, "");

    return 0;
}

static error_t
write_axes(sdf_writer_t w, vector_t axes)
{
    axis_t axis;

    if (!w->binary_format) return 0;

    for (axis = vector_first(axes); axis != NULL; axis = vector_next(axes))
    {
        write_string(w, axis->name);
        write_type(w, axis->type);
        write_string(w, axis->unit);
        write_string(w, axis->description);
    }

    write_string(w, "");

    return 0;
}

error_t
sdf_writer_new(const sdf_config_t c, int binary_format, sdf_writer_t * w_p)
{
    sdf_writer_t w;

    assure(is_little_endian(), ("Sorry, code only works on a little endian system!"));
    /* Because the file format specifies little endian. On a big endian architecture
       bytes would need swapping (in e.g. write_uint64). Probably simple to implement
       but untested, thus explicitly unsupported.  */

    assure(w_p != NULL, ("Null pointer"));
    assure(c != NULL, ("Null pointer"));

    w = malloc(sizeof *w);

    assure(w != NULL, ("Allocation error"));
    
    *w_p = w;
    
    w->sink = NULL;
    w->config = c;
    w->binary_format = binary_format;

    w->dimension = (uint32_t) vector_size(c->axes);

    w->current_chunk_size_pos = 0;
    w->cc = NULL;

    return 0;
}

error_t
sdf_writer_add_sink(sdf_writer_t w,
                    const char *path)
{
    assure( w != NULL, ("Null pointer") );
    assure( w->sink == NULL, ("Sorry, only one sink supported for now") );
    assure(path != NULL, ("Null pointer"));

    if (strcmp(path, "-") == 0)
    {
        w->path = string_dup("<standard output>");
#ifdef _MSC_VER
        /* Reopen stdout as binary, in order to avoid conversion of line ending character */
        fflush(stdout);
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        w->sink = file_new_buffer(CHUNK_SIZE, stdout, w->path);
    }
    else
    {
        w->path = string_dup(path);
        w->sink = file_new_FILE(path);
        assure(w->sink != NULL, ("Failed to open %s", path));
    }
    
    check( write_signature(w) );
    check( write_keys(w, w->config->keys) );
    check( write_axes(w, w->config->axes) );

    return 0;
}


/* Write the previous chunk size,
   remember where to write the next chunk size */
static error_t
write_chunk_finalize(sdf_writer_t w)
{
    if (w->current_chunk_size_pos == 0)
    {
        /* First chunk, no previous chunk */
        return 0;
    }

    if (w->binary_format)
    {
        int64_t current_pos = w->sink->tell(w->sink);

        assure( current_pos >= 0, ("ftell on '%s' failed", w->path) );
        assure( w->sink->seek(w->sink, w->current_chunk_size_pos) == 0, ("fseek on '%s' failed", w->path) );
        write_uint64(w, w->current_chunk_size);
        assure( w->sink->seek(w->sink, current_pos) == 0, ("fseek on '%s' failed", w->path) );
    }
    else
    {
        char c = '\n';

        assure( w->sink->write(&c, 1, 1, w->sink) == 1, ("Error writing newline character to %s\n", w->path) );
    }

    /* flush a memory buffer */
    if (w->sink->flush != NULL)
    {
        assure( w->sink->flush(w->sink) == 0, ("failed to write chunk") );
    }

    return 0;
}



/* Writes the chunk header. The chunk data size can be written
   only when done writing, i.e. when sdf_writer_next_chunk or
   sdf_writer_delete is called again.
*/
error_t
sdf_writer_next_chunk(sdf_writer_t w, sdf_chunk_config_t cc)
{
    struct constant_axis_t_ *c;
    uint32_t number_of_axes = 0;

    assure( w != NULL, ("Null input"));

    if (w->cc != NULL && cc != w->cc)
    {
        sdf_chunk_config_delete(&w->cc);
    }
    w->cc = sdf_chunk_config_copy(cc);

    for (c = vector_first(cc->constants); c != NULL; c = vector_next(cc->constants))
    {
        write_uint32(w, c->axis->id);
        write_value(w, c->value);

        number_of_axes++;
    }

    /* Constant axes sentinel */
    write_uint32(w, (uint32_t) -1);

    {
        axis_t a;
        for (a = vector_first(cc->variables); a != NULL; a = vector_next(cc->variables))
        {
            assure( a->id < w->dimension,
                    ("Axis id is %d but number of axes is only %d\n",
                    a->id, w->dimension));
            write_uint32(w, a->id);
    
            number_of_axes++;
        }
    }

    /* Variables sentinel */
    write_uint32(w, (uint32_t) -1);

    assure( w->dimension == number_of_axes, 
            ("Wrong number of axes (%d) for chunk. File dimension is %d",
             number_of_axes, w->dimension) );

    /* Previous chunk size */
    write_chunk_finalize(w);

    if (w->binary_format)
    {
        int64_t pos = w->sink->tell(w->sink);

        assure( pos >= 0, ("ftell on '%s' failed", w->path) );

        w->current_chunk_size_pos = pos;

        w->current_chunk_size = 0;
    }
    else
    {
        w->current_chunk_size_pos = 1; /* Hack, to force write_chunk_finalize
                                          to insert a space next time
                                          (although current position is zero)
                                       */
    }

    /* Dummy value */
    write_uint64(w, 0);
    
    return 0;
}

error_t
sdf_writer_add_data_float_float(sdf_writer_t w, float f1, float f2, const char *format)
{
    assure( w != NULL, ("Null input") );

    w->current_chunk_size += 1;

    if (w->binary_format)
    {
        write_float32(w, f1);
        write_float32(w, f2);
    }
    else
    {
        w->sink->printf(w->sink, format, f1, f2);
    }
    
    if (w->binary_format)
    {
      if (w->sink->flush != NULL && ((buffer_t *)w->sink)->pos > CHUNK_SIZE)
      {
	  sdf_writer_next_chunk(w, w->cc);
      }
    }

    return 0;
}

error_t
sdf_writer_add_data_double_double(sdf_writer_t w, double f1, double f2, const char *format)
{
    assure( w != NULL, ("Null input") );

    w->current_chunk_size += 1;

    if (w->binary_format)
    {
        write_float32(w, f1);
        write_float32(w, f2);
    }
    else
    {
        w->sink->printf(w->sink, format, f1, f2);
    }

    if (w->binary_format)
    {
      if (w->sink->flush != NULL && ((buffer_t *)w->sink)->pos > CHUNK_SIZE)
      {
	  sdf_writer_next_chunk(w, w->cc);
      }
    }

    return 0;
}


error_t
sdf_writer_add_data_uint32(sdf_writer_t w, uint32_t u)
{
    assure( w != NULL, ("Null input") );

    w->current_chunk_size += 1;

    write_uint32(w, u);

    if (w->binary_format)
    {
        if (w->sink->flush != NULL && ((buffer_t *)w->sink)->pos > CHUNK_SIZE)
        {
            sdf_writer_next_chunk(w, w->cc);
        }
    }

    return 0;
}

error_t
sdf_writer_delete(sdf_writer_t * w)
{
    if (w != NULL && *w != NULL)
    {
        if ((*w)->binary_format)
        {
            write_chunk_finalize(*w);
        }
        /* else: no newline after last chunk */

        string_delete((*w)->path);
        (*w)->sink->close((*w)->sink);

        sdf_chunk_config_delete(&(*w)->cc);
        
        free(*w);
    }

    return 0;
}

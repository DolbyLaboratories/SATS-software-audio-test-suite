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
;	File:	debug.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "wavelib.h"
#include "debug.h"

#define PCM_BUF_SIZE 1024
#ifdef DEBUG
static FILE *pcm;
static unsigned short pcm_buf[PCM_BUF_SIZE];
#endif


void debugInit( void )
{
#ifdef DEBUG
	/* zero out the PCM file */
	pcm = fopen( DEBUG_NAME_PCM, "wb" );
	if ( pcm == NULL )
		return;
#endif
}

void debugSample( double d )
{
	d = d;
#ifdef DEBUG
	short s;

	s = (short)( d * 32768.0 );
	fwrite( &s, 2, 1, pcm );
#endif
}

void debugCleanup( void )
{
#ifdef DEBUG
	struct WaveFile wf;
	long len;
	int res;


	fclose( pcm );
	pcm = fopen( DEBUG_NAME_PCM, "rb" );
	if ( pcm == NULL )
		return;	/* no file, nothing to do */

	fseek( pcm, 0, SEEK_END );	/* go to end of file */
	len = ftell( pcm );			/* find out how many bytes */
	fseek( pcm, 0, SEEK_SET );	/* go to start of file */

	res = write_wav_header( &wf, DEBUG_NAME_WAV, 1, len / 2, 48000, 16 );
	if ( res != NO_ERR )
		return;

	while( ( res = fread( pcm_buf, 1, PCM_BUF_SIZE, pcm ) ) > 0 )
		{
		fwrite( pcm_buf, 1, res, wf.file );
		}

	fclose( pcm );
	close_wav( &wf );
#endif

}

void debug(char *fmt, ...)
{
	fmt = fmt;
#ifdef DEBUG
	va_list args;
	va_start(args,fmt);
	vfprintf(stdout, fmt, args);
	fflush( stdout );
	va_end(args);
#endif
}

void info(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
}

void warning(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "WARNING: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
}

void error(char *fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    /* Print on both the output and error channels */
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, args);
    //fprintf(stdout, "ERROR: ");
    //vfprintf(stdout, fmt, args);
    //fflush( stdout );
    //fflush( stderr );
    va_end(args);
}



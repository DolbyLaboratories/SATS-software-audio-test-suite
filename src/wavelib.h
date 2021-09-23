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
;	File:	wavelib.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

/****************************************************************************
;   Summary of WaveLib functions:
;   -----------
;   int read_wav_header(struct WaveFile *wav, char *file, short *numchans, long *numsamps, long *samprate, short *bitspersamp, short *wavx);
;   int write_wav_header(struct WaveFile *wav, char *file, short numchans, long numsamps, long samprate, short bitspersamp);
;   int write_wavx_header(struct WaveFile *wav, char *file, short numchans, long numsamps, long samprate, short bitspersamp);
;   -----------
;   void read_wav_init(struct WaveFile *wav, unsigned long initial_size);
;   long read_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size);
;   unsigned long read_wav_bytesleft(struct WaveFile *wav);
;   int parse_bwf_subchunk(FILE *p_file, unsigned long subchunk_size, BWF_STRUCT *p_bwf);
;   -----------
;   long write_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size);
;   -----------
;   void close_wav(struct WaveFile *wav);
;***************************************************************************/

/*#ifdef WAV_SUPPORT */

#ifndef WAVE_LIB_H
#define WAVE_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************
*
*	Error Codes
*
*	The wavelib routines may return any of these error codes:
*
****************************************************************************/
enum WaveLibErrors { NO_ERR, FILE_READ_ERR, FORMAT_ERR, FILE_WRITE_ERR, NO_DATA_ERR };

/****************************************************************************
*
*	WaveFile
*
*	The WaveFile structure holds state between calls to wavelib routines.
*
*   file      - file handle to this wave file
*   bytesleft - the number of bytes left (for reading)
****************************************************************************/
struct WaveFile
{
	FILE *file;
	unsigned long bytesleft;
};

/****************************************************************************
*
*	BWF information structure 
*
*	This structure holds broadcast wave file subchunk information as explained
*   in the EBU t3285, Verion 1, July 2001 spec.  Some fields are shorter than
*   the spec since they are ignored and we are mainly interested in time code.
*
****************************************************************************/
typedef struct 
{
	char  description[256];
	char  originator[32];
	char  originator_reference[32];
	char  origination_date[10];
	char  origination_time[8];
	unsigned long  time_reference_low;
	unsigned long  time_reference_high;
	unsigned short version;
	char  umid[64];
	char  reserved[190];
    /* coding_history can be longer than 256. any remaining data is ignored.
    This is done to avoid a complicated interface as recommended by the EBU. */
	char  coding_history[256];	
} BWF_STRUCT;

/****************************************************************************
*
*	read_wav_header
*
*	Reads a WAVEFORMATEX or WAVEFORMATEXTENSIBLE compatible wave file header
*   and sets up the WaveFile structure by opening the wave file.  Callers
*   must manually close the file or call close_wav() to free the file handle.
*      wav         - [i/o] structure to be passed to other wavelib routines
*      file        - [in]  file name to read from 
*      numchans    - [out] number of channels
*      numsamps    - [out] number of samples
*      samprate    - [out] sampling rate 
*      bitspersamp - [out] sample bit depth
*      wavx        - [out] set to a nonzero value if the header conforms to
*                          WAVEFORMATEXTENSIBLE.  If the caller doesn't need
*                          this information, the wavx parameter can be NULL.
*      audio_type  - [out] format code (1: PCM, 3: FLOAT, 0xFFFE: WAVEFORMATEX)
*      format_type - [out] subformat for WAVEFORMATEX
*   Return value:
*      an error code; one of the values from the WaveLibErrors enumeration
****************************************************************************/
int read_wav_header(
		struct WaveFile *wav, 
		char	*file, 
		short	*numchans, 
		long long *numsamps, 
		int	    *samprate, 
		short	*bitspersamp, 
		short	*wavx,
	    int	    *timeref_lo, 
		int	    *timeref_hi,
		int	    *audio_type,
		short   *format_type
);

/****************************************************************************
*
*	write_wav_header
*
*	Writes a standard WAVEFORMATEX compatible wave file header and sets up
*   the WaveFile structure by opening the wave file for writing.  Callers
*   must manually close the file or call close_wav() to free the file handle.
*      wav         - [i/o] structure to be passed to other wavelib routines
*      file        - [in]  file name to write to
*      numchans    - [in]  number of channels
*      numsamps    - [in]  number of samples
*      samprate    - [in]  sampling rate
*      bitspersamp - [in]  sample bit depth
*   Return value:
*      an error code; one of the values from the WaveLibErrors enumeration
****************************************************************************/
int write_wav_header(struct WaveFile *wav, char *file, short numchans, 
	long numsamps, int samprate, short bitspersamp);

/****************************************************************************
*
*	write_wavx_header
*
*	Writes a WAVEFORMATEXTENSIBLE compatible wave file header and sets up
*   the WaveFile structure by opening the wave file for writing.  Callers
*   must manually close the file or call close_wav() to free the file handle.
*      wav         - [i/o] structure to be passed to other wavelib routines
*      file        - [in]  file name to write to
*      numchans    - [in]  number of channels
*      numsamps    - [in]  number of samples
*      samprate    - [in]  sampling rate
*      bitspersamp - [in]  sample bit depth
*   Return value:
*      an error code; one of the values from the WaveLibErrors enumeration
****************************************************************************/
int write_wavx_header(struct WaveFile *wav, char *file, short numchans, 
	long numsamps, int samprate, short bitspersamp);

/****************************************************************************
*
*	read_wav_init
*
*	Initializes file reading by setting up bytesleft counter, to track how
*   many bytes are left to read from the file.
*      wav          - [i/o] maintains state between calls to wavelib routines
*      initial_size - [in]  initial size to set counter to, specified in # bytes
****************************************************************************/
void read_wav_init(struct WaveFile *wav, unsigned long initial_size);

/****************************************************************************
*
*	read_wav_chunk
*
*	Reads a chunk of data from the input file and decrements bytesleft
*      wav  - [i/o] maintains state between calls to wavelib routines
*      buf  - [in]  the buffer to read data into
*      size - [in]  the maximum amount of data (in bytes) to read (buf must be >= size)
*   Return value:
*      the actual number of bytes read or -1 for an error
****************************************************************************/
long read_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size);

/****************************************************************************
*
*	read_wav_bytesleft
*
*   Returns the nubmer of bytes remaining to be read from the file
*      wav - [in] maintains state between calls to wavelib routines
*	Return value:
*      the number of bytes remaining to be read from the file
****************************************************************************/
unsigned long read_wav_bytesleft(struct WaveFile *wav);

/****************************************************************************
*
*	write_wav_chunk
*
*	Writes a chunk of data to the outputfile
*      wav  - [in] maintains state between calls to wavelib routines
*      buf  - [in] the buffer to read data from
*      size - [in] the size of the buffer in bytes
*   Return value:
*      the actual number of bytes written or -1 for an error
****************************************************************************/
long write_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size);

/****************************************************************************
*
*	close_wav
*
*	Closes the wave file handle.
*      wav  - [in] maintains state between calls to wavelib routines
****************************************************************************/
void close_wav(struct WaveFile *wav);

/****************************************************************************
*
*	parse_bwf_subchunk
*
*	Parses the broadcast wave format data chunk.
*      p_file  - file to be read (should this be a WaveFile?)
*      subchunk_size - size (in bytes) of BWF data chunk
*      p_bwf - struct to be filled with BWF metadata
****************************************************************************/
int parse_bwf_subchunk(FILE *p_file, unsigned int subchunk_size, BWF_STRUCT *p_bwf);

enum endianess { DONT_KNOW, LITTLE, BIG, CANT_TELL };
extern enum endianess endian_type;
extern int convert_int32( int il );
extern short convert_int16( short is );

#endif /* WAVE_LIB_H */

/*#endif WAV_SUPPORT*/

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
;	File:	wavelib.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "typedef.h"
#include "wavelib.h"
#include "debug.h"
#include "stdio64.h"

//#ifdef WAV_SUPPORT
//enum endianess endian_type = LITTLE;//DONT_KNOW;
/* GUIDs */
#define KSDATAFORMAT_SUBTYPE_PCM   "\x01\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"
#define KSDATAFORMAT_SUBTYPE_FLOAT "\x03\x00\x00\x00\x00\x00\x10\x00\x80\x00\x00\xaa\x00\x38\x9b\x71"
extern void find_endianess( void );

void find_endianess( void )
{
	unsigned int l = 0x01;
	unsigned char *ucp = (unsigned char *)&l;

	if ( *ucp == 0x01 )
		{
		endian_type = LITTLE;
//		info("LITTLE: (%ld)\n", l );
		}
	else if ( *(ucp+3) == 0x01 )
		{
		endian_type = BIG;
//		info("BIG: (%ld)\n", l );
		}
	else
		{
		error("Can't determine endian_type (%d)\n", l );
		endian_type = CANT_TELL;
		}
}

int convert_int32( int il )
{
	int ol;
	unsigned char *icp;
	unsigned char *ocp;

	switch ( endian_type )
		{
		case DONT_KNOW:
			find_endianess();
			return( convert_int32( il ) );
			break;
		case LITTLE:
			return( il );
			break;
		case BIG:
			icp = (unsigned char *)&il;
			ocp = (unsigned char *)&ol;
			*ocp++ = *(icp + 3 );
			*ocp++ = *(icp + 2 );
			*ocp++ = *(icp + 1 );
			*ocp++ = *(icp + 0 );
			return( ol );
			break;
		case CANT_TELL:
		default:
			return( 0 ); // that ought to break things
			break;
		}
}

short convert_int16( short is )
{
	short os;
	unsigned char *icp = (unsigned char *)&is;
	unsigned char *ocp = (unsigned char *)&os;
	switch ( endian_type )
		{
		case DONT_KNOW:
			find_endianess();
			return( convert_int16( is ) );
			break;
		case LITTLE:
			return( is );
			break;
		case BIG:
			icp = (unsigned char *)&is;
			ocp = (unsigned char *)&os;
			*ocp++ = *(icp + 1 );
			*ocp++ = *(icp + 0 );
			return( os );
			break;
		case CANT_TELL:
		default:
			return( 0 ); // that ought to break things
			break;
		}
}

int read_wav_header(
		struct WaveFile *wav, 
		char	*file , 
		short	*numchans, 
		int	    *numsamps, 
		int	    *samprate, 
		short	*bitspersamp, 
		short	*wavx,
	    int	    *timeref_lo, 
		int	    *timeref_hi,
        int     *audio_type,
		short   *format_type
)
{
	char chunk_id[4];
	char form_type[4];
	char subchunk_id[4];
	short audio_format = 0, num_channels = 0;
	int sample_rate = 0, avg_byte_rate;
	short block_align = 0, bits_per_sample = 0, bytes_per_sample;
	short cbsize, valid_bits_per_sample;
	int channel_mapping;
	short subformat_guid[8];
	int done;
	short is_wavx = 0;
	int bytes_read;
	//short end_of_file = 0;
	short got_data_chunk = 0;
	short got_fmt_chunk = 0;
	unsigned int chunk_size;
	unsigned int subchunk_size;
	unsigned int data_subchunk_size = 0;
    short fmt_type = 1; /* this is for finding the wave format i.e pcm or IEEE*/
    unsigned int  guidLen  = 16;

#ifdef ANSI_FIO
	unsigned int data_chunk_start = 0;
#else
	__int64 data_chunk_start = 0;
#endif /* ANSI_FIO */
	BWF_STRUCT bwf_info;

	assert(wav && numchans && numsamps && samprate && bitspersamp);

/*  Open file */

	/* Don't do this since already opened in exec */
	if (file != NULL)
		if ((wav->file = fopen(file,"rb")) == NULL) 
			return FILE_READ_ERR;

/*	Read chunk header */

	if (fread(chunk_id, 1, 4, wav->file) != 4) return(FILE_READ_ERR);
	if (strncmp("RIFF", chunk_id, 4)) return(FORMAT_ERR);

	if (fread(&chunk_size, 4, 1, wav->file) != 1) return(FILE_READ_ERR);

	if (fread(form_type, 1, 4, wav->file) != 4) return(FILE_READ_ERR);
	if (strncmp("WAVE", form_type, 4)) return(FORMAT_ERR);

	/* Read or skip various subchunks */
	done = 0;
	while (!done)
	{
		/* Read next subchunk type and size, go until end of file */
		bytes_read = (int) fread(subchunk_id,  1,  4, wav->file);

		if (bytes_read < 4)
		{
			//end_of_file = 1;
			break;
		}

		/* Read subchunk size */
		if (fread(&subchunk_size, 1, 4, wav->file) != 4) return(FILE_READ_ERR);

		/* Format subchunk */
		if (strncmp("fmt ", subchunk_id, 4) == 0)
		{
			if (subchunk_size < 16) return(FORMAT_ERR);
			if (fread(&audio_format, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
			
			if (audio_format == 1 || audio_format == 3 ) is_wavx = 0;
			else if (audio_format == -2) is_wavx = 1;
			else return(FORMAT_ERR);

			if (fread(&num_channels, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
			if (fread(&sample_rate, 4, 1, wav->file) != 1) return(FILE_READ_ERR);
			if (fread(&avg_byte_rate, 4, 1, wav->file) != 1) return(FILE_READ_ERR);
			if (fread(&block_align, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
			if (fread(&bits_per_sample, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
			bytes_per_sample = (short)(bits_per_sample / 8);
			if (bits_per_sample > (bytes_per_sample * 8)) bytes_per_sample++;

			/* The following line only has an effect if the bit depth is not  */
			/* a multiple of 8.  Not sure that it's correct. JBL Nov 26, 2003 */
			if ((bytes_per_sample % 2) && (num_channels % 2) && !(block_align % 2)) block_align /= 2;

			if (block_align != (bytes_per_sample * num_channels)) return(FORMAT_ERR);
			if (avg_byte_rate != (sample_rate * block_align)) return(FORMAT_ERR);

			if (is_wavx)
			{
				if (fread(&cbsize, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
				if (cbsize != 22) return(FORMAT_ERR);
				if (fread(&valid_bits_per_sample, 2, 1, wav->file) != 1) return(FILE_READ_ERR);
				if (fread(&channel_mapping, 4, 1, wav->file) != 1) return(FILE_READ_ERR);
				if (fread(subformat_guid, 2, 8, wav->file) != 8) return(FILE_READ_ERR);
				/* Verify this..  */
			    if (!strncmp((char*)subformat_guid, KSDATAFORMAT_SUBTYPE_PCM, guidLen))
			      {
			    	fmt_type = 0;
			      }
			    else if (!strncmp((char*)subformat_guid, KSDATAFORMAT_SUBTYPE_FLOAT, guidLen))
			      {
			    	fmt_type = 1;
			      }

				if (subchunk_size > 40)
				{
					if (fseek(wav->file, subchunk_size - 40, SEEK_CUR)) return(FILE_READ_ERR);
				}
			}
			else
			{	/* Consume any remaining bytes */
				if (subchunk_size > 16)
				{
					if (fseek(wav->file, subchunk_size - 16, SEEK_CUR)) return(FILE_READ_ERR);
				}
			}

			/* If odd subchunk size eat pad byte */
			if (subchunk_size % 2) if (fseek(wav->file, 1, SEEK_CUR) != 0) return(FILE_READ_ERR);

			/* Set flag since this is a required chunk */
			got_fmt_chunk = 1;
		}
		/* Data subchunk - mark it, save size, and skip over it for now */
		else if (strncmp("data", subchunk_id, 4) == 0)
		{
			if (chunk_size < (subchunk_size + 36)) return(FORMAT_ERR);

			/* Keep bytes left for return to caller */
#ifndef ANSI_FIO
			//assert(subchunk_size <= (unsigned long)(-1));  %This is always true ??
			wav->bytesleft = (unsigned long)subchunk_size;
#else
			wav->bytesleft = subchunk_size;
#endif /* ANSI_FIO */

			/* Mark this position in file to return to it later */
#ifdef ANSI_FIO
			data_chunk_start = ftell(wav->file);
#else
			/* We need enough resolution to handle 4GB files, however ftell only 
			   supports 2GB files, as it returns a signed long.  Instead use the
			   64bit version, check the range, and cast back down to a 32bit ulong */
			data_chunk_start = ftello(wav->file);
			assert((unsigned long)data_chunk_start <= (unsigned long)(-1));
#endif

			/* Skip over this chunk in case there are more chunks that follow it */
#ifdef ANSI_FIO
			if (fseek(wav->file, subchunk_size, SEEK_CUR)) return(FILE_READ_ERR);
#else
			/* Again, we need enough resolution to handle 4GB files, however fseek only 
			   supports 2GB files, as it takes a signed long.  Instead use the
			   64bit version */
			if (fseeko(wav->file, (__int64)subchunk_size, SEEK_CUR)) return(FILE_READ_ERR);
#endif /* ANSI_FIO */

			/* If odd subchunk size eat pad byte */
			if (subchunk_size % 2) if (fseek(wav->file, 1, SEEK_CUR) != 0) return(FILE_READ_ERR);

			/* Set flag since this is a required chunk */
			got_data_chunk = 1;
			data_subchunk_size = subchunk_size;
		}
		/* Broacast subchunk */
		else if (strncmp("bext", subchunk_id, 4) == 0)
		{
			/* Parse broadcast wave file subchunk */
			parse_bwf_subchunk(wav->file, subchunk_size, &bwf_info);
		    
            /* allow the user to pass null pointers if they don't care about these values */
            if (timeref_hi)
            {
		        *timeref_hi = bwf_info.time_reference_high;
            }
            if (timeref_lo)
            {
		        *timeref_lo = bwf_info.time_reference_low;
		    }
        }
		/* Skip unsupported subchunk including fact, cue, plst, list, labl, ltxt, note, smpl, inst */
		else
		{
#ifdef ANSI_FIO
			if (fseek(wav->file, subchunk_size, SEEK_CUR)) return(FILE_READ_ERR);
#else
			/* We need enough resolution to handle 4GB files, however fseek only 
			   supports 2GB files, as it takes a signed long.  Instead use the
			   64bit version */
			if (fseeko(wav->file, (__int64)subchunk_size, SEEK_CUR)) return(FILE_READ_ERR);
#endif /* ANSI_FIO */
		}
	}

	/* Check for presence of at least data and fmt chunks */
	if (got_fmt_chunk == 0) return (FORMAT_ERR);
	if (got_data_chunk == 0) return (NO_DATA_ERR);

	/* Set file pointer back to start of data chunk */
#ifdef ANSI_FIO
	if (fseek(wav->file, data_chunk_start, SEEK_SET)) return(FILE_READ_ERR);
#else
	/* We need enough resolution to handle 4GB files, however fseek only 
	   supports 2GB files, as it takes a signed long.  Instead use the
	   64bit version */
	if (fseeko(wav->file, (__int64)data_chunk_start, SEEK_SET)) return(FILE_READ_ERR);
#endif /* ANSI_FIO */

/*	Set up return values */

	*numchans = num_channels;
	*numsamps = (long)(data_subchunk_size / (unsigned long)block_align);
	*samprate = sample_rate;
	*bitspersamp = bits_per_sample;
	if (wavx) *wavx = is_wavx;
    *audio_type = audio_format;
	*format_type = fmt_type; /* this is for passing the type of data i.e PCM or IEEE format */
	return(NO_ERR);
}

int write_wav_header(struct WaveFile *wav, char *file, short numchans, 
	long numsamps, int samprate, short bitspersamp)
{
	char chunk_id[4];
	unsigned int chunk_size;
	char form_type[4];
	char fmt_subchunk_id[4];
	int fmt_subchunk_size;
	short audio_format, num_channels;
	int sample_rate, avg_byte_rate;
	short block_align, bits_per_sample;
	char data_subchunk_id[4];
	unsigned int data_subchunk_size;

	assert(wav);

/*  Open file */

	/* Don't do this since already opened in exec */
	if (file != NULL)
		if ((wav->file = fopen(file,"wb")) == NULL) 
			return(FILE_WRITE_ERR);

/*	Set up data */

	bits_per_sample = bitspersamp;
	num_channels = numchans;
	sample_rate = samprate;
	block_align = (bits_per_sample * num_channels) / 8;
	avg_byte_rate = sample_rate * block_align;
	data_subchunk_size = numsamps * block_align;
	fmt_subchunk_size = 16;
	audio_format = 1;
	chunk_size = data_subchunk_size + 36;
	strncpy(chunk_id, "RIFF", 4);
	strncpy(form_type, "WAVE", 4);
	strncpy(fmt_subchunk_id, "fmt ", 4);
	strncpy(data_subchunk_id, "data", 4);

/*	Write header to file */

	if (fwrite(chunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&chunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(form_type, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(fmt_subchunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&fmt_subchunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&audio_format, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&num_channels, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&sample_rate, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&avg_byte_rate, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&block_align, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&bits_per_sample, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(data_subchunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&data_subchunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);

	return(NO_ERR);
}

int write_wavx_header(struct WaveFile *wav, char *file, short numchans, 
	long numsamps, int samprate, short bitspersamp)
{
	char chunk_id[4];
	unsigned int chunk_size;
	char form_type[4];
	char fmt_subchunk_id[4];
	int fmt_subchunk_size;
	short audio_format, num_channels;
	int sample_rate, avg_byte_rate;
	short block_align, bits_per_sample, bytes_per_sample;
    short wave_format_extensible_size, valid_bits_per_sample;
    int channel_mapping;
	char data_subchunk_id[4];
	unsigned int data_subchunk_size;
	enum { NO_ERR, FILE_WRITE_ERR };

    int guid[8] = { (int) 0x0001, (int) 0x0000, (int) 0x0000, (int) 0x0010, 
                       (int) 0x0080, (int) 0xAA00, (int) 0x3800, (int) 0x719B 
                    };

	assert(wav && file);

/*  Open file */

	if ((wav->file = fopen(file,"wb")) == NULL) return(FILE_WRITE_ERR);

/*	Set up data */

	bits_per_sample = bitspersamp;

	bytes_per_sample = (short)(bits_per_sample / 8);
	if (bits_per_sample > (bytes_per_sample * 8))
	{
		bytes_per_sample++;
	}

	num_channels = numchans;
	sample_rate = samprate;
	block_align = bytes_per_sample * num_channels;
	avg_byte_rate = sample_rate * block_align;

    wave_format_extensible_size = 22;
    valid_bits_per_sample = 16;
    channel_mapping = 0;

	data_subchunk_size = numsamps * block_align;
	fmt_subchunk_size = 40;
	audio_format = -2;
	chunk_size = data_subchunk_size + 36 + 24;
	strncpy(chunk_id, "RIFF", 4);
	strncpy(form_type, "WAVE", 4);
	strncpy(fmt_subchunk_id, "fmt ", 4);
	strncpy(data_subchunk_id, "data", 4);

    switch (num_channels)
    {
    case 1:
        channel_mapping = 0x00000004; /* C */
        break;
    case 2:
        channel_mapping = 0x00000003; /* L R */
        break;
    case 3:
        channel_mapping = 0x00000007; /* L C R */
        break;
    case 4:
        channel_mapping = 0x00000033; /* L R Ls Rs */
        break;
    case 5:
        channel_mapping = 0x00000037; /* L C R Ls Rs */
        break;
    case 6:
        channel_mapping = 0x0000003F; /* L C R Ls Rs SW*/
        break;
    default:
        channel_mapping = 0x0000003F; /* L C R Ls Rs SW*/
        break;
    }

/*	Write header to file */

	if (fwrite(chunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&chunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(form_type, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(fmt_subchunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&fmt_subchunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&audio_format, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&num_channels, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&sample_rate, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&avg_byte_rate, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&block_align, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
	if (fwrite(&bits_per_sample, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
    if (fwrite(&wave_format_extensible_size, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
    if (fwrite(&valid_bits_per_sample, 2, 1, wav->file) != 1) return(FILE_WRITE_ERR);
    if (fwrite(&channel_mapping, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);
    if (fwrite(guid, 2, 8, wav->file) != 8) return(FILE_WRITE_ERR);
	if (fwrite(data_subchunk_id, 1, 4, wav->file) != 4) return(FILE_WRITE_ERR);
	if (fwrite(&data_subchunk_size, 4, 1, wav->file) != 1) return(FILE_WRITE_ERR);

	return(NO_ERR);
}

void read_wav_init(struct WaveFile *wav, unsigned long initial_size)
{
	assert(wav);
	wav->bytesleft = initial_size;
}

long read_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size)
{
	unsigned long readbytes;

	assert(wav && buf);

	if (wav->bytesleft > size)
	{
		readbytes = size;
	}
	else
	{
		readbytes = wav->bytesleft;
	}

	if (fread(buf, 1, (size_t)readbytes, wav->file) != (size_t)readbytes)
	{
		return -1;
	}

	wav->bytesleft -= readbytes;
	return readbytes;
}

unsigned long read_wav_bytesleft(struct WaveFile *wav)
{
	assert(wav);
	return wav->bytesleft;
}

long write_wav_chunk(struct WaveFile *wav, void *buf, unsigned long size)
{
	assert(wav && buf);

	if (fwrite(buf, 1, (size_t)(size), wav->file) != (size_t)(size))
		return -1;

	return size;
}

void close_wav(struct WaveFile *wav)
{
	assert(wav);
	fclose(wav->file);
}

/* Parse bwf broadcast extension 'bext' subchunk */
int parse_bwf_subchunk(FILE *p_file, unsigned int subchunk_size, BWF_STRUCT *p_bwf)
{
	int err;
	unsigned long bytes_remain; 

	err = 0;

	if (fread(p_bwf->description, 1, 256, p_file) != 256) return(FILE_READ_ERR);
	if (fread(p_bwf->originator, 1, 32, p_file) != 32) return(FILE_READ_ERR);
	if (fread(p_bwf->originator_reference, 1, 32, p_file) != 32) return(FILE_READ_ERR);
	if (fread(p_bwf->origination_date, 1, 10, p_file) != 10) return(FILE_READ_ERR);
	if (fread(p_bwf->origination_time, 1, 8, p_file) != 8) return(FILE_READ_ERR);
	/* Parse time reference low and high - Note - this is the data of interest to us! */
	if (fread(&p_bwf->time_reference_low,  4, 1, p_file) != 1) return(FILE_READ_ERR);
	if (fread(&p_bwf->time_reference_high, 4, 1, p_file) != 1) return(FILE_READ_ERR);
	if (fread(&p_bwf->version, 2, 1, p_file) != 1) return(FILE_READ_ERR);
	/* Parse umid - unique material identifier */
	if (fread(p_bwf->umid, 1, 64, p_file) != 64) return(FILE_READ_ERR);
	if (fread(p_bwf->reserved, 1, 190, p_file) != 190) return(FILE_READ_ERR);
	
    /* The total bytes read so far in the BWF chunk */
    bytes_remain = subchunk_size - 602;

	/* Parse coding history - length is determined by subchunk_size minus the size of the other elements in the block */
	if (bytes_remain < 256)
    {
        if (fread(p_bwf->coding_history, 1, bytes_remain, p_file) != bytes_remain) return(FILE_READ_ERR);
    }else{
        if (fread(p_bwf->coding_history, 1, 256, p_file) != bytes_remain) return(FILE_READ_ERR);
        bytes_remain -= 256;
        if (fseek(p_file, bytes_remain, SEEK_CUR) != 0) return(FILE_READ_ERR);
    }

	/* If odd subchunk size eat pad byte */
	if (subchunk_size % 2) if (fseek(p_file, 1, SEEK_CUR) != 0) return(FILE_READ_ERR);

	return (err);
}

//#endif /* WAV_SUPPORT */



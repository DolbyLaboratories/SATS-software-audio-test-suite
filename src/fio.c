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
;	File:	fio.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include "fio.h"
#include "wavelib.h"
#include "power.h"
#include "Utilities.h"
#include "version.h"
#include "debug.h"


enum endianess endian_type = LITTLE;

static void fio_debug( char *fmt, ... );

static void fio_debug( char *fmt, ... )
{
	fmt = fmt;
#ifdef DEBUG
#ifdef FIO_DEBUG
    va_list args;
    va_start(args,fmt);
    fprintf(stdout, "FIO: ");
    vfprintf(stdout, fmt, args);
    fflush( stdout );
    va_end(args);
#endif
#endif
    return;
}

static
const char *OS_string(void)
{
#ifdef _WIN64
    return "Windows 64-bit";
#elif _WIN32
    return "Windows 32-bit";
#elif __APPLE__
    return "OS-X";
#elif __linux
#if __GNUC__
#if __x86_64__
    return "Linux x86_64";
#elif __ppc64__
    return "Linux ppc64";
#else
    return "Linux x86";
#endif  /* x86_64 */
#else
    return "Linux";
#endif /* gnuc */

#elif __unix // all unices not caught above
    return "Unix";
#elif __posix
    return "POSIX";
#else
    return "<unknown-OS>";
#endif
}

int fio_init( pfstruct pfs, 
              const char *tool, 
              const char *axis1,
              const char *axis2,
              const char *axis3) /* 3rd axis added here to meet spectrogram demands*/
{
	int i;
	const char *axes[3];
	const char *format_tag;
	axes[0] = axis1;
	axes[1] = axis2;
	axes[2] = axis3;


    assert( tool != NULL );

    if (strcmp(tool, "bandwidth") == 0)
    {
        /* It is unclear what this tool does and why it exists */
        pfs->header = NULL;
        pfs->sdf_out = NULL;
        return 0;
    }

    assert( axis1 != NULL );
    assert( axis2 != NULL );
    /*No assertion for the 3rd axis needed as its only for spectrogram tool*/

    check( sdf_config_new(&pfs->header) );
    check( sdf_config_key_string(pfs->header, "tool", tool, "SATS tool used") );
    check( sdf_config_key_string(pfs->header, "version", VERSION_STRING, "SATS tool version") );
    check( sdf_config_key_string(pfs->header, "OS", OS_string(), "Operating system, build type") );
    check( sdf_config_key_string(pfs->header, "filename", pfs->filename, "Input WAV file") );

    check( sdf_config_key_uint32(pfs->header, "sample_rate", pfs->fs, "Sample rate in Hz") );
    check( sdf_config_key_uint32(pfs->header, "num_channels", pfs->channels, "Number of channels") );
    check( sdf_config_key_uint64(pfs->header, "num_samples", pfs->size, "Number of audio samples") );
    check( sdf_config_key_uint32(pfs->header, "bit_depth", pfs->bytespersamp * 8, "Bit depth") );
    if (pfs->audio_type == 1)
    {
        format_tag = "PCM";
    }
    else if (pfs->audio_type == 3)
    {
        format_tag = "FLOAT";
    }
    else if (pfs->audio_type == 65534)
    {
        format_tag = "WAVEFORMATEX";
    }
    else
    {
        format_tag = "<unknown>";
    }    
    check( sdf_config_key_string(pfs->header, "format", format_tag, "WAV data type") );

    for (i = 0; i < 3; i++)/*i=3 as all 3 axis need to have units assigned */
    {
        const char *unit;
        const char *description = "";

        if      (axes[i] == NULL)
                {
                    unit = "";
                }

        else if      (strcmp(axes[i], "Time") == 0)
        {
            unit = "seconds";
        }
        else if      (strcmp(axes[i], "Frequency") == 0)
        {
            unit = "Hz";
            if (strcmp(tool, "mult_freq_resp") == 0)
            {
                description = "Nearest available frequency";
            }
        }
        else if (strcmp(axes[i], "Amplitude") == 0)
        {
            unit = "dBFS";
        }
		else if (strcmp(axes[i], "mel") == 0)
        {
            unit = "mel";
        }
        else if (strcmp(axes[i], "Dynamic range") == 0)
        {
            unit = "dB";
        }
        else if (strcmp(axes[i], "Level") == 0)
        {
            unit = "dBFS";
        }
        else if (strcmp(axes[i], "Noise modulation") == 0)
        {
            unit = "dB";
        }
        else if (strcmp(axes[i], "THD+N") == 0)
        {
            if (strcmp(tool, "thd_vs_freq") == 0)
            {
                unit = "dBFS";
            }
            else if (strcmp(tool, "thd_vs_level") == 0)
            {
                unit = "dB";
            }
            else 
            {
                assure( 0, ("Sorry, don't know THD+N unit for tool '%s'", tool) );
            }
        }
        else
        {
            assure( 0, ("Unknown axis name: %s\n", axes[i]) );
        }

        if(axes[i] != NULL)
        {
            /*If the tool is spectrogram, then put the values in the spl array of 5 elements*/
            if ((strcmp(pfs->tool, "spectrogram") == 0)||( strcmp(pfs->tool, "mel_scale") == 0))
            {  
                check( sdf_config_axis(pfs->header, axes[i], SDF_FLOAT32, unit, description, &pfs->spect_time_axes[i]) );
            }
            else
            {
                check( sdf_config_axis(pfs->header, axes[i], SDF_FLOAT32, unit, description, &pfs->axes[i]) );
            }	
        }
    }

    /*Adding the channel and channel number as 4th and 5th axes for spectrogram tool as 3 other axes are used for Frequency, time and amplitude*/

    if ((strcmp(pfs->tool, "spectrogram") == 0)||( strcmp(pfs->tool, "mel_scale") == 0))
    {
        check( sdf_config_axis(pfs->header, "Channel", SDF_UINT32, "1", "channel number, from zero", &pfs->spect_time_axes[3]) );
        check( sdf_config_axis(pfs->header, "Channel name", SDF_STRING, "1", "Channel name", &pfs->spect_time_axes[4]) );
    }
    else
    {
        /* Always add channel and channel name as 3rd and 4th axis */
        check( sdf_config_axis(pfs->header, "Channel", SDF_UINT32, "1", "channel number, from zero", &pfs->axes[2]) );
        check( sdf_config_axis(pfs->header, "Channel name", SDF_STRING, "1", "Channel name", &pfs->axes[3]) );
    }

    /* Create writer */
    check( sdf_writer_new(pfs->header, pfs->binary_out, &pfs->sdf_out) );

    if (pfs->dataOutputFile[0] != '\0')
    {
        check( sdf_writer_add_sink(pfs->sdf_out, pfs->dataOutputFile) );
    }
    if (pfs->stdoutFlag)
    {
        check( sdf_writer_add_sink(pfs->sdf_out, "-") );
    }

    return 0;
}

int fio_next_chunk( pfstruct pfs, int ch )
{
    sdf_chunk_config_t cc;
    char channel_label[1024];

    getChannelLabel(pfs, ch, channel_label);

    /* Declare a new data chunk where the channel index and channel names are constant,
       and where the data axes vary */
    if ((strcmp(pfs->tool, "spectrogram") == 0)||( strcmp(pfs->tool, "mel_scale") == 0))
    {
        check( sdf_chunk_config_new(&cc) );
        check( sdf_chunk_config_constant_uint32(cc, pfs->spect_time_axes[3], ch) );
        check( sdf_chunk_config_constant_string(cc, pfs->spect_time_axes[4], channel_label) );
        check( sdf_chunk_config_constant_float32(cc, pfs->spect_time_axes[1], (float)pfs->time) );
        check( sdf_chunk_config_variable(cc, pfs->spect_time_axes[0]) );
        check( sdf_chunk_config_variable(cc, pfs->spect_time_axes[2]) );
    }
    else
    {
        check( sdf_chunk_config_new(&cc) );
        check( sdf_chunk_config_constant_uint32(cc, pfs->axes[2], ch) );
        check( sdf_chunk_config_constant_string(cc, pfs->axes[3], channel_label) );
        check( sdf_chunk_config_variable(cc, pfs->axes[0]) );
        check( sdf_chunk_config_variable(cc, pfs->axes[1]) );
    }

    check( sdf_writer_next_chunk(pfs->sdf_out, cc) );
    check( sdf_chunk_config_delete(&cc) );
    pfs->channel = ch;

    return 0;
}

int fio_eof( pfstruct pfs )
{
    if (pfs->position >= pfs->size)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}
/*
int fio_eof_audio_data( paudio_struct adst )
{
    if (adst->audio_data_cur_position >= adst->audio_data_size)
        return (1);
    else return (0);
}
*/

unsigned char *wavbuf = NULL;
int wavbuf_size = 0;

static
int convert_2_bytes( unsigned char *icp )
{
    int i = 0;
    unsigned char *ocp = (unsigned char *) &i;

    switch (endian_type)
    {
        case DONT_KNOW:
            return (0);
            break;
        case LITTLE:
            *ocp++ = 0;
            *ocp++ = 0;
            *ocp++ = *(icp + 0);
            *ocp++ = *(icp + 1);
            return (i);
            break;
        case BIG:
            *ocp++ = *(icp + 1);
            *ocp++ = *(icp + 0);
            return (i);
            break;
        case CANT_TELL:
        default:
            return (0); // that ought to break things
            break;
    }
}

static
int convert_3_bytes( unsigned char *icp )
{
    int i = 0;
    unsigned char *ocp = (unsigned char *) &i;

    switch (endian_type)
    {
        case DONT_KNOW:
            return (0);
            break;
        case LITTLE:
            *ocp++ = 0;
            *ocp++ = *(icp + 0);
            *ocp++ = *(icp + 1);
            *ocp++ = *(icp + 2);
            return (i);
            break;
        case BIG:
            *ocp++ = *(icp + 2);
            *ocp++ = *(icp + 1);
            *ocp++ = *(icp + 0);
            return (i);
            break;
        case CANT_TELL:
        default:
            return (0); // that ought to break things
            break;
    }
}

static
int convert_4_bytes( unsigned char *icp )
{
    int i = 0;
    unsigned char *ocp = (unsigned char *) &i;

    switch (endian_type)
    {
        case DONT_KNOW:
            return (0);
            break;
        case LITTLE:
            *ocp++ = *(icp + 0);
            *ocp++ = *(icp + 1);
            *ocp++ = *(icp + 2);
            *ocp++ = *(icp + 3);
            return (i);
            break;
        case BIG:
        	*ocp++ = *(icp + 3);
            *ocp++ = *(icp + 2);
            *ocp++ = *(icp + 1);
            *ocp++ = *(icp + 0);
            return (i);
            break;
        case CANT_TELL:
        default:
            return (0); // that ought to break things
            break;
    }
}

/* Read audio data from a file into memory */
int fio_read( pfstruct pfs, unsigned long num_samples_to_read )
{
    unsigned long bytes_to_read;
    int i;
    unsigned char *cp;
    double d, *pd;

    /* Limit number of samples to read, if necessary */
    if ( ((long) num_samples_to_read) + pfs->position >= pfs->size)
    {
        /* Check the position is sensible */
        if (pfs->position >= pfs->size)
        {
            if (pfs->data != NULL)
            {
                free(pfs->data);
                pfs->malloc_size = 0;
            }

            pfs->data = NULL;
            pfs->data_size = 0;
            pfs->data_position = 0;
            return (1);
        }
        num_samples_to_read = pfs->size - pfs->position;
    }

    /* Convert from samples to bytes */
    bytes_to_read = (num_samples_to_read * pfs->channels * pfs->bytespersamp);

    /* Allocate memory to read into */
    if ((int) bytes_to_read != wavbuf_size)
    {
        /* Free if previously allocated */
        if (wavbuf)
            free(wavbuf);

        wavbuf = (unsigned char *) calloc(bytes_to_read, 1);

        if (wavbuf)
            wavbuf_size = (int) bytes_to_read;
        else
        {
            error("wavbuf malloc failed\n");
            return(-1);
        }
    }

    /* Do the read */
    if (fread(wavbuf, 1, bytes_to_read, pfs->fp) != bytes_to_read)
    {
       if (feof(pfs->fp))
	   {
			   return 0;
	   }
	   else
	   {
		error("Read failed\n");
		return (-1);
	   }
    }
	

    /* Set data_position and data_size */
    pfs->data_position = 0;
    pfs->data_size = num_samples_to_read;

    /* Update "position" */
    pfs->position = pfs->position + num_samples_to_read;

    /* Convert to floating point and scale */
    if (pfs->malloc_size != num_samples_to_read * sizeof(double))
    {

        if (pfs->data)
        {
            free(pfs->data);
            pfs->malloc_size = 0;
            pfs->data = NULL;
        }

        if ((pfs->data = (double *) calloc(num_samples_to_read, sizeof(double))) == NULL)
        {
            error("malloc failed\n");
        }

        if (pfs->data)
            pfs->malloc_size = num_samples_to_read * sizeof(double);
        else
            error("data malloc failed\n");
    }

    //	convert to floating point and scale
    cp = (unsigned char *) wavbuf;
    cp += (pfs->channel * pfs->bytespersamp);
    pd = pfs->data;
    for (i = 0; i < ((long) num_samples_to_read); i++)
    {
    	switch(pfs->bytespersamp)
    	{
    	    case 2:
    	        d = (double) convert_2_bytes(cp);
    	        d = d / (double) 0x80000000;
    	        break;

    	    case 3:
    	        d = (double) convert_3_bytes(cp);
    	        d = d / (double) 0x80000000;
    	        break;

    	    case 4:
    	        // if(pfs->format_tag == 1) /* if data type is PCM */
                if(pfs->audio_type == 1) /* if data type is PCM of usual 32 bit wav*/
    	        {
    	            /*convert to double*/
    	            d = (double) convert_4_bytes(cp);
    	            d = d / (double) 0x80000000;
    	        }
				else if(pfs->audio_type == -2 && pfs->format_tag == 0) /* if data type is PCM of 32 bit extensible wav */
				{
					/*convert to double*/
    	            d = (double) convert_4_bytes(cp);
    	            d = d / (double) 0x80000000;
				}
				else if(pfs->audio_type == -2 && pfs->format_tag == 1) /* if data type is float of 32 bit extensible wav */
				{
					d = (double) (*((float *) (cp)));
				}
				else if(pfs->audio_type == 3) /*if data type is IEEE float of usual 32 bit wav*/
    	        {
    	            d = (double) (*((float *) (cp)));
    	        }
				else
				{
                    error("Invalid 32 bit wave file format.");
    	            return(-44);
				}

    	        break;

    	    default:
    	        error("Invalid wave file format. please use only 16,24,32 bit files.");
    	        return(-44);
    	        break;
    	}

        *pd++ = d;
        cp += (pfs->channels * pfs->bytespersamp);
    }

    return (0);
}

int fio_setpos( pfstruct pfs, long new_pos )
{
    fio_debug("fio_setpos: new_pos: %ld\n", new_pos);
    fio_debug("fio_setpos: position: %ld ftell: %ld\n", pfs->position, ftell(pfs->fp));

    pfs->position = pfs->position + new_pos;
    fseek(pfs->fp, new_pos * pfs->channels * pfs->bytespersamp, SEEK_CUR);

    fio_debug("fio_setpos: position: %ld ftell: %ld\n", pfs->position, ftell(pfs->fp));

    return (0);
}

int fio_resetpos( pfstruct pfs )
{
    pfs->position = 0;

    if (pfs->data)
    {
        free(pfs->data);
        pfs->malloc_size = 0;
        pfs->data = NULL;
    }

    fseek(pfs->fp, pfs->header_size, SEEK_SET);

    return (0);
}

int fio_cleanup( pfstruct pfs )
{
    sdf_writer_delete(&pfs->sdf_out);
    sdf_config_delete(&pfs->header);
    fclose(pfs->fp);
    free(wavbuf);
    free(pfs->data);
    return (0);
}

/*check the channel for silence and remove it if the -s flag is set*/


#define NUM_SUBBLOCKS_SILENCE_STRIP 5  /* Defines the number of successive subblocks, whose average power will be compared against the user-adjustable threshold. */
void strip_lead_silence( pfstruct pfs )
{
	int    block_size;
    int    i = 0;
	int    j;
    int    ch;
    int    selected_ch   = pfs->channel;
    long   start_pos = pfs->size;
    long   candidate_pos = 0;
	double abs_value;
    double *pd;
    double block_thres;
    double rms_db;
	double rms_watt = 0.0;
    double sample_thres = 0.0;
	int    step;
	
	
	double subblock_pw [NUM_SUBBLOCKS_SILENCE_STRIP]; /*define an array with the size of [ NUM_SUBBLOCKS_SILENCE_STRIP ] to store the power of all corresponding subblocks*/

	/* choose the model of block power threshold */
	if ((pfs->thr_sSet == 0) && (pfs->thr_dbSet == 1))
	{
        block_size  = pfs->fs/20;
        block_thres = pfs->block_thr; 
		step = block_size / NUM_SUBBLOCKS_SILENCE_STRIP; /* step of block moving is [ 1/NUM_SUBBLOCKS_SILENCE_STRIP ] block_size, then overlap is [ (1-1/NUM_SUBBLOCKS_SILENCE_STRIP) ] block_size*/

	    switch (pfs->bytespersamp)
	    {
		    case 2:
				sample_thres = 1.0/32768.0;
				break;
			case 3:
	            sample_thres = 1.0/8388608.0;
				break;
			case 4:
	            sample_thres = 1.0/2147483648.0;
				break;
		    default:
    	        error("Invalid wave file format. please use only 16,24,32 bit files.");
    	        break;
	    }
	

        for (ch=0; ch<pfs->channels; ch++)
        {
			pfs->channel = ch;    
            rms_db       = -999.0;

			/*initialize the five array elements to store the smallest power*/
			for ( i=0; i < NUM_SUBBLOCKS_SILENCE_STRIP; i++)
			{
				subblock_pw [i] = -999.0;
			}
            i = 0;

            /* find the first block over the block threshold */			
	        while (rms_db < block_thres && !fio_eof(pfs))
            {
	  	        fio_read(pfs, step);
                subblock_pw[i%NUM_SUBBLOCKS_SILENCE_STRIP]= compute_power(pfs->data, step);
	  	        i++;
				for ( j=0; j < NUM_SUBBLOCKS_SILENCE_STRIP; j++)
				{
					rms_watt += pow(10,subblock_pw[j]/20.0) ;/*addition of power of all subblocks*/
				}
	  	        rms_db = 20.0*log10( (1.0/NUM_SUBBLOCKS_SILENCE_STRIP)*( rms_watt ) );	 /*convert to db value*/
				rms_watt = 0;
            }
			start_pos     = pfs->size;

            /* find the first sample in this block over the sample threshold */
            pd = pfs->data;

            for (i = 0; i < pfs->data_size; i++)
            {
		        abs_value = fabs(*pd );
			    if (abs_value>= sample_thres)
			    {
                    break;
				}
                pd++;
            }

            
            candidate_pos = pfs->position - (pfs->data_size - i);
            if (candidate_pos < start_pos)
            {
                start_pos = candidate_pos;
            }
	        fio_resetpos(pfs);
	    }
        pfs->channel = selected_ch;
        fio_setpos(pfs, start_pos);
	}

	/* choose the model of sample threshold */
	else if ((pfs->thr_sSet == 1) && (pfs->thr_dbSet == 0))
	{
	    block_size = pfs->size;
    
	    switch (pfs->bytespersamp)
		{
		    case 2:
			    sample_thres = pow(2,pfs->sample_thr)/32768.0;
				break;
			case 3:
		        sample_thres = pow(2,pfs->sample_thr)/8388608.0;
				break;
			case 4:
		        sample_thres = pow(2,pfs->sample_thr)/2147483648.0;
				break;
            default:
    	        error("Invalid wave file format. please use only 16,24,32 bit files.");
    	        break;
	    }
	

        for (ch=0; ch<pfs->channels; ch++)
        {
            pfs->channel = ch;    
            fio_read(pfs, block_size);
            pd = pfs->data;

            for (i = 0; i < pfs->data_size; i++)
            {
			    abs_value=fabs(*pd );
			    if (abs_value>= sample_thres)
				{
                    break;
				}
                pd++;
            }
        
            candidate_pos = pfs->position - (pfs->data_size - i);
            if (candidate_pos < start_pos)
            {
                start_pos = candidate_pos;
            }
	        fio_resetpos(pfs);
	    }
        pfs->channel = selected_ch;
        fio_setpos(pfs, start_pos);
	}

    /* execute the default model */
	else
	{
		block_size = pfs->size;
    
	    switch (pfs->bytespersamp)
	    {
		    case 2:
				sample_thres = 1.0/32768.0;
				break;
			case 3:
	            sample_thres = 1.0/8388608.0;
				break;
			case 4:
	            sample_thres = 1.0/2147483648.0;
				break;
		    default:
    	        error("Invalid wave file format. please use only 16,24,32 bit files.");
    	        break;
	    }
	
        for (ch=0; ch<pfs->channels; ch++)
        {
            pfs->channel = ch;    
            fio_read(pfs, block_size);
            pd = pfs->data;

            for (i = 0; i < pfs->data_size; i++)
            {
				abs_value=fabs(*pd );
		        if (abs_value >= sample_thres)
			    {
                    break;
			    }
                pd++;
            }
        
            candidate_pos = pfs->position - (pfs->data_size - i);
            if (candidate_pos < start_pos)
            {
                start_pos = candidate_pos;
            }
	        fio_resetpos(pfs);
	    }
        pfs->channel = selected_ch;
        fio_setpos(pfs, start_pos);
	}
}

char *channelMap_1[] =
{ "L", "R", "Ls", "Rs", "C", "LFE" };
char *channelMap_2[] =
{ "L", "C", "R", "Ls", "Rs", "LFE" };
char *channelMap_3[] =
{ "L", "Ls", "C", "Rs", "R", "LFE" };
char *channelMap_4[] =
{ "L", "R", "C", "LFE", "Ls", "Rs" };
char *channelMap_5[] =
{ "L", "C", "Rs", "R", "Ls", "LFE" };
char *channelMap_6[] =
{ "C", "L", "R", "Ls", "Rs", "LFE" };
char *channelMap_7[] =
{ "L", "R", "Ls", "Rs", "C", "LFE", "Lb", "Rb" };

typedef struct
{
    int channels; // total number of channels in this map
    char **map;
} CHANNEL_MAP, *pCHANNEL_MAP;

CHANNEL_MAP channelMaps[] =
{
{ 6, channelMap_1 },
{ 6, channelMap_2 },
{ 6, channelMap_3 },
{ 6, channelMap_4 },
{ 6, channelMap_5 },
{ 6, channelMap_6 }, 
{ 8, channelMap_7 }, };

void getChannelLabel( pfstruct pfs, int channel, char *cp )
{
    *cp = '\0'; // just in case
    if (pfs->channelMap < 0 || pfs->channelMap >= (short)(sizeof(channelMaps) / sizeof(CHANNEL_MAP)))
    { // just use the number
        sprintf(cp, "%d", channel);
    }
    else
    {
        if (channel >= channelMaps[pfs->channelMap].channels)
        {
            sprintf(cp, "%d", channel);
        }
        else
        {
            strcpy(cp, channelMaps[pfs->channelMap].map[channel]);
        }
    }
}

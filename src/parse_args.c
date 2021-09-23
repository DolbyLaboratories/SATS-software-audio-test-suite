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
;	File:	parse_args.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include "getparam.h"
#include "parse_args.h"
#include "wavelib.h"
/*#ifdef WIN32
#include <windows.h>
#endif*/
#include "debug.h"
/*the max limit of unsigned long is ULONG_MAX. Its defined in limits.h*/
/*the max limit of double is DBL_MAX. Its defined in float.h*/
/*the min limit of double is DBL_MIN. Its defined in float.h*/
extern void print_usage( void );
extern void DLB_PrintGetParamError (DLB_GETPARAM_RETVAL);

#define MAXPATH 400
/*Defining the various error cases incase of wrong parameter parsing*/
void DLB_PrintGetParamError (DLB_GETPARAM_RETVAL err)
{
    switch (err)
    {

    case DLB_GETPARAM_SYNTAX_ERROR:
        printf ("Syntax error!\n");
        break;

    case DLB_GETPARAM_OUT_OF_MEMORY:
        printf ("Out of memory for command line parsing!\n");
        break;

#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
    case DLB_GETPARAM_ERROR_CMDFILEOPEN:
        printf ("Cannot open command.txt file!\n");
        break;
#ifdef DLB_GETPARAM_TIMESTAMP
    case DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR:
        printf ("Timestamp syntax error in command.txt file!\n");
        break;
#endif
    case DLB_GETPARAM_EOF:
        /* no real error */
        break;
#endif

    case DLB_GETPARAM_OUT_OF_RANGE:
        printf ("Parameter value was out of the specified range!\n");
        break;

    case DLB_GETPARAM_UNDEFINED_PARAM:
        printf ("Necessary parameter not set!\n");
        break;

    case DLB_GETPARAM_NO_VALUE:
        printf ("Parameter needs value!\n");
        break;

    case DLB_GETPARAM_OK:
        /* no real error */
        break;

    case DLB_GETPARAM_ALREADY_GOT:
        printf ("This parameter was already parsed. If you aren't using NON_WS, check your code.\n"
        "If you are using NON_WS, and your code is OK, this is just the same as DLB_GETPARAM_UNDEFINED_PARAM.");
        break;

    case DLB_GETPARAM_INVALID_RANGE:
        printf ("The given range for this parameter is not a valid range.");
        break;

    default:
        printf ("Other error (%i)!\n", err);
        break;
    }
}
#define MAX_STRING_LEN 1024

int parse_args( char *tool, int argc, char *argv[], pfstruct fst )
{
    /*make a handle for getparam*/
    DLB_GETPARAM_HANDLE hGetParam;
    int b_is_switch_on = 0;
    DLB_GETPARAM_RETVAL error_code ;
    double gp_value = 0;
    long int gp_value1 = 0;
    const char* gp_string;
    char channelconfig [MAX_STRING_LEN] = {0};
    unsigned long int getParamMemSize;
    unsigned long int *getParamMem;
    //int temp = 0;
    //int i;
    int k; /* stores the .wav header */
    char filein[MAX_STRING_LEN] = {0};
    //char *fileinput = filein;
    const char *OptionalInputFile = NULL;
    int channel = 0;
    //int stdoutFlag = 0;
    FILE *fp;
    FILE *OptionalInputFilep;
    struct WaveFile wf;
    short numchans;
    int numsamps;
    int samprate;
    short bitspersamp;
    int audio_type;
    short format_type; /* subtype for WAVEFORMATEX */
    short wavx;
    //double *dp = NULL;
    //unsigned long *ulp = NULL;

    /*-- default values for parameter flags --*/
    fst->top = 0;
    fst->bottom = 0;
    fst->allChannels = 1;/* use all channels by default */
    fst->channelMap = -1; /* no channel map specified */
    fst->stdoutFlag = 1;
    fst->dataOutputFile[0] = '\0';
    fst->binary_out = 0;
    fst->noSilence = 0;
    fst->nfft = 0;
    fst->navg = 0;
    fst->tool = tool;
    fst->blksz_tSet = 0;
    fst->blksz_sSet = 0;
    fst->minPowerSet = 0;
    fst->hopsize_set = 0;

    /* Allocate and open the command line parser and parse command line switches */
    dlb_getparam_mem_query(&getParamMemSize);
    getParamMem = malloc(getParamMemSize);
    if(getParamMem == NULL){
      return EXIT_FAILURE;
    }
    hGetParam = dlb_getparam_open(getParamMem);
    if(!hGetParam){
      free(getParamMem);
      return EXIT_FAILURE;
    }

    /* parse and store the arguments from the command line */
    error_code = dlb_getparam_parse(hGetParam, argc, (const char **)argv);
    if (error_code != DLB_GETPARAM_OK)
    {
      return error_code;
    }

    /* parsing for the -f switch (second input file for multiple freq tool) */
    error_code = dlb_getparam_maxlenstring(hGetParam, "f", &OptionalInputFile, MAX_STRING_LEN);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        if(strcmp(tool, "mult_freq_resp") == 0)
        {
           strcpy(fst->OptionInputFile, OptionalInputFile);
        }
      }
      else
      {
        error("The -f otpion is for Multiple Frequency Tool only. Use with other tools is prohibited.\n");
        return(1);
      }
    }

	/* parsing for the -thd_s i/e the nfft switch*/ /*type double as large values */
	error_code = dlb_getparam_double(hGetParam, "thr_s", &gp_value, 0,  31);

	if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
	{
		if (error_code == DLB_GETPARAM_OK)
		{
			if ((fst->thr_sSet == 1) || (fst->thr_dbSet == 1))
			{
				error("Illegal option combination: Threshold cannot be set twice. Choose -thr_s or thr_db. \n");
        		return(1);
			}
			fst->thr_sSet = 1;
			fst->sample_thr = (double) gp_value;
		}
		else
		{
			error("Parameter -thr_s not defined correctly. Define it as follows:- -thr_s <value> \n");
			return(1);
		}
	}


	/* parsing for the -thd_db i/e the nfft switch*/ /*type double as large values */
    error_code = dlb_getparam_double(hGetParam, "thr_db", &gp_value, -999,  0);

    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
	{
		if (error_code == DLB_GETPARAM_OK)
		{
			if ((fst->thr_sSet == 1) || (fst->thr_dbSet == 1))
			{
			    error("Illegal option combination: Threshold cannot be set twice. Choose -thr_s or thr_db. \n");
        	    return(1);
			}
		fst->thr_dbSet = 1;
		fst->block_thr = (double) gp_value;
		}
		else
		{
			error("Parameter -thr_db not defined correctly. Define it as follows:- -thr_db <value> \n");
			return(1);
		}
	}


    /* parsing for the -c switch */
    error_code = dlb_getparam_string(hGetParam, "c", &gp_string);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        strcpy(channelconfig,gp_string );
        if (strcmp(channelconfig, "a") == 0) /*IF -CA MODE IS SET*/
        {
          fst->allChannels = 1;
        }
        else /*parse the number following c*/
        {
          fst->allChannels = 0;
          channel= atoi (gp_string);
        }
      }
      else
      {
        error("Misuse if the -c parameter. Please use it as follows:- -c x. \n");
        return 1;
      }
    }

    /* parsing for the -to switch */
    error_code = dlb_getparam_maxlenstring(hGetParam, "to", &gp_string, MAX_STRING_LEN);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        fst->stdoutFlag = 0;
        strcpy(fst->dataOutputFile, gp_string);
      }
    }

    /* parsing for the -w (select binary output) */
    error_code = dlb_getparam_bool(hGetParam, "w", &b_is_switch_on);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
          if (b_is_switch_on)
          {
              fst->binary_out = 1;
          }
      }
      else
      {
    	  error("Binary output switch is not used correctly. Correct use is -w .\n");
          return 1;
      }
    }

    /* parsing for the -i switch */
    error_code = dlb_getparam_maxlenstring(hGetParam, "i", &gp_string, MAX_STRING_LEN);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        strcpy(filein,gp_string);
      }
    }
      else
    {
      fprintf(stderr, "Please specify the input file with the -i flag. Use -h for help.\n");
      print_usage();
      return error_code;
    }

    /* parsing for the -m switch */
    error_code = dlb_getparam_int(hGetParam, "m", &gp_value1, 0, 7);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        fst->channelMap= (short) gp_value1;
        fst->channelMap--;
      }
      else
      {
        error("Channel Map parameter defined incorrectly. Correct usage is -m <value> \n");
      }
    }

    /* parsing for the -window switch for the spectrogram and spectrum NFFT tool */
    error_code = dlb_getparam_int(hGetParam, "window", &gp_value1, 1, 6); 
    if((strcmp(tool, "spectrum_NFFT") == 0) || (strcmp(tool, "spectrogram") == 0) || (strcmp(tool, "mel_spectrum") == 0))
    {
        if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
        {
          if (error_code == DLB_GETPARAM_OK)
          {
            fst->windowtype= (short) gp_value1;
          }
          else
          {
            error("Window selected incorrectly. Correct usage is -window <value> \n");
            return(-1);
          }
        }
    }


    /* parsing for the -n i/e the nfft switch*/ /*type double as large values */
    error_code = dlb_getparam_double(hGetParam, "n", &gp_value, 0, 48000);
    if((strcmp(tool, "spectrum_NFFT") == 0) || (strcmp(tool, "spectrogram") == 0) || (strcmp(tool, "mel_spectrum") == 0))
    {
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          fst->nfft = (long) gp_value;
        }
        else
        {
          error("Parameter -n not defined correctly. Define it as follows:- -n <value> \n");
		  return(1);
        }
      }
	  else if (error_code == DLB_GETPARAM_UNDEFINED_PARAM)
	  {
		  error("Parameter -n not defined correctly. Define it as follows:- -n <value> \n");
		  return(1);
	  }
    }

    /* parsing for the -z switch */
    error_code = dlb_getparam_double(hGetParam, "z", &gp_value, 0, DBL_MAX);
    if(strcmp(tool, "spectrum_NFFT") == 0)
    {
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          fst->navg = (long) gp_value;
        }
        else
        {
          error("Parameter -z defined incorrectly. Correct usage is -z <value>\n");
        }
      }
    }

    /* parsing for the -stride switch */
    error_code = dlb_getparam_double(hGetParam, "stride", &gp_value, 1, DBL_MAX);
    if (strcmp(tool, "spectrogram") == 0|| (strcmp(tool, "mel_spectrum") == 0))
    {
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          fst->hopsize_set = 1;
          fst->stride = (double) gp_value;
        }
        else
        {
          error("Parameter -stride defined incorrectly. Correct usage is -stride <value>\n");
		  return(1);
        }
      }
    }

   /* parsing for the -blksz_s switch */
    error_code = dlb_getparam_double (hGetParam, "blksz_s", &gp_value, 1, DBL_MAX);
    if (strcmp(tool, "pwr_vs_time") == 0)
    {
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          if ((fst->blksz_tSet == 1) || (fst->blksz_sSet == 1))
          {
            error("Illegal option combination:Block size cannot be set twice. Choose -blksz_s or blksz_t. \n");
        	return(1);
          }
          //dp = NULL;
          //ulp = &fst->blksz_s;
          fst->blksz_sSet = 1;
          fst->blksz_s = (unsigned long) gp_value;
        }
        else
        {
          error("parameter -blksz_s not set properly. use -h to get correct usage\n");
		  return(1);
        }
      }
    }

    /* parsing for the -blksz_t switch */
    error_code = dlb_getparam_double(hGetParam, "blksz_t", &gp_value, 1, DBL_MAX );
    if (strcmp(tool, "pwr_vs_time") == 0)
    {
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          if ((fst->blksz_tSet == 1) || (fst->blksz_sSet == 1))
          {
            error("Illegal option combination:Block size cannot be set twice. Choose -blksz_s or blksz_t. \n");
        	return(1);
          }
          //dp = &fst->blksz_t;
          //ulp = NULL;
          fst->blksz_tSet = 1;
          fst->blksz_t = (double) gp_value;
        }
        else
        {
          error("parameter -blksz_t not set properly. use -h to get correct usage\n");
		  return(1);
        }
      }
    }

    /* parsing the -powermin switch */
    if ((strcmp(tool, "pwr_vs_time") == 0) || (strcmp(tool, "spectrum_NFFT") == 0)|| (strcmp(tool, "spectrogram") == 0) || (strcmp(tool, "spectrum_avg") == 0) || (strcmp(tool, "thd_vs_freq") == 0) || (strcmp(tool, "freq_resp") == 0))
    {
      error_code = dlb_getparam_double(hGetParam, "powermin", &gp_value, -192, 0 ); /* values according to the min power formula.*/
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          //dp = &fst->minPower; /*Is this needed??*/
          fst->minPowerSet = 1;
          fst->minPower= (double) gp_value;
        }
        else
        {
          fprintf(stderr, "-powermin switch not set properly. please use it as -powermin -<value>.\n");
          print_usage();
          return error_code;
        }
      }
    }



    if (strcmp(tool, "amp_vs_time") == 0)
    {
      /* parsing for the -xmin switch */
      error_code = dlb_getparam_double(hGetParam, "xmin", &gp_value, DBL_MIN, DBL_MAX);
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
    	{
    	  fst->xminSet = 1;
    	  fst->xmin=(double) gp_value;
    	}
    	else
    	{
    	  error("Parameter -xmin not defined correctly. Correct usage is -xmin <value> or -xmin<value> ./n");
    	}
      }
      /* parsing for the -xmax switch */
      error_code = dlb_getparam_double(hGetParam, "xmax", &gp_value, DBL_MIN, DBL_MAX);
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          fst->xmaxSet = 1;
          fst->xmax=(double) gp_value;
        }
        else
        {
          error("Parameter -xmax not defined correctly. Correct usage is -xmax <value> or -xmax<value> ./n");
        }
      }
    }


    if (strcmp(tool, "res_envelope") == 0)
    {
      /* parsing for the -Top switch */
      error_code =  dlb_getparam_bool(hGetParam, "Top", &b_is_switch_on);
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          if (b_is_switch_on ==1)
          {
	        fst->top = 1;
            fst->bottom = 0;
          }
        }
      }

      /* parsing for the -Bottom switch */
      error_code = dlb_getparam_bool(hGetParam, "Bottom", &b_is_switch_on);
      if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
      {
        if (error_code == DLB_GETPARAM_OK)
        {
          if (b_is_switch_on ==1)
          {
	        fst->bottom = 1;
            fst->top= 0;
          }
        }
      }
    }

    /* parsing for the -h switch */
    error_code = dlb_getparam_bool(hGetParam, "h", &b_is_switch_on);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        if (b_is_switch_on == 1)
        {
    	  print_usage();
    	  free(getParamMem);
    	  return (1);
        }
      }
    }

    /* parsing for the -s (silence stripping) switch */
    error_code = dlb_getparam_bool(hGetParam, "s", &b_is_switch_on);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        if (b_is_switch_on == 1)
        {
          fst->noSilence = 1;
        }
      }
      else
      {
    	  error("Silence stripping parameter not defined correctly. Correct use is -s .\n");
      }
    }

    /* parsing for the -t switch */
    error_code = dlb_getparam_bool(hGetParam, "t", &b_is_switch_on);
    if (error_code != DLB_GETPARAM_UNDEFINED_PARAM)
    {
      if (error_code == DLB_GETPARAM_OK)
      {
        if (b_is_switch_on == 1)
        {
          fst->stdoutFlag = 1;
          //stdoutFlag = 1;
        }
      }
    }

    /* any switches given that we don't know? */
    if (dlb_getparam_left(hGetParam) != 0)
    {
		error("an unrecognised command was given.\n");
        print_usage();
    /* corresponding error message is automatically given by dlb_getparam_usage_show(1) */
		return (-99);
    }
    /*-- checking if res envelope options where set correctly --*/
    if (strcmp(tool, "res_envelope") == 0)
    {
      if ((fst->top == 0) && (fst->bottom == 0))
      {
        print_usage();
        return (-14);
      }

      if ((fst->top == 1) && (fst->bottom == 1))
      {
        error("\nError: Either top or bottom can be specified, Not both \n\n");
        print_usage();
        return (-15);
      }
    }

    if (filein == NULL)
    {
/*#ifdef WIN32
      if (GetOpenFileName(&ofn)==TRUE)
      {
        fileinput = ofn.lpstrFile;
        fst->graphFlag = 1;
        fst->stdoutFlag = 0;
      }
      else
      {
#endif*/
      print_usage();
      return (-5);

/*#ifdef WIN32
    }
#endif*/
    }

    fp = fopen(filein, "r");

    /*-- Checking if exists or if can be opened --*/
    if (fp == NULL)
    {
      error("Error: Couldn't open file: %s\n\n", filein);
      //print_usage();
      return (-6);
    }

    /*-- Checking if file size 0 or checking if file is empty --*/
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) < 1)
    {
      error("Input file empty\n");
      return (-7);
    }

    fclose(fp);

/*Now copy the name of the input file into the structure*/
    if(strcmp(tool, "res_envelope") == 0)
    {
	  strcpy(fst->filename, filein);
    }
    else /*If the file is a .wav file, read the wav header and then copy input file name into the structure*/
    {
      k = read_wav_header(&wf, filein, &numchans, &numsamps, &samprate, &bitspersamp, &wavx, NULL, NULL, &audio_type, &format_type);

      if (k != NO_ERR)
      {
        error("Input file not a WAV file\n");
        return (-8);
      }

#if 0
        debug( "ch: %d samples: %d samprate: %d bits: %d\n",
                numchans, numsamps, samprate, bitspersamp );
#endif

      fst->header_size = ftell(wf.file);

      //close_wav( &wf );

      if (channel >= numchans || channel < 0)
      {
        error("Invalid channel requested\n");
        return (-9);
      }

      if (!((strcmp(tool, "pwr_vs_time") == 0) || (strcmp(tool, "amp_vs_time") == 0) || (strcmp(tool, "mult_freq_resp") == 0)))
      {
        if (!(samprate == 32000 || samprate == 44100 || samprate == 48000))
        {
          error("Invalid sampling Frequency (%ld)\nAllowed Sampling Frequencies: 32kHz, 44.1kHz, 48kHz\n", samprate);
          return (-10);
        }
      }

      switch (bitspersamp)
      {
            case 16:
            case 18:
            case 20:
            case 24:
            case 32:
                break;
            default:
                error("Invalid bits per sample (%d)\n", bitspersamp);
                return (-11);
                break;
      }

     /*Special treatement for the mult freq resp as it takes 2 input files*/
     if (strcmp(tool, "mult_freq_resp") == 0)
     {
       if (fst->OptionInputFile == NULL)
       {
         print_usage();
         return (-12);
        }
        OptionalInputFilep = fopen(fst->OptionInputFile, "r");
        if (OptionalInputFilep == NULL)
        {
          error("Couldn't open file with list of frequencies.\n");
          error("Please use the -f option to input a file that contains the list of frequencies.\n\n");
          return (-13);
        }
        fseek(OptionalInputFilep, 0, SEEK_END);
        if (ftell(OptionalInputFilep) < 1)
        {
          error("%s Input file empty :\n", fst->OptionInputFile);
          print_usage();
          return (-14);
        }
        fclose(OptionalInputFilep);
      }

      strcpy(fst->filename, filein);
      fst->fp = wf.file;
      fst->channel = channel;
      fst->channels = numchans;
      fst->size = numsamps;
      fst->fs = samprate;
      fst->bitspersamp = bitspersamp;
      fst->position = 0;
     //	this assumes they aren't packed tighter than bytes
      //if (bitspersamp <= 16)
      //    fst->bytespersamp = 2;
      //else if (bitspersamp == 32)
      //    fst->bytespersamp = 4;
      //else fst->bytespersamp = 3;

	  switch(bitspersamp)
	  {
	      case 16:
			    fst->bytespersamp = 2;
				break;
		  case 24:
			    fst->bytespersamp = 3;
				break;
		  case 32:
			    fst->bytespersamp = 4;
				break;
		  default:
    	        error("Invalid wave file format. please use only 16,24,32 bit files.");
    	        return(-44);
    	        break;
	  }

      fst->audio_type = audio_type;
      fst->format_tag = format_type; /*write the format type in the fst*/
    }/*end of else loop for reading .wav file for reading .wav header*/

    free (getParamMem);
    return (0);
}

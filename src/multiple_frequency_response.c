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
;	File:	multiple_frequency_response.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fio.h"
#include "mult_freq_resp.h"
#include "wavelib.h"
#include "fft_avg.h"
#include "parse_args.h"
#include "Utilities.h"
#include "debug.h"

/* round double array elements to the nearest int */
void matlab_round( double *input_array, int input_array_size, int *output_array )
{
    int i;
    for (i = 0; i < input_array_size; i++)
    {
        if (input_array[i] >= 0)
        {
            output_array[i] = (int) (input_array[i] + 0.5);
        }
        else
        {
            output_array[i] = (int) (input_array[i] - 0.5);
        }
    }
}

int multiple_frequency_response(pfstruct pfs, int dnr)
{	
	char line[20];
	int line_counter=0; 
	int nc=0; 
	int num_of_freq_points=0; 
	int y=0; /*counter for ordering the frequencies and making sure no duplicates remain*/
	int i=0; /*--"--*/
	int rounded_freq_points_p_size=0; 
	int search_start=0; 
	int search_end=0;
	int index=0; 
	int corresponding_index=0;
	int holder = 0;
	int *rounded_freq_points_p = NULL;

	double n, max_value, freq_points = 0.0; 
	double *freq_points_p, *po = NULL;

	char *format = "%f,\t%3.2f\n"; 

	FILE *fr;
	dnr = dnr;
	n = (pfs->fs / 2) + 1; 

	po = (double *) calloc((size_t) n, sizeof(double));

	/*-- returns the size of output array--*/
	/*-- [points] = fft_avg(fstruct,modes); --*/
	if ((nc = fft_avg(pfs, po, (int) n)) == 0)
	  {
	    error("Bad Input File... \n");
	    return -100;
	  }

	  /*-- Getting the line count of the frequency text input file --*/
	  /*i.e the number of frequency points to be plotted */
	  num_of_freq_points = getFileLineCount(pfs->OptionInputFile);

	  /*-- Allocating memory for freq points array --*/
	  freq_points_p = (double *) malloc(sizeof(double) * num_of_freq_points);

	  fr = fopen(pfs->OptionInputFile, "rt");

	  line_counter = 0;

	    while (fgets(line, 20, fr) != NULL)
	    {
	      if (sscanf(line, "%lf", &freq_points) != EOF)
	      {
	        freq_points_p[line_counter] = freq_points;

	        debug("\n freq_points[%i] : %5.1f \n", line_counter, freq_points_p[line_counter]);

	        line_counter++;
	      }
	    }
	    
            fclose(fr); 

	    /*-- freq_points = round(freq_points); --*/
	    /*-- Allocating memory for rounded freq points array --*/
	    rounded_freq_points_p_size = line_counter;
	    rounded_freq_points_p = (int *) calloc(rounded_freq_points_p_size, sizeof(int));

	    /*call the rounding function to round double array elements to the nearest int */
	    matlab_round(freq_points_p, rounded_freq_points_p_size, rounded_freq_points_p);

	    for (i = 0; i < rounded_freq_points_p_size; i++)
	    {
	      debug("\n rounded_freq_points[%i] : %i \n", i, rounded_freq_points_p[i]);
	    }

	    /*-- got to put code for ordering frequencies --*/
	    /*-- freq_points = sortrows(freq_points); --*/
	    for (i = 0; i < rounded_freq_points_p_size; i++)
	    {
	        for (y = 0; y < rounded_freq_points_p_size - 1; y++)
	        {
	            if (rounded_freq_points_p[y] > rounded_freq_points_p[y + 1])
	            {
	             holder = rounded_freq_points_p[y + 1];
	             rounded_freq_points_p[y + 1] = rounded_freq_points_p[y];
	             rounded_freq_points_p[y] = holder;
	            }
	        } 
	    }

	    /*-- checking duplicates --*/
	    /*-- notdup = ([diff(freq_points(:)); 1] > 0); --*/
	    y = 0;
	    for (i = 1; i < rounded_freq_points_p_size; i++)
	    {
	        if (rounded_freq_points_p[y] != rounded_freq_points_p[i])
	        {
	            rounded_freq_points_p[y + 1] = rounded_freq_points_p[i];
	            y++;
	        }
	    }

	    rounded_freq_points_p_size = y + 1;
	    num_of_freq_points = rounded_freq_points_p_size;

	    for (i = 0; i < rounded_freq_points_p_size; i++)
	    {
	      debug("\n rounded_noDUP_freq_points[%i] : %i \n", i, rounded_freq_points_p[i]);
	    }

	    for (line_counter = 0; line_counter < num_of_freq_points; line_counter++)
	    {

	      if ((rounded_freq_points_p[line_counter] - 4) < 0)
	      {
	        search_start = 0; 
	      }
	      else
	      {
	        search_start = rounded_freq_points_p[line_counter] - 4; 
	      }

	      if ((rounded_freq_points_p[line_counter] + 4) > rounded_freq_points_p[num_of_freq_points - 1])
	      {
	        search_end = rounded_freq_points_p[num_of_freq_points - 1];
	      }
	      else
	      {
	        search_end = rounded_freq_points_p[line_counter] + 4;
	      }

	      /*-- Taking care of the difference in array indexing from C and Matlab --*/
	      if (search_start == 0)
	      {
	        search_start = 1;
	      }

	      if ((search_start - 1) < (int) n)
	      {
	        /*-- Initial max value --*/
	        max_value = po[search_start - 1];
	        index = search_start;
	        corresponding_index = search_start - 1;
	        
                /*-- Searching for max value --*/
	        while (index != search_end)
	        {
	          if (po[index] > max_value)
	            {
	            max_value = po[index];
	            corresponding_index = index;
	            }
	          index++;
	         }

            /*printing the results */
            check( sdf_writer_add_data_float_float(pfs->sdf_out,
                                                   (float) corresponding_index,
                                                   (float) max_value,
                                                   format) );
	      }/*end of if loop to search the max value and print the results*/

	    } /*end of the for loop for going through the frequencies*/

	free(freq_points_p); 
	free(po); 
	free(rounded_freq_points_p); 
	return 0;
}



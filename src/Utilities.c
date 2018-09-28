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
;	File:	Utilities.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

extern int getFileLineCount(char * );
extern double min_d( double , double  );
extern double max_d( double , double  );
extern int imin( int , int  );
extern long long imin64( long long , long long  );
extern int imax( int , int );
extern int sign( double  );


int getFileLineCount(char * fileName)
{
	int ch, prev, line_counter;
	
	FILE *file_handle;
	
	/*-- open the file to count line or number of rows --*/
	file_handle = fopen (fileName, "r");  
	
	if ( file_handle )
	{
		line_counter = 0;
		ch = '\n';	
		prev = '\n';	
		
		while ( ( ch = fgetc(file_handle) ) != EOF )
		{	
			if ( (ch == '\n') && (prev != '\n') )
				line_counter++;
			prev = ch;				
		}
		fclose(file_handle);
	
		if ( prev != '\n' )
			++line_counter;
			
		return line_counter;	
	
	} else {
		error("Could not count number of lines in File : %s \n" , fileName );
		return -15;	
	}

}

/*double round( double d )
{
    if (d >= 0)
    {
        d = (double) ((int) (d + 0.5));
    }
    else
    {
        d = (double) ((int) (d - 0.5));
    }
    return (d);
}
*/

double min_d( double d1, double d2 )
{
    if (d1 < d2)
        return (d1);
    else return (d2);
}

double max_d( double d1, double d2 )
{
    if (d1 > d2)
        return (d1);
    else return (d2);
}

int imin( int i1, int i2 )
{
    if (i1 < i2)
        return (i1);
    else return (i2);
}

long long imin64( long long i1, long long i2 )
{
    if (i1 < i2)
        return (i1);
    else return (i2);
}

int imax( int i1, int i2 )
{
    if (i1 > i2)
        return (i1);
    else return (i2);
}

int sign( double value )
{
    if (value == 0.0)
    {
        return (0);
    }
    else if (value > 0.0)
    {
        return (1);
    }
    else if (value < 0.0)
    {
        return (-1);
    }
    else
    {

        debug("Cannot find the correct sign for value (In sign function)\n");
        return (2);

    }
}


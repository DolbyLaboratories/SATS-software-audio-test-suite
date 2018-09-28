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
;	File:	getparam.c
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

/*! \file
    \brief Minimal Commandline Parser Implementation
*/

#include "getparam.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef DLB_GETPARAM_DOUBLE
# include <math.h>
#endif
#include <assert.h>

/*! \brief Struct holding a parameter and its value as string */
struct GETPARAM_SWITCH
{
  const char *name;      /*!< Parameter name */
  const char *value;     /*!< Parameter value stored as string */
  int got;               /*!< Indicates if the parameter has been accessed by the user */
};

#ifdef DLB_GETPARAM_HELP
/*! \brief Struct holding a parameter and its help text */
struct GETPARAM_HELP
{
  const char *name;  /*!< Parameter name */
  const char *text;  /*!< Help text of the parameter */
};
#endif

/*! \brief This struct holds all information that needs to persist in between calls to the GetParam functions */
struct GETPARAM_MEM
{
#ifdef DLB_GETPARAM_TIMESTAMP
  FILE *pfile;                                                        /*!< File handle to the runtime command file */
  char cmdline[DLB_GETPARAM_MAXLEN + DLB_GETPARAM_MAXTSLEN * 2 + 3];  /*!< String containing the last command line read from the
                                                                           command file */
  const char *_argv[DLB_GETPARAM_MAX_SWITCHES * 2 + 1];               /*!< Array of strings to emulate argv when reading from command
                                                                           file */
  int b_stop_line;                                                    /*!< Flag for DLB_GETPARAM_STOP_LINE detected */
#endif
#ifdef DLB_GETPARAM_TEXTFILE
  FILE *pfile;                                                        /*!< File handle to the runtime command file */
  char cmdline[DLB_GETPARAM_MAXLEN];                                  /*!< String containing the last command line read from the
                                                                           command file */
  const char *_argv[DLB_GETPARAM_MAX_SWITCHES * 2 + 1];               /*!< Array of strings to emulate argv when reading from command
                                                                           file */
#endif
#ifdef DLB_GETPARAM_HELP
  int help_count;                                                     /*!< Number of help texts in the help array */
  struct GETPARAM_HELP help[DLB_GETPARAM_MAX_SWITCHES];               /*!< Array holding all parameters and their helptext set so far */
#endif
  int switches_count;                                                 /*!< Number of command line switches found */
  struct GETPARAM_SWITCH switches[DLB_GETPARAM_MAX_SWITCHES];         /*!< Array with all command line parameters and their values */
};

/*      Local Function Declaration           */

/* Simple error handler */
static DLB_GETPARAM_RETVAL handleError(DLB_GETPARAM_HANDLE hGetParam
                                       ,int reset_switches
                                       ,DLB_GETPARAM_RETVAL errc);

#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
/* get next line which is not a comment and skip leading whitespace */
static char* skipfgets(char *str
                      ,int  num
                      ,FILE *stream);

/* Counts the number of leading whitespace characters in the string */
static int countSpaces(const char *pch);
#endif

#ifdef DLB_GETPARAM_TIMESTAMP
/* Converts a string into a timestamp */
static DLB_GETPARAM_RETVAL StringToTimestamp(char *str, DLB_TIMESTAMP *stamp);

static DLB_GETPARAM_RETVAL parseTextfileLine(DLB_GETPARAM_HANDLE hGetParam
                                             ,char               *pch
                                             ,int                *argc
                                             ,DLB_TIMESTAMP      *from
                                             ,DLB_TIMESTAMP      *next);
#endif

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_parse(DLB_GETPARAM_HANDLE hGetParam
                                      ,int                 argc
                                      ,const char          *argv[])
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;

    int have_switch;
    int nr;
    int i;

    hGetParam->switches_count = 0;
#ifdef DLB_GETPARAM_HELP
    hGetParam->help_count = 0;
#endif

    /* generate switches array */
    have_switch = 0;
    nr = -1;
    for (i = 1; i < argc; i++)
    {
        /* switch starts with '-' and does not continue with 0..9 */
        if (( argv[i][0] == '-') && (argv[i][1] < '0' || argv[i][1] > '9'))
        {
            /* switch found, saving name */
            nr++;
            if (nr >= DLB_GETPARAM_MAX_SWITCHES)
            {
                /* switches array is full */
                return handleError(hGetParam, 1, DLB_GETPARAM_OUT_OF_MEMORY);
            }
            hGetParam->switches[nr].name = argv[i]+1;
            hGetParam->switches[nr].value = NULL;   /* maybe switch without value */
            hGetParam->switches[nr].got = 0;
            have_switch = 1;
        }
        else if (have_switch)
        {
            /* already have switch name, here comes value */
            hGetParam->switches[nr].value = argv[i];
            have_switch = 0;
        }
        else {
            /* value without preceding switch */
            return handleError(hGetParam, 1, DLB_GETPARAM_SYNTAX_ERROR);
        }
    }
    hGetParam->switches_count = nr + 1;

    return status;
}

#ifdef DLB_GETPARAM_TEXTFILE
/* untested code */
DLB_GETPARAM_RETVAL
dlb_getparam_parse_textfile(DLB_GETPARAM_HANDLE hGetParam
                           ,const char          *cmdfile
                           )
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;

    int argc;
    char* pch;

#ifdef DLB_GETPARAM_HELP
    hGetParam->help_count = 0;
#endif

    /* opening cmdfile as a text file to ensure correct linefeed handling; this way, all line feeds become unix-like '\n' */
    if (hGetParam->pfile == NULL && (hGetParam->pfile = fopen(cmdfile, "r")) == NULL)
    {
        /* file was neither open nor could be opened */
        printf("foerr");
        return handleError(hGetParam, 1, DLB_GETPARAM_ERROR_CMDFILEOPEN);
    }

    /* adding argv[0] to be equal to c standard */
    hGetParam->_argv[0] = NULL;

    /* file opened correctly, reading one line to cmdline */
    if (skipfgets(hGetParam->cmdline, DLB_GETPARAM_MAXLEN, hGetParam->pfile) == NULL)
    {
        /* EOF */
        return handleError(hGetParam, 1, DLB_GETPARAM_EOF);
    }

    /* check line length */
    pch = hGetParam->cmdline + strlen(hGetParam->cmdline);
    if ((! feof(hGetParam->pfile)) && *(pch-1) != '\n')
    {
        /* line too long */
        return handleError(hGetParam, 1, DLB_GETPARAM_OUT_OF_MEMORY);
    }
    /* read arguments */
    pch = hGetParam->cmdline;
    argc = 1;
    /* find out the right pointers for each argv */
    while (pch != NULL)
    {
        if (argc > 1)
        {
            /* null-terminating each argument (replacing space) */
            *pch = 0;
            pch++;
        }
        /* skip additional unneeded spaces */
        pch += countSpaces(pch);
        if (*pch == '\n' || *pch == '\0')
        {
            /* end of the line */
            break;
        }
        if (argc >= DLB_GETPARAM_MAX_SWITCHES * 2)
        {
            /* array is full */
            return DLB_GETPARAM_OUT_OF_MEMORY;
        }
        /* everything ok, save pointer */
        hGetParam->_argv[argc] = pch;

        argc++;
        /* seek for next space */
        pch = strpbrk(pch, " \t\n");
    }

    if (status != DLB_GETPARAM_OK)
    {
        return handleError(hGetParam, 1, status);
    }

    if (argc > 0)
    {
        /* hand the command line over to the parser */
        status = dlb_getparam_parse(hGetParam, argc, hGetParam->_argv);
    }
    else
    {
        status = DLB_GETPARAM_EOF;
    }

    return status;
}
#endif

#ifdef DLB_GETPARAM_TIMESTAMP
/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_parse_textfile(
                                               DLB_GETPARAM_HANDLE hGetParam
                                               ,const char         *cmdfile
                                               ,DLB_TIMESTAMP      *from
                                               ,DLB_TIMESTAMP      *next
                                               )
{
    /* in this function, _argv is filled with information and then handed over to dlb_getparam_parse
* this way, the same code is used to parse the switches; additionally, timestamps are parsed
*/
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;

    int argc = 0;
    char* pch;

#ifdef DLB_GETPARAM_HELP
    hGetParam->help_count = 0;
#endif

    /* opening cmdfile as a text file to ensure correct linefeed handling; this way, all line feeds become unix-like '\n' */
    if (hGetParam->pfile == NULL && (hGetParam->pfile = fopen(cmdfile, "r")) == NULL)
    {
        /* file was neither open nor could be opened */
        return handleError(hGetParam, 1, DLB_GETPARAM_ERROR_CMDFILEOPEN);
    }

    /* adding argv[0] to be equal to c standard */
    hGetParam->_argv[0] = NULL;

    /* Reset flag for DLB_GETPARAM_STOP_LINE */
    hGetParam->b_stop_line = 0;

    do
    {
        /* file opened correctly, reading one line to cmdline */
        if (skipfgets(hGetParam->cmdline, DLB_GETPARAM_MAXTSLEN+DLB_GETPARAM_MAXLEN+2, hGetParam->pfile) == NULL)
        {
            /* EOF */
            return handleError(hGetParam, 1, DLB_GETPARAM_EOF);
        }

        /* check line length */
        pch = hGetParam->cmdline + strlen(hGetParam->cmdline);
        if ((!feof(hGetParam->pfile)) && *(pch - 1) != '\n')
        {
            /* line too long */
            return handleError(hGetParam, 1, DLB_GETPARAM_OUT_OF_MEMORY);
        }

        status = parseTextfileLine(hGetParam, pch, &argc, from, next);

    } while(status == DLB_GETPARAM_STOP_LINE);

    if (status != DLB_GETPARAM_OK)
    {
        return handleError(hGetParam, 1, status);
    }

    if (argc > 0)
    {
        /* hand the command line over to the parser */
        status = dlb_getparam_parse(hGetParam, argc, hGetParam->_argv);
    }
    else
    {
        status = DLB_GETPARAM_EOF;
    }

    return status;
}

/*
 * Documentation resides in header file only
 */
int dlb_getparam_stop_request(DLB_GETPARAM_HANDLE hGetParam)
{
    return hGetParam->b_stop_line;
}

#endif /* DLB_GETPARAM_TIMESTAMP */
/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_mem_query(unsigned long int *externStaticSize)
{
    /* with 32 bit alignment */
    *externStaticSize =  ((sizeof(struct GETPARAM_MEM) + sizeof(unsigned long int) - 1) / sizeof(unsigned long int)) * sizeof(unsigned long int);
    return DLB_GETPARAM_OK;
}

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_HANDLE dlb_getparam_open(void *mem)
{
    DLB_GETPARAM_HANDLE hGetParam;

    hGetParam = (DLB_GETPARAM_HANDLE)mem;

    /* initialize with zeros */
    memset(hGetParam, 0, sizeof(struct GETPARAM_MEM));

    return hGetParam;
}

#ifdef DLB_GETPARAM_TIMESTAMP
/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_timestamp_convert(DLB_GETPARAM_HANDLE hGetParam
                                                  ,const DLB_TIMESTAMP *from
                                                  ,DLB_TIMESTAMP       *to
                                                  ,int                 sampleRate
                                                  ,int                 frameSize)
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;
    unsigned long int sample;

    /* Setting hGetParam to NULL: this parameter is depricated and shall not be used! */
    hGetParam = (DLB_GETPARAM_HANDLE)NULL;
    (void)hGetParam;

    assert(frameSize > 0);

    switch (from->format)
    {
    case DLB_TIMESTAMP_SAMPLE:
        if (from->value[1] != 0 || from->value[2] != 0 || from->value[3] != 0)
        {
            return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
        }
        sample = from->value[0];
        break;
    case DLB_TIMESTAMP_FRAME:
        if (from->value[2] != 0 || from->value[3] != 0)
        {
            return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
        }
        if (from->value[0] >= ULONG_MAX/frameSize - 1)
        {
            return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
        }
        sample = from->value[0] * frameSize + from->value[1] * frameSize / 8;
        break;
    case DLB_TIMESTAMP_HMS:
        if (from->value[0] >= ULONG_MAX / sampleRate / 3600 - 1)
        {
            return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
        }
        sample  = from->value[0];    /* hours */
        sample *= 60;
        sample += from->value[1];    /* minutes */
        sample *= 60;
        sample += from->value[2];    /* seconds */
        sample *= 100;
        sample += from->value[3];    /* 100th secs */
        sample *= sampleRate/100;
        break;
    default:
        return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
    }

    to->value[1] = to->value[2] = to->value[3] = 0;

    switch (to->format)
    {
    case DLB_TIMESTAMP_SAMPLE:
        to->value[0] = sample;
        break;
    case DLB_TIMESTAMP_FRAME:
        sample /= frameSize / 8;    /* short blocks */
        to->value[0] = sample / 8;
        to->value[1] = sample % 8;
        break;
    case DLB_TIMESTAMP_HMS:
        sample /= sampleRate / 100;
        to->value[3] = sample % (3600 * 100);
        sample /= 100;
        to->value[2] = sample % 3600;
        sample /= 60;
        to->value[1] = sample % 60;
        sample /= 60;
        to->value[0] = sample;
        break;
    default:
        return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
    }

    return status;
}
#endif
/*
 * Documentation resides in header file only
 */
int dlb_getparam_left(DLB_GETPARAM_HANDLE hGetParam)
{
    int i;
    int left = hGetParam->switches_count;
    for (i = 0; i < hGetParam->switches_count; i++)
    {
        if (hGetParam->switches[i].got)
        {
            left--;
        }
    }
    return left;
}

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_nextremaining(DLB_GETPARAM_HANDLE hGetParam, const char **name, const char **value)
{
    int i;
    for (i = 0; i < hGetParam->switches_count; i++)
    {
        if (!hGetParam->switches[i].got)
        {
            hGetParam->switches[i].got++;
            *name = hGetParam->switches[i].name;
            *value = hGetParam->switches[i].value;
            return DLB_GETPARAM_OK;
        }
    }
    return DLB_GETPARAM_UNDEFINED_PARAM;   /* indicates that no parameter remains unhandled */
}

/*
 * Documentation resides in header file only
 */
void dlb_getparam_finalize(DLB_GETPARAM_HANDLE hGetParam)
{
#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
    if (hGetParam->pfile != NULL) fclose(hGetParam->pfile);
    hGetParam->pfile = NULL;
#endif
    hGetParam->switches_count = -1;
}

/* dlb_getparam_string is always called to retrieve a switch
 * dlb_getparam_nextremaining is *not* using it
 */
DLB_GETPARAM_RETVAL dlb_getparam_string(DLB_GETPARAM_HANDLE hGetParam
                                       ,const char *name
                                       ,const char **value
#ifdef DLB_GETPARAM_HELP
                                       ,const char *helptext
#endif
                                      )
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_UNDEFINED_PARAM;   /* no parameter found so far */
    int i;
    int nr = -1;

    /* search for entry */
    for (i = 0; i < hGetParam->switches_count; i++)
    {
#ifndef DLB_GETPARAM_ALLOW_NON_WS
        if (strcmp(hGetParam->switches[i].name, name) == 0)
        {
#else
        if (strncmp(hGetParam->switches[i].name, name, strlen(name)) == 0)
        {
#endif
                /* entry found */
                if (hGetParam->switches[i].got)
                {
                    status = DLB_GETPARAM_ALREADY_GOT;
                }
                else
                {
                    status = DLB_GETPARAM_OK;
                    nr = i;
                    break;
                }
        }
    }

    if (status == DLB_GETPARAM_OK)
    {
        assert(nr >= 0);

        /* got entry */
        hGetParam->switches[nr].got++;
#ifdef DLB_GETPARAM_ALLOW_NON_WS
        if (strlen(name) == strlen(hGetParam->switches[i].name))
        {
#endif
            /* ALLOW_NON_WS inactive OR whitespace was used OR no value given */
            if ((hGetParam->switches[nr].value != NULL))
            {
                *value = hGetParam->switches[nr].value;
            }
            else
            {
                /* no value given */
                status = DLB_GETPARAM_NO_VALUE;
                *value = NULL;
            }
#ifdef DLB_GETPARAM_ALLOW_NON_WS
        }
        else
        {
            /* ALLOW_NON_WS active */
            if ((hGetParam->switches[nr].value != NULL))
            {
                /* must be empty! value was already given without whitespace */
                status = DLB_GETPARAM_SYNTAX_ERROR;
                *value = NULL;
            }
            else
            {
                *value = hGetParam->switches[nr].name + strlen(name);
            }
        }
#endif
    }
#ifdef DLB_GETPARAM_HELP
    if (hGetParam->help_count < DLB_GETPARAM_MAX_SWITCHES)
    {
        hGetParam->help[hGetParam->help_count].name = name;
        hGetParam->help[hGetParam->help_count].text = helptext;
        hGetParam->help_count++;
    }
#endif

    return status;
}

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_maxlenstring(DLB_GETPARAM_HANDLE hGetParam
                                             ,const char          *name
                                             ,const char          **value
                                             ,int                 maxlen
#ifdef DLB_GETPARAM_HELP
                                             ,const char          *helptext
#endif
                                             )
{

    DLB_GETPARAM_RETVAL status;

    status = dlb_getparam_string(hGetParam
                                ,name
                                ,value
#ifdef DLB_GETPARAM_HELP
                                ,helptext
#endif
                                );

    if (status == DLB_GETPARAM_OK)
    {
        if ((int)strlen(*value) >= maxlen)
        {
            status = DLB_GETPARAM_OUT_OF_MEMORY;
            *value = NULL;
        }
    }
    return status;
}

#if 0
/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL
dlb_getparam_pickstring(DLB_GETPARAM_HANDLE hGetParam
                       ,const char          *name
                       ,int                 *value
                       ,const char          *range
#ifdef DLB_GETPARAM_HELP
                       ,const char          *helptext
#endif
                       )
{
    DLB_GETPARAM_RETVAL status;

    char       ch;
    char       *ending;
    const char *strValue;
    const char *pch;
    const char *pElement = NULL;
    const char *pValue = NULL;
    long int   ival;

    status = dlb_getparam_string(hGetParam
                                ,name
                                ,&strValue
#ifdef DLB_GETPARAM_HELP
                                ,helptext
#endif
                                );

    if (status != DLB_GETPARAM_OK)
    {
        return status;
    }
    else
    {
        status = DLB_GETPARAM_OUT_OF_RANGE;   /* in case we find no element */
    }

    pch = range;
    while (pch != NULL)
    {
        pch += countSpaces(pch);   /* skip leading spaces */

        if (pElement == NULL)
        {
            pElement = pch;
            pValue = NULL;
            if (pElement[0] = '\0')
            {
                /* zero-string elements not possible */
                return DLB_GETPARAM_INVALID_RANGE;
            }
        }
        else
        {
            /* value getting is going on */
            pElement = NULL;
            pValue = pch;
            /* validating value is done later */
        }

        do
        {
            pch = strpbrk(pch, "=;\t ");
            if (pch == NULL)
            {
                ch = 0;
                break;
            }
            ch = *pch;
            *pch = '\0';   /* last string ends here */
            pch++;
        } while (ch == ' ' || ch == '\t');
        /* got separator (ch == ';' || ch == '=' || ch == '\0') */
        /* what about missing '=' ';' not asking for right one just toggling now */

        if (pValue != NULL)
        {
            /* we have the whole value now */
            ival = strtol(pValue, &ending, 0);   /* 0 means auto detect the base */
            if (ending == pValue)
            {
                /* nothing parsed */
                return DLB_GETPARAM_INVALID_RANGE;
            }
            if (*ending != '\0')
            {
                /* number has to be ended with \0 */
                return DLB_GETPARAM_INVALID_RANGE;
            }
            if (ival < INT_MIN || ival > INT_MAX)
            {
                return DLB_GETPARAM_INVALID_RANGE;
            }

            if (strcmp(pElement, strValue) == 0)
            {
                /* element is given one */
                *value = (int)ival;
                status = DLB_GETPARAM_OK;
            }
        }

        if (ch == '\0')
        {
            break;
        }
    }

    return status;
}
#endif

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_bool(DLB_GETPARAM_HANDLE hGetParam
                                     ,const char          *name
                                     ,int                 *bValue
#ifdef DLB_GETPARAM_HELP
                                     ,const char          *helptext
#endif
                                     )
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;
    const char *val;

    status = dlb_getparam_string(hGetParam
                                ,name
                                ,&val
#ifdef DLB_GETPARAM_HELP
                                ,helptext
#endif
                                );

    if (status == DLB_GETPARAM_NO_VALUE)
    {
        /* does not need value */
        status = DLB_GETPARAM_OK;
        *bValue = 1;
    }
    else if (status == DLB_GETPARAM_UNDEFINED_PARAM)
    {
        /* ok for bool values */
        status = DLB_GETPARAM_OK;
        *bValue = 0;
    }
    else
    {
        /* value given, not valid for boolean */
        status = DLB_GETPARAM_SYNTAX_ERROR;
    }

    return status;
}

/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_int(DLB_GETPARAM_HANDLE hGetParam
                                    ,const char *name
                                    ,long int *value
                                    ,long int min
                                    ,long int max
#ifdef DLB_GETPARAM_HELP
                                    ,const char* helptext
#endif
                                    )
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;
    const char *val = NULL;
    char *ending;
    long int ival;

    status = dlb_getparam_string(hGetParam
                                ,name
                                ,&val
#ifdef DLB_GETPARAM_HELP
                                ,helptext
#endif
                                );

    if (status != DLB_GETPARAM_OK)
    {
        return status;
    }

    ival = strtol(val, &ending, 0);   /* 0 means auto detect the base */
    if (ending == val)
    {
        /* nothing parsed */
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if (*ending != '\0')
    {
        /* number has to be ended with \0 */
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if (ival == LONG_MIN || ival == LONG_MAX)
    {
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if (min != 0 || max != 0)
    {
        /* no range limitation on min=max=0 */
        if (min >= max)
        {
            return DLB_GETPARAM_INVALID_RANGE;
        }
        if ((ival < min) || (ival > max))
        {
            return DLB_GETPARAM_OUT_OF_RANGE;
        }
    }

    *value = ival;
    return status;
}

#ifdef DLB_GETPARAM_DOUBLE
/*
 * Documentation resides in header file only
 */
DLB_GETPARAM_RETVAL dlb_getparam_double(DLB_GETPARAM_HANDLE hGetParam
                                       ,const char          *name
                                       ,double              *value
                                       ,double              min
                                       ,double              max
#ifdef DLB_GETPARAM_HELP
                                       ,const char*         helptext
#endif
                                       )
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;
    const char *val = NULL;
    char *ending;
    double dval;

    status = dlb_getparam_string(hGetParam
                                ,name
                                ,&val
#ifdef DLB_GETPARAM_HELP
                                ,helptext
#endif
                                );

    if (status != DLB_GETPARAM_OK)
    {
        return status;
    }

    dval = strtod(val, &ending);
    if (ending == val)
    {
        /* nothing parsed */
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if (*ending != '\0')
    {
        /* number has to be ended with \0 */
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if (dval == +HUGE_VAL || dval == -HUGE_VAL)
    {
        return DLB_GETPARAM_SYNTAX_ERROR;
    }
    if ((min != max)    /* no range limit on min=max */
        &&((dval < min) || (dval > max)))
    {
        return DLB_GETPARAM_OUT_OF_RANGE;
    }
    *value = dval;
    return status;
}
#endif /* DLB_GETPARAM_DOUBLE */
#ifdef DLB_GETPARAM_HELP
/*
 * Documentation resides in header file only
 */
void dlb_getparam_usage_show(DLB_GETPARAM_HANDLE hGetParam
                            ,int                 bShowUnknownParams)
{
    int i;

    if (bShowUnknownParams)
    {
        for (i = 0; i < hGetParam->switches_count; i++)
        {
            if (! hGetParam->switches[i].got)
            {
                fputs("Unknown or duplicated switch: -", stdout);
                fputs(hGetParam->switches[i].name, stdout);
                fputs("\n", stdout);
            }
        }
    }

    for (i = 0; i < hGetParam->help_count; i++)
    {
        fputs("-", stdout);
        fputs(hGetParam->help[i].name, stdout);
        fputs(" ", stdout);
        fputs(hGetParam->help[i].text, stdout);
        fputs("\n", stdout);
    }
}
#endif

/*--------------------------------------------*/
/*        LOCAL FUNCTION IMPLEMENTATION       */
/*--------------------------------------------*/
#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
/* skipfgets()
 * like fgets, but skips leading whitespace characters
 * and blank/comment lines
 */
static char* skipfgets(char *str
                      ,int  num
                      ,FILE *stream)
{
    int x;   /* char cannot be EOF */
    char *pch;

    if (num <= 0)
    {
        return NULL;
    }

    do
    {
        /* skip whitespace */
        do
        {
            x = fgetc(stream);   /* at least one char read here */
        } while (x == ' ' || x == '\t');

        /* skip comment lines */
        if (x == '#')
        {
            do
            {
                x = fgetc(stream);
            } while (x != EOF && x != '\n');   /* read to the end of line */
        }

    } while (x == '\n');   /* skip blank lines */

    if (x == EOF)
    {
        return NULL;
    }

    str[0] = x;   /* the first valid character */
    pch = fgets(str + 1, num - 1, stream);    /* read from second character to the end */

    if (pch != NULL)
    {
        /* wipe out comments at the end of a line */
        while (*pch != '\0')
        {
            if (*pch == '#')
            {
                *pch = '\n';   /* in some code, \n is used to identify the end of a line */
                pch++;
                while (*pch != '\0' && *pch != '\n')
                {
                    *pch = '\0';
                    pch++;
                }
                if (*pch != '\n')
                {
                    /* read to end of line; this is needed if a comment is longer than the buffer */
                    do
                    {
                        x = fgetc(stream);
                    } while (x != EOF && x != '\n');
                }
                break;
            }
            pch++;
        }
    }
    else
    {
        str[1] = '\0';
    }

    return str;
}

/* countSpaces()
 * count whitespace characters
 * used for skipping them in several cases
 */
static int countSpaces(const char *pch)
{
    int i = 0;

    while((pch[i]) == ' ' || (pch[i]) == '\t')
    {
        i++;
    }
    return i;
}
#endif
#ifdef DLB_GETPARAM_TIMESTAMP
  /*! \brief Parses a timestamp string and gives back a DLB_TIMESTAMP structure
   */
static DLB_GETPARAM_RETVAL StringToTimestamp(char          *str
                                            ,DLB_TIMESTAMP *stamp)
{
    int i = 0;
    unsigned long int tmp;

    stamp->format = DLB_TIMESTAMP_UNDEFINED;   /* undefined */
    stamp->value[0] = stamp->value[1] = stamp->value[2] = stamp->value[3] = 0;

    if (*str == '\n' || *str == '\0')
    {
        stamp->format = DLB_TIMESTAMP_UNDEFINED;
        return DLB_GETPARAM_EOF;
    }
    if (strncmp("EOF", str, 3) == 0)
    {
        if (str[3] == '\0')
        {
            stamp->format = DLB_TIMESTAMP_UNDEFINED;
            return DLB_GETPARAM_OUT_OF_MEMORY;
        }
        else if (str[3] != ' ' && str[3] != '\t')
        {
            stamp->format = DLB_TIMESTAMP_UNDEFINED;
            return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
        }
        stamp->format = DLB_TIMESTAMP_EOF;
        return DLB_GETPARAM_OK;
    }
    else
    {
        if (*str == 's')
        {
            stamp->format = DLB_TIMESTAMP_SAMPLE;
            str++;
        }

        while(*str != '\0')
        {
            if (*str >= '0' && *str <= '9')
            {
                /* number */
                if (i > 3)
                {
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                tmp = strtoul(str, &str, 0);
                stamp->value[i] = tmp;

                if (i == 3 && *(str - 3) != '.')
                {
                    /* hundredth seconds have to be given with 2 decimals */
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                if (stamp->value[i] == ULONG_MAX)
                {
                    /* no values we cannot represent */
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                i++;
            }
            else if (*str == ':')
            {
                /* colon */
                if (i == 0 || i > 2)
                {
                    /* only two colons */
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                str++;
            }
            else if (*str == '.')
            {
                /* point */
                if (i != 3)
                {
                    /* '.' only in seconds block */
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                str++;
            }
            else if (*str == ' ' || *str == '\t')
            {
                /* end of timestamp */
                if (i == 0)
                {
                    /* got no number at all */
                    stamp->format = DLB_TIMESTAMP_UNDEFINED;
                    return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                }
                if (stamp->format == DLB_TIMESTAMP_SAMPLE)
                {
                    if (i != 1)
                    {
                        /* only one number for SAMPLE format */
                        stamp->format = DLB_TIMESTAMP_UNDEFINED;
                        return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                    }
                }
                else if (i < 3)
                {
                    stamp->format = DLB_TIMESTAMP_FRAME;
                    if (stamp->value[1] > 7) {
                        /* only eight shortblocks per frame */
                        stamp->format = DLB_TIMESTAMP_UNDEFINED;
                        return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                    }
                }
                else
                {
                    stamp->format = DLB_TIMESTAMP_HMS;
                    if (stamp->value[1] > 59 || stamp->value[2] > 59)
                    {
                        /* only 60 minutes per hour / seconds per minute */
                        stamp->format = DLB_TIMESTAMP_UNDEFINED;
                        return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
                    }
                }
                return DLB_GETPARAM_OK;
            }
            else
            {
                /* unknown char */
                stamp->format = DLB_TIMESTAMP_UNDEFINED;
                return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
            }
        }
        /* end of number must not be \0 */
        stamp->format = DLB_TIMESTAMP_UNDEFINED;
        return DLB_GETPARAM_OUT_OF_MEMORY;
    }
}

static DLB_GETPARAM_RETVAL parseTextfileLine(DLB_GETPARAM_HANDLE hGetParam
                                            ,char                *pch
                                            ,int                 *argc
                                            ,DLB_TIMESTAMP       *from
                                            ,DLB_TIMESTAMP       *next)
{
    DLB_GETPARAM_RETVAL status = DLB_GETPARAM_OK;
    fpos_t position;

    /* read "current timestamp" */
    /* EOF is also catched here */
    status = StringToTimestamp(hGetParam->cmdline, from);
    if (status == DLB_GETPARAM_EOF)
    {
        from->value[0] = from->value[1] = from->value[2] = from->value[3] = 0;
        from->format = DLB_TIMESTAMP_UNDEFINED;
        next->value[0] = next->value[1] = next->value[2] = next->value[3] = 0;
        next->format = DLB_TIMESTAMP_UNDEFINED;
        return status;
    }
    else if (status != DLB_GETPARAM_OK)
    {
        return status;
    }

    /* read next timestamp in next line */
    pch++;
    /* position is saved before; will be read again when parsing next line */
    fgetpos(hGetParam->pfile, &position);
    if (skipfgets(pch, DLB_GETPARAM_MAXTSLEN + 1, hGetParam->pfile) == NULL)
    {
        /* EOF in next line */
        next->format = DLB_TIMESTAMP_EOF;
        next->value[0] = next->value[1] = next->value[2] = next->value[3] = 0;
        fsetpos(hGetParam->pfile, &position);
    }
    else
    {
        fsetpos(hGetParam->pfile, &position);
        status = StringToTimestamp(pch, next);
        if (status != DLB_GETPARAM_OK)
        {
            return status;
        }
    }

    /* read arguments */
    pch = hGetParam->cmdline;
    *argc = 0;   /* first argument is skipped (i.e. frame number) */
    /* find out the right pointers for each argv */
    while (pch != NULL)
    {
        if (*argc > 0)
        {
            /* null-terminating each argument (replacing space) */
            *pch = 0;
            pch++;
        }
        /* skip additional unneeded spaces */
        pch += countSpaces(pch);
        if (*pch == '\n' || *pch == '\0')
        {
            /* end of the line */
            break;
        }
        if (*argc >= DLB_GETPARAM_MAX_SWITCHES * 2)
        {
            /* array is full */
            return DLB_GETPARAM_OUT_OF_MEMORY;
        }
        /* everything ok, save pointer */
        if (*argc != 0)
        {
            /* first argument, "from timestamp", was already read */
            if (*argc == 1 && strncmp("STOP", pch, 4) == 0)
            {
                /* it's a STOP line; drop this line, no more info in here */
                pch += 4;
                pch += countSpaces(pch);
                if (*pch != '\n' && *pch != '\0')
                {
                    /* there must not be anymore information in STOP line*/
                    return DLB_GETPARAM_SYNTAX_ERROR;
                }
                hGetParam->b_stop_line = 1;
                return DLB_GETPARAM_STOP_LINE;
            }
            else if (from->format == DLB_TIMESTAMP_EOF)
            {
                /* line beginning with EOF has to end with STOP */
                return DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR;
            }
            hGetParam->_argv[*argc] = pch;
        }
        (*argc)++;
        /* seek for next space */
        pch = strpbrk(pch, " \t\n");
    }
    return status;
}
#endif

/* Local function which implements a simple error handling */
static DLB_GETPARAM_RETVAL handleError(DLB_GETPARAM_HANDLE  hGetParam
                                       ,int                 reset_switches
                                       ,DLB_GETPARAM_RETVAL errc)
{
    if (reset_switches)
    {
        /* information on command line not parseable => reset switches_count
         * usage information may be usable, will not reset help_count
        */
        hGetParam->switches_count = -1;
    }

    return errc;
}

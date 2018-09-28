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
;	File:	getparam.h
;
;	History:
;		13/06/18		Created		Author: Dolby		Version: 1.0
;***************************************************************************/

/** \file
        \defgroup dlb_clif Command line interface
        \brief Minimal Commandline Parser

        Typical Calling Sequence
        - call dlb_getparam_mem_query() to query the memory demand of the GetParam instance
        - allocate the necessary amount of memory
        - call dlb_getparam_open() to initialize one GetParam instance
        - call dlb_getparam_parse() for parsing commandline input
        - call dlb_getparam_\<string|bool|int...\> to retrieve switches
        - call dlb_getparam_bool() for switches that do not require a value
        - call dlb_getparam_left() to see if any unretrieved switches are present
        - at the end, call dlb_getparam_finalize() to clean up

        How to change parameters during runtime via a command text file
        - Instead (or in addition) to calling dlb_getparam_parse() you can call dlb_getparam_parse_textfile()
            which parses a text file for command line switches
        - Each line in the command file starts with a timestamp which indicates when the parameters should be applied
        - You can convert timestamps to different formats via dlb_getparam_timestamp_convert()
        - call dlb_getparam_parse_textfile() until you get an EOF

        Known Limitations:
        - Parameters with no leading "-" are not handled and confuses GetParam (e.g. "appl input.txt -v" is not possible)
        - Switches may not start with a number, e.g. "-90degree"

        Legacy Features:
        for backward compatibility, "ALLOW_NON_WS" was integrated. This allows switches that are not separated from their values
        with a space. It is enabled by #define DLB_GETPARAM_ALLOW_NON_WS. The switch will then take the value both ways. NOTE: This
        will constrain the differentiation between switches. Consider the following example:
            -aXXXXXX
            -abXXXXXX
            -abcXXXXXX
        with the "XXX" standing for the value and illustrating it is not separated at all from the switch. Obviously, "-a" can not
        take a value starting with "b", as well as "-ab" can not take a value starting with "c". The software engineer is expected
        to take care of this aspect and to define value ranges wisely. The GetParam-specific part is, you have to request the
        switches FROM LONG TO SHORT. This means, first use GetParam{String|Int|...} for getting "-abc", then "-ab", then "-a".
*/

#ifndef __DLB_GETPARAM_H
#define __DLB_GETPARAM_H

/* CONFIGURATION */
/* enable extensions */

/* features */
#if 0
/* do this externally */
#define DLB_GETPARAM_TIMESTAMP      /**< read command text files with timestamps */
#define DLB_GETPARAM_TEXTFILE
#define DLB_GETPARAM_HELP           /**< display usage information */
#define DLB_GETPARAM_DOUBLE         /**< parse switches with floating point value */
#define DLB_GETPARAM_ALLOW_NON_WS   /**< enable value-passing without space */
#endif

/* memory usage */
#ifndef DLB_GETPARAM_MAX_SWITCHES
#define DLB_GETPARAM_MAX_SWITCHES 24    /**< maximum switch count */
#endif
#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
#ifndef DLB_GETPARAM_MAXLEN
#define DLB_GETPARAM_MAXLEN    256     /**< commandline length in text file*/
#endif
#endif

#ifndef DLB_GETPARAM_MAX_SWITCHES
#error "Please define DLB_GETPARAM_MAX_SWITCHES!"
#endif

#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
#ifndef DLB_GETPARAM_MAXLEN
#error "Please define DLB_GETPARAM_MAXLEN!"
#endif
#endif

#if defined DLB_GETPARAM_TIMESTAMP && defined DLB_GETPARAM_TEXTFILE
#error "DLB_GETPARAM_TIMESTAMP and DLB_GETPARAM_TEXTFILE can't be defined together."
#endif

#ifdef DLB_GETPARAM_TIMESTAMP
#define DLB_GETPARAM_MAXTSLEN 12        /**< timestamp length, should be OK on no code changes */
#endif

/*     approx memory usage in bytes:
 *     DLB_GETPARAM_MAXLEN * sizeof(char)       <- DLB_GETPARAM_TIMESTAMP/TEXTFILE only
 *     +  (DLB_GETPARAM_MAX_SWITCHES * 2        <- DLB_GETPARAM_TIMESTAMP/TEXTFILE only
 *     +   DLB_GETPARAM_MAX_SWITCHES * 3
 *     +   DLB_GETPARAM_MAX_SWITCHES * 2)       <- DLB_GETPARAM_HELP only
 *     *  sizeof(void*)
 */

#include <stdio.h>

#ifdef    __cplusplus
extern "C"
{
#endif

    /** \brief dlb_getparam errorcodes
     */
    typedef enum
    {
        DLB_GETPARAM_OK                     = 0
       ,DLB_GETPARAM_SYNTAX_ERROR           = 1

        /* dlb_getparam_parse() specific errorcodes */
       ,DLB_GETPARAM_OUT_OF_MEMORY          = 2     /**< GetParam internal memory error, check defines, print out
                                                     *    "command line too long" */
#if defined DLB_GETPARAM_TIMESTAMP || defined DLB_GETPARAM_TEXTFILE
        /* dlb_getparam_parse_textfile() specific */
       ,DLB_GETPARAM_ERROR_CMDFILEOPEN      = 3
       ,DLB_GETPARAM_EOF                    = 4
#endif
#ifdef DLB_GETPARAM_TIMESTAMP
       ,DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR = 5
#endif
        /* dlb_getparam_<string|int|double>() specific errorcode */
       ,DLB_GETPARAM_UNDEFINED_PARAM        = 6     /**< parameter name was not found in commandline */
       ,DLB_GETPARAM_OUT_OF_RANGE           = 7     /**< parameter value was out of the specified range */
       ,DLB_GETPARAM_NO_VALUE               = 8     /**< parameter value expected but not found */
       ,DLB_GETPARAM_ALREADY_GOT            = 9     /**< all matching parameters were already read; with correct implementation this
                                                     *    means the switch was not given (DLB_GETPARAM_UNDEFINED_PARAM) */
       ,DLB_GETPARAM_INVALID_RANGE          = 10    /**< the range given is not a valid range */
       ,DLB_GETPARAM_STOP_LINE              = 999   /**< stop line; this code is only used internally */
    } DLB_GETPARAM_RETVAL;


    typedef struct GETPARAM_MEM* DLB_GETPARAM_HANDLE; /**< Create a handle type to the static memory of GetParam */


    /**
     * \brief This function opens an instance of GetParam
     *
     * The provided memory which is at least as bytes large as returned by dlb_getparam_mem_query() will be assigned to an handle
     * of the opened instance.
     * \return Handle to the opened instance, NULL in case of an error
     */
    DLB_GETPARAM_HANDLE
    dlb_getparam_open
        (void   *mem    /**< [in] pointer to allocated memory */
        );


    /**
     * \brief This function returns the amount of static memory needed for one instance
     *
     * The caller is expected to allocate the amount of memory returned and provide the dlb_getparam_open() function with a pointer
     * to this memory.
     *
     * \return DLB_GETPARAM_OK on success
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_mem_query
        (unsigned long int  *externStaticSize   /**< [out] memory needed in bytes */
        );

    /**
     * \brief The function parses the switches from argc/argv
     *
     * - has to be initialized with arguments from main()
     * - builds up index over switches
     *
     * \return error code:
     * - DLB_GETPARAM_OUT_OF_MEMORY
     * <br>    more switches than memory allocated, check DLB_GETPARAM_MAX_SWITCHES
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    general syntax error on command line
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_parse
        (DLB_GETPARAM_HANDLE hGetParam     /**< [in,out] handle of the getparam instance */
        ,int                 argc          /**< [in] argc from main() */
        ,const char          *argv[]       /**< [in] argv from main() */
        );

#ifdef DLB_GETPARAM_TIMESTAMP
    /** \brief timestamp formats
     */
    typedef enum
    {
        DLB_TIMESTAMP_UNDEFINED = -1,
        DLB_TIMESTAMP_FRAME     = 0,
        DLB_TIMESTAMP_HMS       = 1,
        DLB_TIMESTAMP_SAMPLE    = 2,
        DLB_TIMESTAMP_EOF       = 3
    } DLB_TIMESTAMP_FORMAT;

    /** \brief timestamp parsed from a command text file
     *
     * - there are four types of valid timestamps:
     *        - sNNNN
     *            <br>sample NNNN
     *        - FFF[:B]
     *            <br>frame FFF, optional shortblock B
     *        - HH:MM:SS[.SS]
     *            <br>hour HH, minute MM, second SS.SS
     *        - EOF
     *            <br>written in cap. letters; end of audiofile
     * - these integers (up to four per timestamp) are assigned to the four integer values
     * - the format is given by \<format\>
     * - timestamps can be converted with dlb_getparam_timestamp_convert()
     */
    typedef struct
    {
        unsigned long int       value[4];   /**< Up to 4 Integer values which have different meaning depending on the format used */
        DLB_TIMESTAMP_FORMAT    format;     /**< Format of the timestamp, defines how the value array will be interpreted */
    } DLB_TIMESTAMP;


    /**
     * \brief Convert timestamp from one timestamp format to another
     *
     * The destination format has to be set by destination->format.
     *
     * \return error code:
     * - DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR
     * <br>    timestamp is not in expected format or format unknown
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_timestamp_convert
        (DLB_GETPARAM_HANDLE hGetParam       /**< [in] Not used (should be NULL) */
        ,const DLB_TIMESTAMP *source         /**< [in] timestamp to convert */
        ,DLB_TIMESTAMP       *destination    /**< [in,out] converted timestamp */
        ,int                 sampleRate      /**< [in] sample rate for conversion */
        ,int                 frameSize       /**< [in] frame size for conversion */
        );


    /**
     * \brief This function parses the switches from a command text file
     *
     * - for a sample command text file, have a look at command.txt given with the sample frontend (demo.c)
     * - usually called if no command line arguments are present (argc==1)
     * - builds up index over switches it got from a single \a cmdfile line
     * - for reading multiple command lines from command text file, it has to be called for each line until DLB_GETPARAM_EOF or an
     *   error occurs.
     * - it depends on the calling application if switches shall be reseted if they are not present in the next line of the \a cmdfile
     * Be aware that if a STOP is in next line, this line will be skipped and the "from timestamp" of the next loop does not equal
     * actual "to timestamp"
     *
     * \return error code:
     * - DLB_GETPARAM_EOF\n
     *     end of \a cmdfile reached, no more switches to read
     * - DLB_GETPARAM_OUT_OF_MEMORY\n
     *     more switches than memory allocated, check DLB_GETPARAM_MAX_SWITCHES\n
     *     or: line in \a cmdfile too long / timestamp too long, check
     *                 DLB_GETPARAM_MAXLEN and DLB_GETPARAM_MAXTSLEN
     * - DLB_GETPARAM_ERROR_COMMANDFILEOPEN\n
     *     \a cmdfile could not be opened (does not exist, no perm., etc.)
     * - DLB_GETPARAM_SYNTAX_ERROR\n
     *     general syntax error on actual command line
     * - DLB_GETPARAM_TIMESTAMP_SYNTAX_ERROR\n
     *     syntax error on actual/next command line in current/next timestamp
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_parse_textfile
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *cmdfile    /**< [in] name of command text file */
        ,DLB_TIMESTAMP       *from       /**< [out] timestamp of actual line */
        ,DLB_TIMESTAMP       *next       /**< [out] timestamp of next line */
        );

    /**
     * \brief This function indicates whether a stop request has been found
     *
     * \return Flag indicating whether a stop was requested
     */
    int
    dlb_getparam_stop_request
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        );
#endif

#ifdef DLB_GETPARAM_TEXTFILE
    /**
     * \brief This function parses the switches from a command text file
     *
     * - usually called if no command line arguments are present (argc==1)
     * - builds up index over switches it got from a single \a cmdfile line
     * - for reading multiple command lines from command text file, it has to be called for each line until DLB_GETPARAM_EOF or an
     *   error occurs.
     * - it depends on the calling application if switches shall be reseted if they are not present in the next line of the \a cmdfile
     *
     * \return error code:
     * - DLB_GETPARAM_EOF\n
     *     end of \a cmdfile reached, no more switches to read
     * - DLB_GETPARAM_OUT_OF_MEMORY\n
     *     more switches than memory allocated, check DLB_GETPARAM_MAX_SWITCHES\n
     *     or: line in \a cmdfile too long, check
     *                 DLB_GETPARAM_MAXLEN
     * - DLB_GETPARAM_ERROR_COMMANDFILEOPEN\n
     *     \a cmdfile could not be opened (does not exist, no perm., etc.)
     * - DLB_GETPARAM_SYNTAX_ERROR\n
     *     general syntax error on actual command line
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_parse_textfile
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *cmdfile    /**< [in] name of command text file */
        );
#endif


#ifdef DLB_GETPARAM_HELP
    /**
     * \brief show usage information on console
     * - to get the complete help text, all switches have to be requested before
     */
    void
    dlb_getparam_usage_show
        (DLB_GETPARAM_HANDLE hGetParam              /**< [in,out] handle of the getparam instance */
        ,int                 bShowUnknownParams     /**< [in] set to 1 to show parameters you have not requested yet */
        );
#endif


    /**
     * \brief get number of parameters left on commandline
     *
     * - usually called to determine if there are switches that have not been recognized properly
     * - this is also a case for leaving main application and showing usage information
     *
     * \return number of switches that are on the command line but have not been requested yet by the main application
     */
    int
    dlb_getparam_left
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        );


    /**
     * \brief This function searches for command line parameters that have not been handled by the application.
     *
     * - Each call returns the name and the value of the next unhandled parameter unless all have been handled and returned.
     * - Use dlb_getparam_left() to find out the initial total number of unhandled parameters.
     *
     * \return DLB_GETPARAM_OK if an unhandled parameter has been returned. DLB_GETPARAM_UNDEFINED_PARAM if none are left.
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_nextremaining
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          **name      /**< [out] Pointer of the name string */
        ,const char          **value     /**< [out] Pointer of the corresponding value string */
        );
    /**
     * \brief clean everything up
     *
     * - after cleaning up, static memory of the instance is resetted and you can start again using an parse function
     */
    void
    dlb_getparam_finalize
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        );

    /**
     * \brief read a switch with string value from command line
     *
     * - this function gives you the pointer to the string value, you do not have to copy it unless you want to use it after the
     *   next call of GetParamParseTextfile() because this overwrites the internal buffer
     * - the helptext given is not copied; it has to stay in memory as long as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_UNDEFINED_PARAM
     * <br>    the switch was not found on commandline
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    no value was given to this string switch
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_string
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *name       /**< [in] switch name on commandline */
        ,const char          **value     /**< [out] pointer to according value */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**< [in] describing help text */
#endif
        );

    /**
     * \brief read a switch with string value from command line
     *
     * - same function as dlb_getparam_string, additionally checks string length
     * - this function is intended to use if you want to copy the value string; copying itself is not implemented because strcopies are
     *   avoided in this project due to ARM,... compatibility issues
     * <br>USAGE:
     * <br>after initializing a
     * <br>     char xy[LEN];
     * <br>and running
     * <br>     dlb_getparam_maxlenstring("name", &value, LEN);
     * <br>it is safe to do a
     * <br>     strcpy(xy, value);
     * - the helptext given is not copied; it has to stay in memory as long
     *        as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_UNDEFINED_PARAM
     * <br>    the switch was not found on commandline
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    no value was given to this string switch
     * - DLB_GETPARAM_OUT_OF_MEMORY
     * <br>    the value given is longer than \<maxlen\>; nothing returned
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_maxlenstring
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *name       /**< [in] switch name on commandline */
        ,const char          **value     /**< [out] pointer to according value */
        ,int                 maxlen      /**< [in] maximum length of **value */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**< [in] describing help text */
#endif
        );
#if 0
    /** TODO
     * \brief read a switch with string value from command line
     *
     * - same function as dlb_getparam_string, additionally checks string length
     * - this function is intended to use if you want to copy the value string;
     *        copying itself is not implemented because strcopies are avoided in
     *        this project due to ARM,... compatibility issues
     * <br>USAGE:
     * <br>after initializing a
     * <br>     char xy[LEN];
     * <br>and running
     * <br>     dlb_getparam_maxlenstring("name", &value, LEN);
     * <br>it is safe to do a
     * <br>     strcpy(xy, value);
     * - the helptext given is not copied; it has to stay in memory as long
     *        as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_UNDEFINED_PARAM
     * <br>    the switch was not found on commandline
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    no value was given to this string switch
     * - DLB_GETPARAM_OUT_OF_MEMORY
     * <br>    the value given is longer than \<maxlen\>; nothing returned
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_pickstring
        (DLB_GETPARAM_HANDLE hGetParam   /**<[in,out] handle of the getparam instance */
        ,const char          *name       /**<[in] switch name on commandline */
        ,int                 *value      /**<[out] pointer to according value */
        ,const char          *range      /**<[in] list of possible strings ';'-separated */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**<[in] describing help text */
#endif
        );
#endif
    /**
     * \brief read a switch without value (boolean) from command line
     *
     * - this function sets bValue=1 if the switch was found, bValue=0 else
     * - the helptext given is not copied; it has to stay in memory as long as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    value was given to this boolean
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_bool
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *name       /**< [in] switch name on commandline */
        ,int                 *bValue     /**< [out] according value */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**< [in] describing help text */
#endif
        );

    /**
     * \brief read a switch with integer value from command line
     *
     * - this function gives you the integer value
     * - a range can be specified through min, max values; set both to zero for disabling this feature
     * - the helptext given is not copied; it has to stay in memory as long as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_UNDEFINED_PARAM
     * <br>    the switch was not found on commandline
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    no value given, no valid integer or range check failed
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_int
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *name       /**< [in] switch name on commandline */
        ,long int            *value      /**< [out] according value */
        ,long int            min         /**< [in] lower range boundary */
        ,long int            max         /**< [in] upper range boundary */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**< [in] describing help text */
#endif
    );

#ifdef DLB_GETPARAM_DOUBLE
    /**
     * \brief read a switch with double value from command line
     *
     * - this function gives you the double value
     * - the helptext given is not copied; it has to stay in memory as long as you might call GetParamUsage()
     *
     * \return error code:
     * - DLB_GETPARAM_UNDEFINED_PARAM
     * <br>    the switch was not found on commandline
     * - DLB_GETPARAM_SYNTAX_ERROR
     * <br>    no value given, no valid double or range check failed
     */
    DLB_GETPARAM_RETVAL
    dlb_getparam_double
        (DLB_GETPARAM_HANDLE hGetParam   /**< [in,out] handle of the getparam instance */
        ,const char          *name       /**< [in] switch name on commandline */
        ,double              *value      /**< [out] according value */
        ,double              min         /**< [in] lower range boundary */
        ,double              max         /**< [in] upper range boundary */
#ifdef DLB_GETPARAM_HELP
        ,const char          *helptext   /**< [in] describing help text */
#endif
        );
#endif /* DLB_GETPARAM_DOUBLE */

#ifdef    __cplusplus
}
#endif
#endif /* __DLB_GETPARAM_H */

/* Copyright 2001 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

//#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

//#include "rsys/common.h"
//#include "rsys/error.h"

#include <windows.h>
#include <stdio.h>

#include "sdl2_hwnd.h"

using namespace Executor;
/*
 * returns number of characters written (or needed if bufp is NULL).
 */

static int
construct_command_line(char *bufp, int n_filenames, char **filenames,
                       const char *suffix)
{
    int retval;
    int i;
    int suffix_len;

    retval = 0;

    suffix_len = strlen(suffix);
    for(i = 0; i < n_filenames; ++i)
    {
        if(bufp)
            bufp += sprintf(bufp, "\"%s%s\"", *filenames, suffix);
        retval += strlen(*filenames) + suffix_len + 3; /* two quotes plus
							NUL or space */
        ++filenames;
        if(i < n_filenames - 1)
        {
            if(bufp)
                *bufp++ = ' '; /* overwrite the NUL */
        }
    }

    return retval;
}

static void
slash_replace(char *p)
{
    while(*p)
    {
        if(*p == '/')
            *p = '\\';
        ++p;
    }
}

enum
{
    noErr = 0,
    paramErr = -50
};

/* the suffixes are now added on the Mac side, so we don't do anything
   special here */

#define COMMAND_SUFFIX ""
#define DOCUMENT_SUFFIX ""

/*
 * NOTE: when you use Executor to launch a native application under win32,
 * the files involved all have to be .lnk files.  This is ugly, but the
 * native launching is just for TTS's benefit for now.
 */

int ROMlib_launch_native_app(int n_filenames, char **filenames)
{
    HINSTANCE hi;
    int buf_len;
    char *command_buf;
    char *args_buf;

    buf_len = strlen(filenames[0]) + sizeof COMMAND_SUFFIX;
    command_buf = (char *)alloca(buf_len);
    sprintf(command_buf, "%s%s", filenames[0], COMMAND_SUFFIX);

    buf_len = construct_command_line(NULL, n_filenames - 1, filenames + 1,
                                     DOCUMENT_SUFFIX);
    args_buf = (char *)alloca(buf_len);
    construct_command_line(args_buf, n_filenames - 1, filenames + 1,
                           DOCUMENT_SUFFIX);
    slash_replace(command_buf);
    slash_replace(args_buf);
    hi = ShellExecute(getMainSDLWindow(), NULL, command_buf, args_buf, NULL,
                      SW_SHOWNORMAL);

    return ((intptr_t)hi > 32 ? noErr : paramErr);
}

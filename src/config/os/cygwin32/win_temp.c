/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/error.h"

#include <windows.h>
#include <string.h>
#include "win_temp.h"

static void
normalize_directory_name(char *str)
{
    for(; *str; ++str)
        if(*str == '\\')
        {
            if(!str[1])
                *str = 0;
            else
                *str = '/';
        }
}

PUBLIC char *
win_temp(void)
{
    char buf[2048];
    DWORD len1;
    char *retval;

    len1 = GetTempPath(sizeof buf, buf);
    if(len1 < sizeof buf)
        retval = strdup(buf);
    else
    {
        DWORD len2;
        char *bufp;

        ++len1;
        bufp = alloca(len1);
        len2 = GetTempPath(len1, bufp);
        if(len2 < len1)
            retval = strdup(bufp);
        else
            retval = NULL;
    }
    normalize_directory_name(retval);
    return retval;
}

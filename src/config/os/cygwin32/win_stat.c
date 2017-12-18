/* Copyright 1997, 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* NOTE: Version 1.2 did things differently and didn't rely on luck to
   avoid collisions.  Unfortunately, it didn't have the property that
   the same directory would have the same name between runs of Executor */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include <windows.h>

#include <ctype.h>
#include <string.h>

#include "rsys/error.h"

#include "win_stat_private.h"
#include "win_stat.h"

/*
 * fold lower to uppper and fold slash to back-slash
 */

PRIVATE uint32_t
char_val(int c)
{
    uint32_t retval;

    retval = toupper(c);
    if(c == '/')
        retval = '\\';
    return retval;
}

/*
 * The original hash function was found to generate a unique value for each
 * of the 14,619 different filenames on uni:/win95 and uni:/d, but then,
 * without thinking it through, I added code to knock the top bit off so
 * that the value would always be positive.  This caused trouble, so I
 * added u to the mix so that a single bit difference will affect more
 * than a single bit.  I also changed the code that zaps the high bit to
 * do a mod of the largest lesser of two primes below 2^31.  The result
 * is what you see now, and it's been tested on 27,483 different files.
 */

PRIVATE uint32_t
hash_func(const char *p)
{
    uint32_t retval, u;
    int c;
    const char *orig_p;

    orig_p = p;
    retval = 0;
    u = 0;
    while((c = *p++))
    {
        int cv;
        int rotate;

        cv = char_val(c);

        retval = ((retval << 5) | (retval >> 27)) ^ cv;
        rotate = cv % 31;
        u = u ^ ((retval << rotate) | (retval >> (32 - rotate)));
    }

    retval ^= u;
    retval %= 2147482949; /* strip the top bit w/o ignoring it */

#if 0
  warning_trace_info ("p = '%s', retval = 0x%08x", orig_p, retval);
#endif

    return retval;
}

PUBLIC uint32_t
ino_from_name(const char *name)
{
    char fullname[2048];
    char *filenamep;
    uint32_t len;
    char *p;
    uint32_t retval;

    len = GetFullPathName(name, sizeof fullname, fullname, &filenamep);
    if(len <= sizeof fullname - 1)
        p = fullname;
    else
    {
        uint32_t len2;

        ++len;
        p = alloca(len);
        len2 = GetFullPathName(name, len, p, &filenamep);
        if(len2 >= len)
            p = (char *)name;
    }

    if(p[1] == ':' && p[2] == '\\' && !p[3])
        retval = 2;
    else
        retval = hash_func(p);
    return retval;
}

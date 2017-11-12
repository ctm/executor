/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/string.h"

#include "MemoryMgr.h"

using namespace Executor;

void Executor::str255_from_c_string(Str255 str255, const char *c_stringp)
{
    int len;

    len = strlen(c_stringp);
    if(len > 255)
        len = 255;
    str255[0] = len;
    memcpy(str255 + 1, c_stringp, len);
}

char *
Executor::pstr_index_after(StringPtr p, char c, int i)
{
    StringPtr ep;

    if(p)
    {
        for(ep = p + (unsigned char)p[0] + 1, p += 1 + i;
            p < ep && *p != c;
            p++)
            ;
        return (p == ep) ? NULL : (char *)p;
    }
    else
        return NULL;
}

PRIVATE void strNassign(Str63 new1, const StringPtr old, int n)
{
    int old_length, new_length;

    old_length = U(old[0]);
    if(old_length <= n)
        new_length = old_length;
    else
    {
        warning_unexpected("Truncating string that's too long for a Str%d:  "
                           "\"%.*s\"",
                           n, old_length, old + 1);
        new_length = n;
    }

    new1[0] = new_length;
    memcpy(new1 + 1, old + 1, new_length);
}

void Executor::str63assign(Str63 new1, const StringPtr old)
{
    strNassign(new1, old, 63);
}

void Executor::str31assign(Str63 new1, const StringPtr old)
{
    strNassign(new1, old, 31);
}

PUBLIC StringHandle
Executor::stringhandle_from_c_string(const char *c_stringp)
{
    int len;
    StringHandle retval;

    len = strlen(c_stringp) + 1;
    retval = (StringHandle)NewHandle(len);
    str255_from_c_string(STARH(retval), c_stringp);
    return retval;
}

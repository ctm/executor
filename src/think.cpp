/* Copyright 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_think[] = "$Id: think.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ThinkC.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "ThinkC.h"

using namespace Executor;

A1(PUBLIC, StringPtr, CtoPstr, char *, str)
{
    Size len;

    len = strlen((char *)str);
    BlockMoveData((Ptr)str, (Ptr)(str + 1), len);
    str[0] = len;
    return (StringPtr)str;
}

A1(PUBLIC, char *, PtoCstr, StringPtr, str)
{
    Size len;

    len = str[0];
    BlockMoveData((Ptr)(str + 1), (Ptr)str, len);
    str[len] = 0;
    return (char *)str;
}

P1(PUBLIC pascal trap, void, DebugStr, StringPtr, p)
{
    int i;

    fprintf(stderr, "debugstr: ");
    for(i = *p++; i-- > 0; fprintf(stderr, "%c", (LONGINT)*p++))
        ;
    fprintf(stderr, "\n");
}

PUBLIC void Executor::CDebugStr(StringPtr p)
{
    char *pstr;
    LONGINT n;

    n = strlen((char *)p);
    pstr = (char *)alloca(n + 1);
    pstr[0] = n;
    BlockMoveData((Ptr)p, (Ptr)pstr + 1, n);
}

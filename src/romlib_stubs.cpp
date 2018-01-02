/* Copyright 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/glue.h"

namespace Executor
{
void ROMlib_printstring(unsigned char *);
void ROMlib_printostype(OSType);
void ROMlib_hook(LONGINT);
}
using namespace Executor;

#if !defined(NDEBUG) && !defined(MSDOS)

#undef putchar

PUBLIC void Executor::ROMlib_printstring(unsigned char *p)
{
    int i;

    for(i = *p++; --i >= 0; putchar(*p++))
        ;
    putchar('\n');
}

PUBLIC void Executor::ROMlib_printostype(OSType t)
{
    putchar(t >> 24);
    putchar(t >> 16);
    putchar(t >> 8);
    putchar(t >> 0);
    putchar('\n');
}
#endif /* !defined(NDEBUG) && !defined(MSDOS) */

#if !defined(NDEBUG)
PUBLIC void Executor::ROMlib_hook(LONGINT hn)
{
    /* don't do anything; This is just here for gdbing */
}
#endif /* !defined (NDEBUG) */

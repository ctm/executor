/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ppc_call[] = "$Id: ppc_call.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

PUBLIC uint32
ppc_call(uint32 toc, uint32 (*func)(uint32), uint32 arg)
{
    uint32 retval;
    register uint32 toc_r2 asm("r2");

    toc_r2 = toc;
    asm volatile(""
                 :
                 : "r"(toc_r2));
    retval = func(arg);
    return retval;
}

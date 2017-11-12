/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

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

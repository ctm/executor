/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

PUBLIC uint32_t
ppc_call(uint32_t toc, uint32_t (*func)(uint32_t), uint32_t arg)
{
    uint32_t retval;
    register uint32_t toc_r2 asm("r2");

    toc_r2 = toc;
    asm volatile(""
                 :
                 : "r"(toc_r2));
    retval = func(arg);
    return retval;
}

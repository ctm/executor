/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "MemoryMgr.h"
namespace Executor
{
#define CFROMP(cp, pp)                                            \
    (BlockMove((Ptr)pp + 1, (Ptr)cp, (Size)(unsigned char)pp[0]), \
     cp[(unsigned char)pp[0]] = 0,                                \
     cp)

extern StringPtr ROMlib_PFROMC(StringPtr pp, char *cp, Size len);

#define PASCALSTR(x, len) \
    (len = strlen(x), ROMlib_PFROMC(alloca(len + 1), x, len))
}

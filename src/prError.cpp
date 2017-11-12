/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"

using namespace Executor;

P0(PUBLIC pascal trap, INTEGER, PrError)
{
    return CW(PrintErr);
}

P1(PUBLIC pascal trap, void, PrSetError, INTEGER, iErr)
{
    PrintErr = CW(iErr);
}

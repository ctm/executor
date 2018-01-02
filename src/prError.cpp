/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"

using namespace Executor;

PUBLIC pascal trap INTEGER Executor::C_PrError()
{
    return CW(PrintErr);
}

PUBLIC pascal trap void Executor::C_PrSetError(INTEGER iErr)
{
    PrintErr = CW(iErr);
}

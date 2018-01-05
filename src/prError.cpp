/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"

using namespace Executor;

INTEGER Executor::C_PrError()
{
    return CW(LM(PrintErr));
}

void Executor::C_PrSetError(INTEGER iErr)
{
    LM(PrintErr) = CW(iErr);
}

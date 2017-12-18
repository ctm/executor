/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
   Development, Inc.  All rights reserved.

   shutdown.c; ShutDown Manager routines */

#include "rsys/common.h"

#include "ShutDown.h"
#include "SegmentLdr.h"

#include "rsys/segment.h"

using namespace Executor;

P0(PUBLIC pascal trap, void, ShutDwnPower)
{
    ROMlib_exit = true;
    ExitToShell();
}

P0(PUBLIC pascal trap, void, ShutDwnStart)
{
    ExitToShell();
}

P2(PUBLIC pascal trap, void, ShutDwnInstall,
   ProcPtr, shutdown_proc,
   int16_t, flags)
{
    /* #warning "ShutDwnInstall unimplemented" */
    warning_unimplemented(NULL_STRING);
}

P1(PUBLIC pascal trap, void, ShutDwnRemove,
   ProcPtr, shutdown_proc)
{
    /* #warning "ShutDwnRemove unimplemented" */
    warning_unimplemented(NULL_STRING);
}

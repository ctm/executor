/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
   Development, Inc.  All rights reserved.

   shutdown.c; ShutDown Manager routines */

#include "rsys/common.h"

#include "ShutDown.h"
#include "SegmentLdr.h"

#include "rsys/segment.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_ShutDwnPower()
{
    ROMlib_exit = true;
    ExitToShell();
}

PUBLIC pascal trap void Executor::C_ShutDwnStart()
{
    ExitToShell();
}

PUBLIC pascal trap void Executor::C_ShutDwnInstall(ProcPtr shutdown_proc, int16_t flags)
{
    /* #warning "ShutDwnInstall unimplemented" */
    warning_unimplemented(NULL_STRING);
}

PUBLIC pascal trap void Executor::C_ShutDwnRemove(ProcPtr shutdown_proc)
{
    /* #warning "ShutDwnRemove unimplemented" */
    warning_unimplemented(NULL_STRING);
}

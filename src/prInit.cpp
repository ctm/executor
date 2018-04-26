/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in PrintMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "PrintMgr.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "ToolboxUtil.h"
#include "MemoryMgr.h"
#include "rsys/print.h"

using namespace Executor;

INTEGER Executor::ROMlib_printresfile = -1;

void Executor::C_PrOpen()
{
    StringHandle h;

    if(ROMlib_printresfile == -1)
    {
        h = GetString(-8192);
        HLock((Handle)h);
        ROMlib_printresfile = OpenRFPerm(STARH(h), Cx(LM(BootDrive)), fsCurPerm);
#if defined(NEXTSTEP)
        if(ROMlib_printresfile == -1)
            ROMlib_printresfile
                = OpenRFPerm("\020NeXTLaserPrinter", Cx(LM(BootDrive)), fsCurPerm);
#endif
        HUnlock((Handle)h);
    }
    LM(PrintErr) = ROMlib_printresfile == -1 ? CWC((OSErr)fnfErr) : CWC((OSErr)noErr);
}

void Executor::C_PrClose()
{
    if(ROMlib_printresfile != -1)
    {
        CloseResFile(ROMlib_printresfile);
        ROMlib_printresfile = -1;
    }
    LM(PrintErr) = CWC(noErr);
}

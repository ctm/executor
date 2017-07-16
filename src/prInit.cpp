/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_prInit[] =
		"$Id: prInit.c 63 2004-12-24 18:19:43Z ctm $";
#endif

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

PUBLIC INTEGER Executor::ROMlib_printresfile = -1;

P0(PUBLIC pascal trap, void, PrOpen)
{
    StringHandle h;

    if (ROMlib_printresfile == -1) {
	h = GetString(-8192);
	HLock((Handle) h);
	ROMlib_printresfile = OpenRFPerm(STARH(h), Cx(BootDrive), fsCurPerm);
#if defined(NEXTSTEP)
	if (ROMlib_printresfile == -1)
	    ROMlib_printresfile
	      = OpenRFPerm("\020NeXTLaserPrinter", Cx(BootDrive), fsCurPerm);
#elif defined(MSDOS)
	if (ROMlib_printresfile == -1)
	    ROMlib_printresfile
	      = OpenRFPerm("\017\\TMP\\EXECOUT.PS", Cx(BootDrive), fsCurPerm);
#endif
	HUnlock((Handle)h);
    }
    PrintErr = ROMlib_printresfile == -1 ? CWC(fnfErr) : CWC(noErr);
}

P0(PUBLIC pascal trap, void, PrClose)
{
    if (ROMlib_printresfile != -1) {
	CloseResFile(ROMlib_printresfile);
	ROMlib_printresfile = -1;
    }
    PrintErr = CWC(noErr);
}

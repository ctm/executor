/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_resMisc[] =
		"$Id: resMisc.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "rsys/resource.h"
#include "rsys/hook.h"

using namespace Executor;

A1(PUBLIC, INTEGER, ROMlib_setreserr, INTEGER, reserr)	/* INTERNAL */
{
    ResErr = CW(reserr);
    if (ResErr != noErr && ResErrProc) {
	ROMlib_hook(res_reserrprocnumber);

	EM_D0 = (unsigned short) reserr;	/* TODO: is unsigned short
							 correct? */
	CALL_EMULATOR((syn68k_addr_t)  CL((long) ResErrProc.raw()));
    }
    return CW(ResErr);
}

P0(PUBLIC pascal trap, INTEGER, ResError)
{
    return CW(ResErr);
}

P1(PUBLIC pascal trap, INTEGER, GetResFileAttrs, INTEGER, rn)
{
    resmaphand map;

    ROMlib_setreserr(noErr);
    map = ROMlib_rntohandl(rn, (Handle *)0);
    if (!map) {
        ROMlib_setreserr(resFNotFound);
        return(0);
    } else
        return(Hx(map, resfatr));
}

P2(PUBLIC pascal trap, void, SetResFileAttrs, INTEGER, rn, INTEGER, attrs)
{
    resmaphand map;

    ROMlib_setreserr(noErr);
    map = ROMlib_rntohandl(rn, (Handle *)0);
    if (!map)
        return;               /* don't set reserr.  I kid you not, see I-127 */
    else
        HxX(map, resfatr) = CW (attrs);
}

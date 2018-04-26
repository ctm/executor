/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "rsys/resource.h"
#include "rsys/hook.h"

using namespace Executor;

INTEGER Executor::ROMlib_setreserr(INTEGER reserr) /* INTERNAL */
{
    LM(ResErr) = CW(reserr);
    if(LM(ResErr) != CWC(noErr) && LM(ResErrProc))
    {
        ROMlib_hook(res_reserrprocnumber);

        EM_D0 = (unsigned short)reserr; /* TODO: is unsigned short
							 correct? */
        CALL_EMULATOR((syn68k_addr_t)CL_RAW((long)LM(ResErrProc).raw()));
    }
    return CW(LM(ResErr));
}

INTEGER Executor::C_ResError()
{
    return CW(LM(ResErr));
}

INTEGER Executor::C_GetResFileAttrs(INTEGER rn)
{
    resmaphand map;

    ROMlib_setreserr(noErr);
    map = ROMlib_rntohandl(rn, (Handle *)0);
    if(!map)
    {
        ROMlib_setreserr(resFNotFound);
        return (0);
    }
    else
        return (Hx(map, resfatr));
}

void Executor::C_SetResFileAttrs(INTEGER rn, INTEGER attrs)
{
    resmaphand map;

    ROMlib_setreserr(noErr);
    map = ROMlib_rntohandl(rn, (Handle *)0);
    if(!map)
        return; /* don't set reserr.  I kid you not, see I-127 */
    else
        HxX(map, resfatr) = CW(attrs);
}

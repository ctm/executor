/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "NotifyMgr.h"
#include "rsys/smash.h"
#include "rsys/hook.h"

using namespace Executor;

/* Forward declarations in NotifyMgr.h (DO NOT DELETE THIS LINE) */

typedef pascal void nmfunc(NMRecPtr nmptr);

/*
 * The stubs routine will take care of the trap conventions:
 *
 * A0: address of NMRec record
 * D0: result code
 */

A1(PUBLIC trap, OSErrRET, NMRemove, NMRecPtr, nmptr)
{
    return noErr;
}

A1(PUBLIC trap, OSErrRET, NMInstall, NMRecPtr, nmptr)
{
    /* The multiple beeps and delays that used to be here make OpenProlog
   * really irritating to use.
   */
    SysBeep(5);
    if(MR(nmptr->nmResp))
    {
        if(guest_cast<LONGINT>(nmptr->nmResp) == CLC(-1))
            NMRemove(nmptr);
        else
        {
            LONGINT saved0, saved1, saved2, savea0, savea1;

            ROMlib_hook(notify_procnumber);

            saved0 = EM_D0;
            saved1 = EM_D1;
            saved2 = EM_D2;
            savea0 = EM_A0;
            savea1 = EM_A1;
            PUSHADDR(US_TO_SYN68K(nmptr));
            CALL_EMULATOR(CL(guest_cast<uint32_t>(nmptr->nmResp)));
            EM_D0 = saved0;
            EM_D1 = saved1;
            EM_D2 = saved2;
            EM_A0 = savea0;
            EM_A1 = savea1;
        }
    }
    return noErr;
}

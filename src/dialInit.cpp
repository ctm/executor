/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "DialogMgr.h"
#include "FontMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/itm.h"
#include "rsys/mman.h"

using namespace Executor;

PUBLIC pascal void Executor::C_ROMlib_mysound(INTEGER i)
{
    while(i--)
        SysBeep(5);
}

PUBLIC pascal trap void Executor::C_ErrorSound(ProcPtr sp) /* IMI-411 */
{
    DABeeper = RM(sp);
}

PUBLIC pascal trap void Executor::C_InitDialogs(ProcPtr rp) /* IMI-411 */
{
    Ptr nothing = (Ptr) "";

    TheZoneGuard guard(SysZone);

    DlgFont = CWC(systemFont);
    ResumeProc = RM(rp);
    ErrorSound((ProcPtr)P_ROMlib_mysound);
    Handle tmp;
    PtrToHand(nothing, &tmp, (LONGINT)1);
    DAStrings[0] = RM(tmp);
    PtrToHand(nothing, &tmp, (LONGINT)1);
    DAStrings[1] = RM(tmp);
    PtrToHand(nothing, &tmp, (LONGINT)1);
    DAStrings[2] = RM(tmp);
    PtrToHand(nothing, &tmp, (LONGINT)1);
    DAStrings[3] = RM(tmp);
}

PUBLIC void Executor::SetDAFont(INTEGER i) /* IMI-412 */
{
    DlgFont = CW(i);
}

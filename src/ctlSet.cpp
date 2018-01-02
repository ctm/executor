/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"

#include "rsys/ctl.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_SetCtlValue(ControlHandle c, /* IMI-326 */
                                                INTEGER v)
{
    CtlCallGuard guard(c);
    if(v < Hx(c, contrlMin))
        HxX(c, contrlValue) = HxX(c, contrlMin);
    else if(v > Hx(c, contrlMax))
        HxX(c, contrlValue) = HxX(c, contrlMax);
    else
        HxX(c, contrlValue) = CW(v);
    CTLCALL(c, drawCntl, ALLINDICATORS);

    EM_D0 = 0;
}

PUBLIC pascal trap INTEGER Executor::C_GetCtlValue(/* IMI-326 */
                                                   ControlHandle c)
{
    return Hx(c, contrlValue);
}

PUBLIC pascal trap void Executor::C_SetCtlMin(ControlHandle c, /* IMI-326 */
                                              INTEGER v)
{
    CtlCallGuard guard(c);
    HxX(c, contrlMin) = CW(v);
    if(Hx(c, contrlValue) < v)
        HxX(c, contrlValue) = CW(v);
    CTLCALL(c, drawCntl, ALLINDICATORS);
}

PUBLIC pascal trap INTEGER Executor::C_GetCtlMin(ControlHandle c) /* IMI-327 */
{
    return Hx(c, contrlMin);
}

PUBLIC pascal trap void Executor::C_SetCtlMax(ControlHandle c, /* IMI-327 */
                                              INTEGER v)
{
    CtlCallGuard guard(c);
    /* #### TEST ON MAC MacBreadboard's behaviour suggests that
	   this code is needed. */
    if(v < Hx(c, contrlMin))
        v = Hx(c, contrlMin);

    HxX(c, contrlMax) = CW(v);
    if(Hx(c, contrlValue) > v)
        HxX(c, contrlValue) = CW(v);
    CTLCALL(c, drawCntl, ALLINDICATORS);
}

PUBLIC pascal trap INTEGER Executor::C_GetCtlMax(ControlHandle c) /* IMI-327 */
{
    return Hx(c, contrlMax);
}

/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in DeskMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "EventMgr.h"
#include "WindowMgr.h"
#include "DeviceMgr.h"
#include "DeskMgr.h"
#include "MenuMgr.h"
#include "QuickDraw.h"
#include "OSEvent.h"
#include "ToolboxEvent.h"
#include "OSUtil.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/hook.h"
#include "rsys/aboutbox.h"

using namespace Executor;

P1(PUBLIC pascal trap, INTEGER, OpenDeskAcc, Str255, acc) /* IMI-440 */
{
    // THINK Reference says that OpenDeskAcc's return value
    // is undefined on error. This returns zero - otherwise
    // we get a compiler warning that might hide real bugs.
    INTEGER retval = 0;
    GUEST<INTEGER> retval_s;
    DCtlHandle dctlh;
    WindowPtr wp;

    if(EqualString(acc, about_box_menu_name_pstr, true, true))
    {
        do_about_box();
        retval = 0;
        goto done;
    }

    if(OpenDriver(acc, &retval_s) == noErr)
    {
        retval = CW(retval_s);
        dctlh = GetDCtlEntry(retval);
        if(dctlh)
        {
            wp = HxP(dctlh, dCtlWindow);
            if(wp)
            {
                ShowWindow(wp);
                SelectWindow(wp);
            }
        }
    }

done:

    SEvtEnb = true;
    return retval;
}

P1(PUBLIC pascal trap, void, CloseDeskAcc, INTEGER, rn)
{
    CloseDriver(rn);
}

P2(PUBLIC pascal trap, void, SystemClick, EventRecord *, evp, WindowPtr, wp)
{
    Point p;
    LONGINT pointaslong, val;
    Rect bounds;
    GUEST<LONGINT> templ;

    if(wp)
    {
        p.h = CW(evp->where.h);
        p.v = CW(evp->where.v);
        if(PtInRgn(p, WINDOW_STRUCT_REGION(wp)))
        {
            pointaslong = ((LONGINT)p.v << 16) | (unsigned short)p.h;
            val = WINDCALL((WindowPtr)wp, wHit, pointaslong);
            switch(val)
            {
                case wInContent:
                    if(WINDOW_HILITED_X(wp))
                    {
                        templ = guest_cast<LONGINT>(RM(evp));
                        Control(WINDOW_KIND(wp), accEvent, (Ptr)&templ);
                    }
                    else
                        SelectWindow(wp);
                    break;
                case wInDrag:
                    bounds.top = CW(CW(MBarHeight) + 4);
                    bounds.left = CW(CW(GD_BOUNDS(MR(TheGDevice)).left) + 4);
                    bounds.bottom = CW(CW(GD_BOUNDS(MR(TheGDevice)).bottom) - 4);
                    bounds.right = CW(CW(GD_BOUNDS(MR(TheGDevice)).right) - 4);
                    DragWindow(wp, p, &bounds);
                    break;
                case wInGoAway:
                    if(TrackGoAway(wp, p))
                        CloseDeskAcc(WINDOW_KIND(wp));
                    break;
            }
        }
        else
        {
            if(DeskHook)
            {
                ROMlib_hook(desk_deskhooknumber);
                EM_D0 = -1;
                EM_A0 = US_TO_SYN68K(evp);
                CALL_EMULATOR((syn68k_addr_t)CL(guest_cast<LONGINT>(DeskHook)));
            }
        }
    }
}

P1(PUBLIC pascal trap, BOOLEAN, SystemEdit, INTEGER, editcmd)
{
    WindowPeek wp;
    BOOLEAN retval;

    wp = (WindowPeek)FrontWindow();
    if(!wp)
        retval = false;
    else if((retval = WINDOW_KIND(wp) < 0))
        Control(WINDOW_KIND(wp), editcmd + accUndo, (Ptr)0);
    return retval;
}

#define rntodctlh(rn) (MR(MR(UTableBase)[-((rn) + 1)]))
#define itorn(i) ((-i) - 1)

P0(PUBLIC pascal trap, void, SystemTask)
{
    DCtlHandle dctlh;
    INTEGER i;

    for(i = 0; i < CW(UnitNtryCnt); ++i)
    {
        dctlh = MR(MR(UTableBase)[i]);
        if((HxX(dctlh, dCtlFlags) & CWC(NEEDTIMEBIT)) && TickCount() >= Hx(dctlh, dCtlCurTicks))
        {
            Control(itorn(i), accRun, (Ptr)0);
            HxX(dctlh, dCtlCurTicks) = CL(Hx(dctlh, dCtlCurTicks) + Hx(dctlh, dCtlDelay));
        }
    }
}

P1(PUBLIC pascal trap, BOOLEAN, SystemEvent, EventRecord *, evp)
{
    BOOLEAN retval;
    WindowPeek wp;
    INTEGER rn;
    DCtlHandle dctlh;
    GUEST<LONGINT> templ;

    if(SEvtEnb)
    {
        wp = 0;
        switch(CW(evp->what))
        {
            default:
            case nullEvent:
            case mouseDown:
            case networkEvt:
            case driverEvt:
            case app1Evt:
            case app2Evt:
            case app3Evt:
            case app4Evt:
                break;
            case mouseUp:
            case keyDown:
            case keyUp:
            case autoKey:
                wp = (WindowPeek)FrontWindow();
                break;
            case updateEvt:
            case activateEvt:
                wp = MR(guest_cast<WindowPeek>(evp->message));
                break;
            case diskEvt:
                /* NOTE:  I think the code around toolevent.c:277 should
		      really be here.  I'm not going to get all excited
		      about it right now though. */
                break;
        }
        if(wp)
        {
            rn = WINDOW_KIND(wp);
            if((retval = rn < 0))
            {
                dctlh = rntodctlh(rn);
                if(Hx(dctlh, dCtlEMask) & (1 << CW(evp->what)))
                {
                    templ = guest_cast<LONGINT>(RM(evp));
                    Control(rn, accEvent, (Ptr)&templ);
                }
            }
        }
        else
            retval = false;
    }
    else
        retval = false;
    return retval;
}

P1(PUBLIC pascal trap, void, SystemMenu, LONGINT, menu)
{
    INTEGER i;
    DCtlHandle dctlh;
    GUEST<LONGINT> menu_s;

    for(i = 0; i < CW(UnitNtryCnt); ++i)
    {
        dctlh = MR(MR(UTableBase)[i]);
        if(HxX(dctlh, dCtlMenu) == MBarEnable)
        {
            menu_s = CL(menu);
            Control(itorn(i), accMenu, (Ptr)&menu_s);
            /*-->*/ break;
        }
    }
}

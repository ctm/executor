/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "ToolboxUtil.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"
#include "ListMgr.h"

#include "rsys/ctl.h"
#include "rsys/wind.h"
#include "rsys/stdfile.h"
#include "rsys/hook.h"
#include <rsys/functions.impl.h>

/*
 * Loser suggests that controls are tested in the opposite order that
 * they're on the linked list.  That would make sense, since it would
 * insure that when drawn under normal circumstances, the control that
 * is visible and clicked is the one that will be FindControl will
 * return.  Recursion is you friend.
 */

using namespace Executor;

INTEGER
find_control_helper(Point p, ControlHandle c,
                    GUEST<ControlHandle> *cp)
{
    INTEGER retval;
    ControlHandle next;

    if(!c)
        retval = 0;
    else
    {
        next = CTL_NEXT_CONTROL(c);
        if(!(retval = find_control_helper(p, next, cp)))
        {
            if(CTL_VIS(c) && CTL_HILITE(c) != 255)
            {
                retval = TestControl(c, p);
                if(retval)
                    *cp = RM(c);
            }
        }
    }
    return retval;
}

INTEGER Executor::C_FindControl(Point p, WindowPtr w,
                                GUEST<ControlHandle> *cp) /* IMI-323 */
{
    INTEGER retval;

    retval = w ? find_control_helper(p, WINDOW_CONTROL_LIST(w), cp) : 0;
    if(!retval)
        *cp = 0;
    return retval;
}


static inline void CALLACTION(ControlHandle ch, INTEGER inpart, ControlActionUPP a)
{
    ROMlib_hook(ctl_cdefnumber);
    if(a == &ROMlib_mytrack)
        ROMlib_mytrack(ch, inpart);
    else if(a == &ROMlib_stdftrack)
        ROMlib_stdftrack(ch, inpart);
    else
        a(ch, inpart);
}

INTEGER Executor::C_TrackControl(ControlHandle c, Point p,
                                 ControlActionUPP a) /* IMI-323 */
{
    INTEGER partstart, inpart;
    EventRecord ev;
    thumbstr thumb;
    RgnHandle rh;
    LONGINT l;
    Point whereunswapped;

    int retval;

    CtlCallGuard guard(c);
    partstart = inpart = TestControl(c, p);

    /* Super-Dice-It hack: It appears that Super Dice It 1.1
	  calls TrackControl with p in Global coordinates.  Without
	  this mod, we get hosed */
    if(!partstart)
    {
        GUEST<Point> ptmp;
        GetMouse(&ptmp);
        p = ptmp.get();
        partstart = inpart = TestControl(c, p);
    }

    if(CTL_ACTION_AS_LONG(c) == -1L)
    {
        /* if we don't draw before/after, then jim's demo cdefs
	      don't update pop up menu bars */
        CTLCALL(c, drawCntl, 0);
        /* this is not how IMI says to do it, but it makes
	      Microsoft Word work. */
        CTLCALL(c, autoTrack, inpart);
        CTLCALL(c, drawCntl, 0);

        /* NOTE 1: force a return of inpart */
        partstart = inpart;
        goto done;
    }

    if(a == (ControlActionUPP)-1)
        a = CTL_ACTION(c);

    /* #if 0 reading the above code suggests that
   it's impossible to get here */
    if(0 && a == (ControlActionUPP)-1)
    {
        /* totally custom */
        while(!GetOSEvent(mUpMask, &ev))
        {
            GlobalToLocal(&ev.where);
            whereunswapped.h = CW(ev.where.h);
            whereunswapped.v = CW(ev.where.v);
            inpart = TestControl(c, whereunswapped);
            CTLCALL(c, autoTrack, inpart);
        }
        CTLCALL(c, posCntl, ((((int32_t)whereunswapped.v - p.v) << 16)
                             | (uint16_t)(whereunswapped.h - p.h)));
        inpart = TestControl(c, whereunswapped);
    }
    else
        /* #endif */
        if(partstart > 128)
    {
        /* indicator */
        /* The code used to & the result of CTLCALL with 0xf000
	      for no apparent reason.  Taking it out fixed a bug in
	      Quicken. */
        if(!CTLCALL(c, dragCntl, partstart))
        {
            thumb._tlimit.left = CW(p.h);
            thumb._tlimit.top = CW(p.v);
            CTLCALL(c, thumbCntl, ptr_to_longint(&thumb));
            rh = NewRgn();

            CTLCALL(c, calcThumbRgn, ptr_to_longint(rh));

            PATASSIGN(LM(DragPattern), ltGray);
            l = DragTheRgn(rh, p, &thumb._tlimit, &thumb._tslop,
                           CW(thumb._taxis), (ProcPtr)a);
            if((uint32_t)l != 0x80008000)
            {
                CTLCALL(c, posCntl, l);
                inpart = partstart;
            }
            DisposeRgn(rh);
        }
    }
    else
    {
        /* not an indicator */
        HxX(c, contrlHilite) = partstart;
        CTLCALL(c, drawCntl, partstart);
        /* CALLACTION can remove mouse up events which is why the
	      following line is not a GetOSEvent call. */
        while(!OSEventAvail(mUpMask, &ev) && StillDown())
        {
            GlobalToLocal(&ev.where);
            whereunswapped.h = CW(ev.where.h);
            whereunswapped.v = CW(ev.where.v);
            inpart = TestControl(c, whereunswapped);
            if(inpart && inpart != partstart)
                inpart = 0;
            if(inpart != U(HxX(c, contrlHilite)))
            {
                HxX(c, contrlHilite) = inpart;
                CTLCALL(c, drawCntl, partstart);
            }
            if(a && inpart)
                CALLACTION(c, inpart, a);
        }
        GetOSEvent(mUpMask, &ev);
        GlobalToLocal(&ev.where);
        whereunswapped.h = CW(ev.where.h);
        whereunswapped.v = CW(ev.where.v);
        if(HxX(c, contrlHilite))
        {
            HxX(c, contrlHilite) = 0;
            CTLCALL(c, drawCntl, partstart);
        }
        inpart = TestControl(c, whereunswapped);
    }
done:;

    retval = (partstart == inpart ? inpart : 0); /* DON'T CHANGE THIS
						  line w/o looking to Note 1
						  above */
    return retval;
}

INTEGER Executor::C_TestControl(ControlHandle c, Point p) /* IMI-325 */
{
    int16_t retval;

    CtlCallGuard guard(c);
    if(Hx(c, contrlVis) && U(Hx(c, contrlHilite)) != 255)
        retval = CTLCALL(c, testCntl, ((LONGINT)p.v << 16) | (unsigned short)p.h);
    else
        retval = 0;
    return retval;
}

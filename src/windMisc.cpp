/* Copyright 1986, 1989, 1990, 1995 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "EventMgr.h"
#include "ToolboxUtil.h"
#include "OSEvent.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/hook.h"
#include "MemoryMgr.h"

#include "rsys/evil.h"
#include "rsys/executor.h"

using namespace Executor;

static bool
is_window_ptr(WindowPeek w)
{
    WindowPeek wp;
    bool retval;

    if(!w)
        retval = false;
    else
    {
        for(wp = MR(LM(WindowList));
            wp && wp != w;
            wp = WINDOW_NEXT_WINDOW(wp))
            ;
        retval = (wp == w);
    }

    return retval;
}

void Executor::C_SetWRefCon(WindowPtr w, LONGINT data)
{
    if(is_window_ptr((WindowPeek)w))
        WINDOW_REF_CON_X(w) = CL(data);
}

/*
 * A bug in BBEdit 3.5 suggests that we shouldn't splode when
 * GetWRefCon (0xffff0000) is called.  Either that, or I'm confused as
 * to how the dialoghook from stdfile is supposed to work.
 */

LONGINT Executor::C_GetWRefCon(WindowPtr w)
{
    LONGINT retval;

    if(!w)
        retval = WINDOW_REF_CON((WindowPtr)ROMlib_offset);
    else
    {

#if 0 /* THIS MOD BREAKS Excel 4's chart making ability */
      if (is_window_ptr ((WindowPeek) w))
	retval = WINDOW_REF_CON (w);
      else
	retval = 0;
#else
        retval = WINDOW_REF_CON(w);
#endif
    }

    return retval;
}

void Executor::C_SetWindowPic(WindowPtr w, PicHandle p)
{
    if(is_window_ptr((WindowPeek)w))
        WINDOW_PIC_X(w) = RM(p);
}

PicHandle Executor::C_GetWindowPic(WindowPtr w)
{
    PicHandle retval;

    if(is_window_ptr((WindowPeek)w))
        retval = WINDOW_PIC(w);
    else
        retval = 0;

    return retval;
}

/*
 * Note, below we do what the mac does, rather than what IMI-294 says
 */

LONGINT Executor::C_PinRect(Rect *r, Point p)
{
    if(p.h < CW(r->left))
        p.h = CW(r->left);
    else if(p.h >= CW(r->right))
        p.h = CW(r->right) - 1;
    if(p.v < CW(r->top))
        p.v = CW(r->top);
    else if(p.v >= CW(r->bottom))
        p.v = CW(r->bottom) - 1;

    return (((LONGINT)p.v << 16) | (unsigned short)p.h);
}

LONGINT Executor::C_DragTheRgn(RgnHandle rgn, Point startp, Rect *limit,
                               Rect *slop, INTEGER axis, ProcPtr proc)
{
    RgnHandle rh;
    PenState ps;
    EventRecord ev;
    Point p;
    LONGINT l;
    int drawn;

    rh = NewRgn();
    CopyRgn(rgn, rh);
    InsetRgn(rh, 1, 1);
    DiffRgn(rgn, rh, rgn);
    p.h = startp.h;
    p.v = startp.v;
    GetPenState(&ps);
    PenPat(LM(DragPattern));
    PenMode(notPatXor);
    if((drawn = PtInRect(p, slop)))
    {
        PaintRgn(rgn); /* was Frame */
        ROMlib_rootless_update(rgn);
    }
    while(!GetOSEvent(mUpMask, &ev))
    {
        GlobalToLocal(&ev.where);
        Point ep = ev.where.get();
        if(PtInRect(ep, slop))
        {
            l = PinRect(limit, ep);
            if(axis == hAxisOnly)
                ep.v = p.v;
            else
                ep.v = HiWord(l);
            if(axis == vAxisOnly)
                ep.h = p.h;
            else
                ep.h = LoWord(l);
            if(p.h != ep.h || p.v != ep.v)
            {
                if(drawn)
                    PaintRgn(rgn);
                drawn = true;
                OffsetRgn(rgn, ep.h - p.h, ep.v - p.v);
                PaintRgn(rgn);
                ROMlib_rootless_update(rgn);
                p.h = ep.h;
                p.v = ep.v;
            }
        }
        else
        {
            if(drawn)
            {
                PaintRgn(rgn);
                ROMlib_rootless_update(nullptr);
            }
            drawn = false;
        }
        if(proc)
        {
            long saved0, saved1, saved2, saved3, savea0, savea1, savea2, savea3;
            ROMlib_hook(wind_dragtheregionnumber);

            saved0 = EM_D0;
            saved1 = EM_D1;
            saved2 = EM_D2;
            saved3 = EM_D3;
            savea0 = EM_A0;
            savea1 = EM_A1;
            savea2 = EM_A2;
            savea3 = EM_A3;
            CALL_EMULATOR(US_TO_SYN68K(proc));
            EM_D0 = saved0;
            EM_D1 = saved1;
            EM_D2 = saved2;
            EM_D3 = saved3;
            EM_A0 = savea0;
            EM_A1 = savea1;
            EM_A2 = savea2;
            EM_A3 = savea3;
        }
        CALLDRAGHOOK();
        PenMode(notPatXor);
    }
    if(drawn)
    {
        PaintRgn(rgn);
        ROMlib_rootless_update(nullptr);
    }
    SetPenState(&ps);
    DisposeRgn(rh);
    if(drawn)
        return ((((LONGINT)p.v - startp.v) << 16) | (unsigned short)(p.h - startp.h));
    else
        return 0x80008000;
}

LONGINT Executor::C_DragGrayRgn(RgnHandle rgn, Point startp, Rect *limit,
                                Rect *slop, INTEGER axis, ProcPtr proc)
{
    PATASSIGN(LM(DragPattern), gray);
    return DragTheRgn(rgn, startp, limit, slop, axis, proc);
}

void Executor::C_ClipAbove(WindowPeek w)
{
    WindowPeek wp;

    SectRgn(PORT_CLIP_REGION(MR(wmgr_port)), MR(LM(GrayRgn)),
            PORT_CLIP_REGION(MR(wmgr_port)));
    for(wp = MR(LM(WindowList)); wp != w; wp = WINDOW_NEXT_WINDOW(wp))
        if(WINDOW_VISIBLE_X(wp))
            DiffRgn(PORT_CLIP_REGION(MR(wmgr_port)), WINDOW_STRUCT_REGION(wp),
                    PORT_CLIP_REGION(MR(wmgr_port)));
}

BOOLEAN Executor::C_CheckUpdate(EventRecord *ev)
{
    WindowPeek wp;
    Rect picr;

    for(wp = MR(LM(WindowList)); wp; wp = WINDOW_NEXT_WINDOW(wp))
        if(WINDOW_VISIBLE_X(wp) && !EmptyRgn(WINDOW_UPDATE_REGION(wp)))
        {
            if(WINDOW_PIC(wp))
            {
                ThePortGuard guard((GrafPtr)wp);

                BeginUpdate((WindowPtr)wp);
                picr = PORT_RECT(wp);
                DrawPicture(WINDOW_PIC(wp), &picr);
                EndUpdate((WindowPtr)wp);
            }
            else
            {
                ev->what = CW(updateEvt);
                ev->message = guest_cast<LONGINT>(RM(wp));
                return true;
            }
        }
    return false;
}

void Executor::C_SaveOld(WindowPeek w)
{
    LM(OldStructure) = RM(NewRgn());
    LM(OldContent) = RM(NewRgn());
    CopyRgn(WINDOW_STRUCT_REGION(w), MR(LM(OldStructure)));
    CopyRgn(WINDOW_CONT_REGION(w), MR(LM(OldContent)));
}

void Executor::C_PaintOne(WindowPeek w, RgnHandle clobbered)
{
    RgnHandle rh;

    ThePortGuard guard(MR(wmgr_port));
    if(w)
        SetClip(WINDOW_STRUCT_REGION(w));
    else
        ClipRect(&GD_BOUNDS(MR(LM(TheGDevice))));
    ClipAbove(w);
    SectRgn(PORT_CLIP_REGION(MR(wmgr_port)), clobbered,
            PORT_CLIP_REGION(MR(wmgr_port)));
    if(w)
    {
        rh = NewRgn();
        SectRgn(PORT_CLIP_REGION(MR(wmgr_port)),
                WINDOW_STRUCT_REGION(w), rh);
        if(!EmptyRgn(rh))
        {
            WINDCALL((WindowPtr)w, wDraw, 0);
            SectRgn(PORT_CLIP_REGION(MR(wmgr_port)),
                    WINDOW_CONT_REGION(w), rh);
            if(!EmptyRgn(rh))
            {
                if(LM(SaveUpdate))
                    UnionRgn(rh, WINDOW_UPDATE_REGION(w),
                             WINDOW_UPDATE_REGION(w));
                /* erase the region with the window's background
		      color and pattern */
                if(LM(PaintWhite))
                {
                    RGBColor *window_colors;

                    window_colors = validate_colors_for_window((WindowPtr)w);

                    RGBBackColor(&window_colors[wContentColor]);
                    EraseRgn(rh);

                    /* restore the background color to it's usual
                          value */
                    RGBBackColor(&ROMlib_white_rgb_color);
                }
            }
        }
        DisposeRgn(rh);
    }
    else if(!EmptyRgn(PORT_CLIP_REGION(MR(wmgr_port))))
    {
        if(LM(DeskHook))
            WINDCALLDESKHOOK();
        else
        {
            if((USE_DESKCPAT_VAR & USE_DESKCPAT_BIT)
               && PIXMAP_PIXEL_SIZE(GD_PMAP(MR(LM(MainDevice)))) > 2)
                FillCRgn(clobbered, MR(LM(DeskCPat)));
            else
                FillRgn(clobbered, LM(DeskPattern));
        }
    }
}

void Executor::C_PaintBehind(WindowPeek w, RgnHandle clobbered)
{
    RgnHandle rh, testrgn;
    WindowPeek wp;

    rh = NewRgn();
    testrgn = NewRgn();
    CopyRgn(clobbered, rh);
    LM(PaintWhite) = CWC(-1);
    LM(SaveUpdate) = CWC(-1);
    for(wp = w; wp; wp = WINDOW_NEXT_WINDOW(wp))
    {
        if(WINDOW_VISIBLE_X(wp))
        {
            SectRgn(rh, WINDOW_STRUCT_REGION(wp), testrgn);
            if(!EmptyRgn(testrgn))
            {
                PaintOne(wp, rh);
                DiffRgn(rh, WINDOW_STRUCT_REGION(wp), rh);
                if(EmptyRgn(rh))
                    break;
            }
        }
    }
    if(!EmptyRgn(rh))
        PaintOne((WindowPeek)0, rh);
    DisposeRgn(testrgn);
    DisposeRgn(rh);
}

void Executor::C_CalcVis(WindowPeek w)
{
    WindowPeek wp;

    if(w && WINDOW_VISIBLE_X(w))
    {
        SectRgn(MR(LM(GrayRgn)), WINDOW_CONT_REGION(w), PORT_VIS_REGION(w));
        for(wp = MR(LM(WindowList)); wp != w; wp = WINDOW_NEXT_WINDOW(wp))
            if(WINDOW_VISIBLE_X(wp))
                DiffRgn(PORT_VIS_REGION(w), WINDOW_STRUCT_REGION(wp),
                        PORT_VIS_REGION(w));
        OffsetRgn(PORT_VIS_REGION(w),
                  CW(PORT_BOUNDS(w).left),
                  CW(PORT_BOUNDS(w).top));
    }
}
void Executor::C_CalcVisBehind(WindowPeek w, RgnHandle clobbered)
{
    RgnHandle rh, testrgn;
    WindowPeek wp;

    if(!w)
        return;
    rh = NewRgn();
    testrgn = NewRgn();
    CopyRgn(clobbered, rh);
    CalcVis((WindowPeek)w);
    for(wp = MR(w->nextWindow); wp; wp = WINDOW_NEXT_WINDOW(wp))
    {
        if(WINDOW_VISIBLE_X(wp))
        {
            SectRgn(rh, WINDOW_STRUCT_REGION(wp), testrgn);
            if(!EmptyRgn(testrgn))
            {
                CalcVis((WindowPeek)wp);
                DiffRgn(rh, WINDOW_STRUCT_REGION(wp), rh);
                if(EmptyRgn(rh))
                    break;
            }
        }
    }

    DisposeRgn(rh);
    DisposeRgn(testrgn);
}

void Executor::C_DrawNew(WindowPeek w, BOOLEAN flag)
{
    RgnHandle r1, r2;

    r1 = NewRgn();
    r2 = NewRgn();
    /*
   * This works as IM describes, but I had to spend some time fiddling with
   * it so the code is still suspect.
   */
    XorRgn(WINDOW_STRUCT_REGION(w), MR(LM(OldStructure)), r1);
    XorRgn(MR(LM(OldContent)), WINDOW_CONT_REGION(w), r2);
    UnionRgn(r1, r2, r2);

    ThePortGuard guard(MR(wmgr_port));
    SetClip(WINDOW_STRUCT_REGION(w));
    ClipAbove(w);
    WINDCALL((WindowPtr)w, wDraw, 0);
    PaintBehind(w, r2);
    CalcVisBehind(w, r2);
    if(flag)
        CopyRgn(WINDOW_CONT_REGION(w), WINDOW_UPDATE_REGION(w));

    DisposeRgn(r1);
    DisposeRgn(r2);
}

INTEGER Executor::C_GetWVariant(WindowPtr w) /* IMV-208 */
{
    AuxWinHandle h;
    INTEGER retval;

    for(h = MR(LM(AuxWinHead)); h != 0 && HxP(h, awOwner) != w; h = HxP(h, awNext))
        ;
    retval = h != 0 ? (Hx(h, awFlags) >> 24) & 0xFF : 0;
    return retval;
}

void Executor::CALLDRAGHOOK(void)
{
    if(LM(DragHook))
    {
        LONGINT saved0, saved1, saved2, saved3,
            savea0, savea1, savea2, savea3;

        ROMlib_hook(wind_draghooknumber);

        saved0 = EM_D0;
        saved1 = EM_D1;
        saved2 = EM_D2;
        saved3 = EM_D3;
        savea0 = EM_A0;
        savea1 = EM_A1;
        savea2 = EM_A2;
        savea3 = EM_A3;
        EM_D0 = 0;
        CALL_EMULATOR((syn68k_addr_t)CL_RAW(LM(DragHook).raw()));
        EM_D0 = saved0;
        EM_D1 = saved1;
        EM_D2 = saved2;
        EM_D3 = saved3;
        EM_A0 = savea0;
        EM_A1 = savea1;
        EM_A2 = savea2;
        EM_A3 = savea3;
    }
}

void Executor::WINDCALLDESKHOOK(void)
{
    LONGINT saved0, saved1, saved2, saved3, savea0, savea1, savea2, savea3;
    ROMlib_hook(wind_deskhooknumber);
    saved0 = EM_D0;
    saved1 = EM_D1;
    saved2 = EM_D2;
    saved3 = EM_D3;
    savea0 = EM_A0;
    savea1 = EM_A1;
    savea2 = EM_A2;
    savea3 = EM_A3;
    EM_D0 = 0;
    CALL_EMULATOR((syn68k_addr_t)CL_RAW(LM(DeskHook).raw()));
    EM_D0 = saved0;
    EM_D1 = saved1;
    EM_D2 = saved2;
    EM_D3 = saved3;
    EM_A0 = savea0;
    EM_A1 = savea1;
    EM_A2 = savea2;
    EM_A3 = savea3;
}

#if defined(EVIL_ILLUSTRATOR_7_HACK)
BOOLEAN Executor::ROMlib_evil_illustrator_7_hack = false;
#endif

int32_t Executor::ROMlib_windcall(WindowPtr wind, int16_t mess, int32_t param)
{
    Handle defproc;
    int32_t retval;
    windprocp wp;
    Rect saverect;

    defproc = WINDOW_DEF_PROC(wind);
    if(*defproc == NULL)
        LoadResource(defproc);

    switch(mess)
    {
        case wCalcRgns:
            saverect = wind->portBits.bounds;
            wind->portBits.bounds = PORT_BOUNDS(wind);
            break;
        default:
            /* do nothing */
            break;
    }

    wp = (windprocp)STARH(defproc);

    if(wp == P_wdef0)
        retval = C_wdef0(var(wind), wind, mess, param);
    else if(wp == P_wdef16)
        retval = C_wdef16(var(wind), wind, mess, param);
    else
    {
#if defined EVIL_ILLUSTRATOR_7_HACK
        BOOLEAN save_hack;

        save_hack = ROMlib_evil_illustrator_7_hack;
        ROMlib_evil_illustrator_7_hack = ROMlib_creator == TICK("ART5");
#endif
        ROMlib_hook(wind_wdefnumber);
        HLockGuard guard(defproc);
        retval = CToPascalCall(STARH(defproc),
                               ctop(&C_wdef0), var(wind), wind, mess, param);
#if defined EVIL_ILLUSTRATOR_7_HACK
        ROMlib_evil_illustrator_7_hack = save_hack;
#endif
    }

    if(mess == wCalcRgns)
        wind->portBits.bounds = saverect;
    return retval;
}

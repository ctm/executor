/* Copyright 1988 - 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in SysErr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "EventMgr.h"
#include "ToolboxEvent.h"
#include "DialogMgr.h"
#include "OSUtil.h"
#include "SegmentLdr.h"
#include "SysErr.h"
#include "MemoryMgr.h"
#include "BinaryDecimal.h"

#include "rsys/cquick.h"
#include "rsys/quick.h"
#include "rsys/segment.h"
#include "rsys/flags.h"
#include "rsys/version.h"
#include "rsys/vdriver.h"
#include "rsys/osevent.h"
#include "rsys/options.h"
#include "rsys/syserr.h"

using namespace Executor;

static myalerttab_t myalerttab = {
    CWC(8),

#define WELCOME_CODE 0x28
    CWC(WELCOME_CODE), /* 1. normally "Welcome to M*cintosh", now copyright stuff */
    CWC(10),
    CWC(50),
    CWC(150),
    CWC(31),
    CWC(0),
    CWC(151),

    CWC(50), /* 2. primary text */
    CWC(56),
    { CWC(108), CWC(105) },
    "\251 Abacus Research and Development, Inc.  1986-1999",

    CWC(31), /* 3. ARDI icon */
    CWC(136),
    { CWC(99), CWC(56), CWC(131), CWC(88) },
    { { 0xff, 0xff, 0xff, 0xff },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0x9d, 0xdd, 0xdd, 0xdd },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0x9d, 0xdd, 0xdd, 0xdd },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0xff, 0xff, 0xff, 0xff },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0x88, 0x80, 0x80, 0x89 },
      { 0x88, 0xbc, 0xbc, 0x89 },
      { 0x80, 0xa2, 0x92, 0x81 },
      { 0x88, 0xa2, 0x92, 0x9d },
      { 0x94, 0xa2, 0x92, 0x89 },
      { 0xa2, 0xbc, 0x92, 0x89 },
      { 0xa2, 0xa8, 0x92, 0x89 },
      { 0xa2, 0xa4, 0x92, 0x89 },
      { 0xbe, 0xa2, 0x92, 0x89 },
      { 0xa2, 0xa2, 0x92, 0x89 },
      { 0xa2, 0xa2, 0x92, 0x89 },
      { 0xa2, 0xa2, 0xbc, 0x9d },
      { 0x80, 0x80, 0x80, 0x81 },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0x88, 0x88, 0x88, 0x89 },
      { 0xff, 0xff, 0xff, 0xff } },

    CWC(150), /* 4. secondary text */
    CWC(50),
    { CWC(122), CWC(125) },
    "Please click on \"Info\" for more information\0\0",

    CWC(151), /* 5. the buttons */
    CWC(26),
    CWC(2),
    CWC(153),
    { CWC(150), CWC(50), CWC(170), CWC(100) }, /* OK rect */
    CWC(0),
    CWC(155),
    { CWC(150), CWC(130), CWC(170), CWC(190) }, /* Abort rect */
    CWC(156),

    CWC(153), /* 6. OK button */
    CWC(4),
    "OK\0",

    CWC(155), /* 7. Info button */
    CWC(6),
    "Info\0",

    CWC(156), /* 8. Info "procedure" */
    CWC(4),
    nullptr, /* was mydolicense, which we no longer use */
};

char syserr_msg[256];

static GUEST<INTEGER> *findid(INTEGER);
static void drawtextstring(INTEGER id, INTEGER offsetx, INTEGER offsety);
static void drawicon(INTEGER id, INTEGER offsetx, INTEGER offsety);
static void dobuttons(INTEGER id, INTEGER offsetx, INTEGER offsety);

static GUEST<INTEGER> *findid(INTEGER id)
{
    int i;
    GUEST<INTEGER> *ip;

    for(i = CW(*(GUEST<INTEGER> *)MR(LM(DSAlertTab))),
    ip = (GUEST<INTEGER> *)MR(LM(DSAlertTab)) + 1;
        i > 0 && CW(*ip) != id;
        --i, ip = (GUEST<INTEGER> *)((char *)ip + CW(ip[1]) + 2 * sizeof(INTEGER)))
        ;
    return i > 0 ? ip : nullptr;
}

static void drawtextstring(INTEGER id, INTEGER offsetx, INTEGER offsety)
{
    struct tdef *tp;

    if(id && (tp = (struct tdef *)findid(id)))
    {
        MoveTo(CW(tp->loc.h) + offsetx, CW(tp->loc.v) + offsety);
        DrawText_c_string(tp->text);
    }
}

static void drawicon(INTEGER id, INTEGER offsetx, INTEGER offsety)
{
    struct idef *ip;
    BitMap bm;
    Rect old_loc;

    ip = (struct idef *)findid(id);
    if(id && ip)
    {
        bm.baseAddr = RM((Ptr)ip->ike);
        bm.rowBytes = CWC(4);
        bm.bounds.left = bm.bounds.top = CWC(0);
        bm.bounds.right = bm.bounds.bottom = CWC(32);
        old_loc = ip->loc;
        C_OffsetRect(&ip->loc, offsetx, offsety);
        CopyBits(&bm, PORT_BITS_FOR_COPY(thePort),
                 &bm.bounds, &ip->loc, srcCopy, NULL);
        ip->loc = old_loc;
    }
}

static void dobuttons(INTEGER id, INTEGER offsetx, INTEGER offsety)
{
    struct bdef *bp;
    struct sdef *sp;
    struct pdef *pp;
    int i;
    EventRecord evt;
    int done;
    int tcnt, twid;
    Point p;
#define BILLBUTTONS /*  */
#if defined(BILLBUTTONS)
    INTEGER h, v;
#endif /* BILLBUTTONS */
    char *textp;

    if((bp = (struct bdef *)findid(id)))
    {
        for(i = 0; i < Cx(bp->nbut); i++)
        {

            /* Offset buttons; this hack is to center the splash screen
	     * on non-512x342 root windows...yuck!
	     */

            C_OffsetRect(&bp->buts[i].butloc, offsetx, offsety);
            if((sp = (struct sdef *)findid(CW(bp->buts[i].butstrid))))
            {
                textp = sp->text;
                tcnt = strlen(textp);
                twid = TextWidth((Ptr)textp, 0, tcnt);
                MoveTo((CW(bp->buts[i].butloc.left) + CW(bp->buts[i].butloc.right) - twid) / 2,
                       (CW(bp->buts[i].butloc.top) + CW(bp->buts[i].butloc.bottom)) / 2 + 4);
                DrawText((Ptr)textp, 0, tcnt);
            }
#if defined(BILLBUTTONS)
            h = CW(bp->buts[i].butloc.right) - CW(bp->buts[i].butloc.left);
            v = (CW(bp->buts[i].butloc.bottom) - CW(bp->buts[i].butloc.top)) / 2;
            if(h > v)
                h = v;
            if(!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
                FrameRoundRect(&bp->buts[i].butloc, h, v);
            else
                FrameRect(&bp->buts[i].butloc);
#else /* BILLBUTTONS */
            if(!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
                FrameRoundRect(&bp->buts[i].butloc, 10, 10);
            else
                FrameRect(&bp->buts[i].butloc);
#endif /* BILLBUTTONS */
        }

        for(done = 0; !done;)
        {
            C_GetNextEvent(mDownMask | keyDownMask, &evt);
            if(evt.what == CWC(mouseDown) || evt.what == CWC(keyDown))
            {
                p.h = CW(evt.where.h);
                p.v = CW(evt.where.v);
                for(i = 0; !done && i < CW(bp->nbut); i++)
                {
                    if(PtInRect(p, &bp->buts[i].butloc) || ((evt.what == CWC(keyDown)) && (((CL(evt.message) & charCodeMask) == '\r') || ((CL(evt.message) & charCodeMask) == NUMPAD_ENTER))))
                    {
                        if((pp = (struct pdef *)
                                findid(CW(bp->buts[i].butprocid))))
                            /* NOTE:  we will have to do a better
				      job here sometime */
                            (*(void (*)(void))MR(pp->proc))();
                        done = 1;
                    }
                }
                if(!done)
                    SysBeep(1);
            }
        }
        if(evt.what == CWC(mouseDown))
            while(!C_GetNextEvent(mUpMask, &evt))
                ;

        /* Move all buttons back. */
        for(i = 0; i < Cx(bp->nbut); i++)
            C_OffsetRect(&bp->buts[i].butloc, -offsetx, -offsety);
    }
}

/*
 * NOTE: The version of SysError below will only handle natively compiled
 *	 code.  When we want to be able to run arbitrary code we'll need
 *	 to call syn68k appropriately.
 */

void Executor::C_SysError(short errorcode)
{
    GrafPort alertport;
    Region viscliprgn;
    GUEST<RgnPtr> rp;
    Rect r;
    struct adef *ap;
    char quickbytes[grafSize];
    INTEGER offsetx, offsety;
    Rect main_gd_rect;

    LONGINT tmpa5;

    main_gd_rect = PIXMAP_BOUNDS(GD_PMAP(MR(LM(MainDevice))));

    if(!LM(DSAlertTab))
    {
#if defined(CLIFF_CENTERING_ALGORITHM)
        LM(DSAlertTab) = CL((Ptr)&myalerttab);
        LM(DSAlertRect).top = CWC(64);
        LM(DSAlertRect).left = CWC(32);
        LM(DSAlertRect).bottom = CWC(190);
        LM(DSAlertRect).right = CWC(480);
#else
        INTEGER screen_width = CW(main_gd_rect.right);
        INTEGER screen_height = CW(main_gd_rect.bottom);

        LM(DSAlertTab) = RM((Ptr)&myalerttab);
        LM(DSAlertRect).top = CW((screen_height - 126) / 3);
        LM(DSAlertRect).left = CW((screen_width - 448) / 2);
        LM(DSAlertRect).bottom = CW(CW(LM(DSAlertRect).top) + 126);
        LM(DSAlertRect).right = CW(CW(LM(DSAlertRect).left) + 448);
#endif

        offsetx = CW(LM(DSAlertRect).left) - 32;
        offsety = CW(LM(DSAlertRect).top) - 64;
    }
    else
    {
        offsetx = offsety = 0;
    }

    /* IM-362 */
    /* 1. Save registers and Stack Pointer */
    /*	  NOT DONE YET... signal handlers sort of do that anyway */

    /* 2. Store errorcode in LM(DSErrCode) */
    LM(DSErrCode) = CW(errorcode);

    /* 3. If no Cx(LM(DSAlertTab)), bitch */
    if(!LM(DSAlertTab))
    {
        write(2, "This machine thinks its a sadmac\n",
              sizeof("This machine thinks its a sadmac\n") - 1);
        exit(255);
    }

/* 4. Allocate and re-initialize QuickDraw */
    EM_A5 = US_TO_SYN68K(&tmpa5);
    LM(CurrentA5) = guest_cast<Ptr>(CL(EM_A5));
    InitGraf((Ptr)quickbytes + sizeof(quickbytes) - 4);
    ROMlib_initport(&alertport);
    SetPort(&alertport);
    InitCursor();
    rp = RM(&viscliprgn);
    alertport.visRgn = alertport.clipRgn = RM(&rp);
    viscliprgn.rgnSize = CWC(10);
    viscliprgn.rgnBBox = main_gd_rect;

    /* 5, 6. Draw alert box if the errorcode is >= 0 */

    if(errorcode < 0)
        errorcode = -errorcode;
    else
    {
        r = LM(DSAlertRect);
        FillRect(&r, white);
#if defined(OLDSTYLEALERT)
        r.right = CW(CW(r.right) - (2));
        r.bottom = CW(CW(r.bottom) - (2));
        FrameRect(&r);
        PenSize(2, 2);
        MoveTo(CW(r.left) + 2, CW(r.bottom));
        LineTo(CW(r.right), CW(r.bottom));
        LineTo(CW(r.right), CW(r.top) + 2);
        PenSize(1, 1);
#else /* OLDSTYLEALERT */
        FrameRect(&r);
        InsetRect(&r, 3, 3);
        PenSize(2, 2);
        FrameRect(&r);
        PenSize(1, 1);
#endif /* OLDSTYLEALERT */
    }

    /* find appropriate entry */

    ap = (struct adef *)findid(errorcode);
    if(!ap)
        ap = (struct adef *)((INTEGER *)MR(LM(DSAlertTab)) + 1);

    /* 7. text strings */
    drawtextstring(CW(ap->primetextid), offsetx, offsety);
    drawtextstring(CW(ap->secondtextid), offsetx, offsety);

    /* 8. icon */
    drawicon(CW(ap->iconid), offsetx, offsety);

    /* 9. TODO: figure out what to do with the proc ... */

    /* 10, 11, 12, 13. check for non-zero button id */
    /* #warning We blow off LM(ResumeProc) until we can properly handle it */
    if(ap->buttonid)
        dobuttons(/* CL(LM(ResumeProc)) ? Cx(ap->buttonid) + 1 : */ Cx(ap->buttonid),
                  offsetx, offsety);
}

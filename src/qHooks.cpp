/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/quick.h"
#include "rsys/hook.h"
#include "rsys/print.h"
#include "rsys/options.h"

using namespace Executor;


static bool text_is_enabled_p = true;

void
Executor::disable_stdtext(void)
{
    if(ROMlib_options & ROMLIB_TEXT_DISABLE_BIT)
        text_is_enabled_p = false;
}

void
Executor::enable_stdtext(void)
{
    text_is_enabled_p = true;
}

void Executor::ROMlib_CALLTEXT(INTEGER bc, Ptr bufp, Point num, Point den)
{
    QDProcsPtr gp;
    void (*pp)(INTEGER, Ptr, Point, Point);

    if(text_is_enabled_p)
    {
        if((gp = MR(thePort->grafProcs))
           && (pp = MR(gp->textProc)) != P_StdText)
        {
            ROMlib_hook(q_textprocnumber);
            CToPascalCall((void *)pp, ctop(&C_StdText), bc, bufp, num, den);
        }
        else
            C_StdText(bc, bufp, num, den);
    }
}

void Executor::ROMlib_CALLLINE(Point p)
{
    QDProcsPtr gp;
    void (*pp)(Point);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->lineProc)) != P_StdLine)
    {
        ROMlib_hook(q_lineprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdLine), p);
    }
    else
        C_StdLine(p);
}

void Executor::ROMlib_CALLRECT(GrafVerb v, Rect *rp)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, Rect *);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rectProc)) != P_StdRect)
    {
        ROMlib_hook(q_rectprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdRect), v, rp);
    }
    else
        C_StdRect(v, rp);
}

void Executor::ROMlib_CALLOVAL(GrafVerb v, Rect *rp)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, Rect *);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->ovalProc)) != P_StdOval)
    {
        ROMlib_hook(q_ovalprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdOval), v, rp);
    }
    else
        C_StdOval(v, rp);
}

void Executor::ROMlib_CALLRRECT(GrafVerb v, Rect *rp, INTEGER ow, INTEGER oh)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, Rect *, INTEGER, INTEGER);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rRectProc)) != P_StdRRect)
    {
        ROMlib_hook(q_rrectprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdRRect), v, rp, ow, oh);
    }
    else
        C_StdRRect(v, rp, ow, oh);
}

void Executor::ROMlib_CALLARC(GrafVerb v, Rect *rp, INTEGER starta, INTEGER arca)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, Rect *, INTEGER, INTEGER);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->arcProc)) != P_StdArc)
    {
        ROMlib_hook(q_arcprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdArc), v, rp, starta, arca);
    }
    else
        C_StdArc(v, rp, starta, arca);
}

void Executor::ROMlib_CALLRGN(GrafVerb v, RgnHandle rh)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, RgnHandle);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rgnProc)) != P_StdRgn)
    {
        ROMlib_hook(q_rgnprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdRgn), v, rh);
    }
    else
        C_StdRgn(v, rh);
}

void Executor::ROMlib_CALLPOLY(GrafVerb v, PolyHandle rh)
{
    QDProcsPtr gp;
    void (*pp)(GrafVerb, PolyHandle);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->polyProc)) != P_StdPoly)
    {
        ROMlib_hook(q_polyprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdPoly), v, rh);
    }
    else
        C_StdPoly(v, rh);
}

void Executor::ROMlib_CALLBITS(BitMap *bmp, const Rect *srcrp, const Rect *dstrp,
                               INTEGER mode, RgnHandle maskrh)
{
    QDProcsPtr gp;
    void (*pp)(BitMap *, Rect *, Rect *, INTEGER, RgnHandle);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->bitsProc)) != P_StdBits)
    {
        ROMlib_hook(q_bitsprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdBits), bmp, srcrp, dstrp, mode, maskrh);
    }
    else
        C_StdBits(bmp, srcrp, dstrp, mode, maskrh);
}

void Executor::ROMlib_CALLCOMMENT(INTEGER kind, INTEGER size, Handle datah)
{
    QDProcsPtr gp;
    void (*pp)(INTEGER, INTEGER, Handle);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->commentProc)) != P_StdComment)
    {
        ROMlib_hook(q_commentprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdComment), kind, size, datah);
    }
    else
        C_StdComment(kind, size, datah);
}

INTEGER
Executor::ROMlib_CALLTXMEAS(INTEGER bc, Ptr bufp, GUEST<Point> *nump, GUEST<Point> *denp,
                            FontInfo *fip)
{
    QDProcsPtr gp;
    txMeasProc_t pp;
    INTEGER retval;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->txMeasProc)) != P_StdTxMeas)
    {
        ROMlib_hook(q_txmeasprocnumber);
        retval = CToPascalCall((void *)pp, ctop(&C_StdTxMeas), bc, bufp,
                               nump, denp, fip);
    }
    else
        retval = C_StdTxMeas(bc, bufp, nump, denp, fip);
    return retval;
}

void Executor::ROMlib_PICWRITE(Ptr addr, INTEGER count)
{
    QDProcsPtr gp;
    void (*pp)(Ptr, INTEGER);

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->putPicProc)) != P_StdPutPic)
    {
        ROMlib_hook(q_putpicprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdPutPic), addr, count);
    }
    else
        C_StdPutPic(addr, count);
}


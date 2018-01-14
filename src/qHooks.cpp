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
    textProc_t pp;

    if(text_is_enabled_p)
    {
        if((gp = MR(thePort->grafProcs))
           && (pp = MR(gp->textProc)) != &StdText)
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
    lineProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->lineProc)) != &StdLine)
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
    rectProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rectProc)) != &StdRect)
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
    ovalProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->ovalProc)) != &StdOval)
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
    rRectProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rRectProc)) != &StdRRect)
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
    arcProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->arcProc)) != &StdArc)
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
    rgnProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->rgnProc)) != &StdRgn)
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
    polyProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->polyProc)) != &StdPoly)
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
    bitsProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->bitsProc)) != &StdBits)
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
    commentProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->commentProc)) != &StdComment)
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
       && (pp = MR(gp->txMeasProc)) != &StdTxMeas)
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
    putPicProc_t pp;

    if((gp = MR(thePort->grafProcs))
       && (pp = MR(gp->putPicProc)) != &StdPutPic)
    {
        ROMlib_hook(q_putpicprocnumber);
        CToPascalCall((void *)pp, ctop(&C_StdPutPic), addr, count);
    }
    else
        C_StdPutPic(addr, count);
}


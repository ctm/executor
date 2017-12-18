/* Copyright 1986, 1989, 1990, 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/mman.h"

using namespace Executor;

PUBLIC PicHandle Executor::ROMlib_OpenPicture_helper(const Rect *pf,
                                                     const OpenCPicParams *params)
{
    piccachehand pch;
    PicHandle ph;
    GUEST<INTEGER> *ip;
    GUEST<RgnHandle> temprh;

    HidePen();
    pch = (piccachehand)NewHandle(sizeof(piccache));
    PORT_PIC_SAVE_X(thePort) = RM((Handle)pch);
    ph = (PicHandle)NewHandle((Size)INITIALPICSIZE);
    HxX(pch, pichandle) = RM(ph);

    HxX(ph, picSize) = CWC(10 + 16 * sizeof(INTEGER));
    HxX(ph, picFrame) = *pf;
    ip = &HxX(ph, picSize) + 5;
    ip[0] = CW(OP_Version);
    ip[1] = CWC(0x2FF); /* see the explanation in IM-V p. 93 */

    ip[2] = CWC(0x0c00); /* see IM:Imaging with QuickDraw A-22 - A-24 */
    if(params)
    {
        ip[3] = params->version;
        ip[4] = params->reserved1;
        *(GUEST<uint32_t> *)&ip[5] = params->hRes;
        *(GUEST<uint32_t> *)&ip[7] = params->vRes;
        memcpy(&ip[9], &params->srcRect, sizeof params->srcRect);
        *(GUEST<uint32_t> *)&ip[13] = params->reserved2;
    }
    else
    {
        ip[3] = CWC(0xffff);
        ip[4] = CWC(0xffff);
        ip[5] = pf->left;
        ip[6] = CWC(0x0000);
        ip[7] = pf->top;
        ip[8] = CWC(0x0000);
        ip[9] = pf->right;
        ip[10] = CWC(0x0000);
        ip[11] = pf->bottom;
        ip[12] = CWC(0x0000);
        ip[13] = CWC(0x0000);
        ip[14] = CWC(0x0000);
    }

    ip[15] = CWC(0x001e);

    HxX(pch, picsize) = CLC(INITIALPICSIZE);
    HxX(pch, pichowfar) = CL((LONGINT)Hx(ph, picSize));
    temprh = RM(NewRgn());
    HxX(pch, picclip) = temprh;
    /* -32766 below is an attempt to force a reload */
    SetRectRgn(HxP(pch, picclip), -32766, -32766, 32767, 32767);
    PATASSIGN(HxX(pch, picbkpat), white);
    HxX(pch, picfont) = 0;
    HxX(pch, picface) = 0;
    HxX(pch, picfiller) = 0;
    HxX(pch, pictxmode) = CWC(srcOr);
    HxX(pch, pictxsize) = 0;
    HxX(pch, picspextra) = 0;
    HxX(pch, pictxnum.h) = CWC(1);
    HxX(pch, pictxnum.v) = CWC(1);
    HxX(pch, pictxden.h) = CWC(1);
    HxX(pch, pictxden.v) = CWC(1);
    HxX(pch, picdrawpnloc.h) = 0;
    HxX(pch, picdrawpnloc.v) = 0;
    HxX(pch, pictextpnloc.h) = 0;
    HxX(pch, pictextpnloc.v) = 0;
    HxX(pch, picpnsize.h) = CWC(1);
    HxX(pch, picpnsize.v) = CWC(1);
    HxX(pch, picpnmode) = CWC(patCopy);
    PATASSIGN(HxX(pch, picpnpat), black);
    PATASSIGN(HxX(pch, picfillpat), black);
    HxX(pch, piclastrect.top) = 0;
    HxX(pch, piclastrect.left) = 0;
    HxX(pch, piclastrect.bottom) = 0;
    HxX(pch, piclastrect.right) = 0;
    HxX(pch, picov.v) = 0;
    HxX(pch, picov.h) = 0;
    HxX(pch, picidunno) = 0;
    HxX(pch, picforeColor) = CLC(blackColor);
    HxX(pch, picbackColor) = CLC(whiteColor);
    return (ph);
}

P1(PUBLIC pascal trap, PicHandle, OpenPicture, Rect *, pf)
{
    PicHandle retval;

    retval = ROMlib_OpenPicture_helper(pf, NULL);
    return retval;
}

PRIVATE void updateclip(void)
{
    piccachehand pch;
    SignedByte state;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(!EqualRgn(PORT_CLIP_REGION(thePort), HxP(pch, picclip)))
    {
        CopyRgn(PORT_CLIP_REGION(thePort), HxP(pch, picclip));
        PICOP(OP_Clip);
        state = HGetState((Handle)PORT_CLIP_REGION(thePort));
        HLock((Handle)PORT_CLIP_REGION(thePort));
        PICWRITE(STARH(PORT_CLIP_REGION(thePort)),
                 Hx(PORT_CLIP_REGION(thePort), rgnSize));
        HSetState((Handle)PORT_CLIP_REGION(thePort), state);
    }
}

PRIVATE BOOLEAN EqualPat(Pattern pat1, Pattern pat2)
{
    LONGINT *lp1, *lp2;

    lp1 = (LONGINT *)pat1;
    lp2 = (LONGINT *)pat2;
    return lp1[0] == lp2[0] && lp1[1] == lp2[1];
}

PRIVATE void updateapat(Pattern srcpat, Pattern dstpat, INTEGER opcode)
{
    if(!EqualPat(srcpat, dstpat))
    {
        PATASSIGN(dstpat, srcpat);
        PICOP(opcode);
        PICWRITE(srcpat, sizeof(Pattern));
    }
}

PRIVATE void updateaninteger(INTEGER src, GUEST<INTEGER> *dstp, INTEGER opcode)
{
    GUEST<INTEGER> gsrc = CW(src);
    if(*dstp != gsrc)
    {
        *dstp = gsrc;
        PICOP(opcode);
        PICWRITE(&gsrc, sizeof(INTEGER));
    }
}

PRIVATE void updatealongint(const GUEST<LONGINT> *srcp, GUEST<LONGINT> *dstp,
                            INTEGER opcode)
{
    if(*dstp != *srcp)
    {
        *dstp = *srcp;
        PICOP(opcode);
        PICWRITE(srcp, sizeof(LONGINT));
    }
}

PRIVATE void updatebkpat(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    /* #warning Questionable code in updatebkpat */
    /* FIXME */
    if(CGrafPort_p(thePort))
        updateapat(PIXPAT_1DATA(CPORT_BK_PIXPAT(thePort)),
                   HxX(pch, picbkpat), OP_BkPat);
    else
        updateapat(PORT_BK_PAT(thePort), HxX(pch, picbkpat), OP_BkPat);
}

PRIVATE void updatepnpat(void)
{
    piccachehand pch;

    pch = (piccachehand)MR(thePort->picSave);
    /* #warning Questionable code in updatepnpat */
    /* FIXME */
    if(CGrafPort_p(thePort))
        updateapat(PIXPAT_1DATA(CPORT_PEN_PIXPAT(thePort)),
                   HxX(pch, picpnpat), OP_PnPat);
    else
        updateapat(PORT_PEN_PAT(thePort), HxX(pch, picpnpat), OP_PnPat);
}

PRIVATE void updatefillpat(void)
{
    piccachehand pch;

    pch = (piccachehand)MR(thePort->picSave);
    if(CGrafPort_p(thePort))
        /* #warning Questionable code in updatefillpat */
        updateapat(PIXPAT_1DATA(CPORT_FILL_PIXPAT(thePort)),
                   HxX(pch, picfillpat), OP_FillPat);
    else
        updateapat(PORT_FILL_PAT(thePort), HxX(pch, picfillpat), OP_FillPat);
}

PRIVATE void updatefont(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updateaninteger(PORT_TX_FONT(thePort), &HxX(pch, picfont), OP_TxFont);
}

PRIVATE void updatetxmode(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updateaninteger(PORT_TX_MODE(thePort), &HxX(pch, pictxmode), OP_TxMode);
}

PRIVATE void updatetxsize(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updateaninteger(PORT_TX_SIZE(thePort), &HxX(pch, pictxsize), OP_TxSize);
}

PRIVATE void updatepnmode(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updateaninteger(PORT_PEN_MODE(thePort), &HxX(pch, picpnmode), OP_PnMode);
}

PRIVATE void updateforeColor(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(CGrafPort_p(thePort))
    {
        /* ### this isn't quite correct since we only want to write the
	 current color if it has changed.  that requires we record the
	 current color, but the current `piccache' doesn't contain
	 that info */
        PICOP(OP_RGBFgCol);
        PICWRITE(&CPORT_RGB_FG_COLOR(thePort),
                 sizeof CPORT_RGB_FG_COLOR(thePort));
    }
    else
        updatealongint(&PORT_FG_COLOR_X(thePort), &HxX(pch, picforeColor),
                       OP_FgColor);
}

PRIVATE void updatebackColor(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(CGrafPort_p(thePort))
    {
        /* ### see comment in `updateforeColor ()' */
        PICOP(OP_RGBBkCol);
        PICWRITE(&CPORT_RGB_BK_COLOR(thePort),
                 sizeof CPORT_RGB_BK_COLOR(thePort));
    }
    else
        updatealongint(&PORT_BK_COLOR_X(thePort),
                       &HxX(pch, picbackColor),
                       OP_BkColor);
}

PRIVATE void updatespextra(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updatealongint(&PORT_SP_EXTRA_X(thePort), &HxX(pch, picspextra), OP_SpExtra);
}

PRIVATE void updatetxnumtxden(Point num, Point den)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(Hx(pch, pictxnum.h) != num.h || Hx(pch, pictxnum.v) != num.v || Hx(pch, pictxden.v) != den.v || Hx(pch, pictxden.h) != den.h)
    {
        HxX(pch, pictxnum.h) = CW(num.h);
        HxX(pch, pictxnum.v) = CW(num.v);
        HxX(pch, pictxden.h) = CW(den.h);
        HxX(pch, pictxden.v) = CW(den.v);
        PICOP(OP_TxRatio);
        GUEST<Point> tmpP;
        tmpP.set(num);
        PICWRITE(&tmpP, sizeof(tmpP));
        tmpP.set(den);
        PICWRITE(&tmpP, sizeof(tmpP));
    }
}

PRIVATE void updatepnsize(void)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    updatealongint((GUEST<LONGINT> *)&thePort->pnSize,
                   (GUEST<LONGINT> *)&HxX(pch, picpnsize), OP_PnSize);
}

PRIVATE void updatetxface(void)
{
    piccachehand pch;
    Style f;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(HxX(pch, picface) != PORT_TX_FACE_X(thePort))
    {
        HxX(pch, picface) = PORT_TX_FACE_X(thePort);
        f = PORT_TX_FACE(thePort);
        PICOP(OP_TxFace);
        PICWRITE(&f, sizeof(f));
        if(sizeof(f) & 1)
            PICWRITE("", 1); /* even things up */
    }
}

PRIVATE void updateoval(GUEST<Point> *ovp)
{
    piccachehand pch;

    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    if(HxX(pch, picov.h) != ovp->h || HxX(pch, picov.v) != ovp->v)
    {
        HxX(pch, picov) = *ovp;
        PICOP(OP_OvSize);
        PICWRITE(ovp, sizeof(*ovp));
    }
}

PUBLIC void Executor::ROMlib_textpicupdate(Point num, Point den)
{
    updateclip();
    updatefont();
    updatetxface();
    updatetxmode();
    updatetxsize();
    updatespextra();
    updatetxnumtxden(num, den);
    updateforeColor();
    updatebackColor();
}

PUBLIC void Executor::ROMlib_drawingpicupdate(void)
{
    updateclip();
    updatepnsize();
    updatepnmode();
    updatepnpat();
    updateforeColor();
    updatebackColor();
}

PUBLIC void Executor::ROMlib_drawingverbpicupdate(GrafVerb v)
{
    ROMlib_drawingpicupdate();
    switch(v)
    {
        case erase:
            updatebkpat();
            break;
        case fill:
            updatefillpat();
            break;
        case invert:
            if(!(HiliteMode & 0x80))
            {
                PICOP(OP_HiliteColor);
                PICWRITE(&HiliteRGB, sizeof HiliteRGB);
                PICOP(OP_HiliteMode);
            }
            break;
        default:
            break;
    }
}

PUBLIC void Executor::ROMlib_drawingverbrectpicupdate(GrafVerb v, Rect *rp)
{
    piccachehand pch;

    ROMlib_drawingverbpicupdate(v);
    pch = (piccachehand)PORT_PIC_SAVE(thePort);
    HxX(pch, piclastrect) = *rp; /* currently unused */
}

PUBLIC void Executor::ROMlib_drawingverbrectovalpicupdate(GrafVerb v, Rect *rp,
                                                          GUEST<Point> *ovp)
{
    ROMlib_drawingverbrectpicupdate(v, rp);
    updateoval(ovp);
}

P0(PUBLIC pascal trap, void, ClosePicture)
{
    piccachehand pch;

    if((pch = (piccachehand)PORT_PIC_SAVE(thePort)))
    {
        PicHandle ph;

        PICOP(OP_EndPic);
        ph = HxP(pch, pichandle);
        SetHandleSize((Handle)ph, Hx(pch, pichowfar));
        DisposeRgn(HxP(pch, picclip));
        DisposHandle((Handle)pch);
        PORT_PIC_SAVE_X(thePort) = nullptr;
        ShowPen();
    }
}

P3(PUBLIC pascal trap, void, PicComment, INTEGER, kind, INTEGER, size,
   Handle, hand)
{
    CALLCOMMENT(kind, size, hand);
}

P3(PUBLIC pascal trap, void, ReadComment, INTEGER, kind, INTEGER, size,
   Handle, hand)
{
    CALLCOMMENT(kind, size, hand);
}

/* look in qPicStuff.c for DrawPicture */

P1(PUBLIC pascal trap, void, KillPicture, PicHandle, pic)
{
    /*
 * It's not clear what the Mac does in the case below.  We really should
 * test exactly how the Mac deals with DisposHandle being called on resource
 * handles.
 */
    SignedByte state;

    state = HGetState((Handle)pic);
    if(!(state & RSRCBIT))
        DisposHandle((Handle)pic);
}

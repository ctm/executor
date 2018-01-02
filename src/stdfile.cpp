/*
 * NOTE: currently we only do an automount when crossing a filesystem.
 *	 This is might be bad:  currently we're seeing an error when
 *	 we try to open "/tmp" on roland, because 4 has already been
 *	 specified as "/Net" from iclone.  I'm really not convinced
 *	 that the proper solution is to do an automount, though.
 */

/* Copyright 1987 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in StdFilePkg.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#if defined(LINUX)
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#endif

#include "DialogMgr.h"
#include "FileMgr.h"
#include "EventMgr.h"
#include "StdFilePkg.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "ResourceMgr.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"
#include "MemoryMgr.h"
#include "OSUtil.h"
#include "ScriptMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/stdfile.h"
#include "rsys/arrowkeys.h"
#include "rsys/glue.h"
#include "rsys/resource.h"
#include "rsys/pstuff.h"
#include "rsys/hfs.h"
#include "rsys/time.h"
#include "rsys/flags.h"
#include "rsys/tempalloc.h"
#include "rsys/hook.h"
#include "rsys/toolevent.h"
#include "rsys/string.h"
#include "rsys/dcache.h"
#include "rsys/menu.h"
#include "rsys/stdfile.h"

#include "rsys/executor.h"

#include "rsys/osevent.h"

PUBLIC int Executor::nodrivesearch_p = false;

#if defined(MSDOS) || defined(CYGWIN32)
#include "dosdisk.h"
#include "aspi.h"
#include "rsys/checkpoint.h"
#endif

#include "rsys/print.h"
#include "rsys/system_error.h"

using namespace Executor;

typedef union {
    SFReply *oreplyp;
    StandardFileReply *nreplyp;
} reply_u;

typedef union {
    FileFilterProcPtr oflfilef;
    FileFilterYDProcPtr cflfilef;
} file_filter_u;

typedef union {
    ModalFilterProcPtr ofilterp;
    ModalFilterYDProcPtr cfilterp;
} filter_u;

typedef union {
    DlgHookProcPtr odh;
    DlgHookYDProcPtr cdh;
} dialog_hook_u;

typedef struct
{
    struct flinfostr
    {
        INTEGER floffs;
        INTEGER flicns;
    };

    sf_flavor_t flavor;
    reply_u flreplyp;
    ControlHandle flsh;
    INTEGER flnmfil;
    INTEGER flnmlin;
    INTEGER fllinht;
    INTEGER flascent;
    INTEGER flsel;
    Rect flrect;
    Rect flcurdirrect;
    GUEST<flinfostr *> *flinfo;
    GUEST<char *> *flstrs;
    file_filter_u flfilef;
    INTEGER flnumt;
    GUEST<OSType> *fltl;
    ControlHandle flch;
    INTEGER flgraynondirs;
    Str255 flcurdirname;
    filter_u magicfp;
    INTEGER fl_cancel_item;
    void* mydata;
} fltype;

PRIVATE INTEGER movealert(INTEGER);
PRIVATE void drawminiicon(INTEGER);
PRIVATE void drawinboxwithicon(StringPtr, Rect *, INTEGER);
PRIVATE void safeflflip(fltype *, INTEGER);
PRIVATE void flupdate(fltype *, INTEGER, INTEGER);
PRIVATE void flscroll(fltype *, INTEGER, INTEGER);
PRIVATE StringPtr getdiskname(BOOLEAN *, bool *);
PRIVATE void drawjobberattop(DialogPeek);
PRIVATE LONGINT getdirid(StringPtr);
PRIVATE void settype(fltype *, INTEGER);
PRIVATE INTEGER flwhich(fltype *, Point);
PRIVATE void flmouse(fltype *, Point, ControlHandle);
PRIVATE void getcurname(fltype *);
PRIVATE void flfinit(fltype *);
PRIVATE void flinsert(fltype *, StringPtr, INTEGER);
PRIVATE int typeinarray(GUEST<OSType>, INTEGER, GUEST<SFTypeList>);
PRIVATE LONGINT stdfcmp(char *, char *);
PRIVATE void flfill(fltype *);
PRIVATE void realcd(DialogPeek, LONGINT);
PRIVATE LONGINT getparent(LONGINT);
PRIVATE BOOLEAN moveuponedir(DialogPtr);
PRIVATE void flinit(fltype *, Rect *, ControlHandle);
PRIVATE void stdfflip(Rect *, INTEGER, INTEGER);
PRIVATE BOOLEAN trackdirs(DialogPeek);
PRIVATE BOOLEAN ejected(HParmBlkPtr);
PRIVATE void bumpsavedisk(DialogPtr, BOOLEAN);
PRIVATE void transformsfpdialog(DialogPtr, Point *, Rect *, BOOLEAN);
PRIVATE void doeject(DialogPtr);
PRIVATE GUEST<OSType> gettypeX(StringPtr, INTEGER, LONGINT);


PRIVATE INTEGER movealert(INTEGER id)
{
    Handle h;
    INTEGER dh, dv;
    Rect *rp;

    h = GetResource(TICK("ALRT"), id);
    if(!*h)
        LoadResource(h);
    rp = (Rect *)STARH(h);
    dh = CW(rp->right) - CW(rp->left);
    dv = CW(rp->bottom) - CW(rp->top);
    rp->left = CWC(150);
    rp->top = CWC(30);
    rp->right = CW(150 + dh);
    rp->bottom = CW(30 + dv);
    return (Alert(id, (ProcPtr)0));
}

#define SICONS -15744

PRIVATE void drawminiicon(INTEGER icon)
{
    Handle h;
    BitMap bm;
    Rect r;

    h = ROMlib_getrestid(TICK("SICN"), SICONS);
    if(!h)
        /*-->*/ return; /* badness */
    HLock(h);
    bm.baseAddr = RM((Ptr)STARH(h) + icon * 16 * 16 / 8);
    bm.rowBytes = CWC(2);
    bm.bounds.left = bm.bounds.top = CWC(0);
    bm.bounds.right = bm.bounds.bottom = CWC(16);
    r.top = PORT_PEN_LOC(thePort).v;
    r.left = PORT_PEN_LOC(thePort).h;
    r.bottom = CW(CW(r.top) + 16);
    r.right = CW(CW(r.left) + 16);
    CopyBits(&bm, PORT_BITS_FOR_COPY(thePort),
             &bm.bounds, &r, srcCopy, NULL);
    HUnlock(h);
}

/*
 * NOTE:  the Rect pointed to below has a normal top, left and right,
 *	  BUT the bottom is the amount to add to the top for drawing
 *	  text, not the real bottom.
 */

PRIVATE void drawinboxwithicon(StringPtr str, Rect * rp, INTEGER icon)
{
    Rect r;
    INTEGER width, strlen, lengthavail;
    GUEST<INTEGER> strwidths[255], *widp;
    static const char *ellipsis = "\003...";

    /*
     * Note:  the ellipsis code is a bit shabby, not truncating at a character
     *	      boundary, nor leaving space on either side, but for now, it's
     *	      ok.
     */

    MoveTo(CW(rp->left) + 2, CW(rp->top));
    drawminiicon(icon);
    MoveTo(CW(rp->left) + 2 + 16 + 3, CW(rp->top) + CW(rp->bottom) - 1); /* see note above */
    r.left = rp->left;
    r.right = rp->right;
    r.top = rp->top;
    r.bottom = CWC(32766);
    ClipRect(&r);
    strlen = *str;
    MeasureText(strlen, (Ptr)(str + 1), (Ptr)strwidths);
    lengthavail = CW(rp->right) - (CW(rp->left) + 2 + 16 + 3);
    if(CW(strwidths[strlen]) > lengthavail)
    {
        width = StringWidth((StringPtr)ellipsis);
        /* 4 might be the space on the right of the ellipsis. */
        /* TODO: figure out exactly what the number is. */
        lengthavail -= (width + 4);
        widp = strwidths;
        while(CW(*++widp) < lengthavail)
            DrawChar(*++str);
        DrawString((StringPtr)ellipsis);
    }
    else
        DrawString(str);
    r.top = CWC(-32767);
    r.left = CWC(-32767);
    r.right = CWC(32766);
    ClipRect(&r);
}

PRIVATE void safeflflip(fltype * f, INTEGER sel)
{
    Rect r;
    INTEGER fltop = GetCtlValue(f->flsh);

    if(sel >= fltop && sel < fltop + f->flnmlin)
    {
        r.left = f->flrect.left;
        r.right = f->flrect.right;
        r.top = CW(CW(f->flrect.top) + (sel - fltop) * f->fllinht);
        r.bottom = CW(CW(r.top) + f->fllinht);
        if(EmptyRgn(MR(((WindowPeek)thePort)->updateRgn))) /* stuff to draw? */
            InvertRect(&r); /* no: we can flip */
        else
            InvalRect(&r); /* yes: flip later */
    }
}

#define GRAYBIT (1 << 5)
#define ICONMASK (~GRAYBIT)

PRIVATE void flupdate(fltype * f, INTEGER st, INTEGER n)
{
    INTEGER i;
    fltype::flinfostr *ip;
    INTEGER sel = f->flsel;
    INTEGER fltop;
    Rect r;

    fltop = GetCtlValue(f->flsh);
    r.top = CW(CW(f->flrect.top) + (st - fltop) * f->fllinht);
    r.bottom = CW(f->flascent);
    r.left = f->flrect.left;
    r.right = f->flrect.right;

    ip = MR(*f->flinfo) + st;
    HLock((Handle)f->flstrs);
    for(i = st; i < st + n && i < fltop + f->flnmlin && i < f->flnmfil; i++)
    {
        drawinboxwithicon((StringPtr)(MR(*f->flstrs) + ip->floffs), &r,
                          ip->flicns & ICONMASK);
        if(ip->flicns & GRAYBIT)
        {
            r.bottom = CW(CW(r.top) + f->fllinht);
            PenMode(notPatBic);
            PenPat(gray);
            PaintRect(&r);
            PenPat(black);
            PenMode(patCopy);
            r.bottom = CW(f->flascent);
        }
        ip++;
        r.top = CW(CW(r.top) + (f->fllinht));
    }
    HUnlock((Handle)f->flstrs);
    if(sel >= st && sel < st + n)
        safeflflip(f, sel);
}

PRIVATE void flscroll(fltype * f, INTEGER from, INTEGER to)
{
    RgnHandle rh;
    INTEGER toscroll;

    if(from != to)
    {
        rh = NewRgn();
        ScrollRect(&f->flrect, 0, (from - to) * f->fllinht, rh);
        if(to > from)
        {
            toscroll = to - from;
            if(toscroll >= f->flnmlin)
                flupdate(f, to, f->flnmlin);
            else
                flupdate(f, from + f->flnmlin, toscroll);
        }
        else
        {
            toscroll = from - to;
            if(toscroll > f->flnmlin)
                toscroll = f->flnmlin;
            flupdate(f, to, toscroll);
        }
        DisposeRgn(rh);
    }
}

#define CTLFL(sh) \
    (MR(guest_cast<fltype *>(((WindowPeek)MR(STARH((sh))->contrlOwner))->refCon)))

/*
 * this hack is necessary because Excel 4 can bring up a dialog on top of
 * stdfile at a time when the refcon has been set to 'stdf'.
 */

static LONGINT emergency_save_ref_con;

PRIVATE fltype *
WINDFL(void *dp)
{
    GUEST<LONGINT> retval;

    retval = ((WindowPeek)dp)->refCon;
    if(retval == TICKX("stdf"))
        return (fltype *)SYN68K_TO_US(emergency_save_ref_con);
    else
        return MR(guest_cast<fltype *>(retval));
}

#if 0
#define WINDFL(dp) \
    ((fltype *)(long)MR(((WindowPeek)dp)->refCon))
#endif

PUBLIC pascal void Executor::C_ROMlib_stdftrack(ControlHandle sh, INTEGER part)
{
    const uint32_t min_between_scroll_msecs = 100;
    static uint32_t last_scroll_msecs;
    uint32_t current_msecs;
    int16_t from, pg;

    current_msecs = msecs_elapsed();

    if(current_msecs - min_between_scroll_msecs < last_scroll_msecs)
        return;
    else
        last_scroll_msecs = current_msecs;

    from = GetCtlValue(sh);
    pg = CTLFL(sh)->flnmlin - 1;
    switch(part)
    {
        case inUpButton:
            SetCtlValue(sh, from - 1);
            break;
        case inDownButton:
            SetCtlValue(sh, from + 1);
            break;
        case inPageUp:
            SetCtlValue(sh, from - pg);
            break;
        case inPageDown:
            SetCtlValue(sh, from + pg);
            break;
    }
    flscroll(CTLFL(sh), from, GetCtlValue(sh));
}

PRIVATE GUEST<INTEGER> cachedvrn = CWC(32767);
PRIVATE INTEGER savesel = -1;
PRIVATE LONGINT oldticks = -1000;
PRIVATE LONGINT lastkeydowntime = 0;
PRIVATE Str255 prefix = { 0 };
PRIVATE GUEST<char *> *holdstr;

PUBLIC void Executor::ROMlib_init_stdfile(void)
{
    cachedvrn = CWC(32767);
    savesel = -1;
    oldticks = -1000;
    lastkeydowntime = 0;
    prefix[0] = 0;
    holdstr = 0;
}

PRIVATE StringPtr getdiskname(BOOLEAN * ejectablep, bool * writablep)
{
    static BOOLEAN ejectable;
    static bool writable;
    static Str255 retval;
    ParamBlockRec pbr;

    if(SFSaveDisk != cachedvrn)
    {
        OSErr err;

        pbr.volumeParam.ioNamePtr = RM(&retval[0]);
        pbr.volumeParam.ioVolIndex = 0;
        pbr.volumeParam.ioVRefNum = CW(-CW(SFSaveDisk));
        err = PBGetVInfo(&pbr, false);
        cachedvrn = SFSaveDisk;
        if(err == noErr)
        {
            ejectable = !(pbr.volumeParam.ioVAtrb & CWC(VNONEJECTABLEBIT));
            writable = !(pbr.volumeParam.ioVAtrb & CWC(VHARDLOCKBIT | VSOFTLOCKBIT));
            if(writable && pbr.volumeParam.ioVFrBlk == CWC(0))
                writable = false;
        }
        else
        {
            warning_unexpected("surprising PBGetVInfo retval = %d\n", err);
            ejectable = false;
            writable = true;
        }
    }
    if(ejectablep)
        *ejectablep = ejectable;
    if(writablep)
        *writablep = writable;
    return retval;
}

PRIVATE void drawjobberattop(DialogPeek dp)
{
    INTEGER icon;
    Rect *rp;
    fltype *flp;
    GUEST<INTEGER> savebottom;
    BOOLEAN ejectable;
    PenState ps;

    GetPenState(&ps);
    PenNormal();
    if(CL(CurDirStore) == 2)
    {
#if 1
        /* TODO: ask cliff about a better way to do this */
        /* unused = */ getdiskname(&ejectable, NULL);
        icon = ejectable ? MICONFLOPPY : MICONDISK;
#else /* 0 */
        icon = MICONDISK;
#endif /* 0 */
    }
    else
        icon = MICONOFOLDER;
    flp = WINDFL(dp);
    rp = &flp->flcurdirrect;
    savebottom = rp->bottom;
    rp->bottom = CW(flp->flascent);
    rp->left = CW(CW(rp->left) + (2));
    drawinboxwithicon(flp->flcurdirname, rp, icon);
    rp->left = CW(CW(rp->left) - (2));
    rp->bottom = savebottom;
    FrameRect(rp);
    MoveTo(CW(rp->left) + 1, CW(rp->bottom));
    LineTo(CW(rp->right), CW(rp->bottom));
    LineTo(CW(rp->right), CW(rp->top) + 1);
    SetPenState(&ps);
}

PRIVATE LONGINT getdirid(StringPtr fname)
{
    CInfoPBRec hpb;
    LONGINT retval;
    OSErr err;

    hpb.dirInfo.ioCompletion = 0;
    hpb.dirInfo.ioNamePtr = RM(fname);
    hpb.dirInfo.ioVRefNum = CW(-CW(SFSaveDisk));
    hpb.dirInfo.ioFDirIndex = CWC(0);
    hpb.dirInfo.ioDrDirID = CurDirStore;
    err = PBGetCatInfo(&hpb, false);
    if(err == noErr)
        retval = CL(hpb.dirInfo.ioDrDirID);
    else
    {
        warning_unexpected("PBGetCatInfo return value err = %d\n", err);
        retval = 2; /* known good id ... an error might be better here */
    }
    return retval;
}

#define MAXPREFIX 64

PRIVATE void
set_type_and_name(fltype *f, OSType type, Str255 name)
{
    switch(f->flavor)
    {
        case original_sf:
            f->flreplyp.oreplyp->fType = CL(type);
            str31assign(f->flreplyp.oreplyp->fName, name);
            break;
        case new_sf:
        case new_custom_sf:
            f->flreplyp.nreplyp->sfType = CL(type);
            str31assign(f->flreplyp.nreplyp->sfFile.name, name);
            f->flreplyp.nreplyp->sfIsFolder = !!type;
            break;
        default:
            warning_unexpected("flavor = %d", f->flavor);
            break;
    }
}

PRIVATE void settype(fltype * f, INTEGER newsel)
{
    StringPtr ip;

    ip = (StringPtr)MR(*f->flstrs) + MR(*f->flinfo)[newsel].floffs;
    if(MR(*f->flinfo)[newsel].flicns == MICONCFOLDER)
        set_type_and_name(f, getdirid(ip), (StringPtr) "");
    else
        set_type_and_name(f, 0, ip);
}

PRIVATE INTEGER flwhich(fltype * f, Point p)
{
    INTEGER retval;
    INTEGER bump, from;

    if(!PtInRect(p, &f->flrect))
    {
        bump = 0;
        if(p.v < CW(f->flrect.top))
            bump = -1;
        else if(p.v >= CW(f->flrect.bottom))
            bump = 1;
        if(bump)
        {
            from = GetCtlValue(f->flsh);
            SetCtlValue(f->flsh, from + bump);
            flscroll(f, from, GetCtlValue(f->flsh));
        }
        /*-->*/ return (-1);
    }
    retval = (p.v - CW(f->flrect.top)) / f->fllinht + GetCtlValue(f->flsh);
    if(retval >= f->flnmfil || MR(*f->flinfo)[retval].flicns & GRAYBIT)
        /*-->*/ retval = -1;
    return (retval);
}

PRIVATE void flmouse(fltype * f, Point p, ControlHandle ch)
{
    INTEGER newsel;
    EventRecord evt;

    evt.where.set(p);
    do
    {
        GlobalToLocal(&evt.where);
        p = evt.where.get();
        if((newsel = flwhich(f, p)) != f->flsel)
        {
            if(f->flsel != -1)
            {
                safeflflip(f, f->flsel);
                if(newsel == -1)
                {
                    if(!f->flgraynondirs)
                        HiliteControl(ch, 255);
                    set_type_and_name(f, 0, (StringPtr) "");
                }
            }
            if(newsel != -1)
            {
                safeflflip(f, newsel);
                if(f->flsel == -1)
                    HiliteControl(ch, 0);
                settype(f, newsel);
            }
            f->flsel = newsel;
        }
    } while(!GetNextEvent(mUpMask, &evt));
}

PRIVATE void getcurname(fltype * f)
{
    int w;
    CInfoPBRec hpb;
    Rect *r;
    OSErr err;

    hpb.dirInfo.ioCompletion = 0;
    hpb.dirInfo.ioNamePtr = RM(&f->flcurdirname[0]);
    f->flcurdirname[0] = 0;
    hpb.dirInfo.ioVRefNum = CW(-CW(SFSaveDisk));
    hpb.dirInfo.ioFDirIndex = CWC(-1);
    hpb.dirInfo.ioDrDirID = CurDirStore;
    err = PBGetCatInfo(&hpb, false);
    if(err != noErr)
    {
        warning_unexpected("PBGetCatInfo err = %d\n", err);
        str255_from_c_string(f->flcurdirname, "?error?");
    }
    r = &f->flrect;
    w = StringWidth(f->flcurdirname) + 2 + 16 + 3 + 2 + 2 + 4;
#if 1
    if(w > CW(r->right) - CW(r->left) + 17)
    {
        f->flcurdirrect.left = r->left;
        f->flcurdirrect.right = CW(CW(r->right) + 17);
    }
    else
    {
        f->flcurdirrect.left = CW((CW(r->left) + CW(r->right) + 17 - w) / 2 - 2);
        f->flcurdirrect.right = CW(CW(f->flcurdirrect.left) + w);
    }
#else /* 1 */
    f->flcurdirrect.left = (CW(r->left) + CW(r->right) + 17 - w) / 2 - 2;
    f->flcurdirrect.right = CW(CW(f->flcurdirrect.left) + w);
#endif /* 1 */
}

PRIVATE void flfinit(fltype * fp)
{
    DisposeControl(fp->flsh);
    DisposHandle((Handle)fp->flinfo);
    DisposHandle((Handle)fp->flstrs);
}

PRIVATE void flinsert(fltype * f, StringPtr p, INTEGER micon)
{
    fltype::flinfostr finfo;

    finfo.floffs = GetHandleSize((Handle)f->flstrs);
    finfo.flicns = micon;
    PtrAndHand((Ptr)p, (Handle)f->flstrs, (LONGINT)U(p[0]) + 1);
    PtrAndHand((Ptr)&finfo, (Handle)f->flinfo,
               (LONGINT)sizeof(fltype::flinfostr));
    ++f->flnmfil;
}

PRIVATE int typeinarray(GUEST<OSType> ft, INTEGER numt, GUEST<SFTypeList> tl)
{
    GUEST<OSType> *ostp = tl;

    while(numt--)
        if(ft == *ostp++)
            /*-->*/ return (true);
    return (false);
}

/*
 * NOTE: stdfcmp is called from qsort, so we can't smash d2.
 */

/* ip1 and ip2 would really be "void *" in ANSI C */

static LONGINT stdfcmpC(const void *ip1, const void *ip2)
{
    fltype::flinfostr *fp1 = (fltype::flinfostr *)ip1;
    fltype::flinfostr *fp2 = (fltype::flinfostr *)ip2;
    LONGINT retval;

    retval = ROMlib_strcmp((StringPtr)(MR(*holdstr) + fp1->floffs),
                           (StringPtr)(MR(*holdstr) + fp2->floffs));
    return retval;
}

PRIVATE LONGINT stdfcmp(char * ip1, char * ip2)
{
    return stdfcmpC(ip1, ip2);
}

PRIVATE bool
passes_filter(fltype *f, CInfoPBRec *cinfop, INTEGER numt)
{
    bool retval;

    ROMlib_hook(stdfile_filefiltnumber);
    switch(f->flavor)
    {
        case original_sf:
            /* NOTE: the code for original_sf was changed after we ran into
	 some trouble with Accordance.  It's not clear whether or not
	 the new code here should also be used for new_sf.  */
            retval = (!f->flfilef.oflfilef
                      || (numt == 0
                          && (cinfop->hFileInfo.ioFlAttrib & ATTRIB_ISADIR))
                      || !f->flfilef.oflfilef(cinfop));
        case new_sf:
            retval = (!f->flfilef.oflfilef
                      || (cinfop->hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
                      || !f->flfilef.oflfilef(cinfop));
            break;
        case new_custom_sf:
            retval = (!f->flfilef.cflfilef
                      || !f->flfilef.cflfilef(cinfop, f->mydata));
            break;
        default:
            warning_unexpected("flavor = %d", f->flavor);
            retval = true;
            break;
    }
    return retval;
}

PRIVATE void flfill(fltype * f)
{
    CInfoPBRec pb;
    OSErr err;
    Str255 s;
    int micon;
    CursHandle watchh;
    INTEGER errcount;
    INTEGER dirindex;

    SetCursor(STARH((watchh = GetCursor(watchCursor))));

    pb.hFileInfo.ioNamePtr = RM(&s[0]);
    pb.hFileInfo.ioVRefNum = CW(-CW(SFSaveDisk));
    err = noErr;
    errcount = 0;
    for(dirindex = 1; err != fnfErr && errcount != 3; dirindex++)
    {
        pb.hFileInfo.ioFDirIndex = CW(dirindex);
        pb.hFileInfo.ioDirID = CurDirStore;
        err = PBGetCatInfo(&pb, false);
        if(err)
        {
            if(err != fnfErr)
            {
                warning_unexpected("PBGetCatInfo err = %d\n", err);
                ++errcount;
                /* int d7 = err; */
            }
        }
        else
        {
            errcount = 0;
            if(f->flnumt <= 0 || (pb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR) || typeinarray(pb.hFileInfo.ioFlFndrInfo.fdType, f->flnumt,
                                                                                          f->fltl))
                if(passes_filter(f, &pb, f->flnumt))
                {
                    if(pb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
                        micon = MICONCFOLDER;
                    else if(pb.hFileInfo.ioFlFndrInfo.fdType == TICKX("APPL"))
                        micon = MICONAPP | f->flgraynondirs;
                    else
                        micon = MICONLETTER | f->flgraynondirs;
                    flinsert(f, MR(pb.hFileInfo.ioNamePtr), micon);
                }
        }
    }
    if(f->flnmfil > f->flnmlin)
    {
        SetCtlMax(f->flsh, f->flnmfil - f->flnmlin);
        SetCtlValue(f->flsh, 0);
    }
    else
        SetCtlMax(f->flsh, 0);

    if(f->flnmfil > 0)
    {
        holdstr = f->flstrs;
        ::qsort(MR(*f->flinfo), f->flnmfil, sizeof(fltype::flinfostr), stdfcmpC);
        if(!(MR(*f->flinfo)[0].flicns & GRAYBIT) && !f->flgraynondirs)
        {
            f->flsel = 0;
            settype(f, 0);
            HiliteControl(f->flch, 0);
        }
    }
    else if(!f->flgraynondirs)
        HiliteControl(f->flch, 255);
    /* i'm not sure this is right, but there is no obvious way to
       save/restore the cursor */
    SetCursor(&arrowX);
}

/*
 * ROMlib_filebox should be renamed to reflect the fact that it also draws the
 * dotted line (whopee!)
 */

PUBLIC pascal void Executor::C_ROMlib_filebox(DialogPeek dp, INTEGER which)
{
    GUEST<Handle> h;
    Rect r, r2;
    GUEST<INTEGER> i;
    int width, strwidth, offset;
    StringPtr diskname;
    GUEST<Handle> tmpH;
    Handle ejhand;
    BOOLEAN ejectable;
    PenState ps;

    GetPenState(&ps);
    PenNormal();

    h = NULL;
    GetDItem((DialogPtr)dp, which, &i, &h, &r);
    /*    h = CL(h); we don't really use h */
    switch(which)
    {
        case getNmList:
        case putNmList:
            if(h)
                FrameRect(&r);
            flupdate(WINDFL(dp), GetCtlValue(WINDFL(dp)->flsh),
                     WINDFL(dp)->flnmlin);
            break;
        case getDotted:
            FillRect(&r, gray);
            break;
        case getDiskName:
            /*  case putDiskName:	getDiskName and putDiskName are the same */
            EraseRect(&r);
            width = CW(r.right) - CW(r.left);
            diskname = getdiskname(&ejectable, NULL);
            GetDItem((DialogPtr)dp, putEject, &i, &tmpH, &r2);
            ejhand = MR(tmpH);
            if(ejectable)
                HiliteControl((ControlHandle)ejhand, 0);
            else
                HiliteControl((ControlHandle)ejhand, 255);
            strwidth = StringWidth(diskname) + 2 + 16 + 3;
            offset = (width - strwidth) / 2;
            if(offset < 3)
                r.left = CW(CW(r.left) + (3));
            else
                r.left = CW(CW(r.left) + (offset));
            r.bottom = CW(WINDFL(dp)->flascent);
            drawinboxwithicon(diskname, &r, ejectable ? MICONFLOPPY : MICONDISK);
            break;
    }
    SetPenState(&ps);
}

PRIVATE void realcd(DialogPeek dp, LONGINT dir)
{
    fltype *fp;

    CurDirStore = CL(dir);
    fp = WINDFL(dp);
    SetHandleSize((Handle)fp->flinfo, (Size)0);
    SetHandleSize((Handle)fp->flstrs, (Size)0);
    fp->flnmfil = 0;
    fp->flsel = -1;
    set_type_and_name(fp, 0, (StringPtr) "");
    flfill(fp);
    fp->flcurdirrect.right = CW(CW(fp->flcurdirrect.right) + 1);
    fp->flcurdirrect.bottom = CW(CW(fp->flcurdirrect.bottom) + 1);
    EraseRect(&fp->flcurdirrect);
    getcurname(fp);
    fp->flcurdirrect.bottom = CW(CW(fp->flcurdirrect.bottom) - 1);
    /* don't need to do right; getcurname does */
    drawjobberattop(dp);
    C_ROMlib_filebox(dp, getDiskName);
    EraseRect(&fp->flrect);
    if(fp->flgraynondirs)
        C_ROMlib_filebox(dp, putNmList);
    else
        C_ROMlib_filebox(dp, getNmList);
}

PRIVATE LONGINT getparent(LONGINT dirid)
{
    OSErr err;
    CInfoPBRec cb;
    LONGINT retval;

    cb.dirInfo.ioCompletion = 0;
    cb.dirInfo.ioNamePtr = 0;
    cb.dirInfo.ioVRefNum = CW(-CW(SFSaveDisk));
    cb.dirInfo.ioFDirIndex = CWC(-1);
    cb.dirInfo.ioDrDirID = CL(dirid);
    err = PBGetCatInfo(&cb, false);
    if(err == noErr)
        retval = CL(cb.dirInfo.ioDrParID);
    else
    {
        warning_unexpected("PBGetCatInfo return = %d\n", err);
        retval = 2;
    }
    return retval;
}

PRIVATE BOOLEAN findparent(GUEST<INTEGER> *vrefp, GUEST<LONGINT> *diridp)
{
    HVCB *vcbp;
    BOOLEAN retval;
    struct stat sbuf;
    char *namecpy, *slashp;
    INTEGER namelen;

    vcbp = ROMlib_vcbbyvrn(CW(*vrefp));
    retval = false;
    if(!vcbp->vcbCTRef)
    {
        namelen = strlen(((VCBExtra *)vcbp)->unixname);
        if(namelen != 1 + SLASH_CHAR_OFFSET)
        { /* i.e. "/" */
            namecpy = (char *)alloca(namelen + 1);
            strcpy(namecpy, ((VCBExtra *)vcbp)->unixname);
            slashp = strrchr(namecpy, '/');
            if(slashp == namecpy + SLASH_CHAR_OFFSET)
            {
                slashp[1] = 0;
            }
            else
            {
                slashp[0] = 0;
            }
            if(Ustat(namecpy, &sbuf) == 0 && (vcbp = ROMlib_vcbbybiggestunixname(namecpy)))
            {
                *vrefp = vcbp->vcbVRefNum;
                *diridp = CL((LONGINT)ST_INO(sbuf));
                retval = true;
            }
        }
    }
    return retval;
}

PRIVATE BOOLEAN moveuponedir(DialogPtr dp)
{
    LONGINT parent;
    GUEST<INTEGER> vrn;
    BOOLEAN retval;

    parent = getparent(CL(CurDirStore));
    if(parent != CL(CurDirStore) && parent != 1)
    {
        CurDirStore = CL(parent);
        retval = true;
    }
    else
    {
        vrn = CW(-CW(SFSaveDisk));
        retval = findparent(&vrn, &CurDirStore);
        SFSaveDisk = CW(-CW(vrn));
    }
    return retval;
}

BOOLEAN keyarrow(fltype *fl, INTEGER incr) /* -1: up, 1: down */
{
    INTEGER nsel, oldval, newval;
    fltype::flinfostr *flp;
    /*
 * If there's no selection we start at the top or bottom, depending on
 * whether we're going down or p
 */
    if(fl->flsel == -1)
        nsel = incr > 0 ? 0 : fl->flnmfil - 1;
    else
        nsel = fl->flsel + incr;

    /*
 * If we land on a grayed out file, advance until non-grayed
 */

    for(flp = MR(*fl->flinfo) + nsel;
        (flp->flicns & GRAYBIT) && nsel >= 0 && nsel < fl->flnmfil;
        nsel += incr, flp += incr)
        ;

    /*
 * If we don't find a non-grayed out entry, we leave
 */
    if(nsel < 0 || nsel >= fl->flnmfil)
        /*-->*/ return false;

    /*
 * Figure out what should be visible and scroll there if necessary
 */

    newval = oldval = GetCtlValue(fl->flsh);
    if(nsel < oldval)
        newval = nsel;
    else if(nsel >= oldval + fl->flnmlin)
        newval = nsel - fl->flnmlin + 1;
    if(newval != oldval)
    {
        SetCtlValue(fl->flsh, newval);
        flscroll(fl, oldval, newval);
    }

    /*
 * Get rid of the previously selected rectangle
 */
    if(fl->flsel != -1)
        safeflflip(fl, fl->flsel);
    /*
 * Flip the new rectangle, set up the type information and make assignment
 */
    safeflflip(fl, nsel);
    settype(fl, nsel);
    fl->flsel = nsel;
    return true;
}

PRIVATE bool
folder_selected_p(fltype *fl)
{
    bool retval;

    switch(fl->flavor)
    {
        case original_sf:
            retval = !!fl->flreplyp.oreplyp->fType;
            break;
        case new_sf:
        case new_custom_sf:
            retval = fl->flreplyp.nreplyp->sfIsFolder;
            break;
        default:
            warning_unexpected("flavor = %d", fl->flavor);
            retval = false;
            break;
    }

    return retval;
}

PRIVATE BOOLEAN
call_magicfp(fltype *fl, DialogPtr dp, EventRecord *evt, GUEST<INTEGER> *ith)
{
    BOOLEAN retval;
    LONGINT save_ref_con;

    save_ref_con = GetWRefCon(dp);
    SetWRefCon(dp, TICK("stdf"));
    ROMlib_hook(stdfile_filtnumber);

    switch(fl->flavor)
    {
        case original_sf:
        case new_sf:
            retval = fl->magicfp.ofilterp
                ? fl->magicfp.ofilterp(dp, evt, ith)
                : false;
            break;
        case new_custom_sf:
            retval = fl->magicfp.cfilterp
                ? fl->magicfp.cfilterp(dp, evt, ith, fl->mydata)
                : false;
            break;
        default:
            warning_unexpected("flavor = %d", fl->flavor);
            retval = false;
    }

    SetWRefCon(dp, save_ref_con);

    return retval;
}

#define keydownbit 0x1000

PUBLIC pascal INTEGER Executor::C_ROMlib_stdffilt(DialogPeek dp, EventRecord * evt, GUEST<INTEGER> * ith) /* handle disk insert */
{
    LONGINT ticks;
    GUEST<INTEGER> i;
    INTEGER from;
    ControlHandle h;
    Rect r;
    GUEST<Point> gp;
    Point p;
    INTEGER t;
    fltype *fl;
    INTEGER opentoken;
    fltype::flinfostr *flp, *flep;
    INTEGER nsel, fltop;
    INTEGER part;
    INTEGER retval, retval2;
    GUEST<Handle> tmpH;

    fl = WINDFL(dp);
    opentoken = getOpen; /* getOpen and putSave are both 1 */
    retval = false;
    switch(CW(evt->what))
    {
        case keyDown:
            *ith = CW((CL(evt->message) & 0xFF) + keydownbit);
            switch(CL(evt->message) & 0xFF)
            {
                case NUMPAD_ENTER:
                case '\r':
                    GetDItem((DialogPtr)dp, CW(dp->aDefItem), &i, &tmpH, &r);
                    h = (ControlHandle)MR(tmpH);
                    if(Hx(h, contrlVis) && U(Hx(h, contrlHilite)) != 255)
                    {
                        prefix[0] = 0;
                        oldticks = -1000;
                        *ith = CW(opentoken);
                        retval = -1;
                        //#if !defined(MACOSX_)
                        HiliteControl(h, inButton);
                        Delay((LONGINT)5, (LONGINT *)0);
                        HiliteControl(h, 0);
                        //#endif
                    }
                    break;
                case ASCIIUPARROW:
                    if(CW(evt->modifiers) & cmdKey)
                        *ith = CWC(getDiskName); /* the same as putDiskName */
                    else
                        keyarrow(fl, -1);
                    retval = -1;
                    break;
                case ASCIIDOWNARROW:
                    if((CW(evt->modifiers) & cmdKey) && fl->flsel != -1 && (MR(*fl->flinfo) + fl->flsel)->flicns == MICONCFOLDER)
                    {
                        prefix[0] = 0;
                        oldticks = -1000;
                        *ith = CWC(opentoken);
                    }
                    else
                        keyarrow(fl, 1);
                    retval = -1;
                    break;
                case '\t':
                    *ith = CWC(putDrive); /* PutDrive is 6 which is also getDrive. */
                    retval = -1;
                    break;
                case '.':
                    if(evt->modifiers & CWC(cmdKey))
                    {
                        *ith = CW(fl->fl_cancel_item);
                        retval = -1;
                        break;
                    }
                default:
                    /*
 * The Cx(dp->editField) check was made to get HFS_XFer to work.  There
 * may be a better place to put it, but not enough tests have been done
 * to say where.
 */
                    if(!fl->flgraynondirs && dp->editField == CWC(-1))
                    {
                        flep = MR(*fl->flinfo) + fl->flnmfil - 1;
                        if(CL(evt->when) > lastkeydowntime + CL(DoubleTime))
                        {
                            flp = MR(*fl->flinfo);
                            prefix[0] = 0;
                            oldticks = -1000;
                        }
                        else
                            flp = MR(*fl->flinfo) + ((fl->flsel) == -1 ? 0 : fl->flsel);
                        lastkeydowntime = CL(evt->when);
                        prefix[++prefix[0]] = CL(evt->message) & 0xff;
                        while(flp < flep && RelString((StringPtr)(MR(*fl->flstrs) + flp->floffs), prefix, false, true) < 0)
                            ++flp;
                        nsel = flp - MR(*fl->flinfo);
                        if(nsel != fl->flsel)
                        {
                            safeflflip(fl, fl->flsel);
                            fltop = GetCtlValue(fl->flsh);
                            if(nsel < fltop || nsel >= fltop + fl->flnmlin)
                            {
                                SetCtlValue(fl->flsh, nsel - fl->flnmlin / 2);
                                flscroll(fl, fltop, GetCtlValue(fl->flsh));
                            }
                            safeflflip(fl, fl->flsel = nsel);
                            settype(fl, nsel);
                        }
                        retval = -1;
                    }
                    break;
            }
            break;
        case mouseDown:
            gp = evt->where;
            GlobalToLocal(&gp);
            p = gp.get();
            if(PtInRect(p, &fl->flrect))
            {
                GUEST<Handle> h_s;
                GetDItem((DialogPtr)dp, getOpen, &i, &h_s, &r);
                h = (ControlHandle)MR(h_s);
                flmouse(fl, evt->where.get(), h);
                ticks = TickCount();
                if(fl->flsel != -1 && savesel == fl->flsel && (ticks < oldticks + CL(DoubleTime)))
                {
                    prefix[0] = 0;
                    *ith = CWC(opentoken);
                    oldticks = -1000;
                    retval = -1;
                }
                else
                    oldticks = ticks;
                savesel = fl->flsel;
            }
            else if((t = TestControl(fl->flsh, p)))
            {
                if(t == inThumb)
                {
                    from = GetCtlValue(fl->flsh);
                    TrackControl(fl->flsh, p, (ProcPtr)0);
                    flscroll(fl, from, GetCtlValue(fl->flsh));
                }
                else
                    TrackControl(fl->flsh, p, (ProcPtr)P_ROMlib_stdftrack);
            }
            else if(PtInRect(p, &fl->flcurdirrect))
            {
                *ith = CWC(FAKECURDIR);
                retval = -1;
            }
            else
            {
                GUEST<Handle> h_s;
                GetDItem((DialogPtr)dp, getOpen, &i, &h_s, &r);
                h = (ControlHandle)MR(h_s);
                if((part = TestControl(h, p)) && TrackControl(h, p, (ProcPtr)0))
                {
                    prefix[0] = 0;
                    oldticks = -1000;
                    *ith = CWC(opentoken);
                    retval = -1;
                }
            }
            break;
        case nullEvent:
            *ith = CWC(100);
            retval = -1;
            break;
        case updateEvt:
            if(MR(guest_cast<DialogPeek>(evt->message)) == dp)
                drawjobberattop(dp);
            *ith = CWC(100);
            break;
    }
    retval2 = call_magicfp(fl, (DialogPtr) dp, evt, ith);

    if(*ith == CWC(getOpen) && folder_selected_p(fl)) /* 1 is getOpen and putSave */
        *ith = CWC(FAKEOPENDIR);

    return retval ? retval : retval2;
}

PRIVATE void flinit(fltype * f, Rect * r, ControlHandle sh)
{
    FontInfo fi;
    GUEST<THz> savezone;

    GetFontInfo(&fi);

    f->flsh = sh;
    f->flrect = *r;
    f->fllinht = CW(fi.ascent) + CW(fi.descent) + CW(fi.leading);

    f->flcurdirrect.top = CW(CW(r->top) - f->fllinht - 5);
    f->flcurdirrect.bottom = CW(CW(f->flcurdirrect.top) + f->fllinht);
    getcurname(f);

    f->flascent = CW(fi.ascent);
    f->flnmlin = (CW(r->bottom) - CW(r->top)) / f->fllinht;
    f->flnmfil = 0;
    f->flsel = -1;
    savezone = TheZone;
    TheZone = SysZone;
    f->flinfo = (GUEST<fltype::flinfostr *> *)NewHandle((Size)0);
    f->flstrs = (GUEST<char *> *)NewHandle((Size)0);
    TheZone = savezone;
}

PRIVATE void stdfflip(Rect * rp, INTEGER n, INTEGER height)
{
    GUEST<INTEGER> savetop = rp->top;

    rp->top = CW(CW(rp->top) + (n * height + 1));
    rp->bottom = CW(CW(rp->top) + height - 2);
    InvertRect(rp);
    rp->top = savetop;
}

PRIVATE BOOLEAN trackdirs(DialogPeek dp)
{
    WRAPPER_PIXMAP_FOR_COPY(wrapper);
    PixMapHandle save_bits;
    Rect therect, fillinrect;
    struct link
    {
        Str255 name;
        LONGINT dirid;
        struct link *next;
        INTEGER vrefnum;
    } first, *next;
    int count, i;
    CInfoPBRec hpb;
    int max_width;
    fltype *fl;
    EventRecord evt;
    LONGINT id;
    int sel, newsel, firstsel;
    BOOLEAN done;
    ALLOCABEGIN
    BOOLEAN ejectable;
    bool seen_up_already;
    TEMP_ALLOC_DECL(temp_save_bits);

    {
        ThePortGuard guard(MR(wmgr_port));
        int str_width;

        next = &first;
        hpb.dirInfo.ioCompletion = 0;
        hpb.dirInfo.ioNamePtr = RM(&next->name[0]);
        next->name[0] = 0;
        hpb.dirInfo.ioVRefNum = CW(-CW(SFSaveDisk));
        hpb.dirInfo.ioFDirIndex = CWC(-1);
        hpb.dirInfo.ioDrDirID = CurDirStore;
        max_width = 0;
        count = 0;
        done = false;

        do
        {
            OSErr err;

            err = PBGetCatInfo(&hpb, false);
            if(err != noErr)
            {
                warning_unexpected("PBGetCatInfo returns err = %d\n", err);
                done = true;
            }
            id = next->dirid = CL(hpb.dirInfo.ioDrDirID);
            next->vrefnum = CW(hpb.dirInfo.ioVRefNum);
            next->next = (struct link *)ALLOCA(sizeof(struct link));
            gui_assert(next->next);
            str_width = StringWidth(next->name);
            if(str_width > max_width)
                max_width = str_width;
            next = next->next; /* make Steve Jobs happy */
            hpb.dirInfo.ioDrDirID = hpb.dirInfo.ioDrParID;
            hpb.dirInfo.ioNamePtr = RM(&next->name[0]);
            next->name[0] = 0;
            count++;
            if(id == 2)
            {
                if(!findparent(&hpb.dirInfo.ioVRefNum, &hpb.dirInfo.ioDrDirID))
                    done = true;
            }
        } while(!done);
        fl = WINDFL(dp);
        therect.top = CW(CW(fl->flcurdirrect.top)
                         - CW(PORT_BOUNDS(dp).top));
        therect.left = CW(CW(fl->flcurdirrect.left)
                          - CW(PORT_BOUNDS(dp).left));
        therect.bottom = CW(CW(therect.top) + count * fl->fllinht + 1);
        therect.right = CW(CW(therect.left) + 2 + 16 + 3 + max_width + 4 + 2 + 3);
        ClipRect(&therect);

        {
            Rect *bounds;
            PixMapHandle port_pixmap;
            GUEST<INTEGER> save_bpp_x;
            int row_bytes;
            void *save_bits_mem;

            save_bits = NewPixMap();
            port_pixmap = CPORT_PIXMAP(thePort);

            bounds = &PIXMAP_BOUNDS(save_bits);
            bounds->top = CWC(0);
            bounds->left = CWC(0);
            bounds->bottom = CW(RECT_HEIGHT(&therect));
            bounds->right = CW(RECT_WIDTH(&therect));
            PIXMAP_PIXEL_SIZE_X(save_bits) = save_bpp_x
                = PIXMAP_PIXEL_SIZE_X(port_pixmap);
            ROMlib_copy_ctab(PIXMAP_TABLE(port_pixmap),
                             PIXMAP_TABLE(save_bits));
            row_bytes = ((CW(bounds->right) * CW(save_bpp_x) + 31) / 32) * 4;
            PIXMAP_SET_ROWBYTES_X(save_bits, CW(row_bytes));

            /* Allocate potentially large temporary pixmap space. */
            TEMP_ALLOC_ALLOCATE(save_bits_mem, temp_save_bits,
                                CW(bounds->bottom) * row_bytes);
            PIXMAP_BASEADDR_X(save_bits) = RM((Ptr)save_bits_mem);
            WRAPPER_SET_PIXMAP_X(wrapper, RM(save_bits));

            CopyBits(PORT_BITS_FOR_COPY(thePort), wrapper,
                     &therect, bounds, srcCopy, NULL);
        }

        EraseRect(&therect);

        /* loop through, displaying stuff */
        /* highlite the appropriate box, etc. */

        fillinrect.top = therect.top;
        fillinrect.left = CW(CW(therect.left) + 2);
        fillinrect.bottom = CW(fl->flascent);
        fillinrect.right = therect.right;

        for(i = count, next = &first; --i >= 0; next = next->next)
        {

            /* TODO: ask cliff about a better way to do this */
            /* unused = */
            getdiskname(&ejectable, NULL);
            drawinboxwithicon(next->name, &fillinrect,
                              i ? MICONCFOLDER : ejectable ? MICONFLOPPY : MICONDISK);
            /*
	   drawinboxwithicon(next->name, &fillinrect,
			     i ? MICONCFOLDER : MICONDISK);
*/
            fillinrect.top = CW(CW(fillinrect.top) + (fl->fllinht));
        }

        therect.right = CW(CW(therect.right) - (1));
        therect.bottom = CW(CW(therect.bottom) - (1));
        FrameRect(&therect);
        MoveTo(CW(therect.left) + 1, CW(therect.bottom));
        LineTo(CW(therect.right), CW(therect.bottom));
        LineTo(CW(therect.right), CW(therect.top) + 1);
        therect.right = CW(CW(therect.right) + (1));

        sel = 0;
        fillinrect.top = therect.top;
        fillinrect.left = CW(CW(fillinrect.left) - 1);
        fillinrect.right = CW(CW(fillinrect.right) - 2);
        stdfflip(&fillinrect, sel, fl->fllinht);
        done = false;
        seen_up_already = false;
        firstsel = -1;
        while(!done)
        {
            Point p = evt.where.get();

            if(PtInRect(p, &therect))
                newsel = (p.v - CW(therect.top)) / fl->fllinht;
            else
                newsel = -1;
            if(newsel != sel)
            {
                if(firstsel == -1)
                    firstsel = newsel;
                if(sel != -1)
                    stdfflip(&fillinrect, sel, fl->fllinht);
                if(newsel != -1)
                    stdfflip(&fillinrect, newsel, fl->fllinht);
                sel = newsel;
            }
            if(!ROMlib_sticky_menus_p)
                done = GetNextEvent(mUpMask, &evt);
            else
            {
                if(OSEventAvail(mUpMask, &evt))
                {
                    if(seen_up_already || (sel != firstsel && firstsel != -1))
                        done = true;
                    else
                    {
                        GetOSEvent(mUpMask, &evt);
                        seen_up_already = true;
                    }
                }
                if(!done && OSEventAvail(mDownMask, &evt))
                {
                    GetOSEvent(mDownMask, &evt);
                    done = sel != -1;
                }
            }
        }

        therect.bottom = CW(CW(therect.bottom) + 1);

        /* restore the rect and clean up after ourselves */
        CopyBits(wrapper, PORT_BITS_FOR_COPY(thePort),
                 &PIXMAP_BOUNDS(save_bits), &therect, srcCopy, NULL);
        DisposPixMap(save_bits);
    }
    if(sel != -1)
    {
        for(i = 0, next = &first; i != sel; ++i, next = next->next)
            ;
        CurDirStore = CL(next->dirid);
        SFSaveDisk = CW(-next->vrefnum);
        return true;
    }
    ALLOCAEND

    TEMP_ALLOC_FREE(temp_save_bits);

    return false;
}

PRIVATE void
makeworking(fltype *f)
{

    switch(f->flavor)
    {
        case original_sf:
        {
            WDPBRec wdpb;
            OSErr err;

            wdpb.ioVRefNum = CW(-CW(SFSaveDisk));
            wdpb.ioWDDirID = CurDirStore;
            wdpb.ioWDProcID = TICKX("STDF");
            wdpb.ioNamePtr = 0;
            err = PBOpenWD(&wdpb, false);
            if(err != noErr)
                warning_unexpected("PBOpenWD returns %d\n", err);

            f->flreplyp.oreplyp->vRefNum = wdpb.ioVRefNum;
        }
        break;
        case new_sf:
        case new_custom_sf:
            f->flreplyp.nreplyp->sfFile.vRefNum = CW(-CW(SFSaveDisk));
            f->flreplyp.nreplyp->sfFile.parID = CurDirStore;
            break;
        default:
            warning_unexpected("flavor = %d", f->flavor);
            break;
    }
}

PRIVATE BOOLEAN ejected(HParmBlkPtr pb)
{
    return pb->volumeParam.ioVDrvInfo == CWC(0);
}

/*
 * returns true if the filesystem that pb refers to is part of a single
 * tree, as in UNIX.  Under MSDOS, filesystems are distinct (i.e. C:, D:,
 * etc.)
 */

PRIVATE bool single_tree_fs_p(HParmBlkPtr pb)
{
#if defined(MSDOS) || defined(CYGWIN32)
    return false;
#else
    HVCB *vcbp = ROMlib_vcbbyvrn(CW(pb->volumeParam.ioVRefNum));

    return vcbp && !vcbp->vcbCTRef;
#endif
}

#if defined(LINUX)
PUBLIC int Executor::linuxfloppy_open(int disk, LONGINT *bsizep,
                                      drive_flags_t *flagsp, const char *dname)
{
    int retval;
    struct cdrom_subchnl sub_info;
    bool force_read_only;

    force_read_only = false;
    *flagsp = 0;
#define FLOPPY_PREFIX "/dev/fd"
    if(strncmp(dname, FLOPPY_PREFIX, sizeof(FLOPPY_PREFIX) - 1) == 0)
        *flagsp |= DRIVE_FLAGS_FLOPPY;

    if(!force_read_only)
        retval = Uopen(dname, O_RDWR, 0);
#if !defined(LETGCCWAIL)
    else
        retval = noErr;
#endif
    if(force_read_only || retval < 0)
    {
        retval = Uopen(dname, O_RDONLY, 0);
        if(retval >= 0)
            *flagsp |= DRIVE_FLAGS_LOCKED;
    }

    memset(&sub_info, 0, sizeof sub_info);
    sub_info.cdsc_format = CDROM_MSF;
    if(retval >= 0 && ioctl(retval, CDROMSUBCHNL, &sub_info) != -1)
    {
        switch(sub_info.cdsc_audiostatus)
        {
            case CDROM_AUDIO_PLAY:
            case CDROM_AUDIO_PAUSED:
            case CDROM_AUDIO_COMPLETED:
            case CDROM_AUDIO_ERROR:
                close(retval);
                return -1;
        }
    }

    *bsizep = 512;
    if(retval >= 0)
    {
        /* *ejectablep = true; DRIVE_FLAGS_FIXED not set */
    }

    return retval;
}

int Executor::linuxfloppy_close(int disk)
{
    return close(disk);
}
#endif

typedef struct
{
    const char *dname;
    DrvQExtra *dqp;
    BOOLEAN loaded;
    int fd;
} dosdriveinfo_t;

#define IGNORED (-1) /* doesn't really matter */

#if defined(MSDOS) || defined(CYGWIN32)

#define N_DRIVES (32)

PRIVATE bool drive_loaded[N_DRIVES] = { 0 };

PRIVATE char *
drive_name_of(int i)
{
    static char retval[] = "A:";

    retval[0] = 'A' + i;
    return retval;
}

enum
{
    NUM_FLOPPIES = 2,
    NON_FLOPPY_BIT = 0x80
};

PRIVATE int
fd_of(int i)
{
    int retval;

#if defined(CYGWIN32)
    retval = i;
#else
    if(i < NUM_FLOPPIES)
        retval = i;
    else
        retval = i - NUM_FLOPPIES + NON_FLOPPY_BIT;
#endif
    return retval;
}

#define DRIVE_NAME_OF(x) drive_name_of(x)
#define FD_OF(x) fd_of(x)
#define DRIVE_LOADED(x) drive_loaded[x]

#else

#define DRIVE_NAME_OF(x) drives[x].dname

#define FD_OF(x) drives[x].fd

#define DRIVE_LOADED(x) drives[x].loaded

#endif

PUBLIC void Executor::futzwithdosdisks(void)
{
#if defined(MSDOS) || defined(LINUX) || defined(CYGWIN32)
    int i, fd;
    GUEST<LONGINT> mess_s;
    LONGINT mess;
    LONGINT blocksize;
    drive_flags_t flags;
#if defined(MSDOS) || defined(CYGWIN32)
/* #warning "We're cheating on DOS drive specs: ejectable, bsize, maxsize, writable" */
#define OPEN_ROUTINE dosdisk_open
#define CLOSE_ROUTINE dosdisk_close
#define EXTRA_CLOSE_PARAM , false
#define MARKER DOSFDBIT
#define EXTRA_PARAM
#define ROMLIB_MACDRIVES ROMlib_macdrives
#elif defined(LINUX)
    static dosdriveinfo_t drives[] = {
        { "/dev/fd0", (DrvQExtra *)0, false, IGNORED },
        { "/dev/cdrom", (DrvQExtra *)0, false, IGNORED },
#if 0
	{ "/dev/fd1", (DrvQExtra *) 0, false, IGNORED },
#endif
    };
#define N_DRIVES NELEM(drives)
#define OPEN_ROUTINE linuxfloppy_open
#define CLOSE_ROUTINE linuxfloppy_close
#define EXTRA_CLOSE_PARAM
#define MARKER 0
#define EXTRA_PARAM , (DRIVE_NAME_OF(i))
#define ROMLIB_MACDRIVES (~0)
#endif

    /* Since we're scanning for new disks, let's be paranoid and
     * flush all cached disk information.
     */
    dcache_invalidate_all(true);

    if(!nodrivesearch_p)
    {
        for(i = 0; i < N_DRIVES; ++i)
        {
            if(/* DRIVE_LOADED(i) */ ROMLIB_MACDRIVES & (1 << i))
            {
#if defined(MSDOS) || defined(CYGWIN32)
                checkpoint_macdrive(checkpointp, begin, 1 << i);
#endif
                if(((fd = OPEN_ROUTINE(FD_OF(i), &blocksize, &flags EXTRA_PARAM)) >= 0)
                   || (flags & DRIVE_FLAGS_FLOPPY))
                {
                    try_to_mount_disk(DRIVE_NAME_OF(i), fd | MARKER, &mess_s,
                                      blocksize, 16 * PHYSBSIZE,
                                      flags, 0);
                    mess = CL(mess_s);
                    if(mess)
                    {
                        if(mess >> 16 == 0)
                        {
                            DRIVE_LOADED(i) = true;
                            PPostEvent(diskEvt, mess, (GUEST<EvQElPtr> *)0);
                            /* TODO: we probably should post if mess returns an
		     error, but I think we get confused if we do */
                        }
                        else
                        {
                            if(!(flags & DRIVE_FLAGS_FLOPPY))
                                CLOSE_ROUTINE(fd EXTRA_CLOSE_PARAM);
                        }
                    }
                    else
                    {
                        if(!(flags & DRIVE_FLAGS_FLOPPY))
                            CLOSE_ROUTINE(fd EXTRA_CLOSE_PARAM);
                    }
                }
#if defined(MSDOS) || defined(CYGWIN32)
                checkpoint_macdrive(checkpointp, end, 1 << i);
#endif
            }
        }
    }
#endif
}

PRIVATE void bumpsavedisk(DialogPtr dp, BOOLEAN always)
{
    GUEST<INTEGER> current;
    HParamBlockRec pb;
    INTEGER vref;
    OSErr err;
    BOOLEAN is_single_tree_fs, seenus;

#if defined(MSDOS) || defined(CYGWIN32)
    /*    static BOOLEAN beenhere = false; */

    if(ROMlib_drive_check /* || !beenhere */)
    {
        futzwithdosdisks();
        /*      beenhere = true; */
    }
#endif
    pb.volumeParam.ioVRefNum = CW(-CW(SFSaveDisk));
    pb.volumeParam.ioNamePtr = 0;
    pb.volumeParam.ioVolIndex = 0;
    err = PBHGetVInfo(&pb, false);
    if(err != noErr)
        warning_unexpected("PBHGetVInfo returns %d", err);
    else if(!SFSaveDisk || ISWDNUM(-CW(SFSaveDisk)))
        SFSaveDisk = CW(-CW(pb.volumeParam.ioVRefNum));
    if(always || pb.ioParam.ioResult != CWC(noErr) || ejected(&pb))
    {
        current = pb.volumeParam.ioVRefNum;
        is_single_tree_fs = single_tree_fs_p(&pb);
        pb.volumeParam.ioVolIndex = 0;
        seenus = false;
        vref = 0;
        do
        {
            pb.volumeParam.ioVolIndex = CW(CW(pb.volumeParam.ioVolIndex) + 1);
            err = PBHGetVInfo(&pb, false);
            if(err != noErr && !seenus)
                warning_unexpected("PBHGetVInfo = %d\n", err);
            else
            {
                if(pb.volumeParam.ioVRefNum == current)
                    seenus = true;
                else if(!ejected(&pb) && (!is_single_tree_fs || !single_tree_fs_p(&pb)))
                {
                    if(!vref || seenus)
                        vref = CW(pb.volumeParam.ioVRefNum);
                    if(seenus)
                        /*-->*/ break;
                }
            }
        } while(err == noErr);
        if(vref)
        {
            SFSaveDisk = CW(-vref);
            CurDirStore = CLC(2);
        }
    }
}

/*
 * NOTE: we no longer swap out the refcon when making the sfHookFirstCall (-1)
 *       or the sfHookLastCall (-2).  This fixes the bug that had Photoshop 3.0
 *	 dying the second time that SFCustomGetFile was called.  This fix
 *	 was determined by trial and error.
 *
 * I have just verified that the fix is incorrect.  It works because we
 * bypass some startup code that Photoshop 3.0 uses to set up its type list.
 * This means that the type list won't work properly.  However if we don't
 * swap in 'stdf' then we wind up making calls to InsMenuItem that add
 * entries to the "Format" menu which somehow results in a crash eventually.
 * If we have ROMlib_CALLDHOOK temporarily disable InsMenuItem then PS3.0
 * will work even w/ 'stdf' in the refcon.  It will also work if we rig
 * CountMItems to always return 0 inside ROMlib_CALLDHOOK.  So something
 * appears to go wrong if we call the startup code and actually add (and
 * notice that we add) items the Format menu.
 */

PRIVATE INTEGER
ROMlib_CALLDHOOK(fltype *fl, INTEGER ihit, DialogPtr dp, dialog_hook_u dhu)
{
    INTEGER retval;
    LONGINT save_ref_con;

    ROMlib_hook(stdfile_dialhooknumber);

    save_ref_con = GetWRefCon(dp);
    emergency_save_ref_con = save_ref_con;
    SetWRefCon(dp, TICK("stdf"));

    switch(fl->flavor)
    {
        case original_sf:
        case new_sf:
            retval = dhu.odh(ihit, dp);
            break;
        case new_custom_sf:
            retval = dhu.cdh(ihit, dp, fl->mydata);
            break;
        default:
            warning_unexpected("flavor = %d", fl->flavor);
            retval = 0;
            break;
    }
    SetWRefCon(dp, save_ref_con);
    return retval;
}

PRIVATE void transformsfpdialog(DialogPtr dp, Point * offset, Rect * scrollrect, BOOLEAN getting)
{
    INTEGER numitems, windheight, i, j, extrasizeneeded;
    GUEST<INTEGER> swapped_itype;
    Handle h;
    GUEST<Handle> tmpH;
    Rect r;
    TEPtr tep;

    if(getting)
    {
        extrasizeneeded = 20;
    }
    else
    {
        extrasizeneeded = 110;
        SetRect(scrollrect, 16, 24, 231, 106);
        tep = STARH(MR(((DialogPeek)dp)->textH));
        tep->destRect.top = CW(CW(tep->destRect.top) + (extrasizeneeded));
        tep->destRect.bottom = CW(CW(tep->destRect.bottom) + (extrasizeneeded));
        tep->viewRect.top = CW(CW(tep->viewRect.top) + (extrasizeneeded));
        tep->viewRect.bottom = CW(CW(tep->viewRect.bottom) + (extrasizeneeded));
    }
    numitems = CW(*(GUEST<INTEGER> *)STARH((MR(((DialogPeek)dp)->items)))) + 1;
    for(j = 1; j <= numitems; j++)
    {
        GetDItem(dp, j, &swapped_itype, &tmpH, &r);
        i = CW(swapped_itype);
        h = MR(tmpH);
        if(!getting || CW(r.bottom) > CW(scrollrect->top))
        {
            r.top = CW(CW(r.top) + (extrasizeneeded));
            r.bottom = CW(CW(r.bottom) + (extrasizeneeded));
            if(i <= 7 && i >= 4) /* It's a control */
                MoveControl((ControlHandle)h, CW(r.left), CW(r.top));
            SetDItem(dp, j, i, h, &r);
        }
    }
    windheight = CW(dp->portRect.bottom) - CW(dp->portRect.top) + extrasizeneeded;
    SizeWindow((WindowPtr)dp, CW(dp->portRect.right) - CW(dp->portRect.left),
               windheight, false);
    if(getting)
    {
        scrollrect->top = CW(CW(scrollrect->top) + (extrasizeneeded));
        scrollrect->bottom = CW(CW(scrollrect->bottom) + (extrasizeneeded));
    }
    InvalRect(&dp->portRect);
    offset->v -= extrasizeneeded / 2;
}

void adjustdrivebutton(DialogPtr dp)
{
    INTEGER count;
    Handle drhand;
    GUEST<Handle> tmpH;
    GUEST<INTEGER> i;
    Rect r;
#if !defined(MSDOS) && !defined(CYGWIN32)
    HVCB *vcbp;
    BOOLEAN seenunix;

    count = 0;
    seenunix = false;
    for(vcbp = (HVCB *)MR(VCBQHdr.qHead); vcbp;
        vcbp = (HVCB *)MR(vcbp->qLink))
        if(vcbp->vcbCTRef && vcbp->vcbDrvNum)
            ++count;
        else if(!seenunix)
        {
            ++count;
            seenunix = true;
        }
#else /* defined(MSDOS) */
    count = 2; /* always allow the user to hit the drive button */
#endif /* defined(MSDOS) */
    GetDItem(dp, putDrive, &i, &tmpH, &r); /* putDrive == getDrive */
    drhand = MR(tmpH);
    HiliteControl((ControlHandle)drhand, count > 1 ? 0 : 255);
}

PRIVATE void doeject(DialogPtr dp)
{
    Eject((StringPtr) "", -CW(SFSaveDisk));
    adjustdrivebutton(dp);
    bumpsavedisk(dp, true);
}

PRIVATE GUEST<OSType> gettypeX(StringPtr name, INTEGER vref, LONGINT dirid)
{
    OSErr err;
    HParamBlockRec pbr;

    pbr.fileParam.ioNamePtr = RM(name);
    pbr.fileParam.ioVRefNum = CW(vref);
    pbr.fileParam.ioFVersNum = 0;
    pbr.fileParam.ioFDirIndex = CWC(0);
    pbr.fileParam.ioDirID = CL(dirid);
    err = PBHGetFInfo(&pbr, false);
    if(err == noErr)
        return pbr.fileParam.ioFlFndrInfo.fdType;
    else
        return CLC(0);
}

PRIVATE OSErr
unixcore(StringPtr namep, INTEGER *vrefnump, LONGINT *diridp)
{
    INTEGER vrefnum;
    HVCB *vcbp;
    char *newname;
    INTEGER namelen;
    OSErr err;
    ParamBlockRec pb;
    char *pathname, *filename, *endname;
    VCBExtra *vcbp2;
    struct stat sbuf;
    LONGINT templ;
    char *tempcp;
    ParamBlockRec pbr;

    vrefnum = *vrefnump;
#if 0
  vcbp = ROMlib_vcbbyvrn(vrefnum);
#else
    pbr.ioParam.ioNamePtr = nullptr;
    pbr.ioParam.ioVRefNum = CW(vrefnum);
    vcbp = ROMlib_breakoutioname(&pbr, &templ, &tempcp, (BOOLEAN *)0, true);
    free(tempcp);
#endif
    if(vcbp && !vcbp->vcbCTRef)
    {
        pb.ioParam.ioNamePtr = nullptr;
        pb.ioParam.ioVRefNum = pbr.ioParam.ioVRefNum;
        err = ROMlib_nami(&pb, *diridp, NoIndex, &pathname, &filename,
                          &endname, false, &vcbp2, &sbuf);
        if(err == noErr)
        {
            VCBExtra *vcbextrap;

            namelen = endname - pathname - 1;
            newname = (char *)alloca(namelen + 1 + namep[0] + 1);
            strncpy(newname, pathname, namelen);
            newname[namelen] = '/';
            strncpy(newname + namelen + 1, (char *)namep + 1, namep[0]);
            newname[namelen + 1 + namep[0]] = 0;
            ROMlib_automount(newname);
            vcbextrap = ROMlib_vcbbyunixname(newname);
            if(vcbextrap)
            {
                *vrefnump = CW(vcbextrap->vcb.vcbVRefNum);
                if(*diridp == vcbextrap->u.ufs.ino)
                    *diridp = 2;
            }
            else
                err = nsvErr;
            free(pathname);
        }
    }
    else
        err = nsvErr;
    return err;
}

PUBLIC pascal trap OSErr Executor::C_unixmount(CInfoPBRec * cbp)
{
    OSErr err;

    if(!cbp)
    {
        dofloppymount();
        err = noErr;
    }
    else
    {
        INTEGER vrefnum;
        LONGINT dirid;

        vrefnum = CW(cbp->hFileInfo.ioVRefNum);
        dirid = CL(cbp->hFileInfo.ioDirID);
        err = unixcore(MR(cbp->hFileInfo.ioNamePtr), &vrefnum, &dirid);
        if(err == noErr)
        {
            cbp->hFileInfo.ioVRefNum = CW(vrefnum);
            cbp->hFileInfo.ioDirID = CL(dirid);
        }
    }
    return err;
}

PRIVATE void unixcd(fltype *f)
{
    StringPtr name;
    INTEGER vrefnum;
    LONGINT dirid;

    name = (StringPtr)(MR(*f->flstrs) + MR(*f->flinfo)[f->flsel].floffs);
    vrefnum = -CW(SFSaveDisk);
    dirid = CL(CurDirStore);
    if(unixcore(name, &vrefnum, &dirid) == noErr)
    {
        SFSaveDisk = CW(-vrefnum);
        CurDirStore = CL(dirid);
    }
}

#if 0
	if (U(rep->fName[0]) > 63) /* remember to clip to 63 characters */
	    rep->fName[0] = 63;
#endif

PRIVATE void
get_starting_point(Point *pp)
{
    INTEGER screen_width, screen_height;
    Rect main_gd_rect;

    main_gd_rect = PIXMAP_BOUNDS(GD_PMAP(MR(MainDevice)));
    screen_width = CW(main_gd_rect.right);
    screen_height = CW(main_gd_rect.bottom);
    pp->h = (screen_width - STANDARD_WIDTH) / 2;
    pp->v = (screen_height - STANDARD_HEIGHT) / 2;
}

PRIVATE ControlHandle
create_new_folder_button(DialogPtr dp)
{
    ControlHandle retval;
    Handle dial_test;

    dial_test = GetResource(TICK("DLOG"), -6044);
    if(!dial_test)
        retval = NULL;
    else
    {
        GUEST<INTEGER> i;
        GUEST<Handle> h;
        Rect r;

        GetDItem(dp, sfItemNewFolderUser, &i, &h, &r);
        retval = NewControl((WindowPtr)dp, &r, (StringPtr) "\012New Folder",
                            true, 0, 0, 0, 0, 0L);
        SetDItem(dp, sfItemNewFolderUser, ctrlItem | btnCtrl,
                 (Handle)retval, &r);
    }
    return retval;
}

PRIVATE void
destroy_new_folder_button(DialogPtr dp, ControlHandle ch)
{
    /* May not need to do anything since CloseDialog should clean up all
     outstanding controls */
}

#define SF_NAME(fp) ((fp)->flavor == original_sf         \
                         ? (fp)->flreplyp.oreplyp->fName \
                         : (fp)->flreplyp.nreplyp->sfFile.name)

#define SF_GOOD_XP(fp) ((fp)->flavor == original_sf         \
                            ? &(fp)->flreplyp.oreplyp->good \
                            : &(fp)->flreplyp.nreplyp->sfGood)

#define SF_FTYPE_XP(fp) ((fp)->flavor == original_sf          \
                             ? &(fp)->flreplyp.oreplyp->fType \
                             : &(fp)->flreplyp.nreplyp->sfType)

#define SF_VREFNUM_X(fp) ((fp)->flavor == original_sf           \
                              ? (fp)->flreplyp.oreplyp->vRefNum \
                              : (fp)->flreplyp.nreplyp->sfFile.vRefNum)

#define SF_VREFNUM(fp) (CW(SF_VREFNUM_X(fp)))

#define SF_DIRID_X(fp) ((fp)->flavor == original_sf \
                            ? (GUEST<LONGINT>)0     \
                            : (fp)->flreplyp.nreplyp->sfFile.parID)

#define SF_DIRID(fp) (CL(SF_DIRID_X(fp)))

PRIVATE void
getditext(DialogPtr dp, INTEGER item, StringPtr text)
{
    GUEST<INTEGER> i;
    GUEST<Handle> h;
    Rect r;

    h = 0;
    GetDItem(dp, item, &i, &h, &r);
    if(!h)
        text[0] = 0;
    else
    {
        GetIText(MR(h), text);
    }
}

PRIVATE void
report_new_folder_failure(OSErr err)
{
    char *message = (char *)alloca(256);
    Str255 str;

    switch(err)
    {
        default:
            sprintf(message, "Error #%d occurred; you may not be able to create a new folder.", err);
            break;
        case dupFNErr:
            snprintf(message, 256, "That name is already taken; please use another name.");
            break;
        case dirFulErr:
            snprintf(message, 256, "This directory is full; you may not be able to create a new folder.");
            break;
        case dskFulErr:
            snprintf(message, 256, "This disk is full; you may not be able to create a new folder.");
            break;
        case permErr:
        case wrPermErr:
            snprintf(message, 256, "Filesystem permissions prevent the creation of a new folder.");
            break;
        case vLckdErr:
            snprintf(message, 256, "This volume is locked; you can not create a new folder.");
            break;
    }
    str255_from_c_string(str, message);
    ParamText(str, 0, 0, 0);
    NoteAlert(GENERIC_COMPLAINT_ID, (ProcPtr)0);
}

PRIVATE bool
new_folder_from_dp(DialogPtr dp, fltype *f)
{
    Str255 str;
    HParamBlockRec hpb;
    OSErr err;
    bool retval;

    getditext(dp, 3, str);
    hpb.ioParam.ioVRefNum = CW(-CW(SFSaveDisk));
    hpb.fileParam.ioDirID = CurDirStore;
    if(str[0] > 31)
        str[0] = 31;
    hpb.ioParam.ioNamePtr = RM(&str[0]);
    err = PBDirCreate(&hpb, false);
    if(err != noErr)
        report_new_folder_failure(err);
    else
    {
        *SF_FTYPE_XP(f) = hpb.fileParam.ioDirID;
        retval = true;
    }
    retval = err == noErr;
    return retval;
}

PRIVATE bool
do_new_folder(fltype *f)
{
    bool retval;
    GrafPtr gp;
    DialogPtr dp;

    retval = false;
    gp = thePort;
    dp = GetNewDialog(-6044, (Ptr)0, (WindowPtr)-1);
    if(dp)
    {
        bool done;
        GUEST<INTEGER> ihit;

        SelIText(dp, 3, 0, 32767);
        ShowWindow(dp);
        SelectWindow(dp);
        done = false;
        while(!done)
        {
            /* TODO: consider a filter that limits the length
	     of the string to 31 letters */
            ModalDialog(NULL, &ihit);
            switch(CW(ihit))
            {
                default:
                    break;
                case 1:
                    done = new_folder_from_dp(dp, f);
                    if(done)
                        retval = true;
                    break;
                case 2:
                    done = true;
                    break;
            }
        }
        CloseDialog(dp);
        SetPort(gp);
    }
    return retval;
}

PRIVATE void
rep_from_host_reply_block(reply_u *repp,
                          const host_spf_reply_block *host_reply)
{
}

PRIVATE bool
is_normal_dlgid(getorput_t getorput, INTEGER dig)
{
    return getorput == get
        ? dig == getDlgID || dig == sfGetDialogID
        : dig == putDlgID || dig == sfPutDialogID;
}

PUBLIC void spfcommon(Point p, StringPtr prompt, StringPtr name,
                      dialog_hook_u dh, reply_u rep, INTEGER dig, filter_u fp,
                      file_filter_u filef, INTEGER numt, GUEST<SFTypeList> tl,
                      getorput_t getorput, sf_flavor_t flavor,
                      Ptr activeList, ActivateYDProcPtr activateproc,
                      void* yourdatap)
{
    bool reply_valid;
    

    reply_valid = false;
    if(is_normal_dlgid(getorput, dig) && host_has_spfcommon())
    {
        host_spf_reply_block reply;
        char *c_prompt;
        const char *local_name = "currently ignored";

        c_prompt = cstring_from_str255(prompt);
#if 0
    local_name = ...;
#else
// FIXME: #warning "TODO: support native file pickers"
#endif
        reply_valid = host_spfcommon(&reply, c_prompt, local_name, &fp, &filef,
                                     numt, tl, getorput, flavor, activeList,
                                     (void *)activateproc, yourdatap);
        free(c_prompt);
        if(reply_valid)
            rep_from_host_reply_block(&rep, &reply);
    }
    if(!reply_valid)
    {
        Handle h;
        DialogPtr dp;
        GUEST<INTEGER> ihit_s, i;
        INTEGER ihit;
        int done, sav;
        Rect r, scrollrect;
        GUEST<Handle> tmpH;
        Handle pnhand, ejhand, drhand, sahand;
        OSErr err;
        ControlHandle scrollh;
// FIXME: #warning structure referenced using 32 bit pointer from refcon
        fltype f;
        GrafPtr gp;
        INTEGER openorsave, promptitem, nmlistitem, diskname, ejectitem, driveitem;
        BOOLEAN transform;
        EventRecord evt;
        ParamBlockRec pbr;
        CInfoPBRec hpb;
        ControlHandle new_folder_button;

        done = false;
        memset(&f, 0, sizeof f);
        f.magicfp = fp;
        f.mydata = yourdatap;
        f.flavor = flavor;
        f.flreplyp = rep;

        if(p.h < 2 || p.v < CW(MBarHeight) + 7)
            get_starting_point(&p);

        *SF_GOOD_XP(&f) = CBC(false);
        if(f.flavor == original_sf)
            f.flreplyp.oreplyp->version = CBC(0);
        else
        {
            f.flreplyp.nreplyp->sfIsFolder = 0;
            f.flreplyp.nreplyp->sfIsVolume = 0;
        }
        *SF_FTYPE_XP(&f) = CLC(0);

        new_folder_button = NULL;
        if(getorput == put)
        {
            str31assign(SF_NAME(&f), name);
            if(f.flavor == original_sf)
            {
                openorsave = putSave;
                promptitem = putPrompt;
                nmlistitem = putNmList;
                diskname = putDiskName;
                ejectitem = putEject;
                driveitem = putDrive;
                f.fl_cancel_item = putCancel;
            }
            else
            {
                openorsave = sfItemOpenButton;
                promptitem = sfItemPromptStaticText;
                nmlistitem = sfItemFileListUser;
                diskname = sfItemVolumeUser;
                ejectitem = sfItemEjectButton;
                driveitem = sfItemDesktopButton;
                f.fl_cancel_item = sfItemCancelButton;
            }
        }
        else
        {
            if(f.flavor == original_sf)
            {
                openorsave = getOpen;
                promptitem = 10; /* according to whom? bill? */
                nmlistitem = getNmList;
                diskname = getDiskName;
                ejectitem = getEject;
                driveitem = getDrive;
                f.fl_cancel_item = getCancel;
            }
            else
            {
                openorsave = sfItemOpenButton;
                promptitem = sfItemPromptStaticText;
                nmlistitem = sfItemFileListUser;
                diskname = sfItemVolumeUser;
                ejectitem = sfItemEjectButton;
                driveitem = sfItemDesktopButton;
                f.fl_cancel_item = sfItemCancelButton;
            }
        }
        gp = thePort;
        dp = GetNewDialog(dig, (Ptr)0, (WindowPtr)-1);
        bumpsavedisk(dp, false);
        SetPort(dp);
        GetDItem(dp, openorsave, &i, &tmpH, &r);
        sahand = MR(tmpH);
        if(getorput == put && SF_NAME(&f)[0])
        {
            bool writable;

            getdiskname(NULL, &writable);
            sav = true;
        }
        else
        {
            sav = false;
            HiliteControl((ControlHandle)sahand, 255);
        }
        GetDItem(dp, ejectitem, &i, &tmpH, &r);
        ejhand = MR(tmpH);
        HiliteControl((ControlHandle)ejhand, 255);
        GetDItem(dp, driveitem, &i, &tmpH, &r);
        drhand = MR(tmpH);
        adjustdrivebutton(dp);

        if(getorput == put)
        {
            GetDItem(dp, promptitem, &i, &tmpH, &r);
            h = MR(tmpH);
            SetIText(h, prompt ? prompt : (StringPtr) "");
        }

        GetDItem(dp, nmlistitem, &i, &tmpH, &scrollrect);
        h = MR(tmpH);

        if(getorput == put)
        {
            INTEGER putname;

            if(f.flavor == original_sf)
            {
                putname = putName;
                transform = CW(scrollrect.right) - CW(scrollrect.left) == 1;
            }
            else
            {
                putname = sfItemFileNameTextEdit;
                transform = false;
                new_folder_button = create_new_folder_button(dp);
            }

            GetDItem(dp, putname, &i, &tmpH, &r);
            pnhand = MR(tmpH);
            SetIText(pnhand, SF_NAME(&f));
            SelIText((DialogPtr)dp, putname, 0, 32767);
        }
        else
        {
            if(f.flavor == original_sf)
            {
                GetDItem(dp, getScroll, &i, &tmpH, &r);
                h = MR(tmpH);
                transform = CW(r.right) - CW(r.left) == 16;
                GetDItem(dp, getDotted, &i, &tmpH, &r);
                h = MR(tmpH);
                SetDItem(dp, getDotted, userItem, (Handle)P_ROMlib_filebox, &r);
            }
            else
                transform = false;
        }

        if(transform)
            transformsfpdialog(dp, &p, &scrollrect, getorput == get);

        SetDItem(dp, nmlistitem, userItem, (Handle)P_ROMlib_filebox, &scrollrect);

        GetDItem(dp, diskname, &i, &tmpH, &r);
        h = MR(tmpH);
        SetDItem(dp, diskname, userItem, (Handle)P_ROMlib_filebox, &r);

        r.left = CW(CW(scrollrect.left) + 1);
        r.right = CW(CW(scrollrect.right) - 16);
        r.top = CW(CW(scrollrect.top) + 1);
        r.bottom = CW(CW(scrollrect.bottom) - 1);
        scrollrect.left = CW(CW(scrollrect.right) - 16);
        scrollh = NewControl((WindowPtr)dp, &scrollrect, (StringPtr) "", true,
                             0, 0, 0, scrollBarProc, 0L);
        flinit(&f, &r, scrollh);
        f.flfilef = filef;
        f.flnumt = numt;
        f.fltl = tl;
        f.flch = (ControlHandle)sahand;
        f.flgraynondirs = getorput == get ? 0 : GRAYBIT;

        if(getorput == get)
        {
            if(f.flnmfil > 0)
            {
                StringPtr ip;

                ip = (StringPtr)MR(*f.flstrs) + MR(*f.flinfo)[0].floffs;
                str31assign(SF_NAME(&f), ip);
            }
            else
                (SF_NAME(&f))[0] = 0;
        }
// FIXME: #warning not 64-bit clean
        SetWRefCon((WindowPtr)dp, US_TO_SYN68K(&f));
        if(CW(dp->portRect.bottom) + p.v + 7 > CW(screenBitsX.bounds.bottom))
            p.v = CW(screenBitsX.bounds.bottom) - CW(dp->portRect.bottom) - 7;
        if(p.v < CW(MBarHeight) + 7)
            p.v = CW(MBarHeight) + 7;
        MoveWindow((WindowPtr)dp, p.h, p.v, false);

        ihit = -1;
        if(dh.odh)
            ihit = ROMlib_CALLDHOOK(&f, -1, dp, dh); /* the mac does this */

        flfill(&f); /* moved to after dhook call of -1 suggested by
		   Wieslaw Kuzmicz */

        ShowWindow((WindowPtr)dp);
        SelectWindow((WindowPtr)dp);
        while(!done)
        {
            ModalDialog((ProcPtr)P_ROMlib_stdffilt, &ihit_s);
            ihit = CW(ihit_s);
            if(getorput == put)
                GetIText(pnhand, SF_NAME(&f));
            if(dh.odh)
                ihit = ROMlib_CALLDHOOK(&f, ihit, dp, dh);
            if(ihit == openorsave)
            {
                makeworking(&f);
                if(getorput == get)
                {
                    if(*SF_FTYPE_XP(&f)) /* will never happen unless someone */
                        (SF_NAME(&f))[0] = 0; /* has a tricky filterproc */
                    else
                        *SF_FTYPE_XP(&f) = gettypeX(SF_NAME(&f), SF_VREFNUM(&f),
                                                    SF_DIRID(&f));
                    done = true;
                    *SF_GOOD_XP(&f) = CBC(true);
                }
                else
                {
                    GetIText(pnhand, SF_NAME(&f));
                    hpb.dirInfo.ioCompletion = 0;
                    hpb.dirInfo.ioNamePtr = RM((StringPtr)SF_NAME(&f));
                    hpb.dirInfo.ioVRefNum = SF_VREFNUM_X(&f);
                    hpb.dirInfo.ioFDirIndex = CWC(0);
                    hpb.dirInfo.ioDrDirID = 0;
                    err = PBGetCatInfo(&hpb, false);
                    switch(err)
                    {
                        case noErr:
                            ParamText(SF_NAME(&f), (StringPtr)0, (StringPtr)0,
                                      (StringPtr)0);
                            if(movealert(-3996) == 1) /* overwrite ... */
                                break;
                            *SF_FTYPE_XP(&f) = hpb.hFileInfo.ioFlFndrInfo.fdType;
                        /* FALL THROUGH */
                        default:
                            if(err != noErr)
                                warning_unexpected("err = %d", err);
                        /* FALL THROUGH */
                        case fnfErr:
                            done = true;
                            *SF_GOOD_XP(&f) = CBC(true); /* great.  That's a take */
                            break;
                        case bdNamErr:
                        case nsvErr:
                        case paramErr:
                            movealert(-3994); /* disknotfound */
                            break;
                    }
                }
            }
            else if((ihit == f.fl_cancel_item) || (ihit == putCancel))
            { /* MYM 6.0 suggests that putCancel
					     cancels a get, too */
                done = true;
            }
            else if(ihit == ejectitem)
            {
                doeject(dp);
                ihit = FAKEREDRAW;
            }
            else if(ihit == driveitem)
            {
                bumpsavedisk(dp, true);
                ihit = FAKEREDRAW;
            }
            else if(ihit == diskname)
            {
                if(moveuponedir(dp))
                    ihit = FAKEREDRAW;
            }
            else if(ihit == FAKECURDIR)
            {
                if(trackdirs((DialogPeek)dp))
                    ihit = FAKEREDRAW;
            }
            else if(ihit == FAKEOPENDIR)
            {
                CurDirStore = *SF_FTYPE_XP(&f);
                unixcd(&f);
                ihit = FAKEREDRAW;
            }
            else if(ihit == sfItemNewFolderUser)
            {
                if(do_new_folder(&f))
                {
                    CurDirStore = *SF_FTYPE_XP(&f);
                    unixcd(&f);
                    ihit = FAKEREDRAW;
                }
            }
            if(getorput == put)
            {
                Str255 file_name;
                bool writable;

                GetIText(pnhand, file_name);
                str31assign(SF_NAME(&f), file_name);
                getdiskname(NULL, &writable);
                if((SF_NAME(&f))[0] && writable && !sav)
                {
                    HiliteControl((ControlHandle)sahand, 0);
                    sav = true;
                }
                else if((!(SF_NAME(&f)[0]) || !writable) && sav)
                {
                    HiliteControl((ControlHandle)sahand, 255);
                    sav = false;
                }
            }
            if(WaitNextEvent(diskMask, &evt, 4, 0) && (evt.message & CLC(0xFFFF0000)) == CLC(0))
            {
                pbr.volumeParam.ioNamePtr = 0;
                pbr.volumeParam.ioVolIndex = 0;
                pbr.volumeParam.ioVRefNum = CW(CL(evt.message) & 0xFFFF);
                err = PBGetVInfo(&pbr, false);
                gui_assert(err == noErr);
                if(err == noErr)
                {
                    adjustdrivebutton(dp);
                    SFSaveDisk = CW(-CW(pbr.volumeParam.ioVRefNum));
                    CurDirStore = CLC(2);
                    ihit = FAKEREDRAW;
                }
            }
            if(ihit == FAKEREDRAW)
                realcd((DialogPeek)dp, CL(CurDirStore));
        }
        if(f.flavor != original_sf && dh.odh)
            ihit = ROMlib_CALLDHOOK(&f, -2, dp, dh); /* the mac does this */
        flfinit(&f);
        destroy_new_folder_button(dp, new_folder_button);
        CloseDialog((DialogPtr)dp);
        makeworking(&f);
        SetPort(gp);
    }
    
}

PUBLIC pascal trap void Executor::C_SFPPutFile(Point p, StringPtr prompt, StringPtr name, DlgHookProcPtr dh, SFReply * rep, INTEGER dig, ModalFilterProcPtr fp)
{
    dialog_hook_u dhu;
    reply_u repu;
    filter_u filteru;
    file_filter_u zero_file_filter = { 0 };

    dhu.odh = dh;
    repu.oreplyp = rep;
    filteru.ofilterp = fp;

    spfcommon(p, prompt, name, dhu, repu, dig, filteru, zero_file_filter, -1,
              nullptr, put, original_sf, 0, 0, 0);
}

PUBLIC pascal trap void Executor::C_SFPutFile(Point p, StringPtr prompt, StringPtr name, DlgHookProcPtr dh, SFReply * rep)
{
    SFPPutFile(p, prompt, name, dh, rep, putDlgID, nullptr);
}

PUBLIC pascal trap void Executor::C_SFPGetFile(Point p, StringPtr prompt, FileFilterProcPtr filef, INTEGER numt, GUEST<SFTypeList> tl, DlgHookProcPtr dh, SFReply * rep, INTEGER dig, ModalFilterProcPtr fp)
{
    dialog_hook_u dhu;
    reply_u repu;
    filter_u filteru;
    file_filter_u file_filteru;

    dhu.odh = dh;
    repu.oreplyp = rep;
    filteru.ofilterp = fp;
    file_filteru.oflfilef = filef;

    spfcommon(p, prompt, (StringPtr) "", dhu, repu, dig, filteru, file_filteru,
              numt, tl, get, original_sf, 0, 0, 0);
}

PUBLIC pascal trap void Executor::C_SFGetFile(Point p, StringPtr prompt, FileFilterProcPtr filef, INTEGER numt, GUEST<SFTypeList> tl, DlgHookProcPtr dh, SFReply * rep)
{
    SFPGetFile(p, prompt, filef, numt, tl, dh, rep, getDlgID, nullptr);
}

PUBLIC pascal trap void Executor::C_CustomPutFile(Str255 prompt, Str255 defaultName, StandardFileReply * replyp, INTEGER dlgid, Point where, DlgHookYDProcPtr dlghook, ModalFilterYDProcPtr filterproc, Ptr activeList, ActivateYDProcPtr activateproc, void* yourdatap)
{
    dialog_hook_u dhu;
    reply_u repu;
    filter_u filteru;
    file_filter_u file_filteru;

    dhu.cdh = dlghook;
    repu.nreplyp = replyp;
    file_filteru.cflfilef = 0;
    filteru.cfilterp = filterproc;

    if(dlgid == 0)
        dlgid = sfPutDialogID;

    spfcommon(where, prompt, defaultName, dhu, repu, dlgid, filteru,
              file_filteru, -1, 0, put, new_custom_sf, activeList,
              activateproc, yourdatap);
}

PUBLIC pascal trap void Executor::C_CustomGetFile(FileFilterYDProcPtr filefilter, INTEGER numtypes, GUEST<SFTypeList> typelist, StandardFileReply * replyp, INTEGER dlgid, Point where, DlgHookYDProcPtr dlghook, ModalFilterYDProcPtr filterproc, Ptr activeList, ActivateYDProcPtr activateproc, void* yourdatap)
{
    dialog_hook_u dhu;
    reply_u repu;
    filter_u filteru;
    file_filter_u file_filteru;

    dhu.cdh = dlghook;
    repu.nreplyp = replyp;
    filteru.cfilterp = filterproc;
    file_filteru.cflfilef = filefilter;

    if(dlgid == 0)
        dlgid = sfGetDialogID;

    spfcommon(where, (StringPtr) "", (StringPtr) "", dhu, repu, dlgid,
              filteru, file_filteru, numtypes,
              typelist, get, new_custom_sf, activeList, activateproc,
              yourdatap);
}

PUBLIC pascal trap void Executor::C_StandardGetFile(FileFilterProcPtr filef, INTEGER numt, GUEST<SFTypeList> tl, StandardFileReply * replyp)
{
    Point p;
    reply_u repu;
    file_filter_u file_filteru;
    dialog_hook_u dhu;
    filter_u filteru;

    repu.nreplyp = replyp;
    file_filteru.oflfilef = filef;
    dhu.cdh = 0;
    filteru.cfilterp = 0;

    p.h = -1;
    p.v = -1;
    spfcommon(p, (StringPtr) "", (StringPtr) "", dhu, repu, sfGetDialogID,
              filteru, file_filteru, numt, tl, get, new_sf, 0, 0, 0);
}

PUBLIC pascal trap void Executor::C_StandardPutFile(Str255 prompt, Str255 defaultname, StandardFileReply * replyp)
{
    Point p;
    reply_u repu;
    file_filter_u file_filteru;
    dialog_hook_u dhu;
    filter_u filteru;

    filteru.cfilterp = 0;
    repu.nreplyp = replyp;
    file_filteru.cflfilef = 0;
    dhu.cdh = 0;

    p.h = -1;
    p.v = -1;
    spfcommon(p, prompt, defaultname, dhu, repu, sfPutDialogID, filteru,
              file_filteru, -1, 0, put, new_sf, 0, 0, 0);
}

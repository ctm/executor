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

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_stdfile[] =
	    "$Id: stdfile.c 120 2005-07-14 21:20:19Z ctm $";
#endif

/* Forward declarations in StdFilePkg.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#if defined (LINUX)
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

#include "rsys/executor.h"

#include "rsys/osevent.h"

PUBLIC int Executor::nodrivesearch_p = FALSE;

#if defined (MSDOS) || defined (CYGWIN32)
#include "dosdisk.h"
#include "aspi.h"
#include "rsys/checkpoint.h"
#endif

#include "rsys/print.h"
#include "rsys/system_error.h"

using namespace Executor;
using namespace ByteSwap;

typedef	pascal BOOLEAN (*filtp)(DialogPtr dp, EventRecord *evp, INTEGER *ith);

#define CALLFILTERPROC(dp, evt, ith, fp)	\
ROMlib_CALLFILTERPROC((dp), (evt), (ith), (filtp) (fp))

typedef	pascal BOOLEAN (*custom_filtp)(DialogPtr dp, EventRecord *evp,
									   INTEGER *ith, UNIV Ptr data);

#define CALL_NEW_FILTER_PROC(dp, evt, ith, data, fp) \
ROMlib_CALL_NEW_FILTER_PROC((dp), (evt), (ith), (data), \
(custom_filtp) (fp))

typedef	pascal BOOLEAN (*filefiltp)( ParmBlkPtr pbp );

#define CALLFILEFILT(pbp, fp)	ROMlib_CALLFILEFILT((pbp), (filefiltp)(fp))


typedef pascal BOOLEAN (*custom_file_filtp) (ParmBlkPtr pbp, UNIV Ptr data);

#define CALL_CUSTOM_FILE_FILT(pbp, data, fp) \
ROMlib_CALL_CUSTOM_FILE_FILT ((pbp), (data), (custom_file_filtp) (fp))


typedef union
{
  SFReply *oreplyp;
  StandardFileReply *nreplyp;
}
reply_u;

typedef union
{
  ProcPtr oflfilef;
  FileFilterYDProcPtr cflfilef;
}
file_filter_u;

typedef union
{
  ProcPtr ofilterp;
  ModalFilterYDProcPtr cfilterp;
}
filter_u;

typedef union
{
  ProcPtr odh;
  DlgHookYDProcPtr cdh;
}
dialog_hook_u;

typedef struct {
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
    struct flinfostr {
	INTEGER floffs;
	INTEGER flicns;
    } **flinfo;
    char **flstrs;
    file_filter_u flfilef;
    INTEGER flnumt;
    OSType * fltl;
    ControlHandle flch;
    INTEGER flgraynondirs;
    Str255 flcurdirname;
    filter_u magicfp;
    INTEGER fl_cancel_item;
    UNIV Ptr mydata;
} fltype;

namespace Executor {
	PRIVATE INTEGER movealert(INTEGER);
	PRIVATE void drawminiicon(INTEGER);
	PRIVATE void drawinboxwithicon(StringPtr, Rect*, INTEGER);
	PRIVATE void safeflflip(fltype *, INTEGER);
	PRIVATE void flupdate(fltype *, INTEGER, INTEGER);
	PRIVATE void flscroll(fltype *, INTEGER, INTEGER);
	PRIVATE StringPtr getdiskname(BOOLEAN *, boolean_t *);
	PRIVATE void drawjobberattop(DialogPeek);
	PRIVATE LONGINT getdirid(StringPtr);
	PRIVATE void settype(fltype *, INTEGER);
	PRIVATE INTEGER flwhich(fltype *, Point);
	PRIVATE void flmouse(fltype *, Point, ControlHandle);
	PRIVATE void getcurname(fltype *);
	static inline BOOLEAN ROMlib_CALLFILTERPROC(DialogPtr, EventRecord *, INTEGER*, filtp);
	PRIVATE void flfinit(fltype *);
	PRIVATE void flinsert(fltype *, StringPtr, INTEGER);
	PRIVATE int typeinarray(OSType, INTEGER, SFTypeList);
	PRIVATE LONGINT stdfcmp(char *, char *);
	static inline BOOLEAN ROMlib_CALLFILEFILT(ParmBlkPtr, filefiltp);
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
	PRIVATE OSType gettypeX(StringPtr, INTEGER, LONGINT);
}

A1(PRIVATE, INTEGER, movealert, INTEGER, id)
{
    Handle h;
    INTEGER dh, dv;
    Rect *rp;
    
    h = GetResource(TICK("ALRT"), id);
    if (!(*h).p)
	LoadResource(h);
    rp = (Rect *) STARH(h);
    dh = BigEndianValue(rp->right)  - BigEndianValue(rp->left);
    dv = BigEndianValue(rp->bottom) - BigEndianValue(rp->top);
    rp->left   = CWC(150);
    rp->top    = CWC(30);
    rp->right  = BigEndianValue(150 + dh);
    rp->bottom = BigEndianValue(30 + dv);
    return(Alert(id, (ProcPtr)0));
}

#define SICONS	-15744

A1(PRIVATE, void, drawminiicon, INTEGER, icon)
{
    Handle h;
    BitMap bm;
    Rect r;

    h = ROMlib_getrestid(TICK("SICN"), SICONS);
    if (!h)
/*-->*/	return;	/* badness */
    HLock(h);
    bm.baseAddr = RM((Ptr) STARH(h) + icon * 16 * 16 / 8);
    bm.rowBytes = CWC(2);
    bm.bounds.left  = bm.bounds.top = CWC(0);
    bm.bounds.right = bm.bounds.bottom = CWC(16);
    r.top  = PORT_PEN_LOC (thePort).v;
    r.left = PORT_PEN_LOC (thePort).h;
    r.bottom = BigEndianValue(BigEndianValue(r.top)  + 16);
    r.right  = BigEndianValue(BigEndianValue(r.left) + 16);
    CopyBits(&bm, PORT_BITS_FOR_COPY (thePort),
	     &bm.bounds, &r, srcCopy, NULL);
    HUnlock(h);
}

/*
 * NOTE:  the Rect pointed to below has a normal top, left and right,
 *	  BUT the bottom is the amount to add to the top for drawing
 *	  text, not the real bottom.
 */

A3(PRIVATE, void, drawinboxwithicon, StringPtr, str, Rect *, rp, INTEGER, icon)
{
    Rect r;
    INTEGER width, strlen, strwidths[255], lengthavail, *widp;
    static const char *ellipsis = "\003...";

    /*
     * Note:  the ellipsis code is a bit shabby, not truncating at a character
     *	      boundary, nor leaving space on either side, but for now, it's
     *	      ok.
     */

    MoveTo(BigEndianValue(rp->left) + 2, BigEndianValue(rp->top));
    drawminiicon(icon);
    MoveTo(BigEndianValue(rp->left) + 2 + 16 + 3 , BigEndianValue(rp->top) + BigEndianValue(rp->bottom)-1); /* see note above */
    r.left   = rp->left;
    r.right  = rp->right;
    r.top    = rp->top;
    r.bottom =  CWC(32766);
    ClipRect(&r);
    strlen = *str;
    MeasureText(strlen, (Ptr) (str + 1), (Ptr) strwidths);
    lengthavail = BigEndianValue(rp->right) - (BigEndianValue(rp->left) + 2 + 16 + 3);
    if (BigEndianValue(strwidths[strlen]) > lengthavail) {
	width = StringWidth((StringPtr) ellipsis);
/* 4 might be the space on the right of the ellipsis. */
/* TODO: figure out exactly what the number is. */
    	lengthavail -= (width + 4);
	widp = strwidths;
	while (BigEndianValue(*++widp) < lengthavail)
	    DrawChar(*++str);
	DrawString((StringPtr) ellipsis);
    } else
	DrawString(str);
    r.top    = CWC(-32767);
    r.left   = CWC(-32767);
    r.right  =  CWC(32766);
    ClipRect(&r);
}

A2(PRIVATE, void, safeflflip, fltype *, f, INTEGER, sel)
{
    Rect r;
    INTEGER fltop = GetCtlValue(f->flsh);
    
    if (sel >= fltop && sel < fltop + f->flnmlin) {
	r.left   = f->flrect.left;
	r.right  = f->flrect.right;
	r.top    = BigEndianValue(BigEndianValue(f->flrect.top) + (sel - fltop) * f->fllinht);
	r.bottom = BigEndianValue(BigEndianValue(r.top) + f->fllinht);
	if (EmptyRgn(MR(((WindowPeek)thePort)->updateRgn))) /* stuff to draw? */
	    InvertRect(&r);	/* no: we can flip */
	else
	    InvalRect(&r);	/* yes: flip later */
    }
}

#define GRAYBIT		(1 << 5 )
#define ICONMASK	(~GRAYBIT)

A3(PRIVATE, void, flupdate, fltype *, f, INTEGER, st, INTEGER, n)
{
    INTEGER i;
    fltype::flinfostr *ip;
    INTEGER sel = f->flsel;
    INTEGER fltop;
    Rect r;

    fltop = GetCtlValue(f->flsh);
    r.top    = BigEndianValue(BigEndianValue(f->flrect.top) + (st - fltop) * f->fllinht);
    r.bottom = BigEndianValue(f->flascent);
    r.left   = f->flrect.left;
    r.right  = f->flrect.right;

    ip = MR(*f->flinfo) + st;
    HLock((Handle) f->flstrs);
    for (i = st; i < st + n && i < fltop + f->flnmlin && i < f->flnmfil; i++) {
	drawinboxwithicon((StringPtr) (MR(*f->flstrs) + ip->floffs), &r,
							ip->flicns & ICONMASK);
	if (ip->flicns & GRAYBIT) {
	    r.bottom = BigEndianValue(BigEndianValue(r.top) + f->fllinht);
	    PenMode(notPatBic);
	    PenPat(gray);
	    PaintRect(&r);
	    PenPat(black);
	    PenMode(patCopy);
	    r.bottom = BigEndianValue(f->flascent);
	}
	ip++;
	r.top = BigEndianValue(BigEndianValue(r.top) + (f->fllinht));
    }
    HUnlock((Handle) f->flstrs);
    if (sel >= st && sel < st + n)
	safeflflip(f, sel);
}

A3(PRIVATE, void, flscroll, fltype *, f, INTEGER, from, INTEGER, to)
{
    RgnHandle rh;
    INTEGER toscroll;
    
    if (from != to) {
	rh = NewRgn();
	ScrollRect(&f->flrect, 0, (from - to) * f->fllinht, rh);
	if (to > from) {
	    toscroll = to - from;
	    if (toscroll >= f->flnmlin)
		flupdate(f, to, f->flnmlin);
	    else
		flupdate(f, from+f->flnmlin, toscroll);
	} else {
	    toscroll = from - to;
	    if (toscroll > f->flnmlin)
		toscroll = f->flnmlin;
	    flupdate(f, to, toscroll);
	}
	DisposeRgn(rh);
    }
}

#define CTLFL(sh) \
	((fltype *)(long) MR(MR((WindowPeek)STARH((sh))->contrlOwner)->refCon))

/*
 * this hack is necessary because Excel 4 can bring up a dialog on top of
 * stdfile at a time when the refcon has been set to 'stdf'.
 */

static LONGINT emergency_save_ref_con;

PRIVATE fltype *
WINDFL (void *dp)
{
  fltype *retval;

  retval = (fltype *) (long) ((WindowPeek)dp)->refCon;
  if ((long) retval == CLC ((long) TICK("stdf")))
    retval = (fltype *) SYN68K_TO_US (emergency_save_ref_con);
  else
    retval = MR (retval);
  return retval;
}

#if 0
#define WINDFL(dp) \
		((fltype *)(long) MR(((WindowPeek)dp)->refCon))
#endif

P2 (PUBLIC, pascal void, ROMlib_stdftrack, ControlHandle, sh, INTEGER, part)
{
  const uint32 min_between_scroll_msecs = 100;
  static uint32 last_scroll_msecs;
  uint32 current_msecs;
  int16 from, pg;
  
  current_msecs = msecs_elapsed ();
  
  if (current_msecs - min_between_scroll_msecs < last_scroll_msecs)
    return;
  else
    last_scroll_msecs = current_msecs;
  
  from = GetCtlValue (sh);
  pg = CTLFL(sh)->flnmlin - 1;
  switch (part) {
    case inUpButton:
      SetCtlValue(sh, from-1);
      break;
    case inDownButton:
      SetCtlValue(sh, from+1);
      break;
    case inPageUp:
      SetCtlValue(sh, from-pg);
      break;
    case inPageDown:
      SetCtlValue(sh, from+pg);
      break;
    }
  flscroll (CTLFL (sh), from, GetCtlValue (sh));
}

PRIVATE INTEGER cachedvrn = 32767;
PRIVATE INTEGER savesel = -1;
PRIVATE LONGINT oldticks = -1000;
PRIVATE LONGINT lastkeydowntime = 0;
PRIVATE Str255 prefix = { 0 };
PRIVATE char **holdstr;

PUBLIC void Executor::ROMlib_init_stdfile(void)
{
    cachedvrn = 32767;
    savesel = -1;
    oldticks = -1000;
    lastkeydowntime = 0;
    prefix[0] = 0;
    holdstr = 0;
}

A2(PRIVATE, StringPtr, getdiskname, BOOLEAN *, ejectablep,
   boolean_t *, writablep)
{
    static BOOLEAN ejectable;
    static boolean_t writable;
    static Str255 retval;
    ParamBlockRec pbr;

    if (SFSaveDisk != cachedvrn) {
        OSErr err;

	pbr.volumeParam.ioNamePtr = RM(&retval[0]);
	pbr.volumeParam.ioVolIndex = 0;
	pbr.volumeParam.ioVRefNum = BigEndianValue(-BigEndianValue(SFSaveDisk));
	err = PBGetVInfo(&pbr, FALSE);
	cachedvrn = SFSaveDisk;
	if (err == noErr)
	  {
	    ejectable = !(pbr.volumeParam.ioVAtrb & CWC(VNONEJECTABLEBIT));
	    writable = !(pbr.volumeParam.ioVAtrb &
			 CWC(VHARDLOCKBIT|VSOFTLOCKBIT));
	    if (writable && pbr.volumeParam.ioVFrBlk == 0)
	      writable = FALSE;
	  }
	else
	  {
	    warning_unexpected ("surprising PBGetVInfo retval = %d\n", err);
	    ejectable = FALSE;
	    writable = TRUE;
	  }
    }
    if (ejectablep)
      *ejectablep = ejectable;
    if (writablep)
      *writablep = writable;
    return retval;
}

A1(PRIVATE, void, drawjobberattop, DialogPeek, dp)
{
    INTEGER icon;
    Rect *rp;
    fltype *flp;
    INTEGER savebottom;
    BOOLEAN ejectable;
    PenState ps;

    GetPenState(&ps);
    PenNormal();
    if (BigEndianValue(CurDirStore) == 2) {
#if 1
/* TODO: ask cliff about a better way to do this */
    /* unused = */ getdiskname( &ejectable, NULL );
	icon = ejectable ? MICONFLOPPY : MICONDISK;
#else /* 0 */
	icon = MICONDISK;
#endif /* 0 */
    } else
	icon = MICONOFOLDER;
    flp = WINDFL(dp);
    rp = &flp->flcurdirrect;
    savebottom = rp->bottom;
    rp->bottom = BigEndianValue(flp->flascent);
    rp->left = BigEndianValue(BigEndianValue(rp->left) + (2));
    drawinboxwithicon(flp->flcurdirname, rp, icon);
    rp->left = BigEndianValue(BigEndianValue(rp->left) - (2));
    rp->bottom = savebottom;
    FrameRect(rp);
    MoveTo(BigEndianValue(rp->left)+1, BigEndianValue(rp->bottom));
    LineTo(BigEndianValue(rp->right),  BigEndianValue(rp->bottom));
    LineTo(BigEndianValue(rp->right),  BigEndianValue(rp->top)+1);
    SetPenState(&ps);
}

A1(PRIVATE, LONGINT, getdirid, StringPtr, fname)
{
    CInfoPBRec hpb;
    LONGINT retval;
    OSErr err;

    hpb.dirInfo.ioCompletion = 0;
    hpb.dirInfo.ioNamePtr    = RM(fname);
    hpb.dirInfo.ioVRefNum    = BigEndianValue(-BigEndianValue(SFSaveDisk));
    hpb.dirInfo.ioFDirIndex  = CWC (0);
    hpb.dirInfo.ioDrDirID    = CurDirStore;
    err = PBGetCatInfo(&hpb, FALSE);
    if (err == noErr)
      retval = BigEndianValue (hpb.dirInfo.ioDrDirID);
    else
      {
	warning_unexpected ("PBGetCatInfo return value err = %d\n", err);
	retval = 2; /* known good id ... an error might be better here */
      }
    return retval;
}

#define MAXPREFIX	64

PRIVATE void
set_type_and_name (fltype *f, OSType type, Str255 name)
{
  switch (f->flavor)
    {
    case original_sf:
      f->flreplyp.oreplyp->fType = BigEndianValue (type);
      str31assign (f->flreplyp.oreplyp->fName, name);
      break;
    case new_sf:
    case new_custom_sf:
      f->flreplyp.nreplyp->sfType = BigEndianValue (type);
      str31assign (f->flreplyp.nreplyp->sfFile.name, name);
      f->flreplyp.nreplyp->sfIsFolder = !!type;
      break;
    default:
      warning_unexpected ("flavor = %d", f->flavor);
      break;
    }
}

A2(PRIVATE, void, settype, fltype *, f, INTEGER, newsel)
{
  StringPtr ip;

  ip = (StringPtr) MR(*f->flstrs) + MR(*f->flinfo)[newsel].floffs;
  if (MR(*f->flinfo)[newsel].flicns == MICONCFOLDER)
    set_type_and_name (f, getdirid (ip), (StringPtr) "");
  else
    set_type_and_name (f, 0, ip);
}

A2(PRIVATE, INTEGER, flwhich, fltype *, f, Point, p)
{
    INTEGER retval;
    INTEGER bump, from;
    
    if (!PtInRect(p, &f->flrect)) {
	bump = 0;
	if (p.v < BigEndianValue(f->flrect.top))
	    bump = -1;
	else if (p.v >= BigEndianValue(f->flrect.bottom))
	    bump =  1;
	if (bump) {
	    from = GetCtlValue(f->flsh);
	    SetCtlValue(f->flsh, from+bump);
	    flscroll(f, from, GetCtlValue(f->flsh));
	}
/*-->*/ return(-1);
    }
    retval = (p.v - BigEndianValue(f->flrect.top)) / f->fllinht + GetCtlValue(f->flsh);
    if (retval >= f->flnmfil || MR(*f->flinfo)[retval].flicns & GRAYBIT)
/*-->*/ retval = -1;
    return(retval);
}

A3(PRIVATE, void, flmouse, fltype *, f, Point, p, ControlHandle, ch)
{
    INTEGER newsel;
    EventRecord evt;
    
    evt.where = p;
    do {
	GlobalToLocal(&evt.where);
	p.h = BigEndianValue(evt.where.h);
	p.v = BigEndianValue(evt.where.v);
	if ((newsel = flwhich(f, p)) != f->flsel) {
	    if (f->flsel != -1) {
		safeflflip(f, f->flsel);
		if (newsel == -1) {
		    if (!f->flgraynondirs)
			HiliteControl(ch, 255);
		    set_type_and_name (f, 0, (StringPtr) "");
		  }
	    }
	    if (newsel != -1) {
		safeflflip(f, newsel);
		if (f->flsel == -1)
		    HiliteControl(ch, 0);
		settype(f, newsel);
	    }
	    f->flsel = newsel;
	}
    } while (!GetNextEvent(mUpMask, &evt));
}

A4(static inline, BOOLEAN, ROMlib_CALLFILTERPROC, DialogPtr, dp,
				 EventRecord *, evtp, INTEGER, *ith, filtp, fp)
{
    BOOLEAN retval;
    LONGINT save_ref_con;

    save_ref_con = GetWRefCon (dp);
    SetWRefCon (dp, TICK("stdf"));
    ROMlib_hook(stdfile_filtnumber);
    HOOKSAVEREGS();
	retval = CToPascalCall(&fp, CTOP_SectRect, dp, evtp, ith);
    HOOKRESTOREREGS();
    SetWRefCon (dp, save_ref_con);
    return retval;
}

#if 0 /* Needed to generate a CTOP value */
P4 (PUBLIC pascal trap, BOOLEAN, unused_stdfile, DialogPtr, dp, EventRecord *,
    evp, INTEGER *, ith, UNIV Ptr, data)
{
}
#endif

PRIVATE BOOLEAN
ROMlib_CALL_NEW_FILTER_PROC (DialogPtr dp, EventRecord *evtp, INTEGER *ith,
			     UNIV Ptr data, custom_filtp fp)
{
    BOOLEAN retval;
    LONGINT save_ref_con;

    save_ref_con = GetWRefCon (dp);
    SetWRefCon (dp, TICK("stdf"));
    ROMlib_hook(stdfile_filtnumber);
    HOOKSAVEREGS();
	retval = Executor::CToPascalCall(&fp, CTOP_unused_stdfile, dp, evtp, ith, data);
    HOOKRESTOREREGS();
    SetWRefCon (dp, save_ref_con);
    return retval;
}

A1(PRIVATE, void, getcurname, fltype *, f)
{
    int w;
    CInfoPBRec hpb;
    Rect *r;
    OSErr err;

    hpb.dirInfo.ioCompletion = 0;
    hpb.dirInfo.ioNamePtr    = RM(&f->flcurdirname[0]);
    f->flcurdirname[0] = 0;
    hpb.dirInfo.ioVRefNum    = BigEndianValue(-BigEndianValue(SFSaveDisk));
    hpb.dirInfo.ioFDirIndex  = CWC(-1);
    hpb.dirInfo.ioDrDirID    = CurDirStore;
    err = PBGetCatInfo(&hpb, FALSE);
    if (err != noErr)
      {
	warning_unexpected ("PBGetCatInfo err = %d\n", err);
	str255_from_c_string (f->flcurdirname, "?error?");
      }
    r = &f->flrect;
    w = StringWidth(f->flcurdirname) + 2 + 16 + 3 + 2 + 2 + 4;
#if 1
    if (w > BigEndianValue(r->right) - BigEndianValue(r->left) + 17) {
	f->flcurdirrect.left   = r->left;
	f->flcurdirrect.right  = BigEndianValue(BigEndianValue(r->right) + 17);
    } else {
	f->flcurdirrect.left   = BigEndianValue((BigEndianValue(r->left) + BigEndianValue(r->right) + 17 - w) / 2 - 2);
	f->flcurdirrect.right  = BigEndianValue(BigEndianValue(f->flcurdirrect.left) + w);
    }
#else /* 1 */
    f->flcurdirrect.left   = (BigEndianValue(r->left) + BigEndianValue(r->right) + 17 - w) / 2 - 2;
    f->flcurdirrect.right  = BigEndianValue(BigEndianValue(f->flcurdirrect.left) + w);
#endif /* 1 */
}

A1(PRIVATE, void, flfinit, fltype *, fp)
{
    DisposeControl(fp->flsh);
    DisposHandle((Handle) fp->flinfo);
    DisposHandle((Handle) fp->flstrs);
}

A3(PRIVATE, void, flinsert, fltype *, f, StringPtr, p, INTEGER, micon)
{
    fltype::flinfostr finfo;
    
    finfo.floffs = GetHandleSize((Handle) f->flstrs);   
    finfo.flicns = micon;
    PtrAndHand((Ptr) p, (Handle) f->flstrs, (LONGINT)U(p[0])+1);
    PtrAndHand((Ptr) &finfo, (Handle) f->flinfo,
					    (LONGINT)sizeof(fltype::flinfostr));
    ++f->flnmfil;
}

A3(PRIVATE, int, typeinarray, OSType, ft, INTEGER, numt, SFTypeList, tl)
{
    OSType *ostp = tl;
    
    while (numt--)
	if (ft == *ostp++)
/*-->*/     return(TRUE);
    return(FALSE);
}

/*
 * NOTE: stdfcmp is called from qsort, so we can't smash d2.
 */

/* ip1 and ip2 would really be "void *" in ANSI C */

static LONGINT stdfcmpC(const void *ip1, const void *ip2)
{
	fltype::flinfostr *fp1 = (fltype::flinfostr *) ip1;
	fltype::flinfostr *fp2 = (fltype::flinfostr *) ip2;
    LONGINT retval;

    retval = ROMlib_strcmp((StringPtr) (MR(*holdstr) + fp1->floffs),
			   (StringPtr) (MR(*holdstr) + fp2->floffs));
    return retval;
}


A2(PRIVATE, LONGINT, stdfcmp, char *, ip1, char *, ip2)
{
	return stdfcmpC(ip1, ip2);
}

A2(static inline, BOOLEAN, ROMlib_CALLFILEFILT, ParmBlkPtr, pbp, filefiltp, fp)
{
    BOOLEAN retval;

    ROMlib_hook(stdfile_filefiltnumber);
    HOOKSAVEREGS();
    retval = CToPascalCall(&fp, CTOP_SystemEvent, pbp);
    HOOKRESTOREREGS();
    return retval;
}

PRIVATE BOOLEAN
ROMlib_CALL_CUSTOM_FILE_FILT (ParmBlkPtr pbp, UNIV Ptr data,
			      custom_file_filtp fp)
{
    BOOLEAN retval;

    ROMlib_hook(stdfile_filefiltnumber);
    HOOKSAVEREGS();
    retval = CToPascalCall(&fp, CTOP_GetAuxCtl, pbp, data);
    HOOKRESTOREREGS();
    return retval;
}

PRIVATE boolean_t
passes_filter (fltype *f, CInfoPBRec *cinfop, INTEGER numt)
{
  boolean_t retval;

  switch (f->flavor)
    {
    case original_sf:
      /* NOTE: the code for original_sf was changed after we ran into
	 some trouble with Accordance.  It's not clear whether or not
	 the new code here should also be used for new_sf.  */
      retval = (!f->flfilef.oflfilef
		|| (numt == 0
		    && (cinfop->hFileInfo.ioFlAttrib & ATTRIB_ISADIR))
		|| !CALLFILEFILT ((ParmBlkPtr) cinfop, f->flfilef.oflfilef));
    case new_sf:
      retval = (!f->flfilef.oflfilef
		|| (cinfop->hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
		|| !CALLFILEFILT ((ParmBlkPtr) cinfop, f->flfilef.oflfilef));
      break;
    case new_custom_sf:
      retval = (!f->flfilef.cflfilef
		|| !CALL_CUSTOM_FILE_FILT ((ParmBlkPtr) cinfop, f->mydata,
					   f->flfilef.cflfilef));
      break;
    default:
      warning_unexpected ("flavor = %d", f->flavor);
      retval = TRUE;
      break;
    }
  return retval;
}

A1(PRIVATE, void, flfill, fltype *, f)
{
    CInfoPBRec pb;
    OSErr err;
    Str255 s;
    int micon;
    CursHandle watchh;
    INTEGER errcount;
    INTEGER dirindex;

    SetCursor(STARH((watchh = GetCursor(watchCursor))));

    pb.hFileInfo.ioNamePtr  = RM(&s[0]);
    pb.hFileInfo.ioVRefNum  = BigEndianValue(-BigEndianValue(SFSaveDisk));
    err = noErr;
    errcount = 0;
    for (dirindex = 1; err != fnfErr && errcount != 3; dirindex++) {
	pb.hFileInfo.ioFDirIndex = BigEndianValue(dirindex);
	pb.hFileInfo.ioDirID    = CurDirStore;
	err = PBGetCatInfo(&pb, FALSE);
	if (err) {
	    if (err != fnfErr) {
	        warning_unexpected ("PBGetCatInfo err = %d\n", err);
		++errcount;
		/* register int d7 = err; */
		
	    }
	} else {
	    errcount = 0;
	    if (f->flnumt <= 0 || (pb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR) ||
		   typeinarray(pb.hFileInfo.ioFlFndrInfo.fdType, f->flnumt,
								      f->fltl))
	        if (passes_filter (f, &pb, f->flnumt))
		  {
		    if (pb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
			micon = MICONCFOLDER;
		    else if (pb.hFileInfo.ioFlFndrInfo.fdType == TICKX("APPL"))
			micon = MICONAPP | f->flgraynondirs;
		    else
			micon = MICONLETTER | f->flgraynondirs;
		    flinsert(f, MR(pb.hFileInfo.ioNamePtr), micon);
		  }
	}
    }
    if (f->flnmfil > f->flnmlin) {
	SetCtlMax(f->flsh, f->flnmfil - f->flnmlin);
	SetCtlValue(f->flsh, 0);
    } else
	SetCtlMax(f->flsh, 0);

    if (f->flnmfil > 0) {
	holdstr = f->flstrs;
		::qsort(MR(*f->flinfo), f->flnmfil, sizeof(**f->flinfo), stdfcmpC);
	if (!(MR(*f->flinfo)[0].flicns&GRAYBIT) && !f->flgraynondirs) {
	    f->flsel = 0;
	    settype(f, 0);
	    HiliteControl(f->flch, 0);
	}
    } else
	if (!f->flgraynondirs)
	    HiliteControl(f->flch, 255);
    /* i'm not sure this is right, but there is no obvious way to
       save/restore the cursor */
    SetCursor (&arrowX);
}

/*
 * ROMlib_filebox should be renamed to reflect the fact that it also draws the
 * dotted line (whopee!)
 */

P2(PUBLIC, pascal void,  ROMlib_filebox, DialogPeek, dp, INTEGER, which)
{
    HIDDEN_Handle h;
    Rect r, r2;
    INTEGER i;
    int width, strwidth, offset;
    StringPtr diskname;
    HIDDEN_Handle ejhand;
    BOOLEAN ejectable;
    PenState ps;

    GetPenState(&ps);
    PenNormal();
    
    h.p = NULL;
    GetDItem((DialogPtr) dp, which, &i, &h, &r);
/*    h.p = BigEndianValue(h.p); we don't really use h */
    switch (which) {
    case getNmList:
    case putNmList:
        if (h.p)
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
	width = BigEndianValue(r.right) - BigEndianValue(r.left);
	diskname = getdiskname( &ejectable, NULL );
	GetDItem((DialogPtr) dp, putEject, &i, &ejhand, &r2);
	ejhand.p = MR(ejhand.p);
	if (ejectable)
	    HiliteControl((ControlHandle) ejhand.p, 0);
	else
	    HiliteControl((ControlHandle) ejhand.p, 255);
	strwidth = StringWidth(diskname) + 2 + 16 + 3;
	offset = (width - strwidth) / 2;
	if (offset < 3)
	    r.left = BigEndianValue(BigEndianValue(r.left) + (3));
	else
	    r.left = BigEndianValue(BigEndianValue(r.left) + (offset));
	r.bottom = BigEndianValue(WINDFL(dp)->flascent);
	drawinboxwithicon(diskname, &r, ejectable ? MICONFLOPPY : MICONDISK);
	break;
    }
    SetPenState(&ps);
}


A2(PRIVATE, void, realcd, DialogPeek, dp, LONGINT, dir)
{
    fltype *fp;

    CurDirStore = BigEndianValue(dir);
    fp = WINDFL(dp);
    SetHandleSize((Handle) fp->flinfo, (Size) 0);
    SetHandleSize((Handle) fp->flstrs, (Size) 0);
    fp->flnmfil = 0;
    fp->flsel = -1;
    set_type_and_name (fp, 0, (StringPtr) "");
    flfill(fp);
    fp->flcurdirrect.right  = BigEndianValue(BigEndianValue(fp->flcurdirrect.right ) + 1);
    fp->flcurdirrect.bottom = BigEndianValue(BigEndianValue(fp->flcurdirrect.bottom) + 1);
    EraseRect(&fp->flcurdirrect);
    getcurname(fp);
    fp->flcurdirrect.bottom = BigEndianValue(BigEndianValue(fp->flcurdirrect.bottom) - 1);
				  /* don't need to do right; getcurname does */
    drawjobberattop(dp);
    C_ROMlib_filebox(dp, getDiskName);
    EraseRect(&fp->flrect);
    if (fp->flgraynondirs)
	C_ROMlib_filebox(dp, putNmList);
    else
	C_ROMlib_filebox(dp, getNmList);
}

A1(PRIVATE, LONGINT, getparent, LONGINT, dirid)
{
    OSErr err;
    CInfoPBRec cb;
    LONGINT retval;

    cb.dirInfo.ioCompletion = 0;
    cb.dirInfo.ioNamePtr = 0;
    cb.dirInfo.ioVRefNum = BigEndianValue(-BigEndianValue(SFSaveDisk));
    cb.dirInfo.ioFDirIndex = CWC (-1);
    cb.dirInfo.ioDrDirID = BigEndianValue(dirid);
    err = PBGetCatInfo(&cb, FALSE);
    if (err == noErr)
      retval = BigEndianValue(cb.dirInfo.ioDrParID);
    else
      {
	warning_unexpected ("PBGetCatInfo return = %d\n", err);
	retval = 2;
      }
    return retval;
}

PRIVATE BOOLEAN findparent(INTEGER *vrefp, LONGINT *diridp)
{
    HVCB *vcbp;
    BOOLEAN retval;
    struct stat sbuf;
    char *namecpy, *slashp;
    INTEGER namelen;

    vcbp = ROMlib_vcbbyvrn(BigEndianValue(*vrefp));
    retval = FALSE;
    if (!vcbp->vcbCTRef) {
	namelen = strlen(((VCBExtra *) vcbp)->unixname);
	if (namelen != 1 + SLASH_CHAR_OFFSET) {	/* i.e. "/" */
	    namecpy = (char*)alloca(namelen + 1);
	    strcpy(namecpy, ((VCBExtra *) vcbp)->unixname);
	    slashp = strrchr(namecpy, '/');
	    if (slashp == namecpy + SLASH_CHAR_OFFSET) {
		slashp[1] = 0;
	    } else {
		slashp[0] = 0;
	    }
	    if (Ustat(namecpy, &sbuf) == 0 &&
			       (vcbp = ROMlib_vcbbybiggestunixname(namecpy))) {
		*vrefp = vcbp->vcbVRefNum;
		*diridp = BigEndianValue((LONGINT) ST_INO (sbuf));
		retval = TRUE;
	    }
	}
    }
    return retval;
}

A1(PRIVATE, BOOLEAN, moveuponedir, DialogPtr, dp)
{
    LONGINT parent;
    INTEGER vrn;
    BOOLEAN retval;

    parent = getparent(BigEndianValue(CurDirStore));
    if (parent != BigEndianValue(CurDirStore) && parent != 1) {
	CurDirStore = BigEndianValue(parent);
	retval = TRUE;
    } else {
	vrn = BigEndianValue(-BigEndianValue(SFSaveDisk));
	retval = findparent(&vrn, &CurDirStore);
	SFSaveDisk = BigEndianValue(-BigEndianValue(vrn));
    }
    return retval;
}

BOOLEAN keyarrow(fltype *fl, INTEGER incr)	/* -1: up, 1: down */
{
    INTEGER nsel, oldval, newval;
    fltype::flinfostr *flp;
/*
 * If there's no selection we start at the top or bottom, depending on
 * whether we're going down or p
 */
    if (fl->flsel == -1)
	nsel = incr > 0 ? 0 : fl->flnmfil - 1;
    else
	nsel = fl->flsel + incr;

/*
 * If we land on a grayed out file, advance until non-grayed
 */

    for (flp = MR(*fl->flinfo) + nsel;
	       (flp->flicns & GRAYBIT) && nsel >= 0 && nsel < fl->flnmfil;
						     nsel += incr, flp += incr)
	;

/*
 * If we don't find a non-grayed out entry, we leave
 */
    if (nsel < 0 || nsel >= fl->flnmfil)
/*-->*/	return FALSE;

/*
 * Figure out what should be visible and scroll there if necessary
 */

    newval = oldval = GetCtlValue(fl->flsh);
    if (nsel < oldval)
	newval = nsel;
    else if (nsel >= oldval + fl->flnmlin)
	newval = nsel - fl->flnmlin + 1;
    if (newval != oldval) {
	SetCtlValue(fl->flsh, newval);
	flscroll(fl, oldval, newval);
    }

/*
 * Get rid of the previously selected rectangle
 */
    if (fl->flsel != -1)
	safeflflip(fl, fl->flsel);
/*
 * Flip the new rectangle, set up the type information and make assignment
 */
    safeflflip(fl, nsel);
    settype(fl, nsel);
    fl->flsel = nsel;
    return TRUE;
}

PRIVATE boolean_t
folder_selected_p (fltype *fl)
{
  boolean_t retval;

  switch (fl->flavor)
    {
    case original_sf:
      retval = !!fl->flreplyp.oreplyp->fType;
      break;
    case new_sf:
    case new_custom_sf:
      retval = fl->flreplyp.nreplyp->sfIsFolder;
      break;
    default:
      warning_unexpected ("flavor = %d", fl->flavor);
      retval = FALSE;
      break;
    }

  return retval;
}

PRIVATE INTEGER
call_magicfp (fltype *fl, DialogPeek dp, EventRecord *evt, INTEGER *ith)
{
  INTEGER retval;

  switch (fl->flavor)
    {
    case original_sf:
    case new_sf:
      retval = fl->magicfp.ofilterp
	? CALLFILTERPROC ((DialogPtr) dp, evt, ith, fl->magicfp.ofilterp)
	  : FALSE;
      break;
    case new_custom_sf:
      retval = fl->magicfp.cfilterp
	? CALL_NEW_FILTER_PROC ((DialogPtr) dp, evt, ith, fl->mydata,
				fl->magicfp.cfilterp)
	  : FALSE;
      break;
    default:
      warning_unexpected ("flavor = %d", fl->flavor);
      retval = FALSE;
    }

  return retval;
}

#define keydownbit	0x1000

P3(PUBLIC, pascal INTEGER,  ROMlib_stdffilt, DialogPeek, dp,
		  EventRecord *, evt, INTEGER *, ith)  /* handle disk insert */
{
    LONGINT ticks;
    INTEGER i, from;
    HIDDEN_ControlHandle h;
    Rect r;
    Point p;
    INTEGER t;
    fltype *fl;
    INTEGER opentoken;
    fltype::flinfostr  *flp, *flep;
    INTEGER nsel, fltop;
    INTEGER part;
    INTEGER retval, retval2;

    fl = WINDFL(dp);
    opentoken = getOpen;	/* getOpen and putSave are both 1 */
    retval = FALSE;
    switch (BigEndianValue(evt->what)) {
    case keyDown:
	*ith = BigEndianValue((BigEndianValue(evt->message) & 0xFF) + keydownbit);
	switch (BigEndianValue(evt->message) & 0xFF) {
	case NUMPAD_ENTER:
	case '\r' :
	    GetDItem((DialogPtr) dp, BigEndianValue(dp->aDefItem), &i, (HIDDEN_Handle *) &h, &r);
	    h.p = MR(h.p);
	    if (Hx(h.p, contrlVis) && U(Hx(h.p, contrlHilite)) != 255)
	      {
		prefix[0] = 0;
		oldticks = -1000;
		*ith = BigEndianValue(opentoken);
		retval = -1;
//#if !defined(MACOSX_)
		HiliteControl(h.p, inButton);
		Delay((LONGINT)5, (LONGINT *) 0);
		HiliteControl(h.p, 0);
//#endif
	      }
	    break;
	case ASCIIUPARROW:
	    if (BigEndianValue(evt->modifiers) & cmdKey)
		*ith = CWC(getDiskName);	/* the same as putDiskName */
	    else
		keyarrow(fl, -1);
	    retval = -1;
	    break;
	case ASCIIDOWNARROW:
	    if ((BigEndianValue(evt->modifiers) & cmdKey) && fl->flsel != -1 &&
			   (MR(*fl->flinfo) + fl->flsel)->flicns == MICONCFOLDER) {
		prefix[0] = 0;
		oldticks = -1000;
		*ith = CWC(opentoken);
	    } else
		keyarrow(fl, 1);
	    retval = -1;
	    break;
	case '\t':
	    *ith = CWC(putDrive); /* PutDrive is 6 which is also getDrive. */
	    retval = -1;
	    break;
        case '.':
	    if (evt->modifiers & CWC(cmdKey))
	      {
		*ith = BigEndianValue(fl->fl_cancel_item);
		retval = -1;
		break;
	      }
        default:
/*
 * The Cx(dp->editField) check was made to get HFS_XFer to work.  There
 * may be a better place to put it, but not enough tests have been done
 * to say where.
 */
	    if (!fl->flgraynondirs && dp->editField == -1) {
		flep = MR(*fl->flinfo) + fl->flnmfil - 1;
		if (BigEndianValue(evt->when) > lastkeydowntime + BigEndianValue(DoubleTime)) {
		    flp = MR(*fl->flinfo);
		    prefix[0] = 0;
		    oldticks = -1000;
		} else
		    flp = MR(*fl->flinfo) + ((fl->flsel) == -1 ? 0 : fl->flsel);
		lastkeydowntime = BigEndianValue(evt->when);
		prefix[++prefix[0]] = BigEndianValue(evt->message) & 0xff;
		while (flp < flep &&
			    RelString((StringPtr) (MR(*fl->flstrs) +
					flp->floffs), prefix, FALSE, TRUE) < 0)
		    ++flp;
		nsel = flp - MR(*fl->flinfo);
		if (nsel != fl->flsel) {
		    safeflflip(fl, fl->flsel);
		    fltop = GetCtlValue(fl->flsh);
		    if (nsel < fltop || nsel >= fltop + fl->flnmlin) {
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
	p = evt->where;
	GlobalToLocal(&p);
	p.h = BigEndianValue(p.h);
	p.v = BigEndianValue(p.v);
	if (PtInRect(p, &fl->flrect)) {
	    GetDItem((DialogPtr) dp, getOpen, &i, (HIDDEN_Handle *) &h, &r);
	    h.p = MR(h.p);
	    flmouse(fl, evt->where, h.p);
	    ticks = TickCount();
	    if (fl->flsel != -1 && savesel == fl->flsel &&
					 (ticks < oldticks + BigEndianValue(DoubleTime))) {
		prefix[0] = 0;
		*ith = CWC(opentoken);
		oldticks = -1000;
		retval = -1;
	    } else
		oldticks = ticks;
	    savesel = fl->flsel;
	} else if ((t = TestControl(fl->flsh, p))) {
	    if (t == inThumb) {
		from = GetCtlValue(fl->flsh);
		TrackControl(fl->flsh, p, (ProcPtr) 0);
		flscroll(fl, from, GetCtlValue(fl->flsh));
	    } else
		TrackControl(fl->flsh, p, (ProcPtr) P_ROMlib_stdftrack);
	} else if (PtInRect(p, &fl->flcurdirrect)) {
	    *ith = CWC(FAKECURDIR);
	    retval = -1;
	} else {
	    GetDItem((DialogPtr) dp, getOpen, &i, (HIDDEN_Handle *) &h, &r);
	    h.p = MR(h.p);
	    if ((part = TestControl(h.p, p)) &&
			     TrackControl(h.p, p, (ProcPtr) 0)) {
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
	if ((DialogPeek) MR(evt->message) == dp)
	    drawjobberattop(dp);
	*ith = CWC(100);
	break;
    }
    retval2 = call_magicfp (fl, dp, evt, ith);

    if (*ith == CWC(getOpen) && folder_selected_p (fl))	/* 1 is getOpen and putSave */
	*ith = CWC(FAKEOPENDIR);
    
    return retval ? retval : retval2;
}

A3(PRIVATE, void, flinit, fltype *, f, Rect *, r, ControlHandle, sh)
{
    FontInfo fi;
    THz savezone;

    GetFontInfo(&fi);
    
    f->flsh = sh;
    f->flrect = *r;
    f->fllinht = BigEndianValue(fi.ascent) + BigEndianValue(fi.descent) + BigEndianValue(fi.leading);

    f->flcurdirrect.top    = BigEndianValue(BigEndianValue(r->top) - f->fllinht - 5);
    f->flcurdirrect.bottom = BigEndianValue(BigEndianValue(f->flcurdirrect.top) + f->fllinht);
    getcurname(f);

    f->flascent = BigEndianValue(fi.ascent);
    f->flnmlin = (BigEndianValue(r->bottom) - BigEndianValue(r->top)) / f->fllinht;
    f->flnmfil = 0;
    f->flsel = -1;
    savezone = TheZone;
    TheZone = SysZone;
    f->flinfo = (fltype::flinfostr **) NewHandle((Size)0);
    f->flstrs = (char **) NewHandle((Size)0);
    TheZone = savezone;
}

A3(PRIVATE, void, stdfflip, Rect *, rp, INTEGER, n, INTEGER, height)
{
    INTEGER savetop = rp->top;
	
    rp->top = BigEndianValue(BigEndianValue(rp->top) + (n * height + 1));
    rp->bottom = BigEndianValue(BigEndianValue(rp->top) + height - 2);
    InvertRect(rp);   
    rp->top = savetop;
}

A1(PRIVATE, BOOLEAN, trackdirs, DialogPeek, dp)
{
  WRAPPER_PIXMAP_FOR_COPY (wrapper);
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
  boolean_t seen_up_already;
  TEMP_ALLOC_DECL (temp_save_bits);
  
  THEPORT_SAVE_EXCURSION
    (MR (wmgr_port),
     {
       int str_width;
       
       next = &first;
       hpb.dirInfo.ioCompletion = 0;
       hpb.dirInfo.ioNamePtr    = RM (&next->name[0]);
       next->name[0] = 0;
       hpb.dirInfo.ioVRefNum    = BigEndianValue (-BigEndianValue(SFSaveDisk));
       hpb.dirInfo.ioFDirIndex  = CWC (-1);
       hpb.dirInfo.ioDrDirID    = CurDirStore;
       max_width = 0;
       count = 0;
       done = FALSE;
       
       do  
	 {
	   OSErr err;

	   err = PBGetCatInfo (&hpb, FALSE);
	   if (err != noErr)
	     {
	       warning_unexpected ("PBGetCatInfo returns err = %d\n", err);
	       done = TRUE;
	     }
	   id = next->dirid = BigEndianValue (hpb.dirInfo.ioDrDirID);
	   next->vrefnum = BigEndianValue (hpb.dirInfo.ioVRefNum);
	   next->next = (struct link *) ALLOCA (sizeof (struct link));
	   gui_assert (next->next);
	   str_width = StringWidth (next->name);
	   if (str_width > max_width)
	     max_width = str_width;
	   next = next->next;	/* make Steve Jobs happy */
	   hpb.dirInfo.ioDrDirID    = hpb.dirInfo.ioDrParID;
	   hpb.dirInfo.ioNamePtr    = RM (&next->name[0]);
	   next->name[0] = 0;
	   count++;
	   if (id == 2) 
	     {
	       if (!findparent (&hpb.dirInfo.ioVRefNum, &hpb.dirInfo.ioDrDirID))
		 done = TRUE;
	     }
	 } while (!done);
       fl = WINDFL(dp);
       therect.top  = BigEndianValue (BigEndianValue (fl->flcurdirrect.top)
			  - BigEndianValue (PORT_BOUNDS (dp).top));
       therect.left = BigEndianValue (BigEndianValue (fl->flcurdirrect.left)
			  - BigEndianValue (PORT_BOUNDS (dp).left));
       therect.bottom = BigEndianValue (BigEndianValue (therect.top) + count * fl->fllinht + 1);
       therect.right  = BigEndianValue (BigEndianValue (therect.left) + 2 + 16 + 3 + max_width + 4 + 2 + 3);
       ClipRect(&therect);

       {
	 Rect *bounds;
	 PixMapHandle port_pixmap;
	 INTEGER save_bpp_x;
	 int row_bytes;
	 void *save_bits_mem;
	 
	 save_bits = NewPixMap ();
	 port_pixmap = CPORT_PIXMAP (thePort);
	 
	 bounds = &PIXMAP_BOUNDS (save_bits);
	 bounds->top    = CWC (0);
	 bounds->left   = CWC (0);
	 bounds->bottom = BigEndianValue (RECT_HEIGHT (&therect));
	 bounds->right  = BigEndianValue (RECT_WIDTH (&therect));
	 PIXMAP_PIXEL_SIZE_X (save_bits) = save_bpp_x
	   = PIXMAP_PIXEL_SIZE_X (port_pixmap);
	 ROMlib_copy_ctab (PIXMAP_TABLE (port_pixmap),
			   PIXMAP_TABLE (save_bits));
	 row_bytes = ((BigEndianValue (bounds->right) * BigEndianValue (save_bpp_x) + 31) /  32) * 4;
	 PIXMAP_SET_ROWBYTES_X (save_bits, BigEndianValue (row_bytes));

	 /* Allocate potentially large temporary pixmap space. */
	 TEMP_ALLOC_ALLOCATE (save_bits_mem, temp_save_bits,
			      BigEndianValue (bounds->bottom) * row_bytes);
	 PIXMAP_BASEADDR_X (save_bits) = (Ptr)RM (save_bits_mem);
	 WRAPPER_SET_PIXMAP_X (wrapper, RM (save_bits));
	 
	 CopyBits (PORT_BITS_FOR_COPY (thePort), wrapper,
		   &therect, bounds, srcCopy, NULL);
       }
       
       EraseRect(&therect);
       
       /* loop through, displaying stuff */
       /* highlite the appropriate box, etc. */
       
       fillinrect.top    = therect.top;
       fillinrect.left   = BigEndianValue(BigEndianValue(therect.left) + 2);
       fillinrect.bottom = BigEndianValue(fl->flascent);
       fillinrect.right  = therect.right;
       
       for (i = count, next = &first; --i >= 0; next = next->next)
	 {

	   /* TODO: ask cliff about a better way to do this */
	   /* unused = */
	   getdiskname (&ejectable, NULL);
	   drawinboxwithicon(next->name, &fillinrect,
			     i ? MICONCFOLDER : ejectable ? MICONFLOPPY : MICONDISK);
/*
	   drawinboxwithicon(next->name, &fillinrect,
			     i ? MICONCFOLDER : MICONDISK);
*/
	   fillinrect.top = BigEndianValue(BigEndianValue(fillinrect.top) + (fl->fllinht));
	 }
       
       therect.right = BigEndianValue(BigEndianValue(therect.right) - (1));
       therect.bottom = BigEndianValue(BigEndianValue(therect.bottom) - (1));
       FrameRect(&therect);
       MoveTo(BigEndianValue(therect.left)+1,  BigEndianValue(therect.bottom));
       LineTo(BigEndianValue(therect.right), BigEndianValue(therect.bottom));
       LineTo(BigEndianValue(therect.right),   BigEndianValue(therect.top)+1);
       therect.right = BigEndianValue(BigEndianValue(therect.right) + (1));
       
       sel = 0;
       fillinrect.top = therect.top;
       fillinrect.left = BigEndianValue (BigEndianValue (fillinrect.left) - 1);
       fillinrect.right = BigEndianValue (BigEndianValue (fillinrect.right) - 2);
       stdfflip (&fillinrect, sel, fl->fllinht);
       done = FALSE;
       seen_up_already = FALSE;
       firstsel = -1;
       while (!done)
	 {
	   evt.where.h = BigEndianValue (evt.where.h);
	   evt.where.v = BigEndianValue (evt.where.v);
	   if (PtInRect (evt.where, &therect))
	     newsel = (evt.where.v - BigEndianValue(therect.top)) / fl->fllinht;
	   else
	     newsel = -1;
	   if (newsel != sel)
	     {
	       if (firstsel == -1)
		 firstsel = newsel;
	       if (sel != -1)
		 stdfflip (&fillinrect, sel, fl->fllinht);
	       if (newsel != -1)
		 stdfflip (&fillinrect, newsel, fl->fllinht);
	       sel = newsel;
	     }
	   if (!ROMlib_sticky_menus_p)
	     done = GetNextEvent (mUpMask, &evt);
	   else
	     {
	       if (OSEventAvail (mUpMask, &evt))
		 {
		   if (seen_up_already || (sel != firstsel && firstsel != -1))
		     done = TRUE;
		   else
		     {
		       GetOSEvent (mUpMask, &evt);
		       seen_up_already = TRUE;
		     }
		 }
	       if (!done && OSEventAvail (mDownMask, &evt))
		 {
		   GetOSEvent (mDownMask, &evt);
		   done = sel != -1;
		 }
	     }
	 }
       
       therect.bottom = BigEndianValue (BigEndianValue (therect.bottom) + 1);

       /* restore the rect and clean up after ourselves */
       CopyBits(wrapper, PORT_BITS_FOR_COPY (thePort),
		&PIXMAP_BOUNDS (save_bits), &therect, srcCopy, NULL);
       DisposPixMap (save_bits);
     });
  if (sel != -1)
    {
      for (i = 0, next = &first; i != sel; ++i, next = next->next)
	;
      CurDirStore = BigEndianValue(next->dirid);
      SFSaveDisk  = BigEndianValue(-next->vrefnum);
      return TRUE;
    }
  ALLOCAEND

  TEMP_ALLOC_FREE (temp_save_bits);

  return FALSE;
}

PRIVATE void
makeworking (fltype *f)
{

  switch (f->flavor)
    {
    case original_sf:
      {
	WDPBRec wdpb;
	OSErr err;
	
	wdpb.ioVRefNum  = BigEndianValue(-BigEndianValue(SFSaveDisk));
	wdpb.ioWDDirID  = CurDirStore;
	wdpb.ioWDProcID = TICKX("STDF");
	wdpb.ioNamePtr  = 0;
	err = PBOpenWD(&wdpb, FALSE);
	if (err != noErr)
	  warning_unexpected ("PBOpenWD returns %d\n", err);
	
	f->flreplyp.oreplyp->vRefNum = wdpb.ioVRefNum;
      }
      break;
    case new_sf:
    case new_custom_sf:
      f->flreplyp.nreplyp->sfFile.vRefNum = BigEndianValue(-BigEndianValue(SFSaveDisk));
      f->flreplyp.nreplyp->sfFile.parID = CurDirStore;
      break;
    default:
      warning_unexpected ("flavor = %d", f->flavor);
      break;
    }
}

A1(PRIVATE, BOOLEAN, ejected, HParmBlkPtr, pb)
{
     return pb->volumeParam.ioVDrvInfo == 0;   
}

/*
 * returns true if the filesystem that pb refers to is part of a single
 * tree, as in UNIX.  Under MSDOS, filesystems are distinct (i.e. C:, D:,
 * etc.)
 */

PRIVATE boolean_t single_tree_fs_p(HParmBlkPtr pb)
{
#if defined(MSDOS) || defined (CYGWIN32)
  return FALSE;
#else
    HVCB *vcbp = ROMlib_vcbbyvrn(BigEndianValue(pb->volumeParam.ioVRefNum));
	
    return vcbp && !vcbp->vcbCTRef;
#endif
}

#if defined(LINUX)
PUBLIC int linuxfloppy_open(int disk, LONGINT *bsizep,
			     drive_flags_t *flagsp, const char *dname)
{
  int retval;
  struct cdrom_subchnl sub_info;
  boolean_t force_read_only;

  force_read_only = FALSE;
  *flagsp = 0;
#define FLOPPY_PREFIX "/dev/fd"
  if (strncmp (dname, FLOPPY_PREFIX, sizeof(FLOPPY_PREFIX)-1) == 0)
    *flagsp |= DRIVE_FLAGS_FLOPPY;

  if (!force_read_only)
    retval = Uopen (dname, O_RDWR, 0);
#if !defined (LETGCCWAIL)
  else
    retval = noErr;
#endif
  if (force_read_only || retval < 0)
    {
      retval = Uopen (dname, O_RDONLY, 0);
      if (retval >= 0)
	*flagsp |= DRIVE_FLAGS_LOCKED;
    }

  memset (&sub_info, 0, sizeof sub_info);
  sub_info.cdsc_format = CDROM_MSF;
  if (retval >= 0 && ioctl (retval, CDROMSUBCHNL, &sub_info) != -1)
    {
      switch (sub_info.cdsc_audiostatus)
	{
	case CDROM_AUDIO_PLAY:
	case CDROM_AUDIO_PAUSED:
	case CDROM_AUDIO_COMPLETED:
	case CDROM_AUDIO_ERROR:
	  close (retval);
	  return -1;
	}
    }


  *bsizep = 512;
  if (retval >= 0)
    {
      /* *ejectablep = TRUE; DRIVE_FLAGS_FIXED not set */
    }

  return retval;
}

PRIVATE int linuxfloppy_close(int disk)
{
  return close(disk);
}
#endif

typedef struct {
    const char *dname;
    DrvQExtra *dqp;
    BOOLEAN loaded;
    int fd;
} dosdriveinfo_t;

#define IGNORED (-1) /* doesn't really matter */

#if defined(MSDOS) || defined (CYGWIN32)

#define N_DRIVES (32)

PRIVATE boolean_t drive_loaded [N_DRIVES] = { 0 };

PRIVATE char *
drive_name_of (int i)
{
  static char retval[] = "A:";

  retval[0] = 'A' + i;
  return retval;
}

enum { NUM_FLOPPIES = 2, NON_FLOPPY_BIT = 0x80 };

PRIVATE int
fd_of (int i)
{
  int retval;

#if defined (CYGWIN32)
  retval = i;
#else
  if (i < NUM_FLOPPIES)
    retval = i;
  else
    retval = i - NUM_FLOPPIES + NON_FLOPPY_BIT;
#endif
  return retval;
}

#define DRIVE_NAME_OF(x) drive_name_of (x)
#define FD_OF(x) fd_of (x)
#define DRIVE_LOADED(x) drive_loaded[x]

#else

#define DRIVE_NAME_OF(x) drives[x].dname

#define FD_OF(x) drives[x].fd

#define DRIVE_LOADED(x) drives[x].loaded

#endif

PUBLIC void Executor::futzwithdosdisks( void )
{
#if defined (MSDOS) || defined (LINUX) || defined(CYGWIN32)
    int i, fd;
    LONGINT mess;
    LONGINT blocksize;
    drive_flags_t flags;
#if defined(MSDOS) || defined (CYGWIN32)
/* #warning "We're cheating on DOS drive specs: ejectable, bsize, maxsize, writable" */
#define OPEN_ROUTINE dosdisk_open
#define CLOSE_ROUTINE dosdisk_close
#define EXTRA_CLOSE_PARAM , FALSE
#define MARKER DOSFDBIT
#define EXTRA_PARAM
#define ROMLIB_MACDRIVES ROMlib_macdrives
#elif defined(LINUX)
    static dosdriveinfo_t drives[] = {
	{ "/dev/fd0", (DrvQExtra *) 0, FALSE, IGNORED },
	{ "/dev/cdrom", (DrvQExtra *) 0, FALSE, IGNORED },
#if 0
	{ "/dev/fd1", (DrvQExtra *) 0, FALSE, IGNORED },
#endif
    };
#define N_DRIVES NELEM(drives)
#define OPEN_ROUTINE linuxfloppy_open
#define CLOSE_ROUTINE linuxfloppy_close
#define EXTRA_CLOSE_PARAM
#define MARKER 0
#define EXTRA_PARAM , (DRIVE_NAME_OF (i))
#define ROMLIB_MACDRIVES (~0)
#endif

    /* Since we're scanning for new disks, let's be paranoid and
     * flush all cached disk information.
     */
    dcache_invalidate_all (TRUE);

#if defined (MSDOS)
    aspi_rescan ();
#endif

    if (!nodrivesearch_p)
      {
	for (i = 0; i < N_DRIVES; ++i) {
	  if (/* DRIVE_LOADED(i) */ ROMLIB_MACDRIVES & (1 << i)) {
#if defined (MSDOS) || defined (CYGWIN32)
	    checkpoint_macdrive (checkpointp, begin, 1 << i);
#endif
	    if (((fd = OPEN_ROUTINE(FD_OF (i), &blocksize, &flags
				    EXTRA_PARAM)) >= 0)
		|| (flags & DRIVE_FLAGS_FLOPPY)) {
	      try_to_mount_disk( DRIVE_NAME_OF (i), fd|MARKER, &mess,
				blocksize, 16 * PHYSBSIZE,
				flags, 0);
	      mess = BigEndianValue(mess);
	      if (mess) {
		if (mess >> 16 == 0) {
		  DRIVE_LOADED(i) = TRUE;
		  PPostEvent(diskEvt, mess, (HIDDEN_EvQElPtr *) 0);
		  /* TODO: we probably should post if mess returns an
		     error, but I think we get confused if we do */
		} else {
		  if (!(flags & DRIVE_FLAGS_FLOPPY))
		    CLOSE_ROUTINE(fd EXTRA_CLOSE_PARAM);
		}
	      } else {
		if (!(flags & DRIVE_FLAGS_FLOPPY))
		  CLOSE_ROUTINE(fd EXTRA_CLOSE_PARAM);
	      }
	    }
#if defined (MSDOS) || defined (CYGWIN32)
	    checkpoint_macdrive (checkpointp, end, 1 << i);
#endif
	  }
	}
      }
#endif
}


A2(PRIVATE, void, bumpsavedisk, DialogPtr, dp, BOOLEAN, always)
{
    INTEGER current;
    HParamBlockRec pb;
    INTEGER vref;
    OSErr err;
    BOOLEAN is_single_tree_fs, seenus;

#if defined (MSDOS) || defined (CYGWIN32)
/*    static BOOLEAN beenhere = FALSE; */

    if (ROMlib_drive_check /* || !beenhere */) {
      futzwithdosdisks();
/*      beenhere = TRUE; */
    }
#endif
    pb.volumeParam.ioVRefNum = BigEndianValue(-BigEndianValue(SFSaveDisk));
    pb.volumeParam.ioNamePtr = 0;
    pb.volumeParam.ioVolIndex = 0;
    err = PBHGetVInfo(&pb, FALSE);
    if (err != noErr)
      warning_unexpected ("PBHGetVInfo returns %d", err);
    else if (!SFSaveDisk || ISWDNUM(-BigEndianValue(SFSaveDisk)))
	SFSaveDisk = BigEndianValue(-BigEndianValue(pb.volumeParam.ioVRefNum));
    if (always || pb.ioParam.ioResult != noErr || ejected(&pb)) {
	current = pb.volumeParam.ioVRefNum;
	is_single_tree_fs = single_tree_fs_p(&pb);
	pb.volumeParam.ioVolIndex = 0;
	seenus = FALSE;
	vref = 0;
	do {
	    pb.volumeParam.ioVolIndex = BigEndianValue(BigEndianValue(pb.volumeParam.ioVolIndex) + 1);
	    err = PBHGetVInfo(&pb, FALSE);
	    if (err != noErr && !seenus)
	      warning_unexpected ("PBHGetVInfo = %d\n", err);
	    else {
		if (pb.volumeParam.ioVRefNum == current)
		    seenus = TRUE;
		else if (!ejected(&pb) &&
			 (!is_single_tree_fs || !single_tree_fs_p(&pb))) {
		    if (!vref || seenus)
			vref = BigEndianValue(pb.volumeParam.ioVRefNum);
		    if (seenus)
/*-->*/			break;
		}
	    }
	} while (err == noErr);
	if (vref) {
	    SFSaveDisk = BigEndianValue(-vref);
	    CurDirStore = CLC(2);
	}
    }
}

#if 0 /* needed to construct ctopflags */
P3 (PUBLIC pascal trap, INTEGER, unused_stdfile_2, INTEGER, ihit,
    DialogPtr, dp, UNIV Ptr, data)
{
}
#endif

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
ROMlib_CALLDHOOK (fltype *fl, INTEGER ihit, DialogPtr dp, dialog_hook_u dhu)
{
  INTEGER retval;
  LONGINT save_ref_con;

  ROMlib_hook(stdfile_dialhooknumber);
  HOOKSAVEREGS();

  save_ref_con = GetWRefCon (dp);
  emergency_save_ref_con = save_ref_con;
  SetWRefCon (dp, TICK("stdf"));

  switch (fl->flavor)
    {
    case original_sf:
    case new_sf:
      retval = CToPascalCall (&dhu.odh, CTOP_Alert, ihit, dp);
      break;
    case new_custom_sf:
      retval = CToPascalCall (&dhu.cdh, CTOP_unused_stdfile_2, ihit, dp,
			      fl->mydata);
      break;
    default:
      warning_unexpected ("flavor = %d", fl->flavor);
      retval = 0;
      break;
    }
  HOOKRESTOREREGS();
  SetWRefCon (dp, save_ref_con);
  return retval;
}

A4(PRIVATE, void, transformsfpdialog, DialogPtr, dp, Point *, offset,
					  Rect *, scrollrect, BOOLEAN, getting)
{
    INTEGER numitems, windheight, i, j, extrasizeneeded;
    HIDDEN_Handle h;
    Rect r;
    TEPtr tep;

    if (getting) {
	extrasizeneeded = 20;
    } else {
	extrasizeneeded = 110;
	SetRect(scrollrect, 16, 24, 231, 106);
	tep = STARH(MR(((DialogPeek)dp)->textH));
	tep->destRect.top = BigEndianValue(BigEndianValue(tep->destRect.top) + (extrasizeneeded));
	tep->destRect.bottom = BigEndianValue(BigEndianValue(tep->destRect.bottom) + (extrasizeneeded));
	tep->viewRect.top = BigEndianValue(BigEndianValue(tep->viewRect.top) + (extrasizeneeded));
	tep->viewRect.bottom = BigEndianValue(BigEndianValue(tep->viewRect.bottom) + (extrasizeneeded));
    }
    numitems = BigEndianValue(*(INTEGER *)STARH((MR(((DialogPeek)dp)->items)))) + 1;
    for (j = 1 ; j <= numitems ; j++) {
	GetDItem(dp, j, &i, &h, &r);
	i = BigEndianValue(i);
	h.p = MR(h.p);
	if (!getting || BigEndianValue(r.bottom) > BigEndianValue(scrollrect->top)) {
	    r.top = BigEndianValue(BigEndianValue(r.top) + (extrasizeneeded));
	    r.bottom = BigEndianValue(BigEndianValue(r.bottom) + (extrasizeneeded));
	    if (i <= 7 && i >= 4)	/* It's a control */
		MoveControl((ControlHandle) h.p, BigEndianValue(r.left), BigEndianValue(r.top));
	    SetDItem(dp, j, i, h.p, &r);
	}
    }
    windheight = BigEndianValue(dp->portRect.bottom) - BigEndianValue(dp->portRect.top) + extrasizeneeded;
    SizeWindow( (WindowPtr) dp, BigEndianValue(dp->portRect.right) - BigEndianValue(dp->portRect.left),
						       windheight, FALSE);
    if (getting) {
	scrollrect->top = BigEndianValue(BigEndianValue(scrollrect->top) + (extrasizeneeded));
	scrollrect->bottom = BigEndianValue(BigEndianValue(scrollrect->bottom) + (extrasizeneeded));
    }
    InvalRect(&dp->portRect);
    offset->v -= extrasizeneeded / 2;
}

void adjustdrivebutton(DialogPtr dp)
{
    INTEGER count;
    HIDDEN_Handle drhand;
    INTEGER i;
    Rect r;
#if !defined(MSDOS) && !defined(CYGWIN32)
    HVCB *vcbp;
    BOOLEAN seenunix;

    count = 0;
    seenunix = FALSE;
    for (vcbp = (HVCB *) MR(VCBQHdr.qHead); vcbp;
					       vcbp = (HVCB *) MR(vcbp->qLink))
	if (vcbp->vcbCTRef && vcbp->vcbDrvNum)
	    ++count;
	else if (!seenunix) {
	    ++count;
	    seenunix = TRUE;
	}
#else /* defined(MSDOS) */
    count = 2;	/* always allow the user to hit the drive button */
#endif /* defined(MSDOS) */
    GetDItem(dp, putDrive, &i, &drhand, &r);	/* putDrive == getDrive */
    drhand.p = MR(drhand.p);
    HiliteControl((ControlHandle) drhand.p, count > 1 ? 0 : 255);
}

A1(PRIVATE, void, doeject, DialogPtr, dp)
{
    Eject((StringPtr) "", -BigEndianValue(SFSaveDisk));
    adjustdrivebutton(dp);
    bumpsavedisk(dp, TRUE);
}

A3(PRIVATE, OSType, gettypeX, StringPtr, name, INTEGER, vref, LONGINT, dirid)
{
  OSType retval;
  OSErr err;
  HParamBlockRec pbr;

  pbr.fileParam.ioNamePtr = RM (name);
  pbr.fileParam.ioVRefNum = BigEndianValue (vref);
  pbr.fileParam.ioFVersNum = 0;
  pbr.fileParam.ioFDirIndex = CWC (0);
  pbr.fileParam.ioDirID = BigEndianValue (dirid);
  err = PBHGetFInfo (&pbr, FALSE);
  if (err == noErr)
    retval = pbr.fileParam.ioFlFndrInfo.fdType;
  else
    retval = 0;
  return retval;
}

PRIVATE OSErr
unixcore (StringPtr namep, INTEGER *vrefnump, LONGINT *diridp)
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
  pbr.ioParam.ioNamePtr = (StringPtr)CLC(0);
  pbr.ioParam.ioVRefNum = BigEndianValue(vrefnum);
  vcbp = ROMlib_breakoutioname(&pbr, &templ, &tempcp, (BOOLEAN *) 0, TRUE);
  free (tempcp);
#endif
  if (vcbp && !vcbp->vcbCTRef)
    {
      pb.ioParam.ioNamePtr = (StringPtr)CLC(0);
      pb.ioParam.ioVRefNum = pbr.ioParam.ioVRefNum;
      err = ROMlib_nami(&pb, *diridp, NoIndex, &pathname, &filename,
			&endname, FALSE, &vcbp2, &sbuf);
      if (err == noErr)
	{
	  VCBExtra *vcbextrap;

	  namelen = endname - pathname - 1;
	  newname = (char*)alloca(namelen + 1 + namep[0] + 1);
	  strncpy(newname, pathname, namelen);
	  newname[namelen] = '/';
	  strncpy(newname + namelen + 1, (char *) namep+1, namep[0]);
	  newname[namelen + 1 + namep[0]] = 0;
	  ROMlib_automount(newname);
	  vcbextrap = ROMlib_vcbbyunixname(newname);
	  if (vcbextrap)
	    {
	      *vrefnump = BigEndianValue(vcbextrap->vcb.vcbVRefNum);
	      if (*diridp == vcbextrap->u.ufs.ino)
		*diridp = 2;
	    }
	  else
	    err = nsvErr;
	  free (pathname);
	}
    }
  else
    err = nsvErr;
  return err;
}

P1(PUBLIC pascal trap, OSErr, unixmount, CInfoPBRec *, cbp)
{
  OSErr err;

  if (!cbp)
    {
      dofloppymount();
      err = noErr;
    }
  else
    {
      INTEGER vrefnum;
      LONGINT dirid;

      vrefnum = BigEndianValue(cbp->hFileInfo.ioVRefNum);
      dirid = BigEndianValue (cbp->hFileInfo.ioDirID);
      err = unixcore(MR(cbp->hFileInfo.ioNamePtr), &vrefnum, &dirid);
      if (err == noErr)
	{
	  cbp->hFileInfo.ioVRefNum = BigEndianValue (vrefnum);
	  cbp->hFileInfo.ioDirID   = BigEndianValue (dirid);
	}
    }
  return err;
}

PRIVATE void unixcd(fltype *f)
{
    StringPtr name;
    INTEGER vrefnum;
    LONGINT dirid;

    name = (StringPtr) (MR(*f->flstrs) + MR(*f->flinfo)[f->flsel].floffs);
    vrefnum = -BigEndianValue(SFSaveDisk);
    dirid = BigEndianValue (CurDirStore);
    if (unixcore(name, &vrefnum, &dirid) == noErr)
      {
	SFSaveDisk = BigEndianValue (-vrefnum);
	CurDirStore = BigEndianValue (dirid);
      }
}

#if 0
	if (U(rep->fName[0]) > 63) /* remember to clip to 63 characters */
	    rep->fName[0] = 63;
#endif

PRIVATE void
get_starting_point (Point *pp)
{
  INTEGER screen_width, screen_height;
  Rect main_gd_rect;

  main_gd_rect = PIXMAP_BOUNDS (GD_PMAP (MR (MainDevice)));
  screen_width = BigEndianValue (main_gd_rect.right);
  screen_height = BigEndianValue (main_gd_rect.bottom);
  pp->h = (screen_width - STANDARD_WIDTH) / 2;
  pp->v = (screen_height - STANDARD_HEIGHT) / 2;
}

PRIVATE ControlHandle
create_new_folder_button (DialogPtr dp)
{
  ControlHandle retval;
  Handle dial_test;

  dial_test = GetResource (TICK("DLOG"), -6044);
  if (!dial_test)
    retval = NULL;
  else
    {
      INTEGER i;
      HIDDEN_Handle h;
      Rect r;

      GetDItem (dp, sfItemNewFolderUser, &i, &h, &r);
      retval = NewControl ((WindowPtr) dp, &r, (StringPtr) "\012New Folder",
			   TRUE, 0, 0, 0, 0, 0L);
      SetDItem (dp, sfItemNewFolderUser, ctrlItem|btnCtrl,
		(Handle) retval, &r);
    }
  return retval;
}

PRIVATE void
destroy_new_folder_button (DialogPtr dp, ControlHandle ch)
{
  /* May not need to do anything since CloseDialog should clean up all
     outstanding controls */
}

#define SF_NAME(fp) ((fp)->flavor == original_sf \
		     ? (fp)->flreplyp.oreplyp->fName \
		     : (fp)->flreplyp.nreplyp->sfFile.name)

#define SF_GOOD_XP(fp) ((fp)->flavor == original_sf \
		       ? &(fp)->flreplyp.oreplyp->good \
		       : &(fp)->flreplyp.nreplyp->sfGood)

#define SF_FTYPE_XP(fp) ((fp)->flavor == original_sf	\
			 ? &(fp)->flreplyp.oreplyp->fType	\
			 : &(fp)->flreplyp.nreplyp->sfType)

#define SF_VREFNUM_X(fp) ((fp)->flavor == original_sf \
			  ? (fp)->flreplyp.oreplyp->vRefNum \
			  : (fp)->flreplyp.nreplyp->sfFile.vRefNum)

#define SF_VREFNUM(fp) (BigEndianValue (SF_VREFNUM_X (fp)))

#define SF_DIRID_X(fp) ((fp)->flavor == original_sf \
			  ? 0 \
			  : (fp)->flreplyp.nreplyp->sfFile.parID)

#define SF_DIRID(fp) (BigEndianValue (SF_DIRID_X (fp)))

PRIVATE void
getditext (DialogPtr dp, INTEGER item, StringPtr text)
{
  INTEGER i;
  HIDDEN_Handle h;
  Rect r;

  h.p = 0;
  GetDItem (dp, item, &i, &h, &r);
  if (!h.p)
    text[0] = 0;
  else
    {
      h.p = MR (h.p);
      GetIText (h.p, text);
    }
}

PRIVATE void
report_new_folder_failure (OSErr err)
{
  char *message = (char*)alloca (256);
  Str255 str;
  
  switch (err) {
    default:
      sprintf (message, "Error #%d occurred; you may not be able to create a new folder.", err);
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
  str255_from_c_string (str, message);
  ParamText (str, 0, 0, 0);
  NoteAlert (GENERIC_COMPLAINT_ID, (ProcPtr) 0);
}

PRIVATE boolean_t
new_folder_from_dp (DialogPtr dp, fltype *f)
{
  Str255 str;
  HParamBlockRec hpb;
  OSErr err;
  boolean_t retval;
	  
  getditext (dp, 3, str);
  hpb.ioParam.ioVRefNum = BigEndianValue (-BigEndianValue (SFSaveDisk));
  hpb.fileParam.ioDirID = CurDirStore;
  if (str[0] > 31)
    str[0] = 31;
  hpb.ioParam.ioNamePtr = RM (&str[0]);
  err = PBDirCreate (&hpb, FALSE);
  if (err != noErr)
    report_new_folder_failure (err); 
  else
    {
      *SF_FTYPE_XP(f) = hpb.fileParam.ioDirID;
      retval = TRUE;
    }
  retval = err == noErr;
  return retval;
}

PRIVATE boolean_t
do_new_folder (fltype *f)
{
  boolean_t retval;
  GrafPtr gp;
  DialogPtr dp;

  retval = FALSE;
  gp = thePort;
  dp = GetNewDialog (-6044, (Ptr) 0, (WindowPtr)-1);
  if (dp)
    {
      boolean_t done;
      INTEGER ihit;

      SelIText (dp, 3, 0, 32767);
      ShowWindow (dp);
      SelectWindow (dp);
      done = FALSE;
      while (!done)
	{
	  /* TODO: consider a filter that limits the length
	     of the string to 31 letters */
	  ModalDialog (NULL, &ihit);
	  switch (ihit)
	    {
	    default:
	      break;
	    case CWC (1):
	      done = new_folder_from_dp (dp, f);
	      if (done)
		retval = TRUE;
	      break;
	    case CWC (2):
	      done = TRUE;
	      break;
	    }
	}
      CloseDialog (dp);
      SetPort (gp);
    }
  return retval;
}

PRIVATE void
rep_from_host_reply_block (reply_u *repp,
			   const host_spf_reply_block *host_reply)
{
}

PRIVATE boolean_t
is_normal_dlgid (getorput_t getorput, INTEGER dig)
{
  return getorput == get
    ? dig == getDlgID || dig == sfGetDialogID
    : dig == putDlgID || dig == sfPutDialogID;
}

PUBLIC void spfcommon(Point p, StringPtr prompt, StringPtr name,
		      dialog_hook_u dh, reply_u rep, INTEGER dig, filter_u fp,
		      file_filter_u filef, INTEGER numt, SFTypeList tl,
		      getorput_t getorput, sf_flavor_t flavor,
		      Ptr activeList, ActivateYDProcPtr activateproc,
		      UNIV Ptr yourdatap)
{
  boolean_t reply_valid;
  TRAPBEGIN();

  reply_valid = FALSE;
  if (is_normal_dlgid (getorput, dig) && host_has_spfcommon ()) {
    host_spf_reply_block reply;
    char *c_prompt;
    const char *local_name = "currently ignored";

    c_prompt = cstring_from_str255 (prompt);
#if 0
    local_name = ...;
#else
#warning "TODO: support native file pickers"
#endif
    reply_valid = host_spfcommon (&reply, c_prompt, local_name, &fp, &filef,
				  numt, tl, getorput, flavor, activeList,
				  &activateproc, yourdatap);
    free (c_prompt);
    if (reply_valid)
      rep_from_host_reply_block (&rep, &reply);
  }
  if (!reply_valid) {
    HIDDEN_Handle h;
    DialogPtr dp;
    INTEGER ihit, i;
    int done, sav;
    Rect r, scrollrect;
    HIDDEN_Handle pnhand, ejhand, drhand, sahand;
    OSErr err;
    ControlHandle scrollh;
    fltype f;
    GrafPtr gp;
    INTEGER openorsave, promptitem, nmlistitem, diskname, ejectitem, driveitem;
    BOOLEAN transform;
    EventRecord evt;
    ParamBlockRec pbr;
    CInfoPBRec hpb;
    ControlHandle new_folder_button;

    done = FALSE;
    memset (&f, 0, sizeof f);
    f.magicfp = fp;
    f.mydata = yourdatap;
    f.flavor = flavor;
    f.flreplyp = rep;

    if (p.h < 2 || p.v < BigEndianValue(MBarHeight) + 7)
      get_starting_point (&p);

    *SF_GOOD_XP(&f) = CBC (FALSE);
    if (f.flavor == original_sf)
      f.flreplyp.oreplyp->version = CBC (0);
    else
      {
	f.flreplyp.nreplyp->sfIsFolder = 0;
	f.flreplyp.nreplyp->sfIsVolume = 0;
      }
    *SF_FTYPE_XP(&f) = CLC (0);
    
    new_folder_button = NULL;
    if ( getorput == put) {
	str31assign (SF_NAME (&f), name);
	if (f.flavor == original_sf)
	  {
	    openorsave = putSave;
	    promptitem = putPrompt;
	    nmlistitem = putNmList;
	    diskname   = putDiskName;
	    ejectitem  = putEject;
	    driveitem  = putDrive;
	    f.fl_cancel_item = putCancel;
	  }
	else
	  {
	    openorsave = sfItemOpenButton;
	    promptitem = sfItemPromptStaticText;
	    nmlistitem = sfItemFileListUser;
	    diskname   = sfItemVolumeUser;
	    ejectitem  = sfItemEjectButton;
	    driveitem  = sfItemDesktopButton;
	    f.fl_cancel_item = sfItemCancelButton;
	  }
    } else {
      if (f.flavor == original_sf)
	{
	  openorsave = getOpen;
	  promptitem = 10;	/* according to whom? bill? */
	  nmlistitem = getNmList;
	  diskname   = getDiskName;
	  ejectitem  = getEject;
	  driveitem  = getDrive;
	  f.fl_cancel_item = getCancel;
	}
      else
	{
	  openorsave = sfItemOpenButton;
	  promptitem = sfItemPromptStaticText;
	  nmlistitem = sfItemFileListUser;
	  diskname   = sfItemVolumeUser;
	  ejectitem  = sfItemEjectButton;
	  driveitem  = sfItemDesktopButton;
	  f.fl_cancel_item = sfItemCancelButton;
	}
    }
    gp = thePort;
    dp = GetNewDialog(dig, (Ptr)0, (WindowPtr)-1);
    bumpsavedisk(dp, FALSE);
    SetPort(dp);
    GetDItem(dp, openorsave, &i, &sahand, &r);
    sahand.p = MR(sahand.p);
    if (getorput == put && SF_NAME (&f)[0])
      {
	boolean_t writable;
	
	getdiskname (NULL, &writable);
	sav = TRUE;
      }
    else
      {
	sav = FALSE;
	HiliteControl((ControlHandle) sahand.p, 255);
      }
    GetDItem(dp, ejectitem, &i, &ejhand, &r);
    ejhand.p = MR(ejhand.p);
    HiliteControl((ControlHandle) ejhand.p, 255);
    GetDItem(dp, driveitem, &i, &drhand, &r);
    drhand.p = MR(drhand.p);
    adjustdrivebutton(dp);

    if (getorput == put)
      {
	GetDItem(dp, promptitem, &i, &h, &r);
	h.p = MR(h.p);
	SetIText(h.p, prompt ? prompt : (StringPtr) "");
      }

    GetDItem(dp, nmlistitem, &i, &h, &scrollrect);
    h.p = MR(h.p);

    if (getorput == put) {
        INTEGER putname;

	if (f.flavor == original_sf)
	  {
	    putname = putName;
	    transform = BigEndianValue(scrollrect.right) - BigEndianValue(scrollrect.left) == 1;
	  }
	else
	  {
	    putname = sfItemFileNameTextEdit;
	    transform = FALSE;
	    new_folder_button = create_new_folder_button (dp);
	  }

	GetDItem(dp, putname, &i, &pnhand, &r);
	pnhand.p = MR(pnhand.p);
	SetIText(pnhand.p, SF_NAME (&f));
	SelIText((DialogPtr) dp, putname, 0, 32767);
    } else {
      if (f.flavor == original_sf)
	{
	  GetDItem(dp, getScroll, &i, &h, &r);
	  h.p = MR(h.p);
	  transform = BigEndianValue(r.right) - BigEndianValue(r.left) == 16;
	  GetDItem(dp, getDotted, &i, &h, &r);
	  h.p = MR(h.p);
	  SetDItem(dp, getDotted, userItem, (Handle) P_ROMlib_filebox, &r);
	}
      else
	transform = FALSE;
    }

    if ( transform )
	transformsfpdialog(dp, &p, &scrollrect, getorput == get);

    SetDItem(dp, nmlistitem, userItem, (Handle) P_ROMlib_filebox, &scrollrect);

    GetDItem(dp, diskname, &i, &h, &r);
    h.p = MR(h.p);
    SetDItem(dp, diskname, userItem, (Handle) P_ROMlib_filebox, &r);

    r.left   = BigEndianValue(BigEndianValue(scrollrect.left) + 1);
    r.right  = BigEndianValue(BigEndianValue(scrollrect.right) - 16);
    r.top    = BigEndianValue(BigEndianValue(scrollrect.top) + 1);
    r.bottom = BigEndianValue(BigEndianValue(scrollrect.bottom) - 1);
    scrollrect.left = BigEndianValue(BigEndianValue(scrollrect.right) - 16);
    scrollh = NewControl((WindowPtr) dp, &scrollrect, (StringPtr) "", TRUE,
						   0, 0, 0, scrollBarProc, 0L);
    flinit(&f, &r, scrollh);
    f.flfilef = filef;
    f.flnumt = numt;
    f.fltl = tl;
    f.flch = (ControlHandle) sahand.p;
    f.flgraynondirs = getorput == get ? 0 : GRAYBIT;

    if (getorput == get) {
	if (f.flnmfil > 0) {
	    StringPtr ip;

	    ip = (StringPtr) MR(*f.flstrs) + MR(*f.flinfo)[0].floffs;
	    str31assign (SF_NAME (&f), ip);
	} else
	    (SF_NAME (&f))[0] = 0;
    }
    SetWRefCon((WindowPtr) dp, (LONGINT)(long)US_TO_SYN68K(&f));
    if (BigEndianValue(dp->portRect.bottom) + p.v  + 7 > BigEndianValue(screenBitsX.bounds.bottom))
	p.v = BigEndianValue(screenBitsX.bounds.bottom) - BigEndianValue(dp->portRect.bottom) - 7;
    if (p.v < BigEndianValue(MBarHeight) + 7)
	p.v = BigEndianValue(MBarHeight) + 7;
    MoveWindow((WindowPtr) dp, p.h, p.v, FALSE);

    ihit = -1;
    if (dh.odh)
      ihit = ROMlib_CALLDHOOK(&f, -1, dp, dh);	/* the mac does this */

    flfill(&f); /* moved to after dhook call of -1 suggested by
		   Wieslaw Kuzmicz */

    ShowWindow((WindowPtr) dp);
    SelectWindow((WindowPtr) dp);
    while (!done) {
	ModalDialog((ProcPtr) P_ROMlib_stdffilt, &ihit);
	ihit = BigEndianValue(ihit);
	if (getorput == put)
	    GetIText(pnhand.p, SF_NAME (&f));
	if (dh.odh)
	  ihit = ROMlib_CALLDHOOK(&f, ihit, dp, dh);
	if (ihit == openorsave) {
	    makeworking (&f);
	    if (getorput == get) {
	        if (*SF_FTYPE_XP(&f))    /* will never happen unless someone */
		    (SF_NAME (&f)) [0] = 0;/* has a tricky filterproc */
		else
		    *SF_FTYPE_XP(&f) = gettypeX(SF_NAME (&f), SF_VREFNUM (&f),
						SF_DIRID (&f));
		done = TRUE;
		*SF_GOOD_XP(&f) = CBC (TRUE);
	    } else {
		GetIText(pnhand.p, SF_NAME (&f));
		hpb.dirInfo.ioCompletion = 0;
		hpb.dirInfo.ioNamePtr    = ((StringPtr)
					    RM((char *) SF_NAME (&f)));
		hpb.dirInfo.ioVRefNum    = SF_VREFNUM_X (&f);
		hpb.dirInfo.ioFDirIndex  = CWC (0);
		hpb.dirInfo.ioDrDirID    = 0;
		err = PBGetCatInfo(&hpb, FALSE);
		switch (err) {
		case noErr:
		    ParamText(SF_NAME (&f), (StringPtr)0, (StringPtr)0,
								 (StringPtr)0);
		    if (movealert(-3996) == 1)  /* overwrite ... */
			break;
		    *SF_FTYPE_XP (&f) = hpb.hFileInfo.ioFlFndrInfo.fdType;
		    /* FALL THROUGH */
		default:
		    if (err != noErr)
		      warning_unexpected ("err = %d", err);
		    /* FALL THROUGH */
		case fnfErr:
		    done = TRUE;
		    *SF_GOOD_XP(&f) = CBC (TRUE); /* great.  That's a take */
		    break;
		case bdNamErr:
		case nsvErr:
		case paramErr:
		    movealert(-3994);   /* disknotfound */
		    break;
		}
	    }
	} else if ((ihit == f.fl_cancel_item) ||
		   (ihit == putCancel)) { /* MYM 6.0 suggests that putCancel
					     cancels a get, too */
	    done = TRUE;
	} else if (ihit == ejectitem) {
	    doeject(dp);
	    ihit = FAKEREDRAW;
	} else if (ihit == driveitem) {
	    bumpsavedisk(dp, TRUE);
	    ihit = FAKEREDRAW;
	} else if (ihit == diskname) {
	    if (moveuponedir(dp))
		ihit = FAKEREDRAW;
	} else if (ihit == FAKECURDIR) {
	    if (trackdirs((DialogPeek) dp))
		ihit = FAKEREDRAW;
	} else if (ihit == FAKEOPENDIR) {
	    CurDirStore = *SF_FTYPE_XP(&f);
	    unixcd(&f);
	    ihit = FAKEREDRAW;
	} else if (ihit == sfItemNewFolderUser)
	  {
	    if (do_new_folder (&f))
	      {
		CurDirStore = *SF_FTYPE_XP(&f);
		unixcd (&f);
		ihit = FAKEREDRAW;
	      }
	  }
	if (getorput == put) {
	    Str255 file_name;
	    boolean_t writable;

	    GetIText(pnhand.p, file_name);
	    str31assign (SF_NAME (&f), file_name);
	    getdiskname (NULL, &writable);
	    if ((SF_NAME (&f))[0] && writable && !sav) {
		HiliteControl((ControlHandle) sahand.p, 0);
		sav = TRUE;
	    } else if ((!(SF_NAME (&f)[0]) || !writable) && sav) {
		HiliteControl((ControlHandle) sahand.p, 255);
		sav = FALSE;
	    }
	}
	if (WaitNextEvent(diskMask, &evt, 4, 0) &&
					(evt.message & CLC(0xFFFF0000)) == 0) {
	    pbr.volumeParam.ioNamePtr = 0;
	    pbr.volumeParam.ioVolIndex = 0;
	    pbr.volumeParam.ioVRefNum = BigEndianValue(BigEndianValue(evt.message) & 0xFFFF);
	    err = PBGetVInfo(&pbr, FALSE);
	    gui_assert(err == noErr);
	    if (err == noErr) {
		adjustdrivebutton(dp);
		SFSaveDisk = BigEndianValue(-BigEndianValue(pbr.volumeParam.ioVRefNum));
		CurDirStore = CLC(2);
		ihit = FAKEREDRAW;
	    }
	}
	if (ihit == FAKEREDRAW)
	    realcd((DialogPeek) dp, BigEndianValue(CurDirStore));
    }
    if (f.flavor != original_sf && dh.odh)
      ihit = ROMlib_CALLDHOOK(&f, -2, dp, dh);	/* the mac does this */
    flfinit(&f);
    destroy_new_folder_button (dp, new_folder_button);
    CloseDialog((DialogPtr) dp);
    makeworking (&f);
    SetPort(gp);
  }
  TRAPEND();
}

P7(PUBLIC pascal trap, void, SFPPutFile, Point, p, StringPtr, prompt,
       StringPtr, name, ProcPtr, dh, SFReply *, rep, INTEGER, dig, ProcPtr, fp)
{
  dialog_hook_u dhu;
  reply_u repu;
  filter_u filteru;
  file_filter_u zero_file_filter = { 0 };

  dhu.odh = dh;
  repu.oreplyp = rep;
  filteru.ofilterp = fp;

  spfcommon(p, prompt, name, dhu, repu, dig, filteru, zero_file_filter, -1,
	    (OSType *) 0, put, original_sf, 0, 0, 0);
}

P5(PUBLIC pascal trap, void, SFPutFile, Point, p, StringPtr, prompt,
				  StringPtr, name, ProcPtr, dh, SFReply *, rep)
{
    SFPPutFile(p, prompt, name, dh, rep, putDlgID, (ProcPtr) 0);
}

P9(PUBLIC pascal trap, void, SFPGetFile, Point, p, StringPtr, prompt,
		ProcPtr, filef, INTEGER, numt, SFTypeList, tl, ProcPtr, dh,
				     SFReply *, rep, INTEGER, dig, ProcPtr, fp)
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

P7(PUBLIC pascal trap, void, SFGetFile, Point, p, StringPtr, prompt,
    ProcPtr, filef, INTEGER, numt, SFTypeList, tl, ProcPtr, dh, SFReply *, rep)
{
    SFPGetFile(p, prompt, filef, numt, tl, dh, rep, getDlgID, (ProcPtr) 0);
}

P10(PUBLIC pascal trap, void, CustomPutFile, Str255, prompt,
    Str255, defaultName, StandardFileReply *, replyp, INTEGER, dlgid,
    Point, where, DlgHookYDProcPtr, dlghook, ModalFilterYDProcPtr, filterproc,
    Ptr, activeList, ActivateYDProcPtr, activateproc, UNIV Ptr, yourdatap)
{
  dialog_hook_u dhu;
  reply_u repu;
  filter_u filteru;
  file_filter_u file_filteru;

  dhu.cdh = dlghook;
  repu.nreplyp = replyp;
  file_filteru.cflfilef = 0;
  filteru.cfilterp = filterproc;

  if (dlgid == 0)
    dlgid = sfPutDialogID;


  spfcommon (where, prompt, defaultName, dhu, repu, dlgid, filteru,
	     file_filteru, -1, 0, put, new_custom_sf, activeList,
	     activateproc, yourdatap);
}

P11(PUBLIC pascal trap, void, CustomGetFile, FileFilterYDProcPtr, filefilter,
    INTEGER, numtypes, SFTypeList, typelist, StandardFileReply *, replyp,
    INTEGER, dlgid, Point, where, DlgHookYDProcPtr, dlghook,
    ModalFilterYDProcPtr, filterproc, Ptr, activeList,
    ActivateYDProcPtr, activateproc, UNIV Ptr, yourdatap)
{
  dialog_hook_u dhu;
  reply_u repu;
  filter_u filteru;
  file_filter_u file_filteru;
  
  dhu.cdh = dlghook;
  repu.nreplyp = replyp;
  filteru.cfilterp = filterproc;
  file_filteru.cflfilef = filefilter;

  if (dlgid == 0)
    dlgid = sfGetDialogID;

  spfcommon (where, (StringPtr) "", (StringPtr) "", dhu, repu, dlgid,
	     filteru, file_filteru, numtypes,
	     typelist, get, new_custom_sf, activeList, activateproc,
	     yourdatap);
}

P4(PUBLIC pascal trap, void, StandardGetFile, ProcPtr, filef, INTEGER, numt,
   SFTypeList, tl, StandardFileReply *, replyp)
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
  spfcommon (p, (StringPtr) "", (StringPtr) "", dhu, repu, sfGetDialogID,
	     filteru, file_filteru, numt, tl, get, new_sf, 0, 0, 0);
}

P3(PUBLIC pascal trap, void, StandardPutFile, Str255, prompt,
   Str255, defaultname, StandardFileReply *, replyp)
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
  spfcommon (p, prompt, defaultname, dhu, repu, sfPutDialogID, filteru,
	     file_filteru, -1, 0, put, new_sf, 0, 0, 0);
}

/* Copyright 1988 - 1999 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_syserr[] =
	    "$Id: syserr.c 63 2004-12-24 18:19:43Z ctm $";
#endif

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
#include "rsys/screen.h"
#include "rsys/next.h"
#include "rsys/segment.h"
#include "rsys/flags.h"
#include "rsys/version.h"
#include "rsys/license.h"
#include "rsys/vdriver.h"
#include "rsys/next.h"
#include "rsys/osevent.h"
#include "rsys/options.h"

PRIVATE void mydolicense( void )
{
    ROMlib_dolicense = old_license;
}

PRIVATE struct {
    INTEGER	count	PACKED;

    INTEGER	id1	PACKED,
		len1	PACKED,
		prim1	PACKED,
		sec1	PACKED,
		icon1	PACKED,
		proc1	PACKED,
		but1	PACKED;

    INTEGER	id2	PACKED,
    		len2	PACKED;
    Point	point2	PACKED;
    unsigned char str2[52]	PACKED;

    INTEGER	id3	PACKED,
		len3	PACKED;

    Rect	rect3	PACKED;
    char	icon3[32][4]	PACKED;

    INTEGER	id4	PACKED,
		len4	PACKED;

    Point	point4	PACKED;
    char	str4[46]	PACKED;

    INTEGER	id5	PACKED,
   	        len5	PACKED,
		num5	PACKED,
		str5	PACKED;
    Rect	rect5	PACKED;
    INTEGER	proc5	PACKED,
    		str5b	PACKED;
    Rect	rect5b	PACKED;
    INTEGER	proc5b	PACKED;

    INTEGER	id6	PACKED,
		len6	PACKED;

    char	str6[4]	PACKED;

    INTEGER	id7	PACKED,
		len7	PACKED;
    char	str7[6]	PACKED;

    INTEGER	id8	PACKED,
		len8	PACKED;
    void	(*func8)(void)	PACKED;

} myalerttab = {
    CWC(8),

#define WELCOME_CODE	0x28
    CWC(WELCOME_CODE),	/* 1. normally "Welcome to M*cintosh", now copyright stuff */
    CWC(10),
    CWC(50),
    CWC(150),
    CWC(31),
    CWC(0),
    CWC(151),

    CWC(50),	/* 2. primary text */
    CWC(56),
    { CWC(108), CWC(105) },
    "\251 Abacus Research and Development, Inc.  1986-1999",

    CWC(31),	/* 3. ARDI icon */
    CWC(136),
    { CWC(99), CWC(56), CWC(131), CWC(88) },
    {
	{ 0xff,0xff,0xff,0xff },
	{ 0x88,0x88,0x88,0x89 },
	{ 0x9d,0xdd,0xdd,0xdd },
	{ 0x88,0x88,0x88,0x89 },
	{ 0x9d,0xdd,0xdd,0xdd },
	{ 0x88,0x88,0x88,0x89 },
	{ 0xff,0xff,0xff,0xff },
	{ 0x88,0x88,0x88,0x89 },
	{ 0x88,0x88,0x88,0x89 },
	{ 0x88,0x80,0x80,0x89 },
	{ 0x88,0xbc,0xbc,0x89 },
	{ 0x80,0xa2,0x92,0x81 },
	{ 0x88,0xa2,0x92,0x9d },
	{ 0x94,0xa2,0x92,0x89 },
	{ 0xa2,0xbc,0x92,0x89 },
	{ 0xa2,0xa8,0x92,0x89 },
	{ 0xa2,0xa4,0x92,0x89 },
	{ 0xbe,0xa2,0x92,0x89 },
	{ 0xa2,0xa2,0x92,0x89 },
	{ 0xa2,0xa2,0x92,0x89 },
	{ 0xa2,0xa2,0xbc,0x9d },
	{ 0x80,0x80,0x80,0x81 },
	{ 0x88,0x88,0x88,0x89 },
	{ 0x88,0x88,0x88,0x89 },
	{ 0xff,0xff,0xff,0xff }
    },

    CWC(150),	/* 4. secondary text */
    CWC(50),
    { CWC(122), CWC(125) },
    "Please click on \"Info\" for more information\0\0",

    CWC(151),	/* 5. the buttons */
    CWC(26),
    CWC(2),
    CWC(153),
    { CWC(150), CWC(50), CWC(170), CWC(100) }, /* OK rect */
    CWC(0),
    CWC(155),
    { CWC(150), CWC(130), CWC(170), CWC(190) }, /* Abort rect */
    CWC(156),

    CWC(153),	/* 6. OK button */
    CWC(4),
    "OK\0",

    CWC(155),	/* 7. Info button */
    CWC(6),
    "Info\0",

    CWC(156),	/* 8. Info "procedure" */
    CWC(4),
#if !defined(LITTLEENDIAN)
    (void (*)()) mydolicense,
#else
    0,	/* dolicense will be filled in later */
#endif

};

char syserr_msg[256];

struct adef {
    INTEGER id			PACKED,
	    alen		PACKED,
	    primetextid		PACKED,
	    secondtextid	PACKED,
	    iconid		PACKED,
	    procid		PACKED,
	    buttonid		PACKED;
};

struct tdef {
    INTEGER id			PACKED,
	    alen		PACKED;
    Point loc			PACKED;
    char text[1]		PACKED;	/* at least one NUL byte */
};

struct idef {
    INTEGER id		PACKED,
	    alen	PACKED;
    Rect loc		PACKED;
    LONGINT ike[32]	PACKED;
};

struct pdef {
    INTEGER id		PACKED,
	    alen	PACKED;
    void (*proc)()	PACKED;
			/* NOTE:  THIS IS NOT THE WAY IT WORKS IN THE M*C */
};

struct bdef {
    INTEGER id			PACKED,
	    alen		PACKED,
	    nbut		PACKED;
    struct but {
	INTEGER butstrid	PACKED;
	Rect butloc		PACKED;
	INTEGER butprocid	PACKED;
    } buts[1]			PACKED;
};

struct sdef {
    INTEGER id		PACKED,
	    alen	PACKED;
    char text[1]	PACKED;
};

A1(PRIVATE, INTEGER *, findid, INTEGER, id)
{
    int i;
    INTEGER *ip;

    for (i = CW(*(INTEGER *) MR(DSAlertTab)),
					   ip = (INTEGER *) MR(DSAlertTab) + 1;
						        i > 0 && CW(*ip) != id;
	 --i, ip = (INTEGER *) ((char *) ip + CW(ip[1]) + 2 * sizeof(INTEGER)))
    ;
    return i > 0 ? ip : (INTEGER *) 0;
}

A3(PRIVATE, void, drawtextstring, INTEGER, id, INTEGER, offsetx,
   INTEGER, offsety)
{
    struct tdef *tp;

    if (id && (tp = (struct tdef *) findid(id))) {
	MoveTo(CW(tp->loc.h) + offsetx, CW(tp->loc.v) + offsety);
	DrawText_c_string (tp->text);
    }
}

A3 (PRIVATE, void, drawicon, INTEGER, id,
    INTEGER, offsetx, INTEGER, offsety)
{
    struct idef *ip;
    BitMap bm;
    Rect old_loc;

    ip = (struct idef *) findid (id);
    if (id && ip)
      {
        bm.baseAddr = RM ((Ptr) ip->ike);
	bm.rowBytes = CWC (4);
	bm.bounds.left = bm.bounds.top = CWC (0);
	bm.bounds.right = bm.bounds.bottom = CWC (32);
	old_loc = ip->loc;
	C_OffsetRect (&ip->loc, offsetx, offsety);
	CopyBits (&bm, PORT_BITS_FOR_COPY (thePort),
		  &bm.bounds, &ip->loc, srcCopy, NULL);
	ip->loc = old_loc;
    }
}

A4(PRIVATE, void, dobuttons, INTEGER, id, INTEGER, offsetx,
   INTEGER, offsety, BOOLEAN, demo_button_p)
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
#if defined (BILLBUTTONS)
    INTEGER h, v;
#endif /* BILLBUTTONS */
    char *textp;

    if ((bp = (struct bdef *) findid(id))) {
	for (i = 0; i < Cx(bp->nbut); i++) {

	    /* Offset buttons; this hack is to center the splash screen
	     * on non-512x342 root windows...yuck!
	     */

	    C_OffsetRect (&bp->buts[i].butloc, offsetx, offsety);
	    if ((sp = (struct sdef *)findid(CW(bp->buts[i].butstrid)))) {
  	        if (demo_button_p && sp->text[0] == 'O' && sp->text[1] == 'K')
		    textp = "Demo";
	        else
		    textp = sp->text;
		tcnt = strlen(textp);
		twid = TextWidth((Ptr) textp, 0, tcnt);
		MoveTo((CW(bp->buts[i].butloc.left)  +
		        CW(bp->buts[i].butloc.right) - twid) / 2,
		       (CW(bp->buts[i].butloc.top)   +
			CW(bp->buts[i].butloc.bottom)) / 2 + 4);
		DrawText((Ptr) textp, 0, tcnt);
	    }
#if defined (BILLBUTTONS)
	    h = CW(bp->buts[i].butloc.right) - CW(bp->buts[i].butloc.left);
	    v = (CW(bp->buts[i].butloc.bottom) - CW(bp->buts[i].butloc.top))/2;
	    if (h > v)
		h = v;
	    if (!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
	      FrameRoundRect(&bp->buts[i].butloc, h, v);
	    else
	      FrameRect(&bp->buts[i].butloc);
#else /* BILLBUTTONS */
	    if (!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
	      FrameRoundRect(&bp->buts[i].butloc, 10, 10);
	    else
	      FrameRect(&bp->buts[i].butloc);
#endif /* BILLBUTTONS */
	  }

	for (done = 0; !done;) {
	    C_GetNextEvent(mDownMask|keyDownMask, &evt);
	    if (evt.what == CWC(mouseDown) || evt.what == CWC(keyDown)) {
		p.h = CW(evt.where.h);
		p.v = CW(evt.where.v);
		for (i = 0; !done && i < CW(bp->nbut); i++) {
		    if (PtInRect(p, &bp->buts[i].butloc) ||
			((evt.what == CWC(keyDown)) &&
			 (((CL(evt.message) & charCodeMask) == '\r') ||
			  ((CL(evt.message) & charCodeMask) == NUMPAD_ENTER)))
			) {
			if ((pp = (struct pdef *)
					    findid(CW(bp->buts[i].butprocid))))
			    /* NOTE:  we will have to do a better
				      job here sometime */
			    (*(void (*)(void))MR(pp->proc))();
			done = 1;
		    }
		}
		if (!done)
		    SysBeep(1);
	    }
	}
	if (evt.what == CWC(mouseDown))
	    while (!C_GetNextEvent(mUpMask, &evt))
		;

	/* Move all buttons back. */
	for (i = 0; i < Cx(bp->nbut); i++)
	  C_OffsetRect (&bp->buts[i].butloc, -offsetx, -offsety);
      }
}

#define DEMO_MODE_VERSION "Time Limited Demo Mode ("
#define REGISTERED_VERSION "Fully Enabled Version, Do NOT Redistribute ("

#if defined (EXPERIMENTAL)
#define EXPERIMENTAL_STRING " -- experimental)"
#else
#define EXPERIMENTAL_STRING ")"
#endif

PRIVATE void
DrawVersion (char *str)
{
  DrawText_c_string (str);
  DrawText_c_string ((char *) ROMlib_executor_version);
  DrawText_c_string (EXPERIMENTAL_STRING);
}

/*
 * NOTE: The version of SysError below will only handle natively compiled
 *	 code.  When we want to be able to run arbitrary code we'll need
 *	 to call syn68k appropriately.
 */

P1(PUBLIC pascal, void, SysError, short, errorcode)
{
    GrafPort alertport;
    Region viscliprgn;
    HIDDEN_RgnPtr rp;
    Rect r;
    struct adef *ap;
    char quickbytes[grafSize];
    INTEGER offsetx, offsety;
    Rect main_gd_rect;

#if defined (BINCOMPAT)
    LONGINT tmpa5;
#endif /* BINCOMPAT */

    main_gd_rect = PIXMAP_BOUNDS (GD_PMAP (MR (MainDevice)));
    
#if defined(LITTLEENDIAN)
    myalerttab.func8 =
      (void (*)(void)) (long) CL((LONGINT) (long) mydolicense);
#endif

    if (!DSAlertTab) {
#if defined (CLIFF_CENTERING_ALGORITHM)
	DSAlertTab = CL((Ptr) &myalerttab);
	DSAlertRect.top    = CWC(64);
	DSAlertRect.left   = CWC(32);
	DSAlertRect.bottom = CWC(190);
	DSAlertRect.right  = CWC(480);
#else
	INTEGER screen_width = CW (main_gd_rect.right);
	INTEGER screen_height = CW (main_gd_rect.bottom);

	DSAlertTab = RM((Ptr) &myalerttab);
	DSAlertRect.top    = CW((screen_height - 126) / 3);
	DSAlertRect.left   = CW((screen_width - 448) / 2);
	DSAlertRect.bottom = CW(CW(DSAlertRect.top) + 126);
	DSAlertRect.right  = CW(CW(DSAlertRect.left) + 448);
#endif

	offsetx = CW (DSAlertRect.left) - 32;
	offsety = CW (DSAlertRect.top) - 64;
    }
    else {
      offsetx = offsety = 0;
    }

    /* IM-362 */
    /* 1. Save registers and Stack Pointer */
    /*	  NOT DONE YET... signal handlers sort of do that anyway */

    /* 2. Store errorcode in DSErrCode */
    DSErrCode = CW(errorcode);

    /* 3. If no Cx(DSAlertTab), bitch */
    if (!DSAlertTab) {
	write(2, "This machine thinks its a sadmac\n",
	  sizeof("This machine thinks its a sadmac\n")-1);
	exit(255);
    }

    /* 4. Allocate and re-initialize QuickDraw */
#if defined (BINCOMPAT)
    a5 = (LONGINT) (long) US_TO_SYN68K (&tmpa5);
    CurrentA5 = (Ptr) (long) CL(a5);
#endif /* BINCOMPAT */
    InitGraf((Ptr) quickbytes + sizeof(quickbytes) - 4);
    ROMlib_initport(&alertport);
    SetPort(&alertport);
    InitCursor();
    rp.p = RM(&viscliprgn);
    alertport.visRgn = alertport.clipRgn = RM(&rp);
    viscliprgn.rgnSize = CWC(10);
#if 0 && !defined(MSDOS)
    viscliprgn.rgnBBox = DSAlertRect;
#else
    viscliprgn.rgnBBox = main_gd_rect;
#endif

    /* 5, 6. Draw alert box if the errorcode is >= 0 */
    TRAPBEGIN();
    if (
#if defined (DISPLAY_SPLASH_INFO_BOX)
	!ROMlib_nosplash
#else
	0
#endif
	) {
	if (errorcode < 0)
	    errorcode = -errorcode;
	else {
	    r = DSAlertRect;
	    FillRect(&r, white);
#if defined (OLDSTYLEALERT)
	    r.right = CW(CW(r.right) - (2));
	    r.bottom = CW(CW(r.bottom) - (2));
	    FrameRect(&r);
	    PenSize(2, 2);
	    MoveTo(CW(r.left)+2, CW(r.bottom));
	    LineTo(CW(r.right), CW(r.bottom));
	    LineTo(CW(r.right), CW(r.top)+2);
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

	ap = (struct adef *) findid(errorcode);
	if (!ap)
	    ap = (struct adef *) ((INTEGER *) MR(DSAlertTab) + 1);
	
	/* 7. text strings */
	drawtextstring(CW(ap->primetextid), offsetx, offsety);
	drawtextstring(CW(ap->secondtextid), offsetx, offsety);

	/* 8. icon */
	drawicon(CW(ap->iconid), offsetx, offsety);

	/* 9. TODO: figure out what to do with the proc ... */

#define NTK_LINE_1	"Distributed in Asia by NTK, Inc."
#define NTK_LINE_2	"+82 2 551 3505 FAX"
#define NTK_LINE_3	"+82 2 551 8425 Phone"

	if (errorcode == WELCOME_CODE) {
#if 0
	    MoveTo((CW(DSAlertRect.left) + CW(DSAlertRect.right)) / 2,
						  CW(DSAlertRect.bottom) - 48);
	    DrawText_c_string (NTK_LINE_1);
	    MoveTo((CW(DSAlertRect.left) + CW(DSAlertRect.right)) / 2,
						  CW(DSAlertRect.bottom) - 32);
	    DrawText_c_string (NTK_LINE_2);
	    MoveTo((CW(DSAlertRect.left) + CW(DSAlertRect.right)) / 2,
						  CW(DSAlertRect.bottom) - 16);
	    DrawText_c_string (NTK_LINE_3);
#endif

	    MoveTo (CW(main_gd_rect.left) + 5,
		    CW(main_gd_rect.bottom) - 17);
#if defined (USE_VGAVDRIVER)
	    TextMode(srcBic);
#endif

	    if (ROMlib_info.serialnumber)
	      {
		Str255 s;
		
		DrawVersion (REGISTERED_VERSION);
		MoveTo(CW(main_gd_rect.left) + 5,
		       CW(main_gd_rect.bottom) - 33);
		DrawText_c_string ("SN:");
		NumToString(ROMlib_info.serialnumber, s);
		DrawString(s);
		DrawText_c_string ("  ");
		DrawText_c_string (ROMlib_info.name);
		DrawText_c_string (" --- ");
		DrawText_c_string (ROMlib_info.organization);
	      }
	    else
	      DrawVersion (DEMO_MODE_VERSION);
	    TextMode(srcOr);
	}

	/* 10, 11, 12, 13. check for non-zero button id */
/* #warning We blow off ResumeProc until we can properly handle it */
	if (ap->buttonid)
	    dobuttons(/* CL(ResumeProc) ? Cx(ap->buttonid) + 1 : */ Cx(ap->buttonid),
		      offsetx, offsety,
		      !ROMlib_info.serialnumber && errorcode == WELCOME_CODE);
      }

    TRAPEND();
}

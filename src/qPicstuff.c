/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qPicstuff[] =
	    "$Id: qPicstuff.c 87 2005-05-25 01:57:33Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "MemoryMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "OSUtil.h"
#include "ToolboxUtil.h"
#include "FontMgr.h"

#include "rsys/toolutil.h"
#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/mman.h"
#include "rsys/xdata.h"
#include "rsys/tempalloc.h"
#include "rsys/print.h"
#include "rsys/executor.h"

/*
 * Hooray for static variables.  We have to do something like this because
 * the StdGetPic routine doesn't actually supply a source byte pointer.
 * We used to be nice and re-entrant, but Collate likes to patch out StdGetPic
 * with a routine that uses PBRead() to supply values, and our old trick of
 * requesting one chunk that is a  picSize long doesn't work, because the
 * picture Collate wants to print contains more than 32767 characters.
 *
 * We could still make *our* implementation re-entrant, but it's clear that
 * Apple's isn't.
 */

PRIVATE unsigned char *nextbytep;
PRIVATE pascal void (*procp)(Ptr, INTEGER);

typedef void (*pfv)();

typedef struct {
    pfv func;
    LONGINT argcode;
} wps;

/*
 * The top four bits that are used to describe how an opcode behaves are an
 * encoded form of which of the operands (described in the remaining 28 bits)
 * need to be scaled.
 *
 * Two bits are used for each operand.  The msb is set only when the operand
 * in question requies scaling only as a v quantity (i.e. if it is not set
 * then the operand requires scaling either in both h and v or just h.  Which
 * it is depends on the type of the operand (e.g. rects require scaling of
 * both components).  The lower bit specifies whether or not to do any scaling.
 *
 * WoRDs and BYTes are taken to be relative quantities while other types are
 * assumed to be absolute coordinates (this effects exactly what scaling is:
 * relative quantities are scaled but not offset).
 *
 * Note the actual top four bits only specify an index into the array 
 * scalevalues. The array scalevalues is where the scaling info belongs.
 */

#define NOSCALE		0L
#define SCALE0		1L
#define SCALE01		2L
#define SCALE012V	3L
#define SCALE01V	4L
#define SCALE0V		5L
#define SCALE3		6L
#define SCALE35		7L

#define SCALEIT	1L
#define VONLY	2L

PRIVATE unsigned short scalevalues[16] = {
    0x000, /* 00000000 0000 0000 NOSCALE   */
    0x001, /* 00000000 0000 0001 SCALE0    */
    0x005, /* 00000000 0000 0101 SCALE01   */
    0x035, /* 00000000 0011 0101 SCALE012V */
    0x00D, /* 00000000 0000 1101 SCALE01V  */
    0x003, /* 00000000 0000 0011 SCALE0V   */
    0x040, /* 00000000 0100 0000 SCALE3    */
    0x140, /* 00000001 0100 0000 SCALE35   */
           /* the rest are all zeros */
};

#define xxx0()                0
#define xxx1(a)               xxx0()|a
#define xxx2(a,b)             xxx1(a)|(b<<4)
#define xxx3(a,b,c)           xxx2(a,b)|(c<<8)
#define xxx4(a,b,c,d)         xxx3(a,b,c)|(d<<12)
#define xxx5(a,b,c,d,e)       xxx4(a,b,c,d)|(e<<16)
#define xxx6(a,b,c,d,e,f)     xxx5(a,b,c,d,e)|(f<<20)
#define xxx7(a,b,c,d,e,f,g)   xxx6(a,b,c,d,e,f)|(g<<24)

#define yyy1(s,a)               (s<<28)|xxx1(a)
#define yyy2(s,a,b)             (s<<28)|xxx2(a,b)
#define yyy3(s,a,b,c)           (s<<28)|xxx3(a,b,c)
#define yyy4(s,a,b,c,d)         (s<<28)|xxx4(a,b,c,d)
#define yyy5(s,a,b,c,d,e)       (s<<28)|xxx5(a,b,c,d,e)
#define yyy6(s,a,b,c,d,e,f)     (s<<28)|xxx6(a,b,c,d,e,f)
#define yyy7(s,a,b,c,d,e,f,g)   (s<<28)|xxx7(a,b,c,d,e,f,g)

#define OVP 1L  /* 0 bytes: oval point */
#define BYT 2L  /* 1 byte */
#define WRD 3L  /* 2 bytes */
#define LNG 4L  /* 4 bytes */
#define PNT 5L  /* 4 bytes */
#define RCT 6L  /* 8 bytes */
#define PAT 7L  /* 8 bytes */
#define TXT 8L  /* first byte 0..255 tells how many succesive bytes */
#define RGN 9L  /* first word tells how many inclusive bytes */
#define DAT 10L /* first word tells how many exclusive bytes */
#define SAM 11L /* Same, as in FrameSameRect */
#define RGB 12L /* RGB color */
#define PXP 13L /* PixPat */
#define SPL 14L /* Special ... do by hand */
#define PLY 15L	/* polys have to be mapped differently from regions */

#define ARGMASK	0x0FFFFFFF	/* everything except the scale bits */

A0(PRIVATE, void, nop)
{
}

A1(PRIVATE, void, thepat, Pattern *, p)
{
  ROMlib_fill_pat (*p);
}

PRIVATE LONGINT txnumh, txnumv, txdenh, txdenv;
PRIVATE Rect srcpicframe, dstpicframe;
PRIVATE LONGINT picnumh, picnumv, picdenh, picdenv;

PRIVATE Point txtpoint;

/*
 * TODO: reduce is exceedingly inefficient.  Reconsider its use here.
 */

/*
 * NOTE: reduce is used internally and hence doesn't treat its pointer
 *	 arguments as pointers into syn space
 */

PRIVATE void reduce(LONGINT *nump, LONGINT *denp)
{
    LONGINT num, den, max, i;

    num = *nump;
    den = *denp;

    max = MIN(num / 2, den / 2);

    for (i = max; i >= 2; --i) {
	if ((num % i == 0) && (den % i == 0))
	    break;
    }
    if (i > 1) {
	*nump = num / i;
	*denp = den / i;
    }
    if (*nump > *denp) {
	if (*denp && (*nump % *denp == 0)) {
	    *nump = (*nump) / (*denp);
	    *denp = 1;
	}
    } else {
	if (*nump && (*denp % *nump == 0)) {
	    *denp = (*denp) / (*nump);
	    *nump = 1;
	}
    }
}

A2(PRIVATE, void, txratio, Point, num, Point, den)
{
    txnumh = num.h;
    txnumv = num.v;
    txdenh = den.h;
    txdenv = den.v;
    reduce(&txnumh, &txdenh);
    reduce(&txnumv, &txdenv);
}

A2(PRIVATE, void, line, Point, op, Point, np)
{
  PORT_PEN_LOC (thePort).h = CW (op.h);
  PORT_PEN_LOC (thePort).v = CW (op.v);
  CALLLINE(np);
  PORT_PEN_LOC (thePort).h = CW (np.h);
  PORT_PEN_LOC (thePort).v = CW (np.v);
}

A3(PRIVATE, void, shrtline, Point, op, SignedByte, dh, SignedByte, dv)
{
  PORT_PEN_LOC (thePort).h = CW(op.h);
  PORT_PEN_LOC (thePort).v = CW(op.v);
  op.h += dh;
  op.v += dv;
  CALLLINE (op);
  PORT_PEN_LOC (thePort).h = CW(op.h);
  PORT_PEN_LOC (thePort).v = CW(op.v);
}

PRIVATE void setnumerdenom(Point *nump, Point *denp)
{
    LONGINT numerh, numerv, denomh, denomv;

    numerh = txnumh * picnumh;
    numerv = txnumv * picnumv;
    denomh = txdenh * picdenh;
    denomv = txdenv * picdenv;
    reduce(&numerh, &denomh);
    reduce(&numerv, &denomv);
    nump->h = numerh;
    nump->v = numerv;
    denp->h = denomh;
    denp->v = denomv;
}

A3(PRIVATE, void, longtext, Point, pt, StringPtr, s, Point *, pp)
{
    Point save, numer, denom;

    save = PORT_PEN_LOC (thePort);
    pp->h = CW (pt.h);
    pp->v = CW (pt.v);
    PORT_PEN_LOC (thePort).h = CW (pt.h);
    PORT_PEN_LOC (thePort).v = CW (pt.v);
    setnumerdenom (&numer, &denom);
    CALLTEXT ((INTEGER) U(s[0]), (Ptr) (s+1), numer, denom);
    PORT_PEN_LOC (thePort) = save;
}

A3(PRIVATE, void, dhtext, unsigned char, dh, StringPtr, s, Point *, pp)
{
    Point save, numer, denom;

    pp->h = CW(CW(pp->h) + (dh));
    save = PORT_PEN_LOC (thePort);
    PORT_PEN_LOC (thePort) = *pp;
    setnumerdenom (&numer, &denom);
    CALLTEXT((INTEGER) U(s[0]), (Ptr) (s+1), numer, denom);
    PORT_PEN_LOC (thePort) = save;
}

A3(PRIVATE, void, dvtext, unsigned char, dv, StringPtr, s, Point *, pp)
{
  Point save, numer, denom;

  pp->v = CW (CW (pp->v) + (dv));
  save = PORT_PEN_LOC (thePort);
  PORT_PEN_LOC (thePort) = *pp;
  setnumerdenom (&numer, &denom);
  CALLTEXT ((INTEGER)U(s[0]), (Ptr) (s+1), numer, denom);
  PORT_PEN_LOC (thePort) = save;
}

A4(PRIVATE, void, dhdvtext, Byte, dh, Byte, dv,
						    StringPtr, s, Point *, pp)
{
    Point save, numer, denom;

    pp->h = CW(CW(pp->h) + (dh));
    pp->v = CW(CW(pp->v) + (dv));
    save = PORT_PEN_LOC (thePort);
    PORT_PEN_LOC (thePort) = *pp;
    setnumerdenom(&numer, &denom);
    CALLTEXT ((INTEGER)U(s[0]), (Ptr) (s+1), numer, denom);
    PORT_PEN_LOC (thePort) = save;
}

A1(PRIVATE, void, fillrct, Rect *, r)
{
  CALLRECT (fill, r);
}

A3(PRIVATE, void, fillrrct, Rect *, r, INTEGER, ow, INTEGER, oh)
{
  CALLRRECT (fill, r, ow, oh);
}

A1(PRIVATE, void, fillovl, Rect *, r)
{
  CALLOVAL (fill, r);
}

A3(PRIVATE, void, fillarc, Rect *, r, INTEGER, stang, INTEGER, arcang)
{
  CALLARC (fill, r, stang, arcang);
}

A1(PRIVATE, void, fillpoly, PolyHandle, p)
{
  CALLPOLY (fill, p);
}

A1(PRIVATE, void, fillrgn, RgnHandle, r)
{
  CALLRGN (fill, r);
}

PRIVATE RgnHandle saveclip;

A2 (PRIVATE, void, origin, INTEGER, dh, INTEGER, dv)
{
  OffsetRect (&srcpicframe, dh, dv);
  
  txtpoint.h = CW (CW (txtpoint.h) - dh);
  txtpoint.v = CW (CW (txtpoint.v) - dv);
}

A1(PRIVATE, void, pnlochfrac, INTEGER, f)
{
    /* make sure that we scale f, since it's fixed and can't be scaled
       automatically */
}

A1(PRIVATE, void, myreadcment, INTEGER, kind)
{
    C_ReadComment(kind, 0, (Handle) 0);
}

PRIVATE RGBColor saveHiliteRGB;

A0(PRIVATE, void, defhilite)
{
  HiliteRGB = saveHiliteRGB;
}

A0(PRIVATE, void, hilitemode)
{
  HiliteMode &= ~(0x80);
}

A1(PRIVATE, void, fillpixpat, PixPatHandle, ph)
{
  if (CGrafPort_p (thePort))
    {
      HandToHand ((HIDDEN_Handle *) &ph);
      
      CPORT_FILL_PIXPAT_X (theCPort) = RM (ph);
    }
}

A2(PRIVATE, void, pnsize, INTEGER, pv, INTEGER, ph)
{
    Point p;

    p.h = CW(ph);
    p.v = CW(pv);
    ScalePt(&p, &srcpicframe, &dstpicframe);
    PenSize(CW(p.h), CW(p.v));
}

A1(PRIVATE, void, textface, Byte, f)
{
    TextFace(f);
}

A1(PRIVATE, void, charextra, INTEGER, extra)
{
    /* TODO:  Can't use CharExtra 'cause argument is Fixed */
}

A2(PRIVATE, void, shrtlinefrom, SignedByte, dh, SignedByte, dv)
{
    Line(dh, dv);
}

A1(PRIVATE, void, setpicclip, RgnHandle, rh)
{
    SectRgn(rh, saveclip, rh);
    SetClip(rh);
}

PRIVATE void W_BackPat( Pattern pp )
{
    BackPat( pp );
}

/* routines that associate a PICT font number with a font name */

typedef struct assoc_link
{
  struct assoc_link *nextp;
  StringPtr str;
  INTEGER i;
}
assoc_link_t;

PRIVATE assoc_link_t *assoc_headp = 0;

PRIVATE void
begin_assoc (void)
{
  /* Don't do anything */
}

PRIVATE assoc_link_t **
assoc_find_str (StringPtr sp)
{
  assoc_link_t **retval;

  for (retval = &assoc_headp;
       *retval && EqualString ((*retval)->str, sp, FALSE, TRUE) != 0;
       retval = &(*retval)->nextp)
    ;
  return retval;
}

PRIVATE assoc_link_t **
assoc_find_i (INTEGER i)
{
  assoc_link_t **retval;

  for (retval = &assoc_headp;
       *retval && (*retval)->i != i;
       retval = &(*retval)->nextp)
    ;
  return retval;
}

PRIVATE StringPtr
makestr (StringPtr sp)
{
  StringPtr retval;
  int len;

  len = sp[0] + 1;
  retval = malloc (len);
  memcpy (retval, sp, len);
  return retval;
}

PRIVATE void
add_assoc (INTEGER i, StringPtr sp)
{
  assoc_link_t **pp;

  pp = assoc_find_str (sp);
  if (*pp)
    (*pp)->i = i;
  else
    {
      assoc_link_t *p;

      p = malloc (sizeof *p);
      p->nextp = 0;
      p->str = makestr (sp);
      p->i = i;
      *pp = p;
    }
}

PRIVATE StringPtr
assoc (INTEGER i)
{
  StringPtr retval;
  assoc_link_t **pp;

  pp = assoc_find_i (i);
  if (*pp)
    retval = (*pp)->str;
  else
    retval = 0;

  return retval;
}

PRIVATE void
end_assoc (void)
{
  assoc_link_t *p, *nextp;

  for (p = assoc_headp; p; p = nextp)
    {
      nextp = p->nextp;
      free (p->str);
      free (p);
    }
  assoc_headp = 0;
}

PRIVATE void W_TextFont( INTEGER f )
{
  StringPtr sp;

  sp = assoc (f);
  if (sp)
    {
      INTEGER new_f;
      GetFNum (sp, &new_f);
      if (new_f)
	f = CW (new_f);
    }
  TextFont( f );
}

PRIVATE void W_TextMode( INTEGER m )
{
    TextMode( m );
}

PRIVATE void W_SpaceExtra( Fixed e )
{
    SpaceExtra( e );
}

PRIVATE void W_PenMode( INTEGER m )
{

#define BACHMAN_HACK
#if defined(BACHMAN_HACK)
    if ( m == 23 )
	m = 7;
#endif

    PenMode( m );
}

PRIVATE void W_PenPat( Pattern pp )
{
    PenPat( pp );
}

PRIVATE void W_TextSize( INTEGER s )
{
    TextSize( s );
}

PRIVATE void W_ForeColor( LONGINT c )
{
    ForeColor( c );
}

PRIVATE void W_BackColor( LONGINT c )
{
    BackColor( c );
}

PRIVATE void W_BackPixPat( PixPatHandle ph )
{
    BackPixPat( ph );
}

PRIVATE void W_PenPixPat( PixPatHandle ph )
{
    PenPixPat( ph );
}

PRIVATE void W_RGBForeColor( RGBColor *colorp )
{
    RGBForeColor( colorp );
}

PRIVATE void W_RGBBackColor( RGBColor *colorp )
{
    RGBBackColor( colorp );
}

PRIVATE void W_HiliteColor( RGBColor *colorp )
{
    HiliteColor( colorp );
}

PRIVATE void W_OpColor( RGBColor *colorp )
{
    OpColor( colorp );
}

PRIVATE void W_LineTo( INTEGER h, INTEGER v )
{
    LineTo( h, v );
}

PRIVATE void W_FrameRect( Rect *r )
{
    FrameRect( r );
}

PRIVATE void W_PaintRect( Rect *r )
{
    PaintRect( r );
}

PRIVATE void W_EraseRect( Rect *r )
{
    EraseRect( r );
}

PRIVATE void
reset_hilite_mode (void)
{
  HiliteMode |= 0x80;
}

PRIVATE void W_InvertRect( Rect *r )
{
    InvertRect( r );
    reset_hilite_mode ();
}

PRIVATE void W_FrameRoundRect( Rect *r, INTEGER ow, INTEGER oh )
{
    FrameRoundRect( r, ow, oh );
}

PRIVATE void W_PaintRoundRect( Rect *r, INTEGER ow, INTEGER oh )
{
    PaintRoundRect( r, ow, oh );
}

PRIVATE void W_EraseRoundRect( Rect *r, INTEGER ow, INTEGER oh )
{
    EraseRoundRect( r, ow, oh );
}

PRIVATE void W_InvertRoundRect( Rect *r, INTEGER ow, INTEGER oh )
{
    InvertRoundRect( r, ow, oh );
    reset_hilite_mode ();
}

PRIVATE void W_FrameOval( Rect *r )
{
    FrameOval( r );
}

PRIVATE void W_PaintOval( Rect *r )
{
    PaintOval( r );
}

PRIVATE void W_EraseOval( Rect *r )
{
    EraseOval( r );
}

PRIVATE void W_InvertOval( Rect *r )
{
    InvertOval( r );
    reset_hilite_mode ();
}

PRIVATE void W_FrameArc( Rect *r, INTEGER start, INTEGER angle )
{
    FrameArc( r, start, angle );
}

PRIVATE void W_PaintArc( Rect *r, INTEGER start, INTEGER angle )
{
    PaintArc( r, start, angle );
}

PRIVATE void W_EraseArc( Rect *r, INTEGER start, INTEGER angle )
{
    EraseArc( r, start, angle );
}

PRIVATE void W_InvertArc( Rect *r, INTEGER start, INTEGER angle )
{
    InvertArc( r, start, angle );
    reset_hilite_mode ();
}

PRIVATE void W_FramePoly( PolyHandle poly )
{
    FramePoly( poly );
}

PRIVATE void W_PaintPoly( PolyHandle poly )
{
    PaintPoly( poly );
}

PRIVATE void W_ErasePoly( PolyHandle poly )
{
    ErasePoly( poly );
}

PRIVATE void W_InvertPoly( PolyHandle poly )
{
    InvertPoly( poly );
    reset_hilite_mode ();
}

PRIVATE void W_FrameRgn( RgnHandle rh )
{
    FrameRgn( rh );
}

PRIVATE void W_PaintRgn( RgnHandle rh )
{
    PaintRgn( rh );
}

PRIVATE void W_EraseRgn( RgnHandle rh )
{
    EraseRgn( rh );
}

PRIVATE void W_InvertRgn( RgnHandle rh )
{
    InvertRgn( rh );
    reset_hilite_mode ();
}

PRIVATE void W_ReadComment( INTEGER kind, INTEGER size, Handle hand )
{
    C_ReadComment( kind, size, hand );
}

PRIVATE void
fontname (INTEGER hsize, Handle hand)
{
  char *p;
  INTEGER i;
  StringPtr sp;

  p = (char *) STARH (hand);
  i = CW (*(INTEGER *)p);
  sp = (StringPtr) p + 2;
  add_assoc (i, sp);
}

PRIVATE void
glyphstate (INTEGER hsize, Handle hand)
{
  char *p;

  p = (char *) STARH (hand);

  warning_unimplemented ("partially implemented");
  SetFractEnable (!p[2]);
  SetFScaleDisable (p[3]);
}

PRIVATE wps wparray[] = {
  { (pfv) nop,		xxx0(),					/* 00 */ },
  { (pfv) setpicclip,	yyy1(SCALE0, RGN),			/* 01 */ },
  { (pfv) W_BackPat,	xxx1(PAT),				/* 02 */ },
  { (pfv) W_TextFont,	xxx1(WRD),				/* 03 */ },
  { (pfv) textface,	xxx1(BYT),				/* 04 */ },
  { (pfv) W_TextMode,	xxx1(WRD),				/* 05 */ },
  { (pfv) W_SpaceExtra,	xxx1(LNG),				/* 06 */ },
  { (pfv) pnsize,	xxx2(WRD,WRD),				/* 07 */ },
  { (pfv) W_PenMode,	xxx1(WRD),				/* 08 */ },
  { (pfv) W_PenPat,	xxx1(PAT),				/* 09 */ },
  { (pfv) thepat,	xxx1(PAT),				/* 0A */ },
  { (pfv) nop,		xxx1(PNT),		/* OvSize */	/* 0B */ },
  { (pfv) origin,	xxx2(WRD, WRD),				/* 0C */ },
  { (pfv) W_TextSize,	xxx1(WRD),				/* 0D */ },
  { (pfv) W_ForeColor,	xxx1(LNG),				/* 0E */ },
  { (pfv) W_BackColor,	xxx1(LNG),				/* 0F */ },

  { (pfv) txratio,	xxx2(PNT, PNT),				/* 10 */ },
  { (pfv) nop,		xxx1(BYT),		/* PicVers */	/* 11 */ },
  { (pfv) W_BackPixPat,	xxx1(PXP),				/* 12 */ },
  { (pfv) W_PenPixPat,	xxx1(PXP),				/* 13 */ },
  { (pfv) fillpixpat,	xxx1(PXP),				/* 14 */ },
  { (pfv) pnlochfrac,	xxx1(WRD),				/* 15 */ },
  { (pfv) charextra,	xxx1(WRD),				/* 16 */ },
  { (pfv) nop,		xxx0(),	 /* reserved for Apple use */	/* 17 */ },
  { (pfv) nop,		xxx0(),	 /* reserved for Apple use */	/* 18 */ },
  { (pfv) nop,		xxx0(),	 /* reserved for Apple use */	/* 19 */ },
  { (pfv) W_RGBForeColor, xxx1(RGB),				/* 1A */ },
  { (pfv) W_RGBBackColor, xxx1(RGB),				/* 1B */ },
  { (pfv) hilitemode,	xxx0(),					/* 1C */ },
  { (pfv) W_HiliteColor,xxx1(RGB),				/* 1D */ },
  { (pfv) defhilite,	xxx0(),					/* 1E */ },
  { (pfv) W_OpColor,	xxx1(RGB),				/* 1F */ },

  { (pfv) line,	yyy2(SCALE01, PNT, PNT),			/* 20 */ },
  { (pfv) W_LineTo,	yyy1(SCALE0, PNT),			/* 21 */ },
  { (pfv) shrtline,	yyy3(SCALE012V, PNT, BYT, BYT),		/* 22 */ },
  { (pfv) shrtlinefrom,	yyy2(SCALE01V, BYT, BYT),		/* 23 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 24 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 25 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 26 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 27 */ },
  { (pfv) longtext,	yyy2(SCALE0, PNT, TXT),			/* 28 */ },
  { (pfv) dhtext,	yyy2(SCALE0, BYT, TXT),			/* 29 */ },
  { (pfv) dvtext,	yyy2(SCALE0V, BYT, TXT),		/* 2A */ },
  { (pfv) dhdvtext,	yyy3(SCALE01V, BYT, BYT, TXT),		/* 2B */ },
  { (pfv) fontname, xxx1(DAT),					/* 2C */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 2D */ },
  { (pfv) glyphstate,	xxx1(DAT),				/* 2E */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 2F */ },

  { (pfv) W_FrameRect,	yyy1(SCALE0, RCT),			/* 30 */ },
  { (pfv) W_PaintRect,	yyy1(SCALE0, RCT),			/* 31 */ },
  { (pfv) W_EraseRect,	yyy1(SCALE0, RCT),			/* 32 */ },
  { (pfv) W_InvertRect,	yyy1(SCALE0, RCT),			/* 33 */ },
  { (pfv) fillrct,	yyy1(SCALE0, RCT),			/* 34 */ },
  { (pfv) nop,	xxx1(RCT),	/* reserved for Apple use */	/* 35 */ },
  { (pfv) nop,	xxx1(RCT),	/* reserved for Apple use */	/* 36 */ },
  { (pfv) nop,	xxx1(RCT),	/* reserved for Apple use */	/* 37 */ },
  { (pfv) W_FrameRect,	xxx2(SAM, RCT),				/* 38 */ },
  { (pfv) W_PaintRect,	xxx2(SAM, RCT),				/* 39 */ },
  { (pfv) W_EraseRect,	xxx2(SAM, RCT),				/* 3A */ },
  { (pfv) W_InvertRect,	xxx2(SAM, RCT),				/* 3B */ },
  { (pfv) fillrct,	xxx2(SAM, RCT),				/* 3C */ },
  { (pfv) nop,	xxx2(SAM, RCT),	/* reserved for Apple use */	/* 3D */ },
  { (pfv) nop,	xxx2(SAM, RCT),	/* reserved for Apple use */	/* 3E */ },
  { (pfv) nop,	xxx2(SAM, RCT),	/* reserved for Apple use */	/* 3F */ },

  { (pfv) W_FrameRoundRect,	yyy2(SCALE0, RCT, OVP),		/* 40 */ },
  { (pfv) W_PaintRoundRect,	yyy2(SCALE0, RCT, OVP),		/* 41 */ },
  { (pfv) W_EraseRoundRect,	yyy2(SCALE0, RCT, OVP),		/* 42 */ },
  { (pfv) W_InvertRoundRect,	yyy2(SCALE0, RCT, OVP),		/* 43 */ },
  { (pfv) fillrrct,		yyy2(SCALE0, RCT, OVP),		/* 44 */ },
  { (pfv) nop,		xxx2(RCT, OVP),	/* Apple use */		/* 45 */ },
  { (pfv) nop,		xxx2(RCT, OVP),	/* Apple use */		/* 46 */ },
  { (pfv) nop,		xxx2(RCT, OVP),	/* Apple use */		/* 47 */ },
  { (pfv) W_FrameRoundRect,	xxx3(SAM, RCT, OVP),		/* 48 */ },
  { (pfv) W_PaintRoundRect,	xxx3(SAM, RCT, OVP),		/* 49 */ },
  { (pfv) W_EraseRoundRect,	xxx3(SAM, RCT, OVP),		/* 4A */ },
  { (pfv) W_InvertRoundRect,	xxx3(SAM, RCT, OVP),		/* 4B */ },
  { (pfv) fillrrct,		xxx3(SAM, RCT, OVP),		/* 4C */ },
  { (pfv) nop,		xxx3(SAM, RCT, OVP),	/* Apple use */	/* 4D */ },
  { (pfv) nop,		xxx3(SAM, RCT, OVP),	/* Apple use */	/* 4E */ },
  { (pfv) nop,		xxx3(SAM, RCT, OVP),	/* Apple use */	/* 4F */ },

  { (pfv) W_FrameOval,	yyy1(SCALE0, RCT),			/* 50 */ },
  { (pfv) W_PaintOval,	yyy1(SCALE0, RCT),			/* 51 */ },
  { (pfv) W_EraseOval,	yyy1(SCALE0, RCT),			/* 52 */ },
  { (pfv) W_InvertOval,	yyy1(SCALE0, RCT),			/* 53 */ },
  { (pfv) fillovl,	yyy1(SCALE0, RCT),			/* 54 */ },
  { (pfv) nop,	xxx1(RCT),	/* Apple use */			/* 55 */ },
  { (pfv) nop,	xxx1(RCT),	/* Apple use */			/* 56 */ },
  { (pfv) nop,	xxx1(RCT),	/* Apple use */			/* 57 */ },
  { (pfv) W_FrameOval,	xxx2(SAM, RCT),				/* 58 */ },
  { (pfv) W_PaintOval,	xxx2(SAM, RCT),				/* 59 */ },
  { (pfv) W_EraseOval,	xxx2(SAM, RCT),				/* 5A */ },
  { (pfv) W_InvertOval,	xxx2(SAM, RCT),				/* 5B */ },
  { (pfv) fillovl,	xxx2(SAM, RCT),				/* 5C */ },
  { (pfv) nop,	xxx2(SAM, RCT),		/* Apple use */		/* 5D */ },
  { (pfv) nop,	xxx2(SAM, RCT),		/* Apple use */		/* 5E */ },
  { (pfv) nop,	xxx2(SAM, RCT),		/* Apple use */		/* 5F */ },

  { (pfv) W_FrameArc,	yyy3(SCALE0, RCT, WRD, WRD),		/* 60 */ },
  { (pfv) W_PaintArc,	yyy3(SCALE0, RCT, WRD, WRD),		/* 61 */ },
  { (pfv) W_EraseArc,	yyy3(SCALE0, RCT, WRD, WRD),		/* 62 */ },
  { (pfv) W_InvertArc,	yyy3(SCALE0, RCT, WRD, WRD),		/* 63 */ },
  { (pfv) fillarc,	yyy3(SCALE0, RCT, WRD, WRD),		/* 64 */ },
  { (pfv) nop,	xxx3(RCT, WRD, WRD),	/* Apple use */		/* 65 */ },
  { (pfv) nop,	xxx3(RCT, WRD, WRD),	/* Apple use */		/* 66 */ },
  { (pfv) nop,	xxx3(RCT, WRD, WRD),	/* Apple use */		/* 67 */ },
  { (pfv) W_FrameArc,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),	/* 68 */ },
  { (pfv) W_PaintArc,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),	/* 69 */ },
  { (pfv) W_EraseArc,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),	/* 6A */ },
  { (pfv) W_InvertArc,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),	/* 6B */ },
  { (pfv) fillarc,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),	/* 6C */ },
  { (pfv) nop,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),/* Apple use *//* 6D */ },
  { (pfv) nop,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),/* Apple use *//* 6E */ },
  { (pfv) nop,	xxx6(WRD, WRD, SAM, RCT, WRD, WRD),/* Apple use *//* 6F */ },

  { (pfv) W_FramePoly,	yyy1(SCALE0, PLY),			/* 70 */ },
  { (pfv) W_PaintPoly,	yyy1(SCALE0, PLY),			/* 71 */ },
  { (pfv) W_ErasePoly,	yyy1(SCALE0, PLY),			/* 72 */ },
  { (pfv) W_InvertPoly,	yyy1(SCALE0, PLY),			/* 73 */ },
  { (pfv) fillpoly,	yyy1(SCALE0, PLY),			/* 74 */ },
  { (pfv) nop,	xxx1(PLY),	/* Apple use */			/* 75 */ },
  { (pfv) nop,	xxx1(PLY),	/* Apple use */			/* 76 */ },
  { (pfv) nop,	xxx1(PLY),	/* Apple use */			/* 77 */ },
  { (pfv) W_FramePoly,	xxx2(SAM, PLY),				/* 78 */ },
  { (pfv) W_PaintPoly,	xxx2(SAM, PLY),				/* 79 */ },
  { (pfv) W_ErasePoly,	xxx2(SAM, PLY),				/* 7A */ },
  { (pfv) W_InvertPoly,	xxx2(SAM, PLY),				/* 7B */ },
  { (pfv) fillpoly,	xxx2(SAM, PLY),				/* 7C */ },
  { (pfv) nop,	xxx2(SAM, PLY),	/* Apple use */			/* 7D */ },
  { (pfv) nop,	xxx2(SAM, PLY),	/* Apple use */			/* 7E */ },
  { (pfv) nop,	xxx2(SAM, RGN),	/* Apple use */			/* 7F */ },

  { (pfv) W_FrameRgn,	yyy1(SCALE0, RGN),			/* 80 */ },
  { (pfv) W_PaintRgn,	yyy1(SCALE0, RGN),			/* 81 */ },
  { (pfv) W_EraseRgn,	yyy1(SCALE0, RGN),			/* 82 */ },
  { (pfv) W_InvertRgn,	yyy1(SCALE0, RGN),			/* 83 */ },
  { (pfv) fillrgn,	yyy1(SCALE0, RGN),			/* 84 */ },
  { (pfv) nop,	xxx1(RGN),	/* Apple use */			/* 85 */ },
  { (pfv) nop,	xxx1(RGN),	/* Apple use */			/* 86 */ },
  { (pfv) nop,	xxx1(RGN),	/* Apple use */			/* 87 */ },
  { (pfv) W_FrameRgn,	xxx2(SAM, RGN),				/* 88 */ },
  { (pfv) W_PaintRgn,	xxx2(SAM, RGN),				/* 89 */ },
  { (pfv) W_EraseRgn,	xxx2(SAM, RGN),				/* 8A */ },
  { (pfv) W_InvertRgn,	xxx2(SAM, RGN),				/* 8B */ },
  { (pfv) fillrgn,	xxx2(SAM, RGN),				/* 8C */ },
  { (pfv) nop,	xxx2(SAM, RGN),	/* Apple use */			/* 8D */ },
  { (pfv) nop,	xxx2(SAM, RGN),	/* Apple use */			/* 8E */ },
  { (pfv) nop,	xxx2(SAM, RGN),	/* Apple use */			/* 8F */ },

  { (pfv) nop,	xxx1(SPL),		   /* unpacked bits rect   90 */ },
  { (pfv) nop,	xxx1(SPL),		    /* unpacked bits rgn   91 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 92 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 93 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 94 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 95 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 96 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 97 */ },
  { (pfv) nop,	xxx1(SPL),		     /* packed bits rect   98 */ },
  { (pfv) nop,	xxx1(SPL),		      /* packed bits rgn   99 */ },
  { (pfv) nop,	xxx1(SPL),	/* direct bits rect */		/* 9A */ },
  { (pfv) nop,	xxx1(SPL),	/* direct bits rgn */		/* 9B */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 9C */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 9D */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 9E */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* 9F */ },

  { (pfv) myreadcment,xxx1(WRD),				/* A0 */ },
  { (pfv) W_ReadComment,xxx2(WRD, DAT),				/* A1 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A2 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A3 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A4 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A5 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A6 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A7 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A8 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* A9 */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AA */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AB */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AC */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AD */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AE */ },
  { (pfv) nop,	xxx1(DAT),	/* reserved for Apple use */	/* AF */ },
};

#define RGBPat	2
 
/*
 * NOTE: we move nextbytep even if we're calling someone else's routine
 *	 so we can maintain the proper alignment in nextop()
 */

PRIVATE Byte eatByte()
{
    Byte retval;

    if (procp)
	CToPascalCall(procp, CTOP_StdGetPic, &retval, sizeof(Byte));
    else
	retval = *nextbytep;
    ++nextbytep;
    return retval;
}

PRIVATE INTEGER eatINTEGERX()
{
    INTEGER retval;

    if (procp)
	CToPascalCall(procp, CTOP_StdGetPic, &retval, sizeof(INTEGER));
    else
	retval = *(INTEGER *)nextbytep;
    nextbytep += sizeof(INTEGER);
    return retval;
}

PRIVATE INTEGER eatINTEGER()
{
    INTEGER retval;

    retval = eatINTEGERX();
    return CW(retval);
}

PRIVATE LONGINT eatLONGINTX()
{
    LONGINT retval;

    if (procp)
	CToPascalCall(procp, CTOP_StdGetPic, &retval, sizeof(LONGINT));
    else
	retval = *(LONGINT *)nextbytep;
    nextbytep += sizeof(LONGINT);
    return retval;
}

PRIVATE LONGINT eatLONGINT()
{
    LONGINT retval;

    retval = eatLONGINTX();
    return CL(retval);
}

PRIVATE void eatString(Str255 str)
{
    str[0] = eatByte();
    if (procp)
	CToPascalCall(procp, CTOP_StdGetPic, str+1, str[0]);
    else
        BlockMove((Ptr) nextbytep, (Ptr) str+1, str[0]);
    nextbytep += str[0];
}

PRIVATE void eatNBytes(LONGINT n)
{
    char *bufp;

    if (procp) {
        TEMP_ALLOC_DECL (temp_alloc_space);
	TEMP_ALLOC_ALLOCATE (bufp, temp_alloc_space, n);
	CToPascalCall(procp, CTOP_StdGetPic, bufp, n);
	TEMP_ALLOC_FREE (temp_alloc_space);
    }
    nextbytep += n;
}

A2(PRIVATE, void, eatRegion, RgnHandle, rh, Size, hs)
{
    SignedByte state;

    SetHandleSize((Handle) rh, hs);
    if (procp) {
	state = HGetState((Handle) rh);
	HLock((Handle) rh);
	CToPascalCall(procp, CTOP_StdGetPic, (Ptr) STARH(rh) + sizeof(INTEGER),
							 hs - sizeof(INTEGER));
	HSetState((Handle) rh, state);
    } else
        BlockMove((Ptr) nextbytep, (Ptr) STARH(rh) + sizeof(INTEGER),
							 hs - sizeof(INTEGER));
    HxX(rh, rgnSize) = CW(hs);
    nextbytep += hs - sizeof(INTEGER);
}

A1(PRIVATE, void, eatRect, Rect *, rp)
{
    rp->top    = eatINTEGERX();
    rp->left   = eatINTEGERX();
    rp->bottom = eatINTEGERX();
    rp->right  = eatINTEGERX();
}

A2(PRIVATE, void, eatPixMap, register PixMapPtr, pixp, INTEGER, rowb)
{
    /* TODO:  byte swapping stuff, testing */

    /* x(pixp->baseAddr)   = 0; will be set later */
    pixp->rowBytes   = rowb ? CW(rowb) : eatINTEGERX();
    eatRect(&pixp->bounds);
    pixp->pmVersion  = eatINTEGERX();
    pixp->packType   = eatINTEGERX();
    pixp->packSize   = eatLONGINTX();
    pixp->hRes       = eatLONGINTX();
    pixp->vRes       = eatLONGINTX();
    pixp->pixelType  = eatINTEGERX();
    pixp->pixelSize  = eatINTEGERX();
    pixp->cmpCount   = eatINTEGERX();
    pixp->cmpSize    = eatINTEGERX();
    pixp->planeBytes = eatLONGINTX();
    (void) eatLONGINTX();	/* IMV-104 */
    pixp->pmTable    = RM((CTabHandle) NewHandle(sizeof(ColorTable)));
					/* will be filled in later */
    pixp->pmReserved = eatLONGINTX();
}

A2(PRIVATE, void, eatBitMap, BitMap *, bp, INTEGER, rowb)
{
    bp->baseAddr = 0;
    bp->rowBytes = rowb ? CW(rowb) : eatINTEGERX();
    eatRect(&bp->bounds);
}

A2(PRIVATE, Size, eatpixdata, PixMapPtr, pixmap, BOOLEAN *, freep)
{
    int rowb;
    Size pic_data_size, final_data_size;
    HIDDEN_Ptr temp_pp, dp;
    Handle h;
    Byte *inp;
    Handle temph;
    INTEGER length;
    boolean_t insert_pad_byte_p;
    int comp_bytes;
    int height;
    
    height = RECT_HEIGHT (&pixmap->bounds);
    rowb = BITMAP_ROWBYTES (pixmap);
    /* comp bytes is the number of bytes take up by each of r, g, b
       per scanline */
    comp_bytes = rowb / 4;
    insert_pad_byte_p = (pixmap->pixelSize == CWC (32)
			 && pixmap->cmpCount != CWC (4));

    
    final_data_size = rowb * height;
    
    if (pixmap->packType == CWC (2))
      pic_data_size = 3 * comp_bytes * height;
    else
      pic_data_size = final_data_size;
    if (rowb < 8 || pixmap->packType == CWC (2))
      {
	if (procp
	    || pixmap->packType == CWC (2))
	  {
	    h = NewHandle (final_data_size);
	    /* The practice of trying again in SysZone comes from the
	       database "Panorama" which calls MaxMem, subtracts a
	       small number from that, then calls NewPtr with the
	       result (i.e. asking for all of memory minus a small
	       number of bytes), then later DrawPicture is called, and
	       our implementation runs out of memory.  It's not clear
	       what happens on a Mac.  This hack should be more
	       thoroughly investigated sometime. */
	    
	    if (h == NULL)
	      {
		ZONE_SAVE_EXCURSION
		  (SysZone,
		   {
		     h = NewHandle (final_data_size);
		   });
	      }
	    HLock (h);
	    
	    if (procp)
	      CToPascalCall (procp, CTOP_StdGetPic, STARH (h), pic_data_size);
	    else if (pixmap->packType == CWC (2))
	      memcpy (STARH (h), nextbytep, pic_data_size);
	    
	    pixmap->baseAddr = h->p;
	    *freep = TRUE;
	  }
	else
	  {
	    pixmap->baseAddr = RM ((Ptr) nextbytep);
	    *freep = FALSE;
	  }
	nextbytep += pic_data_size;
	if (pic_data_size & 1)
	  ++nextbytep;
      }
    else
      {
	uint8 *temp_scanline, *scanline, *ep;
	
	h = NewHandle (final_data_size);
	if (h == NULL)
	  {
	    ZONE_SAVE_EXCURSION
	      (SysZone,
	       {
		 h = NewHandle (final_data_size);
	       });
	  }
	HLock(h);
	pixmap->baseAddr = (*h).p;	/* can't use STARH 'cause we don't */
					/* want to byte swap the result */
	temp_scanline = alloca (rowb);
	for (scanline = (uint8 *) BITMAP_BASEADDR (pixmap),
	       ep = scanline + final_data_size;
	     scanline < ep;
	     scanline += rowb)
	  {
	    int i;
	    
	    length = rowb > 250 ? eatINTEGER () : eatByte ();
	    if (procp)
	      {
		temph = NewHandle (length);
		HLock (temph);
		CToPascalCall (procp, CTOP_StdGetPic, STARH (temph), length);
		inp = (Byte *) STARH (temph);
	      }
	    else
	      {
		inp = nextbytep;
		temph = NULL;
	      }
	    dp.p = (Ptr) RM (temp_scanline);
	    temp_pp.p = (Ptr) RM (inp);

	    if (pixmap->pixelSize == CWC (16)
		&& pixmap->packType == CWC (3))
	      unpack_int16_bits (&temp_pp, &dp, rowb);
	    else
	      UnpackBits (&temp_pp, &dp,
			  insert_pad_byte_p ? comp_bytes * 3 : rowb);
	    inp = MR ((unsigned char *) temp_pp.p);
	    
	    if (pixmap->pixelSize == CWC (32))
	      {
		for (i = 0; i < comp_bytes; i ++)
		  {
		    /* ### are the packed unused bytes first or last in
		       the packed scanline? */
		    if (insert_pad_byte_p)
		      scanline[i * 4] = 0;
		    else
		      scanline[i * 4] = temp_scanline[i + 3 * comp_bytes];
		
		    scanline[i * 4 + 1] = temp_scanline[i                 ];
		    scanline[i * 4 + 2] = temp_scanline[i + comp_bytes    ];
		    scanline[i * 4 + 3] = temp_scanline[i + 2 * comp_bytes];
		  }
	      }
	    else
	      memcpy (scanline, temp_scanline, rowb);
	    
	    if (!procp)
	      nextbytep = inp;
	    else
	      {
		nextbytep += length;
		DisposHandle (temph);
	      }
	  }
	*freep = TRUE;
      }
    
    if (pixmap->packType == CWC (2))
      {
	uint8 *start, *src, *dst;
	
	start = (uint8 *) BITMAP_BASEADDR (pixmap);
	
	src = start + height * comp_bytes * 3;
	dst = start + height * rowb;
	
	while (src > start)
	  {
	    *--dst = *--src;
	    *--dst = *--src;
	    *--dst = *--src;
	    *--dst = 0;
	  }
      }
    
    pixmap->packType = CWC (0);
    
    return final_data_size;
}

A2(PRIVATE, void, eatbitdata, register BitMap *, bp, BOOLEAN, packed)
{
    INTEGER rowb;
    Size datasize;
    Ptr ep;
    HIDDEN_Ptr dp, temp_pp;
    Handle h;
    Byte *inp;
    INTEGER length;
    Handle temph;

    rowb = CW(bp->rowBytes) & ROWMASK;
    datasize = (LONGINT) rowb * (CW(bp->bounds.bottom) - CW(bp->bounds.top));
    if (!packed) {
	if (procp) {
	    h = NewHandle(datasize);
	    if (!h)
	      {
		THz savezone;

		savezone = TheZone;
		TheZone = SysZone;
		h = NewHandle(datasize);
		TheZone = savezone;
	      }
	    HLock(h);
	    CToPascalCall(procp, CTOP_StdGetPic, STARH(h), datasize);
	    bp->baseAddr = (*h).p;
	} else
	    bp->baseAddr = RM((Ptr) nextbytep);
	nextbytep += datasize;
    } else {
	h = NewHandle(datasize);
	if (!h)
	  {
	    THz savezone;
	    
	    savezone = TheZone;
	    TheZone = SysZone;
	    h = NewHandle(datasize);
	    TheZone = savezone;
	  }
	HLock(h);
	bp->baseAddr = (*h).p;	/* can't use STARH */
	for (dp.p = MR(bp->baseAddr), ep = dp.p + datasize; dp.p < ep; ) {
	    length = rowb > 250 ? eatINTEGER() : eatByte();
	    if (procp) {
		temph = NewHandle(length);
		HLock(temph);
		CToPascalCall(procp, CTOP_StdGetPic, STARH(temph), length);
		inp = (Byte *) STARH(temph);
	    } else {
		inp = nextbytep;
#if !defined(LETGCCWAIL)
		temph = 0;
#endif
	    }
	    inp = RM(inp);
	    dp.p = RM(dp.p);
	    temp_pp.p = (Ptr) inp;
	    UnpackBits(&temp_pp, &dp, rowb);
	    inp = (unsigned char *) temp_pp.p;
	    dp.p = MR(dp.p);
	    inp = MR(inp);
	    if (!procp)
		nextbytep = inp;
	    else {
		nextbytep += length;
		DisposHandle(temph);
	    }
	}
    }
}

A1(PRIVATE, void, eatRGBColor, RGBColor *, rgbp)
{
    rgbp->red   = eatINTEGERX();
    rgbp->green = eatINTEGERX();
    rgbp->blue  = eatINTEGERX();
}

A1(PRIVATE, void, eatColorTable, register PixMapPtr, pixmap)
{
    register CTabPtr cp;
    register ColorSpec *cspecp, *cspecep;
    register CTabHandle ch;

    ch = MR(pixmap->pmTable);
    cp = STARH(ch);
    /* cp->ctSeed = */  eatLONGINTX();
    cp->ctSeed = CL (GetCTSeed ());
    cp->ctFlags = eatINTEGERX();
    cp->ctSize  = eatINTEGERX();
    SetHandleSize((Handle) ch, (Size)sizeof(ColorTable) - sizeof(cp->ctTable) +
				     (Cx(cp->ctSize)+1) * 4 * sizeof(INTEGER));
    for (cspecp = HxX(ch, ctTable), cspecep = cspecp + Hx(ch, ctSize) + 1;
						 cspecp != cspecep; cspecp++) {
	cspecp->value = eatINTEGERX();
	eatRGBColor(&cspecp->rgb);
    }
}

A1(PRIVATE, void, eatPattern, Pattern, pat)
{
    pat[0] = eatByte();
    pat[1] = eatByte();
    pat[2] = eatByte();
    pat[3] = eatByte();
    pat[4] = eatByte();
    pat[5] = eatByte();
    pat[6] = eatByte();
    pat[7] = eatByte();
}

A1 (PRIVATE, void, eatPixPat, PixPatHandle, pixpat)
{
  RGBColor rgb;
  Size datasize;
  Handle temph;

  LOCK_HANDLE_EXCURSION_1
    (pixpat,
     {
       PIXPAT_TYPE_X (pixpat) = eatINTEGERX ();
       if (PIXPAT_TYPE_X (pixpat) == CWC (RGBPat))
	 {
	   eatPattern (PIXPAT_1DATA (pixpat));
	   eatRGBColor (&rgb);
	   {
	     PixMapHandle patmap;

	     patmap = (PixMapHandle) NewHandleClear (sizeof (PixMap));
	     PIXMAP_TABLE_X (patmap) = (CTabHandle) RM (NewHandle (0));
	     PIXPAT_MAP_X (pixpat) = RM (patmap);
	   }
	   MakeRGBPat (pixpat, &rgb);
	 }
       else
	 {
	   PixMapHandle patmap;
	   BOOLEAN free;
	   
	   eatPattern (PIXPAT_1DATA (pixpat));
	   patmap = (PixMapHandle) NewHandle (sizeof (PixMap));
	   PIXPAT_MAP_X (pixpat) = RM (patmap);
	   LOCK_HANDLE_EXCURSION_1
	     (patmap,
	      {
		PixMapPtr patmap_ptr = STARH (patmap);
		
		eatPixMap (patmap_ptr, 0);
		eatColorTable (patmap_ptr);
		
		datasize = eatpixdata (patmap_ptr, &free);
		temph = NewHandle (0); /* do not use NewEmptyHandle, because
					  the call to PtrToXHand below will
					  fail, with a nilHandleErr as per
					  IM Memory 2-62 */
		PIXPAT_DATA_X (pixpat) = RM (temph);
		PtrToXHand (BITMAP_BASEADDR (patmap_ptr), temph, datasize);
		if (free)
		  DisposHandle (RecoverHandle (BITMAP_BASEADDR (patmap_ptr)));
		
		HASSIGN_3
		  (pixpat,
		   patXMap, RM (NULL),
		   patXData, RM (NewHandleClear (sizeof (xdata_t))),
		   patXValid, RM (-1));
	      });
	 }
   });
}

#define opEndPic	0xff

A1 (PRIVATE, unsigned short, nextop, INTEGER, vers)
{
  unsigned int retval;
  
  if (vers == 1)
    {
      retval = eatByte() & 0xFF;
    }
  else
    {
      /* require even alignment */
      if ((int32) nextbytep & 1)
	eatByte();
      retval = eatINTEGER();
    }
  return retval;
}

#define SE(x) ((x & 0x80) ? x|(~0^0xff) : x & 0xff)	/* sign extend */

P2(PUBLIC pascal trap, void, DrawPicture, PicHandle, pic, Rect *, destrp)
{
    INTEGER words[2], *wp;
    Point  points[2], *pp;
    Byte    bytes[2], *bp;
    Rect    rects[3], *rp;
    Handle  hand;
    LONGINT lng;
    ULONGINT ac;
    unsigned short sc;
    unsigned int opcode;
    INTEGER ovh, ovw;
    Pattern ourpattern;
    Str255 ourstring;
    INTEGER hsize;
    void (*f)();
    INTEGER vers;
    auto Fixed scaleh, scalev, tempf;	/* "auto":  cc -a bug avoidance */
    RGBColor rgb;
    BitMap bm;
    auto PixMap pm;	/* "auto":  cc -a bug avoidance */
    BOOLEAN packed;
    GrafPort saveport, *the_port;
    CGrafPtr the_cport;
#if 0
    INTEGER tempshort;
#endif
    INTEGER state;
    BitMap *bmp;
    QDProcsPtr grafprocp;
    LONGINT templ;
    SignedByte state2;
    /* this is not saved automagically if the port is a color
       grafport, hence we need to save and restore it */
    Rect saveportbounds;
    
    int16 version_2, version_2ext;
    Fixed hRes, vRes;
    PixPatHandle junk_pen_pixpat, junk_bk_pixpat, junk_fill_pixpat;
    BOOLEAN saveFractEnable, saveFScaleDisable;
    Byte saveHiliteMode;

#if 0
    fprintf (stderr, "DrawPicture (%d): %d\n", debugnumber,
	     GetHandleSize (pic));
#endif
    
#if 0
/*
 * NOTE: At one time I believed that picSize 10 pictures should be
 *	 silently ignored, but I'm not convinced that this is true;
 *	 it could be that a different bug was creating/passing spurious
 *	 pictures.  I know for a fact (having tested it on a Mac) that
 *	 picSize 0 pictures should be printed (Word5 creates 'em sometimes).
 *	 --ctm
 */
    if (Hx(pic, picSize) <= 10)
/*-->*/	return;
#endif
#if !defined (LETGCCWAIL)
    lng = 0;
    ovh = 0;
    ovw = 0;
    hsize = 0;
#endif /* LETGCCWAIL */

#if 0
    if (!pic || (CW (destrp->top) == CW (destrp->bottom)
		 && CW (destrp->left) == CW (destrp->right)))
      return;
#else
    if (!pic || !pic->p || EmptyRect(destrp) || EmptyRect(&HxX(pic, picFrame)))
      return;
#endif


    saveHiliteMode = HiliteMode;
    saveHiliteRGB = HiliteRGB;
    saveFractEnable = FractEnable;
    saveFScaleDisable = FScaleDisable;
    begin_assoc ();
    the_port = thePort;
    if (CGrafPort_p (the_port))
      the_cport = theCPort;
    else
      the_cport = NULL;

    saveport = *the_port;
    saveportbounds = PORT_BOUNDS (the_port);
    PORT_CLIP_REGION_X (the_port) = RM (NewRgn ());

    SetRectRgn (PORT_CLIP_REGION (the_port), -32768, -32768, 32767, 32767);
    
    PORT_VIS_REGION_X (the_port) = RM (NewRgn ());
    CopyRgn (PORT_VIS_REGION (&saveport),
	     PORT_VIS_REGION (the_port));
    
    saveclip = NewRgn ();
    CopyRgn (PORT_CLIP_REGION (&saveport), saveclip);
    
    if (CGrafPort_p (the_port))
      {
 	junk_pen_pixpat  = NewPixPat ();
 	junk_bk_pixpat   = NewPixPat ();
	junk_fill_pixpat = NewPixPat ();
 	CPORT_PEN_PIXPAT_X (the_cport)  = RM (junk_pen_pixpat);
 	CPORT_BK_PIXPAT_X (the_cport)   = RM (junk_bk_pixpat);
 	CPORT_FILL_PIXPAT_X (the_cport) = RM (junk_fill_pixpat);
      }
    else
      {
	junk_pen_pixpat = junk_bk_pixpat = junk_fill_pixpat = 0;
      }

    /* These will replace the junk pixpats we just installed. */
    BackPat (white);
    ROMlib_fill_pat (black);
    PenPat (black);

    /* Free these up now, since they are no longer used. */
    if (CGrafPort_p (the_port))
      {
	DisposPixPat (junk_pen_pixpat);
	DisposPixPat (junk_bk_pixpat);
	DisposPixPat (junk_fill_pixpat);
      }

    PORT_PEN_LOC (the_port).h  = destrp->left;	/* This is a guess, based on */
    PORT_PEN_LOC (the_port).v  = destrp->top;	/* Word's EPS handling */
    PORT_PEN_SIZE (the_port).h = PORT_PEN_SIZE (the_port).v = CWC (1);
    PORT_PEN_MODE_X (the_port) = CWC (patCopy);
    PORT_TX_FONT_X (the_port) = CWC (0);
    PORT_TX_FACE_X (the_port) = 0;
    PORT_TX_SIZE_X (the_port) = CWC (0);
    PORT_SP_EXTRA_X (the_port) = CWC (0);

#if 0
    /* this will fail if we are actually drawing to a
       color graph port; we'll get spew colors instead of
       b/w, so let ForeColor or BackColor do the work */
    the_port->fgColor = CL(blackColor);
    the_port->bkColor = CL(whiteColor);
#else
    ForeColor (blackColor);
    BackColor (whiteColor);
#endif 
    PORT_COLR_BIT_X (the_port) = CWC (0);
    PORT_PAT_STRETCH_X (the_port) = CWC (0);

    /*
     * NOTE:  I used to start pics out with srcXor because of a PICT
     *	      in LightspeedC, but it turns out the problem is that
     *	      srcOr with outline doesn't do what one might think
     */
    TextMode(srcOr);
    vers = 1;
    state = HGetState((Handle) pic);
    HLock((Handle) pic);

    grafprocp = MR (the_port->grafProcs);
    if (grafprocp)
      {
	procp = MR(grafprocp->getPicProc);
	if (procp == P_StdGetPic)
	  procp = 0;
      }
    else
      procp = 0;
    nextbytep = (unsigned char *) (&HxX(pic, picSize) + 5);
    hand  = 0;
    
    dstpicframe = *destrp;
    srcpicframe = HxX (pic, picFrame);
    
    picnumh = RECT_WIDTH (&dstpicframe);
    picnumv = RECT_HEIGHT (&dstpicframe);
    picdenh = RECT_WIDTH (&srcpicframe);
    picdenv = RECT_HEIGHT (&srcpicframe);

    txnumh = txdenh = txnumv = txdenv = 1;
    reduce(&picnumh, &picdenh);
    reduce(&picnumv, &picdenv);

    txtpoint.h = CW(CW(dstpicframe.left) - CW(srcpicframe.left));
    txtpoint.v = CW(CW(dstpicframe.top)  - CW(srcpicframe.top));

    scaleh = FixRatio (RECT_WIDTH (&dstpicframe), RECT_WIDTH (&srcpicframe));
    scalev = FixRatio (RECT_HEIGHT (&dstpicframe), RECT_HEIGHT (&srcpicframe));
    while ((opcode = nextop(vers)) != opEndPic)
      {
	wp = words;
	pp = points;
	bp = bytes;
	rp = rects;
	if (opcode < NELEM(wparray))
	  {
	    for (ac = wparray[opcode].argcode, sc = scalevalues[ac >> 28],
				       ac &= ARGMASK; ac; ac >>= 4, sc >>= 2) {
		switch (ac & 0xF)
		  {
		  case OVP:
		    break;
		  case BYT:
		    *bp++ = eatByte();
		    if (sc & SCALEIT) {
			tempf = ((LONGINT) bp[-1] << 16) |
					      (bp[-1] & 0x80 ? 0xFF000000 : 0);
			if (sc & VONLY)
			    bp[-1] = FixMul(scalev, tempf) >> 16;
			else
			    bp[-1] = FixMul(scaleh, tempf) >> 16;
		    }
		    break;
		case WRD:
		    *wp++ = eatINTEGER();
		    if (sc & SCALEIT) {
			tempf = (LONGINT) wp[-1] << 16;
			if (sc & VONLY)
			    wp[-1] = FixMul(scalev, tempf) >> 16;
			else
			    wp[-1] = FixMul(scaleh, tempf) >> 16;
		    }
		    break;
		case LNG:
		    lng = eatLONGINT();
		    break;
		case PNT:
		    pp->v = eatINTEGERX();
		    pp->h = eatINTEGERX();
		    if (sc & SCALEIT)
			MapPt(pp, &srcpicframe, destrp);
		    pp->h = CW(pp->h);
		    pp->v = CW(pp->v);
		    ++pp;
		    break;
		case RCT:
		    eatRect(rp++);
		    if (sc & SCALEIT)
			MapRect(&rp[-1], &srcpicframe, destrp);
		    break;
		case PAT:
		    eatPattern(ourpattern);
		    break;
		case TXT:
		    eatString(ourstring);
		    break;
		case RGN:
		case DAT:
		case PLY:
		    hsize = eatINTEGER();
		    if (!hand)
			hand = NewHandle(hsize);
		    if ((ac&0xf) == DAT) {
			SetHandleSize(hand, hsize);
			if (procp) {
			    state2 = HGetState(hand);
			    HLock(hand);
			    CToPascalCall(procp, CTOP_StdGetPic, STARH(hand),
									hsize);
			    HSetState(hand, state2);
			} else
			    BlockMove((Ptr) nextbytep, STARH(hand), hsize);
			nextbytep += hsize;
		    } else
			eatRegion((RgnHandle) hand, hsize);
		    if (sc & SCALEIT)
		      {
			if ((ac&0xF) == PLY)
			    MapPoly((PolyHandle) hand, &srcpicframe, destrp);
			else
			    MapRgn((RgnHandle) hand, &srcpicframe, destrp);
		      }
		    break;
		case SAM:
		    ac = 0;	/* we've already read everything */
		    break;

		case RGB:
		    eatRGBColor(&rgb);
		    break;

		case PXP:
		    if (!hand)
			hand = NewHandle(sizeof(PixPat));
		    else
			ReallocHandle(hand, sizeof(PixPat));
		    memset (STARH (hand), 0, sizeof (PixPat));
		    eatPixPat((PixPatHandle) hand);
		    break;

		case SPL:

		  /* Apparently calling DrawPicture while a pict is open
		     can create a new pict that is much larger than the
		     previous one, due to the way we handle direct bits.
		     It looks like by the time we actually log the CopyBits
		     associated with direct bits, we've already done some
		     transformations (unpacking, and perhaps changing to
		     32-bpp).  We need to have code here that saves all
		     the args that we're reading here and then writes them
		     to the open pict with the only change being the dest
		     rect modifications that we may need to do.  Then we'll
		     have to set a flag (or call an internal variant of
		     CopyBits) to prevent the CopyBits from logging again.
		     Of course this could be problematic if people patch
		     out CopyBits */
		     

		    if (   opcode == OP_DirectBitsRect
			|| opcode == OP_DirectBitsRgn)
		      /* suck off `0xFF' baseaddr */
		      eatLONGINT ();
		    words[0] = eatINTEGER();	/* rowb */
		    packed = (   opcode == OP_PackBitsRect
			      || opcode == OP_PackBitsRgn);
		    if (words[0] & 0x8000)
		      {
			eatPixMap (&pm, words[0]);
			if (   opcode != OP_DirectBitsRect
			    && opcode != OP_DirectBitsRgn)
			  eatColorTable (&pm);
		      }
		    else
		      eatBitMap (&bm, words[0]);
		    eatRect (&rects[0]);	/* src rect */
		    eatRect (&rects[1]);	/* dst rect */
		    MapRect (&rects[1], &srcpicframe, destrp);
		    words[1] = eatINTEGER ();	/* mode */
		    if (   opcode == OP_BitsRgn
			|| opcode == OP_PackBitsRgn
			|| opcode == OP_DirectBitsRgn)
		      {
			if (!hand)
			  hand = NewHandle (0); /* NewEmptyHandle may cause
						   trouble here, since
						   SizeHandle won't properly
						   adjust it */
			hsize = eatINTEGER();
			eatRegion ((RgnHandle) hand, hsize);
			MapRgn ((RgnHandle) hand, &srcpicframe, destrp);
		      }
		    else
		      {
			if (hand)
			  DisposHandle (hand);
			hand = NULL;
		      }
		    
		    if (words[0] & 0x8000)
		      {
			eatpixdata (&pm, &packed);
			bmp = (BitMap *) &pm;
		      }
		    else
		      {
			eatbitdata (&bm, packed);
			bmp = &bm;
		      }
		    CopyBits (bmp, PORT_BITS_FOR_COPY (the_port),
			      &rects[0], &rects[1],
			      words[1], (RgnHandle) hand);
		    if (packed)
		      DisposHandle (RecoverHandle (BITMAP_BASEADDR (bmp)));
		    if (words[0] & 0x8000)
		      DisposHandle ((Handle) MR (pm.pmTable));
		    else if (!packed && procp)
		      DisposHandle (RecoverHandle (BITMAP_BASEADDR (bmp)));
		    break;
		    
		default:
		    gui_fatal ("unknown arg code `%d'",
			       ac & 0xF);
		}
	    }
	  }
	else if (opcode == 0x02FF)
	  {
	    version_2 = eatINTEGER ();
	  }
	else if (opcode == 0x0C00)
	  {
	    /* extended v2 opcode header */
	    /* version */
	    version_2ext = eatINTEGER ();
	    /* reserved */
	    eatINTEGER ();
	    
	    /* hres, Fixed */
	    hRes = eatLONGINT ();
	    /* vres, Fixed */
	    vRes = eatLONGINT ();

#define CLIFFS_QUICK_199_P2_HACK /* makes risk work properly */
#if defined (CLIFFS_QUICK_199_P2_HACK)
	    {
	      Rect new_src_pic_frame;

	      eatRect (&new_src_pic_frame);
	      if (! EmptyRect (&new_src_pic_frame))
		srcpicframe = new_src_pic_frame;
	    }
#else
	    eatRect (&srcpicframe);
	    if (EmptyRect (&srcpicframe))
	      goto cleanup;
#endif
	    
	    /* reserved */
	    eatLONGINT ();
	  }
	else
	  {
	    warning_unexpected ("hit opcode `0x%X' with no table entry",
				opcode);
	    
	    if (opcode < 0x00D0)
		;	/* do nothing; no extra data */
	    else if (opcode < 0x0100) {
		templ = eatLONGINT();
		eatNBytes(templ + sizeof(LONGINT));
	    } else if (opcode < 0x8000)
		eatNBytes((opcode >> 8) * 2);
	    else if (opcode < 0x8100)
		;	/* do nothing; no extra data */
	    else {
		templ = eatLONGINT();
		eatNBytes(templ);
	    }
	  }
	if (opcode == OP_OvSize) {
	    points[0].h = CW(points[0].h);
	    points[0].v = CW(points[0].v);
	    ScalePt(&points[0], &srcpicframe, &dstpicframe);
	    points[0].h = CW(points[0].h);
	    points[0].v = CW(points[0].v);
	    ovh = points[0].v;
	    ovw = points[0].h;
	} else if (opcode == OP_Version) {
	    vers = bytes[0];
	} else if (opcode < NELEM(wparray)) {
	    f = wparray[opcode].func;
	    switch (wparray[opcode].argcode & ARGMASK) {
	    case xxx0():
#if !defined (__STDC__)
		(*f)();
#else /* __STDC__ */
		(* (void (*)(void)) f)();
#endif /* __STDC__ */
		break;
	    case xxx1(BYT):
#if !defined (__STDC__)
		(*f)(SE(bytes[0]));
#else /* __STDC__ */
		(* (void (*)(Byte)) f)(SE(bytes[0]));
#endif /* __STDC__ */
		break;
	    case xxx1(LNG):
#if !defined (__STDC__)
		(*f)(lng);
#else /* __STDC__ */
		(* (void (*)(LONGINT)) f)(lng);
#endif /* __STDC__ */
		break;
	    case xxx1(PAT):
#if !defined (__STDC__)
		(*f)(ourpattern);
#else /* __STDC__ */
		(* (void (*)(Pattern)) f)(ourpattern);
#endif /* __STDC__ */
		break;
	    case xxx1(PNT):
#if !defined (__STDC__)
		(*f)(points[0]);
#else /* __STDC__ */
		(* (void (*)(Point)) f)(points[0]);
#endif /* __STDC__ */
		break;
	    case xxx1(RCT):
	    case xxx2(SAM, RCT):
#if !defined (__STDC__)
		(*f)(&rects[0]);
#else /* __STDC__ */
		(* (void (*)(Rect *)) f)(&rects[0]);
#endif /* __STDC__ */
		break;
	    case xxx1(RGB):
#if !defined (__STDC__)
		(*f)(&rgb);
#else /* __STDC__ */
		(* (void (*)(RGBColor *)) f)(&rgb);
#endif /* __STDC__ */
		break;
	    case xxx1(DAT):
#if !defined (__STDC__)
		(*f)(hsize, hand);
#else /* __STDC__ */
		(* (void (*)(INTEGER, Handle)) f)(hsize, hand);
#endif /* __STDC__ */
		break;
	    case xxx1(PXP):
#if !defined (__STDC__)
		(*f)(hand);
#else /* __STDC__ */
		(* (void (*)(Handle)) f)(hand);
#endif /* __STDC__ */
		hand = NULL;
		break;
	    case xxx1(RGN):
	    case xxx1(PLY):
	    case xxx2(SAM, RGN):
	    case xxx2(SAM, PLY):
#if !defined (__STDC__)
		(*f)(hand);
#else /* __STDC__ */
		(* (void (*)(Handle)) f)(hand);
#endif /* __STDC__ */
		break;
	    case xxx1(WRD):
#if !defined (__STDC__)
		(*f)(words[0]);
#else /* __STDC__ */
		(* (void (*)(INTEGER)) f)(words[0]);
#endif /* __STDC__ */
		break;
	    case xxx1(SPL):
		/* was done when the args were picked up */
		break;
	    case xxx2(BYT, BYT):
#if !defined (__STDC__)
		(*f)(SE(bytes[0]), SE(bytes[1]));
#else /* __STDC__ */
		(* (void (*)(Byte, Byte)) f)
						  (SE(bytes[0]), SE(bytes[1]));
#endif /* __STDC__ */
		break;
	    case xxx2(BYT, TXT):
#if !defined (__STDC__)
		(*f)(SE(bytes[0]), ourstring, &txtpoint);
#else /* __STDC__ */
		(* (void (*)(Byte, StringPtr, Point *)) f)
					       (SE(bytes[0]), ourstring, &txtpoint);
#endif /* __STDC__ */
		break;
	    case xxx2(PNT, PNT):
#if !defined (__STDC__)
		(*f)(points[0], points[1]);
#else /* __STDC__ */
		(* (void (*)(Point, Point)) f)(points[0], points[1]);
#endif /* __STDC__ */
		break;
	    case xxx2(PNT, TXT):
#if !defined (__STDC__)
		(*f)(points[0], ourstring, &txtpoint);
#else /* __STDC__ */
		(* (void (*)(Point, StringPtr, Point *)) f)
						  (points[0], ourstring, &txtpoint);
#endif /* __STDC__ */
		break;
	    case xxx2(RCT, OVP):
	    case xxx3(SAM, RCT, OVP):
#if !defined (__STDC__)
		(*f)(&rects[0], ovw, ovh);
#else /* __STDC__ */
		(* (void (*)(Rect *, INTEGER, INTEGER)) f)
							 (&rects[0], ovw, ovh);
#endif /* __STDC__ */
		break;
	    case xxx2(WRD, WRD):
#if !defined (__STDC__)
		(*f)(words[0], words[1]);
#else /* __STDC__ */
		(* (void (*)(INTEGER, INTEGER)) f)(words[0], words[1]);
#endif /* __STDC__ */
		break;
	    case xxx3(BYT, BYT, TXT):
#if !defined (__STDC__)
		(*f)(SE(bytes[0]), SE(bytes[1]), ourstring, &txtpoint);
#else /* __STDC__ */
		(* (void (*)(Byte, Byte, StringPtr, Point *)) f)
				 (SE(bytes[0]), SE(bytes[1]), ourstring, &txtpoint);
#endif /* __STDC__ */
		break;
	    case xxx3(PNT, BYT, BYT):
#if !defined (__STDC__)
		(*f)(points[0], SE(bytes[0]), SE(bytes[1]));
#else /* __STDC__ */
		(* (void (*)(Point, Byte, Byte)) f)
				       (points[0], SE(bytes[0]), SE(bytes[1]));
#endif /* __STDC__ */
		break;
	    case xxx3(RCT, WRD, WRD):
	    case xxx4(SAM, RCT, WRD, WRD):
	    case xxx6(WRD, WRD, SAM, RCT, WRD, WRD):
#if !defined (__STDC__)
		(*f)(&rects[0], words[0], words[1]);
#else /* __STDC__ */
		(* (void (*)(Rect *, INTEGER, INTEGER)) f)
					       (&rects[0], words[0], words[1]);
#endif /* __STDC__ */
		break;
	    case xxx2(WRD, DAT):
#if !defined (__STDC__)
		(*f)(words[0], hsize, hand);
#else /* __STDC__ */
		(* (void (*)(INTEGER, INTEGER, Handle)) f)
						       (words[0], hsize, hand);
#endif /* __STDC__ */
		break;
	    default:
		gui_assert(0);
		break;
	    }
	}
    }
    
    HSetState((Handle) pic, state);
    if (hand)
	DisposHandle(hand);
    if (CGrafPort_p (the_port))
      {
 	DisposPixPat (CPORT_PEN_PIXPAT (the_cport));
 	DisposPixPat (CPORT_BK_PIXPAT (the_cport));
 	DisposPixPat (CPORT_FILL_PIXPAT (the_cport));
      }

    do_textend (); /* in case some clowns included a textbegin without
		      a textend */
		      
    DisposeRgn (PORT_CLIP_REGION (the_port));
    DisposeRgn (PORT_VIS_REGION (the_port));

    *the_port = saveport;
    PORT_BOUNDS (the_port) = saveportbounds;
    DisposeRgn (saveclip);
    saveclip = NULL;
    end_assoc ();
    SetFractEnable (saveFractEnable);
    SetFScaleDisable (saveFScaleDisable);
    HiliteRGB = saveHiliteRGB;
    HiliteMode = saveHiliteMode;
}

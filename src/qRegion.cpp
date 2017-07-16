/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qRegion[] =
	    "$Id: qRegion.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/region.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/safe_alloca.h"

using namespace Executor;
using namespace ByteSwap;

#undef ALLOCABEGIN
#define ALLOCABEGIN     SAFE_DECL();
#undef ALLOCA
#define ALLOCA(n)       SAFE_alloca(n)

#if !defined (NDEBUG)

#define RGN_SLAM(rgn)	(ROMlib_sledgehammer_rgn (rgn))

static void
ROMlib_sledgehammer_rgn (RgnHandle rgn)
{
  boolean_t special_rgn_p;
  int16 size;
  int x, y;
  int16 *ip, *start_ip;
  
  gui_assert (rgn);
  
  if (RGN_SMALL_P (rgn))
    return;
  
  special_rgn_p = RGN_SPECIAL_P (rgn);
  size = RGN_SIZE (rgn);
  
#if 0
  /* the line drawing code is as slimy as can be, and creates a bogo
     handle that points to `alloca'ed data */
  gui_assert (size <= GetHandleSize ((Handle) rgn));
#endif
  
  start_ip = ip = RGN_DATA (rgn);
  
  /* #### verify that `y's are increasing also */
  for (y = BigEndianValue (*ip++); y != RGN_STOP; y = BigEndianValue (*ip++))
    {
      /* #### verify that there are an even numbers of `x's */
      if (special_rgn_p)
	{
	  int32 prev_x = INT32_MIN;
	  
	  for (x = *ip++; x != RGN_STOP; x = *ip++)
	    gui_assert (x > prev_x);
	}
      else
	{
	  int32 prev_x = INT32_MIN;
	  
	  for (x = BigEndianValue (*ip++); x != RGN_STOP; x = BigEndianValue (*ip++))
	    gui_assert (x > prev_x);
	}
    }
  gui_assert (ip - start_ip == (size - 10) / (int) sizeof *ip);
}

#else

#define RGN_SLAM(rgn)

#endif

P0 (PUBLIC pascal trap, RgnHandle, NewRgn)
{
  RgnHandle h;
  
  h = (RgnHandle) NewHandleClear (RGN_SMALL_SIZE);
  RGN_SET_SMALL (h);
  return h;
}

P0(PUBLIC pascal trap, void, OpenRgn)
{
  RgnHandle rh;
  
  rh = (RgnHandle) NewHandleClear (RGN_SMALL_SIZE + sizeof (INTEGER));
  
  RGN_SET_SIZE_AND_SPECIAL (rh, RGN_SMALL_SIZE + sizeof (INTEGER), FALSE);
  /* sentinel */
  (RGN_DATA (rh))[0] = RGN_STOP_X;
  
  PORT_REGION_SAVE_X (thePort) = (Handle) RM (rh);
  HidePen();
}

static boolean_t
rgn_is_rect_p (const RgnHandle rgnh)
{
  const RgnPtr rgnp = STARH (rgnh);
  const INTEGER *ip;
  
  ip = RGNP_DATA (rgnp);
  return (! RGNP_SPECIAL_P (rgnp)
	  && RGNP_SIZE_X (rgnp) == CWC (RGN_SMALL_SIZE + 9 * sizeof *ip)
	  && ip[1] == ip[5]
	  && ip[2] == ip[6]);
}

/* ROMlib_sizergn: crawl through a region and set bbox and size fields */

void
Executor::ROMlib_sizergn (RgnHandle rh, boolean_t special_p) /* INTERNAL */
{
  register INTEGER *ip, i, left = RGN_STOP, right = -RGN_STOP, y;
  Size rs;

  ip = RGN_DATA (rh);
  RGN_BBOX (rh).top = *ip;
  y = INT16_MIN;
  if (special_p)
    {
      while (*ip != RGN_STOP_X)
	{
	  y = BigEndianValue (*ip++);
	  while ((i = *ip++) != RGN_STOP)
	    {
	      if (i < left)	/* testing every element is a waste here */
		left = i;
	      if (i > right)	/* and here. */
		right = i;
	    }
	}
    }
  else
    {
      while (*ip != RGN_STOP_X)
	{
	  y = BigEndianValue (*ip++);
	  while ((i = BigEndianValue (*ip++)) != RGN_STOP)
	    {
	      if (i < left)	/* testing every element is a waste here */
		left = i;
	      if (i > right)	/* and here. */
		right = i;
	    }
	}
    }
  if (y == INT16_MIN)
    {
      SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
      RGN_SET_SMALL (rh);
      RECT_ZERO (&RGN_BBOX (rh));
    }
  else
    {
      rs = (char *) ++ip - (char *) STARH (rh);
      RGN_SET_SIZE_AND_SPECIAL (rh, rs, FALSE);
      if (rgn_is_rect_p (rh))
	{
	  SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
	  RGN_SET_SMALL (rh);
	}
      else if (rs == RGN_SMALL_SIZE + sizeof (INTEGER))
	{
	  SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
	  RGN_SET_SMALL (rh);
	  HASSIGN_1
	    (rh,
	     rgnBBox.top, CWC (0));
	  left = right = y = 0;
	}
      else
	SetHandleSize ((Handle) rh, rs);
      
      HASSIGN_3
	(rh,
	 rgnBBox.left,   BigEndianValue (left),
	 rgnBBox.bottom, BigEndianValue (y),
	 rgnBBox.right,  BigEndianValue (right));
    }
}

P2 (PUBLIC pascal trap, void, CopyRgn, RgnHandle, s, RgnHandle, d)
{
  Size size;
  
  if (s == d)
    return;
  size = RGN_SIZE (s);
  ReallocHandle((Handle) d, size);
  memcpy ((Ptr) STARH (d), (Ptr) STARH (s), size);
}

P1(PUBLIC pascal trap, void, CloseRgn, RgnHandle, rh)
{
  RgnHandle rgn_save = (RgnHandle) PORT_REGION_SAVE (thePort);
  
  if (RGN_SIZE_X (rgn_save) == CWC (RGN_SMALL_SIZE + sizeof (INTEGER))
      || rgn_is_rect_p (rgn_save))
    RGN_SET_SMALL (rgn_save);
  
  ROMlib_installhandle (PORT_REGION_SAVE (thePort), (Handle) rh);
  SetHandleSize ((Handle) rh, RGN_SIZE (rh));
  PORT_REGION_SAVE_X (thePort) = (Handle)RM (NULL);
  ShowPen ();
}

P1 (PUBLIC pascal trap, void, DisposeRgn, RgnHandle, rh)
{
  DisposHandle ((Handle) rh);
}

P1 (PUBLIC pascal trap, void, SetEmptyRgn, RgnHandle, rh)
{
  SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
  RGN_SET_SMALL (rh);
  RECT_ZERO (&RGN_BBOX (rh));
}

P5 (PUBLIC pascal trap, void, SetRectRgn, RgnHandle, rh, INTEGER, left,
    INTEGER, top, INTEGER, right, INTEGER, bottom)
{
  SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
  SetRect (&RGN_BBOX (rh), left, top, right, bottom);
  RGN_SET_SMALL (rh);
}

P2 (PUBLIC pascal trap, void, RectRgn, RgnHandle, rh, Rect *, rect)
{
  Rect r;

  /* suck the rect into a local before we do the `SetHandleSize ()'
     because `rect' may point into an unlocked handle (that shouldn't
     actually ever happen, but it is better to be safe than sorry) */
  r = *rect;
  SetHandleSize ((Handle) rh, RGN_SMALL_SIZE);
  RGN_SET_SMALL (rh);
  RGN_BBOX (rh) = r;
}

/* IM is wrong... not based on offsets */
P3 (PUBLIC pascal trap, void, OffsetRgn, RgnHandle, rh,
    INTEGER, dh, INTEGER, dv)
{
  if (dh || dv)
    {
      INTEGER *ip, *ep;
      RgnPtr rp;
      
      rp = STARH (rh);
      OffsetRect (&RGNP_BBOX (rp), dh, dv);
      for (ip = RGNP_DATA (rp),
	     ep = (INTEGER *) ((char *) ip + RGNP_SIZE (rp)) - 6;
	   ip < ep; ip++)
	{
	  *ip = BigEndianValue (BigEndianValue (*ip) + (dv));
	  ++ip;
	  do
	    {
	      *ip = BigEndianValue (BigEndianValue (*ip) + (dh));
	      ++ip;
	      *ip = BigEndianValue (BigEndianValue (*ip) + (dh));
	      ++ip;
	    }
	  while (*ip != RGN_STOP_X);
	}
    }
}

#define NHPAIR 1024

P2 (PUBLIC pascal trap, BOOLEAN, PtInRgn, Point, p, RgnHandle, rh)
{
  if (!PtInRect (p, &RGN_BBOX (rh)))
    return FALSE;
  if (RGN_SMALL_P (rh))
    return TRUE;
  else
    {
      INTEGER *ipe, *op;
      INTEGER *ipr;
      INTEGER v, in;
      /* double buffered (really [2][NHPAIR]) */
      INTEGER endpoints[2 * NHPAIR];
      
      ipr = RGN_DATA (rh);
      *endpoints = RGN_STOP;
      ipe = endpoints;
      op = endpoints + NHPAIR;
      while ((v = BigEndianValue (*ipr++)) != RGN_STOP)
	{
	  if (v > p.v)
	    {
	      in = FALSE;
	      for (op = ipe; *op++ <= p.h; in = !in)
		;
	      return in;
	    }
	  while (*ipr != RGN_STOP_X || *ipe != RGN_STOP)
	    {
	      if (BigEndianValue (*ipr) < *ipe)
		*op++ = BigEndianValue (*ipr++);
	      else if (*ipe < BigEndianValue (*ipr))
		*op++ = *ipe++;
	      else
		{
		  ipr++;
		  ipe++;
		}
	    }
	  ipr++;
	  ipe++;
	  *op = RGN_STOP;
	  if (op >= endpoints + NHPAIR)
	    {
	      ipe = endpoints + NHPAIR;
	      op = endpoints;
	    }
	  else
	    {
	      ipe = endpoints;
	      op = endpoints + NHPAIR;
	    }
	}
    }
  return FALSE;
}

/*
 * NOTE: ipr points to native memory and hence has to have dereferences
 *	 swapped.  Neither mergebuf, nor freebuf have this requirement.
 */

#define merge(ipr, mergebuf, freebuf)					\
{									\
  INTEGER *ipe, *op;							\
  									\
  ipe = mergebuf;							\
  op = freebuf;								\
  while (*ipr != RGN_STOP_X || *ipe != RGN_STOP)			\
    {									\
      if (BigEndianValue (*ipr) < *ipe)						\
	*op++ = BigEndianValue (*ipr++);						\
      else if (*ipe < BigEndianValue (*ipr))					\
	*op++ = *ipe++;							\
      else								\
	{								\
	  ipr++;							\
	  ipe++;							\
	}								\
    }									\
  ipr++;								\
  ipe++;								\
  *op++ = RGN_STOP;							\
  *op = RGN_STOP;							\
  op = freebuf;								\
  freebuf = mergebuf;							\
  mergebuf = op;							\
									\
  /* if (mergebuf[0] == mergebuf[1] && mergebuf[0] != RGN_STOP)		\
       printf("m[0] = %d, m[1] = %d, f[0] = %d, f[1] = %d\n",		\
              mergebuf[0], mergebuf[1], freebuf[0], freebuf[1]); */	\
}

#define nextline(ipr, mergebuf)	\
{				\
    mergebuf = ipr;		\
    while (*ipr++ != RGN_STOP)	\
	;			\
}

/*
 * NOTE: I *think* outputrgn has inputs in native space, but should
 *	 output to synthetic space.
 */

#define outputrgn(vx, cur, new, out)		\
{						\
  INTEGER *ipe, *ipr;				\
  LONGINT hold;					\
						\
  ipe = cur;					\
  ipr = new;					\
  *out++ = BigEndianValue (vx);				\
  hold = (LONGINT) (long) out;			\
  while (*ipr != RGN_STOP || *ipe != RGN_STOP)	\
    {						\
      if (*ipr < *ipe)				\
	*out++ = BigEndianValue (*ipr++);			\
      else if (*ipe < *ipr)			\
	*out++ = BigEndianValue (*ipe++);			\
      else					\
	{					\
	  ipr++;				\
	  ipe++;				\
	}					\
    }						\
  if (hold == (LONGINT) (long) out)		\
    --out;					\
  else						\
    {						\
      *out++ = BigEndianValue (RGN_STOP);			\
    }						\
  ipe = cur;					\
  cur = new;					\
  new = ipe;					\
}

#define newsource                    { sstart = *sp1++; sstop = *sp1++; }
#define newdest                      { dstart = *sp2++; dstop = *sp2++; }
#define includ(start, stop, outp)    { *outp++ = start; *outp++ = stop; }
    
#define sect(src1, src2, outptr)			\
{							\
   INTEGER sstart, dstart, sstop, dstop;		\
   INTEGER *sp1, *sp2, *outp;				\
							\
    sp1 = src1;						\
    sp2 = src2;						\
    outp = outptr;					\
    newsource						\
    newdest						\
    while (sstart != RGN_STOP && dstart != RGN_STOP)	\
        if (sstop <= dstart)				\
            newsource					\
        else if (dstop <= sstart)			\
            newdest					\
        else if (sstart <= dstart) {			\
            if (sstop < dstop) {			\
                includ(dstart, sstop, outp)		\
                dstart = sstop;				\
                newsource				\
            } else {					\
                includ(dstart, dstop, outp)		\
                sstart = dstop;				\
                newdest					\
            }						\
        } else {					\
            if (dstop < sstop) {			\
                includ(sstart, dstop, outp)		\
                sstart = dstop;				\
                newdest					\
            } else {					\
                includ(sstart, sstop, outp)		\
                dstart = sstop;				\
                newsource				\
            }						\
        }						\
    *outp = RGN_STOP;					\
}

/*
 * NOTE: here we're creating a "special" region that is really a set
 *	 of start stop pairs that are in native endianness.
 */
    
#define sectline(src1, src2, outp, y)			\
{							\
    INTEGER sstart, dstart, sstop, dstop;		\
    INTEGER *sp1, *sp2, *outptr;			\
							\
    sp1 = src1;						\
    sp2 = src2;						\
    newsource						\
    newdest						\
    *outp++ = BigEndianValue(y);					\
    outptr = outp;					\
    while (sstart != RGN_STOP && dstart != RGN_STOP) {	\
        if (sstop <= dstart)				\
            newsource					\
        else if (dstop <= sstart)			\
            newdest					\
        else if (sstart <= dstart) {			\
            if (sstop < dstop) {			\
                includ(dstart, sstop, outp)		\
                dstart = sstop;				\
                newsource				\
            } else {					\
                includ(dstart, dstop, outp)		\
                sstart = dstop;				\
                newdest					\
            }						\
        } else {					\
            if (dstop < sstop) {			\
                includ(sstart, dstop, outp)		\
                sstart = dstop;				\
                newdest					\
            } else {					\
                includ(sstart, sstop, outp)		\
                dstart = sstop;				\
                newsource				\
            }						\
        }						\
    }							\
    if (wehavepairs || outp > outptr) {			\
	*outp++ = RGN_STOP;				\
	wehavepairs = TRUE;				\
    } else {						\
	outp--;						\
	wehavepairs = FALSE;				\
    }							\
}

#define uunion(src1, src2, outptr)			\
{							\
    INTEGER sstart, dstart, sstop, dstop;		\
    INTEGER *sp1, *sp2, *outp;				\
							\
    sp1 = src1;						\
    sp2 = src2;						\
    outp = outptr;					\
    newsource						\
    newdest						\
    while (sstart != RGN_STOP || dstart != RGN_STOP)	\
        if (sstart <= dstart) {				\
            if (sstop < dstart) {			\
                includ(sstart, sstop, outp)		\
                newsource				\
            } else if (sstop <= dstop) {		\
		dstart = sstart;			\
		newsource				\
	    } else					\
                newdest					\
        } else {					\
            if (dstop < sstart) {			\
                includ(dstart, dstop, outp)		\
                newdest					\
            } else if (dstop <= sstop) {		\
		sstart = dstart;			\
		newdest					\
	    } else					\
		newsource				\
	}						\
    *outp = RGN_STOP;					\
}
    
#define diff(src1, src2, outptr)			\
{							\
    INTEGER sstart, dstart, sstop, dstop;		\
    INTEGER *sp1, *sp2, *outp;				\
							\
    sp1 = src1;						\
    sp2 = src2;						\
    outp = outptr;					\
    newsource						\
    newdest						\
    while (sstart != RGN_STOP || dstart != RGN_STOP) {	\
        if (sstop <= dstart) {				\
            includ(sstart, sstop, outp)			\
            newsource					\
        } else if (dstop <= sstart)			\
            newdest					\
        else {						\
            if (sstart < dstart)			\
                includ(sstart, dstart, outp)		\
            if (sstop < dstop)				\
                newsource				\
            else if (sstop == dstop) {			\
                newsource				\
                newdest					\
            } else {					\
                sstart = dstop;				\
                newdest					\
            }						\
        }						\
    }							\
    *outp = RGN_STOP;					\
}

#if !defined (NDEBUG)

namespace Executor {
  PRIVATE void assertincreasing(INTEGER * ip);
}

A1(PRIVATE, void, assertincreasing, INTEGER *, ip)
{
    LONGINT lastx = -327680;
    while (*ip != RGN_STOP) {
	gui_assert(lastx < *ip);
	lastx = *ip++;
    }
}
#endif /* NDEBUG */

namespace Executor {
  PRIVATE void sectbinop(RgnHandle srcrgn1, RgnHandle srcrgn2,
	 RgnHandle dstrgn);
  typedef enum {
	sectop,
	unionop,
	diffop
  } optype;
  

  
  PRIVATE void binop(optype op, RgnHandle srcrgn1, RgnHandle srcrgn2,
					 RgnHandle dstrgn);
  PRIVATE LONGINT comparex(char * cp1, char * cp2);
  PRIVATE LONGINT comparey(char * cp1, char * cp2);
  PRIVATE void ptorh(INTEGER *p, RgnHandle rh);
}

A3(PRIVATE, void, sectbinop, RgnHandle, srcrgn1, RgnHandle, srcrgn2,
							    RgnHandle, dstrgn)
{
    /* note that the buffers will not be restricted to the endpoints
       that their name implies.  The pointers will always point to
       the buffer containing the most current set of endpoints */
       
    INTEGER src1endpoints[NHPAIR], *src1ep = src1endpoints;
    INTEGER src2endpoints[NHPAIR], *src2ep = src2endpoints;
    INTEGER sectsegendpoints[NHPAIR], *sectsegep = sectsegendpoints;
    INTEGER sectcurendpoints[NHPAIR], *sectcurep = sectcurendpoints;
    INTEGER freeendpoints[NHPAIR], *freeep = freeendpoints;
    INTEGER *ipr1, *ipr2;
    INTEGER *temppoints, *tptr;
    register INTEGER v1, v2, vx;
    INTEGER r1[9], r2[9];
    Rect *rp;
    INTEGER nspecial;
    RgnHandle exchrgn;
    BOOLEAN wehavepairs;
    ALLOCABEGIN

    /*
     * Some performance enhancements could be put here...
     * if we are secting and the rgnBox's don't sect ... etc.
     */
    if (RGN_SPECIAL_P (srcrgn1)) {
	if (RGN_SPECIAL_P (srcrgn2))
	    nspecial = 2;
	else
	    nspecial = 1;
    } else {
        if (RGN_SPECIAL_P (srcrgn2)) {
	    nspecial = 1;
	    exchrgn  = srcrgn1;
	    srcrgn1  = srcrgn2;
	    srcrgn2  = exchrgn;
	} else
	    nspecial = 0;
    }
    tptr = temppoints = ((INTEGER *)
			 ALLOCA (2 * ((RGN_SIZE (srcrgn1))
				      + RGN_SIZE (srcrgn2)
				      + 18 * sizeof (INTEGER))));
    /* todo ... look over these */
    if (RGN_SMALL_P (srcrgn1)) {
        rp = &RGN_BBOX (srcrgn1);
        r1[0] = rp->top;
        r1[1] = rp->left;
        r1[2] = rp->right  != RGN_STOP_X ? rp->right  : CWC(RGN_STOP - 1);
        r1[3] = RGN_STOP_X;
        r1[4] = rp->bottom != RGN_STOP_X ? rp->bottom : CWC(RGN_STOP - 1);
        r1[5] = rp->left;
        r1[6] = rp->right  != RGN_STOP_X ? rp->right  : CWC(RGN_STOP - 1);
        r1[7] = RGN_STOP_X;
        r1[8] = RGN_STOP_X;
        ipr1 = r1;
    } else
        ipr1 = RGN_DATA (srcrgn1);
    if (RGN_SMALL_P (srcrgn2)) {
        rp = &RGN_BBOX (srcrgn2);
        r2[0] = rp->top;
        r2[1] = rp->left;
        r2[2] = rp->right  != RGN_STOP_X ? rp->right  : CWC(RGN_STOP - 1);
        r2[3] = RGN_STOP_X;
        r2[4] = rp->bottom != RGN_STOP_X ? rp->bottom : CWC(RGN_STOP - 1);
        r2[5] = rp->left;
        r2[6] = rp->right  != RGN_STOP_X ? rp->right  : CWC(RGN_STOP - 1);
        r2[7] = RGN_STOP_X;
        r2[8] = RGN_STOP_X;
        ipr2 = r2;
    } else
        ipr2 = RGN_DATA (srcrgn2);
    *src1ep = *src2ep = *sectsegep = *sectcurep = *freeep = 
      *(src1ep+1) = *(src2ep+1) = RGN_STOP;

    v1 = BigEndianValue(*ipr1++);
    v2 = BigEndianValue(*ipr2++);
    wehavepairs = FALSE;	/* whether or not scan lines have stuff */
    switch (nspecial) {
    case 0:
	while (v1 != RGN_STOP && v2 != RGN_STOP) {
	    if (v1 < v2) {
		merge(ipr1, src1ep, freeep) /* no semi ... macro */
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
	    } else if (v2 < v1) {
		merge(ipr2, src2ep, freeep)
		vx = v2;
		v2 = BigEndianValue(*ipr2++);
	    } else {    /* equal */
		merge(ipr1, src1ep, freeep)
		merge(ipr2, src2ep, freeep)
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
		v2 = BigEndianValue(*ipr2++);
	    }
	    sect(src1ep, src2ep, sectsegep)
	    outputrgn(vx, sectcurep, sectsegep, tptr);
	}
	break;
    case 1:
	vx = -32768;
	while (v1 != RGN_STOP && v2 != RGN_STOP) {
	    if (v1 < v2) {
		nextline(ipr1, src1ep) /* no semi ... macro */
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
	    } else if (v2 < v1) {
		merge(ipr2, src2ep, freeep)
		vx = v2;
		v2 = BigEndianValue(*ipr2++);
	    } else {    /* equal */
		nextline(ipr1, src1ep)
		merge(ipr2, src2ep, freeep)
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
		v2 = BigEndianValue(*ipr2++);
	    }
#if !defined (NDEBUG)
	    assertincreasing(src1ep);
	    assertincreasing(src2ep);
#endif /* NDEBUG */
	    sectline(src1ep, src2ep, tptr, vx)
	}
	break;
    case 2:
	vx = -32768;
	while (v1 != RGN_STOP && v2 != RGN_STOP) {
	    if (v1 < v2) {
		nextline(ipr1, src1ep) /* no semi ... macro */
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
	    } else if (v2 < v1) {
		nextline(ipr2, src2ep)
		vx = v2;
		v2 = BigEndianValue(*ipr2++);
	    } else {    /* equal */
		nextline(ipr1, src1ep)
		nextline(ipr2, src2ep)
		vx = v1;
		v1 = BigEndianValue(*ipr1++);
		v2 = BigEndianValue(*ipr2++);
	    }
	    sectline(src1ep, src2ep, tptr, vx)
	}
	*tptr++ = vx;
	*tptr++ = RGN_STOP;
	break;
    }
    *tptr++ = RGN_STOP_X;
    gui_assert(sizeof(INTEGER) * (tptr - temppoints) <=
       2 * ((Hx(srcrgn1, rgnSize)&0x7FFF) + (Hx(srcrgn2, rgnSize) & 0x7FFF) +
						        18 * sizeof(INTEGER)));

    {
      int dst_rgn_size = (RGN_SMALL_SIZE
			  + sizeof(INTEGER) * (tptr - temppoints));
      RGN_SET_SIZE_AND_SPECIAL (dstrgn, dst_rgn_size, FALSE);
      /* TODO fix rgnBBox here */
      ReallocHandle ((Handle) dstrgn, dst_rgn_size);
    }

    memmove (RGN_DATA (dstrgn), temppoints,
	     (tptr - temppoints) * sizeof *temppoints);
    ROMlib_sizergn (dstrgn, nspecial > 0);	/* could do this while copying... */
    if (nspecial > 0)
      RGN_SET_SPECIAL (dstrgn, TRUE);
    ASSERT_SAFE (temppoints);
    ALLOCAEND
}
    
A4(PRIVATE, void, binop, optype, op, RgnHandle, srcrgn1, RgnHandle, srcrgn2,
							    RgnHandle, dstrgn)
{
    /* note that the buffers will not be restricted to the endpoints
       that their name implies.  The pointers will always point to
       the buffer containing the most current set of endpoints */
       
    INTEGER src1endpoints[NHPAIR], *src1ep = src1endpoints;
    INTEGER src2endpoints[NHPAIR], *src2ep = src2endpoints;
    INTEGER sectsegendpoints[NHPAIR], *sectsegep = sectsegendpoints;
    INTEGER sectcurendpoints[NHPAIR], *sectcurep = sectcurendpoints;
    INTEGER freeendpoints[NHPAIR], *freeep = freeendpoints;
    INTEGER *ipr1, *ipr2;
    INTEGER *temppoints, *tptr;
    register INTEGER v1, v2, vx;
    INTEGER r1[9], r2[9];
    Rect *rp;
    ALLOCABEGIN

    /*
     * Some performance enhancements could be put here...
     * if we are secting and the rgnBox's don't sect ... etc.
     */
    tptr = temppoints = (INTEGER *) ALLOCA((Size)2 * (Hx(srcrgn1, rgnSize) +
				  Hx(srcrgn2, rgnSize) + 18 * sizeof(INTEGER)));
                                          /* todo ... look over these */
    if (RGN_SMALL_P (srcrgn1)) {
        rp = &(RGN_BBOX (srcrgn1));
        r1[0] = rp->top;
        r1[1] = rp->left;
        r1[2] = rp->right  != RGN_STOP_X ? rp->right  : BigEndianValue(RGN_STOP - 1);
        r1[3] = RGN_STOP_X;
        r1[4] = rp->bottom != RGN_STOP_X ? rp->bottom : BigEndianValue(RGN_STOP - 1);
        r1[5] = rp->left;
        r1[6] = rp->right  != RGN_STOP_X ? rp->right  : BigEndianValue(RGN_STOP - 1);
        r1[7] = RGN_STOP_X;
        r1[8] = RGN_STOP_X;
        ipr1 = r1;
    } else
        ipr1 = RGN_DATA (srcrgn1);
    if (RGN_SMALL_P (srcrgn2)) {
        rp = &(RGN_BBOX (srcrgn2));
        r2[0] = rp->top;
        r2[1] = rp->left;
        r2[2] = rp->right  != RGN_STOP ? rp->right  : BigEndianValue(RGN_STOP - 1);
        r2[3] = RGN_STOP_X;
        r2[4] = rp->bottom != RGN_STOP ? rp->bottom : BigEndianValue(RGN_STOP - 1);
        r2[5] = rp->left;
        r2[6] = rp->right  != RGN_STOP ? rp->right  : BigEndianValue(RGN_STOP - 1);
        r2[7] = RGN_STOP_X;
        r2[8] = RGN_STOP_X;
        ipr2 = r2;
    } else
        ipr2 = RGN_DATA (srcrgn2);
    *src1ep = *src2ep = *sectsegep = *sectcurep = *freeep = 
    *(src1ep+1) = *(src2ep+1) =  RGN_STOP;

    v1 = BigEndianValue(*ipr1++);
    v2 = BigEndianValue(*ipr2++);
    while (v1 != RGN_STOP || v2 != RGN_STOP) {
        if (v1 < v2) {
            merge(ipr1, src1ep, freeep) /* no semi ... macro */
            vx = v1;
            v1 = BigEndianValue(*ipr1++);
        } else if (v2 < v1) {
            merge(ipr2, src2ep, freeep)
            vx = v2;
            v2 = BigEndianValue(*ipr2++);
        } else {    /* equal */
            merge(ipr1, src1ep, freeep)
            merge(ipr2, src2ep, freeep)
            vx = v1;
            v1 = BigEndianValue(*ipr1++);
            v2 = BigEndianValue(*ipr2++);
        }
        switch (op) {
        case sectop:
            sect(src1ep, src2ep, sectsegep)
            break;
        case diffop:
            diff(src1ep, src2ep, sectsegep)
            break;
        case unionop:
            uunion(src1ep, src2ep, sectsegep)
            break;
        }
        outputrgn(vx, sectcurep, sectsegep, tptr);
    }
    *tptr++ = RGN_STOP_X;
    gui_assert(sizeof(INTEGER) * (tptr - temppoints) <=
       2 * (Hx(srcrgn1, rgnSize) + Hx(srcrgn2, rgnSize) + 18 * sizeof(INTEGER)));
    HxX(dstrgn, rgnSize) = BigEndianValue(RGN_SMALL_SIZE + sizeof(INTEGER) * (tptr - temppoints));
    /* TODO fix rgnBBox here */
    ReallocHandle((Handle) dstrgn,
		  RGN_SMALL_SIZE + sizeof(INTEGER) * (tptr - temppoints));
    { register INTEGER *ip, *op;
	ip = temppoints;
	op = (INTEGER *) STARH(dstrgn) + 5;
	while (ip != tptr)
	    *op++ = *ip++;
	ROMlib_sizergn(dstrgn, FALSE);	/* could do this while copying... */
    }
    ASSERT_SAFE (temppoints);
    ALLOCAEND
}


void
Executor::nonspecial_rgn_to_special_rgn (const INTEGER *src, INTEGER *dst)
{
  static const INTEGER empty_row[2] = { 0, RGN_STOP };
  const INTEGER *prev;
  
  /* Each scanline gets XOR'd with the previous one.  We proceed until we
   * hit a RGN_STOP_X y value.
   */
  for (prev = empty_row; (dst[0] = src[0]) != RGN_STOP_X; )
    {
      int srcx, prevx;
      INTEGER *next_prev;
      
      /* Fetch the first X on this new scanline. */
      srcx = BigEndianValue (src[1]);
      if (srcx == RGN_STOP)
	{
	  /* The row with which to XOR is empty, so just extend the
	   * current row.
	   */
	  src += 2;
	  continue;
	}

      next_prev = dst;

      /* Fetch the first X on the previous scanline.  If the previous
       * scanline was empty, then XORing just gives us the scanline
       * from src.
       */
      prevx = prev[1];
      if (prevx == RGN_STOP)
	{
	  for (; (dst[1] = BigEndianValue (src[1])) != RGN_STOP; src += 2, dst += 2)
	    dst[2] = BigEndianValue (src[2]);
	  src += 2;
	  dst += 2;
	  prev = next_prev;
	  continue;
	}

      /* Neither scanline is empty.  This is the tricky case.  Since
       * we are XORing two scanlines of start/stop X pairs, we just
       * merge the two lists of X coordinates into one sorted array.
       * If you draw two scanlines, one above the other, you'll see
       * that all of the X coordinates just "drop down" to the
       * resulting scanline.  The only exception is when both
       * scanlines have the same X value; in that case, it
       * "disappears".
       */
      src += 2;
      prev += 2;
      dst++;

      while (1)
	{
	  if (srcx < prevx)
	    {
	      *dst++ = srcx;
	      srcx = BigEndianValue (*src++);
	      if (srcx == RGN_STOP)
		goto read_prev_only;
	    }
	  else if (srcx > prevx)
	    {
	      *dst++ = prevx;
	      prevx = *prev++;
	      if (prevx == RGN_STOP)
		goto read_src_only;
	    }
	  else /* srcx == prevx */
	    {
	      srcx = BigEndianValue (*src++);
	      prevx = *prev++;
	      if (srcx == RGN_STOP)
		goto read_prev_only;
	      if (prevx == RGN_STOP)
		goto read_src_only;
	    }
	}

      /* We've run out of data from "src", so just copy the rest of the
       * prev scanline.
       */
    read_prev_only:
      while (prevx != RGN_STOP)
	{
	  *dst++ = prevx;
	  prevx = *prev++;
	}
      goto do_next;

      /* We've run out of data from "prev", so just copy the rest of the
       * src scanline.
       */
    read_src_only:
      while (srcx != RGN_STOP)
	{
	  *dst++ = srcx;
	  srcx = BigEndianValue (*src++);
	}
      
    do_next:
      *dst++ = RGN_STOP;
      prev = next_prev;
    }
}


/*
 * Here's how the little bugger works:
 *  Three macros are needed to be defined so that some boilerplate
 *  can expand into rhtopandinseth and inseth
 *  the pairs will always be kept as (y, x) so there need to be
 *  two different NEXTPAIR routines and comparison routines...
 */

static INTEGER npairs;

#define DECL void rhtopandinseth(RgnHandle rh, INTEGER *p, register INTEGER dh)

#define STATEDECL SignedByte state;
#define SETIO ip = &HxX(rh, rgnSize) + 5; op = p; y = BigEndianValue(*ip++); npairs = 0; \
					      state = HGetState((Handle) rh); \
							     HLock((Handle) rh)
#define NEXTPAIR (x = BigEndianValue(*ip++)) == RGN_STOP ? (y = BigEndianValue(*ip++), 0) : 1
#define INCLXY(x, y) *op++ = y, *op++ = x, npairs++
#define UNSETIO HSetState((Handle) rh, state); INCLXY(RGN_STOP, RGN_STOP)

#include "hintemplate.h"

#define DECL void hinset(INTEGER *p, INTEGER dh)

#define STATEDECL
#define SETIO ip = p; op = p; y = *(ip+1); npairs = 0
#define NEXTPAIR y == *(ip+1) ? (x = *ip++, ip++, 1) : (y = *(ip+1), 0)
#define INCLXY(x, y) *op++ = x, *op++ = y, npairs++
#define UNSETIO

#include "hintemplate.h"

static int comparex(const void* cp1, const void* cp2)
{
  return Executor::comparex((char*)cp1, (char*)cp2);
}

static int comparey(const void* cp1, const void* cp2)
{
  return Executor::comparey((char*)cp1, (char*)cp2);
}

A2(PRIVATE, LONGINT, comparex, char *, cp1, char *, cp2)
{
    register INTEGER *p1, *p2;
    LONGINT retval;
    
    p1 = (INTEGER *) cp1 + 1;
    p2 = (INTEGER *) cp2 + 1;
    if (*p1 < *p2)
	retval = -1;
    else if (*p1 > *p2)
	retval = 1;
    else {
	p1--;
	p2--;
	if (*p1 < *p2)
	    retval = -1;
	else if (*p1 > *p2)
	    retval = 1;
	else
	    retval = 0;
    }
    return retval;
}

A2(PRIVATE, LONGINT, comparey, char *, cp1, char *, cp2)
{
    register INTEGER *p1, *p2;
    LONGINT retval;
    
    p1 = (INTEGER *) cp1;
    p2 = (INTEGER *) cp2;
    if (*p1 < *p2)
	retval = -1;
    else if (*p1 > *p2)
	retval = 1;
    else {
	p1++;
	p2++;
	if (*p1 < *p2)
	    retval = -1;
	else if (*p1 > *p2)
	    retval = 1;
	else
	    retval = 0;
    }
    return retval;
}

/*
 * BEWARE:  ptorh can trash memory... regions can grow by being inset (honest)
 */

A2(PRIVATE, void, ptorh, INTEGER *, p, RgnHandle, rh)
{
    INTEGER y, oy, *op;
    
    op = RGN_DATA (rh);
    if (npairs) {	/* decrement one 'cause of the 32767 sentinel */
	*op++ = BigEndianValue(oy = *p);
	for (;npairs; npairs -= 2) {
	    if ((y = *p++) != oy) {
		*op++ = RGN_STOP_X;
		*op++ = BigEndianValue(y);
		oy = y;
	    }
	    *op++ = BigEndianValue(*p++);
	    ++p;			/* if Cx((*ip)++ != oy) error! */
	    *op++ = BigEndianValue(*p++);
	}
    }
    *op++ = RGN_STOP_X;	/* need one or two? */
    *op++ = RGN_STOP_X;	/* need one or two? */
}

P3(PUBLIC pascal trap, void, InsetRgn, RgnHandle, rh, INTEGER, dh, INTEGER, dv)
{
    Handle h;
    INTEGER *p;
    Rect *rp;
    register Size newsize;;
    
    if (RGN_SMALL_P (rh)) {
	InsetRect(&RGN_BBOX (rh), dh, dv);
#define INSANEBUTNECESSARY
#if defined (INSANEBUTNECESSARY)
	rp = &RGN_BBOX (rh);
	if (BigEndianValue(rp->top) >= BigEndianValue(rp->bottom) || BigEndianValue(rp->left) >= BigEndianValue(rp->right))
	  RECT_ZERO (rp);
#endif /* INSANEBUTNECESSARY */
    } else {
	newsize = 4 * RGN_SIZE (rh);
	h = NewHandle(newsize);
	HLock(h);
	p = (INTEGER *) STARH(h);
	rhtopandinseth(rh, p, dh);	/* must be combined for efficiency */
	gui_assert(npairs * (int) sizeof(INTEGER) * 2 <= newsize);
	qsort(p, npairs, sizeof(INTEGER) * 2, ::comparex);
	hinset(p, dv);
	qsort(p, npairs, sizeof(INTEGER) * 2, ::comparey);
	ReallocHandle((Handle) rh, newsize);
	ptorh(p, rh);
	ROMlib_sizergn(rh, FALSE);
	gui_assert(Hx(rh, rgnSize) <= newsize);
	HUnlock(h);
	DisposHandle(h);
    }
}

static boolean_t
justone (const Rect *rp, RgnHandle rgn, RgnHandle dest)
{
  const Rect *rp2 = &RGN_BBOX (rgn);
  
  if (   BigEndianValue (rp->left) <= BigEndianValue (rp2->left)
      && BigEndianValue (rp->top) <= BigEndianValue (rp2->top)
      && BigEndianValue (rp->right) >= BigEndianValue (rp2->right)
      && BigEndianValue (rp->bottom) >= BigEndianValue (rp2->bottom))
    {
      CopyRgn (rgn, dest);
      return TRUE;
    }
  else
    return FALSE;
}

P3(PUBLIC pascal trap, void, SectRgn, RgnHandle, s1, RgnHandle, s2,
   RgnHandle, dest)
{
  Rect dummy;
  const Region *rp1, *rp2;
  
  RGN_SLAM (s1);
  RGN_SLAM (s2);

  rp1 = STARH (s1);
  rp2 = STARH (s2);
  
  if (RGNP_SMALL_P (rp1))
    {
      if (RGNP_SMALL_P (rp2))
	{
	  SectRect (&RGNP_BBOX (rp1), &RGNP_BBOX (rp2), &RGN_BBOX (dest));
	  /* #### should this set the handle size of `dest' */
	  RGN_SET_SMALL (dest);
	  return;
	}
      else
	if (justone (&RGNP_BBOX (rp1), s2, dest))
	  return;
    }
  else if (RGNP_SMALL_P (rp2))
    if (justone(&RGNP_BBOX (rp2), s1, dest))
      return;
  if (SectRect (&RGNP_BBOX (rp1), &RGNP_BBOX (rp2), &dummy))
    sectbinop (s1, s2, dest);
  else
    SetEmptyRgn (dest);
}

P3(PUBLIC pascal trap, void, UnionRgn, RgnHandle, s1, RgnHandle, s2,
							       RgnHandle, dest)
{
    if (EmptyRgn(s1))
	CopyRgn(s2, dest);
    else if (EmptyRgn(s2))
	CopyRgn(s1, dest);
    else
	binop(unionop, s1, s2, dest);
}

P3(PUBLIC pascal trap, void, DiffRgn, RgnHandle, s1, RgnHandle, s2,
							       RgnHandle, dest)
{
    if (EmptyRgn(s1) || EmptyRgn(s2))
	CopyRgn(s1, dest);
    else
	binop(diffop, s1, s2, dest);
}

P3(PUBLIC pascal trap, void, XorRgn, RgnHandle, s1, RgnHandle, s2,
							       RgnHandle, dest)
{
    INTEGER y1, y2, x1, x2;
    INTEGER *ip1, *ip2, *op;
    INTEGER left, right, bottom;
    INTEGER cnt;
    RgnHandle finalrestingplace;
    HIDDEN_RgnPtr temp2, temp3;
    ALLOCABEGIN

    /* the +36 below is necessary because small regions have size of 10
       yet they actually have an additional implied 18 bytes associated
       with them */

    if (s1 == dest || s2 == dest) {
	finalrestingplace = dest;
	dest = (RgnHandle) NewHandle((Size) Hx(s1, rgnSize) +
					    Hx(s2, rgnSize) + 36);
    } else {
	finalrestingplace = 0;
	ReallocHandle((Handle) dest, Hx(s1, rgnSize) + Hx(s2, rgnSize) + 36);
    }

    if (RGN_SMALL_P (s1)) {
	temp2.p = (RgnPtr) ALLOCA( RGN_SMALL_SIZE + 9 * sizeof(INTEGER) );
#if 0
	BlockMove(BigEndianValue(*(Ptr *) s1), (Ptr) temp2.p, RGN_SMALL_SIZE);
#else
	memcpy((Ptr) temp2.p, MR(*(Ptr *) s1), RGN_SMALL_SIZE);
#endif
	op = (INTEGER *) ((char *)temp2.p + RGN_SMALL_SIZE);
	*op++ = HxX(s1, rgnBBox.top);
	*op++ = HxX(s1, rgnBBox.left);
	*op++ = HxX(s1, rgnBBox.right);
	*op++ = RGN_STOP_X;
	*op++ = HxX(s1, rgnBBox.bottom);
	*op++ = HxX(s1, rgnBBox.left);
	*op++ = HxX(s1, rgnBBox.right);
	*op++ = RGN_STOP_X;
	*op++ = RGN_STOP_X;
	ASSERT_SAFE (temp2.p);
	temp2.p = RM(temp2.p);
	s1 = &temp2;
    }

    if (RGN_SMALL_P (s2)) {
	temp3.p = (RgnPtr) ALLOCA (RGN_SMALL_SIZE + 9 * sizeof (INTEGER));
#if 0
	BlockMove(BigEndianValue(*(Ptr *) s2), (Ptr) temp3.p, RGN_SMALL_SIZE);
#else
	memcpy((Ptr) temp3.p, STARH (s2), RGN_SMALL_SIZE);
#endif
	op = (INTEGER *) ((char *)temp3.p + RGN_SMALL_SIZE);
	*op++ = HxX(s2, rgnBBox.top);
	*op++ = HxX(s2, rgnBBox.left);
	*op++ = HxX(s2, rgnBBox.right);
	*op++ = RGN_STOP_X;
	*op++ = HxX(s2, rgnBBox.bottom);
	*op++ = HxX(s2, rgnBBox.left);
	*op++ = HxX(s2, rgnBBox.right);
	*op++ = RGN_STOP_X;
	*op++ = RGN_STOP_X;
	ASSERT_SAFE (temp3.p);	
	temp3.p = RM(temp3.p);
	s2 = &temp3;
    }

    ip1 = RGN_DATA (s1);
    ip2 = RGN_DATA (s2);
    op  = RGN_DATA (dest);
    left = RGN_STOP; right = -32768;
    bottom = -32768;
    for (y1 = BigEndianValue(*ip1++), y2 = BigEndianValue(*ip2++); y1 != RGN_STOP || y2 != RGN_STOP;) {
	if (y1 < y2) {
	    bottom = y1;
	    *op++ = BigEndianValue(y1);
	    while ((*op++ = *ip1++) != RGN_STOP_X) {
		x1 = BigEndianValue(op[-1]);
		if (x1 < left)
		    left = x1;
		if (x1 > right)
		    right = x1;
	    }
	    y1 = BigEndianValue(*ip1++);
	} else if (y2 < y1) {
	    bottom = y2;
	    *op++ = BigEndianValue(y2);
	    while ((*op++ = *ip2++) != RGN_STOP_X) {
		x2 = BigEndianValue(op[-1]);
		if (x2 < left)
		    left = x2;
		if (x2 > right)
		    right = x2;
	    }
	    y2 = BigEndianValue(*ip2++);
	} else {
	    cnt = 0;
	    for (x1 = BigEndianValue(*ip1++), x2 = BigEndianValue(*ip2++);
		 x1 != RGN_STOP || x2 != RGN_STOP;) {
		if (x1 < x2) {
		    if (!cnt) {
			bottom = y1;
			*op++ = BigEndianValue(y1);
			if (x1 < left)
			    left = x1;
		    } else if (x1 > right)
			right = x1;
		    *op++ = BigEndianValue(x1);
		    cnt++;
		    x1 = BigEndianValue(*ip1++);
		} else if (x2 < x1) {
		    if (!cnt) {
			bottom = y1;
			*op++ = BigEndianValue(y1);
			if (x2 < left)
			    left = x2;
		    } else if (x2 > right)
			right = x2;
		    *op++ = BigEndianValue(x2);
		    cnt++;
		    x2 = BigEndianValue(*ip2++);
		} else {
		    x1 = BigEndianValue(*ip1++);
		    x2 = BigEndianValue(*ip2++);
		}
	    }
	    if (cnt)
		*op++ = RGN_STOP_X;
	    y1 = BigEndianValue(*ip1++);
	    y2 = BigEndianValue(*ip2++);
	}
    }
    *op++ = RGN_STOP_X;
    HASSIGN_5 (dest,
	       rgnBBox.top,    *RGN_DATA (dest),
	       rgnBBox.left,   BigEndianValue (left),
	       rgnBBox.bottom, BigEndianValue (bottom),
	       rgnBBox.right,  BigEndianValue (right),
	       rgnSize,        BigEndianValue ((char *) op - (char *) STARH (dest)));
    if (rgn_is_rect_p (dest))
      RGN_SET_SMALL (dest);
    if (finalrestingplace) {
	ROMlib_installhandle((Handle) dest, (Handle) finalrestingplace);
	SetHandleSize((Handle) finalrestingplace,
		      RGN_SIZE (finalrestingplace));
    } else
	SetHandleSize((Handle) dest, RGN_SIZE (dest));
    ALLOCAEND
}

P2(PUBLIC pascal trap, BOOLEAN, RectInRgn, Rect *, rp,	/* IMIV-23 */
						       RgnHandle, rh)
{
    RgnHandle newrh;
    BOOLEAN retval;
    
    newrh = NewRgn();
    RectRgn(newrh, rp);
    SectRgn(newrh, rh, newrh);
    retval = !EmptyRgn(newrh);
    DisposeRgn(newrh);
    return retval;
}

P2(PUBLIC pascal trap, BOOLEAN, EqualRgn, RgnHandle, r1, RgnHandle, r2)
{
  /* Since the first field of the region is the size, this
   * will return FALSE if the sizes differ, too.
   */
  return !memcmp (STARH (r1), STARH (r2), RGN_SIZE (r1));
}

P1(PUBLIC pascal trap, BOOLEAN, EmptyRgn, RgnHandle, rh)
{
#warning What does a mac do with a NULL HANDLE here?
  BOOLEAN retval;

  retval = rh ? EmptyRect (&RGN_BBOX (rh)) : TRUE;
  return retval;
}

#if !defined (NDEBUG)
A1(PUBLIC, void, ROMlib_printrgn, RgnHandle, h)
{
  INTEGER *ip, x, y;
  INTEGER special, size, newsize;

  size = Hx (h, rgnSize);
  special = size & 0x8000;
  size &= ~0x8000;
  if (special)
    printf ("SPECIAL, ");
  printf ("size = %ld, l = %ld, t = %ld, r = %ld, b = %ld\n",
	  (long) size, (long) Hx (h, rgnBBox.left),
	  (long) Hx (h, rgnBBox.top),
	  (long) Hx (h, rgnBBox.right), (long) Hx (h, rgnBBox.bottom));

  LOCK_HANDLE_EXCURSION_1
    (h,
     {
       if (!RGN_SMALL_P (h))
	 {
	   ip = RGN_DATA (h);
	   while ((y = BigEndianValue (*ip++)) != RGN_STOP)
	     {
	       printf ("%ld:", (long) y);
	       if (special)
		 {
		   while ((x = *ip++) != RGN_STOP)
		     printf (" %ld", (long) x);
		 }
	       else
		 {
		   while ((x = BigEndianValue (*ip++)) != RGN_STOP)
		     printf (" %ld", (long) x);
		 }
	       printf (" 32767\n");
	     }
	   printf ("32767\n");
	   newsize = ((char *) ip - (char *) STARH (h));
	   if (newsize != size)
	     printf ("WARNING: computed size = %d\n", newsize);
	 }
     });
}

#endif /* NDEBUG */

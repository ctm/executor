/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qMisc[] =
		    "$Id: qMisc.c 87 2005-05-25 01:57:33Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */


#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ToolboxEvent.h"
#include "ToolboxUtil.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/glue.h"
#include "rsys/region.h"

using namespace Executor;
using namespace ByteSwap;

namespace Executor {
	static BOOLEAN EquivRect(Rect*, Rect*);
	static INTEGER fromhex(char c);
}

A2(PRIVATE, BOOLEAN, EquivRect, Rect *, rp1, Rect *, rp2)
{
    return Cx(rp1->bottom) - Cx(rp1->top)  == Cx(rp2->bottom) - Cx(rp2->top) &&
	   Cx(rp1->right)  - Cx(rp1->left) == Cx(rp2->right)  - Cx(rp2->left);
}

#define RANDSEED	((ULONGINT) randSeed)
P0(PUBLIC pascal trap, INTEGER, Random)
{
    INTEGER retval;

    RndSeed = Cx(TickCount());	/* what better? */
    if (RANDSEED >= 0x80000000)
	randSeedX = BigEndianValue((RANDSEED & 0x7FFFFFFF) + 1);
    randSeedX = BigEndianValue((RANDSEED * 16807 +
		(
		    (((RANDSEED >> 14) * 16807) +
		    (((RANDSEED & ((1<<14)-1)) * 16807) >> 14))
		>> 17)
	       ) & 0x7FFFFFFF);
    if (RANDSEED == 0x7FFFFFFF)
	randSeedX = 0;
    retval = randSeed;
    return retval == -32768 ? 0 : retval;
}


P2 (PUBLIC pascal trap, BOOLEAN, GetPixel, INTEGER, h, INTEGER, v)
{
  BitMap temp_bm;
  unsigned char temp_fbuf[4];
  Rect src_rect, dst_rect;
  
  gui_assert (! CGrafPort_p (thePort));
  
  temp_bm.baseAddr      = RM ((Ptr) temp_fbuf);
  temp_bm.bounds.top    = CWC (0);
  temp_bm.bounds.bottom = CWC (1);
  temp_bm.bounds.left   = CWC (0);
  temp_bm.bounds.right  = CWC (1);
  temp_bm.rowBytes      = CWC (4);
  
  src_rect.top    = BigEndianValue (v);
  src_rect.bottom = BigEndianValue (v + 1);
  src_rect.left   = BigEndianValue (h);
  src_rect.right  = BigEndianValue (h + 1);
  
  dst_rect = temp_bm.bounds;
  
  CopyBits (PORT_BITS_FOR_COPY (thePort), &temp_bm,
	    &src_rect, &dst_rect, srcCopy, NULL);
  
  return (*temp_fbuf & 0x80) != 0;
}

/* fromhex: converts from '0'-'9' to 0-9, 'a-z' and 'A-Z' similarly */

A1(PRIVATE, INTEGER, fromhex, char, c)
{
    if (c >= '0' && c <= '9')
        return(c - '0');
    else if (c >= 'A' && c <= 'Z')
        return(c - 'A' + 10);
    else
        return(c - 'a' + 10);
}

P2 (PUBLIC pascal trap, void, StuffHex, Ptr, p, StringPtr, s)
{
  char *sp, *ep;
  unsigned len;
  
  len = s[0];
  sp = (char *) s + 1;
  ep = sp + (len & ~1);
  for (; sp != ep; sp += 2)
    *p ++ = (fromhex (*sp) << 4) | fromhex (sp[1]);
  
  if (len & 1)
    *p = (*p & 0xF) | (fromhex (*sp) << 4);
}

P3(PUBLIC pascal trap, void, ScalePt, Point *, pt, Rect *, srcr, Rect *, dstr)
{
    register INTEGER srcdh, srcdv, dstdh, dstdv;

    if (pt->h || pt->v) {
	srcdh = Cx(srcr->right)  - Cx(srcr->left);
	srcdv = Cx(srcr->bottom) - Cx(srcr->top);
	dstdh = Cx(dstr->right)  - Cx(dstr->left);
	dstdv = Cx(dstr->bottom) - Cx(dstr->top);

	pt->h = BigEndianValue(((((LONGINT) BigEndianValue(pt->h) * dstdh) << 1) / srcdh + 1) >> 1);
	pt->v = BigEndianValue(((((LONGINT) BigEndianValue(pt->v) * dstdv) << 1) / srcdv + 1) >> 1);

	if (BigEndianValue(pt->v) < 1)
	    pt->v = CWC(1);
	if (BigEndianValue(pt->h) < 1)
	    pt->h = CWC(1);
    }
}

P3(PUBLIC pascal trap, void, MapPt, Point *, pt, Rect *, srcr, Rect *, dstr)
{
    register INTEGER srcdh, srcdv, dstdh, dstdv;

    srcdh = Cx(srcr->right)  - Cx(srcr->left);
    srcdv = Cx(srcr->bottom) - Cx(srcr->top);
    dstdh = Cx(dstr->right)  - Cx(dstr->left);
    dstdv = Cx(dstr->bottom) - Cx(dstr->top);

    pt->h = BigEndianValue(BigEndianValue(pt->h) - (Cx(srcr->left)));
    pt->v = BigEndianValue(BigEndianValue(pt->v) - (Cx(srcr->top)));
    pt->h = BigEndianValue((LONGINT) BigEndianValue(pt->h) * dstdh / srcdh);
    pt->v = BigEndianValue((LONGINT) BigEndianValue(pt->v) * dstdv / srcdv);
    pt->h = BigEndianValue(BigEndianValue(pt->h) + (Cx(dstr->left)));
    pt->v = BigEndianValue(BigEndianValue(pt->v) + (Cx(dstr->top)));
}

P3(PUBLIC pascal trap, void, MapRect, Rect *, r, Rect *, srcr, Rect *, dstr)
{
    MapPt((Point *) &r->top,    srcr, dstr);
    MapPt((Point *) &r->bottom, srcr, dstr);
}

#define IMPOSSIBLE -1
#define MAPH(x) (x = UNFIX((x - xoff1) * xcoff) + xoff2)
#define MAPV(y) (y = UNFIX((y - yoff1) * ycoff) + yoff2)
#define UNFIX(x) ((x) >> 16)

P3(PUBLIC pascal trap, void, MapRgn, RgnHandle, rh, Rect *, srcr, Rect *, dstr)
{
    INTEGER *ip, *op, oldv, newv, *tempp, srcdh, dstdh, srcdv, dstdv;
    INTEGER xoff1, xoff2, yoff1, yoff2, *saveop, x1, x2;
    Fixed xcoff, ycoff;
    INTEGER buf1[1000], buf2[1000], *mergebuf, *freebuf, *ipe;
    LONGINT hold;
    BOOLEAN done;
    
    if (EquivRect(srcr, dstr))
	OffsetRgn(rh, Cx(dstr->left) - Cx(srcr->left),
						Cx(dstr->top) - Cx(srcr->top));
    else {
	if (RGN_SMALL_P (rh))
	    MapRect(&HxX(rh, rgnBBox), srcr, dstr);
	else {
	    srcdh = BigEndianValue(srcr->right)  - (xoff1 = BigEndianValue(srcr->left));
	    dstdh = BigEndianValue(dstr->right)  - (xoff2 = BigEndianValue(dstr->left));
	    srcdv = BigEndianValue(srcr->bottom) - (yoff1 = BigEndianValue(srcr->top) );
	    dstdv = BigEndianValue(dstr->bottom) - (yoff2 = BigEndianValue(dstr->top) );
	    xcoff = FixRatio(dstdh, srcdh);
	    ycoff = FixRatio(dstdv, srcdv);
	    ip = op = (INTEGER *) STARH(rh) + 5;
	    oldv = -32768;
	    buf1[0] = 32767;
	    mergebuf = buf1;
	    freebuf  = buf2;
	    do {
		done = (newv = BigEndianValue(*ip++)) == 32767;
		MAPV(newv);
		if (newv != oldv || done) {
		    if (mergebuf[0] != 32767) {
			*op++ = BigEndianValue(oldv);
			saveop = op;
			hold = IMPOSSIBLE;
			for (tempp = mergebuf; (x1 = *tempp++) != 32767;) {
			    MAPH(x1);
			    if (hold == IMPOSSIBLE)
				hold = (unsigned short) x1;
			    else if (hold == x1)
				hold = IMPOSSIBLE;
			    else {
				*op++ = BigEndianValue(hold);
				hold = (unsigned short) x1;
			    }
			}
			if (hold != IMPOSSIBLE)
			    *op++ = BigEndianValue(hold);
			gui_assert(!((op - saveop)&1));
			if (op == saveop)
			    --op;
			else
			    *op++ = CWC(32767);
		    }
		    gui_assert(op < ip);
		    mergebuf[0] = 32767;
		    oldv = newv;
		}
		if (!done) {
		    for (tempp = freebuf, ipe = mergebuf,
					   x1 = BigEndianValue(*ip++), x2 = *ipe++;;) {
			if (x1 < x2) {
			    *tempp++ = x1;
			    x1 = BigEndianValue(*ip++);
			} else if (x1 > x2) {
			    *tempp++ = x2;
			    x2 = *ipe++;
			} else {
			    if (x1 == 32767)
/*-->*/			        break;
			    x1 = BigEndianValue(*ip++);
			    x2 = *ipe++;
			}
		    }
		    gui_assert(!((tempp-freebuf)&1));
		    *tempp++ = 32767;
		    tempp    = freebuf;
		    freebuf  = mergebuf;
		    mergebuf = tempp;
		}
	    } while (!done);
	    *op++ = CWC(32767);
	    ROMlib_sizergn(rh, FALSE);
	}
    }
}

P3(PUBLIC pascal trap, void, MapPoly, PolyHandle, poly, Rect *, srcr,
								  Rect *, dstr)
{
    Point *ip, *ep;
    
    if (EquivRect(srcr, dstr))
	OffsetPoly(poly, Cx(dstr->left) - Cx(srcr->left),
					        Cx(dstr->top) - Cx(srcr->top));
    else {
	MapPt((Point *) &HxX(poly, polyBBox.top),    srcr, dstr);
	MapPt((Point *) &HxX(poly, polyBBox.bottom), srcr, dstr);
	for (ip = HxX(poly, polyPoints),
	     ep = ip + (Hx(poly, polySize) - SMALLPOLY)/sizeof(Point);
	     ip != ep;)
	    MapPt(ip++, srcr, dstr);
    }
}

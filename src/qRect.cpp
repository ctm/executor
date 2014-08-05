/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qRect[] =
	    "$Id: qRect.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "ToolboxUtil.h"

using namespace Executor;
using namespace ByteSwap;

P5(PUBLIC pascal trap, void, SetRect, Rect *, r, INTEGER, left, INTEGER, top,
					    INTEGER, right, INTEGER, bottom)
{
    r->top    = BigEndianValue(top);
    r->left   = BigEndianValue(left);
    r->bottom = BigEndianValue(bottom);
    r->right  = BigEndianValue(right);
}

P3(PUBLIC pascal trap, void, OffsetRect, Rect *, r, INTEGER, dh, INTEGER, dv)
{
  SWAPPED_OPW (r->top,    +, dv);
  SWAPPED_OPW (r->bottom, +, dv);
  SWAPPED_OPW (r->left,   +, dh);
  SWAPPED_OPW (r->right,  +, dh);
}

P3(PUBLIC pascal trap, void, InsetRect, Rect *, r, INTEGER, dh, INTEGER, dv)
{
  SWAPPED_OPW (r->top,    +, dv);
  SWAPPED_OPW (r->bottom, -, dv);
  SWAPPED_OPW (r->left,   +, dh);
  SWAPPED_OPW (r->right,  -, dh);

#if defined (INCOMPATIBLEBUTSANE)
  if (BigEndianValue(r->top) >= BigEndianValue(r->bottom) || BigEndianValue(r->left) >= BigEndianValue(r->right))
    RECT_ZERO (r);
#endif /* INCOMPATIBLEBUTSANE */
}

P3(PUBLIC pascal trap, BOOLEAN, SectRect, const Rect *, s1, const Rect *, s2,
   Rect *, dest)
{
  if (   BigEndianValue (s1->top)  < BigEndianValue (s2->bottom)
      && BigEndianValue (s2->top)  < BigEndianValue (s1->bottom)
      && BigEndianValue (s1->left) < BigEndianValue (s2->right)
      && BigEndianValue (s2->left) < BigEndianValue (s1->right))
    {
      dest->top    = BigEndianValue (MAX (BigEndianValue (s1->top),    BigEndianValue (s2->top)));
      dest->left   = BigEndianValue (MAX (BigEndianValue (s1->left),   BigEndianValue (s2->left)));
      dest->bottom = BigEndianValue (MIN (BigEndianValue (s1->bottom), BigEndianValue (s2->bottom)));
      dest->right  = BigEndianValue (MIN (BigEndianValue (s1->right),  BigEndianValue (s2->right)));
      return !EmptyRect (dest);
    }
  else
    {
      RECT_ZERO (dest);
      return FALSE;
    }
}

P1(PUBLIC pascal trap, BOOLEAN, EmptyRect, Rect *, r)
{
    return(BigEndianValue(r->top) >= BigEndianValue(r->bottom) ||
           BigEndianValue(r->left) >= BigEndianValue(r->right));
}

P3(PUBLIC pascal trap, void, UnionRect, Rect *, s1, Rect *, s2, Rect *, dest)
{
    if (EmptyRect(s1))
	*dest = *s2;
    else if (EmptyRect(s2))
	*dest = *s1;
    else {
	dest->top    = BigEndianValue(MIN (BigEndianValue(s1->top),    BigEndianValue(s2->top)));
	dest->left   = BigEndianValue(MIN (BigEndianValue(s1->left),   BigEndianValue(s2->left)));
	dest->bottom = BigEndianValue(MAX (BigEndianValue(s1->bottom), BigEndianValue(s2->bottom)));
	dest->right  = BigEndianValue(MAX (BigEndianValue(s1->right),  BigEndianValue(s2->right)));
    }
}

P2(PUBLIC pascal trap, BOOLEAN, PtInRect, Point, p, Rect *, r)
{
  BOOLEAN retval;

  retval = (   p.h >= BigEndianValue(r->left)
	    && p.h <  BigEndianValue(r->right)
	    && p.v >= BigEndianValue(r->top)
	    && p.v <  BigEndianValue(r->bottom));
  return retval;
}

P3(PUBLIC pascal trap, void, Pt2Rect, Point, p1, Point, p2, Rect *, dest)
{
    dest->top    = BigEndianValue(MIN (p1.v, p2.v));
    dest->left   = BigEndianValue(MIN (p1.h, p2.h));
    dest->bottom = BigEndianValue(MAX (p1.v, p2.v));
    dest->right  = BigEndianValue(MAX (p1.h, p2.h));
}

P3(PUBLIC pascal trap, void, PtToAngle, Rect *, rp, Point, p, INTEGER *, angle)
{
  int a, dx, dy;

  /* I ran some tests on this code, for a square input rectangle.  We
   * are now off by no more than one degree from what the Mac
   * generates for any point within that rectangle.
   * Since we aren't exactly the same as the Mac here, why not
   * just call atan2()?
   */

  dx = p.h - (BigEndianValue (rp->left) + BigEndianValue (rp->right)) / 2;
  dy = p.v - (BigEndianValue (rp->top) + BigEndianValue (rp->bottom)) / 2;

  if (dx != 0)
    {
      a = (AngleFromSlope (FixMul (FixRatio (dx, dy),
				   FixRatio (RECT_HEIGHT (rp),
					     RECT_WIDTH (rp))))
	   % 180);
      if (dx < 0)
	a += 180;
    }
  else /* dx == 0 */
    {
      if (dy <= 0)
	a = 0;
      else
	a = 180;
    }

  *angle = BigEndianValue (a);
}

P2 (PUBLIC pascal trap, BOOLEAN, EqualRect, const Rect *, r1, const Rect *, r2)
{
  return RECT_EQUAL_P (r1, r2);
}

/* see regular.c for {Frame, Paint, Erase, Invert, Fill} Rect */

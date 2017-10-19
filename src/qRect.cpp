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

P5(PUBLIC pascal trap, void, SetRect, Rect *, r, INTEGER, left, INTEGER, top,
					    INTEGER, right, INTEGER, bottom)
{
    r->top    = CW(top);
    r->left   = CW(left);
    r->bottom = CW(bottom);
    r->right  = CW(right);
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
  if (CW(r->top) >= CW(r->bottom) || CW(r->left) >= CW(r->right))
    RECT_ZERO (r);
#endif /* INCOMPATIBLEBUTSANE */
}

P3(PUBLIC pascal trap, BOOLEAN, SectRect, const Rect *, s1, const Rect *, s2,
   Rect *, dest)
{
  if (   CW (s1->top)  < CW (s2->bottom)
      && CW (s2->top)  < CW (s1->bottom)
      && CW (s1->left) < CW (s2->right)
      && CW (s2->left) < CW (s1->right))
    {
      dest->top    = CW (MAX (CW (s1->top),    CW (s2->top)));
      dest->left   = CW (MAX (CW (s1->left),   CW (s2->left)));
      dest->bottom = CW (MIN (CW (s1->bottom), CW (s2->bottom)));
      dest->right  = CW (MIN (CW (s1->right),  CW (s2->right)));
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
    return(CW(r->top) >= CW(r->bottom) ||
           CW(r->left) >= CW(r->right));
}

P3(PUBLIC pascal trap, void, UnionRect, Rect *, s1, Rect *, s2, Rect *, dest)
{
    if (EmptyRect(s1))
	*dest = *s2;
    else if (EmptyRect(s2))
	*dest = *s1;
    else {
	dest->top    = CW(MIN (CW(s1->top),    CW(s2->top)));
	dest->left   = CW(MIN (CW(s1->left),   CW(s2->left)));
	dest->bottom = CW(MAX (CW(s1->bottom), CW(s2->bottom)));
	dest->right  = CW(MAX (CW(s1->right),  CW(s2->right)));
    }
}

P2(PUBLIC pascal trap, BOOLEAN, PtInRect, Point, p, Rect *, r)
{
  BOOLEAN retval;

  retval = (   p.h >= CW(r->left)
	    && p.h <  CW(r->right)
	    && p.v >= CW(r->top)
	    && p.v <  CW(r->bottom));
  return retval;
}

P3(PUBLIC pascal trap, void, Pt2Rect, Point, p1, Point, p2, Rect *, dest)
{
    dest->top    = CW(MIN (p1.v, p2.v));
    dest->left   = CW(MIN (p1.h, p2.h));
    dest->bottom = CW(MAX (p1.v, p2.v));
    dest->right  = CW(MAX (p1.h, p2.h));
}

P3(PUBLIC pascal trap, void, PtToAngle, Rect *, rp, Point, p, GUEST<INTEGER> *, angle)
{
  int a, dx, dy;

  /* I ran some tests on this code, for a square input rectangle.  We
   * are now off by no more than one degree from what the Mac
   * generates for any point within that rectangle.
   * Since we aren't exactly the same as the Mac here, why not
   * just call atan2()?
   */

  dx = p.h - (CW (rp->left) + CW (rp->right)) / 2;
  dy = p.v - (CW (rp->top) + CW (rp->bottom)) / 2;

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

  *angle = CW (a);
}

P2 (PUBLIC pascal trap, BOOLEAN, EqualRect, const Rect *, r1, const Rect *, r2)
{
  return RECT_EQUAL_P (r1, r2);
}

/* see regular.c for {Frame, Paint, Erase, Invert, Fill} Rect */

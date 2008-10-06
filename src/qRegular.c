/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qRegular[] =
	    "$Id: qRegular.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "DialogMgr.h"
#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/ctl.h"
#include "rsys/wind.h"

P1(PUBLIC pascal trap, void, FrameRect, Rect *, r)
{
    CALLRECT(frame, r);
}

P1(PUBLIC pascal trap, void, PaintRect, Rect *, r)
{
    CALLRECT(paint, r);
}

P1(PUBLIC pascal trap, void, EraseRect, Rect *, r)
{
    CALLRECT(erase, r);
}

P1(PUBLIC pascal trap, void, InvertRect, Rect *, r)
{
    CALLRECT(invert, r);
}

P2(PUBLIC pascal trap, void, FillRect, Rect *, r, Pattern, pat)
{
  if (!EmptyRgn (PORT_VIS_REGION (thePort)))
    {
      ROMlib_fill_pat (pat);
      CALLRECT(fill, r);
    }
}

P1(PUBLIC pascal trap, void, FrameOval, Rect *, r)
{
    CALLOVAL(frame, r);
}

P1(PUBLIC pascal trap, void, PaintOval, Rect *, r)
{
    CALLOVAL(paint, r);
}

P1(PUBLIC pascal trap, void, EraseOval, Rect *, r)
{
    CALLOVAL(erase, r);
}

P1(PUBLIC pascal trap, void, InvertOval, Rect *, r)
{
    CALLOVAL(invert, r);
}

P2(PUBLIC pascal trap, void, FillOval, Rect *, r, Pattern, pat)
{
  ROMlib_fill_pat (pat);
  CALLOVAL(fill, r);
}

PRIVATE boolean_t
rect_matches_control_item (WindowPtr w, Rect *rp)
{
  boolean_t retval;
  ControlHandle c;

  retval = FALSE;
  for (c = WINDOW_CONTROL_LIST (w); !retval && c ; c = CTL_NEXT_CONTROL (c))
    {
      Rect r;

      r = CTL_RECT (c);
      retval = ((CW (r.top) - CW (rp->top) == CW (rp->bottom) - CW (r.bottom)) &&
		(CW (r.left) - CW (rp->left) == CW (rp->right) - CW (r.right)));
    }

  return retval;
}

P3(PUBLIC pascal trap, void, FrameRoundRect, Rect *, r, INTEGER, ow,
								   INTEGER, oh)
{
  boolean_t do_rect;

  do_rect = FALSE;

  if (ROMlib_cdef0_is_rectangular)
    {
      AuxWinHandle aux;

      aux = MR (*lookup_aux_win (thePort));
      if (aux && rect_matches_control_item (HxP(aux, awOwner), r))
	do_rect = TRUE;
    }

  if (do_rect)
    FrameRect (r);
  else
    CALLRRECT(frame, r, ow, oh);
}

P3(PUBLIC pascal trap, void, PaintRoundRect, Rect *, r, INTEGER, ow,
								   INTEGER, oh)
{
    CALLRRECT(paint, r, ow, oh);
}

P3(PUBLIC pascal trap, void, EraseRoundRect, Rect *, r, INTEGER, ow,
								   INTEGER, oh)
{
    CALLRRECT(erase, r, ow, oh);
}

P3(PUBLIC pascal trap, void, InvertRoundRect, Rect *, r, INTEGER, ow,
								   INTEGER, oh)
{
    CALLRRECT(invert, r, ow, oh);
}

P4(PUBLIC pascal trap, void, FillRoundRect, Rect *, r, INTEGER, ow,
						     INTEGER, oh, Pattern, pat)
{
  ROMlib_fill_pat (pat);
  CALLRRECT(fill, r, ow, oh);
}

P3(PUBLIC pascal trap, void, FrameArc, Rect *, r, INTEGER, start,
								INTEGER, angle)
{
    CALLARC(frame, r, start, angle);
}

P3(PUBLIC pascal trap, void,  PaintArc, Rect *, r, INTEGER, start,
								INTEGER, angle)
{
    CALLARC(paint, r, start, angle);
}

P3(PUBLIC pascal trap, void, EraseArc, Rect *, r, INTEGER, start,
								INTEGER, angle)
{
    CALLARC(erase, r, start, angle);
}

P3(PUBLIC pascal trap, void, InvertArc, Rect *, r, INTEGER, start,
								INTEGER, angle)
{
    CALLARC(invert, r, start, angle);
}

P4(PUBLIC pascal trap, void, FillArc, Rect *, r, INTEGER, start,
						  INTEGER, angle, Pattern, pat)
{
  ROMlib_fill_pat (pat);
  CALLARC (fill, r, start, angle);
}

P1(PUBLIC pascal trap, void, FrameRgn, RgnHandle, rh)
{
    CALLRGN(frame, rh);
}

P1(PUBLIC pascal trap, void, PaintRgn, RgnHandle, rh)
{
    CALLRGN(paint, rh);
}

P1(PUBLIC pascal trap, void, EraseRgn, RgnHandle, rh)
{
    CALLRGN(erase, rh);
}

P1(PUBLIC pascal trap, void, InvertRgn, RgnHandle, rh)
{
    CALLRGN(invert, rh);
}

P2(PUBLIC pascal trap, void, FillRgn, RgnHandle, rh, Pattern, pat)
{
  ROMlib_fill_pat (pat);
  CALLRGN(fill, rh);
}


P1(PUBLIC pascal trap, void, FramePoly, PolyHandle, poly)
{
    CALLPOLY(frame, poly);
}

P1(PUBLIC pascal trap, void, PaintPoly, PolyHandle, poly)
{
    CALLPOLY(paint, poly);
}

P1(PUBLIC pascal trap, void, ErasePoly, PolyHandle, poly)
{
    CALLPOLY(erase, poly);
}

P1(PUBLIC pascal trap, void, InvertPoly, PolyHandle, poly)
{
    CALLPOLY(invert, poly);
}

P2(PUBLIC pascal trap, void, FillPoly, PolyHandle, poly, Pattern, pat)
{
  ROMlib_fill_pat (pat);
  CALLPOLY (fill, poly);
}

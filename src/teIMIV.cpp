/* Copyright 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teIMIV[] =
	    "$Id: teIMIV.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "TextEdit.h"
#include "ToolboxEvent.h"
#include "MemoryMgr.h"

#include "rsys/tesave.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;
using namespace ByteSwap;

P3 (PUBLIC pascal trap, void, TEPinScroll, int16, dh,		/* IMIV-57 */
    int16, dv, TEHandle, te)
{
  Rect *view_rect;
  Rect *dest_rect;
  SignedByte te_flags;
  TEPtr tep;
  
  te_flags = HGetState ((Handle) te);
  HLock ((Handle) te);
  tep = STARH (te);

  view_rect = &TEP_VIEW_RECT (tep);
  dest_rect = &TEP_DEST_RECT (tep);
  
  if (dv > 0)
    {
      int view_rect_top = BigEndianValue (view_rect -> top);
      int dest_rect_top = BigEndianValue (dest_rect -> top);
      
      /* `destRect.top' must stay below `viewRect.bottom' */
      
      if (dest_rect_top + dv > view_rect_top)
	dv = view_rect_top - dest_rect_top;
    }
  else
    {
      Point end_pt;
      int view_rect_bottom = BigEndianValue (view_rect -> bottom);
      int dest_rect_bottom;
      
      {
	int lineno;
	int length;
	
	length = TEP_LENGTH (tep);
	/* ### this should use `dest_rect -> bottom', but that isn't
	   updated correctly at the moment */
	TEP_CHAR_TO_POINT (tep, length, &end_pt);
	
	lineno = TEP_CHAR_TO_LINENO (tep, length);
	dest_rect_bottom = end_pt.v + TEP_HEIGHT_FOR_LINE (tep, lineno);
      }
      
      /* `destRect.bottom' must stay above `viewRect.bottom' */
      if (dest_rect_bottom + dv < view_rect_bottom)
	dv = view_rect_bottom - dest_rect_bottom ;
    }

  {
    /* ### i fixed the above code, but didn't take a look at horiz
       scrolling yet */
      int16 maxshift;
      
      if (dh > 0)
	{
	  maxshift = BigEndianValue (view_rect->left) - BigEndianValue (dest_rect->left);
	  if (maxshift > 0)
	    dh = MIN (maxshift, dh);
	  else
	    dh = 0;
	}
      else
	{
	  maxshift = BigEndianValue (view_rect->left) - BigEndianValue (dest_rect->left);
	  if (maxshift < 0)
	    dh = MAX (maxshift, dh);
	  else
	    dh = 0;
	}
  }
  
  HSetState ((Handle) te, te_flags);
  
  if (dh || dv)
    TEScroll (dh, dv, te);
}

/*
 * teh paramater not within spec.  Shouldn't hurt anything with C calling
 * conventions.
 */

A1 (PUBLIC, void, ROMlib_teautoloop, TEHandle, teh)
{
  Point pt;
  
  GetMouse(&pt);
  if (BigEndianValue(pt.v) < Hx(teh, viewRect.top))
    TEPinScroll(0, Hx(teh, lineHeight), teh);
  else if (BigEndianValue(pt.v) > Hx(teh, viewRect.bottom))
    TEPinScroll(0, -Hx(teh, lineHeight), teh);
}

static int16
getdelta (int16 selstart, int16 selstop,
	  int16 viewstart, int16 viewstop)
{
  if (selstart < viewstart)
    return viewstart - selstart;
  else if (selstop > viewstop)
    {
      if (selstop - selstart > viewstop - viewstart)
	return viewstart - selstart;
      else
	return viewstop - selstop;
    }
  else
    return 0;
}

/*
 * NOTE:  We should be using SelRect in TESelView and elsewhere (sigh).
 */

P1 (PUBLIC pascal trap, void, TESelView, TEHandle, teh)	/* IMIV-57 */
{
  int16 dh, dv;
  Point start, stop;
  
  if (STARH(TEHIDDENH(teh))->flags & CLC(TEAUTOVIEWBIT))
    {
      TE_CHAR_TO_POINT (teh, TE_SEL_START (teh), &start);
      TE_CHAR_TO_POINT (teh, TE_SEL_END (teh), &stop);
      stop.v += Hx (teh, lineHeight);
      dv = getdelta(start.v, stop.v + Hx(teh, lineHeight),
		    Hx(teh, viewRect.top), Hx(teh, viewRect.bottom));
      dh = getdelta(start.h, stop.h,
		    Hx(teh, viewRect.left), Hx(teh, viewRect.right));
      TEPinScroll(dh, dv, teh);
    }
}

P2 (PUBLIC pascal trap, void, TEAutoView, BOOLEAN, autoflag,	/* IMIV-57 */
    TEHandle, teh)
{
  if (autoflag)
    STARH(TEHIDDENH(teh))->flags |=  CLC(TEAUTOVIEWBIT);
  else
    STARH(TEHIDDENH(teh))->flags &= CLC(~TEAUTOVIEWBIT);
}

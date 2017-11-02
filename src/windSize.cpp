/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windSize[] =
	    "$Id: windSize.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "EventMgr.h"
#include "OSEvent.h"
#include "ToolboxUtil.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"

using namespace Executor;

/*
 * Note, the code below probably be rewritten to use XorRgn as much
 *	 as possible and probably have only one CalcVisBehind
 */

P4(PUBLIC pascal trap, void, MoveWindow, WindowPtr, wp, INTEGER, h, INTEGER, v,
								BOOLEAN, front)
{
    GrafPtr gp;
    RgnHandle movepart, updatepart, behindpart;
    Rect r;
    register WindowPeek w;

    TRAPBEGIN();
    w = (WindowPeek) wp;
    gp = thePort;
    if (WINDOW_VISIBLE_X (w))
      {
	SetPort (MR (wmgr_port));
	ClipRect (&GD_BOUNDS (MR (TheGDevice)));
	ClipAbove(w);
	movepart = NewRgn();
	updatepart = NewRgn();
	behindpart = NewRgn();
	SectRgn(PORT_CLIP_REGION (MR (wmgr_port)),
		WINDOW_STRUCT_REGION (w), movepart);

#if 1
	/* 
	 * CopyBits does unaligned 32-bit reads from the source, which can
	 * cause it to read beyond the framebuffer in certain circumstances.
	 * This is a cheesy way to prevent that from happening here.  A
	 * better fix would be either in CopyBits or to force an extra page
	 * after the framebuffer.
	 */
	{
	  Rect tmpr;
	  RgnHandle last_three_pixels;

	  tmpr = GD_BOUNDS (MR (TheGDevice));
	  tmpr.top = CW (CW (tmpr.bottom) - 1);
	  tmpr.left = CW (CW (tmpr.right) - 3);
	  last_three_pixels = NewRgn ();
	  RectRgn (last_three_pixels, &tmpr);
	  DiffRgn (movepart, last_three_pixels, movepart);
	  DisposeRgn (last_three_pixels);
	}
#endif

	CopyRgn(movepart, behindpart);
	r = HxX (WINDOW_STRUCT_REGION (w), rgnBBox);
      }
#if !defined(LETGCCWAIL)
    else 
      {
	movepart = 0;
	updatepart = 0;
	behindpart = 0;
      }
#endif

#if 1
/*
 * NOTE: the use of portRect below was introduced by Bill, without comment
 *       either here or in the rcslog.  But taking it out made the MSW5.1
 *	 Picture editting window come up in the wrong place.
 *	 (That could be due to other inconsistencies though, like the
 */
    h += CW (PORT_BOUNDS (w).left) - CW (PORT_RECT (w).left);
    v += CW (PORT_BOUNDS (w).top)  - CW (PORT_RECT (w).top);
#else
    h += CW (PORT_BOUNDS (w).left);
    v += CW (PORT_BOUNDS (w).top);
#endif
    if (WINDOW_VISIBLE_X (w))
      {
	WRAPPER_PIXMAP_FOR_COPY (wrapper);
	
	OffsetRect (&r, h, v);
	OffsetRgn (movepart, h, v);
	SectRgn (movepart, PORT_CLIP_REGION (MR (wmgr_port)), movepart);
	ClipRect (&GD_BOUNDS (MR (TheGDevice)));

	WRAPPER_SET_PIXMAP_X (wrapper, GD_PMAP_X (MR (TheGDevice)));

#define NEW_CLIP_HACK
#if defined(NEW_CLIP_HACK)
	/* 
	 * This hack appears to be necessary because clipping via the
	 * clip-region isn't enough to prevent us from reading bits that
	 * are outside the framebuffer.  If there is unmapped memory on
	 * either side of the framebuffer we can eat flaming death for
	 * just looking at it.  This appears to happen under NT4.0.
	 */
	{
	  Rect srcr, dstr;
	  
	  SectRect (&HxX (WINDOW_STRUCT_REGION (w), rgnBBox),
		    &GD_BOUNDS (MR (TheGDevice)), &srcr);

	  dstr = GD_BOUNDS (MR (TheGDevice));
	  OffsetRect (&dstr, h, v);
	  SectRect (&dstr, &r, &dstr);
	  CopyBits (wrapper, wrapper, &srcr, &dstr, srcCopy, movepart);
	}
#else
	CopyBits (wrapper, wrapper,
		  &HxX (WINDOW_STRUCT_REGION (w), rgnBBox), &r,
		  srcCopy, movepart);
#endif
      }
    OffsetRgn (WINDOW_STRUCT_REGION (w), h, v);
    OffsetRgn (WINDOW_CONT_REGION (w), h, v);
    OffsetRgn (WINDOW_UPDATE_REGION (w), h, v);
    OffsetRect (&PORT_BOUNDS (w), -h, -v);
    if (WINDOW_VISIBLE_X (w))
      {
	ClipRect (&GD_BOUNDS (MR (TheGDevice)));
	ClipAbove(w);
	DiffRgn (WINDOW_STRUCT_REGION (w), movepart, updatepart);
	SectRgn (PORT_CLIP_REGION (MR (wmgr_port)), updatepart, updatepart);
	DiffRgn(behindpart, movepart, behindpart);
	DiffRgn(behindpart, updatepart, behindpart);
	PaintOne(w, updatepart);
	PaintBehind (WINDOW_NEXT_WINDOW (w), behindpart);
	CalcVisBehind(w, updatepart);
	CalcVisBehind (WINDOW_NEXT_WINDOW (w), behindpart);
	CalcVisBehind (WINDOW_NEXT_WINDOW (w), movepart);

	DisposeRgn(movepart);
	DisposeRgn(updatepart);
	DisposeRgn(behindpart);
      }
    if (front)
	SelectWindow((WindowPtr) w);   
    SetPort(gp);
    TRAPEND();
}

P3 (PUBLIC pascal trap, void, DragWindow, WindowPtr, wp, Point, p, Rect *, rp)
{
  RgnHandle rh;
  LONGINT l;
  EventRecord ev;
  bool cmddown;
  Rect r;
    
  THEPORT_SAVE_EXCURSION
    (MR (wmgr_port),
     {
       GetOSEvent (0, &ev);
       SetClip (MR (GrayRgn));
       cmddown = (bool)(ev.modifiers & CWC (cmdKey));
       if (cmddown)
	 ClipAbove ((WindowPeek) wp);
       rh = NewRgn ();
       CopyRgn (WINDOW_STRUCT_REGION (wp), rh);
       r = *rp;
       if (CW (r.top) < 24)
	 r.top = CWC (24);
       l = DragGrayRgn (rh, p, &r, &r, noConstraint, (ProcPtr) 0);
       if ((uint32) l != 0x80008000)
	 MoveWindow(wp,
		    (- CW (PORT_BOUNDS (wp).left)
		     + LoWord (l) + CW (PORT_RECT (wp).left)),
		    (- CW (PORT_BOUNDS (wp).top)
		     + HiWord (l) + CW (PORT_RECT (wp).top)),
		    !cmddown);
    
       DisposeRgn (rh);
     });
}

#define SETUP_PORT(p)				\
do						\
{						\
  SetPort (p);					\
  PenPat(gray);					\
  PenMode(notPatXor);				\
}						\
while (FALSE)

#define RESTORE_PORT(p)				\
do						\
{						\
  PenPat(black);				\
  PenMode(patCopy);				\
  SetPort (p);					\
}						\
while (FALSE)

P3(PUBLIC pascal trap, LONGINT, GrowWindow, WindowPtr, w, Point, startp,
								    Rect *, rp)
{
    EventRecord ev;
    GrafPtr gp;
    Point p;
    Rect r;
    Rect pinr;
    LONGINT l;

    p.h = startp.h;
    p.v = startp.v;
#if 0
    r.left   = CW (- CW (PORT_BOUNDS (w).left));
    r.top    = CW (- CW (PORT_BOUNDS (w).top));
    r.right  = CW (CW (r.left) + RECT_WIDTH (&PORT_RECT (w)));
    r.bottom = CW (CW (r.top)  + RECT_HEIGHT (&PORT_RECT (w)));
#else
    r.left   = CW (CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left));
    r.top    = CW (CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top));
    r.right  = CW (CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left));
    r.bottom = CW (CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top));
#endif

    pinr.left = CW(CW(r.left) + CW(rp->left));
    if (CW(pinr.left) <= CW(r.left) && CW(rp->left) > 0)
      pinr.left = CWC(32767);

    pinr.top = CW(CW(r.top) + CW(rp->top));
    if (CW(pinr.top) <= CW(r.top) && CW(rp->top) > 0)
      pinr.top = CWC(32767);

    pinr.right = CW(CW(r.left) + CW(rp->right));
    if (CW(pinr.right) <= CW(r.left) && CW(rp->right) > 0)
      pinr.right = CWC(32767);

    pinr.bottom = CW(CW(r.top) + CW(rp->bottom));
    if (CW(pinr.bottom) <= CW(r.top) && CW(rp->bottom) > 0)
      pinr.bottom = CWC(32767);

    gp = thePort;
    SETUP_PORT ((GrafPtr) MR (WMgrPort));
    SETUP_PORT (MR (wmgr_port));
    ClipRect (&GD_BOUNDS (MR (TheGDevice)));
    ClipAbove((WindowPeek) w);
    WINDCALL((WindowPtr) w, wGrow, ptr_to_longint(&r));
    while (!GetOSEvent(mUpMask, &ev))
      {
        Point ep = ev.where.get();
	l = PinRect (&pinr, ep);
	ep.v = HiWord(l);
	ep.h = LoWord(l);
        if (p.h != ep.h || p.v != ep.v)
	  {
            WINDCALL((WindowPtr) w, wGrow, ptr_to_longint(&r));
            r.right = CW(CW(r.right) + (ep.h - p.h));
            r.bottom = CW(CW(r.bottom) + (ep.v - p.v));
            WINDCALL((WindowPtr) w, wGrow, ptr_to_longint(&r));
            p.h = ep.h;
            p.v = ep.v;
	  }
	CALLDRAGHOOK();
      }
    WINDCALL ((WindowPtr) w, wGrow, ptr_to_longint(&r));
    RESTORE_PORT ((GrafPtr) MR (WMgrPort));
    RESTORE_PORT (gp);
    if (p.h != startp.h || p.v != startp.v)
/*-->*/ return(((LONGINT)(CW(r.bottom) - CW(r.top)) << 16)|
	        (unsigned short)(CW(r.right) - CW(r.left)));
    else
        return(0L);
}

/* #### speedup? bag saveold, drawnew */

P4 (PUBLIC pascal trap, void, SizeWindow, WindowPtr, w,
    INTEGER, width, INTEGER, height, BOOLEAN, flag)
{
  if (width || height)
    {
      if (WINDOW_VISIBLE_X (w))
	SaveOld ((WindowPeek) w);
      
      PORT_RECT (w).right  = CW (CW (PORT_RECT (w).left) + width);
      PORT_RECT (w).bottom = CW (CW (PORT_RECT (w).top)  + height);
      
      THEPORT_SAVE_EXCURSION
	(MR (wmgr_port),
	 {
	   WINDCALL (w, wCalcRgns, 0);
	   if (WINDOW_VISIBLE_X (w))
	     DrawNew ((WindowPeek) w, flag);
	 });
    }
}

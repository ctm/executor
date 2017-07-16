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
using namespace ByteSwap;

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
	  tmpr.top = BigEndianValue (BigEndianValue (tmpr.bottom) - 1);
	  tmpr.left = BigEndianValue (BigEndianValue (tmpr.right) - 3);
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
    h += BigEndianValue (PORT_BOUNDS (w).left) - BigEndianValue (PORT_RECT (w).left);
    v += BigEndianValue (PORT_BOUNDS (w).top)  - BigEndianValue (PORT_RECT (w).top);
#else
    h += BigEndianValue (PORT_BOUNDS (w).left);
    v += BigEndianValue (PORT_BOUNDS (w).top);
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
  int cmddown;
  Rect r;
    
  THEPORT_SAVE_EXCURSION
    (MR (wmgr_port),
     {
       GetOSEvent (0, &ev);
       SetClip (MR (GrayRgn));
       cmddown = ev.modifiers & CWC (cmdKey);
       if (cmddown)
	 ClipAbove ((WindowPeek) wp);
       rh = NewRgn ();
       CopyRgn (WINDOW_STRUCT_REGION (wp), rh);
       r = *rp;
       if (BigEndianValue (r.top) < 24)
	 r.top = CWC (24);
       l = DragGrayRgn (rh, p, &r, &r, noConstraint, (ProcPtr) 0);
       if ((uint32) l != 0x80008000)
	 MoveWindow(wp,
		    (- BigEndianValue (PORT_BOUNDS (wp).left)
		     + LoWord (l) + BigEndianValue (PORT_RECT (wp).left)),
		    (- BigEndianValue (PORT_BOUNDS (wp).top)
		     + HiWord (l) + BigEndianValue (PORT_RECT (wp).top)),
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
    r.left   = BigEndianValue (- BigEndianValue (PORT_BOUNDS (w).left));
    r.top    = BigEndianValue (- BigEndianValue (PORT_BOUNDS (w).top));
    r.right  = BigEndianValue (BigEndianValue (r.left) + RECT_WIDTH (&PORT_RECT (w)));
    r.bottom = BigEndianValue (BigEndianValue (r.top)  + RECT_HEIGHT (&PORT_RECT (w)));
#else
    r.left   = BigEndianValue (BigEndianValue (PORT_RECT (w).left)   - BigEndianValue (PORT_BOUNDS (w).left));
    r.top    = BigEndianValue (BigEndianValue (PORT_RECT (w).top)    - BigEndianValue (PORT_BOUNDS (w).top));
    r.right  = BigEndianValue (BigEndianValue (PORT_RECT (w).right)  - BigEndianValue (PORT_BOUNDS (w).left));
    r.bottom = BigEndianValue (BigEndianValue (PORT_RECT (w).bottom) - BigEndianValue (PORT_BOUNDS (w).top));
#endif

    pinr.left = BigEndianValue(BigEndianValue(r.left) + BigEndianValue(rp->left));
    if (BigEndianValue(pinr.left) <= BigEndianValue(r.left) && BigEndianValue(rp->left) > 0)
      pinr.left = CWC(32767);

    pinr.top = BigEndianValue(BigEndianValue(r.top) + BigEndianValue(rp->top));
    if (BigEndianValue(pinr.top) <= BigEndianValue(r.top) && BigEndianValue(rp->top) > 0)
      pinr.top = CWC(32767);

    pinr.right = BigEndianValue(BigEndianValue(r.left) + BigEndianValue(rp->right));
    if (BigEndianValue(pinr.right) <= BigEndianValue(r.left) && BigEndianValue(rp->right) > 0)
      pinr.right = CWC(32767);

    pinr.bottom = BigEndianValue(BigEndianValue(r.top) + BigEndianValue(rp->bottom));
    if (BigEndianValue(pinr.bottom) <= BigEndianValue(r.top) && BigEndianValue(rp->bottom) > 0)
      pinr.bottom = CWC(32767);

    gp = thePort;
    SETUP_PORT (MR ((GrafPtr) WMgrPort));
    SETUP_PORT (MR (wmgr_port));
    ClipRect (&GD_BOUNDS (MR (TheGDevice)));
    ClipAbove((WindowPeek) w);
    WINDCALL((WindowPtr) w, wGrow, (LONGINT) (long) &r);
    while (!GetOSEvent(mUpMask, &ev))
      {
	ev.where.h = BigEndianValue(ev.where.h);
	ev.where.v = BigEndianValue(ev.where.v);
        l = PinRect (&pinr, ev.where);
	ev.where.v = HiWord(l);
	ev.where.h = LoWord(l);
        if (p.h != ev.where.h || p.v != ev.where.v)
	  {
            WINDCALL((WindowPtr) w, wGrow, (LONGINT) (long) &r);
            r.right = BigEndianValue(BigEndianValue(r.right) + (ev.where.h - p.h));
            r.bottom = BigEndianValue(BigEndianValue(r.bottom) + (ev.where.v - p.v));
            WINDCALL((WindowPtr) w, wGrow, (LONGINT) (long) &r);
            p.h = ev.where.h;
            p.v = ev.where.v;
	  }
	CALLDRAGHOOK();
      }
    WINDCALL ((WindowPtr) w, wGrow, (LONGINT) (long) &r);
    RESTORE_PORT (MR ((GrafPtr) WMgrPort));
    RESTORE_PORT (gp);
    if (p.h != startp.h || p.v != startp.v)
/*-->*/ return(((LONGINT)(BigEndianValue(r.bottom) - BigEndianValue(r.top)) << 16)|
	        (unsigned short)(BigEndianValue(r.right) - BigEndianValue(r.left)));
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
      
      PORT_RECT (w).right  = BigEndianValue (BigEndianValue (PORT_RECT (w).left) + width);
      PORT_RECT (w).bottom = BigEndianValue (BigEndianValue (PORT_RECT (w).top)  + height);
      
      THEPORT_SAVE_EXCURSION
	(MR (wmgr_port),
	 {
	   WINDCALL (w, wCalcRgns, 0);
	   if (WINDOW_VISIBLE_X (w))
	     DrawNew ((WindowPeek) w, flag);
	 });
    }
}

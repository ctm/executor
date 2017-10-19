/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windDisplay[] =
	"$Id: windDisplay.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

/* get {C}PORT_... accessors */
#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/glue.h"

using namespace Executor;

P2(PUBLIC pascal trap, void, SetWTitle, WindowPtr, w, StringPtr, t)
{
    if (!w)
      return;
    PtrToXHand((Ptr) t, (Handle) WINDOW_TITLE (w), (LONGINT) t[0] + 1);

    THEPORT_SAVE_EXCURSION
      (MR (wmgr_port),
       {
	 WINDOW_TITLE_WIDTH_X (w) = CW (StringWidth (t));
	 
	 if (WINDOW_VISIBLE_X (w))
	   {
	     SetClip (WINDOW_STRUCT_REGION (w));
	     ClipAbove ((WindowPeek) w);
	     WINDCALL (w, wDraw, 0);
	   }
       });
}

P2(PUBLIC pascal trap, void, GetWTitle, WindowPtr, w, StringPtr, t)
{
    if (!w)
/*-->*/	return;
    str255assign (t, STARH (WINDOW_TITLE (w)));
}

A1(PUBLIC, WindowPeek, ROMlib_firstvisible, WindowPtr, w)	/* INTERNAL */
{
    WindowPeek wp;

    for (wp = (WindowPeek) w;
	 wp && (!WINDOW_VISIBLE_X (wp)
		|| wp == (WindowPeek) MR (GhostWindow));
	 wp = WINDOW_NEXT_WINDOW (wp))
      ;
    return wp;
}

P0(PUBLIC pascal trap, WindowPtr, FrontWindow)
{
  WindowPtr retval;

  retval = (WindowPtr) ROMlib_firstvisible ((WindowPtr) MR (WindowList));
  return retval;
}

P2(PUBLIC pascal trap, void, HiliteWindow, WindowPtr, w, BOOLEAN, flag)
{
    if (!w)
      return;
    THEPORT_SAVE_EXCURSION
      (MR (wmgr_port),
       {
	 SetClip (WINDOW_STRUCT_REGION (w));
	 ClipAbove ((WindowPeek) w);
	 if (flag && !WINDOW_HILITED_X (w))
	   {
	     WINDOW_HILITED_X (w) = TRUE;
	     WINDCALL(w, wDraw, 0);
	   }
	 else if (!flag && WINDOW_HILITED_X (w))
	   {
	     WINDOW_HILITED_X (w) = FALSE;
	     WINDCALL(w, wDraw, 0);
	   }
       });
}

P1(PUBLIC pascal trap, void, BringToFront, WindowPtr, w)
{
  WindowPeek wp;
  RgnHandle hidden;
  
  if (MR (WindowList) != (WindowPeek) w)
    THEPORT_SAVE_EXCURSION
      (MR (wmgr_port),
       {
	 SetClip (MR (GrayRgn));
	 for (wp = MR (WindowList);
	      wp && WINDOW_NEXT_WINDOW (wp) != (WindowPeek) w;
	      wp = WINDOW_NEXT_WINDOW (wp))
	   ;
	 if (wp)
	   {
	     WINDOW_NEXT_WINDOW_X (wp) = WINDOW_NEXT_WINDOW_X (w);
	     WINDOW_NEXT_WINDOW_X (w) = WindowList;
	     WindowList = RM ((WindowPeek) w);
	     if (WINDOW_VISIBLE_X (w))
	       {
		 /* notify the palette manager that the `FrontWindow ()'
		    may have changed */
		 pm_front_window_maybe_changed_hook ();
		 
		 hidden = NewRgn ();
		 CopyRgn (PORT_VIS_REGION (w), hidden);
		 OffsetRgn (hidden,
			    - Cx (PORT_BOUNDS (w).left),
			    - Cx (PORT_BOUNDS (w).top));
		 XorRgn (WINDOW_STRUCT_REGION (w), hidden, hidden);
		 PaintOne ((WindowPeek) w, hidden);
		 CalcVisBehind ((WindowPeek) w, hidden);
		 DisposeRgn (hidden);
	       }
	   }
       });
}

P1(PUBLIC pascal trap, void, SelectWindow, WindowPtr, w)
{
  WindowPtr cactive;

  cactive = FrontWindow ();
  if (cactive != w)
    {
      HiliteWindow (cactive, FALSE);
      CurDeactive = RM (cactive);
      CurActivate = RM (w);
    }
  BringToFront (w);
  HiliteWindow (w, TRUE);
}

P2(PUBLIC pascal trap, void, ShowHide, WindowPtr, w, BOOLEAN, flag)
{
  if (flag && !WINDOW_VISIBLE_X (w))
    {
      WINDOW_VISIBLE_X (w) = TRUE;
      /* notify the palette manager that the `FrontWindow ()' may have
	 changed */
      pm_front_window_maybe_changed_hook ();
      THEPORT_SAVE_EXCURSION
	(MR (wmgr_port),
	 {
	   AuxWinHandle aux_w;
	   RGBColor *content_color = NULL;
	   CTabHandle w_ctab;

/*
 * Energy Scheming suggests that this test is incorrect.  They change portRect
 * and then call ShowHide twice (once to hide, once to show) to see their
 * changes.  With this test in place, their code fails.
 *
 *	   if (EmptyRgn (WINDOW_STRUCT_REGION (w)))
 */
	     WINDCALL(w, wCalcRgns, 0);
	   SetClip (WINDOW_STRUCT_REGION (w));
	   ClipAbove ((WindowPeek) w);
	   CalcVisBehind ((WindowPeek) w, PORT_CLIP_REGION (MR (wmgr_port)));
	   WINDCALL (w, wDraw, 0);
	   CopyRgn (WINDOW_CONT_REGION (w), WINDOW_UPDATE_REGION (w));
	   
	   aux_w = MR (*lookup_aux_win (w));
	   w_ctab = HxP (aux_w, awCTable);
	   if (w_ctab)
	     {
	       int i;
	       ColorSpec *w_ctab_table = CTAB_TABLE (w_ctab);
	       
	       for (i = 0; i < CTAB_SIZE (w_ctab); i ++)
		 {
		   if (w_ctab_table[i].value == wContentColor)
		     content_color = &w_ctab_table[i].rgb;
		 }
	     }
	   if (content_color)
	     RGBBackColor (content_color);
	   FillRgn (WINDOW_CONT_REGION (w), white);
	   if (content_color)
	     RGBBackColor (&ROMlib_white_rgb_color);
	 });
    }
  else if (!flag && WINDOW_VISIBLE_X (w))
    {
      WINDOW_VISIBLE_X (w) = FALSE;
      /* notify the palette manager that the `FrontWindow ()' may have
	 changed */
      pm_front_window_maybe_changed_hook ();
      THEPORT_SAVE_EXCURSION
	(MR (wmgr_port),
	 {
	   SetClip (MR (GrayRgn));
	   SetEmptyRgn (PORT_VIS_REGION (w));
	   PaintBehind (WINDOW_NEXT_WINDOW (w), WINDOW_STRUCT_REGION (w));
	   CalcVisBehind (WINDOW_NEXT_WINDOW (w), WINDOW_STRUCT_REGION (w));
	 });
    }
}

P1(PUBLIC pascal trap, void, HideWindow, WindowPtr, w)
{
  WindowPeek nextvis;
  
  if (!w)
    return;
  if (WINDOW_VISIBLE_X (w))
    {
      if (w == FrontWindow())
	{
	  nextvis = ROMlib_firstvisible ((WindowPtr) WINDOW_NEXT_WINDOW (w));
	  if (nextvis)
	    SelectWindow ((WindowPtr) nextvis);
	  else
	    CurDeactive = RM (w);
	  WINDOW_HILITED_X (w) = FALSE;
	}
      ShowHide(w, FALSE);
    }
}

P1(PUBLIC pascal trap, void, ShowWindow, WindowPtr, w)
{
  WindowPeek t;

  if (!w)
/*-->*/	return;
  if (!WINDOW_VISIBLE_X (w))
    {
      TRAPBEGIN ();
      ShowHide (w, TRUE);
      if (FrontWindow () == w && !WINDOW_HILITED_X (w))
	{
	  HiliteWindow (w, TRUE);
	  CurActivate = RM (w);
	  for (t = WINDOW_NEXT_WINDOW (w);
	       t && !WINDOW_HILITED_X (t);
	       t = WINDOW_NEXT_WINDOW (t))
	    ;
	  HiliteWindow ((WindowPtr) t, FALSE);
	  CurDeactive = (WindowPtr) RM (t);
	}
      TRAPEND();
    }
}

P2(PUBLIC pascal trap, void, SendBehind, WindowPtr, w, WindowPtr, behind)
{
  HIDDEN_WindowPeek *wpp;
  WindowPeek oldfront, newfront, oldbehind;
  RgnHandle temprgn;
  Rect r;

  oldfront = (WindowPeek) FrontWindow();

/* NOTE: the following nasty code is to make our SendBehind behave like the Mac
	 send behind, which is that it looks for a new FrontWindow with the
	 WindowList as it would be if w were totally removed from the list
   BEGINNING of nasty code */

  if (oldfront == (WindowPeek) w &&
      (newfront = ROMlib_firstvisible ((WindowPtr) WINDOW_NEXT_WINDOW (oldfront))))
    SelectWindow ((WindowPtr) newfront);

/* END of nasty code */

  if ((!WINDOW_NEXT_WINDOW (w) && !behind)
      || WINDOW_NEXT_WINDOW (w) == (WindowPeek) w)
/*-->*/ return;
  for (wpp = (HIDDEN_WindowPeek *) &WindowList;
       *wpp && STARH(wpp) != (WindowPeek) w;
       wpp = (HIDDEN_WindowPeek *) &WINDOW_NEXT_WINDOW_X (STARH (wpp)))
    ;
  if (! *wpp)
/*-->*/	return;
  *wpp = WINDOW_NEXT_WINDOW_X (w);
  oldbehind = STARH (wpp);
  if (behind)
    {
	WINDOW_NEXT_WINDOW_X (w) = WINDOW_NEXT_WINDOW_X (behind);
	WINDOW_NEXT_WINDOW_X (behind) = (WindowPeek) RM (w);
    }
  else
    {
#define SEND_BEHIND
#if defined (SEND_BEHIND)
	if (!*wpp) /* what if 'w' is the only window? */
	  wpp = (HIDDEN_WindowPeek *) &WindowList;
    	for (; WINDOW_NEXT_WINDOW_X (STARH (wpp));
	     wpp = (HIDDEN_WindowPeek *) &WINDOW_NEXT_WINDOW_X (STARH (wpp)))
	  ;
	if (STARH (wpp) != (WindowPeek) w) 
	  WINDOW_NEXT_WINDOW_X (STARH (wpp)) = (WindowPeek) RM (w);
#endif /* SEND_BEHIND */
	WINDOW_NEXT_WINDOW_X (w) = 0;
      }
    CalcVis ((WindowPeek) w);
    temprgn = NewRgn ();
    CopyRgn (PORT_VIS_REGION (w), temprgn);
    OffsetRgn (temprgn,
	       - Cx (PORT_BOUNDS (w).left),
	       - Cx (PORT_BOUNDS (w).top));
    XorRgn (WINDOW_STRUCT_REGION (w), temprgn, temprgn);
    CalcVisBehind (oldbehind, temprgn);
    PaintBehind (oldbehind, temprgn);
    DisposeRgn (temprgn);
#if 0
    newfront = (WindowPeek) FrontWindow ();
    if (oldfront != newfront) {
	CurDeactive = CL((WindowPtr) oldfront);
	HiliteWindow((WindowPtr) oldfront, FALSE);
	CurActivate = CL((WindowPtr) newfront);
	HiliteWindow((WindowPtr) newfront, TRUE);
    }
#else /* 0 */
    if (oldfront == (WindowPeek) w && MR (WindowList) != (WindowPeek) w)
      {
	THEPORT_SAVE_EXCURSION
	  (w,
	   {
	     r = HxX (PORT_VIS_REGION (w), rgnBBox);
	     EraseRect(&r);			/* ick! The bad Mac made me do it */
	     InvalRect(&r);
	   });
      }
#endif /* 0 */
}

P1(PUBLIC pascal trap, void, DrawGrowIcon, WindowPtr, w)
{
  if (!ROMlib_emptyvis && WINDOW_VISIBLE (w))
    THEPORT_SAVE_EXCURSION
      (MR (wmgr_port),
       {
	 SetClip (PORT_CLIP_REGION (w));
	 OffsetRgn (WINDOW_STRUCT_REGION (w),
		    CW (PORT_BOUNDS (w).left),
		    CW (PORT_BOUNDS (w).top));
	 SectRgn (PORT_CLIP_REGION (thePort), WINDOW_STRUCT_REGION (w),
		  PORT_CLIP_REGION (thePort));
	 OffsetRgn (WINDOW_STRUCT_REGION (w),
		    - CW (PORT_BOUNDS (w).left),
		    - CW (PORT_BOUNDS (w).top));
	 OffsetRgn (PORT_CLIP_REGION (thePort),
		    - CW (PORT_BOUNDS (w).left),
		    - CW (PORT_BOUNDS (w).top));
	 ClipAbove ((WindowPeek) w);
	 WINDCALL(w, wDrawGIcon, 0);
       });
}

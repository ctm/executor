/* Copyright 1986 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlStddef[] =
	    "$Id: ctlStddef.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "ToolboxUtil.h"
#include "MemoryMgr.h"

#include "rsys/ctl.h"
#include "rsys/cquick.h"

using namespace Executor;

enum
{
  frame_color,
  body_color,
  text_color,
  n_ctl_colors,
};

static RGBColor current_control_colors[n_ctl_colors];

static int text_mode;

static Pattern frame_pattern;

void
validate_colors_for_control (ControlHandle ctl)
{
  RGBColor ctl_ctab_colors[n_ctl_colors];
  AuxCtlHandle t_aux_c;
  int hilited_p, active_p;
  int i;
  
  hilited_p = (CTL_HILITE_X (ctl) != 255);
  active_p = (CTL_HILITE_X (ctl) == inButton);
  
  for (i = 0; i < n_ctl_colors; i ++)
    ctl_ctab_colors[i] = default_ctl_colors[i].rgb;
  
#if 0
  {
    int def_ctl_ctab_size;
    ColorSpec *def_ctl_ctab_table;

    def_ctl_ctab_size = CTAB_SIZE (default_ctl_ctab);
    def_ctl_ctab_table = CTAB_TABLE (default_ctl_ctab);
    
    for (i = 0; i < n_ctl_colors; i ++)
      {
	int j;
	
	for (j = 0; j < def_ctl_ctab_size; j ++)
	  if (CW (def_ctl_ctab_table[j].value) == i)
	    ctl_ctab_colors[i] = def_ctl_ctab_table[j].rgb;
      }
  }
#endif
  
  t_aux_c = MR (*lookup_aux_ctl (ctl));
  if (t_aux_c && HxX (t_aux_c, acCTable))
    {
      CTabHandle c_ctab;
      ColorSpec *c_ctab_table;
      int c_ctab_size;
      
      c_ctab = (CTabHandle) HxP (t_aux_c, acCTable);
      c_ctab_table = CTAB_TABLE (c_ctab);
      c_ctab_size = CTAB_SIZE (c_ctab);
      
      for (i = c_ctab_size; i >= 0; i --)
	{
	  ColorSpec *c_ctab_entry;
	  
	  c_ctab_entry = &c_ctab_table[i];
	  if (CW (c_ctab_entry->value) >= n_ctl_colors)
	    {
	      /* don't make so much noise; our own default control
                 color table will set off this warning */
#if 0
	      warning_unexpected
		("control color table with index `%d' > %d or < 0; ignored",
		 CW (c_ctab_entry->value), n_ctl_colors);
#endif
	      continue;
	    }
	  ctl_ctab_colors[CW (c_ctab_entry->value)] = c_ctab_entry->rgb;
	}
    }
  
  if (hilited_p)
    {
      text_mode = srcOr;
      
      if (active_p)
	{
	  /* swapped */
	  current_control_colors[body_color] = ctl_ctab_colors[cTextColor];
	  current_control_colors[text_color] = ctl_ctab_colors[cBodyColor];
	}
      else
	{
	  current_control_colors[text_color] = ctl_ctab_colors[cTextColor];
	  current_control_colors[body_color] = ctl_ctab_colors[cBodyColor];
	}
      
      current_control_colors[frame_color] = ctl_ctab_colors[frame_color];
      PATASSIGN (frame_pattern, black);
    }
  else
    {
      if (! CGrafPort_p (thePort)
	  || ! AVERAGE_COLOR (&ctl_ctab_colors[cFrameColor],
			      &ctl_ctab_colors[cBodyColor], 0x8000,
			      &current_control_colors[frame_color]))
	{
	  current_control_colors[frame_color] = ctl_ctab_colors[frame_color];
	  PATASSIGN (frame_pattern, gray);
	}
      else
	{
	  PATASSIGN (frame_pattern, black);
	}
      
      text_mode = grayishTextOr;
      
      current_control_colors[body_color] = ctl_ctab_colors[cBodyColor];
      current_control_colors[text_color] = ctl_ctab_colors[cTextColor];
    }
}

static int countchar (StringPtr str, char c)
{
  int retval;
  char *p, *ep;

  for (retval = 0, p = (char *) str+1, ep = p + str[0]; p != ep; )
    if (*p++ == c)
      retval++;
  return retval;
}

typedef enum { justleft, justmiddle } justenum;

#define SEPCHAR '\r'

static void
drawlabel (StringPtr str, Rect *rp, justenum just)
{
  struct tempstr
    {
      INTEGER firstb;
      INTEGER bytec;
      INTEGER left;
    } *infop, *temp;
  int i, ei, nlines;
  FontInfo fi;
  INTEGER mid, top, incr;
  INTEGER text_mode_save;
  ALLOCABEGIN
    
  RGBForeColor (&current_control_colors[text_color]);
  
  text_mode_save = PORT_TX_MODE (thePort);
  TextMode (text_mode);
  
  GetFontInfo(&fi);
  nlines = countchar(str, SEPCHAR) + 1;
  temp = infop = (struct tempstr *) ALLOCA(sizeof(*infop) * nlines);
  temp->firstb = 1;
  for (i = 1, ei = str[0]+1; i < ei; i++)
    {
      if (str[i] == SEPCHAR)
	{
	  temp->bytec = i - temp->firstb;
	  temp++;
	  temp->firstb = i+1;
	}
    }
  temp->bytec = i - temp->firstb;
  if (just == justmiddle)
    {
      mid = (CW(rp->left) + CW(rp->right)) / 2;
      for (i = 0; i < nlines; i++)
	infop[i].left
	  = mid - TextWidth ((Ptr) str, infop[i].firstb, infop[i].bytec) / 2;
    }
  else
    {
      for (i = 0; i < nlines; i++)
	infop[i].left = CW(rp->left);
    }
  incr = CW(fi.ascent) + CW(fi.descent) + CW(fi.leading);
  top = (CW(rp->top) + CW(rp->bottom)) / 2 -
    (nlines * incr - CW(fi.leading) + 1) / 2 + CW(fi.ascent);
  for (i = 0; i < nlines; i++, top += incr)
    {
      MoveTo(infop[i].left, top);
      DrawText((Ptr) str, infop[i].firstb, infop[i].bytec);
    }
  
  TextMode (text_mode_save);
  ALLOCAEND
}

/*
 * NOTE: mapvar used to return 0 if v couldn't be matched to pushButProc,
 *	 checkBoxProc or radioButProc.  However, the behaviour of Quicken 3.0
 *	 suggests that the test really should be anything other than
 *	 pushButProc returns inCheckBox.  Blech!
 */

static int
mapvar (int v)
{
#if 0
    switch (v) {
    case  pushButProc & 0xF:
        return inButton;
    case checkBoxProc & 0xF:
    case radioButProc & 0xF:
        return inCheckBox;
    }
    return 0;
#else
    return ((v & 7) == pushButProc) ? inButton : inCheckBox;
#endif
}

static void
draw_push (ControlHandle c, int16 part)
{
  GUEST<RgnHandle> save;
  int16 h, v;
  Rect r;
  
  r = CTL_RECT (c);
  h = CW (r.right) - CW(r.left);
  v = (CW (r.bottom) - CW (r.top)) / 2; 
  if (h > v)
    h = v;
  save = PORT_CLIP_REGION_X (CTL_OWNER (c));
  PORT_CLIP_REGION_X (CTL_OWNER (c)) = RM (NewRgn ());
  OpenRgn();
    FrameRoundRect (&r, h, v);
  CloseRgn (PORT_CLIP_REGION (CTL_OWNER (c)));
  
  /* erase in the body color */
  RGBForeColor (&current_control_colors[body_color]);
  PaintRoundRect (&r, h, v);
  
  /* frame in the frame color */
  RGBForeColor (&current_control_colors[frame_color]);
  PenPat (frame_pattern);
  FrameRoundRect (&r, h, v);
  
  /* inset the rect by a pixel so that drawing cannot overlap the oval */
  InsetRgn (PORT_CLIP_REGION (CTL_OWNER (c)), 1, 1);
  
  /* draw the label in the body color, since it is internal to the
     button.  by default, `drawlabel ()' draws the text background in
     the current background color (as used by radio and check buttons) */
  RGBBackColor (&current_control_colors[body_color]);
  drawlabel (CTL_TITLE (c), &r, justmiddle);
  DisposeRgn (PORT_CLIP_REGION (CTL_OWNER (c)));
  
  PORT_CLIP_REGION_X (CTL_OWNER (c)) = save;
}

static void
add_title (ControlHandle c)
{
  WindowPtr control_owner;
  GUEST<RgnHandle> save;
  Rect r;
  
  control_owner = CTL_OWNER (c);
  save = PORT_CLIP_REGION_X (control_owner);
  PORT_CLIP_REGION_X (control_owner) = RM (NewRgn ());
  r = CTL_RECT (c);
  RectRgn (PORT_CLIP_REGION (control_owner), &r);
  r.left = CW (CW (r.left) + 16);
  drawlabel (CTL_TITLE (c), &r, justleft);
  DisposeRgn (PORT_CLIP_REGION (control_owner));
  PORT_CLIP_REGION_X (control_owner) = save;
}

static void
draw_check (ControlHandle c, int16 part)
{
  Rect r;
  
  if (!part)
    {
      r = CTL_RECT (c);
      r.right = CW (CW (r.right) - 2);
      EraseRect (&r);
      add_title (c);
    }
  r.left   = CW (CW (CTL_RECT (c).left) + 2);
  r.top    = CW ((CW (CTL_RECT (c).top) + CW (CTL_RECT (c).bottom)) / 2 - 6);
  r.bottom = CW (CW (r.top) + 12);
  r.right  = CW (CW (r.left) + 12);
  EraseRect (&r);
  
  RGBForeColor (&current_control_colors[frame_color]);
  PenPat (frame_pattern);
  if (CTL_HILITE (c) == inCheckBox)
    PenSize (2, 2);
  FrameRect (&r);
  if (CTL_VALUE (c))
    {
      PenSize (1, 1);
      MoveTo (CW (r.left) + 1, CW (r.top) + 1);
      LineTo (CW (r.right) - 2, CW (r.bottom) - 2);
      MoveTo (CW (r.right) - 2, CW (r.top) + 1);
      LineTo (CW (r.left) + 1, CW (r.bottom) - 2);
    }
}

static void
draw_radio (ControlHandle c, int16 part)
{
  Rect r;

  if (!part)
    {
      r = CTL_RECT (c);
      r.right = CW (CW (r.right) - 2);
      EraseRect (&r);
      add_title (c);
    }
  r.left   = CW (CW (CTL_RECT (c).left) + 2);
  r.top    = CW ((CW (CTL_RECT (c).top) + CW (CTL_RECT (c).bottom)) / 2 - 6);
  r.bottom = CW (CW (r.top) + 12);
  r.right  = CW (CW (r.left) + 12);
  
  EraseRect (&r);
  if (CTL_HILITE (c) == inCheckBox)
    PenSize (2, 2);
  RGBForeColor (&current_control_colors[frame_color]);
  PenPat (frame_pattern);
  FrameOval (&r);
  if (CTL_VALUE (c))
    {
      InsetRect (&r, 3, 3);
      PaintOval (&r);
    }
}

P4 (PUBLIC, pascal LONGINT, cdef0, INTEGER, var, ControlHandle, c,
    INTEGER, mess, LONGINT, param)	/* IMI-328 */
{
  Point p;
  Rect r;
  draw_state_t draw_state;
  int draw_p;
  
  /* if drawing can occur, validate the color state */
  draw_p = (mess == drawCntl);
  
  if (draw_p)
    {
      validate_colors_for_control (c);
      draw_state_save (&draw_state);
    }
  
  switch (mess)
    {
    case drawCntl:
      if (Hx(c, contrlVis) &&
	  SectRect (&HxX (PORT_VIS_REGION (thePort), rgnBBox),
		    &HxX(c, contrlRect), &r))
	{
	  PenNormal ();
	  if (! (var & useWFont))
	    {
	      TextFont (0);
	      TextSize (0);
	      TextFace (0);
	      TextMode (srcCopy);
	    }
	  switch (var & ~useWFont)
	    {
	    case  pushButProc & 0xF:
	      draw_push (c, (int16) param);
	      break;
	    case checkBoxProc & 0xF:
	      draw_check(c, (int16) param);
	      break;
	    case radioButProc & 0xF:
	      draw_radio(c, (int16) param);
	      break;
	    }
        }
      break;
    case testCntl:
      p.v = HiWord(param);
      p.h = LoWord(param);
      if (CTL_HILITE (c) != 255 && PtInRect (p, &CTL_RECT (c)))
	return mapvar(var);
      else
	return 0;
    case calcCRgns:
    case calcCntlRgn:
      {
	RgnHandle rgn = ptr_from_longint<RgnHandle>( param );
	SignedByte state;
	
	state = HGetState ((Handle) rgn);
	if (state & LOCKBIT)
	  HSetState ((Handle) rgn, state & ~LOCKBIT);
	
	RectRgn (rgn, &CTL_RECT (c));
	
	HSetState ((Handle) rgn, state);
	break;
      }
    case initCntl:
    case dispCntl:
    case posCntl:
    case thumbCntl:
    case dragCntl:
    case autoTrack:
      break;
    default:
      gui_assert (0);	/* die */
      return 0;	/* make gcc -Wall happy */
    }
  
  if (draw_p)
    draw_state_restore (&draw_state);
  
  return 0;
}

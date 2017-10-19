/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_windDocdef[] =
		"$Id: windDocdef.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "ToolboxUtil.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/ctl.h"

#include "rsys/image.h"

using namespace Executor;

namespace Executor {
#include "zoom.cmap"
#include "go_away.cmap"
#include "active.cmap"
#include "ractive.cmap"
#include "grow.cmap"

#if 1

PUBLIC BOOLEAN ROMlib_window_zoomed(WindowPeek wp)
{
    BOOLEAN retval;
    const Rect *portrp, *staterp, *boundsrp;

    portrp   = &PORT_RECT (wp);
    staterp  = &MR (*(WStateData **) WINDOW_DATA (wp))->stdState;
    boundsrp = &PORT_BOUNDS (wp);

    retval = (WINDOW_SPARE_FLAG_X (wp)
	      && CW (portrp->top)  - CW (boundsrp->top)  == CW (staterp->top)
	      && CW (portrp->left) - CW (boundsrp->left) == CW (staterp->left)
	      && RECT_WIDTH(portrp) == RECT_WIDTH(staterp)
	      && RECT_HEIGHT(portrp) == RECT_HEIGHT(staterp) );
    return retval;
}

#define WINDOW_ZOOMED(w) (ROMlib_window_zoomed(w))

#endif
}
/* `color_p' is TRUE if the current call to the window definition
   function is for a while to be drawn in color */
static int color_p;

/* TRUE if the current window being serviced is a rounded window */
static int rounded_window_p;

/* colors to draw the current window with; has the appropriate
   black and white colors if `!color_p' */
static RGBColor *window_colors;

#define content (&window_colors[wContentColor])
/* FIXME: hack */
#undef frame
#define frame (&window_colors[wFrameColor])
#define text (&window_colors[wTextColor])
#undef hilite
#define hilite (&window_colors[wHiliteColor])
#define title_bar (&window_colors[wTitleBarColor])
#define title_bar (&window_colors[wTitleBarColor])
#define hilite_light (&window_colors[wHiliteColorLight])
#define hilite_dark (&window_colors[wHiliteColorDark])
#define title_bar_light (&window_colors[wTitleBarLight])
#define title_bar_dark (&window_colors[wTitleBarDark])
#define dialog_light (&window_colors[wDialogLight])
#define dialog_dark (&window_colors[wDialogDark])
#define tinge_light (&window_colors[wTingeLight])
#define tinge_dark (&window_colors[wTingeDark])

#define title_bar_bk (&window_colors[13])
#define title (&window_colors[14])
#define frame_outline (&window_colors[15])
#define half_tinge (&window_colors[16])
#define lines (&window_colors[17])
#define half_dialog (&window_colors[18])

/* aliases */
#define frame_shadow frame

RGBColor *
Executor::validate_colors_for_window (GrafPtr w)
{
  static RGBColor color_window_colors[19];
  /* just make the unused color something noticable */
#define UNUSED  { CWC ((unsigned short)0xFFFF), CWC (0), CWC (0) }
#define WHITE   { CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF), CWC ((unsigned short)0xFFFF) }
#define BLACK   { CWC (0), CWC (0), CWC (0) }
  static RGBColor bw_window_colors[19] =
    {
      WHITE,  /* content */
      BLACK,   /* frame */
      UNUSED,  /* text */
      UNUSED,  /* hilite */
      UNUSED,  /* title_bar */
      UNUSED,  /* hilite_light */
      UNUSED,  /* hilite_dark */
      UNUSED,  /* title_bar_light */
      UNUSED,  /* title_bar_dark */
      UNUSED,  /* dialog_light */
      BLACK,  /* dialog_dark */
      WHITE,   /* tinge_light */
      BLACK,   /* tinge_dark */
      WHITE,   /* title_bar_bk */
      BLACK,   /* title */
      BLACK,   /* frame_outline */
      UNUSED,  /* half_tinge */
      BLACK,   /* lines */
      UNUSED,  /* half_dialog */
    };
#undef BLACK
#undef WHITE
#undef UNUSED
  RGBColor *window_colors;
  AuxWinHandle t_aux_w;
  int i;
  
  /* FIXME: assuming that colors not specified come from the default
     color table; verify this */
  for (i = 0; i <= 12; i ++)
    color_window_colors[i] = default_color_win_ctab[i].rgb;
  
  t_aux_w = MR (*lookup_aux_win (w));
  if (t_aux_w && HxX (t_aux_w, awCTable))
    {
      CTabHandle w_ctab;
      ColorSpec *w_ctab_table;
      int w_ctab_size;
      
      w_ctab = HxP (t_aux_w, awCTable);
      w_ctab_table = CTAB_TABLE (w_ctab);
      /* this window definition function uses only the first 12
	 entries in the window color table */
      w_ctab_size = CTAB_SIZE (w_ctab);
      if (w_ctab_size > 12)
	{
	  warning_unexpected ("window color table with size `%d' > 12; ignored",
			      w_ctab_size);
	  w_ctab_size = 0;
	}
      
      /* blow off system6 style color tables */
      if (w_ctab_size <= 4 && 0) /* really blow off? Thresholder suggests
				    otherwise.  I'm leaving this code in
				    for now because there's probably a
				    reason it was in there in the first
				    place.  Perhaps the <= should be < */
	{
	  /* same content color */
	  for (i = 0; i <= w_ctab_size; i ++)
	    if (w_ctab_table[i].value == CW (wContentColor))
	      color_window_colors[wContentColor]
		= w_ctab_table[i].rgb;
	}
      else
	{
	  for (i = 0; i <= w_ctab_size; i ++)
	    {
	      ColorSpec *w_ctab_entry;
	      int w_ctab_entry_index;
	      
	      w_ctab_entry = &w_ctab_table[i];
	      w_ctab_entry_index = CW (w_ctab_entry->value);
	      if (w_ctab_entry_index < 0 || w_ctab_entry_index > 12)
		{
#if !defined (CYGWIN32) /* just gets in the way of debugging under windows */
		  warning_unexpected
		    ("window color table with index `%d' > 12 or < 0; ignored",
		     w_ctab_entry_index);
#endif
		  continue;
		}
	      color_window_colors[w_ctab_entry_index] = w_ctab_entry->rgb;
	    }
	}
    }

  bw_window_colors[wContentColor] = color_window_colors[wContentColor];
  
#define FAIL goto failure
#define DONE goto done
#define DO_BLOCK_WITH_FAILURE(try_block, fail_block)	\
  {							\
    { try_block }					\
    goto done;						\
  failure:						\
    { fail_block }					\
    /* fall through */					\
  done:;						\
  }

  DO_BLOCK_WITH_FAILURE
    ({
      RGBColor image_colors[5];
      RGBColor temp1;
      RGBColor temp2;
      RGBColor temp3;
      
      window_colors = color_window_colors;
      
      if (rounded_window_p)
	{
	  if (WINDOW_HILITED_X (w))
	    {
	      *title_bar_bk = *frame;
	      *title = *title_bar;	      
	    }
	  else
	    {
	      *title = *frame;
	      *title_bar_bk = *title_bar;
	    }
	  DONE;
	}
      
      if (!AVERAGE_COLOR (title_bar_light, title_bar_dark, 0xEEEE,
			  &temp1))
	FAIL;
      if (!AVERAGE_COLOR (hilite_light, &ROMlib_black_rgb_color, 0x8888,
			  &temp2))
	FAIL;
      if (!AVERAGE_COLOR (hilite_light, &ROMlib_black_rgb_color, 0x5555,
			  &temp3))
	FAIL;
      
      if (WINDOW_HILITED_X (w))
	{
	  *title_bar_bk = temp1;
	  *title = *text;
	  *frame_outline = *frame;
	}
      else
	{
	  *title = temp2;
	  *title_bar_bk = *content;
	  *frame_outline = temp3;
	}
      if (!AVERAGE_COLOR (tinge_light, tinge_dark, 0xAAAA,
			  half_tinge))
	FAIL;
      if (!AVERAGE_COLOR (title_bar_light, title_bar_dark, 0xAAAA,
			  lines))
	FAIL;
      if (!AVERAGE_COLOR (dialog_light, dialog_dark, 0x8888,
			  half_dialog))
	FAIL;
      
      /* now do the image colors */
      image_colors[0] = *title_bar_bk;
      image_colors[1] = *tinge_light;
      if (!AVERAGE_COLOR (title_bar_light, title_bar_dark, 0xAAAA,
			  &image_colors[2]))
	FAIL;
      image_colors[3] = *tinge_dark;
      /* used only in the grow icon */
      if (!AVERAGE_COLOR (title_bar_light, title_bar_dark, 0xBBBB,
			  &image_colors[4]))
	FAIL;

      image_update_ctab (go_away, image_colors, 3);
      image_update_ctab (zoom, image_colors, 3);
      image_update_ctab (active, image_colors, 3);
      image_update_ctab (grow, image_colors, 4);
      
      color_p = TRUE;
     },
     {
       gui_assert (!rounded_window_p);

       window_colors = bw_window_colors;
       color_p = FALSE;
     });
#undef DO_BLOCK_WITH_FAILURE
#undef FAIL

  return window_colors;
}

enum box_flag
{
  zoom_box_flag, go_away_box_flag,
};

void
toggle_box_active (enum box_flag which_box, Point origin)
{
  Rect dst_rect;
  pixel_image_t *box;

  /* compute the destination rectangle */
  dst_rect.top    = CW (origin.v);
  dst_rect.left   = CW (origin.h);
  dst_rect.bottom = CW (origin.v + 13);
  dst_rect.right  = CW (origin.h + 13);

  /* set box */
  if (rounded_window_p)
    color_p = FALSE;
  
  if (which_box == zoom_box_flag)
    box = zoom;
  else if (which_box == go_away_box_flag)
    box = go_away;
  else
    gui_abort ();  /* there are no more boxes */

  if (rounded_window_p)
    {
      /* invert the set bits of the box */
      PORT_FG_COLOR_X (thePort)
	= CL ((1 << PIXMAP_PIXEL_SIZE (CPORT_PIXMAP (thePort))) - 1);
      PORT_FG_COLOR_X (thePort) = CLC (0);
    }
  else
    {
      /* make sure the fg/bk colors are b/w so as not to confuse the
	 `CopyBits ()' */
      RGBForeColor (&ROMlib_black_rgb_color);
      RGBBackColor (&ROMlib_white_rgb_color);
    }

  image_copy (box, color_p, &dst_rect, srcXor);

  if (rounded_window_p)
    {
      RGBColor *target_color;

      /* this isn't quite what the mac does, but i couldn't figure
	 that one out.  it works for the common case (white on block
	 rounded window).  the mac function takes into account both
	 `title' and `title_bar' in some unknown fasion */
      if (((  title_bar->red
	    + title_bar->green
	    + title_bar->blue) / 3) > 0x8000)
	target_color = &ROMlib_black_rgb_color;
      else
	target_color = &ROMlib_white_rgb_color;

      PORT_FG_COLOR_X (thePort)
	= CL (Color2Index (target_color) ^ Color2Index (frame));
    }

  image_copy (rounded_window_p ? ractive : active,
	      color_p, &dst_rect, srcXor);
}


void
toggle_zoom_box (GrafPtr w)
{
  Point origin;
  
  origin.h = CW (PORT_RECT (w).right) - CW (PORT_BOUNDS (w).left) - 21;
  origin.v = CW (PORT_RECT (w).top)   - CW (PORT_BOUNDS (w).top) - 16;
  
  toggle_box_active (zoom_box_flag, origin);
}

void
toggle_go_away_box (GrafPtr w)
{
  Point origin;
  
  origin.h = CW (PORT_RECT (w).left) - CW (PORT_BOUNDS (w).left) + 8;
  origin.v = CW (PORT_RECT (w).top)  - CW (PORT_BOUNDS (w).top) - 16;

  toggle_box_active (go_away_box_flag, origin);
}

void
hilite_window (int left, int top, int right, int bottom)
{
  int l, r;

  /* we are called from `draw_frame ()' only, pen is (1, 1), patCopy,
     pattern black */

  /* draw title bar shadow/tinge */
  if (color_p)
    {
      /* bottom and right; we get both disputed corners */
      RGBForeColor (half_tinge);
      MoveTo (left, top - 2);
      LineTo (right - 1, top - 2);
      LineTo (right - 1, top - 18);

      /* top and left */
      RGBForeColor (tinge_light);
      MoveTo (right - 2, top - 18);
      LineTo (left, top - 18);
      LineTo (left, top - 3);
    }

  /* draw the lines */
  RGBForeColor (lines);

  l = left + 1;
  r = right - 2;
  MoveTo (l, top - 15);
  LineTo (r, top - 15);
  MoveTo (l, top - 13);
  LineTo (r, top - 13);
  MoveTo (l, top - 11);
  LineTo (r, top - 11);
  MoveTo (l, top - 9);
  LineTo (r, top - 9);
  MoveTo (l, top - 7);
  LineTo (r, top - 7);
  MoveTo (l, top - 5);
  LineTo (r, top - 5);
}

void
draw_title (GrafPtr w,
	    int go_away_drawn, int zoom_drawn)
{
  GrafPtr tp;
  int left, top, right, bottom;
  StringHandle th;
  RgnHandle saveclip = NULL;
  int title_width;
  int title_start;
  int left_bound;

  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

/* #warning "clean up this port mess in draw_title ()" */
  GUEST<GrafPtr> tmpPort;  
  GetPort(&tmpPort);
  tp = tmpPort.get();
  
  if (go_away_drawn)
    left_bound = left + 28;
  else
    left_bound = left;
  
  title_width = WINDOW_TITLE_WIDTH (w);
  if (title_width)
    {
      title_start = left + (right - left - title_width) / 2 - 6;
      saveclip = PORT_CLIP_REGION (tp);
      PORT_CLIP_REGION_X (tp) = RM (NewRgn ());
      if (title_start >= left_bound)
	CopyRgn (saveclip, PORT_CLIP_REGION (tp));
      else
	{
	  title_start = left_bound;
	  SetRectRgn (PORT_CLIP_REGION (tp),
		      title_start, top - 16,
		      right - (zoom_drawn ? 28 : 0), top-3);
	  SectRgn (PORT_CLIP_REGION (tp), saveclip, PORT_CLIP_REGION (tp));
	}
      
      {
	RgnHandle additional_clip_rgn;

	additional_clip_rgn = NewRgn ();
	SetRectRgn (additional_clip_rgn, title_start, top - 16,
		    title_start + title_width + 12, top - 2);
	SectRgn (PORT_CLIP_REGION (tp), additional_clip_rgn,
		 PORT_CLIP_REGION (tp));
      
	/* erase the text to be drawn with the title bar background
	   color */
	RGBForeColor (title_bar_bk);
	PaintRgn (additional_clip_rgn);
	DisposeRgn (additional_clip_rgn);
      }
      
      RGBBackColor (title_bar_bk);
      RGBForeColor (title);
      
      th = WINDOW_TITLE (w);
      LOCK_HANDLE_EXCURSION_1
	(th,
	 {
	   PORT_TX_MODE_X (thePort) = CWC (srcCopy);
	   MoveTo (title_start + 6, top - 5);
	   DrawString (STARH (th));
	 });
      
      if (saveclip)
	{
	  DisposeRgn (PORT_CLIP_REGION (tp));
	  PORT_CLIP_REGION_X (tp) = RM (saveclip);
	}
    }
}

void
draw_go_away (int h, int v)
{
  Rect r;

  if (rounded_window_p)
    {
      RGBForeColor (title_bar);
      RGBBackColor (title_bar_bk);
    }
  else
    {
      /* make sure the fg/bk colors are b/w so as not to confuse the
	 `CopyBits ()' */
      RGBForeColor (&ROMlib_black_rgb_color);
      RGBBackColor (&ROMlib_white_rgb_color);
    }
  
  SetRect (&r, h + 8, v - 16, h + 21, v - 3);

  image_copy (go_away, rounded_window_p ? 0 : color_p, &r, srcCopy);
}

void
draw_zoom (int h, int v)
{
  Rect r;

  if (rounded_window_p)
    {
      RGBForeColor (title_bar);
      RGBBackColor (title_bar_bk);
    }
  else
    {
      /* make sure the fg/bk colors are b/w so as not to confuse the
	 `CopyBits ()' */
      RGBForeColor (&ROMlib_black_rgb_color);
      RGBBackColor (&ROMlib_white_rgb_color);
    }

  SetRect (&r, h - 21, v - 16, h - 8, v - 3);

  image_copy (zoom, rounded_window_p ? 0 : color_p, &r, srcCopy);
}

void
draw_frame (GrafPtr w, int draw_zoom_p, boolean_t goaway_override)
{
  int left, top, right, bottom;
  Rect r;

  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
  
  PenSize (1, 1);
  /* draw with the current foreground color */
  PenPat (black);
  PenMode (patCopy);

  /* bounds of the title bar */
  SetRect (&r, left - 1, top - 19, right + 1, top);
  /* clear the title bar with the titlebar background color; title bar
     color is dependent on whether or not the window is hilited,
     `validate_colors_for_window ()' takes this into account */
  RGBForeColor (title_bar_bk);
  PaintRect (&r);

  /* draw the frame; these are drawn in the frame_outline_color */
  RGBForeColor (frame_outline);
  r.bottom = CW (bottom + 1);
  FrameRect (&r);
  MoveTo (left, top - 1);
  LineTo (right, top - 1);

  /* draw the `shadow' */
  RGBForeColor (frame_shadow);
  MoveTo (left, bottom + 1);
  LineTo (right + 1, bottom + 1);
  MoveTo (right + 1, top - 18);
  LineTo (right + 1, bottom);

  if (WINDOW_HILITED_X (w))
    {
      int draw_go_away_p = WINDOW_GO_AWAY_FLAG (w) && !goaway_override;
      
      hilite_window (left, top, right, bottom);
      if (draw_go_away_p)
	draw_go_away (left, top);
      if (draw_zoom_p)
	draw_zoom (right, top);
      draw_title (w, draw_go_away_p, draw_zoom_p);
    }
  else
    draw_title (w, FALSE, FALSE);

/* #warning "delete these, they shouldn't be necessary" */
  /* be a sneaky bastard, and set the fg/bk color to b/w */
  RGBForeColor (&ROMlib_black_rgb_color);
  RGBBackColor (&ROMlib_white_rgb_color);
}

void
draw_grow_lines (Rect *bounds)
{
  int left, top, right, bottom;
  Rect r;
  
  left   = CW (bounds->left);
  top    = CW (bounds->top);
  right  = CW (bounds->right);
  bottom = CW (bounds->bottom);
  
  PenSize (1, 1);
  SetRect (&r, left - 1, top - 19, right + 1, bottom + 1);
  FrameRect (&r);  /* outside */
  
  MoveTo (left, top - 1);
  LineTo (right, top - 1);   /* under title */
  
  MoveTo (left, bottom - 15);
  LineTo (right - 1, bottom - 15); /* at bottom */
  
  MoveTo (right - 15, top);
  LineTo (right - 15, bottom - 1); /* at side */
}

void
draw_dialog_box (GrafPtr w)
{
  int left, top, right, bottom;
  Rect r;
  RGBColor middle_color;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  PenSize (1, 1);
  PenMode (patCopy);

  /* draw the outtermost rectangle; in the frame_shadow_color */
  RGBForeColor (frame_shadow);
  SetRect (&r, left - 8, top - 8, right + 8, bottom + 8);
  FrameRect (&r);
  
  /* draw the center rectangle */
  if (color_p)
    {
      middle_color.red   = 0xBBBB;
      middle_color.green = 0xBBBB;
      middle_color.blue  = 0xBBBB;
    }
  else
    middle_color = ROMlib_white_rgb_color;
  
  RGBForeColor (&middle_color);
  InsetRect (&r, 2, 2);
  FrameRect (&r);
  
  /* draw the innermost rectangle; in the dark dialog color */
  RGBForeColor (dialog_dark);
  InsetRect (&r, 2, 2);
  FrameRect (&r);

  /* FIXME:
     go in one more pixel, and white out a rect
     three pixels wide; having to do this worries
     me that the location of the border may be wrong,
     resolve this */
  InsetRect (&r, 1, 1);
  PenSize (3, 3);
  RGBForeColor (content);
  FrameRect (&r);
  PenSize (1, 1);
  
  if (color_p)
    {
      /* and now for the inbetween hiliting */
      RGBForeColor (dialog_light);
      MoveTo (left - 7, bottom + 6);
      LineTo (left - 7, top - 7);
      LineTo (right + 6, top - 7);
      
      RGBForeColor (half_dialog);
      LineTo (right + 6, bottom + 6);
      LineTo (left - 7, bottom + 6);
      
      
      /* same color; half_dialog_color */
      MoveTo (left - 5, bottom + 4);
      LineTo (left - 5, top - 5);
      LineTo (right + 4, top - 5);
      
      RGBForeColor (dialog_light);
      MoveTo (right + 4, top - 4);
      LineTo (right + 4, bottom + 4);
      LineTo (left - 4, bottom + 4);
    }
  else
    {
      RGBForeColor (&ROMlib_black_rgb_color);
      InsetRect (&r, -2, -2);
      FrameRect (&r);

      RGBForeColor (&ROMlib_white_rgb_color);
      InsetRect (&r, -2, -2);
      FrameRect (&r);
    }
}

void
draw_plain_dialog_box (GrafPtr w)
{
  int left, top, right, bottom;
  Rect r;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  PenSize (1, 1);
  PenMode (patCopy);
  SetRect (&r, left - 1, top - 1, right + 1, bottom + 1);
  RGBForeColor (frame_outline);
  FrameRect (&r);
}

void
draw_alt_dialog_box (GrafPtr w)
{
  int left, top, right, bottom;
  Rect r;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
  
  PenSize (1, 1);
  PenMode (patCopy);
  SetRect (&r, left-1, top-1, right+1, bottom+1);
  RGBForeColor (frame_outline);
  FrameRect (&r);
  
  PenSize (2, 2);
  RGBForeColor (frame_outline);
  MoveTo (left + 1, bottom + 1);
  LineTo (right + 1, bottom + 1);
  MoveTo (right + 1, top + 1);
  LineTo (right + 1, bottom);
  
  PenSize (1, 1);
}

void
draw_grow_icon (GrafPtr w)
{
  int left, top, right, bottom;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
    
  PenPat (black);
  PenSize (1, 1);
  PenMode (patCopy);
    
  /* NOTE: IMI-302 says we should be drawing the lines here, but
     Microsoft Excel suggests otherwise (choose Number from the format
     menu and then type clover-/) */

  MoveTo (left, bottom - 15);
  LineTo (right - 1, bottom - 15);     /* line at bottom */
  
  MoveTo (right - 15, top);
  LineTo (right - 15, bottom - 1);     /* line at side */

  {
    Rect rect;

    rect.top = CW (bottom - 14);
    rect.left = CW (right - 14);
    rect.bottom  = CW (bottom);
    rect.right = CW (right);
    
    image_copy (grow, color_p, &rect, srcCopy);
  }
}

void
erase_grow_icon (GrafPtr w)
{
  int left, top, right, bottom;
  Rect r;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
  
  PenPat (black);
  PenSize (1, 1);
  PenMode (patCopy);
  MoveTo (left, bottom - 15);
  LineTo (right - 1, bottom - 15); 
  MoveTo (right - 15, top);
  LineTo (right - 15, bottom - 1);
  SetRect (&r, right - 14, bottom - 14, right, bottom);
  FillRect (&r, white);
}

LONGINT
hit_doc (WindowPeek w, LONGINT parm, int growable_p,
	 boolean_t goaway_override)
{
  Point p;
  int left, top, right, bottom;
    
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  p.v = HiWord (parm);
  p.h = LoWord (parm);
  if (WINDOW_HILITED_X (w))
    {
      if (PtInRgn (p, WINDOW_CONT_REGION (w)))
	{
	  if (!growable_p || (p.h < right - 16) || (p.v < bottom - 16))
	    return wInContent;
	  else
	    return wInGrow;
	}
      if ((p.h >= left - 1) && (p.h < right + 1)
	  && (p.v >= top - 19) && (p.v < top))
	{
	  if (WINDOW_GO_AWAY_FLAG_X (w) && !goaway_override
	      && (p.h > left + 8) && (p.h < left + 20)
	      && (p.v > top - 16) && (p.v < top - 4))
	    return wInGoAway;
	  else if (WINDOW_SPARE_FLAG_X (w)
		   && (p.h > right - 20) && (p.h < right - 8)
		   && (p.v > top - 16)   && (p.v < top - 4))
	    return WINDOW_ZOOMED ((WindowPeek) w) ? wInZoomIn : wInZoomOut;
	  else
	    return wInDrag;
	}
      else
	return wNoHit;
    }
  else
    {
      if (PtInRgn (p, WINDOW_CONT_REGION (w)))
	return wInContent;
      else if ((p.h >= left - 1) && (p.h < right + 1)
	       && (p.v >= top - 19) && (p.v < top))
	return wInDrag;
      else
	return wNoHit;
    }
}

LONGINT
hit_dialog_box (WindowPeek w, LONGINT parm)
{
  Point p;
    
  p.v = HiWord (parm);
  p.h = LoWord (parm);
  if (PtInRgn (p, WINDOW_CONT_REGION (w)))
    return wInContent;
  else
    return wNoHit;
}

void
calc_doc (GrafPtr w)
{
  RgnHandle rh;
  INTEGER *ip;
  int left, top, right, bottom;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  SetRectRgn (WINDOW_CONT_REGION (w), left, top, right, bottom);

  rh = WINDOW_STRUCT_REGION (w); 
  ReallocHandle ((Handle) rh, (Size) 44);
  HxX (rh, rgnBBox.left)   = CW (left   -  1); 
  HxX (rh, rgnBBox.top)    = CW (top    - 19);  
  HxX (rh, rgnBBox.right)  = CW (right  +  2);   
  HxX (rh, rgnBBox.bottom) = CW (bottom +  2);
  HxX (rh, rgnSize) = CWC (44);
  ip = (INTEGER *) STARH (rh) + 5;

  *ip++ = CW(top   - 19); 
  *ip++ = CW(left  - 1);
  *ip++ = CW(right + 1);
  *ip++ = CWC(32767);
  
  *ip++ = CW(top   - 18);
  *ip++ = CW(right + 1);
  *ip++ = CW(right + 2);
  *ip++ = CWC(32767);
  
  *ip++ = CW(bottom + 1);
  *ip++ = CW(left   - 1);
  *ip++ = CW(left);
  *ip++ = CWC(32767);
  
  *ip++ = CW(bottom + 2);
  *ip++ = CW(left);
  *ip++ = CW(right  + 2);
  *ip++ = CWC(32767);
  
  *ip++ = CWC(32767);
}

void
calc_alt_dialog_box (GrafPtr w)
{
  RgnHandle rh;
  INTEGER *ip;
  int left, top, right, bottom;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  SetRectRgn (WINDOW_CONT_REGION (w), left, top, right, bottom);

  rh = WINDOW_STRUCT_REGION (w); 

  ReallocHandle ((Handle) rh, (Size) 44);
  HxX (rh, rgnBBox.left)   = CW (left   - 1);
  HxX (rh, rgnBBox.top)    = CW (top    - 1);
  HxX (rh, rgnBBox.right)  = CW (right  + 3);   
  HxX (rh, rgnBBox.bottom) = CW (bottom + 3);
  HxX (rh, rgnSize) = CWC (44);
  ip = (INTEGER *) STARH (rh) + 5;
  
  *ip++ = CW(top   - 1);
  *ip++ = CW(left  - 1);
  *ip++ = CW(right + 1);
  *ip++ = CWC(32767);
  
  *ip++ = CW(top   + 1);
  *ip++ = CW(right + 1);
  *ip++ = CW(right + 3);
  *ip++ = CWC(32767);
  
  *ip++ = CW(bottom + 1);
  *ip++ = CW(left   - 1);
  *ip++ = CW(left   + 1);
  *ip++ = CWC(32767);
  
  *ip++ = CW(bottom + 3);
  *ip++ = CW(left   + 1);
  *ip++ = CW(right  + 3);
  *ip++ = CWC(32767);
  
  *ip++ = CWC(32767);
}

void
calc_dialog_box (GrafPtr w, INTEGER n)
{
  int left, top, right, bottom;
  
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  SetRectRgn (WINDOW_CONT_REGION (w), left, top, right, bottom);
  SetRectRgn (WINDOW_STRUCT_REGION (w),
	      left - n, top - n, right + n, bottom + n);
}

struct draw_save
{
  PenState pen_state;
  RGBColor fg_color;
  RGBColor bk_color;
  LONGINT fg;
  LONGINT bk;
};

P4 (PUBLIC pascal, LONGINT, wdef0,
    INTEGER, varcode, WindowPtr, window,
    INTEGER, message, LONGINT, parm)
{
  WindowPeek w = (WindowPeek) window;
  draw_state_t draw_state;
  int zoom_bit;
  boolean_t draw_p;
  
  rounded_window_p = FALSE;
  
  /* extract and clean ZOOMBIT from varcode into zoom_bit */
  zoom_bit = varcode & ZOOMBIT;
  varcode &= ~ZOOMBIT;

  if (varcode > 4 && varcode != 5)
    varcode &= 3;

  draw_p = (message == wDraw
	    || message == wDrawGIcon
	    || message == wGrow);

  if (draw_p)
    {
      draw_state_save (&draw_state);
      
      window_colors = validate_colors_for_window ((GrafPtr) w);
    }
  
  switch (message)
    {
    case wDraw:
      if (!WINDOW_VISIBLE_X (w))
	return 0;
      PenNormal ();
      TRAPBEGIN ();
      switch (varcode)
	{
	case documentProc:
	case noGrowDocProc:
	case movableDBoxProc:
	  switch (parm)
	    {
	    case 0: /* draw entire window frame */
	      draw_frame ((GrafPtr) w, zoom_bit, varcode == movableDBoxProc);
	      break;
	    case wInGoAway:
	      toggle_go_away_box ((GrafPtr) w);
	      break;
	    case wInZoomIn:
	    case wInZoomOut:
	      toggle_zoom_box ((GrafPtr) w);
	      break;
	    }
	  break;
        case dBoxProc:
	  draw_dialog_box ((GrafPtr) w);
	  break;
        case plainDBox:
	  draw_plain_dialog_box ((GrafPtr) w);
	  break;
        case altDBoxProc:
	  draw_alt_dialog_box ((GrafPtr) w);
	  break;
        }
      TRAPEND ();
      break;
    case wHit:
      switch (varcode)
	{
	case documentProc:
	  return hit_doc (w, parm, TRUE, FALSE);
	case noGrowDocProc:
	case movableDBoxProc:
	  return hit_doc (w, parm, FALSE, varcode == movableDBoxProc);
	default:
	  return hit_dialog_box (w, parm);
	}
      /* control never gets here */
      gui_abort ();
    case wCalcRgns:
      switch(varcode)
	{
        case documentProc:
        case noGrowDocProc:
	case movableDBoxProc:
#warning TODO: correct frame for movableDBoxProc (toolbox essentials 4-10)
	  calc_doc ((GrafPtr) w);
	  break;
        case dBoxProc:
	  calc_dialog_box ((GrafPtr) w, 8);
	  break;
        case plainDBox:
	  calc_dialog_box ((GrafPtr) w, 1);
	  break;
        case altDBoxProc:
	  calc_alt_dialog_box ((GrafPtr) w);
	  break;
        }
      break;
    case wNew:
      if (zoom_bit)
	{
	  WStateData *wsp;
	  
	  WINDOW_DATA_X (w) = RM (NewHandle ((Size) sizeof (WStateData)));
	  wsp = MR (* (WStateData **) WINDOW_DATA (w));
	  
	  wsp->stdState = GD_BOUNDS (MR (TheGDevice));
	  InsetRect (&wsp->stdState, 3, 3);
	  wsp->stdState.top = CW (CW (wsp->stdState.top) + 38);
	  wsp->userState = PORT_RECT (w);
	  
	  /* local to global */
	  OffsetRect (&wsp->userState, -CW (PORT_BOUNDS (w).left),
		                       -CW (PORT_BOUNDS (w).top));
	  
	  WINDOW_SPARE_FLAG_X (w) = TRUE;
	}
      else
	WINDOW_SPARE_FLAG_X (w) = FALSE;
      break;
    case wDispose:
      if (WINDOW_SPARE_FLAG_X (w))
	DisposHandle (WINDOW_DATA (w));
      break;
    case wGrow:
      TRAPBEGIN ();
      draw_grow_lines ((Rect *) (long) SYN68K_TO_US(parm));
      TRAPEND ();
      break;
    case wDrawGIcon:
      {
	/* clip the growicon to the window vis/clip regions */
	RgnHandle temp_rgn;
	RgnHandle save_clip;
	
	temp_rgn = NewRgn ();
	save_clip = NewRgn ();
	
	SectRgn (PORT_VIS_REGION (w), PORT_CLIP_REGION (w),
		 temp_rgn);
	
	OffsetRgn (temp_rgn,
		   - CW (PORT_BOUNDS (w).left),
		   - CW (PORT_BOUNDS (w).top));

	CopyRgn (PORT_CLIP_REGION (thePort), save_clip);
	SectRgn (PORT_CLIP_REGION (thePort), temp_rgn,
		 PORT_CLIP_REGION (thePort));
	
	TRAPBEGIN();
	if (varcode == documentProc)
	  {
	    if (WINDOW_HILITED_X (w))
	      draw_grow_icon ((GrafPtr) w);
	    else
	      erase_grow_icon ((GrafPtr) w);
	  }
	TRAPEND();

	CopyRgn (save_clip, PORT_CLIP_REGION (thePort));
	
	DisposeRgn (temp_rgn);
	DisposeRgn (save_clip);
	
	break;
      }
    }

  if (draw_p)
    draw_state_restore (&draw_state);
  
  return 0;
}

/* begin rounded window definition functions */

void
hilite_rounded_window (WindowPeek w,
		       INTEGER left,  INTEGER top,
		       INTEGER right, INTEGER bottom)
{
  RgnHandle rh1, rh2;

  rh1 = NewRgn ();
  rh2 = NewRgn ();
  CopyRgn (WINDOW_STRUCT_REGION (w), rh1);
  InsetRgn (rh1, 1, 1);
  SetRectRgn (rh2, left, top - 18, right, top - 1);
  SectRgn (rh1, rh2, rh1);

  RGBForeColor (title_bar_bk);
  PenPat (black);
  PaintRgn (rh1);
  
  DisposeRgn (rh2);
  DisposeRgn (rh1);
}

void
draw_rounded_doc (GrafPtr w)
{
  int left, top, right, bottom;
  int draw_go_away_p;
  Rect r;

  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);

  SetRect (&r, left - 1, top - 19, right + 1, top);
  FillRect (&r, white);
  RGBForeColor (frame);
  FrameRgn (WINDOW_STRUCT_REGION (w));
  PenSize (1, 1);
  PenMode (patCopy);
  MoveTo (left, top - 1);
  LineTo (right, top - 1);

  hilite_rounded_window ((WindowPeek) w, left, top, right, bottom);
  if (WINDOW_HILITED_X (w))
    {
      draw_go_away_p = WINDOW_GO_AWAY_FLAG (w);
      if (draw_go_away_p)
	draw_go_away (left, top);
      draw_title (w, draw_go_away_p, FALSE);
    }
  else
    draw_title (w, FALSE, FALSE);
}

LONGINT
hit_rounded_doc (GrafPtr w, LONGINT param)
{
  Point p;
  int left, top, right, bottom;
    
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
  
  p.v = HiWord (param);
  p.h = LoWord (param);
  if (WINDOW_HILITED_X (w))
    {
      if (PtInRgn (p, WINDOW_CONT_REGION (w)))
	return wInContent;
      if (p.h >= left-1 && p.h < right+1 && p.v >= top-19 && p.v < top)
	{
	  if (WINDOW_GO_AWAY_FLAG (w)
	      && p.h > left+8 && p.h < left+20
	      && p.v > top-16 && p.v < top-4)
	    return wInGoAway;
	  else
	    return wInDrag;
	}
      else
	return wNoHit;
    }
  else
    {
      if (PtInRgn (p, WINDOW_CONT_REGION (w)))
	return wInContent;
      else if (p.h >= left-1 && p.h < right+1
	       && p.v >= top-19 && p.v < top)
	return wInDrag;
      else
	return wNoHit;
    }
}


void
calc_rounded_doc (GrafPtr w, INTEGER curve_code)
{
  int left, top, right, bottom;
  int curve;
  static const int curve_code_to_curve[] =
    {
      4, 6, 8, 10, 12, 20, 24,
    };
  RgnHandle rh;
  Rect r;
    
  left   = CW (PORT_RECT (w).left)   - CW (PORT_BOUNDS (w).left);
  top    = CW (PORT_RECT (w).top)    - CW (PORT_BOUNDS (w).top);
  right  = CW (PORT_RECT (w).right)  - CW (PORT_BOUNDS (w).left);
  bottom = CW (PORT_RECT (w).bottom) - CW (PORT_BOUNDS (w).top);
  
  SetRectRgn (WINDOW_CONT_REGION (w), left, top, right, bottom);

  if (curve_code >= 1 && curve_code <= 7)
    curve = curve_code_to_curve[curve_code - 1];
  else
    curve = 16;

  r.left   = CW(left   - 1);
  r.top    = CW(top    - 19);
  r.right  = CW(right  + 1);
  r.bottom = CW(bottom + 1);
  OpenRgn ();
  FrameRoundRect (&r, curve, curve);
  CloseRgn (WINDOW_STRUCT_REGION (w));
  CopyRgn (WINDOW_STRUCT_REGION (w), WINDOW_CONT_REGION (w));
  InsetRgn (WINDOW_CONT_REGION (w), 1, 1);
  rh = NewRgn ();
  SetRectRgn (rh, left, top, right, bottom);
  SectRgn (rh, WINDOW_CONT_REGION (w), WINDOW_CONT_REGION (w));
  DisposeRgn (rh);
}

P4 (PUBLIC pascal, LONGINT, wdef16,
    INTEGER, varcode, WindowPtr, wp,
    INTEGER, message, LONGINT, param)
{
  WindowPeek w = (WindowPeek) wp;
  draw_state_t draw_state;
  boolean_t draw_p;
  
  rounded_window_p = TRUE;

  /* mask out zoom bit; it will be ignored */
  varcode &= ~ZOOMBIT;
  
  draw_p = (message == wDraw);
  
  if (draw_p)
    {
      draw_state_save (&draw_state);
      
      window_colors = validate_colors_for_window ((GrafPtr) w);
    }

  switch (message)
    {
    case wDraw:
      if (!WINDOW_VISIBLE_X (w))
	break;
      TRAPBEGIN ();
      switch (param)
	{
	case 0:
	  draw_rounded_doc ((GrafPtr) w);
	  break;

	case wInGoAway:
	  toggle_go_away_box ((GrafPtr) w);
	  break;
	}
      TRAPEND ();
      break;

    case wHit:
      return hit_rounded_doc ((GrafPtr) w, param);
      break;
      
    case wCalcRgns:
      calc_rounded_doc ((GrafPtr) w, varcode);
      break;
      
    case wNew:
      WINDOW_SPARE_FLAG_X (w) = FALSE;
      break;
      
    case wDispose:
      break;
      
    case wGrow:
    case wDrawGIcon:
      break;
    }
  
  if (draw_p)
    draw_state_restore (&draw_state);
  
  return 0;
}
 

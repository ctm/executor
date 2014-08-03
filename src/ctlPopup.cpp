/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlPopup[] =
  "$Id: ctlPopup.c 86 2005-05-25 00:47:12Z ctm $";
#endif


#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "ToolboxUtil.h"
#include "Iconutil.h"
#include "FontMgr.h"
#include "OSUtil.h"

#include "rsys/ctl.h"
#include "rsys/menu.h"

using namespace Executor;

#if 0 /* It's not clear why Cotton had all these global variables,
	 but it is clear that in at least one case he was using one
	 before setting it.  Use RCS to see what used to be done. */

static draw_state_t draw_state;
static ControlHandle ctl;
static WindowPtr ctl_owner;
static int16 window_font;
static int16 window_size;
static boolean_t window_font_p;

#endif

static void
init (ControlHandle ctl)
{
  popup_data_handle data;
  MenuHandle mh;
  int16 mid;
  int flags;
  
  data = (popup_data_handle) NewHandle (sizeof (popup_data_t));
  /* ### check for NULL return */
  
  mid = CTL_MIN (ctl);
  mh = GetMenu (mid);
  InsertMenu (mh, -1);
  {
    HIDDEN_Handle hh;

    hh.p = (Handle) mh;
    HandToHand (&hh);
    flags = CTL_VALUE (ctl);

    POPUP_MENU_ID_X (data)     = CW (mid);
    POPUP_MENU_X (data)        = RM ((MenuHandle) hh.p);
  }  
  /* private fields */
  POPUP_TITLE_WIDTH (data)   = CTL_MAX (ctl);
  POPUP_FLAGS (data)	     = flags;
  
  CTL_DATA_X (ctl) = (Handle) RM (data);
  
  CTL_ACTION_X (ctl) = (ProcPtr) CLC (-1);
  CTL_VALUE_X (ctl) = CWC (1);
  CTL_MIN_X (ctl) = CWC (1);
  CTL_MAX_X (ctl) = CW (CountMItems (mh));
  CheckItem (mh,  1, TRUE);
}

static void
calc_rgns (ControlHandle ctl, RgnHandle rgn)
{
  SignedByte state;
  
  state = HGetState ((Handle) rgn);
  if (state & LOCKBIT)
    HSetState ((Handle) rgn, state & ~LOCKBIT);
  
  RectRgn (rgn, &CTL_RECT (ctl));
  
  HSetState ((Handle) rgn, state);
}

static void
restore_bk_color (ControlHandle ctl, draw_state_t draw_state)
{
  WindowPtr ctl_owner;

  ctl_owner = CTL_OWNER (ctl);
  if (CGrafPort_p (ctl_owner))
    CPORT_RGB_BK_COLOR (ctl_owner) = draw_state.bk_color;
  PORT_BK_COLOR_X (ctl_owner) = draw_state.bk;
}

static void
set_text_face (boolean_t item_p, int flags, mextp item_info)
{
  if (item_p)
    TextFace (item_info->mstyle);
  else
    {
      if (flags & popupTitleNoStyle)
	TextFace (0);
      else
	TextFace ((flags >> 8) & 0x7F);
    }
  TextMode (srcOr);
}

/* #### todo:
   title should be hilited in menu item text color
   title should be hilited height of menu item (see #### below)
   menu item text should be drawn in menu item text color
   menu item text should be disabled if appropraite */

static void
draw (ControlHandle ctl, draw_state_t draw_state,
      int16 window_font, int16 window_size, boolean_t window_font_p,
      boolean_t invert_title_p, boolean_t title_only_p,
      int *return_popup_top, int *return_popup_left)
{
  FontInfo font_info;
  int ascent;
  int height;
  int title_width, actual_title_width;
  StringPtr title;
  
  int item_total_height;
  int item_pad;
  
  icon_info_t icon_info;
  boolean_t icon_p;
  
  mextp item_info;
  StringPtr item_title;
  
  Rect t_rect;
  Rect *ctl_rect;
  int top, left, bottom, right;
  int draw_top, draw_bottom;
  int item_top, item_left;
  int center, baseline;
  
  popup_data_handle data;
  int flags;

  /* true if we should draw the pulldown arrow; pulldown arrow is not
     drawn if there are no menu items */
  boolean_t draw_pulldown_arrow_p = TRUE;
  
  data = (popup_data_handle) CTL_DATA (ctl);
  flags = POPUP_FLAGS (data);
  
  item_info = ROMlib_mitemtop (POPUP_MENU (data), CTL_VALUE (ctl), &item_title);
  
  if (item_info == NULL)
    {
      item_title = (StringPtr) "";

      item_info = (mextp)alloca (sizeof *item_info);
      memset (item_info, '\000', sizeof *item_info);

      draw_pulldown_arrow_p = FALSE;
    }
  
  ctl_rect = &CTL_RECT (ctl);
  top      = CW (ctl_rect->top);
  left     = CW (ctl_rect->left);
  bottom   = CW (ctl_rect->bottom);
  right    = CW (ctl_rect->right);
  
  title = CTL_TITLE (ctl);
  
  if (window_font_p)
    {
      TextFont (window_font);
      TextFont (window_size);
    }
  else
    {
      TextFont (0);
      TextSize (0);
    }
  
  GetFontInfo (&font_info);
  ascent = CW (font_info.ascent);
  height = (  ascent
	    + CW (font_info.descent)
	    + CW (font_info.leading));
  title_width = POPUP_TITLE_WIDTH (data);
  
  icon_p = get_icon_info (item_info, &icon_info, TRUE);
  
  item_total_height = ((icon_p ? MAX (icon_info.height, height)
			       : height)
		       + 4);
  
  item_pad = CharWidth (' ');
  
  center = (top + bottom) / 2;
  if (item_total_height > height)
    {
      draw_top = center - item_total_height / 2;
      draw_bottom = draw_top + item_total_height;
      if (draw_top < top)
	{
	  center += (top - draw_top);
	  draw_bottom += (top - draw_top);
	  draw_top = top;
	}
      
      baseline = center - height / 2 + ascent;
    }
  else
    {
      draw_top = center - height / 2;
      draw_bottom = draw_top + height;
      if (draw_top < top)
	{
	  center += (top - draw_top);
	  draw_bottom += (top - draw_top);
	  draw_top = top;
	}
      baseline = center - height / 2 + ascent;
    }

  {
    Rect erase_rect;
    
    /* clear out the entire control */
    restore_bk_color (ctl, draw_state);
    erase_rect.top      = ctl_rect->top;
    erase_rect.left     = ctl_rect->left;
    erase_rect.bottom   = ctl_rect->bottom;
    if (title_only_p)
      erase_rect.right  = CW (left + title_width);
    else
      erase_rect.right  = ctl_rect->right;
    EraseRect (&erase_rect);
  }
  
  /* draw the title */
  set_text_face (FALSE, flags, item_info);
  
  RGBForeColor (invert_title_p ? &ROMlib_white_rgb_color
		               : &ROMlib_black_rgb_color);
  if (invert_title_p)
    RGBBackColor (&ROMlib_black_rgb_color);
  
  /* ### if icon_p && icon_info.height > height
     then should probably hilite from
     `center - icon_info.height / 2' to
     `center + icon_info.height / 2' */
  t_rect.top    = CW (baseline - ascent);
  t_rect.left   = CW (left);
  t_rect.bottom = CW (baseline - ascent + height);
  t_rect.right  = CW (left + title_width);
  
  PenMode (patCopy);
  EraseRect (&t_rect);
  
  TextMode (srcCopy);
  actual_title_width = StringWidth (title);
  switch (flags & 255)
    {
    case popupTitleLeftJust:
      MoveTo (left, baseline);
      break;
    case popupTitleRightJust:
      MoveTo (left + title_width - actual_title_width - item_pad, baseline);
      break;
    default:
    case popupTitleCenterJust:
      MoveTo (left + (title_width - actual_title_width) / 2, baseline);
      break;
    }
  DrawText ((Ptr) title, 1, *title);
  
  
  item_top = center - item_total_height / 2;
  item_left = left + title_width;
  
  if (return_popup_top && return_popup_left)
    {
      if (icon_p)
	*return_popup_top  = center - icon_info.height / 2;
      else
	*return_popup_top  = baseline - ascent;
      *return_popup_left = item_left;
    }
  
  if (title_only_p)
    return;
  
  /* draw box */
  RGBForeColor (&ROMlib_black_rgb_color);
  RGBBackColor (&ROMlib_white_rgb_color);
  
  t_rect.top    = CW (draw_top);
  t_rect.left   = CW (item_left - 1);
  t_rect.bottom = CW (draw_bottom - 1);
  t_rect.right  = CW (right - 1);

  EraseRect (&t_rect);
  
  PenSize (1, 1);
  
  FrameRect (&t_rect);
  
  /* line along bottom */
  MoveTo (item_left + 2,
	  draw_bottom - 1);
  LineTo (right - 2,
	  draw_bottom - 1);
  
  MoveTo (right - 1,
	  draw_top + 3);
  LineTo (right - 1,
	  draw_bottom - 1);
  
  /* draw the icon */
  item_left += item_pad;
  
  if (icon_p)
    {
      t_rect.top    = CW (center - icon_info.height / 2);
      t_rect.left   = CW (item_left);
      t_rect.bottom = CW (CW (t_rect.top) + (icon_info.height - ICON_PAD));
      t_rect.right  = CW (CW (t_rect.left) + (icon_info.width - ICON_PAD));
      
      if (icon_info.color_icon_p)
	PlotCIcon (&t_rect, (CIconHandle) icon_info.icon);
      else
	PlotIcon (&t_rect, icon_info.icon);
    }

  /* draw the item name, clipping to max width, displaying `...'  if
     appropriate */
  {
    int title_left, title_right;
    uint8 title_length;
    
    set_text_face (TRUE, flags, item_info);
    
    title_left = item_left + icon_info.width;
    /* arrow is padded by the arrow width `11' on either side */
    title_right = right - 33;
    
    if (title_right - title_left < StringWidth (item_title))
      {
	int i, width;
	char ellipsis[] = { 4, ' ', '.', '.', '.' };
	
	title_right -= StringWidth ((StringPtr) ellipsis);
	
	title_length = *(uint8 *) item_title;
	for (i = 1, width = 0; i <= title_length; i ++)
	  {
	    width += CharWidth (item_title[i]);
	    
	    if (title_right - title_left < width)
	      {
		i --;
		break;
	      }
	  }
	
	MoveTo (title_right, baseline);
	DrawString ((StringPtr) ellipsis);
	
	*(uint8 *) item_title = i;
	MoveTo (item_left + icon_info.width,
		baseline);
	DrawString (item_title);
	*(uint8 *) item_title = title_length;
      }
    else
      {
	MoveTo (item_left + icon_info.width,
		baseline);
	DrawString (item_title);
      }
  }

  /* draw pulldown arrow */
  if (draw_pulldown_arrow_p)
    {
      BitMap arrow_bitmap;
      Rect dst_rect;
      const unsigned char down_arrow_bits[] = {
	  image_bits (11111111), image_bits (11100000),
	  image_bits (01111111), image_bits (11000000),
	  image_bits (00111111), image_bits (10000000),
	  image_bits (00011111), image_bits (00000000),
	  image_bits (00001110), image_bits (00000000),
	  image_bits (00000100), image_bits (00000000),
	};
      
      arrow_bitmap.baseAddr = RM ((Ptr) down_arrow_bits);
      arrow_bitmap.rowBytes = CWC (2);
      SetRect (&arrow_bitmap.bounds, 0, 0, /* right, bottom */ 11, 6);
      
      dst_rect.top    = CW (center - 3);
      dst_rect.left   = CW (right - 22);
      dst_rect.bottom = CW (center - 3 + /* arrows are `6' tall */ 6);
      dst_rect.right  = CW (right - 22
			    +	/* arrows are `11' wide */ 11);
      CopyBits (&arrow_bitmap, PORT_BITS_FOR_COPY (thePort),
		&arrow_bitmap.bounds, &dst_rect, srcCopy, NULL);
    }
}

P4 (PUBLIC pascal, int32, cdef1008,
    int16, var, ControlHandle, ctl, int16, message, int32, param)
{
  boolean_t draw_p;
  int32 retval = /* dummy */ 0;
  draw_state_t draw_state;
  int16 window_font;
  int16 window_size;
  boolean_t window_font_p;

  
  if (message == initCntl)
    {
      init (ctl);
      return /* dummy */ 0;
    }
  
  draw_p = (message == drawCntl || message == autoTrack);
  /* technically we only need to do the following block if draw_p is true,
     but gcc will then warn us that window_font, window_size and window_font_p
     may be used uninitialized */
  {
    WindowPtr ctl_owner;

    ctl_owner = CTL_OWNER (ctl);
    window_font_p = (var & popupUseWFont ? TRUE : FALSE);
    window_font = PORT_TX_FONT (ctl_owner);
    window_size = PORT_TX_SIZE (ctl_owner);
  }
  
  if (draw_p)
    draw_state_save (&draw_state);
  
  switch (message)
    {
    case drawCntl:
      draw (ctl, draw_state, window_font, window_size, window_font_p,
	    FALSE, FALSE, NULL, NULL);
      break;
      
    case testCntl:
      {
	Point pt;

	pt.v = HiWord (param);
	pt.h = LoWord (param);
	
	if (CTL_HILITE (ctl) != 255
	    && PtInRect (pt, &CTL_RECT (ctl)))
	  retval = inButton;
	else
	  retval = 0;
	break;
      }
    case calcCRgns:
    case calcCntlRgn:
      calc_rgns (ctl, (RgnHandle) SYN68K_TO_US (param));
      break;
      
    case dispCntl:
      {
	popup_data_handle data;
	
	data = (popup_data_handle) CTL_DATA (ctl);
	
	DeleteMenu (POPUP_MENU_ID (data));
	DisposHandle ((Handle) POPUP_MENU (data));
	DisposHandle ((Handle) data);
	break;
      }
      
    case autoTrack:
      {
	MenuHandle mh;
	int top, left;
	int16 orig_value, value;
	Rect *port_bounds;
	popup_data_handle data;
	int i, count;
	
	data = (popup_data_handle) CTL_DATA (ctl);
	mh = POPUP_MENU (data);
	count = CountMItems (mh);
	
	if (! count)
	  break;
	
	/* make sure `orig_value' is checked */
	orig_value = CTL_VALUE (ctl);
	
	for (i = 0; i <= count; i ++)
	  CheckItem (mh, i, FALSE);
	CheckItem (mh, orig_value, TRUE);
	
	draw (ctl, draw_state, window_font, window_size, window_font_p,
	      TRUE, TRUE, &top, &left);
	
	port_bounds = &PORT_BOUNDS (CTL_OWNER (ctl));
	top  -= CW (port_bounds->top);
	left -= CW (port_bounds->left);
        CalcMenuSize (mh);
	value = PopUpMenuSelect (mh, top, left, orig_value);
	
	if (value)
	  CTL_VALUE_X (ctl) = CW (value);
	draw (ctl, draw_state, window_font, window_size, window_font_p,
	      FALSE, FALSE, NULL, NULL);
	
	break;
      }
      
    case posCntl:
    case dragCntl:
    case thumbCntl:
      /* fall through */
      
    /* case calcCntlRgn: */
    /* case calcThumbRgn: */
      
    default:
      warning_unexpected ("surprising message %d", message);
      break;
    }
  
  if (draw_p)
    draw_state_restore (&draw_state);
  
  return retval;
}

/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialCreate[] =
	    "$Id: dialCreate.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "OSUtil.h"
#include "DialogMgr.h"
#include "ControlMgr.h"
#include "ResourceMgr.h"
#include "FontMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"
#include "Iconutil.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/itm.h"
#include "rsys/ctl.h"
#include "rsys/glue.h"
#include "rsys/mman.h"
#include "rsys/resource.h"
#include "rsys/host.h"

using namespace Executor;

#define _PtrToHand(ptr, hand, len)			\
  ((void)						\
   ({							\
     Handle __temp_handle;			\
							\
     PtrToHand ((Ptr) (ptr), &__temp_handle, len);	\
     *(hand) = RM (__temp_handle);			\
   }))

void
Executor::dialog_create_item (DialogPeek dp, itmp dst, itmp src,
		    int item_no, Point base_pt)
{
  GUEST<INTEGER> *data;
  int16 res_id;
  int gd_bpp;
  
  if (dst != src)
    memcpy (dst, src, ITEM_LEN (src));
  OffsetRect (&dst->itmr, base_pt.h, base_pt.v);
  data = ITEM_DATA (dst);
  
  gd_bpp = PIXMAP_PIXEL_SIZE (GD_PMAP (MR (MainDevice)));
  
  /* many items have a resource id at the beginning of the resource
     data */
  res_id = CW (*data);
  
  if (CB (dst->itmtype) & ctrlItem)
    {
      Rect r;
      boolean_t visible_p = TRUE;
      ControlHandle ctl;
      
      r = dst->itmr;
      if (CW (r.left) > 8192)
	{
	  visible_p = FALSE;
	  r.left = CW (CW (r.left) - 16384);
	  r.right = CW (CW (r.right) - 16384);
	}
      
      if ((CB (dst->itmtype) & resCtrl) == resCtrl)
	{
	  Rect *ctl_rect;
	  GUEST<INTEGER> top, left;
	  
	  /* ### is visibility correct here? */
	  ctl = GetNewControl (res_id, (WindowPtr) dp);
	  if (ctl)
	    {
	      ctl_rect = &CTL_RECT (ctl);
	      top = ctl_rect->top;
	      left = ctl_rect->left;
	      if (r.top != top || r.left != left)
		MoveControl (ctl, CW (r.left), CW (r.top));
	    }
	}
      else
	{
	  ctl = NewControl ((WindowPtr) dp, &r,
			    (StringPtr) &dst->itmlen,
			    visible_p, 0, 0, 1,
			    CB (dst->itmtype) & resCtrl, 0L);
	}
      dst->itmhand = RM ((Handle) ctl);
      
      ValidRect (&dst->itmr);
      
      {
	AuxWinHandle aux_win_h;
	
	aux_win_h = MR (*lookup_aux_win ((WindowPtr) dp));
	if (aux_win_h && HxX (aux_win_h, dialogCItem))
	  {
	    Handle item_color_info_h;
	    item_color_info_t *item_color_info;
	    
	    item_color_info_h = HxP (aux_win_h, dialogCItem);
	    item_color_info = (item_color_info_t *) STARH (item_color_info_h);
	    
	    if (item_color_info)
	      {
		item_color_info_t *ctl_color_info;
		
		ctl_color_info = &item_color_info[item_no - 1];
		if (ctl_color_info->data || ctl_color_info->offset)
		  {
		    int color_table_bytes;
		    int color_table_offset;
		    char *color_table_base;
		    
		    CCTabHandle color_table;
		    
		    color_table_bytes = CW (ctl_color_info->data);
		    color_table_offset = CW (ctl_color_info->offset);
		    
		    color_table_base = ((char *) item_color_info
					+ color_table_offset);
		    
		    color_table = (CCTabHandle) NewHandle (color_table_bytes);
		    memcpy (STARH (color_table), color_table_base,
			    color_table_bytes);
		    
		    SetCtlColor (ctl, color_table);
		  }
	      }
	  }
      }
    }
  else if (CB (dst->itmtype) & (statText | editText))
    {
      _PtrToHand (data, &dst->itmhand, dst->itmlen);
      if ((CB (dst->itmtype) & editText)
	  && DIALOG_EDIT_FIELD (dp) == -1)
	ROMlib_dpntoteh (dp, item_no);
    }
  else if (CB (dst->itmtype) & iconItem)
    {
      Handle h = NULL;
      
      if (/* CGrafPort_p (dp) */
	  /* since copybits will do the right thing copying a color
	     image to a old-style grafport on the screen, we check the
	     screen depth to see if we should check for a color icon */
	   gd_bpp > 2)
	{
	  h = (Handle) GetCIcon (res_id);
	  gui_assert (!h || CICON_P (h));
	}
      if (!h)
	{
	  h = GetIcon (res_id);
	  if (! h || CICON_P (h))
	    warning_unexpected ("dubious icon handle");
	}
      dst->itmhand = RM (h);
    }
  else if (CB (dst->itmtype) & picItem)
    {
      dst->itmhand = RM ((Handle) GetPicture (res_id));
    }
  else
    {
      /* useritem */
      dst->itmhand = CLC_NULL;
    }
}

static DialogPtr
ROMlib_new_dialog_common (DialogPtr dp,
			  boolean_t color_p,
			  CTabHandle w_ctab,
			  Handle item_color_table_h,
			  Rect *bounds, StringPtr title,
			  boolean_t visible_p,
			  int16 proc_id, WindowPtr behind,
			  boolean_t go_away_flag,
			  int32 ref_con,
			  Handle items)
{
  GUEST<INTEGER> *ip;
  INTEGER i;
  itmp itp;
  
  if (!dp)
    dp = (DialogPtr) NewPtr (sizeof (DialogRecord));
  
  if (color_p)
    {
      NewCWindow ((Ptr) dp, bounds, title, visible_p,
		  proc_id, (CWindowPtr) behind, go_away_flag, ref_con);
      if (w_ctab && CTAB_SIZE (w_ctab) > -1)
	{
	  THEPORT_SAVE_EXCURSION
	    (thePort,
	     {
	       SetWinColor (DIALOG_WINDOW (dp), w_ctab);
	     });
	}
    }
  else
    NewWindow ((Ptr) dp, bounds, title, visible_p, proc_id,
	       behind, go_away_flag, ref_con);
  
  if (item_color_table_h)
    {
      AuxWinHandle aux_win_h;
      
      aux_win_h = MR (*lookup_aux_win (dp));
      gui_assert (aux_win_h);
      
      HxX (aux_win_h, dialogCItem) = RM (item_color_table_h);
    }

#warning We no longer call TEStylNew, this helps LB password
  
  THEPORT_SAVE_EXCURSION
    ((GrafPtr) dp,
     {
       Rect newr;
       
       TextFont (CW (DlgFont));
       newr.top = newr.left = CWC (0);
       newr.bottom = CW (CW (bounds->bottom) - CW (bounds->top));
       newr.right = CW (CW (bounds->right) - CW (bounds->left));
       InvalRect (&newr);
       WINDOW_KIND_X (dp) = CWC (dialogKind);
       
       {
	 Rect emptyrect;
	 TEHandle te;
	 
	 RECT_ZERO (&emptyrect);

	 /************************ DO NOT CHECK THIS IN ****************
	 if (color_p && item_color_table_h)
	   te = TEStylNew (&emptyrect, &emptyrect);
	 else
	 ***************************************************************/
	   te = TENew (&emptyrect, &emptyrect);
	 
	 DIALOG_TEXTH_X (dp) = RM (te);
	 TEAutoView (TRUE, te);
	 DisposHandle (TE_HTEXT (te));
	 TE_HTEXT_X (te) = CLC_NULL;
       }
       
       DIALOG_EDIT_FIELD_X (dp) = CWC (-1);
       DIALOG_EDIT_OPEN_X (dp) = CWC (0);
       DIALOG_ADEF_ITEM_X (dp) = CWC (1);
       
       DIALOG_ITEMS_X (dp) = RM (items);
       if (items)
	 {
	   Point zero_pt;
	   
	   memset (&zero_pt, '\000', sizeof zero_pt);
	   
	   MoveHHi (items);
	   
	   LOCK_HANDLE_EXCURSION_1
	     (items,
	      {
		int item_no;
		
		ip = (GUEST<INTEGER> *) STARH (items);
		itp = (itmp) (ip + 1);
		i = CW (*ip);
		item_no = 1;
		while (i-- >= 0)
		  {
		    dialog_create_item ((DialogPeek) dp, itp, itp, item_no,
					zero_pt);
		    
		    BUMPIP (itp);
		    item_no ++;
		  }
	      });
	 }
     });
  
  return (DialogPtr) dp;
}
			  
/* IM-MTE calls this `NewColorDialog ()' */
P9 (PUBLIC pascal trap, CDialogPtr, NewCDialog, Ptr, storage, 	/* IMI-412 */
    Rect *, bounds, StringPtr, title, BOOLEAN, visible_p, INTEGER, proc_id,
    WindowPtr, behind, BOOLEAN, go_away_flag, LONGINT, ref_con, Handle, items)
{
  return (CDialogPtr) ROMlib_new_dialog_common ((DialogPtr) storage,
						/* color */ TRUE, NULL, NULL,
						bounds, title, visible_p, proc_id,
						behind, go_away_flag, ref_con,
						items);
}

P9 (PUBLIC pascal trap, DialogPtr, NewDialog, Ptr, storage, 	/* IMI-412 */
    Rect *, bounds, StringPtr, title, BOOLEAN, visible_p, INTEGER, proc_id,
    WindowPtr, behind, BOOLEAN, go_away_flag, LONGINT, ref_con, Handle, items)
{
  return ROMlib_new_dialog_common ((DialogPtr) storage,
				   /* not color */ FALSE, NULL, NULL,
				   bounds, title, visible_p, proc_id,
				   behind, go_away_flag, ref_con, items);
}

void
Executor::dialog_compute_rect (Rect *dialog_rect, Rect *dst_rect,
		     int position)
{
  Rect *screen_rect;
  int dialog_width, dialog_height;
  int screen_width, screen_height;
  
  dialog_width  = RECT_WIDTH (dialog_rect);
  dialog_height = RECT_HEIGHT (dialog_rect);
  
  screen_rect   = &GD_RECT (MR (MainDevice));
  screen_width  = RECT_WIDTH (screen_rect);
  screen_height = RECT_HEIGHT (screen_rect);
  
  switch (position)
    {
      /* #### find out what `stagger' position means */
      
    default:
    case noAutoCenter:
      /* noAutoCenter */
      *dst_rect = *dialog_rect;
      break;
      
    case alertPositionParentWindow:
    case dialogPositionParentWindow:
      {
	WindowPtr parent;
	
	parent = FrontWindow ();
	if (parent)
	  {
	    Rect *parent_rect;
	    int top;
	    int left;

	    parent_rect = &PORT_RECT (parent);

	    top  = CW (parent_rect->top) + 16;
	    left = (  (  CW (parent_rect->left)
		       + CW (parent_rect->right)) / 2
		    + dialog_width / 2);
		     
	    SetRect (dst_rect,
		     left, top,
		     left + dialog_width, top + dialog_height);
	    break;
	  }
	/* else fall through */
      }
      
    case alertPositionMainScreen:
    case dialogPositionMainScreen:
    case alertPositionParentWindowScreen:
    case dialogPositionParentWindowScreen:
      SetRect (dst_rect,
	       (screen_width - dialog_width) / 2,
	       (screen_height - dialog_height) / 3,
	       (screen_width - dialog_width) / 2 + dialog_width,
	       (screen_height - dialog_height) / 3 + dialog_height);
      break;
    }
}

P3 (PUBLIC pascal trap, DialogPtr, GetNewDialog, INTEGER, id,	/* IMI-413 */
    Ptr, dst, WindowPtr, behind)
{
  dlogh dialog_res_h;
  Handle dialog_item_list_res_h;
  Handle item_ctab_res_h;
  DialogPtr retval;
  Handle dialog_ctab_res_h;
  boolean_t color_p;
  
  dialog_res_h = (dlogh) ROMlib_getrestid (TICK ("DLOG"), id);
  
  dialog_item_list_res_h = ROMlib_getrestid (TICK ("DITL"),
					     Hx (dialog_res_h, dlgditl));
  dialog_item_list_res_h = ROMlib_copy_handle (dialog_item_list_res_h);
  
  if (!dialog_res_h || !dialog_item_list_res_h)
    return NULL;
  
  dialog_ctab_res_h = ROMlib_getrestid (TICK ("dctb"), id);
  item_ctab_res_h = ROMlib_getrestid (TICK ("ictb"), id);
  
  color_p = (dialog_ctab_res_h || item_ctab_res_h);
  
  LOCK_HANDLE_EXCURSION_1
    (dialog_res_h,
     {
       Rect adjusted_rect;
       
       dialog_compute_rect (&HxX (dialog_res_h, dlgr),
			    &adjusted_rect,
			    (DIALOG_RES_HAS_POSITION_P (dialog_res_h)
			     ? DIALOG_RES_POSITION (dialog_res_h)
			     : noAutoCenter));
       
       /* if there is a dialog color table resource make this a color
	  dialog */
       retval
	 = ROMlib_new_dialog_common ((DialogPtr) dst, color_p,
				     (CTabHandle) dialog_ctab_res_h,
				     item_ctab_res_h,
				     &adjusted_rect,
				     (StringPtr) (&HxX (dialog_res_h, dlglen)),
				     Hx (dialog_res_h, dlgvis), 
				     Hx (dialog_res_h, dlgprocid),
				     behind,
				     Hx (dialog_res_h, dlggaflag),
				     Hx (dialog_res_h, dlgrc),
				     dialog_item_list_res_h);
     });
  
  return retval;
}

P1(PUBLIC pascal trap, void, CloseDialog, DialogPtr, dp)	/* IMI-413 */
{
  Handle items;
  GUEST<INTEGER> *ip;
  INTEGER i;
  itmp itp;
  
  items = DIALOG_ITEMS (dp);
  if (items)
    {
      /* #### should `items' be locked? */
      ip = (GUEST<INTEGER> *) STARH (items);
      i = CW(*ip);
      itp = (itmp)(ip + 1);
      while (i-- >= 0)
	{
	  if (CB (itp->itmtype) & (editText | statText))
	    DisposHandle ((Handle) MR(itp->itmhand));
	  BUMPIP (itp);
	}
    }
  CloseWindow((WindowPtr) dp);
}

P1 (PUBLIC pascal trap, void, DisposDialog, DialogPtr, dp)	/* IMI-415 */
{
  TEHandle teh;
  
  CloseDialog (dp);
  DisposHandle(MR(((DialogPeek)dp)->items));
  teh = DIALOG_TEXTH (dp);
  
  /* accounted for elsewhere */
  TE_HTEXT_X (teh) = NULL;
  TEDispose (teh);
  
  DisposPtr ((Ptr) dp);
}

/* see dialAlert.c for CouldDialog, FreeDialog */

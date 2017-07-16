/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialItem[] =
	"$Id: dialItem.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "QuickDraw.h"
#include "DialogMgr.h"
#include "Iconutil.h"

#include "rsys/itm.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;
using namespace ByteSwap;

#define _GetDItem(dp, item_no, item_type, item_h, item_rect)		\
  ({									\
    HIDDEN_Handle __item_handle;					\
									\
    GetDItem (dp, item_no, item_type, &__item_handle, item_rect);	\
    *(item_h) = MR (__item_handle.p);					\
  })
  

void
Executor::AppendDITL (DialogPtr dp, Handle new_items_h, DITLMethod method)
{
  Handle items_h;
  int items_h_size, new_items_h_size;
  Rect *dp_port_rect;
  int width, height;
  boolean_t resize_p;
  Point base_pt;
  
  dp_port_rect = &PORT_RECT (dp);
  width = RECT_WIDTH (dp_port_rect);
  height = RECT_HEIGHT (dp_port_rect);
  
  if (method < 0)
    {
      int16 item_type;
      Handle item_h;
      Rect item_rect;
      
      method = -method;
      
      _GetDItem (dp, method, &item_type, &item_h, &item_rect);

      resize_p = FALSE;
      base_pt.v = BigEndianValue (item_rect.top);
      base_pt.h = BigEndianValue (item_rect.left);
    }
  else
    {
      switch (method)
	{
	default:
	case overlayDITL:
	  resize_p = FALSE;
	  base_pt.h = base_pt.v = 0;
	  break;
      
	case appendDITLRight:
	  resize_p = TRUE;
	  base_pt.v = 0;
	  base_pt.h = BigEndianValue (dp_port_rect->right);
	  break;
      
	case appendDITLBottom:
	  resize_p = TRUE;
	  base_pt.v = BigEndianValue (dp_port_rect->bottom);
	  base_pt.h = 0;
	  break;
	}
    }
  
  items_h = DIALOG_ITEMS (dp);
  
  {
    /* compute the size of the current item list (and where the new
       items will go */
    
    char *base_itemp;
    int item_count;
    itmp itemp;
    int i;
    
    base_itemp = (char *) STARH (items_h);
    item_count = BigEndianValue (*(int16 *) base_itemp) + 1;
    itemp = (itmp) ((int16 *) STARH (items_h) + 1);
    
    for (i = 0; i < item_count; i ++)
      itemp = (itmp) ((char *) itemp + ITEM_LEN (itemp));
    
    items_h_size = (char *) itemp - base_itemp;
  }

  {
    Size items_h_handle_size;

    items_h_handle_size = GetHandleSize (items_h);
    if (items_h_size != items_h_handle_size)
      warning_unexpected ("items_h_size = %d, items_h_handle_size = %d",
			  items_h_size, items_h_handle_size);
  }

  new_items_h_size = GetHandleSize (new_items_h);
  
  SetHandleSize (items_h, items_h_size + new_items_h_size);
  
  LOCK_HANDLE_EXCURSION_2
    (items_h, new_items_h,
     {
       itmp new_itemp;
       char *base_itemp;
       char *base_new_itemp;
       int item_count;
       int new_item_count;
       int i;
       
       base_itemp = (char *) STARH (items_h);
       item_count = BigEndianValue (*(int16 *) base_itemp) + 1;
       
       base_new_itemp = (char *) STARH (new_items_h);
       new_itemp = (itmp) ((int16 *) STARH (new_items_h) + 1);
       new_item_count = BigEndianValue (*(int16 *) base_new_itemp) + 1;
       
       /* update the count for the new items */
       *(int16 *) base_itemp = BigEndianValue (item_count + new_item_count - 1);
       
       THEPORT_SAVE_EXCURSION
	 (dp,
	  {
	    
	    for (i = 0; i < new_item_count; i ++)
	      {
		itmp itemp;
		
		itemp = (itmp) (base_itemp + items_h_size
				+ (  (char *) new_itemp
				   - (base_new_itemp + sizeof (int16))));
		
		dialog_create_item ((DialogPeek) dp, itemp, new_itemp,
				    (i + 1) + item_count, base_pt);
	   
		InvalRect (&itemp->itmr);
		
		width  = MAX (width,  BigEndianValue (itemp->itmr.left));
		height = MAX (height, BigEndianValue (itemp->itmr.bottom));
		
		BUMPIP (new_itemp);
	      }
	  });
     });
  
  if (resize_p)
    SizeWindow ((WindowPtr) dp, width, height,
		/* cause the window to be redraw */
		TRUE);
}

void
Executor::ShortenDITL (DialogPtr dp, int16 n_items)
{
  Handle item_h;
  char *base_itemp;
  itmp itemp;
  int count, i;
  int item_h_size;
  int first_item_to_dispose;
  
  item_h = DIALOG_ITEMS (dp);
  if (item_h == NULL)
    return;
  
  item_h_size = GetHandleSize (item_h);
  
  LOCK_HANDLE_EXCURSION_1
    (item_h,
     {  
       base_itemp = (char *) STARH (item_h);
       itemp = (itmp) ((int16 *) STARH (item_h) + 1);
       count = BigEndianValue (*(int16 *) base_itemp) + 1;
       
       if (count < n_items)
	 n_items = count;
       first_item_to_dispose = count - n_items;
       
       for (i = 0; i < count; i ++, BUMPIP (itemp))
	 {
	   if (i == first_item_to_dispose)
	     item_h_size = (char *) itemp - (char *) base_itemp;
	   
	   if (i >= first_item_to_dispose)
	     {
	       boolean_t erase_p = FALSE;
	       Rect erase_rect;
	       
	       erase_rect = itemp->itmr;
	       
	       if (itemp->itmtype & (editText | statText))
		 {
		   if (DIALOG_EDIT_FIELD (dp) == i)
		     {
		       TEHandle te;
		       
		       te = DIALOG_TEXTH (dp);
		       
		       /* this te currently points to some bogus
			  fields, but before it is used again, those
			  fields will be set up with sane values */
		       TEDeactivate (te);
		       
		       DIALOG_EDIT_FIELD_X (dp) = CWC (-1);
		       DIALOG_EDIT_OPEN_X (dp) = CWC (0);
		     }
		   
		   DisposHandle ((Handle) MR (itemp->itmhand));
		   /* when editText items are drawn, the box around
		      them is inset `-3', so we also need to do that
		      when erasing */
		   InsetRect (&erase_rect, -3, -3);
		   erase_p = TRUE;
		 }
	       else if (itemp->itmtype & ctrlItem)
		 {
		   DisposeControl ((ControlHandle) MR (itemp->itmhand));
		 }
	       else if (itemp->itmtype & iconItem)
		 {
		   Handle icon;
		   
		   icon = itemp->itmhand;
		   if (CICON_P (icon))
		     DisposeCIcon ((CIconHandle) icon);
		   erase_p = TRUE;
		 }
	       else if (itemp->itmtype & picItem)
		 {
		   erase_p = TRUE;
		 }
	       else
		 {
		   /* user item */
		   erase_p = TRUE;
		 }
	       
	       if (erase_p)
		 {
		   /* #### do we erase with `EraseRect ()' or the
                      window background color defined by the window
                      color table? */
		   THEPORT_SAVE_EXCURSION
		     (dp, { EraseRect (&erase_rect); });
		 }
	     }
	 }
       *(int16 *) base_itemp = BigEndianValue (first_item_to_dispose - 1);
     });
  
  SetHandleSize ((Handle) item_h, item_h_size);
}
    
int16
Executor::CountDITL (DialogPtr dp)
{
  Handle items;
  int16 count;
  
  items = DIALOG_ITEMS (dp);
  count = BigEndianValue (*(int16 *) STARH (items)) + 1;
  
  return count;
}

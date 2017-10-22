/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qCursor[] =
		"$Id: qCursor.c 87 2005-05-25 01:57:33Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
/* ack */
#include "WindowMgr.h"
#include "CQuickDraw.h"
#include "EventMgr.h"
#include "OSEvent.h"
#include "MemoryMgr.h"

#include "rsys/notmac.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/resource.h"
#include "rsys/host.h"

#include "rsys/osevent.h"

#include "rsys/quick.h"

#if !defined (CURSOR_DEBUG)

using namespace Executor;

#define HOST_SET_CURSOR(d,m,x,y) host_set_cursor (d,m,x,y)

#else

#include "rsys/dirtyrect.h"
#include "rsys/vdriver.h"

using namespace Executor;

PRIVATE void
cursor_debug (const uint8 *datap, const uint8 *maskp, int hot_x, int hot_y)
{
  int y;
  uint8 *pixel;
  int offset;

  pixel = vdriver_fbuf;
  offset = vdriver_row_bytes - 16;
  for (y = 0; y < 16; ++y)
    {
      uint16 u, bit;

      u = *datap++;
      u = (u << 8) | *datap++;
      for (bit = 1 << 15; bit; bit >>= 1)
	*pixel++ = u & bit ? 0 : 255;
      pixel += offset;
    }
  dirty_rect_accrue (0, 0, 16, 16);
  dirty_rect_update_screen ();
  vdriver_flush_display();
}

#define HOST_SET_CURSOR(d,m,x,y)		\
do						\
{						\
  cursor_debug (d, m, x, y);			\
  host_set_cursor (d, m, x, y);			\
}						\
while (FALSE)

#endif


static CCrsrHandle current_ccrsr;
static Cursor current_crsr;

static boolean_t current_cursor_valid_p = FALSE;
static int current_cursor_color_p;

void
Executor::cursor_reset_current_cursor (void)
{
  current_cursor_valid_p = FALSE;
  if (current_cursor_color_p)
    SetCCursor (current_ccrsr);
  else
    SetCursor (&current_crsr);
}

A0 (PUBLIC, void, ROMlib_showcursor)
{
  if (!CrsrVis)
    {
      host_set_cursor_visible (TRUE);
      CrsrVis = TRUE;
    }
}

A0 (PUBLIC, void, ROMlib_restorecursor)
{
  if (CrsrVis)
    {
      host_set_cursor_visible (FALSE);
      CrsrVis = FALSE;
    }
}

A0(PUBLIC, void, ROMlib_showhidecursor)				/* INTERNAL */
{
    if (CrsrState == CWC(0))
	ROMlib_showcursor();
    else
	ROMlib_restorecursor();
}

P1(PUBLIC pascal trap, void, SetCursor, Cursor *, cp)
{
  if (current_cursor_valid_p
      && !current_cursor_color_p
      && ! memcmp (&current_crsr, cp, sizeof (Cursor)))
    return;
  
  if (host_cursor_depth == 1)
    {
      HOST_SET_CURSOR ((char *) cp->data, (unsigned short *) cp->mask,
		       CW (cp->hotSpot.h), CW (cp->hotSpot.v));
    }
  else
    {
      PixMap data_pixmap, target_pixmap;
      char *data_baseaddr;
      /* rowbytes of expanded cursor data */
      int rowbytes;
      
      data_pixmap.baseAddr = RM ((Ptr) cp->data);
      data_pixmap.rowBytes = CWC (2);
      data_pixmap.bounds = ROMlib_cursor_rect;
      data_pixmap.cmpCount = CWC (1);
      data_pixmap.pixelType = CWC (0);
      data_pixmap.pixelSize = data_pixmap.cmpSize = CWC (1);
      data_pixmap.pmTable = RM (validate_relative_bw_ctab ());
      
      rowbytes = (16 * host_cursor_depth) / 8;
      data_baseaddr = (char*)alloca (16 * rowbytes);
      
      target_pixmap.baseAddr = RM ((Ptr) data_baseaddr);
      target_pixmap.rowBytes = CWC (rowbytes);
      target_pixmap.bounds = ROMlib_cursor_rect;
      target_pixmap.cmpCount = CWC (1);
      target_pixmap.pixelType = CWC (0);
      target_pixmap.pixelSize = target_pixmap.cmpSize 
	= CW (host_cursor_depth);
      /* the target pixmap colortable is not used by `convert_pixmap ()'
	 target_pixmap.pmTable = ...; */
      
      convert_pixmap (&data_pixmap, &target_pixmap,
		      &ROMlib_cursor_rect, NULL);
      
      HOST_SET_CURSOR (data_baseaddr, (unsigned short *) cp->mask,
		       CW (cp->hotSpot.h), CW (cp->hotSpot.v));
    }
  
  current_crsr = *cp;
  current_cursor_color_p = FALSE;
  current_cursor_valid_p = TRUE;
  
  if (CrsrState == CWC(0))
    {
      CrsrVis = FALSE;
      ROMlib_showcursor();
    }
}

P0(PUBLIC pascal trap, void, InitCursor)
{
    CrsrState = 0;
    SetCursor(&arrowX);
    CrsrVis = FALSE;
    ROMlib_showcursor();
}

P0(PUBLIC pascal trap, void, HideCursor)	/* IMI-168 */
{
    ROMlib_restorecursor();
    CrsrState = CW(CW(CrsrState) - 1);
}

P0(PUBLIC pascal trap, void, ShowCursor)	/* IMI-168 */
{
    if ( (CrsrState = CW(CW(CrsrState) + 1)) == CWC(0) )
	ROMlib_showcursor();
    if (CW(CrsrState) > 0)
	CrsrState = 0;
}

namespace Executor {
	PRIVATE void wewantpointermovements(INTEGER);
}

A1(PRIVATE, void, wewantpointermovements, INTEGER, x)
{
    CrsrState = CW(CW(CrsrState) + x);
    ROMlib_bewaremovement = TRUE;
}

P0(PUBLIC pascal trap, void, ObscureCursor)	/* IMI-168 */
{
    ROMlib_restorecursor();
    wewantpointermovements(0);
}

P2(PUBLIC pascal trap, void, ShieldCursor, Rect *, rp, Point, p) /* IMI-474 */
{
    EventRecord evt;
    Point ep;

    GetOSEvent(0, &evt);
    ep.v = CW(evt.where.v) + p.v;
    ep.h = CW(evt.where.h) + p.h;
    if (PtInRect(ep, rp))
	HideCursor();
    else
	wewantpointermovements(-1);
}


typedef GUEST<ccrsr_res_ptr> *ccrsr_res_handle;

P1 (PUBLIC pascal trap, CCrsrHandle, GetCCursor, INTEGER, crsr_id)
{
  CCrsrHandle ccrsr_handle;
  ccrsr_res_handle res_handle;

  ccrsr_handle = (CCrsrHandle) NewHandle (sizeof (CCrsr));
  
  res_handle = (ccrsr_res_handle) ROMlib_getrestid (TICK ("crsr"), crsr_id);
  if (res_handle == NULL)
    return NULL;

  LOCK_HANDLE_EXCURSION_2
    (ccrsr_handle, res_handle,
     {
       ccrsr_res_ptr resource;
       CCrsrPtr ccrsr;
       CTabPtr tmp_ctab;
       int ccrsr_data_size;
       int ccrsr_data_offset;
       int ccrsr_ctab_size;
       int ccrsr_ctab_offset;
       
       int cursor_pixel_map_offset;
       PixMapPtr cursor_pixel_map_resource;
       PixMapHandle cursor_pixel_map;
       
       GUEST<Handle> h;
       
       resource = STARH (res_handle);
       ccrsr = STARH (ccrsr_handle);
       
       BlockMove ((Ptr) &resource->crsr, (Ptr) ccrsr,
		  sizeof (CCrsr));
       
       /* NOTE: use CL below instead of MR because they're overloading
	  crsrMap to have an offset rather than a handle */

       cursor_pixel_map_offset = CL (guest_cast<int32_t> (ccrsr->crsrMap));
       cursor_pixel_map_resource
	 = (PixMapPtr) ((char *) resource + cursor_pixel_map_offset);
       
       cursor_pixel_map = NewPixMap ();
       ccrsr->crsrMap = RM (cursor_pixel_map);
       BlockMove ((Ptr) cursor_pixel_map_resource,
		  (Ptr) STARH (cursor_pixel_map),
		  sizeof *cursor_pixel_map_resource);
       
       ccrsr_data_offset = CL (guest_cast<int32_t>(ccrsr->crsrData));
       
       ccrsr_ctab_offset = (int) PIXMAP_TABLE_AS_OFFSET (MR (ccrsr->crsrMap));
       ccrsr_data_size = ccrsr_ctab_offset - ccrsr_data_offset;
       
       ccrsr->crsrData = RM (NewHandle (ccrsr_data_size));
       BlockMove ((Ptr) resource + ccrsr_data_offset,
		  STARH (MR (ccrsr->crsrData)),
		  ccrsr_data_size);
       
       tmp_ctab = (CTabPtr) ((char *) resource + ccrsr_ctab_offset);
       ccrsr_ctab_size = CTAB_STORAGE_FOR_SIZE (CW (tmp_ctab->ctSize));
       h = RM (NewHandle (ccrsr_ctab_size));
       PIXMAP_TABLE_X (MR (ccrsr->crsrMap)) = guest_cast<CTabHandle> (h);
       BlockMove ((Ptr) tmp_ctab,
		  (Ptr) STARH (PIXMAP_TABLE (MR (ccrsr->crsrMap))),
		  ccrsr_ctab_size);
     });
  
  return ccrsr_handle;
}

Rect Executor::ROMlib_cursor_rect =
{
  CWC (0), CWC (0), CWC (16), CWC (16),
};

/* ### this isn't a cursor specific routine, and it may be helpful
   elsewhere, but since it isn't clear exactly what to compare, we'll
   leave it here for now */

static boolean_t
pixmap_eq_p (PixMapHandle pm0, PixMapHandle pm1)
{
  return (PIXMAP_PIXEL_SIZE_X (pm0) == PIXMAP_PIXEL_SIZE_X (pm1)
	  && PIXMAP_PIXEL_TYPE_X (pm0) == PIXMAP_PIXEL_TYPE_X (pm1)
	  && PIXMAP_CMP_COUNT_X (pm0) == PIXMAP_CMP_COUNT_X (pm1)
	  && PIXMAP_CMP_SIZE_X (pm0) == PIXMAP_CMP_SIZE_X (pm1)
	  && (CTAB_SEED_X
	      (PIXMAP_TABLE (pm0)) == CTAB_SEED_X (PIXMAP_TABLE (pm1)))
	  && !memcmp (&PIXMAP_BOUNDS (pm0),
		      &PIXMAP_BOUNDS (pm1),
		      sizeof (Rect)));
}

P1 (PUBLIC pascal trap, void, SetCCursor,
    CCrsrHandle, ccrsr)
{
  if (current_cursor_valid_p
      && current_cursor_color_p)
    {
      int current_ccrsr_data_size, ccrsr_data_size;
      
      current_ccrsr_data_size = GetHandleSize (CCRSR_DATA (current_ccrsr));
      ccrsr_data_size = GetHandleSize (CCRSR_DATA (ccrsr));
      
      if (current_ccrsr_data_size == ccrsr_data_size
	  && !memcmp (STARH (CCRSR_DATA (current_ccrsr)),
		      STARH (CCRSR_DATA (ccrsr)),
		      ccrsr_data_size)
	  && CCRSR_TYPE_X (current_ccrsr) == CCRSR_TYPE_X (ccrsr)
	  && !memcmp (CCRSR_1DATA (current_ccrsr),
		      CCRSR_1DATA (ccrsr),
		      sizeof (Bits16))
	  && !memcmp (CCRSR_MASK (current_ccrsr),
		      CCRSR_MASK (ccrsr),
		      sizeof (Bits16))
	  && !memcmp (&CCRSR_HOT_SPOT (current_ccrsr),
		      &CCRSR_HOT_SPOT (ccrsr),
		      sizeof (Point))
	  && pixmap_eq_p (CCRSR_MAP (current_ccrsr),
			  CCRSR_MAP (ccrsr)))
	return;
    }
  
  LOCK_HANDLE_EXCURSION_1
    ((Handle) ccrsr,
     {
       GDHandle gdev;
       PixMapHandle gd_pmap;
       GUEST<Point> *hot_spot;

       gdev = MR (MainDevice);
       gd_pmap = GD_PMAP (gdev);
       
       hot_spot = &CCRSR_HOT_SPOT (ccrsr);
       
       if (host_cursor_depth > 2)
	 {
	   Handle ccrsr_xdata;

	   ccrsr_xdata = CCRSR_XDATA (ccrsr);
	   if (!ccrsr_xdata)
	     {
	       ccrsr_xdata = NewHandle (32 * host_cursor_depth);
	       CCRSR_XDATA_X (ccrsr) = RM (ccrsr_xdata);
	     }
      
	   if (CCRSR_XVALID_X (ccrsr) == CWC (0)
	       || (CCRSR_XVALID (ccrsr) != host_cursor_depth)
	       || (CCRSR_ID_X (ccrsr) != CTAB_SEED_X (PIXMAP_TABLE (gd_pmap))))
	     {
	       SetHandleSize (ccrsr_xdata, 32 * host_cursor_depth);
	       
	       LOCK_HANDLE_EXCURSION_2
		 (CCRSR_MAP (ccrsr), CCRSR_DATA (ccrsr),
		  {
		    PixMap src;
		    PixMap ccrsr_xmap;

		    /* only fields used by `convert_pixmap ()',
		       baseAddr is filled in below */
		    memset (&ccrsr_xmap, 0, sizeof ccrsr_xmap);
		    ccrsr_xmap.rowBytes = CW (2 * host_cursor_depth);
		    ccrsr_xmap.pixelSize = CW (host_cursor_depth);
		    
		    src = *STARH (CCRSR_MAP (ccrsr));
		    src.baseAddr = *CCRSR_DATA (ccrsr);
		    LOCK_HANDLE_EXCURSION_1
		      (ccrsr_xdata,
		       {
			 ccrsr_xmap.baseAddr = *ccrsr_xdata;
			 convert_pixmap (&src, &ccrsr_xmap,
					 &ROMlib_cursor_rect, NULL);
		       });
		  });
	       CCRSR_XVALID_X (ccrsr) = CW (host_cursor_depth);
	       CCRSR_ID_X (ccrsr) = CTAB_SEED_X (PIXMAP_TABLE (gd_pmap));
	     }
      
	   /* Actually set the current cursor. */
	   LOCK_HANDLE_EXCURSION_1
	     (ccrsr_xdata,
	      {
		HOST_SET_CURSOR ((char *) STARH (ccrsr_xdata),
				 (unsigned short *) CCRSR_MASK (ccrsr),
				 CW (hot_spot->h), CW (hot_spot->v));
	      });
	 }
       else
	 {
	   HOST_SET_CURSOR ((char *) CCRSR_1DATA (ccrsr),
			    (unsigned short *) CCRSR_MASK (ccrsr),
			    CW (hot_spot->h), CW (hot_spot->v));
	 }
     });

  /* copy the cursor so if there is a depth change, we have the cursor
     data around to reset the cursor; a pain in the butt */
  if (current_ccrsr != ccrsr)
    {
      int data_size;
    
      if (!current_ccrsr)
	{
	  ZONE_SAVE_EXCURSION
	    (SysZone,
	     {
	       current_ccrsr = (CCrsrHandle) NewHandle (sizeof (CCrsr));
	       CCRSR_DATA_X (current_ccrsr) = RM (NewHandle (0));
	       CCRSR_XDATA_X (current_ccrsr) = (Handle)RM (NULL);
	       CCRSR_MAP_X (current_ccrsr) = RM (NewPixMap ());
	     });
	}
    
      /* copy the cursor structure */
      CCRSR_TYPE_X (current_ccrsr) = CCRSR_TYPE_X (ccrsr);
      memcpy (CCRSR_1DATA (current_ccrsr), CCRSR_1DATA (ccrsr),
	      sizeof (Bits16));
      memcpy (CCRSR_MASK (current_ccrsr), CCRSR_MASK (ccrsr),
	      sizeof (Bits16));
      CCRSR_HOT_SPOT (current_ccrsr) = CCRSR_HOT_SPOT (ccrsr);
    
      /* copy the pixmap */
      CopyPixMap (CCRSR_MAP (ccrsr), CCRSR_MAP (current_ccrsr));
    
      /* copy the cursor data */
      data_size = GetHandleSize (CCRSR_DATA (ccrsr));
      SetHandleSize (CCRSR_DATA (current_ccrsr), data_size);
      memcpy (STARH (CCRSR_DATA (current_ccrsr)),
	      STARH (CCRSR_DATA (ccrsr)), data_size);
    
      /* invalidate this cursor */
      CCRSR_XVALID_X (current_ccrsr) = CWC (0);
      CCRSR_ID_X (current_ccrsr) = CLC (-1);
      
      current_cursor_valid_p = TRUE;
      current_cursor_color_p = TRUE;
    }
}

P1 (PUBLIC pascal trap, void, DisposCCursor,
    CCrsrHandle, ccrsr)
{
/* #warning "implement DisposCCursor" */
  warning_unimplemented (NULL_STRING);
}

P0 (PUBLIC pascal trap, void, AllocCursor)
{
  /* This function is a NOP for us as far as I know. */
  warning_unexpected ("AllocCursor called -- this is unusual");
}

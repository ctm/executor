/* Copyright 1995, 1996, 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_splash[] =
	    "$Id: splash.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"

#if defined (DISPLAY_SPLASH_SCREEN)

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ToolboxEvent.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/splash.h"
#include "rsys/vdriver.h"
#include "rsys/cquick.h"
#include "rsys/slash.h"
#include "rsys/notmac.h"
#include "rsys/mman.h"
#include "rsys/region.h"
#include "rsys/srcblt.h"
#include "rsys/custom.h"

static int bpp;
static int log2_bpp;

boolean_t
splash_screen_display (boolean_t button_p, char *basename)
{
  struct splash_screen_header header;
  struct splash_screen_color color_buf[256];
  int splash_top, splash_left;
  uint8 bg_pixel;
  char *p;

  if (!ROMlib_splashp)
    {
      warning_unexpected ("no splash");
      return FALSE;
    }
  
  header = *(struct splash_screen_header *) ROMlib_splashp->chars;

  bpp = CL (header.bpp);
  log2_bpp = CL (header.log2_bpp);
  
  memcpy (color_buf, ROMlib_splashp->chars + CL (header.color_offset),
	  CL (header.color_count) * sizeof *color_buf);

  bg_pixel = ((bpp == vdriver_bpp)
	      ? header.bg_pixel
	      : (header.bg_pixel
		 ? ((1 << vdriver_bpp) - 1)
		 : 0));
  switch (vdriver_bpp)
    {
    case 1:
      bg_pixel = bg_pixel | (bg_pixel << 1);
    case 2:
      bg_pixel = bg_pixel | (bg_pixel << 2);
    case 4:
      bg_pixel = bg_pixel | (bg_pixel << 4);
    case 8:
      break;
    }

  if (vdriver_bpp == bpp)
    vdriver_set_colors (0, CL (header.color_count), (ColorSpec *) color_buf);
  
  splash_top  = (vdriver_height - SPLASH_SCREEN_HEIGHT) / 2;
  splash_left = (vdriver_width - SPLASH_SCREEN_WIDTH) / 2;

  /* We can't handle a splash screen bigger than the real screen yet. */
  if (splash_top < 0 || splash_left < 0)
    return FALSE;
  
  {
    int splash_row_bytes;
    uint8 *tmp_buf;
    int i;
    blt_bitmap_t blank_bitmap, src_row_bitmap, screen_bitmap;
    Point src_origin, dst_origin;
    RgnHandle row_rgn;

#define ROWS_PER_PASS 32

    p = (char *) ROMlib_splashp->chars + CL (header.splash_bits_offset);

    splash_row_bytes = SPLASH_SCREEN_WIDTH >> (3 - log2_bpp);
    tmp_buf = alloca (vdriver_row_bytes * ROWS_PER_PASS);

    /* Set up phony bitmap for screen. */
    screen_bitmap.baseAddr = RM ((Ptr)vdriver_fbuf);
    screen_bitmap.rowBytes = CW (vdriver_row_bytes);
    SetRect (&screen_bitmap.bounds, 0, 0, vdriver_width, ROWS_PER_PASS);

    src_origin.h = src_origin.v = CWC (0);
    dst_origin.h = dst_origin.v = CWC (0);

    /* Set up a region for the one row in question. */
    ZONE_SAVE_EXCURSION
      (SysZone,
       {
	 row_rgn = NewRgn ();
	 RGN_BBOX (row_rgn) = screen_bitmap.bounds;
       });

    /* Clear the screen if there's a border. */
    if (SPLASH_SCREEN_HEIGHT != vdriver_height
	|| SPLASH_SCREEN_WIDTH != vdriver_width)
      {
	/* Set up phony bitmap to clear screen. */
	blank_bitmap.baseAddr = RM ((Ptr)tmp_buf);
	blank_bitmap.rowBytes = CW (vdriver_row_bytes);
	SetRect (&blank_bitmap.bounds, 0, 0, vdriver_width, ROWS_PER_PASS);
	memset (tmp_buf, bg_pixel, vdriver_row_bytes * ROWS_PER_PASS);

	for (i = 0; i < vdriver_height; i += ROWS_PER_PASS)
	  {
	    int num_rows;
	    
	    num_rows = MIN (ROWS_PER_PASS, vdriver_height - i);
	    (RGN_BBOX (row_rgn)).bottom = CW (num_rows);
	    srcblt_rgn (row_rgn, srcCopy, vdriver_log2_bpp,
			&blank_bitmap, &screen_bitmap,
			&src_origin, &dst_origin, ~0, 0);
	    
	    /* Move on to the next row. */
#if 0
	    SWAPPED_OPL (screen_bitmap.baseAddr, +,
			 vdriver_row_bytes * num_rows);
#else
	    screen_bitmap.baseAddr = RM(MR(screen_bitmap.baseAddr) +
			 vdriver_row_bytes * num_rows);
#endif
	  }
      }

    /* Set up phony bitmap for src row. */
    src_row_bitmap.baseAddr = RM ((Ptr)tmp_buf);
    src_row_bitmap.rowBytes = CW (SPLASH_SCREEN_WIDTH
				  >> (3 - vdriver_log2_bpp));
    SetRect (&src_row_bitmap.bounds, 0, 0, SPLASH_SCREEN_WIDTH, ROWS_PER_PASS);
    RGN_BBOX (row_rgn) = src_row_bitmap.bounds;

    SWAPPED_OPW (screen_bitmap.bounds.left, -, splash_left);
    screen_bitmap.bounds.top = CW (-splash_top);
    screen_bitmap.baseAddr = RM ((Ptr)vdriver_fbuf);

    /* Actually read and display the splash screen bits. */
    for (i = 0; i < SPLASH_SCREEN_HEIGHT; i += ROWS_PER_PASS)
      {
	int num_rows;

	num_rows = MIN (ROWS_PER_PASS, SPLASH_SCREEN_HEIGHT - i);
	(RGN_BBOX (row_rgn)).bottom = CW (num_rows);

	memcpy (tmp_buf, p, splash_row_bytes * num_rows);
	p += splash_row_bytes * num_rows;

	srcblt_rgn (row_rgn, srcCopy, vdriver_log2_bpp,
		    &src_row_bitmap, &screen_bitmap,
		    &src_origin, &dst_origin, ~0, 0);

	/* Move on to the next row. */
	SWAPPED_OPW (screen_bitmap.bounds.top, -, num_rows);
      }

    DisposeRgn (row_rgn);
  }
  
  vdriver_update_screen (0, 0, vdriver_height, vdriver_width, FALSE);
  vdriver_flush_display();
  
  return TRUE;
}

#endif /* DISPLAY_SPLASH_SCREEN */

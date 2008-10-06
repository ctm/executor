/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qColorutil[] =
		"$Id: qColorutil.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"

/* ctab with `zero' as the seed; if StdBits encounters a source pixmap
   with this as its color table, no depth conversion will be
   performed */
CTabHandle ROMlib_dont_depthconv_ctab;

/* black and white color table; element zero is always white, elt one
   always black

   this colortable is always valid */
CTabHandle ROMlib_bw_ctab;

CTabHandle no_stdbits_color_conversion_color_table;

/* fore- and back-ground color table; elt zero is always fore-ground,
   elt one the background

   this color table is validated and returned by
   `validate_fg_bk_ctab ()' */
static CTabHandle ROMlib_fg_bk_ctab;

/* relative black and white color table; elt zero is always the zero
   elt of the (?) current gdevice colortable; elt one is always the
   CTAB_SIZE (gd_ctab) elt of the current gdevice's ctab

   validated and returned by `validate_relative_bw_ctab ()' */
static CTabHandle ROMlib_relative_bw_ctab;

CTabHandle
validate_fg_bk_ctab (void)
{
  int pixel_size;
  PixMapHandle gd_pmap;
  GDHandle gd;
  ColorSpec *ctab_table, *gd_ctab_table;
  RGBColor old_fg, old_bk, *fg, *bk;
  
  gd = MR (TheGDevice);
  gd_pmap = GD_PMAP (gd);
  pixel_size = PIXMAP_PIXEL_SIZE (gd_pmap);
  gd_ctab_table = CTAB_TABLE (PIXMAP_TABLE (gd_pmap));
  
  ctab_table = CTAB_TABLE (ROMlib_fg_bk_ctab);
  fg = &ctab_table[1].rgb;
  bk = &ctab_table[0].rgb;

  old_fg = *fg;
  old_bk = *bk;
  
  if (CGrafPort_p (thePort))
    {
      if (pixel_size > 8)
	{
	  const rgb_spec_t *rgb_spec;
	  
	  rgb_spec = pixel_size == 16 ? &mac_16bpp_rgb_spec
	                              : &mac_32bpp_rgb_spec;
	  ((rgb_spec->pixel_to_rgbcolor)
	   (rgb_spec, PORT_FG_COLOR (thePort), fg));
	  ((rgb_spec->pixel_to_rgbcolor)
	   (rgb_spec, PORT_BK_COLOR (thePort), bk));
	}
      else
	{
	  *fg = gd_ctab_table[PORT_FG_COLOR (thePort)].rgb;
	  *bk = gd_ctab_table[PORT_BK_COLOR (thePort)].rgb;
	}
    }
  else
    {
      /* determine rgb values of the current bk/fg
	 via `QDColors' */
      *fg = *ROMlib_qd_color_to_rgb (PORT_FG_COLOR (thePort));
      *bk = *ROMlib_qd_color_to_rgb (PORT_BK_COLOR (thePort));
    }

  if (memcmp (&old_fg, fg, sizeof old_fg)
      || memcmp (&old_bk, bk, sizeof old_bk))
    CTAB_SEED_X (ROMlib_fg_bk_ctab) = CL (GetCTSeed ());
  
  return ROMlib_fg_bk_ctab;
}

CTabHandle
validate_relative_bw_ctab (void)
{
  GDHandle gd;
  PixMapHandle gd_pmap;
  ColorSpec *ctab_table;
  RGBColor old_entry0, old_entry1, *entry0, *entry1;
  int pixel_size;
  
  gd = MR (TheGDevice);
  gd_pmap = GD_PMAP (gd);
  pixel_size = PIXMAP_PIXEL_SIZE (gd_pmap);
  
  ctab_table = CTAB_TABLE (ROMlib_relative_bw_ctab);
  entry0 = &ctab_table[0].rgb;
  entry1 = &ctab_table[1].rgb;
  
  old_entry0 = *entry0;
  old_entry1 = *entry1;
  
  if (pixel_size <= 8)
    {
      CTabHandle gd_ctab;
      ColorSpec *gd_ctab_table;

      gd_ctab = PIXMAP_TABLE (gd_pmap);
      gd_ctab_table = CTAB_TABLE (gd_ctab);
      
      *entry0 = gd_ctab_table[0].rgb;
      *entry1 = gd_ctab_table[CTAB_SIZE (gd_ctab)].rgb;
    }
  else
    {
      *entry0 = ROMlib_white_rgb_color;
      *entry1 = ROMlib_black_rgb_color;
    }
  
  if (memcmp (&old_entry0, entry0, sizeof old_entry0)
      || memcmp (&old_entry1, entry1, sizeof old_entry1))
    CTAB_SEED_X (ROMlib_relative_bw_ctab) = CL (GetCTSeed ());
  
  return ROMlib_relative_bw_ctab;
}

void
ROMlib_copy_ctab (CTabHandle src, CTabHandle dst)
{
  int ctab_size;

  /* use the handle size of the  source instead of the effective color
     table size or else copying bogus colortables fails */
  ctab_size /* = CTAB_STORAGE_FOR_SIZE (CTAB_SIZE (src)); */
    = GetHandleSize ((Handle) src);
  SetHandleSize ((Handle) dst, ctab_size);
  BlockMove ((Ptr) STARH (src), (Ptr) STARH (dst), ctab_size);
}

void
ROMlib_color_init (void)
{
  ColorSpec *bw_ctab_table;

  ZONE_SAVE_EXCURSION
    (SysZone,
     {
       /* allocate and initialize ROMlib_bw_ctab */
       ROMlib_bw_ctab = (CTabHandle) NewHandle (CTAB_STORAGE_FOR_SIZE (1));
       CTAB_SIZE_X (ROMlib_bw_ctab) = CWC (1);
       CTAB_SEED_X (ROMlib_bw_ctab) = CL (GetCTSeed ());
       CTAB_FLAGS_X (ROMlib_bw_ctab) = CWC (0);
       
       bw_ctab_table = CTAB_TABLE (ROMlib_bw_ctab);
       bw_ctab_table[0].value = CWC (0);
       bw_ctab_table[0].rgb = ROMlib_white_rgb_color;
       bw_ctab_table[1].value = CWC (1);
       bw_ctab_table[1].rgb = ROMlib_black_rgb_color;
       
       /* allocate and initialize ROMlib_fg_bk_ctab */
       ROMlib_fg_bk_ctab = (CTabHandle) NewHandle (CTAB_STORAGE_FOR_SIZE (1));
       /* defer filling in the correct values until they are needed */
       ROMlib_copy_ctab (ROMlib_bw_ctab, ROMlib_fg_bk_ctab);
       
       ROMlib_relative_bw_ctab
	 = (CTabHandle) NewHandle (CTAB_STORAGE_FOR_SIZE (1));
       /* defer filling in correct values until they are needed */
       ROMlib_copy_ctab (ROMlib_bw_ctab, ROMlib_relative_bw_ctab);
       
       ROMlib_dont_depthconv_ctab
	 = (CTabHandle) NewHandle (CTAB_STORAGE_FOR_SIZE (0));
       CTAB_SIZE_X (ROMlib_dont_depthconv_ctab) = CWC (0);
       CTAB_SEED_X (ROMlib_dont_depthconv_ctab) = CWC (0);
       CTAB_FLAGS_X (ROMlib_dont_depthconv_ctab) = CWC (0);

       no_stdbits_color_conversion_color_table
	 = (CTabHandle) NewHandle (sizeof (ColorTable));
       CTAB_SEED_X (no_stdbits_color_conversion_color_table) = CWC (0);
    

     });
}

Handle
ROMlib_copy_handle (Handle src)
{
  Handle retval;
  int handle_size;

  handle_size = GetHandleSize (src);
  retval = NewHandle (handle_size);
  BlockMove (STARH (src), STARH (retval),
	     handle_size);
  return retval;
}

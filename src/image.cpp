/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_image[] =
		"$Id: image.c 88 2005-05-25 03:59:37Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"

#include "rsys/image.h"

using namespace Executor;

#define MAX_ROWBYTES_FOR_WIDTH(w) ((((w) * 32 + 31) / 32) * 4)

pixel_image_t *
Executor::image_init (pixel_image_desc_t *image_desc)
{
  pixel_image_t *retval;
  int i;

  ZONE_SAVE_EXCURSION
    (SysZone,
     {
       retval = (pixel_image_t *) NewPtr (sizeof *retval);
       
       for (i = 0; i < 2; i ++)
	 {
	   PixMapHandle bits;
	   PixMapHandle x_bits;
	   int x_row_bytes;
	   Rect bounds;
	   int bpp;
	   Ptr p;
	   
	   IMAGE_BITS (retval, i) = bits = NewPixMap ();
	   IMAGE_X_BITS (retval, i) = x_bits = NewPixMap ();
	   IMAGE_X_BITS_VALID (retval, i) = -1;
	   
	   PIXMAP_BASEADDR_X (bits) = RM ((Ptr)image_desc->bits[i].raw_bits);
	   PIXMAP_SET_ROWBYTES_X (bits, CW (image_desc->bits[i].row_bytes));
	   bounds.top    = CW (image_desc->bounds.top);
	   bounds.left   = CW (image_desc->bounds.left);
	   bounds.bottom = CW (image_desc->bounds.bottom);
	   bounds.right  = CW (image_desc->bounds.right);
	   retval->bounds = bounds;
	   PIXMAP_BOUNDS (bits) = bounds;
	   bpp = image_desc->bits[i].bpp;
	   PIXMAP_CMP_SIZE_X (bits) = PIXMAP_PIXEL_SIZE_X (bits) = CW (bpp);
	   if (i == 0)
	     {
	       /* the `zero' image is simply a 1bbp black and white image */
	       gui_assert (bpp == 1);
	       ROMlib_copy_ctab (ROMlib_bw_ctab,
				 PIXMAP_TABLE (bits));
	     }
	   else
	     {
	       CTabHandle bits_ctab;
	       ColorSpec *bits_ctab_table;
	       int j;
	       
	       bits_ctab = PIXMAP_TABLE (bits);
	       SetHandleSize ((Handle) bits_ctab,
			      CTAB_STORAGE_FOR_SIZE ((1 << bpp) - 1));
	       CTAB_SIZE_X (bits_ctab) = CW ((1 << bpp) - 1);
	       bits_ctab_table = CTAB_TABLE (bits_ctab);
	       for (j = 0; j <= (1 << bpp) - 1; j ++)
		 bits_ctab_table[j].value = CW (j);
	       CTAB_FLAGS_X (bits_ctab) = CWC (0);
	       CTAB_SEED_X (bits_ctab) = CL (GetCTSeed ());
	     }

	   {
	     int width;
	     int height;

	     width = RECT_WIDTH (&bounds);
	     height = RECT_HEIGHT (&bounds);
	     
	     x_row_bytes = MAX_ROWBYTES_FOR_WIDTH (width);
	     p = NewPtr (x_row_bytes * height);
	     memset (p, 0, x_row_bytes * height);
	     PIXMAP_BASEADDR_X (x_bits) = RM (p);
	     PIXMAP_SET_ROWBYTES_X (x_bits, CW (x_row_bytes));
	   }
	 }
       
       return retval;
     });
}

/* all image_... functions assume thePort as target */
void
Executor::image_copy (pixel_image_t *image, int color_p /* visual */,
	    Rect *dst_rect, int mode)
{
  WRAPPER_PIXMAP_FOR_COPY (wrapper);
  PixMapHandle x_bits;
  Rect x_bits_bounds;
  
  image_validate_x_bits (image, color_p);

  x_bits = IMAGE_X_BITS (image, color_p);
  WRAPPER_SET_PIXMAP_X (wrapper, RM2 (x_bits));
  x_bits_bounds = PIXMAP_BOUNDS (x_bits);

#if 0
  RGBForeColor (&ROMlib_black_rgb_color);
  RGBBackColor (&ROMlib_white_rgb_color);
#endif
  
  CopyBits (wrapper, PORT_BITS_FOR_COPY (thePort),
	    &x_bits_bounds, dst_rect, mode, NULL);
}
     
void
Executor::image_validate_x_bits (pixel_image_t *image, int color_p /* visual */)
{
  GDHandle gdev;
  PixMapHandle bits, x_bits;
  PixMapHandle gd_pixmap;
  CTabHandle gd_pixmap_ctab;
  GUEST<INTEGER> gd_bpp_x;
  int bits_ctab_seed_x;

  gdev = MR (TheGDevice);
  gd_pixmap = GD_PMAP (gdev);
  gd_pixmap_ctab = PIXMAP_TABLE (gd_pixmap);
  gd_bpp_x = PIXMAP_PIXEL_SIZE_X (gd_pixmap);
  
  bits = IMAGE_BITS (image, color_p);
  bits_ctab_seed_x = CTAB_SEED_X (PIXMAP_TABLE (bits)).raw();
  x_bits = IMAGE_X_BITS (image, color_p);
  
  if (IMAGE_X_BITS_VALID (image, color_p) == -1
      || IMAGE_X_BITS_VALID (image, color_p) != bits_ctab_seed_x
      || CTAB_SEED_X (PIXMAP_TABLE (x_bits)) != CTAB_SEED_X (gd_pixmap_ctab)
      || PIXMAP_PIXEL_SIZE_X (x_bits) != gd_bpp_x)
    {
      PIXMAP_CMP_SIZE_X (x_bits) = PIXMAP_PIXEL_SIZE_X (x_bits)
	= gd_bpp_x;
      ROMlib_copy_ctab (gd_pixmap_ctab, PIXMAP_TABLE (x_bits));

      LOCK_HANDLE_EXCURSION_2
	(bits, x_bits,
	 {
	   convert_pixmap (STARH (bits), STARH (x_bits),
			   &PIXMAP_BOUNDS (bits), NULL);
	 });
      
      IMAGE_X_BITS_VALID (image, color_p) = bits_ctab_seed_x;
    }
}

/* assumes color */
void
Executor::image_update_ctab (pixel_image_t *image, const RGBColor *new_colors,
		   int max_color)
{
  PixMapHandle bits;
  CTabHandle bits_ctab;
  ColorSpec *bits_ctab_table;
  int bits_ctab_size;
  int ctab_changed_p;
  int i;

  bits = IMAGE_BITS (image, 1);
  bits_ctab = PIXMAP_TABLE (bits);
  bits_ctab_size = CTAB_SIZE (bits_ctab);
  bits_ctab_table = CTAB_TABLE (bits_ctab);

  ctab_changed_p = 0;
  for (i = MIN (bits_ctab_size, max_color); i >= 0; i --)
    {
      if (memcmp (&bits_ctab_table[i].rgb,
		  &new_colors[i], sizeof bits_ctab_table[i].rgb))
	{
	  ctab_changed_p = 1;
	  bits_ctab_table[i].rgb = new_colors[i];
	}
    }

  if (ctab_changed_p)
    CTAB_SEED_X (bits_ctab) = CL (GetCTSeed ());
}

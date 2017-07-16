/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qIMVxfer[] =
		"$Id: qIMVxfer.c 87 2005-05-25 01:57:33Z ctm $";
#endif


#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "rsys/cquick.h"

using namespace Executor;
using namespace ByteSwap;

/* Helper function: creates a new, sorted table so the value is the
 * same as the index (although we don't bother filling in most of the
 * color table fields).
 */
static CTabPtr
sort_table (CTabPtr old, CTabPtr new1, unsigned max_color)
{
  int i;

  memset (&new1->ctTable, 0, (max_color + 1) * sizeof (ColorSpec));
  for (i = BigEndianValue (old->ctSize); i >= 0; i--)
    new1->ctTable[BigEndianValue (old->ctTable[i].value) & max_color].rgb =
      old->ctTable[i].rgb;

  return new1;
}

/* declare a case in the transform switch */
#define TRANSFORM_CASE(name, macro)				\
  case MUNGE (name, 2, FALSE):					\
    CONVERT_BITS (READ_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  NONPAT_NEXT1,					\
		  macro, 2);					\
    break;							\
  case MUNGE (name, 4, FALSE):					\
    CONVERT_BITS (READ_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  NONPAT_NEXT1,					\
		  macro, 4);					\
    break;							\
  case MUNGE (name, 8, FALSE):					\
    CONVERT_BITS (READ_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  NONPAT_NEXT1,					\
		  macro, 8);					\
    break;							\
  case MUNGE (name, 16, FALSE):					\
    CONVERT_BITS (READ_DIRECT16_PIXEL, READ_DIRECT16_PIXEL,	\
		  WRITE_DIRECT16_PIXEL,				\
		  NONPAT_NEXT1,					\
		  macro, 8);					\
    break;							\
  case MUNGE (name, 32, FALSE):					\
    CONVERT_BITS (READ_DIRECT32_PIXEL, READ_DIRECT32_PIXEL,	\
		  WRITE_DIRECT32_PIXEL,				\
		  NONPAT_NEXT1,					\
		  macro, 8);					\
    break;							\
  case MUNGE (name, 2, TRUE):					\
    CONVERT_BITS (READ_PAT_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  PAT_NEXT1,					\
		  macro, 2);					\
    break;							\
  case MUNGE (name, 4, TRUE):					\
    CONVERT_BITS (READ_PAT_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  PAT_NEXT1,					\
		  macro, 4);					\
    break;							\
  case MUNGE (name, 8, TRUE):					\
    CONVERT_BITS (READ_PAT_INDIRECT_PIXEL, READ_INDIRECT_PIXEL,	\
		  WRITE_INDIRECT_PIXEL,				\
		  PAT_NEXT1,					\
		  macro, 8);					\
    break;							\
  case MUNGE (name, 16, TRUE):					\
    CONVERT_BITS (READ_PAT_DIRECT16_PIXEL, READ_DIRECT16_PIXEL,	\
		  WRITE_DIRECT16_PIXEL,				\
		  PAT_NEXT1,					\
		  macro, 8);					\
    break;							\
  case MUNGE (name, 32, TRUE):					\
    CONVERT_BITS (READ_PAT_DIRECT32_PIXEL, READ_DIRECT32_PIXEL,	\
		  WRITE_DIRECT32_PIXEL,				\
		  PAT_NEXT1,					\
		  macro, 8);					\
    break

void
Executor::convert_transparent (const PixMap *src1, const PixMap *src2,
		     PixMap *dst,
		     const Rect *r1, const Rect *r2,
		     int16 mode,
		     boolean_t tile_src1_p,
		     int pat_x_offset, int pat_y_offset)
{
  unsigned char *src1_row_base, *src2_row_base, *dst_row_base;
  int src1_rowbytes, src2_rowbytes, dst_rowbytes, src1_deltax, src2_deltax;
  int width, height, s1_width, s1_height;
  RGBColor *hilite_rgb;
  uint32 bk_color, hilite_color;
  
  int bits_per_pixel;
  const rgb_spec_t *rgb_spec;
  
  boolean_t copy1_p, copy2_p;
  write_back_data_t write_back1, write_back2;
  
  bits_per_pixel = BigEndianValue (src1->pixelSize);
  rgb_spec = pixmap_rgb_spec (src1);
  
  /* For bits_per_pixel == 1, you are just supposed to use the original
   * boolean transfer modes.
   */
  gui_assert (bits_per_pixel > 1);
  
  copy1_p = pixmap_copy_if_screen (src1, r1, &write_back1);
  if (copy1_p)
    {
      src1 = &write_back1.src_pm;
      r1   = &write_back1.src_rect;
    }
  copy2_p = pixmap_copy_if_screen (src2, r2, &write_back2);
  if (copy2_p)
    {
      src2 = &write_back2.src_pm;
      r2   = &write_back2.src_rect;
    }
  
  PIXMAP_ASSERT_NOT_SCREEN (dst);
  
  if (tile_src1_p)
    {
      s1_width = RECT_WIDTH (&src1->bounds);
      s1_height = RECT_HEIGHT (&src1->bounds);
    }
  else
    {
      s1_width = RECT_WIDTH (r1);
      s1_height = RECT_HEIGHT (r1);
    }
  width  = RECT_WIDTH (r2);
  height = RECT_HEIGHT (r2);
  
  /* Grab the rowbytes for the three bitmaps. */
  src1_rowbytes = BITMAP_ROWBYTES (src1);
  src2_rowbytes = BITMAP_ROWBYTES (src2);
  dst_rowbytes = (width * bits_per_pixel + 31) / 32 * 4;
  dst->rowBytes = BigEndianValue (dst_rowbytes);
  
  /* We want to run x from 0 to width; adding these offsets gives us
   * the real x for the source bitmaps.
   */
  if (tile_src1_p)
    src1_deltax = pat_x_offset;
  else
    src1_deltax = BigEndianValue (r1->left) - BigEndianValue (src1->bounds.left);
  src2_deltax = BigEndianValue (r2->left) - BigEndianValue (src2->bounds.left);

  /* Compute a pointer to the base of the first row of each bitmap. */
  if (tile_src1_p)
    {
      src1_row_base = (unsigned char *)
	(MR (src1->baseAddr)
	 + (src1_rowbytes * (pat_y_offset & (s1_height - 1))));
    }
  else
    {
      src1_row_base = (unsigned char *)
	(MR (src1->baseAddr)
	 + ((BigEndianValue (r1->top) - BigEndianValue (src1->bounds.top)) * src1_rowbytes));
    }
  
  src2_row_base = (unsigned char *)
    (MR (src2->baseAddr)
     + (BigEndianValue (r2->top) - BigEndianValue (src2->bounds.top)) * src2_rowbytes);
  dst_row_base = (unsigned char *) MR (dst->baseAddr);
  
#define RGB_TO_INDIRECT_PIXEL(rgb, pixel)	\
  ((void) ((pixel) = Color2Index (rgb)))
  
#define RGB_TO_DIRECT_PIXEL(bpp, rgb, pixel)		\
  ((void)						\
   ({							\
     uint32 swapped_pixel;				\
     							\
     swapped_pixel = ((*rgb_spec->rgbcolor_to_pixel)	\
		      (rgb_spec, rgb, TRUE));		\
     switch (bpp)					\
       {						\
       case 16:						\
	 (pixel) = BigEndianValue (swapped_pixel);			\
	 break;						\
       case 32:						\
	 (pixel) = BigEndianValue (swapped_pixel);			\
	 break;						\
       default:						\
	 gui_fatal ("unknown bpp");			\
       }						\
   }))
#define RGB_TO_PIXEL(bpp, rgb, pixel)		\
  ((void)					\
   ((bpp) == 32 || (bpp) == 16			\
    ? RGB_TO_DIRECT_PIXEL (bpp, rgb, pixel)	\
    : RGB_TO_INDIRECT_PIXEL (rgb, pixel)))

  if (CGrafPort_p (thePort))
    bk_color = PORT_BK_COLOR (thePort);
  else
    {
      if (active_screen_addr_p (&PORT_BITS (thePort)))
	{
	  int i;
	  
	  /* defaults */
	  bk_color = 0;
	  
	  for (i = 0; i < 8; i ++)
	    if (PORT_BK_COLOR (thePort) == ROMlib_QDColors[i].value)
	      {
		RGB_TO_PIXEL (bits_per_pixel,
			      &ROMlib_QDColors[i].rgb, bk_color);
		break;
	      }
	}
      else
	{
	  /* FIXME: this might not be right */
	  bk_color = (PORT_BK_COLOR (thePort) == whiteColor
		      ? 0
		      : ((1 << bits_per_pixel) - 1));
	}
    }

  if (CGrafPort_p (thePort))
    hilite_rgb = &CPORT_HILITE_COLOR (theCPort);
  else
    hilite_rgb = &HiliteRGB;
  RGB_TO_PIXEL (bits_per_pixel, hilite_rgb, hilite_color);
  
#define CONVERT_BITS(read1, read2, write, next1, transform, bpp)	\
  {									\
    int x, y;								\
									\
    for (y = 0; y < height; y++)					\
      {									\
	for (x = 0; x < width; x++)					\
	  {								\
	    long src1_v, src2_v, dst_v;					\
									\
	    src1_v = read1 (src1_row_base, x + src1_deltax, bpp);	\
	    src2_v = read2 (src2_row_base, x + src2_deltax, bpp);	\
	    dst_v = transform (src1_v, src2_v);				\
	    								\
	    write (dst_v, dst_row_base, x, bpp);			\
	  }								\
									\
	next1;								\
	src2_row_base += src2_rowbytes;					\
	dst_row_base  += dst_rowbytes;					\
      }									\
  }

#define NONPAT_NEXT1 src1_row_base += src1_rowbytes
#define PAT_NEXT1 \
  src1_row_base = (unsigned char *) (MR (src1->baseAddr)		\
		   + (src1_rowbytes * ((y + pat_y_offset) & (s1_height - 1))))

#define SHIFT_COUNT(x, bpp)  (8 - (bpp) - (bpp) * ((x) & (7 / (bpp))))

#define READ_INDIRECT_PIXEL(b, x, bpp)						\
  ((bpp) == 8									\
   ? b[x]									\
   : ((b[(x) * (bpp) / 8] >> SHIFT_COUNT ((x), (bpp))) & ((1 << (bpp)) - 1)))
#define READ_DIRECT16_PIXEL(b, x, bpp)		\
  ((uint16 *) b)[x]
#define READ_DIRECT32_PIXEL(b, x, bpp)		\
  ((uint32 *) b)[x]

#define READ_PAT_INDIRECT_PIXEL(b, x, bpp)	\
  READ_INDIRECT_PIXEL (b, (x) & (s1_width - 1), bpp)
#define READ_PAT_DIRECT16_PIXEL(b, x, bpp)	\
  READ_DIRECT16_PIXEL (b, (x) & (s1_width - 1), bpp)
#define READ_PAT_DIRECT32_PIXEL(b, x, bpp)	\
  READ_DIRECT32_PIXEL (b, (x) & (s1_width - 1), bpp)

#define WRITE_INDIRECT_PIXEL(v, b, x, bpp)				\
  ((void)								\
   ((bpp) == 8 ? b[x] = (v)						\
  : ({									\
       uint8 *p;							\
       p = &b[(x) * (bpp) / 8];						\
       *p &= ~(((1 << (bpp)) - 1) << SHIFT_COUNT (x, bpp));		\
       *p |= (((v) & ((1 << (bpp)) - 1)) << SHIFT_COUNT (x, bpp));	\
    })))
#define WRITE_DIRECT16_PIXEL(v, b, x, bpp)	\
  ((void)					\
   (((uint16 *) (b))[(x)] = (v)))
#define WRITE_DIRECT32_PIXEL(v, b, x, bpp)	\
  ((void)					\
   (((uint32 *) (b))[(x)] = (v)))

#define TRANSPARENT_TRANSFORM(src1_v, src2_v)	\
  (((typeof (bk_color)) src1_v != bk_color) ? src1_v : src2_v)

#define HILITE_TRANSFORM(src1_v, src2_v)				\
({									\
  long v;								\
									\
  /* only invert the pixel if the src/pattern is `on', ie., not the	\
     background color */						\
  if ((typeof (bk_color))src1_v == bk_color)				\
    v = src2_v;								\
  else									\
    {									\
      if ((typeof (bk_color)) src2_v == bk_color)			\
	v = hilite_color;						\
      else if ((typeof (hilite_color)) src2_v == hilite_color)		\
	v = bk_color;							\
      else								\
	v = src2_v;							\
    }									\
  v;									\
})
  
/* Silly macro to let me switch on both mode, bpp, tile_src1_p. */
#define MUNGE(mode, bpp, ts1) ((mode & 0x3F) + (bpp) * 0x40 + (ts1) * 0x1000)

  switch (MUNGE (mode, bits_per_pixel, tile_src1_p))
    {
      TRANSFORM_CASE (transparent, TRANSPARENT_TRANSFORM);
      TRANSFORM_CASE (hilite, HILITE_TRANSFORM);

    default:
      gui_fatal ("unknown (mode, bpp, tile_p)");
    }
#undef CONVERT_BITS

#undef RGB_TO_PIXEL
#undef RGB_TO_DIRECT_PIXEL
#undef RGB_TO_INDIRECT_PIXEL
  
  /* Set up the dst bitmap's bounds so that rectangle r2 identifies
     the newly created bits.  */
  dst->bounds = *r2;

  if (copy1_p)
    pixmap_free_copy (&write_back1.src_pm);
  if (copy2_p)
    pixmap_free_copy (&write_back2.src_pm);
}

/* This function combines rectangles from two source bitmaps via one of
 * the complex transfer modes described in IMV.  dst must have baseAddr
 * already allocated; rowBytes and bounds will be filled in by this
 * function.
 */
void
Executor::convert_pixmap_with_IMV_mode (const PixMap *src1, const PixMap *src2,
			      PixMap *dst,
			      CTabHandle src1_ctabh, CTabHandle src2_ctabh,
			      ITabHandle itabh,
			      const Rect *r1, const Rect *r2,
			      int16 mode, const RGBColor *op_color,
			      boolean_t tile_src1_p,
			      int pat_x_offset, int pat_y_offset)
{
  CTabPtr src1_ctab, src2_ctab;
  unsigned max_color;
  uint8 *src1_row_base, *src2_row_base, *dst_row_base;
  int src1_rowbytes, src2_rowbytes, dst_rowbytes, src1_deltax, src2_deltax;
  int width, height, s1_width, s1_height;
  int itab_res, itab_res_mask;
  const uint8 *itab_array;
  uint32 op_red, op_green, op_blue;
  
  const rgb_spec_t *rgb_spec;
  int bits_per_pixel;
  
  boolean_t copy1_p, copy2_p;
  write_back_data_t write_back1, write_back2;
  
  bits_per_pixel = BigEndianValue (src1->pixelSize);
  rgb_spec = pixmap_rgb_spec (src1);
  
  /* For bits_per_pixel == 1, you are just supposed to use the original
   * boolean transfer modes.
   */
  gui_assert (bits_per_pixel > 1);
  
  copy1_p = pixmap_copy_if_screen (src1, r1, &write_back1);
  if (copy1_p)
    {
      src1 = &write_back1.src_pm;
      r1   = &write_back1.src_rect;
    }
  copy2_p = pixmap_copy_if_screen (src2, r2, &write_back2);
  if (copy2_p)
    {
      src2 = &write_back2.src_pm;
      r2   = &write_back2.src_rect;
    }
  
  PIXMAP_ASSERT_NOT_SCREEN (dst);
  
  if (tile_src1_p)
    {
      s1_width = RECT_WIDTH (&src1->bounds);
      s1_height = RECT_HEIGHT (&src1->bounds);
    }
  else
    {
      s1_width = RECT_WIDTH (r1);
      s1_height = RECT_HEIGHT (r1);
    }
  width  = RECT_WIDTH (r2);
  height = RECT_HEIGHT (r2);
  
  if (rgb_spec)
    {
      /* initalize to some sane values to shut gcc up */
      src1_ctab = src2_ctab = NULL;
      itab_res = itab_res_mask = 0;
      itab_array = NULL;
    }
  else
    {
      max_color = (1 << bits_per_pixel) - 1;
      
      /* Create a nice, sorted table for src1_ctab. */
      if (CTAB_FLAGS_X (src1_ctabh) & CTAB_GDEVICE_BIT_X)
	{
	  src1_ctab = STARH (src1_ctabh);
	}
      else
	{
	  src1_ctab = sort_table (STARH (src1_ctabh),
				  (CTabPtr) alloca (sizeof *src1_ctab
						    + (max_color
						       * sizeof (ColorSpec))),
				  max_color);
	}

      /* Create a nice, sorted table for src2_ctab. */
      if (CTAB_SEED_X (src1_ctabh) == CTAB_SEED_X (src2_ctabh))
	src2_ctab = src1_ctab;
      else
	{
	  if (CTAB_FLAGS_X (src2_ctabh) & CTAB_GDEVICE_BIT_X)
	    {
	      src2_ctab = STARH (src2_ctabh);
	    }
	  else
	    {
	      src2_ctab = sort_table (STARH (src2_ctabh),
				      (CTabPtr) alloca (sizeof *src2_ctab
							+ (max_color
							   * sizeof (ColorSpec))),
				      max_color);
	    }
	}

      /* Grab the inverse color table.  Note that we are to use this
       * table directly, and *not* call Color2Index.
       */
      itab_res = ITAB_RES (itabh);
      itab_res_mask = (1 << itab_res) - 1;
      itab_array = ITAB_TABLE (itabh);
    }
  
  /* Grab the rowbytes for the three bitmaps. */
  src1_rowbytes = BITMAP_ROWBYTES (src1);
  src2_rowbytes = BITMAP_ROWBYTES (src2);
  dst_rowbytes = (width * bits_per_pixel + 31) / 32 * 4;
  dst->rowBytes = BigEndianValue (dst_rowbytes);
  
  /* We want to run x from 0 to width; adding these offsets gives us
   * the real x for the source bitmaps.
   */
  if (tile_src1_p)
    src1_deltax = pat_x_offset;
  else
    src1_deltax = BigEndianValue (r1->left) - BigEndianValue (src1->bounds.left);
  src2_deltax = BigEndianValue (r2->left) - BigEndianValue (src2->bounds.left);
  
  /* Compute a pointer to the base of the first row of each bitmap. */
  if (tile_src1_p)
    {
      src1_row_base = (unsigned char *) (MR (src1->baseAddr)
		       + (src1_rowbytes * (pat_y_offset & (s1_height - 1))));
    }
  else
    {
      src1_row_base = (unsigned char *) (MR (src1->baseAddr)
		       + ((BigEndianValue (r1->top) - BigEndianValue (src1->bounds.top))
			  * src1_rowbytes));
    }
  src2_row_base = (unsigned char *) (MR (src2->baseAddr)
		   + (BigEndianValue (r2->top) - BigEndianValue (src2->bounds.top)) * src2_rowbytes);
  dst_row_base = (unsigned char *) MR (dst->baseAddr);
  
  /* Fetch the "op color" fields, in case they are needed. */
  op_red   = BigEndianValue (op_color->red);
  op_green = BigEndianValue (op_color->green);
  op_blue  = BigEndianValue (op_color->blue);
  
#define CONVERT_BITS(read1, read2, write, next1, transform, bpp)	\
{									\
  int x, y;								\
  for (y = 0; y < height; y++)						\
    {									\
      for (x = 0; x < width; x++)					\
	{								\
	  /* source 1, 2 rgb's */					\
	  uint32 r1, g1, b1;						\
	  uint32 r2, g2, b2;						\
	  /* result rgb's */						\
	  uint32 rr, gr, br;						\
	  /* final pixel value */					\
	  uint32 p1, p2, pr;						\
									\
	  p1 = read1 (src1_row_base, x + src1_deltax, bpp);		\
	  p2 = read2 (src2_row_base, x + src2_deltax, bpp);		\
									\
	  PIXEL_TO_RGB (bpp, p1, r1, g1, b1, src1_ctab);		\
	  PIXEL_TO_RGB (bpp, p2, r2, g2, b2, src2_ctab);		\
									\
	  transform (r1, g1, b1,					\
		     r2, g2, b2,					\
		     rr, gr, br);					\
									\
	  RGB_TO_PIXEL (bpp, rr, gr, br, pr);				\
									\
	  write (pr, dst_row_base, x, bpp);				\
	}								\
									\
      next1;								\
      src2_row_base += src2_rowbytes;					\
      dst_row_base  += dst_rowbytes;					\
    }									\
}

#define INDIRECT_PIXEL_TO_RGB(pixel, r, g, b, ctab)	\
  ((void)						\
   ({							\
     const RGBColor *color;				\
     color = &ctab->ctTable[pixel].rgb;			\
     (r)   = BigEndianValue (color->red);				\
     (g) = BigEndianValue (color->green);				\
     (b)  = BigEndianValue (color->blue);				\
   }))
#define DIRECT_PIXEL_TO_RGB(bpp, pixel, red_out, green_out, blue_out,	\
			    dummy_ctab)					\
  ((void)								\
   ({									\
     RGBColor color;							\
									\
     (*rgb_spec->pixel_to_rgbcolor) (rgb_spec, (pixel), &color);	\
     (red_out) = BigEndianValue (color.red);					\
     (green_out) = BigEndianValue (color.green);					\
     (blue_out) = BigEndianValue (color.blue);					\
   }))
#define PIXEL_TO_RGB(bpp, pixel, red, green, blue, ctab)	\
  ((void)							\
   ((bpp) == 32 || (bpp) == 16					\
    ? DIRECT_PIXEL_TO_RGB (bpp, pixel, red, green, blue, ctab)	\
    : INDIRECT_PIXEL_TO_RGB (pixel, red, green, blue, ctab)))

#define RGB_TO_INDIRECT_PIXEL(red, green, blue, pixel)	\
  ((pixel) = ITAB_LOOKUP (red, green, blue))
#define RGB_TO_DIRECT_PIXEL(bpp, red, green, blue, pixel)	\
  ((void)							\
   ({								\
     RGBColor color = { (unsigned short)red, (unsigned short)green, (unsigned short)blue };			\
     								\
     (pixel) = ((*rgb_spec->rgbcolor_to_pixel)			\
		(rgb_spec, &color, FALSE));			\
   }))

#define RGB_TO_PIXEL(bpp, red, green, blue, pixel)		\
  ((void)							\
   ((bpp) == 32 || (bpp) == 16					\
    ? (void)RGB_TO_DIRECT_PIXEL (bpp, (unsigned short)red, (unsigned short)green, (unsigned short)blue, pixel)	\
    : (void)RGB_TO_INDIRECT_PIXEL (red, green, blue, pixel)))

#define NONPAT_NEXT1 src1_row_base += src1_rowbytes
#define PAT_NEXT1 \
  src1_row_base = (unsigned char *) (MR (src1->baseAddr)		\
		   + (src1_rowbytes * ((y + pat_y_offset) & (s1_height - 1))))

#define SHIFT_COUNT(x, bpp)  (8 - (bpp) - (bpp) * ((x) & (7 / (bpp))))

#define READ_INDIRECT_PIXEL(b, x, bpp)						\
  ((bpp) == 8									\
   ? b[x]									\
   : ((b[(x) * (bpp) / 8] >> SHIFT_COUNT ((x), (bpp))) & ((1 << (bpp)) - 1)))
#define READ_DIRECT16_PIXEL(b, x, bpp)		\
  ((uint16 *) b)[x]
#define READ_DIRECT32_PIXEL(b, x, bpp)		\
  ((uint32 *) b)[x]

#define READ_PAT_INDIRECT_PIXEL(b, x, bpp)	\
  READ_INDIRECT_PIXEL (b, (x) & (s1_width - 1), bpp)
#define READ_PAT_DIRECT16_PIXEL(b, x, bpp)	\
  READ_DIRECT16_PIXEL (b, (x) & (s1_width - 1), bpp)
#define READ_PAT_DIRECT32_PIXEL(b, x, bpp)	\
  READ_DIRECT32_PIXEL (b, (x) & (s1_width - 1), bpp)

#define WRITE_INDIRECT_PIXEL(v, b, x, bpp)				\
  ((void)								\
   ((bpp) == 8 ? b[x] = (v)						\
  : ({									\
       uint8 *p;							\
       p = &b[(x) * (bpp) / 8];						\
       *p &= ~(((1 << (bpp)) - 1) << SHIFT_COUNT (x, bpp));		\
       *p |= (((v) & ((1 << (bpp)) - 1)) << SHIFT_COUNT (x, bpp));	\
    })))
#define WRITE_DIRECT16_PIXEL(v, b, x, bpp)	\
  ((void)					\
   (((uint16 *) (b))[(x)] = (v)))
#define WRITE_DIRECT32_PIXEL(v, b, x, bpp)	\
  ((void)					\
   (((uint32 *) (b))[(x)] = (v)))

#define ITAB_LOOKUP(r, g, b)			 			      \
  itab_array[((((r) >> (16 - itab_res)) & itab_res_mask) << (2 * itab_res))   \
	     | ((((g) >> (16 - itab_res)) & itab_res_mask) << (1 * itab_res)) \
	     | ((((b) >> (16 - itab_res)) & itab_res_mask) << (0 * itab_res))]

/* Define macros to actually munge together two RGB's in the right way. */
#define ADD_OVER_TRANSFORM(red1, green1, blue1,	\
			   red2, green2, blue2,	\
			   redr, greenr, bluer)	\
  ((void)					\
   (redr = red1 + red2,				\
    greenr = green1 + green2,			\
    bluer = blue1 + blue2))
#define ADD_PIN_TRANSFORM(red1, green1, blue1,	\
			  red2, green2, blue2,	\
			  redr, greenr, bluer)	\
  ((void)					\
   (redr = red1 + red2,				\
    greenr = green1 + green2,			\
    bluer = blue1 + blue2,			\
    redr = MIN (redr, op_red),			\
    greenr = MIN (greenr, op_green),		\
    bluer = MIN (bluer, op_blue)))
#define SUB_OVER_TRANSFORM(red1, green1, blue1,	\
			   red2, green2, blue2,	\
			   redr, greenr, bluer)	\
  ((void)					\
   (redr = red1 - red2,				\
    greenr = green1 - green2,			\
    bluer = blue1 - blue2))
#define SUB_PIN_TRANSFORM(red1, green1, blue1,	\
			  red2, green2, blue2,	\
			  redr, greenr, bluer)	\
  ((void)					\
   (redr = red1 - red2,				\
    greenr = green1 - green2,			\
    bluer = blue1 - blue2,			\
    redr = MAX (redr, op_red),			\
    greenr = MAX (greenr, op_green),		\
    bluer = MAX (bluer, op_blue)))
#define AD_MAX_TRANSFORM(red1, green1, blue1,	\
			 red2, green2, blue2,	\
			 redr, greenr, bluer)	\
  ((void)					\
   (redr = MAX (red1, red2),			\
    greenr = MAX (green1, green2),		\
    bluer = MAX (blue1, blue2)))
#define AD_MIN_TRANSFORM(red1, green1, blue1,	\
			 red2, green2, blue2,	\
			 redr, greenr, bluer)	\
  ((void)					\
   (redr = MIN (red1, red2),			\
    greenr = MIN (green1, green2),		\
    bluer = MIN (blue1, blue2)))
#define BLEND_TRANSFORM(red1, green1, blue1,				\
			red2, green2, blue2,				\
			redr, greenr, bluer)				\
  ((void)								\
   (redr = (red1 * op_red + red2 * (65535 - op_red)) / 65535,		\
    greenr = (green1 * op_green + green2 * (65535 - op_green)) / 65535,	\
    bluer = (blue1 * op_blue + blue2 * (65535 - op_blue)) / 65535))

/* Silly macro to let me switch on both mode, bpp, tile_src1_p. */
#define MUNGE(mode, bpp, ts1) ((mode & 0x3F) + (bpp) * 0x40 + (ts1) * 0x1000)

  switch (MUNGE (mode, bits_per_pixel, tile_src1_p))
    {
      TRANSFORM_CASE (addOver, ADD_OVER_TRANSFORM);
      TRANSFORM_CASE (addPin, ADD_PIN_TRANSFORM);
      TRANSFORM_CASE (subOver, SUB_OVER_TRANSFORM);
      TRANSFORM_CASE (subPin, SUB_PIN_TRANSFORM);
      TRANSFORM_CASE (adMin, AD_MIN_TRANSFORM);
      TRANSFORM_CASE (adMax, AD_MAX_TRANSFORM);
      TRANSFORM_CASE (blend, BLEND_TRANSFORM);
    default:
      gui_fatal ("invalid case");
    }
#undef CONVERT_BITS

  /* Set up the dst bitmap's bounds so that rectangle r2 identifies
   * the newly created bits.
   */
  dst->bounds = *r2;

  if (copy1_p)
    pixmap_free_copy (&write_back1.src_pm);
  if (copy2_p)
    pixmap_free_copy (&write_back2.src_pm);
}


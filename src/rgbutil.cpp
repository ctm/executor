/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_rgbutil[] =
		"$Id: rgbutil.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "CQuickDraw.h"
#include "rsys/rgbutil.h"
#include "rsys/cquick.h"

using namespace Executor;

/* This file contains routines useful for manipulating RGB pixels.
 *
 * `make_rgb_spec' fills in an rgb_spec_t structure with the appropriate
 *  fields.  Function pointers in this structure can be used to quickly
 *  translate between RGBColor structs and pixel values.
 */

void
rgbutil_init (void)
{
}


static void
make_component_map (uint16 *map, int num_bits, bool swap_p)
{
  unsigned mask;
  int i;

  /* Handle all the cases where the bits matter. */
  mask = (1 << num_bits) - 1;
  for (i = mask; i >= 0; i--)
    {
      unsigned v;
      int shift;

      v = i << (16 - num_bits);
      for (shift = num_bits; shift < 16; shift += num_bits)
	v |= v >> shift;
      map[i] = swap_p ? CW_RAW (v) : v;
    }

  /* Fill out the rest of the table with duplicate entries. */
  for (i = mask + 1; i < 256; i++)
    map[i] = map[i & mask];
}


static void
rgb_extract_from_unswapped_pixel (const rgb_spec_t *rgb_spec,
				  uint32 in, RGBColor *out)
{
  const rgb_map_t *table;

  table = &rgb_spec->map;
  /* NOTE: we & 0xFF here because our lookup tables are built in
   * such a way as to ignore any extraneous high bits.  This way we
   * don't have to bother computing the exact mask to grab the right
   * number of RGB bits.
   */
  out->red  .raw( table->map[0][(in >> table->low_red_bit  ) & 0xFF] );
  out->green.raw( table->map[1][(in >> table->low_green_bit) & 0xFF] );
  out->blue .raw( table->map[2][(in >> table->low_blue_bit ) & 0xFF] );
}


#if defined (LITTLEENDIAN)
static void
rgb_extract_from_swapped_16bpp_pixel (const rgb_spec_t *rgb_spec,
				      uint32 in, RGBColor *out)
				      
{
  const rgb_map_t *table;

  table = &rgb_spec->map;
  in = CW_RAW (in);
  
  /* NOTE: we & 0xFF here because our lookup tables are built in
   * such a way as to ignore any extraneous high bits.  This way we
   * don't have to bother computing the exact mask to grab the right
   * number of RGB bits.
   */
  out->red  .raw( table->map[0][(in >> table->low_red_bit  ) & 0xFF] );
  out->green.raw( table->map[1][(in >> table->low_green_bit) & 0xFF] );
  out->blue .raw( table->map[2][(in >> table->low_blue_bit ) & 0xFF] );
}


static void
rgb_extract_from_swapped_32bpp_pixel (const rgb_spec_t *rgb_spec,
				      uint32 in, RGBColor *out)
{
  const rgb_map_t *table;

  table = &rgb_spec->map;
  in = CL_RAW (in);

  /* NOTE: we & 0xFF here because our lookup tables are built in
   * such a way as to ignore any extraneous high bits.  This way we
   * don't have to bother computing the exact mask to grab the right
   * number of RGB bits.
   */
  out->red  .raw( table->map[0][(in >> table->low_red_bit  ) & 0xFF] );
  out->green.raw( table->map[1][(in >> table->low_green_bit) & 0xFF] );
  out->blue .raw( table->map[2][(in >> table->low_blue_bit ) & 0xFF] );
}
#endif /* defined (LITTLEENDIAN) */


/* This function creates a table that maps RGB bits to big-endian
 * RGBColor components.  This function will always map 8 RGB bits; if
 * the actual component uses less than 8 RGB bits, this table ends up
 * ignoring the high bits.  So if you have a 7 bit RGB component,
 * table[0][0] == table[0][0x80], and so on.
 */
static rgb_extract_func_t
make_pixel_to_rgbcolor_table (const rgb_spec_t *spec, rgb_map_t *d,
			      bool big_endian_rgbcolor_p)
{
  d->low_red_bit   = spec->low_red_bit;
  d->low_green_bit = spec->low_green_bit;
  d->low_blue_bit  = spec->low_blue_bit;

  /* Create a map for the red bits. */
  make_component_map (d->map[0], spec->num_red_bits, big_endian_rgbcolor_p);

  /* Create a map for the green bits (copying red's if they are the same). */
  if (spec->num_green_bits == spec->num_red_bits)
    memcpy (d->map[1], d->map[0], sizeof d->map[1]);
  else
    make_component_map (d->map[1], spec->num_green_bits,big_endian_rgbcolor_p);
  
  /* Create a map for the blue bits (copying red's or green's if they
   * are the same).
   */
  if (spec->num_blue_bits == spec->num_red_bits)
    memcpy (d->map[2], d->map[0], sizeof d->map[2]);
  else if (spec->num_blue_bits == spec->num_green_bits)
    memcpy (d->map[2], d->map[1], sizeof d->map[2]);
  else
    make_component_map (d->map[2], spec->num_blue_bits, big_endian_rgbcolor_p);

#if defined (LITTLEENDIAN)
  if (spec->big_endian_p)
    {
      if (spec->bpp == 16)
	return rgb_extract_from_swapped_16bpp_pixel;
      else
	return rgb_extract_from_swapped_32bpp_pixel;
    }
  else
#endif /* LITTLEENDIAN */
    {
      return rgb_extract_from_unswapped_pixel;
    }
}


static uint32
rgbcolor_to_pixel (const rgb_spec_t *spec,
		   const RGBColor *color,
		   bool big_endian_rgbcolor_p)
{
  const uint8 *values;
  uint32 v;

  /* Treat RGBColor as an array of bytes, so we can easily grab MSB's. */
  values = (const uint8 *)color;
#if defined (LITTLEENDIAN)
  if (!big_endian_rgbcolor_p)
    ++values;
#endif

  /* Translate RGBColor to RGB pixel of the appropriate format. */
  v = ((values[0]
	>> (8 - spec->num_red_bits))
       << spec->low_red_bit);
  v |= ((values[2]
	 >> (8 - spec->num_green_bits))
	<< spec->low_green_bit);
  v |= ((values[4]
	 >> (8 - spec->num_blue_bits))
	<< spec->low_blue_bit);
  v ^= spec->xor_mask;

  /* Byte swap if necessary. */
  if (spec->big_endian_p)
    {
      if (spec->bpp == 16)
	v = (uint16) CW_RAW (v);  /* cast masks off extra cruft in high bits. */
      else
	v = CL_RAW (v);
    }

  return v;
}


void
Executor::make_rgb_spec (rgb_spec_t *rgb_spec,
	       int bpp, bool big_endian_p,
	       uint32 xor_mask,
	       int num_red_bits, int low_red_bit,
	       int num_green_bits, int low_green_bit,
	       int num_blue_bits, int low_blue_bit,
	       uint32 seed_x)
{
  uint32 w, b;

  /* fill in the ordinary fields */
  rgb_spec->bpp            = bpp;
  rgb_spec->big_endian_p   = big_endian_p;
  rgb_spec->xor_mask       = (bpp == 32) ? xor_mask : xor_mask * 0x10001;
  rgb_spec->num_red_bits   = num_red_bits;
  rgb_spec->low_red_bit    = low_red_bit;
  rgb_spec->num_green_bits = num_green_bits;
  rgb_spec->low_green_bit  = low_green_bit;
  rgb_spec->num_blue_bits  = num_blue_bits;
  rgb_spec->low_blue_bit   = low_blue_bit;
  rgb_spec->seed_x         = seed_x;

  /* for now, use the generic rgbcolor to pixel conversion routine.
     this should be specified by the caller */
  rgb_spec->rgbcolor_to_pixel = rgbcolor_to_pixel;

  rgb_spec->pixel_to_rgbcolor
    = make_pixel_to_rgbcolor_table (rgb_spec, &rgb_spec->map, true);

  /* Create black and white pixels tiled out to 32 bits. */
  b = (*rgb_spec->rgbcolor_to_pixel) (rgb_spec, &ROMlib_black_rgb_color,
				      true);
  w = (*rgb_spec->rgbcolor_to_pixel) (rgb_spec, &ROMlib_white_rgb_color,
				      true);
  if (bpp == 16)
    {
      /* Tile out to 32 bits. */
      b *= 0x10001;
      w *= 0x10001;
    }

  rgb_spec->black_pixel     = b;
  rgb_spec->white_pixel     = w;
  rgb_spec->pixel_bits_mask = w ^ rgb_spec->xor_mask;
}

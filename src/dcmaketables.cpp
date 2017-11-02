/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dcmaketables[] =
		"$Id: dcmaketables.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/depthconv.h"
#include "rsys/cquick.h"

namespace Executor {

/* This file contains routines to construct lookup tables used for
 * depth conversion, and works hand in hand with dcconvert.c.  The
 * model is that the appropriate lookup table creation routine is
 * called with a pointer to allocated table space, that routine fills
 * in the table space with private data, and returns a pointer to a
 * function pointer that knows how to use the created table to
 * efficiently depth convert a rectangle.
 *
 * So how do you know how much table space to allocate?  Good
 * question.  Each of the table creation routines returns by reference
 * the table size it needs for the parameters you specified.  If you
 * pass in NULL for the allocated table space, the function will not
 * actually create the table.  So you can say:
 *
 *   unsigned size;
 *   void *table;
 *
 *   depthconv_make_ind_to_ind_table (NULL, 1, 8, &size, cspec_array);
 *   table = malloc (size);
 *   depthconv_make_ind_to_ind_table (table, 1, 8, NULL, cspec_array);
 *
 * You can also use the DEPTHCONV_MAX_TABLE_SIZE macro, which is
 * guaranteed to always be "big enough":
 *
 *   static uint8 table[DEPTHCONV_MAX_TABLE_SIZE];
 *
 *   depthconv_make_ind_to_ind_table (table, 1, 8, &size, cspec_array);
 *
 * The disadvantage, of course, is potentially wasted space.
 *
 * There are five functions to handle different conversion types.
 * "ind" is short for "indirect", and refers to 1, 2, 4, or 8 bpp
 * pixels.  "rgb" refers to 16 or 32 bpp pixels containing separate
 * red, green, and blue components.
 *
 *   depthconv_make_raw_table
 *   depthconv_make_ind_to_ind_table
 *   depthconv_make_ind_to_rgb_table
 *   depthconv_make_rgb_to_ind_table
 *   depthconv_make_rgb_to_rgb_table
 *
 * `depthconv_make_raw_table' can be used for mapping indirect pixels
 * to raw values either 1, 2, 4, 8, 16, or 32 bits wide without
 * constructing a ColorSpec array.
 */


/* This is a table of the conversion funcs to use for various in:out ratios. */
static const depthconv_func_t
ind_src_conversion_funcs[] =
{
  depthconv_1_32,
  depthconv_1_16,
  depthconv_1_8,
  depthconv_1_4,
  depthconv_1_2,
  depthconv_1_1,
  depthconv_2_1,
  depthconv_4_1,
  depthconv_8_1,
  NULL,  /* There is no generic 16->1 converter. */
  NULL,  /* There is no generic 32->1 converter. */
};


/* This is an array of the various alignments we require for different
 * conversion types.  The desired alignment varies depending on the
 * algorithm used.
 */
const int
depthconv_ind_src_table_alignment[] =
{
  32,	/* 1  -> 32 */
  16,	/* 1  -> 16 */
  8,	/* 1  -> 8  */
  4,	/* 1  -> 4  */
  8,	/* 1  -> 2  */
  256,	/* 1  -> 1  */
  2,	/* 2  -> 1  */
  4,	/* 4  -> 1 */
  8,	/* 8  -> 1 */
  0,	/* 16 -> 1, impossible case for indirect pixel src. */
  0,	/* 32 -> 1, impossible case for indirect pixel src. */
};


/* Table sizes needed for various in:out bpp ratios. */
static const int
ind_src_unaligned_table_size[] =
{
  sizeof (depthconv_1_32_data_t),
  sizeof (depthconv_1_16_data_t),
  sizeof (depthconv_1_8_data_t),
  sizeof (depthconv_1_4_data_t),
  sizeof (depthconv_1_2_data_t),
  sizeof (depthconv_1_1_data_t),
  sizeof (depthconv_2_1_data_t),
  sizeof (depthconv_4_1_data_t),
  sizeof (depthconv_8_1_data_t),
  0,  /* 16->1, impossible case for indirect pixel src. */
  0,  /* 32->1, impossible case for indirect pixel src. */
};

#define CONVERSION_FUNC(log2_in_bpp, log2_out_bpp) \
     ind_src_conversion_funcs[(log2_in_bpp) - (log2_out_bpp) + 5]
#define TABLE_SIZE(log2_in_bpp, log2_out_bpp)				 \
     ((ind_src_unaligned_table_size[(log2_in_bpp) - (log2_out_bpp) + 5]  \
       + sizeof (uint32)  /* First long specifies log2_in_bpp. */	 \
       + DEPTHCONV_TABLE_ALIGNMENT (log2_in_bpp, log2_out_bpp) - 1)	 \
      & (DEPTHCONV_TABLE_ALIGNMENT (log2_in_bpp, log2_out_bpp) - 1))



/* This macro handles the cases where the pixel depth is increasing by
 * a factor of two.  We need a special case here because the lookup table
 * we use for this algorithm is in a different format than for the other
 * nondecreasing modes.
 */
#define DEPTH_INCREASING_BY_FACTOR_OF_2(bpp1, bpp2)			      \
static void								      \
maketable_ ## bpp1 ## _ ## bpp2 (void *d, const uint32 *map)		      \
{									      \
  int c;								      \
  uint16 *dst;								      \
									      \
  for (c = 0, dst = (uint16 *) d; c < 256; c++)				      \
    {									      \
      uint16 new1;							      \
      int r, l;								      \
									      \
      /* Compute initial left shift count. */				      \
      l = 16 - bpp2;							      \
									      \
      /* Loop over all input pixels and create the lookup table entry. */     \
      for (new1 = 0, r = 8 - bpp1; r >= 0; r -= bpp1)			      \
	{								      \
          uint32 v = map[(c >> r) & ((1UL << bpp1) - 1)];		      \
          new1 |= (v & ((1UL << bpp2) - 1)) << l;			      \
	  if (l == 0)							      \
	    {								      \
	      dst[0] = dst[3] = CW_RAW (new1);				      \
	      dst[1] = dst[2] = CWC_RAW (0);				      \
	      dst += 4;							      \
	      l = 16 - bpp2;						      \
	      new1 = 0;							      \
	    }								      \
	  else								      \
	    l -= bpp2;							      \
	}								      \
    }									      \
}


DEPTH_INCREASING_BY_FACTOR_OF_2 (1, 2)
DEPTH_INCREASING_BY_FACTOR_OF_2 (2, 4)
DEPTH_INCREASING_BY_FACTOR_OF_2 (4, 8)
DEPTH_INCREASING_BY_FACTOR_OF_2 (8, 16)


/* This macro handles the cases where the pixel depth is not decreasing. */
#define DEPTH_NONDECREASING(bpp1, bpp2, new_type)			      \
static void								      \
maketable_ ## bpp1 ## _ ## bpp2 (void *d, const uint32 *map)		      \
{									      \
  int c;								      \
  new_type *dst;							      \
									      \
  for (c = 0, dst = (new_type *) d; c < 256; c++)			      \
    {									      \
      new_type new1;							      \
      int r, l;								      \
									      \
      /* Compute initial left shift count. */				      \
      l = (8 * sizeof new1) - bpp2;					      \
									      \
      /* Loop over all input pixels and create the lookup table entry. */     \
      for (new1 = 0, r = 8 - bpp1; r >= 0; r -= bpp1)			      \
	{								      \
          uint32 v = map[(c >> r) & ((1UL << bpp1) - 1)];		      \
          new1 |= (v & (0xFFFFFFFFUL >> (32 - bpp2))) << l;		      \
	  if (l == 0)							      \
	    {								      \
	      *dst++ = Cx_RAW (new1);					      \
	      l = (8 * sizeof new1) - bpp2;				      \
	      new1 = 0;							      \
	    }								      \
	  else								      \
	    l -= bpp2;							      \
	}								      \
    }									      \
}


DEPTH_NONDECREASING (1,  1, uint8)
/* 1 -> 2 handled above by `DEPTH_INCREASING_BY_FACTOR_OF_2'. */
DEPTH_NONDECREASING (1,  4, uint32)
DEPTH_NONDECREASING (1,  8, uint32)
DEPTH_NONDECREASING (1, 16, uint32)
DEPTH_NONDECREASING (1, 32, uint32)

DEPTH_NONDECREASING (2,  2, uint8)
/* 2 -> 4 handled above by `DEPTH_INCREASING_BY_FACTOR_OF_2'. */
DEPTH_NONDECREASING (2,  8, uint32)
DEPTH_NONDECREASING (2, 16, uint32)
DEPTH_NONDECREASING (2, 32, uint32)

DEPTH_NONDECREASING (4,  4, uint8)
/* 4 -> 8 handled above by `DEPTH_INCREASING_BY_FACTOR_OF_2'. */
DEPTH_NONDECREASING (4, 16, uint32)
DEPTH_NONDECREASING (4, 32, uint32)

DEPTH_NONDECREASING (8,  8, uint8)
/* 8 -> 16 handled above by `DEPTH_INCREASING_BY_FACTOR_OF_2'. */
DEPTH_NONDECREASING (8, 32, uint32)



#define DEPTH_DECREASING(bpp1, bpp2)					 \
static void								 \
maketable_ ## bpp1 ## _ ## bpp2 (void *d, const uint32 *map)		 \
{									 \
  int c;								 \
  uint8 *dst;								 \
									 \
  for (c = 0, dst = (uint8 *) d; c < 256; c++)				 \
    {									 \
      long offset;							 \
      int r, l;								 \
      uint8 new1;							 \
									 \
      /* Loop over all input pixels and create the lookup table entry. */\
      for (r = 8 - bpp1, l = (8 * bpp2 / bpp1) - bpp2, new1 = 0;		 \
	   r >= 0;							 \
	   r -= bpp1, l -= bpp2)					 \
	{								 \
          new1 |= ((map[(c >> r) & ((1UL << bpp1) - 1)]			 \
		   & ((1UL << bpp2) - 1))				 \
		  << l);						 \
	}								 \
									 \
      for (offset = 256 * (bpp1 / bpp2 - 1); offset >= 0; offset -= 256) \
	{								 \
	  dst[offset] = new1;						 \
	  new1 <<= (8 * bpp2 / bpp1);					 \
	}								 \
      ++dst;								 \
    }									 \
}


DEPTH_DECREASING (2, 1)

DEPTH_DECREASING (4, 1)
DEPTH_DECREASING (4, 2)

DEPTH_DECREASING (8, 1)
DEPTH_DECREASING (8, 2)
DEPTH_DECREASING (8, 4)



typedef void (*maketable_func_t)(void *, const uint32 *);

/* This is a table of the functions to create mapping tables
 * for all combinations of in->out bpp's where in <= 8.
 */
static const maketable_func_t
ind_src_table_builders[4][6] =
{
  { maketable_1_1, maketable_1_2, maketable_1_4,
      maketable_1_8, maketable_1_16, maketable_1_32 },
  { maketable_2_1, maketable_2_2, maketable_2_4,
      maketable_2_8, maketable_2_16, maketable_2_32 },
  { maketable_4_1, maketable_4_2, maketable_4_4,
      maketable_4_8, maketable_4_16, maketable_4_32 },
  { maketable_8_1, maketable_8_2, maketable_8_4,
      maketable_8_8, maketable_8_16, maketable_8_32 },
};


/* Returns TRUE iff the specified mapping has no effect (i.e. maps 0 to 0,
 * 1 to 1, 2 to 2, etc.
 */
static inline bool
nop_map_p (const uint32 *map, int bpp)
{
  int i;

  for (i = (1 << bpp) - 1; i >= 0; i--)
    if (map[i] != (uint32) i)
      return FALSE;
  
  return TRUE;
}


/* This creates a table to map incoming pixels to "raw" bit patterns.
 * Bit patterns > 8 bits wide will be byte swapped before they are
 * written out.
 */
depthconv_func_t
depthconv_make_raw_table (void *table_space, unsigned in_bpp, unsigned out_bpp,
			  uint32 *table_size, const uint32 *mapping)
{
  int log2_in_bpp, log2_out_bpp;

  /* Default to requesting an empty table size. */
  if (table_size)
    *table_size = 0;

  /* Sanity check the incoming bpp's. */
  if (in_bpp > 32 || out_bpp > 32
      || (in_bpp > 8 && (out_bpp <= 8 || mapping)))
    return NULL;

  /* Compute the log2 of those bpp's and make sure they are legitimate. */
  log2_in_bpp = ROMlib_log2[in_bpp];
  log2_out_bpp = ROMlib_log2[out_bpp];
  if (log2_in_bpp < 0 || log2_out_bpp < 0)
    return NULL;

  /* Check for the no translation case. */
  if (in_bpp == out_bpp && (mapping == NULL || nop_map_p (mapping, in_bpp)))
    {
      if (table_size)
	*table_size = sizeof (uint32);  /* To hold log2_in_bpp. */
      if (table_space)
	*(uint32 *)table_space = log2_in_bpp;	
      return depthconv_copy;
    }

  /* Compute the actual table size we need. */
  if (table_size)
    {
      *table_size = TABLE_SIZE (log2_in_bpp, log2_out_bpp);
      gui_assert (*table_size <= DEPTHCONV_MAX_TABLE_SIZE);
    }
     
  /* Fill in the conversion table if necessary. */
  if (table_space)
    {
      void *dst;
      uint32 map_space[256];

      /* Create a NOP mapping if they specify a NULL mapping. */
      if (mapping == NULL)
	{
	  int i;
	  for (i = (1 << in_bpp) - 1; i >= 0; i--)
	    map_space[i] = i;
	  mapping = map_space;
	}

      /* Align the table and crank out the data. */
      *(uint32 *)table_space = log2_in_bpp;
      dst = DEPTHCONV_ALIGN_TABLE (table_space, log2_in_bpp, log2_out_bpp);
      (ind_src_table_builders[log2_in_bpp][log2_out_bpp]) (dst, mapping);
    }

  return CONVERSION_FUNC (log2_in_bpp, log2_out_bpp);
}


/* This creates a table to map indirect pixels to indirect pixels.
 * If MAPPING is NULL, assumes identity mapping (useful for straight
 * depth conversion).
 */
depthconv_func_t
depthconv_make_ind_to_ind_table (void *table_space,
				 unsigned in_bpp, unsigned out_bpp,
				 uint32 *table_size, const ColorSpec *mapping)
{
  uint32 *raw_map;

  /* Verify bpp. */
  if (in_bpp > 8 || out_bpp > 8)
    {
      if (table_size)
	*table_size = 0;
      return NULL;
    }

  /* Translate the mapping table to a raw table if it will be used. */
  raw_map = NULL;
  if (table_space)
    {
      int i;

      if (mapping)
	{
	  raw_map = (uint32 *) alloca (256 * sizeof raw_map[0]);
	  for (i = (1 << in_bpp) - 1; i >= 0; i--)
	    raw_map[i] = COLORSPEC_VALUE_LOW_BYTE (&mapping[i]);
	}
    }

  return depthconv_make_raw_table (table_space, in_bpp, out_bpp, table_size,
				   raw_map);
}


/* This creates a table to map indirect pixels to RGB pixels. */
depthconv_func_t
depthconv_make_ind_to_rgb_table (void *table_space, unsigned in_bpp,
				 uint32 *table_size, const ColorSpec *mapping,
				 const rgb_spec_t *dst_rgb_spec)
{
  uint32 raw_map[256];
  unsigned out_bpp;

  /* Verify bpp. */
  out_bpp = dst_rgb_spec->bpp;
  if (in_bpp > 8 || (out_bpp != 16 && out_bpp != 32))
    {
      if (table_size)
	*table_size = 0;
      return NULL;
    }

  /* Translate the mapping table to a raw table if it will be used. */
  if (table_space)
    {
      int i;

      for (i = (1 << in_bpp) - 1; i >= 0; i--)
	{
	  uint32 v;

	  /* Assemble the new RGB value. */
	  v = (*dst_rgb_spec->rgbcolor_to_pixel)(dst_rgb_spec, &mapping[i].rgb,
						 TRUE);

	  /* Write out the value to the raw array.  We byte swap here
	   * to counteract the byte swap that will happen later when
	   * the raw table is built.
	   */
	  raw_map[i] = (out_bpp == 16) ? CW_RAW (v) : CL_RAW (v);
	}
    }

  return depthconv_make_raw_table (table_space, in_bpp, out_bpp, table_size,
				   raw_map);
}


/* This creates a table to map RGB pixels to indirect pixels. */
depthconv_func_t
depthconv_make_rgb_to_ind_table (void *table_space, unsigned out_bpp,
				 uint32 *table_size, CTabHandle mapping,
				 ITabHandle itab,
				 const rgb_spec_t *src_rgb_spec)
{
  if (table_size)
    *table_size = sizeof (depthconv_rgb_to_ind_data_t);

  if (table_space)
    {
      depthconv_rgb_to_ind_data_t *d;

      /* Grab a pointer to the struct itself. */
      d = (depthconv_rgb_to_ind_data_t *) table_space;

      d->log2_in_bpp = ROMlib_log2[src_rgb_spec->bpp];
      
      /* Fill in the src rgb_spec. */
      d->src_rgb_spec = src_rgb_spec;
      
      /* Save away some other info for later. */
      d->swapped_ctab = RM (mapping);
      d->swapped_itab = RM (itab);
    }

  /* Return the function that does the actual transfer. */
  if (src_rgb_spec->bpp == 16)
    switch (out_bpp)
      {
      case 1: return depthconv_16_1;
      case 2: return depthconv_16_2;
      case 4: return depthconv_16_4;
      case 8: return depthconv_16_8;
      }
  else if (src_rgb_spec->bpp == 32)
    switch (out_bpp)
      {
      case 1: return depthconv_32_1;
      case 2: return depthconv_32_2;
      case 4: return depthconv_32_4;
      case 8: return depthconv_32_8;
      }

  /* Shouldn't get here. */
  gui_abort ();
#if !defined (LETGCCWAIL)
  return NULL;
#endif
}


/* This creates a table to map RGB pixels to RGB pixels. */
depthconv_func_t
depthconv_make_rgb_to_rgb_table (void *table_space, uint32 *table_size,
				 const rgb_spec_t *src_rgb_spec,
				 const rgb_spec_t *dst_rgb_spec)
{
  /* If no conversion takes place, hand it off to the simpler converter. */
  if (src_rgb_spec == dst_rgb_spec)
    return depthconv_make_raw_table (table_space, src_rgb_spec->bpp,
				     dst_rgb_spec->bpp, table_size, NULL);

  /* Nope, the RGB specs differ.  Create appropriate info here. */
  if (table_size)
    *table_size = sizeof (depthconv_rgb_to_rgb_data_t);

  if (table_space)
    {
      depthconv_rgb_to_rgb_data_t *d;

      /* Grab a pointer to the struct itself. */
      d = (depthconv_rgb_to_rgb_data_t *) table_space;
      
      d->log2_in_bpp = ROMlib_log2[src_rgb_spec->bpp];
      
      /* Record src rgb spec. */
      d->src_rgb_spec = src_rgb_spec;

      /* Record dst rgb spec. */
      d->dst_rgb_spec = dst_rgb_spec;
    }

  /* Return the appropriate conversion function. */
  if (src_rgb_spec->bpp == 16)
    {
      if (dst_rgb_spec->bpp == 16)
	return depthconv_16_16;
      else
	return depthconv_16_32;
    }
  else
    {
      if (dst_rgb_spec->bpp == 16)
	return depthconv_32_16;
      else
	return depthconv_32_32;
    }
}

}

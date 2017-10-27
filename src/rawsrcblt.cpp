/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_rawsrcblt[] =
		"$Id: rawsrcblt.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "rsys/srcblt.h"
#include "rsys/quick.h"
#include "rsys/xdblt.h"
#include "rsys/rawblt.h"

#if defined (USE_PORTABLE_SRCBLT)

static int srcblt_src_row_stride;
static int srcblt_dst_row_stride;


/* Dummy table, not actually dereferenced. */
const void *srcblt_nop_table[1] = { NULL };

/* This macro is used in `rawsrcstubs.c' */
#define NEXT_ROW (src = (const uint32 *) &src_row_base[s[1].offset], \
		  dst = (uint32 *) &dst_row_base[s[1].offset])

namespace Executor {
#include "rawsrcstubs.ctable"
}

using namespace Executor;

const void **srcblt_noshift_stubs[8] = {
  srcblt_copy_noshift_labels,
  srcblt_or_noshift_labels,
  srcblt_xor_noshift_labels,
  srcblt_notand_noshift_labels,
  srcblt_notcopy_noshift_labels,
  srcblt_notor_noshift_labels,
  srcblt_notxor_noshift_labels,
  srcblt_and_noshift_labels
};

const void **srcblt_noshift_fgbk_stubs[8] = {
  srcblt_copy_noshift_fgbk_labels,
  srcblt_or_noshift_fgbk_labels,
  srcblt_xor_noshift_labels,
  srcblt_notand_noshift_fgbk_labels,
  srcblt_notcopy_noshift_fgbk_labels,
  srcblt_notor_noshift_fgbk_labels,
  srcblt_notxor_noshift_labels,
  srcblt_and_noshift_fgbk_labels
};

const void **srcblt_shift_stubs[8] = {
  srcblt_copy_shift_labels,
  srcblt_or_shift_labels,
  srcblt_xor_shift_labels,
  srcblt_notand_shift_labels,
  srcblt_notcopy_shift_labels,
  srcblt_notor_shift_labels,
  srcblt_notxor_shift_labels,
  srcblt_and_shift_labels
};

const void **srcblt_shift_fgbk_stubs[8] = {
  srcblt_copy_shift_fgbk_labels,
  srcblt_or_shift_fgbk_labels,
  srcblt_xor_shift_labels,
  srcblt_notand_shift_fgbk_labels,
  srcblt_notcopy_shift_fgbk_labels,
  srcblt_notor_shift_fgbk_labels,
  srcblt_notxor_shift_labels,
  srcblt_and_shift_fgbk_labels
};


void
srcblt_bitmap (void)
{
  blt_section_t section[MAX_BLT_SECTIONS], *sec;
  const INTEGER *rgn;
  int x, y, next_y, start_x_bit, stop_x_bit, start_x_long, stop_x_long;
  int num_rows, num_longs;
  uint32 mask;
  int log2_bpp;

  if (srcblt_stub_table == srcblt_nop_table)
    return;

  if (srcblt_reverse_scanlines_p)
    {
      srcblt_src_row_stride = -srcblt_src_row_bytes;
      srcblt_dst_row_stride = -srcblt_dst_row_bytes;
    }
  else
    {
      srcblt_src_row_stride = srcblt_src_row_bytes;
      srcblt_dst_row_stride = srcblt_dst_row_bytes;
    }

  /* Store log2_bpp in a local variable, for speed. */
  log2_bpp = srcblt_log2_bpp;

  rgn = srcblt_rgn_start;
  y = CW_RAW (*rgn++);
  if (y == RGNSTOP)
    goto done_with_scanlines;

 start_scanline:
  x = rgn[0];
  mask = 0;
  sec = section;
  if (x == RGNSTOP)
    goto fetch_next_y;

  start_x_bit = (x << log2_bpp) + srcblt_x_offset;

 still_same_long:
  mask ^= xdblt_mask_array[start_x_bit & 31];
  start_x_long = start_x_bit >> 5;

 next_stop:
  stop_x_bit = (rgn[1] << log2_bpp) + srcblt_x_offset;
  rgn += 2;
  stop_x_long = stop_x_bit >> 5;
  
  num_longs = stop_x_long - start_x_long;
  if (num_longs == 0)
    goto same_long;

  /* Different longs. */
  if (mask == (uint32) ~0)
    goto blt_contig;

  /* Mask not solid. */
  sec->label = srcblt_stub_table[MASK_STUB];
  sec->offset = start_x_long << 2;
  sec->arg = mask;
  sec++;

  if (sec >= &section[MAX_BLT_SECTIONS])
    abort ();

  /* account for that mask boundary long. */
  --num_longs;
  if (num_longs == 0)
    goto xfer_done;
  ++start_x_long;

 blt_contig:
  sec->label = srcblt_stub_table[REPEAT_MOD_0_STUB
				+ (num_longs % MAX_LOOP_UNWRAP)];
  sec->offset = (start_x_long - ((-num_longs) % MAX_LOOP_UNWRAP)) << 2;
  sec->arg = num_longs;
  sec++;

 xfer_done:
  mask = ~0;

 same_long:
  /* NOTE:  the start x bit fetched here may be RGNSTOP.  That case
   * will be detected later; RGNSTOP will cause the boundary cruft
   * to get blitted just like an X not in the same long, so this
   * hack works.
   */
  start_x_bit = (rgn[0] << log2_bpp) + srcblt_x_offset;
  mask ^= xdblt_mask_array[stop_x_bit & 31];
  start_x_long = start_x_bit >> 5;
  if (start_x_long == stop_x_long)
    goto still_same_long;

  if (mask == 0)
    goto no_boundary_cruft;

  sec->label = srcblt_stub_table[MASK_STUB];
  sec->offset = stop_x_long << 2;
  sec->arg = mask;
  sec++;

  if (sec >= &section[MAX_BLT_SECTIONS])
    abort ();

 no_boundary_cruft:
  if (rgn[0] == RGNSTOP)
    goto fetch_next_y;
  mask = xdblt_mask_array[start_x_bit & 31];
  goto next_stop;

 fetch_next_y:
 /* Fetch the starting y value for the next scanline.  When we
  * get here, rgn points to the RGNSTOP for the last scanline,
  * so we have to look two bytes farther to find the next y
  * value.  Special regions store y's as big endian.
  */
  next_y = CW_RAW (rgn[1]);
  rgn += 2;
  if (next_y == RGNSTOP)
    goto done_with_scanlines;

  /* If this is an empty scanline, don't loop, just move on. */
  if (sec == section)
    goto done_looping;

  sec->label = srcblt_stub_table[DONE_STUB];
  sec->offset = sec->arg = 0;  /* won't be used */
  /* don't bother incrementing sec here */

  /* Compute the number of rows to blt. */
  num_rows = next_y - y;

  /* Call the blitter function. */
  {
    int first_y;

    first_y = srcblt_reverse_scanlines_p ? (next_y - 1) : y;

    ((void (*)(const blt_section_t *, const uint8 *, uint8 *, long))
     srcblt_stub_table[FUNC_PTR]) (section,
				   ((const uint8 *) srcblt_src_baseaddr
				    + (first_y * srcblt_src_row_bytes)),
				   ((uint8 *) srcblt_dst_baseaddr
				    + (first_y * srcblt_dst_row_bytes)),
				   num_rows);
  }

 done_looping:
  y = next_y;
  goto start_scanline;

 done_with_scanlines:
  ;
}

#endif /* USE_PORTABLE_SRCBLT */

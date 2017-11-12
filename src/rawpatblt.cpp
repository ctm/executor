/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "rsys/xdblt.h"
#include "rsys/quick.h"
#include "rsys/rawblt.h"

#if defined(USE_PORTABLE_PATBLT)

/* This macro is used in `rawpatstubs.c' */
#define NEXT_ROW (dst = (uint32 *)&row_base[s[1].offset])

namespace Executor
{
#include "rawpatstubs.ctable"
}

using namespace Executor;

/* Dummy table, not actually dereferenced. */
const void *xdblt_nop_table[1] = { NULL };

const void **xdblt_ones_stubs[5] = {
    xdblt_copy_short_narrow_ones_labels,
    xdblt_copy_short_narrow_ones_labels,
    xdblt_xor_short_narrow_ones_labels,
    xdblt_nop_table,
    xdblt_insert_short_narrow_labels
};

const void **xdblt_zeros_stubs[5] = {
    xdblt_copy_short_narrow_zeros_labels,
    xdblt_nop_table,
    xdblt_nop_table,
    xdblt_copy_short_narrow_zeros_labels,
    xdblt_insert_short_narrow_labels
};

const void **xdblt_short_narrow_stubs[5] = {
    xdblt_copy_short_narrow_labels,
    xdblt_or_short_narrow_labels,
    xdblt_xor_short_narrow_labels,
    xdblt_and_short_narrow_labels,
    xdblt_insert_short_narrow_labels
};

const void **xdblt_tall_narrow_stubs[5] = {
    xdblt_copy_tall_narrow_labels,
    xdblt_or_tall_narrow_labels,
    xdblt_xor_tall_narrow_labels,
    xdblt_and_tall_narrow_labels,
    xdblt_insert_tall_narrow_labels
};

const void **xdblt_short_wide_stubs[5] = {
    xdblt_copy_short_wide_labels,
    xdblt_or_short_wide_labels,
    xdblt_xor_short_wide_labels,
    xdblt_and_short_wide_labels,
    xdblt_insert_short_wide_labels
};

const void **xdblt_tall_wide_stubs[5] = {
    xdblt_copy_tall_wide_labels,
    xdblt_or_tall_wide_labels,
    xdblt_xor_tall_wide_labels,
    xdblt_and_tall_wide_labels,
    xdblt_insert_tall_wide_labels
};

/* This code uses gotos and weird conventions because I translated
 * it from the original x86 assembly version.
 */
void xdblt_canon_pattern(void)
{
    blt_section_t section[MAX_BLT_SECTIONS], *sec;
    const INTEGER *rgn;
    int x, y, next_y, start_x_bit, stop_x_bit, start_x_long, stop_x_long;
    int num_rows, num_longs;
    uint32 mask;
    int log2_bpp;

    if(xdblt_stub_table == xdblt_nop_table)
        return;

    /* Store log2_bpp in a local variable, for speed. */
    log2_bpp = xdblt_log2_bpp;

    rgn = xdblt_rgn_start;
    y = CW_RAW(*rgn++);
    if(y == RGNSTOP)
        goto done_with_scanlines;

start_scanline:
    x = rgn[0];
    mask = 0;
    sec = section;
    if(x == RGNSTOP)
        goto fetch_next_y;

    start_x_bit = (x << log2_bpp) + xdblt_x_offset;

still_same_long:
    mask ^= xdblt_mask_array[start_x_bit & 31];
    start_x_long = start_x_bit >> 5;

next_stop:
    stop_x_bit = (rgn[1] << log2_bpp) + xdblt_x_offset;
    rgn += 2;
    stop_x_long = stop_x_bit >> 5;

    num_longs = stop_x_long - start_x_long;
    if(num_longs == 0)
        goto same_long;

    /* Different longs. */
    if(mask == (uint32)~0)
        goto blt_contig;

    /* Mask not solid. */
    sec->label = xdblt_stub_table[MASK_STUB];
    sec->offset = start_x_long << 2;
    sec->arg = mask;
    sec++;

    if(sec >= &section[MAX_BLT_SECTIONS])
        abort();

    /* account for that mask boundary long. */
    --num_longs;
    if(num_longs == 0)
        goto xfer_done;
    ++start_x_long;

blt_contig:
    sec->label = xdblt_stub_table[REPEAT_MOD_0_STUB
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
    start_x_bit = (rgn[0] << log2_bpp) + xdblt_x_offset;
    mask ^= xdblt_mask_array[stop_x_bit & 31];
    start_x_long = start_x_bit >> 5;
    if(start_x_long == stop_x_long)
        goto still_same_long;

    if(mask == 0)
        goto no_boundary_cruft;

    sec->label = xdblt_stub_table[MASK_STUB];
    sec->offset = stop_x_long << 2;
    sec->arg = mask;
    sec++;

    if(sec >= &section[MAX_BLT_SECTIONS])
        abort();

no_boundary_cruft:
    if(rgn[0] == RGNSTOP)
        goto fetch_next_y;
    mask = xdblt_mask_array[start_x_bit & 31];
    goto next_stop;

fetch_next_y:
    /* Fetch the starting y value for the next scanline.  When we
  * get here, rgn points to the RGNSTOP for the last scanline,
  * so we have to look two bytes farther to find the next y
  * value.  Special regions store y's as big endian.
  */
    next_y = CW_RAW(rgn[1]);
    rgn += 2;
    if(next_y == RGNSTOP)
        goto done_with_scanlines;

    /* If this is an empty scanline, don't loop, just move on. */
    if(sec == section)
        goto done_looping;

    sec->label = xdblt_stub_table[DONE_STUB];
    sec->offset = sec->arg = 0; /* won't be used */
    /* don't bother incrementing sec here */

    /* Compute the number of rows to blt. */
    num_rows = next_y - y;

    /* Call the blitter function. */
    ((void (*)(const blt_section_t *, uint8 *, long, long))
         xdblt_stub_table[FUNC_PTR])(section,
                                     ((uint8 *)xdblt_dst_baseaddr
                                      + (y * xdblt_dst_row_bytes)),
                                     num_rows, y);

done_looping:
    y = next_y;
    goto start_scanline;

done_with_scanlines:;
}

#endif /* USE_PORTABLE_PATBLT */

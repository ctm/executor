/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qScale[] = "$Id: qScale.c 87 2005-05-25 01:57:33Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"

#include "rsys/cquick.h"

using namespace Executor;

/* This routine scales old_bitmap and stores the result in dst_bitmap.
 * The only field of dst_bitmap that needs to be valid on entry is
 * baseAddr, which should point to enough information to hold the
 * resulting scaled bitmap, with rowBytes evenly divisble by 4.
 * dst_bitmap's bounds will be filled in such that "new_rect" will refer
 * to the newly scaled bits in that coordinate system.
 */
void Executor::scale_blt_bitmap(const blt_bitmap_t *src_bitmap, blt_bitmap_t *dst_bitmap,
                                const Rect *old_rect, const Rect *new_rect,
                                int log2_bits_per_pixel)
{
    long old_width, new_width, old_height, new_height;
    long y, dx, dy, left_x, src_rowbytes, dst_rowbytes, dst_byte_width, old_v;
    long rows_left;
    const uint8 *src_base;
    uint8 *dst_row_base;

    /* Fetch the sizes of the two bitmaps. */
    old_width = RECT_WIDTH(old_rect);
    new_width = RECT_WIDTH(new_rect);
    old_height = RECT_HEIGHT(old_rect);
    new_height = RECT_HEIGHT(new_rect);

    /* If the old bitmap was empty, just create a new, empty bitmap.  We
   * do this to avoid dividing by zero.
   */
    if(new_width == 0 || new_height == 0)
    {
        dst_bitmap->bounds.left = dst_bitmap->bounds.right
            = dst_bitmap->bounds.top = dst_bitmap->bounds.bottom = CWC(0);
        /*->*/ return;
    }

    /* Compute the scale ratio as a fixed-point number. */
    dx = (old_width << 16) / new_width;
    dy = (old_height << 16) / new_height;

    /* Compute some parameters for the main loop. */
    dst_byte_width = ((new_width << log2_bits_per_pixel) + 7) / 8;
    dst_rowbytes = (dst_byte_width + 3) & ~3; /* Divisible by 4. */
    src_rowbytes = BITMAP_ROWBYTES(src_bitmap);
    dst_row_base = (uint8 *)MR(dst_bitmap->baseAddr);
    src_base = (uint8 *)(MR(src_bitmap->baseAddr)
                         + ((CW(old_rect->top) - CW(src_bitmap->bounds.top)) * src_rowbytes));
    left_x = (CW(old_rect->left) - CW(src_bitmap->bounds.left)) << 16;
    old_v = -1;

/* This macro expresses the main horizontal scaling loop.  The bits
 * for each byte in the destination bitmap are grabbed and ORed together,
 * and then written out.
 */
#define SCALE_LOOP(x_count, scale_code)                                        \
    for(rows_left = new_height, y = 0; rows_left > 0; y += dy, rows_left--)    \
    {                                                                          \
        long v = y >> 16;                                                      \
        if(v == old_v)                                                         \
        {                                                                      \
            memcpy(dst_row_base, dst_row_base - dst_rowbytes, dst_byte_width); \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            long x, h;                                                         \
            const unsigned char *src_row_base;                                 \
                                                                               \
            /* Loop across this row. */                                        \
            src_row_base = &src_base[src_rowbytes * v];                        \
            for(h = 0, x = left_x; h < (x_count); h++)                         \
            {                                                                  \
                scale_code;                                                    \
            }                                                                  \
                                                                               \
            old_v = v;                                                         \
        }                                                                      \
        dst_row_base += dst_rowbytes;                                          \
    }

/* This helper macro grabs the bits corresponding to the x / 65536th pixel
 * from src_row_base on the current line, assuming the specified number of
 * bits per pixel.  FIXME: this code can read beyond the end of src_bitmap's
 * memory when collecting unneeded boundary pixels.
 */

#undef BITS
#define BITS(log2_bpp)                                                           \
    ((src_row_base[x >> (19 - (log2_bpp))] /* This is the containing byte.  */   \
      >> (((~(x >> 16)) & (7 >> (log2_bpp))) /* Pixel # within that byte.     */ \
          << (log2_bpp))) /* Scale by pixel size.          */                    \
     & ((1 << (1 << (log2_bpp))) - 1)) /* Mask out all but wanted bits. */

    /* #warning "Can look too far into memory for the boundary pixels" */

    switch(log2_bits_per_pixel)
    {
        case 0: /* 1 bpp */
            SCALE_LOOP(dst_byte_width,
                       {
                           unsigned char new1;
                           new1 = BITS(0) << 7;
                           x += dx;
                           new1 |= BITS(0) << 6;
                           x += dx;
                           new1 |= BITS(0) << 5;
                           x += dx;
                           new1 |= BITS(0) << 4;
                           x += dx;
                           new1 |= BITS(0) << 3;
                           x += dx;
                           new1 |= BITS(0) << 2;
                           x += dx;
                           new1 |= BITS(0) << 1;
                           x += dx;
                           new1 |= BITS(0);
                           x += dx;
                           dst_row_base[h] = new1;
                       });
            break;
        case 1: /* 2 bpp */
            SCALE_LOOP(dst_byte_width,
                       {
                           unsigned char new1;
                           new1 = BITS(1) << 6;
                           x += dx;
                           new1 |= BITS(1) << 4;
                           x += dx;
                           new1 |= BITS(1) << 2;
                           x += dx;
                           new1 |= BITS(1);
                           x += dx;
                           dst_row_base[h] = new1;
                       });
            break;
        case 2: /* 4 bpp */
            SCALE_LOOP(dst_byte_width,
                       {
                           unsigned char new1;
                           new1 = BITS(2) << 4;
                           x += dx;
                           new1 |= BITS(2);
                           x += dx;
                           dst_row_base[h] = new1;
                       });
            break;
        case 3: /* 8 bpp */
            SCALE_LOOP(dst_byte_width,
                       {
                           dst_row_base[h] = BITS(3);
                           x += dx;
                       });
            break;
        case 4:
            SCALE_LOOP(new_width,
                       {
                           ((uint16 *)dst_row_base)[h]
                               = ((uint16 *)src_row_base)[x >> 16];
                           x += dx;
                       });
            break;
        case 5:
            SCALE_LOOP(new_width,
                       {
                           ((uint32 *)dst_row_base)[h]
                               = ((uint32 *)src_row_base)[x >> 16];
                           x += dx;
                       });
            break;
        default:
            gui_fatal("invalid target depth %d",
                      (1 << log2_bits_per_pixel));
    }

    BITMAP_SET_ROWBYTES_X(dst_bitmap, CW(dst_rowbytes));
    dst_bitmap->bounds = *new_rect;
}

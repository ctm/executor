/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "rsys/srcblt.h"
#include "rsys/quick.h"
#include "rsys/xdblt.h"
#include "rsys/vdriver.h"
#include "rsys/prefs.h"
#include "rsys/host.h"

// FIXME: #warning This seems unsafe...

using namespace Executor;

int srcblt_log2_bpp asm("_srcblt_log2_bpp");

const INTEGER *srcblt_rgn_start asm("_srcblt_rgn_start");

const void **srcblt_stub_table asm("_srcblt_stub_table");

int32 srcblt_x_offset asm("_srcblt_x_offset");

int32 srcblt_src_row_bytes asm("_srcblt_src_row_bytes");
int32 srcblt_dst_row_bytes asm("_srcblt_dst_row_bytes");

uint32 srcblt_fg_color asm("_srcblt_fg_color");
uint32 srcblt_bk_color asm("_srcblt_bk_color");

char *srcblt_src_baseaddr asm("_srcblt_src_baseaddr");
char *srcblt_dst_baseaddr asm("_srcblt_dst_baseaddr");

int srcblt_shift_offset asm("_srcblt_shift_offset");

bool srcblt_reverse_scanlines_p asm("_srcblt_reverse_scanlines_p");

#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
uint16 srcblt_src_selector asm("_srcblt_src_selector");
uint16 srcblt_dst_selector asm("_srcblt_dst_selector");
#endif

/* We use this macro to avoid page faults when aligning pointers. */
#define MIN_PAGE_SIZE 512

bool srcblt_rgn(RgnHandle rh, int mode, int log2_bpp,
                const blt_bitmap_t *src, const blt_bitmap_t *dst,
                GUEST<Point> *src_origin, GUEST<Point> *dst_origin,
                uint32 fg_color, uint32 bk_color)
{
    uint32 mask, tile;
    unsigned long dst_align32_offset, src_align32_offset;
    long src_x_offset, src_y_offset, left_shift;
    char *dst_baseaddr, *src_baseaddr;
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    bool cursor_maybe_changed_p, old_vis_p;
#endif
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
    bool needs_seg_override_p;
#endif

    /* Record log2 bpp. */
    srcblt_log2_bpp = log2_bpp;

    /* Tile fg and bk colors out to 32bpp. */
    mask = ROMlib_pixel_size_mask[log2_bpp];
    tile = ROMlib_pixel_tile_scale[log2_bpp];
    srcblt_fg_color = (fg_color & mask) * tile;
    srcblt_bk_color = (bk_color & mask) * tile;

    /* Canonicalize mode for RGB. */
    if(log2_bpp > 3)
        mode ^= (srcCopy ^ notSrcCopy);

    mode &= 7;

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    cursor_maybe_changed_p = old_vis_p = false;
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P())
    {
        int top, left;
        RgnPtr rp = STARH(rh);

        if(active_screen_addr_p(src))
        {
            srcblt_src_row_bytes = vdriver_real_screen_row_bytes;
            src_baseaddr = (char *)vdriver_real_screen_baseaddr;
            if(vdriver_flip_real_screen_pixels_p)
                mode ^= (srcCopy ^ notSrcCopy);
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
            srcblt_src_selector = vga_screen_selector;
#endif

            /* I'm a lazy bastard and don't want to figure out the
	   * coordinate system sludge.  Copying from the screen is
	   * uncommon anyway.
	   */
            old_vis_p = host_set_cursor_visible(false);
            cursor_maybe_changed_p = true;
        }
        else
        {
            srcblt_src_row_bytes = CW(src->rowBytes) & ROWBYTES_VALUE_BITS;
            src_baseaddr = (char *)MR(src->baseAddr);
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
            asm("movw %%ds,%0"
                : "=m"(srcblt_src_selector));
#endif
        }
        if(active_screen_addr_p(dst))
        {
            srcblt_dst_row_bytes = vdriver_real_screen_row_bytes;
            dst_baseaddr = (char *)vdriver_real_screen_baseaddr;
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
            srcblt_dst_selector = vga_screen_selector;
#endif
            top = CW(dst->bounds.top);
            left = CW(dst->bounds.left);

            /* Hide the cursor if necessary. */
            old_vis_p |= (host_hide_cursor_if_intersects(CW(rp->rgnBBox.top) - top,
                                                         CW(rp->rgnBBox.left) - left,
                                                         CW(rp->rgnBBox.bottom) - top,
                                                         CW(rp->rgnBBox.right) - left));
            cursor_maybe_changed_p = true;
        }
        else
        {
            srcblt_dst_row_bytes = CW(dst->rowBytes) & ROWBYTES_VALUE_BITS;
            dst_baseaddr = (char *)MR(dst->baseAddr);
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
            asm("movw %%ds,%0"
                : "=m"(srcblt_dst_selector));
#endif
        }
    }
    else
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */
    {
        /* Default to values for non-screen blit. */
        srcblt_src_row_bytes = CW(src->rowBytes) & ROWBYTES_VALUE_BITS;
        srcblt_dst_row_bytes = CW(dst->rowBytes) & ROWBYTES_VALUE_BITS;
        src_baseaddr = (char *)MR(src->baseAddr);
        dst_baseaddr = (char *)MR(dst->baseAddr);
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
        asm("movw %%ds,%0\n\t"
            "movw %%ds,%1"
            : "=m"(srcblt_src_selector), "=m"(srcblt_dst_selector));
#endif
    }

    /* Compute the offset to map dst y coords to src bitmap coords.*/
    src_y_offset = (CW(src_origin->v) - CW(src->bounds.top)
                    - CW(dst_origin->v));
    src_baseaddr += src_y_offset * srcblt_src_row_bytes;
    dst_baseaddr -= CW(dst->bounds.top) * srcblt_dst_row_bytes;

    /* Handle the common case of flipped fg/bk colors and a copy xfer mode. */
    if((mode & 3) == (srcCopy & 3) /* either srcCopy or notSrcCopy */
       && srcblt_fg_color == 0 && srcblt_bk_color == (uint32)~0)
    {
        mode ^= (srcCopy ^ notSrcCopy);
        srcblt_bk_color = 0;
        srcblt_fg_color = ~0;
    }

    srcblt_x_offset = -(CW(dst->bounds.left) << log2_bpp);

    src_x_offset = (((CW(src_origin->h) - CW(src->bounds.left))
                     - (CW(dst_origin->h) - CW(dst->bounds.left)))
                    << log2_bpp);
    src_baseaddr += (src_x_offset >> 3);
    left_shift = src_x_offset & 7;

    dst_align32_offset = (unsigned long)dst_baseaddr & 3;

    /* Only align dst % 4 bytes when that cannot cause src to get pushed
   * across a page boundary (which might cause a segfault).  If that
   * fails, we'll try to align src % 4 bytes.
   */
    if(dst_align32_offset)
    {
        int offset;

        offset = -1; /* default value. */
        if(((unsigned long)src_baseaddr & (MIN_PAGE_SIZE - 1))
           >= dst_align32_offset)
            offset = dst_align32_offset;
        else if(!left_shift)
        {
            /* Might as well align src if we can't align dst. */
            src_align32_offset = (unsigned long)src_baseaddr & 3;

            if(((unsigned long)dst_baseaddr & (MIN_PAGE_SIZE - 1))
               >= src_align32_offset)
                offset = src_align32_offset;
        }

        if(offset > 0)
        {
            int bit_offset = offset * 8;

            srcblt_fg_color = ((srcblt_fg_color >> bit_offset)
                               | (srcblt_fg_color << (32 - bit_offset)));
            srcblt_bk_color = ((srcblt_bk_color >> bit_offset)
                               | (srcblt_bk_color << (32 - bit_offset)));

            srcblt_x_offset += bit_offset;
            dst_baseaddr -= offset;
            src_baseaddr -= offset;
        }
    }

    /* If we are forced to do bit shifting anyway, we might as well
   * long-align the source bitmap and increase the shift count.
   */
    if(left_shift)
    {
        src_align32_offset = (unsigned long)src_baseaddr & 3;
        src_baseaddr -= src_align32_offset;
        left_shift += src_align32_offset * 8;
    }

    srcblt_shift_offset = left_shift;
    srcblt_src_baseaddr = src_baseaddr;
    srcblt_dst_baseaddr = dst_baseaddr;

    /* Note whether we should reverse the order in which we process
   * scanlines.  This trick will only work for certain simple regions
   * (e.g. those with only one repeated scanline).  At the moment,
   * more complex regions require the bitmap be copied offscreen to a
   * temp buffer.
   */
    srcblt_reverse_scanlines_p = (src_baseaddr < dst_baseaddr);

#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
    needs_seg_override_p = (srcblt_src_selector != srcblt_dst_selector);
#define FIRST_DIM [needs_seg_override_p]
#else
#define FIRST_DIM
#endif

    if(left_shift == 0)
    {
        if(srcblt_fg_color == (uint32)~0 && srcblt_bk_color == 0)
            srcblt_stub_table = srcblt_noshift_stubs FIRST_DIM[mode];
        else
            srcblt_stub_table = srcblt_noshift_fgbk_stubs FIRST_DIM[mode];
    }
    else
    {
#if defined(USE_PORTABLE_SRCBLT) || !defined(i386)
        if(srcblt_fg_color == (uint32)~0 && srcblt_bk_color == 0)
            srcblt_stub_table = srcblt_shift_stubs[mode];
        else
            srcblt_stub_table = srcblt_shift_fgbk_stubs[mode];
#else /* i386 */
        if(arch_type == ARCH_TYPE_I386)
        {
            /* i386 */
            if(srcblt_fg_color == (uint32)~0 && srcblt_bk_color == 0)
                srcblt_stub_table = srcblt_shift_i386_stubs FIRST_DIM[mode];
            else
                srcblt_stub_table = srcblt_shift_fgbk_i386_stubs FIRST_DIM[mode];
        }
        else
        {
            /* i486 or better */
            if(srcblt_fg_color == (uint32)~0 && srcblt_bk_color == 0)
                srcblt_stub_table = srcblt_shift_i486_stubs FIRST_DIM[mode];
            else
                srcblt_stub_table = srcblt_shift_fgbk_i486_stubs FIRST_DIM[mode];
        }
#endif /* i386 */
    }

    SETUP_SPECIAL_RGN(rh, srcblt_rgn_start);

    /* Make sure we have access to the raw screen bits. */
    vdriver_accel_wait();

    /* Actually do the blit. */
    srcblt_bitmap();

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(cursor_maybe_changed_p)
        host_set_cursor_visible(old_vis_p);
#endif

    return !VDRIVER_BYPASS_INTERNAL_FBUF_P();
}

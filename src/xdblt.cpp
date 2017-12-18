/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/xdata.h"
#include "rsys/xdblt.h"
#include "rsys/vdriver.h"
#if defined(USE_VGAVDRIVER)
#include "rsys/vgavdriver.h"
#endif

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/dirtyrect.h"
#include "rsys/prefs.h"
#include "rsys/host.h"
#include "rsys/autorefresh.h"

using namespace Executor;

/* Holds the four-byte pattern value, for "short & narrow" patterns. */
uint32_t xdblt_pattern_value asm("_xdblt_pattern_value");

/* Holds the row bytes for the pattern.  Always evenly divisible by four. */
uint32_t xdblt_log2_pattern_row_bytes asm("_xdblt_log2_pattern_row_bytes");

/* Holds the row bytes for the pattern.  Always evenly divisible by four. */
uint32_t xdblt_pattern_height_minus_1 asm("_xdblt_pattern_height_minus_1");

/* Start and end of the pattern. */
uint32_t *xdblt_pattern_baseaddr asm("_xdblt_pattern_baseaddr");
uint32_t *xdblt_pattern_end asm("_xdblt_pattern_end");

/* Since the pattern may be rotated vertically, we add this to the
 * row we would otherwise compute to display on bitmap row #N.
 */
uint32_t xdblt_pattern_row_0 asm("_xdblt_pattern_row_0");

/* Table of functions that describe this transfer mode. */
const void **xdblt_stub_table asm("_xdblt_stub_table");

/* log base 2 of the bits per pixel of the destination bitmap [0, 5]. */
uint32_t xdblt_log2_bpp asm("_xdblt_log2_bpp");

/* "Special region" data to be transferred. */
const INTEGER *xdblt_rgn_start asm("_xdblt_rgn_start");

/* Added to the X value grabbed from the region to get the "real" value. */
uint32_t xdblt_x_offset asm("_xdblt_x_offset");

/* Canonicalized base address of the destination bitmap.  We offset
 * the bitmap's base so that (region_y_val * dst_rowbytes) is the
 * offset from the baseaddr to the proper row.
 */
uint32_t *xdblt_dst_baseaddr asm("_xdblt_dst_baseaddr");

/* Bytes per row of the destination bitmap. */
uint32_t xdblt_dst_row_bytes asm("_xdblt_dst_row_bytes");

/* Bit pattern inserted for bit insertion modes. */
uint32_t xdblt_insert_bits asm("_xdblt_insert_bits");

#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
uint16_t xdblt_dst_selector asm("_xdblt_dst_selector");
#endif

#define M(n) CLC_RAW(0xFFFFFFFFU >> (n))
const uint32_t xdblt_mask_array[32] asm("_xdblt_mask_array") = {
    M(0), M(1), M(2), M(3), M(4), M(5), M(6), M(7),
    M(8), M(9), M(10), M(11), M(12), M(13), M(14), M(15),
    M(16), M(17), M(18), M(19), M(20), M(21), M(22), M(23),
    M(24), M(25), M(26), M(27), M(28), M(29), M(30), M(31)
};
#undef M

/* Since we map all modes to one of { copy, or, xor, and }, sometimes we
 * need to flip the pattern bits.  This table tells us when to do that.
 */
static const uint32_t flip_mask_for_mode[8] = { 0, 0, 0, ~(uint32_t)0, ~(uint32_t)0, ~(uint32_t)0, ~(uint32_t)0, 0 };

/* This macro rotates the specified 32 bit value right by the given
 * number of bits and modifies the input value.  It can only be called
 * if 0 < count < 32; otherwise, the result is undefined.
 */
#if defined(__x86_64__)
#define RORL(count, n)       \
    asm("rorl %%cl,%0"       \
        : "=g"(n)            \
        : "c"(count), "0"(n) \
        : "cc");
#elif defined(mc68000)
#define RORL(count, n)       \
    asm("rorl %1,%0"         \
        : "=d"(n)            \
        : "d"(count), "0"(n) \
        : "cc");
#else /* !i386 && !mc68000 */
#define RORL(count, n)                                        \
    do                                                        \
    {                                                         \
        uint32_t _tmp_ = (n);                                   \
        int _count_ = (count);                                \
        (n) = (_tmp_ >> _count_) | (_tmp_ << (32 - _count_)); \
    } while(0)
#endif /* !i386 && !mc68000 */

/* Sometimes we call internal routines but we don't want the mode to
 * get touched like it normally does.  As a workaround we mark the mode
 * as "canonical" by ORing in some magic high bits.
 */
#define MODE_CANON_BITS 0x73A12300
#define MODE_CANON_MASK 0xFFFFFF00
#define MODE_CANON_P(m) (((m)&MODE_CANON_MASK) == MODE_CANON_BITS)

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
static inline bool
hide_cursor_if_necessary(RgnHandle rh, const PixMap *dst, bool *old_vis)
{
    int top, left;
    RgnPtr rp;

    top = CW(dst->bounds.top);
    left = CW(dst->bounds.left);
    rp = STARH(rh);

    *old_vis = host_hide_cursor_if_intersects(CW(rp->rgnBBox.top) - top,
                                              CW(rp->rgnBBox.left) - left,
                                              CW(rp->rgnBBox.bottom) - top,
                                              CW(rp->rgnBBox.right) - left);

    return true;
}
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

static inline int
setup_dst_bitmap(int log2_bpp, PixMap *dst_pixmap)
{
    char *dst;
    int byte_slop;
    int row_bytes;

/* Long-align the bitmap, and set it up so that
   * baseaddr + y * row_bytes evaluates to the correct row.
   */
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P()
       && active_screen_addr_p(dst_pixmap))
    {
        row_bytes = vdriver_real_screen_row_bytes;
        dst = (char *)vdriver_real_screen_baseaddr;
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
        xdblt_dst_selector = vga_screen_selector;
#endif
    }
    else
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */
    {
        row_bytes = BITMAP_ROWBYTES(dst_pixmap);
        dst = (char *)MR(dst_pixmap->baseAddr);
#if defined(VGA_SCREEN_NEEDS_FAR_PTR)
        asm("movw %%ds,%0"
            : "=m"(xdblt_dst_selector));
#endif
    }

    dst -= row_bytes * CW(dst_pixmap->bounds.top);
    xdblt_dst_row_bytes = row_bytes;
    byte_slop = (unsigned long)dst & 3;
    xdblt_dst_baseaddr = (uint32_t *)(dst - byte_slop);

    xdblt_x_offset = ((byte_slop << 3)
                      - (CW(dst_pixmap->bounds.left) << log2_bpp));

    return xdblt_x_offset << 3;
}

INTEGER phony_special_region[7] = { 0, 0, 0, RGN_STOP, 0, RGN_STOP, RGN_STOP_X };

/* This is the fastest blitter function.  It can be called when the
 * xdata is one row tall and 4 bytes wide, the xdata pixels are not
 * RGB, and when no different value can be achieved by rotating the
 * pattern value by any multiple of the bits per pixel.  Some examples
 * are 0 or ~0 at any bpp, 0xA1A1A1A1 at 8bpp, 0x55555555 at 2bpp, etc.
 */
bool Executor::xdblt_xdata_norgb_norotate(RgnHandle rh, int mode,
                                          int pat_x_rotate_count, int pat_y_rotate_count,
                                          xdata_t *x, PixMap *dst)
{
    RgnPtr r;
    int log2_bpp;
    bool active_screen_p;
    vdriver_accel_result_t accel_result;
    bool mode_canon_p;
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    bool cursor_maybe_changed_p, cursor_vis_p;
#endif

    mode_canon_p = MODE_CANON_P(mode);
    mode &= 7;

    active_screen_p = active_screen_addr_p(dst);

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    /* If we can blit directly to the real screen, change the mode
   * appropriately.
   */
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P()
       && active_screen_p)
    {
        cursor_maybe_changed_p = hide_cursor_if_necessary(rh, dst,
                                                          &cursor_vis_p);
    }
    else
        cursor_maybe_changed_p = false;
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

    if(!mode_canon_p)
    {
        xdblt_pattern_value = x->pat_value ^ flip_mask_for_mode[mode];
        mode &= 3;
    }

    if(active_screen_p
       && mode == (patCopy & 3) /* patCopy or notPatCopy */
       && (r = STARH(rh), r->rgnSize == RGN_SMALL_SIZE_X))
    {
        int top, left;

        top = CW(dst->bounds.top);
        left = CW(dst->bounds.left);

        accel_result = vdriver_accel_rect_fill(CW(r->rgnBBox.top) - top, CW(r->rgnBBox.left) - left,
                                               CW(r->rgnBBox.bottom) - top, CW(r->rgnBBox.right) - left,
                                               xdblt_pattern_value & ROMlib_pixel_size_mask[x->log2_bpp]);

        if(accel_result != VDRIVER_ACCEL_NO_UPDATE)
            note_executor_changed_screen(CW(r->rgnBBox.top) - top,
                                         CW(r->rgnBBox.bottom) - top);
    }
    else
        accel_result = VDRIVER_ACCEL_NO_UPDATE;

    if(accel_result != VDRIVER_ACCEL_FULL_UPDATE)
    {
        xdblt_log2_pattern_row_bytes = 2;
        xdblt_pattern_row_0 = 0;
        xdblt_pattern_baseaddr = &xdblt_pattern_value;
        xdblt_pattern_end = &xdblt_pattern_value + 1;
        xdblt_stub_table = x->stub_table_for_mode[mode];
        xdblt_log2_bpp = log2_bpp = x->log2_bpp;

        setup_dst_bitmap(log2_bpp, dst);
        SETUP_SPECIAL_RGN(rh, xdblt_rgn_start);

        /* Make sure we have access to the raw screen bits. */
        vdriver_accel_wait();

        /* Actually do the blit. */
        xdblt_canon_pattern();
    }

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(cursor_maybe_changed_p)
        host_set_cursor_visible(cursor_vis_p);
#endif

    return (!VDRIVER_BYPASS_INTERNAL_FBUF_P()
            && accel_result == VDRIVER_ACCEL_NO_UPDATE);
}

/* Contrary to my expectation, a small test program shows that RGB
 * pattern modes do not seem to get mapped to different low-level
 * modes.  Exactly the same boolean transfer function happens in 16bpp
 * as in 8bpp.
 */

#if defined(RGB_NEEDS_MODE_MAPPING)

/* This table maps normal modes to the mode to use when the transfer
 * involves RGB pixels.  This is necessary because the 0 bits of
 * indirect pixels are white, while the 0 bits of indirect pixels are
 * black.  Consequently, the transfer modes have different semantics
 * when viewed at a raw bit level.  */
static const int
    ind_mode_to_rgb_mode[8]
    = {
        patCopy & 7, /* patCopy    -> patCopy    */
        notPatBic & 7, /* patOr      -> notPatBic  */
        patXor & 7, /* patXor     -> patXor     */
        notPatOr & 7, /* patBic     -> notPatOr   */
        notPatCopy & 7, /* notPatCopy -> notPatCopy */
        patBic & 7, /* notPatOr   -> patBic     */
        notPatXor & 7, /* notPatXor  -> notPatXor  */
        patOr & 7, /* notPatBic  -> patOr      */
      };

#endif /* RGB_NEEDS_MODE_MAPPING */

bool Executor::xdblt_xdata_short_narrow(RgnHandle rh, int mode,
                                        int pat_x_rotate_count, int pat_y_rotate_count,
                                        xdata_t *x, PixMap *dst)
{
    uint32_t v, flip;
    int rcount, log2_bpp;
    bool mode_canon_p;
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    bool cursor_maybe_changed_p, cursor_vis_p;
#endif

    mode_canon_p = MODE_CANON_P(mode);
    mode &= 7;

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    /* If we can blit directly to the real screen, change the mode
   * appropriately.
   */
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P()
       && active_screen_addr_p(dst))
    {
        cursor_maybe_changed_p = hide_cursor_if_necessary(rh, dst,
                                                          &cursor_vis_p);
    }
    else
        cursor_maybe_changed_p = false;
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

#if defined(RGB_NEEDS_MODE_MAPPING)
    if(x->rgb_spec && !mode_canon_p)
        mode = ind_mode_to_rgb_mode[mode];
#endif

    if(!mode_canon_p)
        xdblt_stub_table = x->stub_table_for_mode[mode & 3];
    else
        xdblt_stub_table = x->stub_table_for_mode[mode];
    if(xdblt_stub_table == xdblt_nop_table)
        return false;

    xdblt_log2_bpp = log2_bpp = x->log2_bpp;

    pat_x_rotate_count <<= log2_bpp;
    pat_x_rotate_count += setup_dst_bitmap(log2_bpp, dst);

    SETUP_SPECIAL_RGN(rh, xdblt_rgn_start);

    if(mode_canon_p)
        flip = 0;
    else
        flip = flip_mask_for_mode[mode];
    if(x->rgb_spec)
        flip &= x->rgb_spec->pixel_bits_mask;
    v = x->pat_value ^ flip;
    rcount = pat_x_rotate_count & 31;
    if(rcount)
        RORL(rcount, v);
    xdblt_pattern_value = v;
    xdblt_log2_pattern_row_bytes = 2;
    xdblt_pattern_row_0 = 0;
    xdblt_pattern_baseaddr = &xdblt_pattern_value;
    xdblt_pattern_end = &xdblt_pattern_value + 1;

    /* Make sure we have access to the raw screen bits. */
    vdriver_accel_wait();

    /* Actually do the blit. */
    xdblt_canon_pattern();

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(cursor_maybe_changed_p)
        host_set_cursor_visible(cursor_vis_p);
#endif

    return !VDRIVER_BYPASS_INTERNAL_FBUF_P();
}

/* This function XORs FLIP_MASK with all of the longs in the xdata,
 * and then rotates the given xdata by XROT bits.
 */
static inline void
rotate_and_flip_xdata(xdata_t *x, int xrot, uint32_t flip_mask)
{
    /* First flip all bits appropriately. */
    if(flip_mask != 0)
    {
        uint32_t *p, *e;
        e = xdblt_pattern_end;
        for(p = xdblt_pattern_baseaddr; p != e; p++)
            *p ^= flip_mask;
    }

    /* Now rotate them appropriately. */
    if(xrot != 0)
    {
        int row_bytes = (1 << x->log2_row_bytes);

        if(row_bytes == 4)
        {
            uint32_t *p, *e;
            e = xdblt_pattern_end;
            for(p = xdblt_pattern_baseaddr; p != e; p++)
                RORL(xrot, *p);
        }
        else
        {
            uint8 *scratch, *s, *p, *e;
            int rs, bs;

            /* The following code rotates the pattern array right by
	   * XROT bits, assuming the bits are stored in bits-big-endian
	   * byte order.
	   */

            scratch = (uint8 *)alloca(x->byte_size);
            e = (uint8 *)xdblt_pattern_end;

            bs = xrot >> 3;
            rs = xrot & 7;

            if(rs != 0)
            {
                int ls;
                unsigned xmask;

                xmask = row_bytes - 1;
                ls = 8 - rs;

                /* This is the tricky case, where we aren't
	       * rotating by an integral number of bytes.
	       */

                for(p = (uint8 *)xdblt_pattern_baseaddr, s = scratch;
                    p != e;
                    p += row_bytes, s += row_bytes)
                {
                    uint8 next;
                    int i;

                    next = p[0];
                    for(i = xmask; i >= 0; i--)
                    {
                        uint8 v = p[i];
                        s[(i + bs) & xmask] = ((v >> rs) | (next << ls));
                        next = v;
                    }
                }

                memcpy(xdblt_pattern_baseaddr, scratch, x->byte_size);
            }
            else
            {
                /* Special case for when we are rotating an integral number
	       * of bytes.
	       */
                p = (uint8 *)xdblt_pattern_baseaddr;
                memcpy(scratch, p, x->byte_size);
                for(s = scratch; p != e; p += row_bytes, s += row_bytes)
                {
                    memcpy(p, s + row_bytes - bs, bs);
                    memcpy(p + bs, s, row_bytes - bs);
                }
            }
        }
    }
}

bool Executor::xdblt_xdata_complex(RgnHandle rh, int mode,
                                   int pat_x_rotate_count, int pat_y_rotate_count,
                                   xdata_t *x, PixMap *dst)
{
    const char *base;
    uint32_t flip_mask;
    bool mode_canon_p;
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    bool cursor_maybe_changed_p, cursor_vis_p;
#endif

    mode_canon_p = MODE_CANON_P(mode);
    mode &= 7;

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    /* If we can blit directly to the real screen, change the mode
   * appropriately.
   */
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P()
       && active_screen_addr_p(dst))
    {
        cursor_maybe_changed_p = hide_cursor_if_necessary(rh, dst,
                                                          &cursor_vis_p);
    }
    else
        cursor_maybe_changed_p = false;
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

#if defined(RGB_NEEDS_MODE_MAPPING)
    if(x->rgb_spec && !mode_canon_p)
        mode = ind_mode_to_rgb_mode[mode];
#endif

    base = (const char *)x->pat_bits;
    xdblt_pattern_baseaddr = (uint32_t *)base;
    xdblt_pattern_end = (uint32_t *)(&base[x->byte_size]);

    pat_x_rotate_count <<= x->log2_bpp;
    pat_x_rotate_count += setup_dst_bitmap(x->log2_bpp, dst);

    if(mode_canon_p)
        flip_mask = 0;
    else
        flip_mask = flip_mask_for_mode[mode];
    rotate_and_flip_xdata(x, ((pat_x_rotate_count - x->pat_x_rot)
                              & x->row_bits_minus_1),
                          flip_mask ^ x->pat_flip_mask);
    x->pat_flip_mask = flip_mask;
    x->pat_x_rot = pat_x_rotate_count;

    xdblt_log2_pattern_row_bytes = x->log2_row_bytes;
    xdblt_log2_bpp = x->log2_bpp;
    if(mode_canon_p)
        xdblt_stub_table = x->stub_table_for_mode[mode];
    else
        xdblt_stub_table = x->stub_table_for_mode[mode & 3];
    xdblt_pattern_row_0 = (-pat_y_rotate_count) & x->height_minus_1;
    xdblt_pattern_height_minus_1 = x->height_minus_1;
    SETUP_SPECIAL_RGN(rh, xdblt_rgn_start);

    /* Make sure we have access to the raw screen bits. */
    vdriver_accel_wait();

    xdblt_canon_pattern();

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(cursor_maybe_changed_p)
        host_set_cursor_visible(cursor_vis_p);
#endif

    return !VDRIVER_BYPASS_INTERNAL_FBUF_P();
}

static bool
do_short_narrow_pattern(RgnHandle rh, int mode, uint32_t v, PixMap *dst,
                        uint32_t fg_color, uint32_t bk_color, int log2_bpp,
                        const rgb_spec_t *rgb_spec, int pat_x_rotate_count)
{
    int extra_rot, raw_mode;
    uint32_t tv, op_color;
    RgnPtr r;
    vdriver_accel_result_t accel_result;

    extra_rot = setup_dst_bitmap(log2_bpp, dst);

    /* Flip the bits if it's a "not" mode. */
    if(mode & (patCopy ^ notPatCopy))
        v ^= ~0;

    /* Set up the xdblt mode for the given QuickDraw mode. */
    switch(mode & 3)
    {
        case(patCopy & 3):
        default: /* Not possible to hit default case w/2 bit switch. */
            v = (v & fg_color) | ((~v) & bk_color);
            if(rgb_spec)
                v = (v & rgb_spec->pixel_bits_mask) ^ rgb_spec->xor_mask;
            raw_mode = XDBLT_COPY;
            break;

        case(patXor & 3):
            if(rgb_spec)
                v &= rgb_spec->pixel_bits_mask;
            raw_mode = XDBLT_XOR;
            break;

        case(patOr & 3):
        case(patBic & 3):
            if(v == 0 || (rgb_spec && (v & rgb_spec->pixel_bits_mask) == 0))
                return false;
            op_color = ((mode & 3) == (patOr & 3)) ? fg_color : bk_color;

            if(v == (uint32_t)~0 || (rgb_spec && v == rgb_spec->white_pixel))
            {
                /* Just a copy. */
                v = op_color;
                if(rgb_spec)
                    v = (v & rgb_spec->pixel_bits_mask) ^ rgb_spec->xor_mask;
                raw_mode = XDBLT_COPY;
            }
            else if((op_color & v) == v
                    || (rgb_spec && op_color == rgb_spec->white_pixel))
            {
                /* Just an or. */
                if(rgb_spec)
                    v &= rgb_spec->pixel_bits_mask;
                raw_mode = XDBLT_OR;
            }
            else if((op_color & v) == 0
                    || (rgb_spec && op_color == rgb_spec->black_pixel))
            {
                /* Just an and. */
                v ^= ~0;
                if(rgb_spec)
                    v |= ~rgb_spec->pixel_bits_mask;
                raw_mode = XDBLT_AND;
            }
            else /* Tricky case; insert colored bit wherever "v" has a 1 bit. */
            {
                xdblt_insert_bits = op_color;
                if(rgb_spec)
                {
                    xdblt_insert_bits = (rgb_spec->xor_mask
                                         ^ (xdblt_insert_bits
                                            & rgb_spec->pixel_bits_mask));
                    v |= ~rgb_spec->pixel_bits_mask; /* not really necessary. */
                }
                raw_mode = XDBLT_INSERT;
            }

            break;
    }

    if(v == 0)
    {
        xdblt_stub_table = xdblt_zeros_stubs[raw_mode];
    }
    else if(v == (uint32_t)~0)
    {
        xdblt_stub_table = xdblt_ones_stubs[raw_mode];
    }
    else
    {
        int rcount = ((pat_x_rotate_count << log2_bpp) + extra_rot) & 31;
        if(rcount)
            RORL(rcount, v);
        xdblt_stub_table = xdblt_short_narrow_stubs[raw_mode];
    }

    xdblt_pattern_value = v;

    /* We can only use the accelerated func for solid color patCopy/notPatCopy
   * to the screen.
   */

    tv = v;
    if(log2_bpp < 5)
        RORL(1 << log2_bpp, tv);

    if(v == tv
       && raw_mode == XDBLT_COPY
       && (r = STARH(rh), r->rgnSize == RGN_SMALL_SIZE_X)
       && active_screen_addr_p(dst))
    {
        int top, left;

        top = CW(dst->bounds.top);
        left = CW(dst->bounds.left);

        accel_result = vdriver_accel_rect_fill(CW(r->rgnBBox.top) - top, CW(r->rgnBBox.left) - left,
                                               CW(r->rgnBBox.bottom) - top, CW(r->rgnBBox.right) - left,
                                               xdblt_pattern_value & ROMlib_pixel_size_mask[log2_bpp]);

        if(accel_result != VDRIVER_ACCEL_NO_UPDATE)
            note_executor_changed_screen(CW(r->rgnBBox.top) - top,
                                         CW(r->rgnBBox.bottom) - top);
    }
    else
        accel_result = VDRIVER_ACCEL_NO_UPDATE;

    if(accel_result != VDRIVER_ACCEL_FULL_UPDATE)
    {
        xdblt_log2_bpp = log2_bpp;
        xdblt_log2_pattern_row_bytes = 2;
        xdblt_pattern_row_0 = 0;
        xdblt_pattern_height_minus_1 = 0;
        xdblt_pattern_baseaddr = &xdblt_pattern_value;
        xdblt_pattern_end = &xdblt_pattern_value + 1;

        /* Set up the region appropriately. */
        SETUP_SPECIAL_RGN(rh, xdblt_rgn_start);

        /* Make sure we have access to the raw screen bits. */
        vdriver_accel_wait();

        /* Actually do the blit. */
        xdblt_canon_pattern();
    }

    return (!VDRIVER_BYPASS_INTERNAL_FBUF_P()
            && accel_result == VDRIVER_ACCEL_NO_UPDATE);
}

static inline uint32_t
canonicalize_pat_value_for_mode(uint32_t v, int mode, uint32_t fg_color,
                                uint32_t bk_color, const rgb_spec_t *rgb_spec)
{
    if(mode & (patCopy ^ notPatCopy))
        v ^= ~0;

    switch(mode & 3)
    {
        case(patCopy & 3):
            v = (v & fg_color) | ((~v) & bk_color);
            if(rgb_spec)
                v = (v & rgb_spec->pixel_bits_mask) ^ rgb_spec->xor_mask;
            break;

        case(patXor & 3):
            if(rgb_spec)
                v &= rgb_spec->pixel_bits_mask;
            break;

        case(patOr & 3):
            if(rgb_spec)
                v &= rgb_spec->pixel_bits_mask;
            break;

        case(patBic & 3):
            if(bk_color == 0)
                v ^= ~0;
            else if(rgb_spec)
            {
                if(bk_color == rgb_spec->black_pixel)
                    v = ~(v & rgb_spec->pixel_bits_mask); /* Because we can "and" */
                else
                    v &= rgb_spec->pixel_bits_mask;
            }
            break;
    }

    return v;
}

bool Executor::xdblt_pattern(RgnHandle rh, int mode,
                             int pat_x_rotate_count, int pat_y_rotate_count,
                             const Pattern pattern, PixMap *dst,
                             uint32_t fg_color, uint32_t bk_color)
{
    uint32_t v, mask, tile, *p, *end;
    const rgb_spec_t *rgb_spec = NULL;
    int log2_bpp;
    bool update_dirty_p;
#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    bool cursor_maybe_changed_p, cursor_vis_p;
#endif

    rgb_spec = pixmap_rgb_spec(dst);

    /* Tile fg and bk colors out to 32bpp. */
    log2_bpp = ROMlib_log2[CW(dst->pixelSize)];
    mask = ROMlib_pixel_size_mask[log2_bpp];
    tile = ROMlib_pixel_tile_scale[log2_bpp];
    fg_color = (fg_color & mask) * tile;
    bk_color = (bk_color & mask) * tile;

    mode &= 7;

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    /* If we can blit directly to the real screen, change the mode
   * appropriately.
   */
    if(VDRIVER_BYPASS_INTERNAL_FBUF_P()
       && active_screen_addr_p(dst))
    {
        cursor_maybe_changed_p = hide_cursor_if_necessary(rh, dst,
                                                          &cursor_vis_p);
    }
    else
        cursor_maybe_changed_p = false;
#endif /* VDRIVER_SUPPORTS_REAL_SCREEN_BLITS */

#if defined(RGB_NEEDS_MODE_MAPPING)
    if(rgb_spec)
        mode = ind_mode_to_rgb_mode[mode];
#endif

    /* See if the pattern is 0x0000000000000000 or 0xFFFFFFFFFFFFFFFF.
   * These are _extremely_ common cases.
   */
    v = *(const uint32_t *)pattern;
    if((v + 1U) <= 1U && v == ((const uint32_t *)pattern)[1])
    {
        update_dirty_p = do_short_narrow_pattern(rh, mode, v, dst, fg_color,
                                                 bk_color, log2_bpp,
                                                 rgb_spec, pat_x_rotate_count);
    }
    else
    {
        xdata_handle_t xh = xdata_for_pattern(pattern, dst);

        {
            HLockGuard guard(xh);
            xdata_t *x = STARH(xh);
            p = x->pat_bits;
            if(p)
            {
                int raw_mode;

                end = (uint32_t *)((char *)p + x->byte_size);
                for(; p != end; p++)
                {
                    /* This is a little slow, but who cares about this
		    * case?  I can easily speed this up later if need be.
		    */
                    *p = canonicalize_pat_value_for_mode(*p, mode, fg_color,
                                                         bk_color, rgb_spec);
                }

                if((mode & 3) == (patBic & 3)
                   && !(bk_color == 0
                        || (rgb_spec && bk_color == rgb_spec->black_pixel)))
                {
                    raw_mode = XDBLT_INSERT;
                    xdblt_insert_bits = bk_color;
                }
                else if((mode & 3) == (patOr & 3)
                        && !(fg_color == (uint32_t)~0
                             || (rgb_spec
                                 && fg_color == rgb_spec->white_pixel)))
                {
                    raw_mode = XDBLT_INSERT;
                    xdblt_insert_bits = fg_color;
                }
                else
                {
                    raw_mode = mode & 3;
                }

                update_dirty_p = (*x->blt_func)(rh, raw_mode | MODE_CANON_BITS,
                                                pat_x_rotate_count,
                                                pat_y_rotate_count, x, dst);
            }
            else
            {
                update_dirty_p = do_short_narrow_pattern(rh, mode,
                                                         x->pat_value,
                                                         dst, fg_color,
                                                         bk_color, log2_bpp,
                                                         rgb_spec,
                                                         pat_x_rotate_count);
            }
        }

        xdata_free(xh);
    }

#if defined(VDRIVER_SUPPORTS_REAL_SCREEN_BLITS)
    if(cursor_maybe_changed_p)
        host_set_cursor_visible(cursor_vis_p);
#endif

    return update_dirty_p;
}

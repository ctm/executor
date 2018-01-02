/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "MemoryMgr.h"

#include "rsys/xdata.h"
#include "rsys/xdblt.h"
#include "rsys/tempalloc.h"
#include "rsys/mman.h"

#include "rsys/vdriver.h"

using namespace Executor;

bool Executor::update_xdata_if_needed(xdata_handle_t xh, PixPat *pixpat,
                                      PixMap *dest)
{
    xdata_t *x;

    x = STARH(xh);

    if(x->ctab_seed_x != CTAB_SEED_X(MR(dest->pmTable))
       || (1 << x->log2_bpp) != CW(dest->pixelSize)
       || (x->log2_bpp >= 4 && x->rgb_spec != pixmap_rgb_spec(dest)))
    {
        if(x->raw_pat_bits_mem)
            DisposPtr(x->raw_pat_bits_mem);
        xdata_for_pixpat_with_space(pixpat, dest, xh);
        return true;
    }

    return false;
}

static void
raw_bits_for_pattern(const Pattern pattern, PixMap *target,
                     uint32_t *bits, int *row_bytes)
{
    /* this is a template pattern to be used as the source
     when performing the conversion from the old style
     pattern to the destination */
    static PixMap pattern_pixmap_tmpl = {
        /* baseAddr; to be filled in later */
        CLC_NULL,
        CWC(PIXMAP_DEFAULT_ROWBYTES | 1),
        { CWC(0), CWC(0), CWC(8), CWC(8) },
        /* version */
        CWC(0),
        CWC(0),
        CLC(0),

        CLC(72 << 16),
        CLC(72 << 16),

        CWC(0),
        /* 1bpp */
        CWC(1),
        /* 1 component */
        CWC(1),
        CWC(1),
        CLC(0),
        /* color table; to be filled in later */
        CLC_NULL,

        CLC(0),
    };
    /* this is a template pattern to be used as the dest
     when performing the conversion from the old style
     pattern to the destination */
    static PixMap dst_pixmap_tmpl = {
        /* baseAddr; to be filled in later */
        CLC_NULL,
        CWC(0),
        { CWC(0), CWC(0), CWC(8), CWC(8) },
        /* version */
        CWC(0),
        CWC(0),
        CLC(0),

        CLC(72 << 16),
        CLC(72 << 16),

        CWC(0),
        CWC(0),
        CWC(0),
        CWC(0),
        CLC(0),
        /* color table; to be filled in later */
        CLC_NULL,

        CLC(0),
    };
    CTabPtr conv_table;
    CTabHandle fg_bk_ctab;
    int dst_row_bytes, target_depth;

    pattern_pixmap_tmpl.baseAddr = RM((Ptr)&pattern[0]);
    fg_bk_ctab = validate_fg_bk_ctab();
    pattern_pixmap_tmpl.pmTable = RM(ROMlib_bw_ctab);
    conv_table = (CTabPtr)alloca(CTAB_STORAGE_FOR_SIZE(1));
    conv_table->ctSeed = CTAB_SEED_X(fg_bk_ctab);
    conv_table->ctFlags = CWC(0);
    conv_table->ctSize = CWC(1);

    conv_table->ctTable[0].value = CWC(0);
    conv_table->ctTable[1].value = CWC(~0);

    target_depth = CW(target->pixelSize);
    dst_row_bytes = target_depth; /* old-style Patterns always 8 pixels wide. */
    *row_bytes = dst_row_bytes;
    dst_pixmap_tmpl.rowBytes = (CW(dst_row_bytes)
                                | (target->rowBytes & ROWBYTES_FLAG_BITS_X)
                                | PIXMAP_DEFAULT_ROWBYTES_X);
    pixmap_set_pixel_fields(&dst_pixmap_tmpl, target_depth);
    if(target_depth > 8
       && active_screen_addr_p(target))
        dst_pixmap_tmpl.pixelType = CWC(vdriver_rgb_pixel_type);

    dst_pixmap_tmpl.pmTable = PIXMAP_TABLE_X(GD_PMAP(MR(TheGDevice)));
    dst_pixmap_tmpl.baseAddr = RM((Ptr)bits);

    convert_pixmap(&pattern_pixmap_tmpl, &dst_pixmap_tmpl,
                   &ROMlib_pattern_bounds, conv_table);
}

static void
raw_bits_for_color_pattern(PixPatPtr pixpat, PixMap *target,
                           uint32_t *bits, int *row_bytesp)
{
    PixMapHandle patmap;

    patmap = MR(pixpat->patMap);
    HLockGuard guard(patmap);
    int row_bytes;
    int target_depth;
    PixMap *src;
    PixMap dst;
    Handle data;
    const Rect *bounds;

    src = STARH(patmap);
    bounds = &src->bounds;
    target_depth = CW(target->pixelSize);
    row_bytes = ((RECT_WIDTH(bounds) * target_depth) + 7) / 8;
    *row_bytesp = row_bytes;

    dst = *target;
    dst.bounds = *bounds;
    dst.rowBytes = (CW(row_bytes)
                    | (target->rowBytes & ROWBYTES_FLAG_BITS_X)
                    | PIXMAP_DEFAULT_ROWBYTES_X);
    dst.baseAddr = RM((Ptr)bits);

    data = MR(pixpat->patData);

    HLockGuard guard2(data);
    src->baseAddr = *data;
    convert_pixmap(src, &dst, bounds, NULL);
}

static void
raw_bits_for_rgb_pattern(PixPatPtr pixpat, PixMap *target,
                         uint32_t *bits, int *row_bytes)
{
    uint32_t actual_value;
    RGBColor desired_color;
    int target_depth;

    /* The color desired for this pixpat is stored in the 5th color
   * table entry for the pixpat.  We copy the color in case any of the
   * relevant handles move when we call Color2Index.
   */
    desired_color = CTAB_TABLE(PIXMAP_TABLE(MR(pixpat->patMap)))[4].rgb;

    target_depth = CW(target->pixelSize);

    if(target_depth <= 8)
    {
        actual_value = Color2Index(&desired_color);

        switch(target_depth)
        {
            case 1:
                actual_value = ((int32_t)(actual_value << 31)) >> 31;
                break;
            case 2:
                actual_value = (actual_value & 3) * 0x55555555U;
                break;
            case 4:
                actual_value = (actual_value & 0xF) * 0x11111111U;
                break;
            case 8:
                actual_value = (actual_value & 0xFF) * 0x01010101U;
                break;
            default:
                gui_abort();
        }
    }
    else /* RGB pixel. */
    {
        const rgb_spec_t *rgb_spec;

        rgb_spec = pixmap_rgb_spec(target);
        gui_assert(rgb_spec != NULL);

        actual_value = (*rgb_spec->rgbcolor_to_pixel)(rgb_spec, &desired_color,
                                                      true);
        if(target_depth == 16)
            actual_value |= (actual_value << 16);
    }

    bits[0] = actual_value;
    *row_bytes = 4;
}

static void
raw_bits_for_pixpat(PixPat *pixpat, PixMap *target,
                    uint32_t *bits, int *row_bytes, int *height_override)
{
    switch(CW(pixpat->patType))
    {
        case pixpat_old_style_pattern:
            raw_bits_for_pattern(pixpat->pat1Data, target, bits, row_bytes);
            break;
        case pixpat_color_pattern:
            raw_bits_for_color_pattern(pixpat, target, bits, row_bytes);
            break;
        case pixpat_rgb_pattern:
            raw_bits_for_rgb_pattern(pixpat, target, bits, row_bytes);
            *height_override = 1;
            break;
        default:
            /* Error!  Bogus pixpat type. */
            *row_bytes = 4;
            *bits = 0;
            break;
    }
}

/* Returns true iff the specified pattern can be compressed to be only
 * four bytes wide with no loss of information.
 */
static inline bool
narrow_p(const uint32_t *bits, int row_longs, int height)
{
    const uint32_t *p, *end;

    end = &bits[row_longs * height];

    for(p = bits; p != end; p += row_longs)
    {
        uint32_t v = p[0];
        int i;

        for(i = row_longs - 1; i > 0; i--)
            if(p[i] != v)
                return false;
    }

    return true;
}

/* Returns true iff the specified pattern can be compressed to be only
 * one row tall with no loss of information.
 */
static inline bool
short_p(const uint32_t *bits, int row_longs, int height)
{
    const uint8 *p, *end;
    int row_bytes;

    row_bytes = row_longs * sizeof(uint32_t);
    end = ((const uint8 *)bits) + row_bytes * height;
    for(p = ((const uint8 *)bits) + row_bytes; p != end; p += row_bytes)
    {
        if(memcmp(bits, p, row_bytes))
            return false;
    }

    return true;
}

/* Don't call this directly, use the `xdata_for_pixpat' macro. */
static void
xdata_for_raw_data(PixMap *target, xdata_t *x, uint32_t *raw_bits,
                   int row_bytes, int height)
{
    uint32_t *p, v;
    unsigned row_longs;

    memset(x, 0, sizeof *x);

    x->magic_cookie = XDATA_MAGIC_COOKIE;
    x->log2_bpp = ROMlib_log2[CW(target->pixelSize)];
    x->ctab_seed_x = CTAB_SEED_X(MR(target->pmTable));
    x->rgb_spec = pixmap_rgb_spec(target);

    /* See if it's not a full long wide. */
    if(row_bytes < 4)
    {
        int y;

        /* Tile the raw bits out to four bytes wide.  We do this
       * by converting rows in backwards order, so we don't smash
       * anything before we use it.
       */
        if(row_bytes == 1)
        {
            for(y = height - 1; y >= 0; y--)
                raw_bits[y] = (((const uint8 *)raw_bits)[y]) * 0x01010101U;
        }
        else if(row_bytes == 2)
        {
            for(y = height - 1; y >= 0; y--)
                raw_bits[y] = (((const uint16_t *)raw_bits)[y]) * 0x00010001U;
        }
        else
            abort();

        row_bytes = 4;
    }

    row_longs = row_bytes >> 2;

    /* Check for a short & narrow pattern (i.e. one that can be
   * compressed to only one row tall and 4 bytes wide with no
   * loss of information).  This is a common case because solid
   * colors are so common.
   */
    v = raw_bits[0];
    for(p = &raw_bits[row_longs * height - 1]; p != raw_bits; p--)
        if(*p != v)
            break;

    if(p == raw_bits) /* is every uint32_t in the pattern the same? */
    {
        int shift;

        /* short & narrow. */
        x->pat_value = v;
        x->row_bytes = 4;
        x->log2_row_bytes = 2;
        x->byte_size = 4;
        x->row_bits_minus_1 = 31;
        x->height_minus_1 = 0;

        /* Choose the fastest blitting function we can use. */
        if(x->rgb_spec
           || ((shift = 1 << x->log2_bpp), /* non-rgb ==> shift < 32 */
               (v != ((v >> shift) | (v << (32 - shift))))))
        {
            x->blt_func = xdblt_xdata_short_narrow;
            x->stub_table_for_mode = xdblt_short_narrow_stubs;
        }
        else
        {
            x->blt_func = xdblt_xdata_norgb_norotate;
            if(v == 0)
                x->stub_table_for_mode = xdblt_zeros_stubs;
            else if(v == (uint32_t)~0)
                x->stub_table_for_mode = xdblt_ones_stubs;
            else
                x->stub_table_for_mode = xdblt_short_narrow_stubs;
        }
    }
    else
    {
        if(narrow_p(raw_bits, row_longs, height))
        {
            int i;

            /* narrow. */

            /* Allocate some memory, and compute a long-aligned
	   * pointer for it.  We can't use a Handle here because
	   * the Handle might move in such a way as to become no
	   * longer long-aligned.
	   */
            x->raw_pat_bits_mem = NewPtr(height * sizeof(uint32_t) + 3);
            x->pat_bits = (uint32_t *)(((uintptr_t)x->raw_pat_bits_mem
                                        + 3)
                                       & ~3);
            for(i = 0; i < height; i++)
                x->pat_bits[i] = raw_bits[i * row_longs];
            x->stub_table_for_mode = xdblt_tall_narrow_stubs;

            x->row_bytes = sizeof(uint32_t);
            x->log2_row_bytes = 2;
            x->row_bits_minus_1 = 31;
            x->byte_size = height * sizeof(uint32_t);
        }
        else /* wide */
        {
            int log2_row_bytes, c;

            if(short_p(raw_bits, row_longs, height))
                height = 1;

            for(log2_row_bytes = 0, c = row_bytes; c >>= 1; log2_row_bytes++)
                ;

            x->log2_row_bytes = log2_row_bytes;
            x->row_bytes = row_bytes;
            x->row_bits_minus_1 = (row_bytes * 8) - 1;
            x->byte_size = height << log2_row_bytes;

            /* Allocate long-aligned memory, as described above. */
            x->raw_pat_bits_mem = NewPtr(x->byte_size + 3);
            x->pat_bits = (uint32_t *)(((uintptr_t)x->raw_pat_bits_mem
                                        + 3)
                                       & ~3);
            memcpy(x->pat_bits, raw_bits, x->byte_size);

            if(height == 1)
                x->stub_table_for_mode = xdblt_short_wide_stubs;
            else
                x->stub_table_for_mode = xdblt_tall_wide_stubs;
        }

        x->blt_func = xdblt_xdata_complex;
        x->height_minus_1 = height - 1;
    }
}

xdata_handle_t
Executor::xdata_for_pixpat_with_space(PixPat *pixpat, PixMap *target,
                                      xdata_handle_t xh)
{
    uint32_t *raw_bits;
    int height;
    unsigned width;
    int max_byte_size, row_bytes;
    TEMP_ALLOC_DECL(raw_bits_temp_storage);

    HLockGuard guard(xh);

    xdata_t *x = STARH(xh);

    /* Compute the dimensions of the PixPat, and allocate scratch
	* space for the biggest possible bitmap that might be required.
	*/
    {
        const Rect *bounds = &PIXMAP_BOUNDS(MR(pixpat->patMap));
        height = RECT_HEIGHT(bounds);
        width = RECT_WIDTH(bounds);
    }
    max_byte_size = width * sizeof(uint32_t) * height;
    TEMP_ALLOC_ALLOCATE(raw_bits, raw_bits_temp_storage, max_byte_size);

    /* Compute the raw bits for this pixpat. */
    raw_bits_for_pixpat(pixpat, target, raw_bits, &row_bytes, &height);

    /* Crank out the xdata. */
    xdata_for_raw_data(target, x, raw_bits, row_bytes, height);

    TEMP_ALLOC_FREE(raw_bits_temp_storage);

    return xh;
}

xdata_handle_t
Executor::xdata_for_pattern(const Pattern pattern, PixMap *target)
{
    xdata_handle_t xh;

    xh = (xdata_handle_t)NewHandle(sizeof(xdata_t));

    HLockGuard guard(xh);
    uint32_t raw_bits[8 * 8]; /* Maximum possible resultant pattern size. */
    int row_bytes;
    xdata_t *x = STARH(xh);

    /* Compute the raw bits for this pixpat and crank out the xdata. */
    raw_bits_for_pattern(pattern, target, raw_bits, &row_bytes);
    xdata_for_raw_data(target, x, raw_bits, row_bytes, 8);

    return xh;
}

void Executor::xdata_free(xdata_handle_t xh)
{
    bool xdata_valid_p;

    xdata_valid_p = xh && (GetHandleSize((Handle)xh) == sizeof(xdata_t));

    if(xdata_valid_p)
    {
        Ptr p = HxX(xh, raw_pat_bits_mem);
        if(p)
            DisposPtr(p);
    }
    else
    {
        warning_unexpected("invalid xdata size; appl juked xdata maybe");
    }
    DisposHandle((Handle)xh);
}

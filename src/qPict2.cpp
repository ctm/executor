/* Copyright 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/picture.h"
#include "rsys/cquick.h"
#include "rsys/rgbutil.h"

using namespace Executor;

OSErr Executor::C_DisposePictInfo(PictInfoID pict_info_id)
{
    gui_fatal("unimplemented");
}

OSErr Executor::C_RecordPictInfo(PictInfoID pict_info_id, PicHandle pic_h)
{
    gui_fatal("unimplemented");
}

OSErr Executor::C_RecordPixMapInfo(PictInfoID pict_info_id, PixMapHandle pixmap)
{
    gui_fatal("unimplemented");
}

OSErr Executor::C_RetrievePictInfo(PictInfoID pict_info_id,
                                   PictInfo *pict_info,
                                   int16_t colors_requested)
{
    gui_fatal("unimplemented");
}

OSErr Executor::C_NewPictInfo(GUEST<PictInfoID> *pict_info_id, int16_t verb,
                              int16_t colors_requested,
                              int16_t color_pick_method, int16_t version)
{
    gui_fatal("unimplemented");
#if !defined(LETGCCWAIL)
    return paramErr;
#endif
}

OSErr Executor::C_GetPictInfo(PicHandle pic_h, PictInfo *pict_info,
                              int16_t verb, int16_t color_version,
                              int16_t color_pick_method, int16_t version)
{
    gui_fatal("unimplemented");
#if !defined(LETGCCWAIL)
    return paramErr;
#endif
}

#define INDIRECT_PIXEL_TO_RGB(pixel, r, g, b, color_table) \
    ((void)({                                              \
        const RGBColor *color;                             \
        color = &CTAB_TABLE(color_table)[pixel].rgb;       \
        (r) = CW(color->red);                              \
        (g) = CW(color->green);                            \
        (b) = CW(color->blue);                             \
    }))
#define DIRECT_PIXEL_TO_RGB(bpp, pixel, red_out, green_out, blue_out, \
                            dummy_color_table)                        \
    ((void)({                                                         \
        RGBColor color;                                               \
                                                                      \
        (*rgb_spec->pixel_to_rgbcolor)(rgb_spec, (pixel), &color);    \
        (red_out) = CW(color.red);                                    \
        (green_out) = CW(color.green);                                \
        (blue_out) = CW(color.blue);                                  \
    }))
#define PIXEL_TO_RGB(bpp, pixel, red, green, blue, color_table)                  \
    ((void)((bpp) == 32 || (bpp) == 16                                           \
                ? DIRECT_PIXEL_TO_RGB(bpp, pixel, red, green, blue, color_table) \
                : INDIRECT_PIXEL_TO_RGB(pixel, red, green, blue, color_table)))
#define SHIFT_COUNT(x, bpp) (8 - (bpp) - (bpp) * ((x) & (7 / (bpp))))
#define READ_INDIRECT_PIXEL(b, x, bpp) \
    ((bpp) == 8                        \
         ? b[x]                        \
         : ((b[(x) * (bpp) / 8] >> SHIFT_COUNT((x), (bpp))) & ((1 << (bpp)) - 1)))
#define READ_DIRECT16_PIXEL(b, x, bpp) \
    ((uint16_t *)b)[x]
#define READ_DIRECT32_PIXEL(b, x, bpp) \
    ((uint32_t *)b)[x]
#define RECORD_COLORS(read, record, bpp)                               \
    ({                                                                 \
        int x, y;                                                      \
                                                                       \
        for(y = 0; y < height; y++)                                    \
        {                                                              \
            for(x = 0; x < width; x++)                                 \
            {                                                          \
                uint32_t r, g, b;                                      \
                uint32_t pixel;                                        \
                                                                       \
                pixel = read(row_base, x, bpp);                        \
                                                                       \
                PIXEL_TO_RGB(bpp, pixel, r, g, b, pixmap_color_table); \
                                                                       \
                record(r, g, b);                                       \
            }                                                          \
            row_base += row_bytes;                                     \
        }                                                              \
    })

#define systemMethod 0
#define popularMethod 1
#define medianMethod 2

#define returnColorTable 1
#define returnPalette 2
#define suppressBlackAndWhite 16

struct link
{
    int bank_index;
    int count;
    struct link *next, *prev;
};

OSErr Executor::C_GetPixMapInfo(PixMapHandle pixmap, PictInfo *pict_info,
                                int16_t verb, int16_t colors_requested,
                                int16_t color_pick_method, int16_t version)
{
    CTabHandle pixmap_color_table;
    uint8_t *row_base;
    int row_bytes, bpp;
    const rgb_spec_t *rgb_spec;
    uint16_t *bank;
    int32_t unique_colors;
    int height, width;
    struct link *head, *tail;
    OSErr retval;

    retval = noErr;
    head = tail = NULL;

    /* suck out the relevent pixmap bits */
    row_bytes = PIXMAP_ROWBYTES(pixmap);
    row_base = (uint8_t *)PIXMAP_BASEADDR(pixmap);

    bpp = PIXMAP_PIXEL_SIZE(pixmap);

    pixmap_color_table = PIXMAP_TABLE(pixmap);

    rgb_spec = pixmap_rgb_spec(STARH(pixmap));

    width = RECT_WIDTH(&PIXMAP_BOUNDS(pixmap));
    height = RECT_HEIGHT(&PIXMAP_BOUNDS(pixmap));

    if(color_pick_method != systemMethod
       /* currently system method defaults to popular method */
       && color_pick_method != popularMethod)
    {
        warning_unimplemented("unknown pick method `%d', using popularMethod",
                              color_pick_method);
    }
    color_pick_method = popularMethod;

    bank = (uint16_t *)NewPtr(32768 * sizeof *bank);
    if(MemError() != noErr)
        return MemError();
    memset(bank, '\000', 32768 * sizeof *bank);

    unique_colors = 0;

#define RECORD_555(red, green, blue)                                               \
    ({                                                                             \
        int bank_index;                                                            \
        const uint16_t mask = 0xF100;                                              \
        uint16_t count;                                                            \
                                                                                   \
        bank_index                                                                 \
            = ((red & mask) >> 1) | ((green & mask) >> 6) | ((blue & mask) >> 11); \
                                                                                   \
        count = /* ### CW*/ (bank[bank_index]);                                    \
        if(!count)                                                                 \
            unique_colors++;                                                       \
        bank[bank_index] = /* ### CW*/ (count + 1);                                \
    })

    switch(bpp)
    {
        case 1:
            RECORD_COLORS(READ_INDIRECT_PIXEL, RECORD_555, 1);
            break;
        case 2:
            RECORD_COLORS(READ_INDIRECT_PIXEL, RECORD_555, 2);
            break;
        case 4:
            RECORD_COLORS(READ_INDIRECT_PIXEL, RECORD_555, 4);
            break;
        case 8:
            RECORD_COLORS(READ_INDIRECT_PIXEL, RECORD_555, 8);
            break;
        case 16:
            RECORD_COLORS(READ_DIRECT16_PIXEL, RECORD_555, 16);
            break;
        case 32:
            RECORD_COLORS(READ_DIRECT32_PIXEL, RECORD_555, 32);
            break;
    }

    {
        int i;

        for(i = 0; i < colors_requested; i++)
        {
            struct link *t;

            t = (struct link *)NewPtr(sizeof *t);
            if(MemError() != noErr)
            {
                retval = MemError();
                goto cleanup_and_return;
            }
            memset(t, '\000', sizeof *t);

            if(head)
                head->prev = t;
            t->next = head;
            head = t;

            if(tail == NULL)
                tail = t;
        }

        for(i = 0; i < 32768; i++)
        {
            if((verb & suppressBlackAndWhite)
               && (!i
                   || i == 32767))
                continue;

            if(tail->count < bank[i])
            {
                struct link *t;

                t = tail;

                t->bank_index = i;
                t->count = bank[i];

                if(t->count > t->prev->count)
                {
                    tail = t->prev;
                    tail->next = NULL;
                    t->next = t->prev = NULL;

                    if(t->count > head->count)
                    {
                        head->prev = t;
                        t->next = head;
                        head = t;
                    }
                    else
                    {
                        struct link *current;

                        /* insert sorted */
                        for(current = head; current; current = current->next)
                        {
                            if(t->count <= current->count
                               && current->next
                               && current->next->count < t->count)
                            {
                                /* insert */
                                t->next = current->next;
                                t->prev = current;

                                t->next->prev = t;
                                t->prev->next = t;
                                break;
                            }
                        }
                        if(current == NULL)
                            gui_abort();
                    }
                }
            }
        }

        {
            CTabHandle color_table;
            ColorSpec *table;

            color_table
                = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(colors_requested));

            CTAB_SEED_X(color_table) = CL(GetCTSeed());
            CTAB_FLAGS_X(color_table) = CW(0);
            CTAB_SIZE_X(color_table) = CW(colors_requested);

            table = CTAB_TABLE(color_table);

            {
                struct link *t;

                for(i = 0, t = head; i < colors_requested && t; i++, t = t->next)
                {
                    table[i].value = CW(i);

#define TILE(x) CW(((uint32_t)(x & 0x1F) * 0x8421UL) >> 4)

                    table[i].rgb.red = TILE(t->bank_index >> 10);
                    table[i].rgb.green = TILE(t->bank_index >> 5);
                    table[i].rgb.blue = TILE(t->bank_index >> 0);
                }
                gui_assert(t == NULL && i == colors_requested);
            }

            /* `head' now points to a list, in sorted order, of the most
	 popular colors in the pixmap */
            memset(pict_info, '\000', sizeof *pict_info);

            pict_info->uniqueColors = CL(unique_colors);

            if(verb & returnPalette)
            {
                PaletteHandle palette;

                palette = NewPalette(colors_requested, color_table,
                                     /* #### verify correct default values */
                                     pmTolerant, 0);
                pict_info->thePalette = RM(palette);
            }

            if(verb & returnColorTable)
                pict_info->theColorTable = RM(color_table);
            else
                DisposHandle((Handle)color_table);
        }

    cleanup_and_return:
    {
        struct link *t, *next;

        DisposPtr((Ptr)bank);

        for(t = head, next = t->next;
            t;
            t = next, next = t ? t->next : NULL)
        {
            if(t)
                DisposPtr((Ptr)t);
        }
        return retval;
    }
    }
}

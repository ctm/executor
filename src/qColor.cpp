/* Copyright 1986 - 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/resource.h"

#include "rsys/host.h"
#include "rsys/qcolor.h"

using namespace Executor;

/* color quickdraw global stuff */

ColorSpec Executor::ROMlib_white_cspec = {
    CWC(0), { CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF) }
};

ColorSpec Executor::ROMlib_black_cspec = {
    CWC(1), { CWC(0x0), CWC(0x0), CWC(0x0) }
};

ColorSpec Executor::ROMlib_gray_cspec = {
    CWC(0), { CWC((unsigned short)0x8000), CWC((unsigned short)0x8000), CWC((unsigned short)0x8000) }
};

Rect Executor::ROMlib_pattern_bounds = {
    CWC(0), CWC(0), CWC(8), CWC(8),
};

/* end global color quickdraw stuff */

/* FIXME: replace this with a correct references
   to the low global QDColors */
struct qd_color_elt Executor::ROMlib_QDColors[] = {
    { { CWC(0x0000), CWC(0x0000), CWC(0x0000) }, blackColor },
    { { CWC((unsigned short)0xFC00), CWC((unsigned short)0xF37D), CWC(0x052F) }, yellowColor },
    { { CWC((unsigned short)0xF2D7), CWC((unsigned short)0x0856), CWC((unsigned short)0x84EC) }, magentaColor },
    { { CWC((unsigned short)0xDD6B), CWC((unsigned short)0x08C2), CWC((unsigned short)0x06A2) }, redColor },
    { { CWC((unsigned short)0x0241), CWC((unsigned short)0xAb54), CWC((unsigned short)0xEAFF) }, cyanColor },
    { { CWC(0x0000), CWC((unsigned short)0x64AF), CWC((unsigned short)0x11B0) }, greenColor },
    { { CWC(0x0000), CWC(0x0000), CWC(0xD400) }, blueColor },
    { { CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF) }, whiteColor },
};

RGBColor *
Executor::ROMlib_qd_color_to_rgb(LONGINT qd_color)
{
    int i;

    for(i = 0; i < 8; i++)
        if(qd_color == ROMlib_QDColors[i].value)
            return &ROMlib_QDColors[i].rgb;

    return &ROMlib_black_rgb_color;
}

/* NOTE: previously, these functions did nothing if the color didn't
   match a predefined, and the current port was a cgrafport; now it
   sets the color black */

/* NOTE2:  if the_port == 0 we blow off the call.  Realmz 2.5 suggests
   that we should do this, although there's still the lingering question
   of whether or not a call to SetPort (0) should be blown off */

P1(PUBLIC pascal trap, void, ForeColor, LONGINT, c)
{
    GrafPtr the_port;

    the_port = thePort;
    if(the_port)
    {
        if(CGrafPort_p(the_port))
            RGBForeColor(ROMlib_qd_color_to_rgb(c));
        else
            PORT_FG_COLOR_X(the_port) = CL(c);
    }
}

P1(PUBLIC pascal trap, void, BackColor, LONGINT, c)
{
    GrafPtr the_port;

    the_port = thePort;
    if(the_port)
    {
        if(CGrafPort_p(the_port))
            RGBBackColor(ROMlib_qd_color_to_rgb(c));
        else
            PORT_BK_COLOR_X(the_port) = CL(c);
    }
}

P1(PUBLIC pascal trap, void, ColorBit, INTEGER, b)
{
    PORT_COLR_BIT_X(thePort) = CW(b);
}

typedef CTabHandle clut_res_handle;

ColorSpec *Executor::default_ctab_colors[] = {
    ctab_1bpp_values,
    ctab_2bpp_values,
    ctab_4bpp_values,
    ctab_8bpp_values,
};

P1(PUBLIC pascal trap, CTabHandle, GetCTable,
   INTEGER, ctab_res_id)
{
    CTabHandle ctab;
    clut_res_handle clut;
    int32_t ctab_id;

    ctab_id = ctab_res_id;
    switch(ctab_id)
    {
        /* grayscale */
        case 33:
        case 34:
        case 36:
        case 40:
        {
            /* number of elements in the color table minus 1 */
            int ctab_size;
            int i;
            uint32_t stride, c;
            ColorSpec *table;

            ctab_size = (1 << (ctab_id - 32)) - 1;

            /* #### should we first check for a 'clut' resource of
	   `ctab_id' before creating a grayscale clut? */

            /* #### are we to make a new copy every time? */
            ctab = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(ctab_size));

            /* if the color table is b/w, set the seed to be the b/w clut
	   seed */
            /* #### ctab_id or a new seed? */
            CTAB_SEED_X(ctab) = CL(ctab_id == 33 ? 1 : ctab_id);
            CTAB_SIZE_X(ctab) = CW(ctab_size);
            CTAB_FLAGS_X(ctab) = CTAB_GDEVICE_BIT_X;

            table = CTAB_TABLE(ctab);

            stride = 0xFFFF0000UL / ctab_size;
            for(c = 0xFFFF0000UL, i = 0; i < ctab_size; c -= stride, i++)
            {
                table[i].value = CW(i);
                table[i].rgb.red = table[i].rgb.green = table[i].rgb.blue
                    = CW((c + 0x8000) >> 16);
            }

            /* Make sure the last entry is _exactly_ black. */
            table[ctab_size].value = CW(ctab_size);
            table[ctab_size].rgb.red = table[ctab_size].rgb.green
                = table[ctab_size].rgb.blue = CWC(0);

            return ctab;
        }

        /* just like 1-8 */
        case 65:
        case 66:
        case 68:
        case 72:
            ctab_id -= 64;
        /* fall through */
        default:
        {
            clut = (clut_res_handle)ROMlib_getrestid(TICK("clut"), ctab_res_id);

            if(clut)
            {
                int ctab_handle_size;

                ctab_handle_size
                    = sizeof(ColorTable) + (sizeof(ColorSpec) * CTAB_SIZE(clut));
                ctab = (CTabHandle)NewHandle(ctab_handle_size);

                BlockMoveData((Ptr)STARH(clut), (Ptr)STARH(ctab),
                              ctab_handle_size);

                /* #### ctab_id or a new seed? */
                CTAB_SEED_X(ctab) = CL(ctab_id);
            }
            else if(ctab_id >= 0 && ctab_id <= 8)
            {
                /* if there is no clut, use the ``rom'' default values */
                int ctab_size;
                ColorSpec *ctab_table;

                ctab_size = (1 << ctab_id) - 1;
                ctab = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(ctab_size));

                CTAB_SIZE_X(ctab) = CW(ctab_size);
                CTAB_FLAGS_X(ctab) = CTAB_GDEVICE_BIT_X;
                CTAB_SEED_X(ctab) = CL(ctab_id);

                ctab_table = CTAB_TABLE(ctab);
                memcpy(ctab_table, default_ctab_colors[ROMlib_log2[ctab_id]],
                       (1 << ctab_id) * sizeof *ctab_table);
            }
            else
                ctab = NULL;
            return ctab;
        }
    }
}

P1(PUBLIC pascal trap, void, DisposCTable,
   CTabHandle, ctab)
{
    DisposHandle((Handle)ctab);
}

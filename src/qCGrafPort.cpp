/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "ResourceMgr.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"

#include "rsys/xdata.h"
#include "rsys/evil.h"

using namespace Executor;

P1(PUBLIC pascal trap, void, OpenCPort,
   CGrafPtr, port)
{
    PixPatHandle temp_pixpat;

    /* set up port version before using any other macros */
    port->portVersion = CWC((3 << 14) | /* color quickdraw version */ 0);

    /* allocate storage for new CGrafPtr members, including portPixMap,
     pnPixPat, fillPixPat, bkPixPat, and grafVar */
    CPORT_PIXMAP_X(port) = RM(NewPixMap());

    /* Free up the empty color table, since we're not going to use it. */
    DisposHandle((Handle)PIXMAP_TABLE(CPORT_PIXMAP(port)));

    temp_pixpat = NewPixPat();
    PIXPAT_TYPE_X(temp_pixpat) = CWC(pixpat_type_orig);
    CPORT_PEN_PIXPAT_X(port) = RM(temp_pixpat);

    temp_pixpat = NewPixPat();
    PIXPAT_TYPE_X(temp_pixpat) = CWC(pixpat_type_orig);
    CPORT_FILL_PIXPAT_X(port) = RM(temp_pixpat);

    temp_pixpat = NewPixPat();
    PIXPAT_TYPE_X(temp_pixpat) = CWC(pixpat_type_orig);
    CPORT_BK_PIXPAT_X(port) = RM(temp_pixpat);

    CPORT_GRAFVARS_X(port) = RM(NewHandleClear(sizeof(GrafVars)));

    /* allocate storage for members also present in GrafPort */
    PORT_VIS_REGION_X(port) = RM(NewRgn());
    PORT_CLIP_REGION_X(port) = RM(NewRgn());

    InitCPort(port);
}

P1(PUBLIC pascal trap, void, CloseCPort, CGrafPtr, port)
{
    ClosePort((GrafPtr)port);
}

/* FIXME:
   do these belong here */

P1(PUBLIC pascal trap, void, InitCPort,
   CGrafPtr, p)
{
    GDHandle gd;
    RgnHandle rh;

    if(!p || !CGrafPort_p(p))
        return;

    /* set the port early so we can call convenience functions
     that operate on thePort */
    SetPort((GrafPtr)p);

    CPORT_VERSION_X(p) = CPORT_FLAG_BITS_X;
    // | CWC (/* color quickdraw version */ 0));

    gd = MR(TheGDevice);
    *STARH(CPORT_PIXMAP(p)) = *STARH(GD_PMAP(gd));

    PORT_DEVICE_X(p) = CWC(0);
    PORT_RECT(p) = GD_RECT(gd);
    PORT_PEN_LOC(p).h = PORT_PEN_LOC(p).v = CWC(0);
    PORT_PEN_SIZE(p).h = PORT_PEN_SIZE(p).v = CWC(1);
    PORT_PEN_MODE_X(p) = CWC(patCopy);
    PORT_PEN_VIS_X(p) = CWC(0);
    PORT_TX_FONT_X(p) = CWC(0);
    PORT_TX_FACE_X(p) = 0;
    *((char *)&p->txFace + 1) = 0; /* Excel & tests show we need to do this. */
    PORT_TX_MODE_X(p) = CWC(srcOr);
    PORT_TX_SIZE_X(p) = CWC(0);
    PORT_SP_EXTRA_X(p) = CLC(0);
    RGBForeColor(&ROMlib_black_rgb_color);
    RGBBackColor(&ROMlib_white_rgb_color);
    PORT_COLR_BIT_X(p) = CWC(0);
    PORT_PAT_STRETCH_X(p) = CWC(0);
    PORT_PIC_SAVE_X(p) = CLC_NULL;
    PORT_REGION_SAVE_X(p) = CLC_NULL;
    PORT_POLY_SAVE_X(p) = CLC_NULL;
    PORT_GRAF_PROCS_X(p) = CLC_NULL;

    PenPat(black);
    BackPat(white);

    /* hack */
    ROMlib_fill_pat(black);

    /* initialize default values for newly allocated
     CGrafPtr */
    PORT_DEVICE_X(p) = CWC(0);

    /* rgbOpColor of GrafVars field is set to black,
     rgbHiliteColor is set to the default value (where does this come from)
     and all other fields are zero'd */

    /* A test case shows that grafVars is allocated only if it was NULL. */
    if(CPORT_GRAFVARS_X(p) == CLC_NULL)
        CPORT_GRAFVARS_X(p) = RM(NewHandleClear(sizeof(GrafVars)));

    /* #warning "p->grafVars not initialized" */

    CPORT_OP_COLOR(p) = ROMlib_black_rgb_color;
    CPORT_HILITE_COLOR(p) = HiliteRGB;

    CPORT_CH_EXTRA_X(p) = CWC(0);
    /* represents the low word of a Fixed number
     whose value is 0.5 */
    CPORT_PENLOC_HFRAC_X(p) = CWC(0x8000);

    rh = PORT_VIS_REGION(p);
    SetEmptyRgn(rh);
    RectRgn(rh, &GD_RECT(gd));

    rh = PORT_CLIP_REGION(p);
    SetEmptyRgn(rh);
    SetRectRgn(rh, -32767, -32767, 32767, 32767);
}

P1(PUBLIC pascal trap, void, SetPortPix, PixMapHandle, pixmap)
{
    CPORT_PIXMAP_X(theCPort) = RM(pixmap);
}

static const LONGINT high_bits_to_colors[2][2][2] = {
    {
        {
            blackColor, blueColor,
        },
        { greenColor, cyanColor },
    },
    {
        {
            redColor, magentaColor,
        },
        { yellowColor, whiteColor },
    },
};

P1(PUBLIC pascal trap, void, RGBForeColor,
   RGBColor *, color)
{
    if(CGrafPort_p(thePort))
    {
        CPORT_RGB_FG_COLOR(theCPort) = *color;

        /* pick the best color and store it into `theCPort->fgColor' */
        PORT_FG_COLOR_X(theCPort) = CL(Color2Index(color));
    }
    else
    {
        /* GrafPort */
        int basic_qd_color;

        basic_qd_color
            = high_bits_to_colors
                [CW(color->red) >> 15]
                [CW(color->green) >> 15]
                [CW(color->blue) >> 15];

        ForeColor(basic_qd_color);
    }
}

P1(PUBLIC pascal trap, void, RGBBackColor,
   RGBColor *, color)
{

#if defined(EVIL_ILLUSTRATOR_7_HACK)
    if(ROMlib_evil_illustrator_7_hack)
    {
        color = (RGBColor *)alloca(sizeof(RGBColor));
        color->red = CWC(65535);
        color->green = CWC(65535);
        color->blue = CWC(65535);
    }
#endif

    if(CGrafPort_p(thePort))
    {
        CPORT_RGB_BK_COLOR(theCPort) = *color;

        /* pick the best color and store it into `theCPort->bkColor' */
        PORT_BK_COLOR_X(theCPort) = CL(Color2Index(color));
    }
    else
    {
        /* GrafPort */
        int basic_qd_color;

        basic_qd_color
            = high_bits_to_colors
                [CW(color->red) >> 15]
                [CW(color->green) >> 15]
                [CW(color->blue) >> 15];

        BackColor(basic_qd_color);
    }
}

P1(PUBLIC pascal trap, void, GetForeColor,
   RGBColor *, color)
{
    if(CGrafPort_p(thePort))
        *color = CPORT_RGB_FG_COLOR(theCPort);
    else
        *color = *(ROMlib_qd_color_to_rgb(PORT_FG_COLOR(thePort)));
}

P1(PUBLIC pascal trap, void, GetBackColor,
   RGBColor *, color)
{
    if(CGrafPort_p(thePort))
        *color = CPORT_RGB_BK_COLOR(theCPort);
    else
        *color = *(ROMlib_qd_color_to_rgb(PORT_BK_COLOR(thePort)));
}

P1(PUBLIC pascal trap, void, PenPixPat,
   PixPatHandle, new_pen)
{
    if(CGrafPort_p(thePort))
    {
        PixPatHandle old_pen;

        old_pen = CPORT_PEN_PIXPAT(theCPort);
        if(old_pen == new_pen)
            return;

        if(old_pen && (PIXPAT_TYPE_X(old_pen) == CWC(pixpat_type_orig)))
            DisposPixPat(old_pen);

        CPORT_PEN_PIXPAT_X(theCPort) = RM(new_pen);
    }
    else
        PATASSIGN(PORT_PEN_PAT(thePort), PIXPAT_1DATA(new_pen));
}

P1(PUBLIC pascal trap, void, BackPixPat,
   PixPatHandle, new_bk)
{
    if(CGrafPort_p(thePort))
    {
        PixPatHandle old_bk;

        old_bk = CPORT_BK_PIXPAT(theCPort);
        if(old_bk == new_bk)
            return;

        if(old_bk && PIXPAT_TYPE_X(old_bk) == CWC(pixpat_type_orig))
            DisposPixPat(old_bk);

        CPORT_BK_PIXPAT_X(theCPort) = RM(new_bk);
    }
    else
        PATASSIGN(PORT_BK_PAT(thePort), PIXPAT_1DATA(new_bk));
}

void Executor::ROMlib_fill_pixpat(PixPatHandle new_fill)
{
    if(CGrafPort_p(thePort))
    {
        PixPatHandle old_fill;

        old_fill = CPORT_FILL_PIXPAT(theCPort);
        if(old_fill == new_fill)
            return;

#if 0
      if (PIXPAT_TYPE_X (old_fill) == CWC (pixpat_type_orig))
	DisposPixPat (old_fill);
#endif

        CPORT_FILL_PIXPAT_X(theCPort) = RM(new_fill);
    }
    else
        PATASSIGN(PORT_BK_PAT(thePort), PIXPAT_1DATA(new_fill));
}

/* where is FillPixPat */

P1(PUBLIC pascal trap, void, OpColor,
   RGBColor *, color)
{
    if(!CGrafPort_p(thePort))
        return;

    HxX(CPORT_GRAFVARS(theCPort), rgbOpColor) = *color;
}

P1(PUBLIC pascal trap, void, HiliteColor,
   RGBColor *, color)
{
    if(!CGrafPort_p(thePort))
        return;

    HxX(CPORT_GRAFVARS(theCPort), rgbHiliteColor) = *color;
}

/* PixMap operations */

P0(PUBLIC pascal trap, PixMapHandle, NewPixMap)
{
    PixMapHandle pixmap;

    pixmap = (PixMapHandle)NewHandle(sizeof(PixMap));
    if(pixmap == NULL)
    {
#if defined(FLAG_ALLOCATION_FAILURES)
        gui_fatal("allocation failure");
#endif
        return NULL;
    }

    HLock((Handle)pixmap); /* so we can use accessor macros even when we're
  				calling routines that move memory */

    /* All PixMap fields except the ColorTable come from TheGDevice.
   * The ColorTable is allocated but not initialized.  (IMV-70)
   */
    if(TheGDevice)
    {
        *STARH(pixmap) = *STARH(GD_PMAP(MR(TheGDevice)));
    }
    else
    {
        /* If TheGDevice is NULL, we fill in some useful default values.
       * This is a hack to make Executor bootstrap properly.
       */
        memset(STARH(pixmap), 0, sizeof(PixMap));
        HxX(pixmap, rowBytes) = PIXMAP_DEFAULT_ROWBYTES_X;
        PIXMAP_HRES_X(pixmap) = PIXMAP_VRES_X(pixmap) = CLC(72 << 16);
        PIXMAP_PIXEL_TYPE_X(pixmap) = CWC(chunky_pixel_type);
        PIXMAP_CMP_COUNT_X(pixmap) = CWC(1);
    }

    /* The ColorTable is set to an empty ColorTable (IMV-70). */
    PIXMAP_TABLE_X(pixmap)
        = RM((CTabHandle)NewHandleClear(sizeof(ColorTable)));

    HUnlock((Handle)pixmap);
    return pixmap;
}

P1(PUBLIC pascal trap, void, DisposPixMap,
   PixMapHandle, pixmap)
{
    if(pixmap)
    {
        DisposCTable(PIXMAP_TABLE(pixmap));
        DisposHandle((Handle)pixmap);
    }
}

P2(PUBLIC pascal trap, void, CopyPixMap,
   PixMapHandle, src,
   PixMapHandle, dst)
{
    CTabHandle dst_ctab;

    /* save away the destination ctab; it is going to get clobbered. */
    dst_ctab = PIXMAP_TABLE(dst);

    /* #warning "determine actual CopyPixMap behavior" */
    *(STARH(dst)) = *(STARH(src));

    PIXMAP_TABLE_X(dst) = RM(dst_ctab);
    ROMlib_copy_ctab(PIXMAP_TABLE(src), dst_ctab);
}

/* PixPat operations */

P0(PUBLIC pascal trap, PixPatHandle, NewPixPat)
{
    PixPatHandle pixpat;
    Handle xdata;

    pixpat = (PixPatHandle)NewHandle(sizeof(PixPat));

    xdata = NewHandleClear(sizeof(xdata_t));

    HASSIGN_6(pixpat,
              patMap, RM(NewPixMap()),
              patData, RM(NewHandle(0)),
              patType, CWC(pixpat_type_color),
              patXMap, CLC_NULL,
              patXData, RM(xdata),
              patXValid, CWC(-1));

    return pixpat;
}

typedef struct pixpat_res *pixpat_res_ptr;

typedef GUEST<pixpat_res_ptr> *pixpat_res_handle;

P1(PUBLIC pascal trap, PixPatHandle, GetPixPat, INTEGER, pixpat_id)
{
    pixpat_res_handle pixpat_res;
    PixPatHandle pixpat;
    PixMapHandle patmap;
    int pixpat_data_offset, pixpat_data_size;
    CTabPtr ctab_ptr;
    int ctab_size;
    Handle xdata;

    pixpat_res = (pixpat_res_handle)GetResource(TICK("ppat"), pixpat_id);
    if(pixpat_res == NULL)
        return (PixPatHandle)NULL;
    if(*pixpat_res == NULL)
        LoadResource((Handle)pixpat_res);

    pixpat = (PixPatHandle)NewHandle(sizeof(PixPat));
    patmap = (PixMapHandle)NewHandle(sizeof(PixMap));
    *STARH(pixpat) = HxX(pixpat_res, pixpat);
    *STARH(patmap) = HxX(pixpat_res, patmap);

    {
        int pixpat_type;

        pixpat_type = PIXPAT_TYPE(pixpat);

        /* ### are `rgb' (and `old_style'?) valid pattern types for pixpat
       resources */
        if(pixpat_type != pixpat_old_style_pattern
           && pixpat_type != pixpat_color_pattern
           && pixpat_type != pixpat_rgb_pattern)
        {
            warning_unexpected("unknown pixpat type `%d'",
                               PIXPAT_TYPE(pixpat));
            PIXPAT_TYPE_X(pixpat) = CWC(pixpat_color_pattern);
        }
    }

    gui_assert((int)(PIXPAT_MAP_X(pixpat).raw()) == CLC(sizeof(PixPat)).raw());

    PIXPAT_MAP_X(pixpat) = RM(patmap);

    PIXPAT_XVALID_X(pixpat) = CWC(-1);

    xdata = NewHandle(sizeof(xdata_t));
    memset(STARH(xdata), 0, sizeof(xdata_t));
    PIXPAT_XDATA_X(pixpat) = RM(xdata);
    PIXPAT_XMAP_X(pixpat) = CLC_NULL;

    pixpat_data_offset = PIXPAT_DATA_AS_OFFSET(pixpat);
    pixpat_data_size = (PIXMAP_TABLE_AS_OFFSET(patmap)
                        - pixpat_data_offset);

    HLock((Handle)pixpat);
    PIXPAT_DATA_X(pixpat) = RM(NewHandle(pixpat_data_size));
    HUnlock((Handle)pixpat);

    BlockMoveData((Ptr)((char *)STARH(pixpat_res) + pixpat_data_offset),
                  STARH(PIXPAT_DATA(pixpat)),
                  pixpat_data_size);

    HLockGuard guard(pixpat_res);
    /* ctab_ptr is a pointer into the pixpat_res_handle;
	  make sure no allocations are done while it is in use */
    ctab_ptr = (CTabPtr)((char *)STARH(pixpat_res)
                         + (int)PIXMAP_TABLE_AS_OFFSET(patmap));
    ctab_size = (sizeof(ColorTable)
                 + (sizeof(ColorSpec) * CW(ctab_ptr->ctSize)));

    /* SetHandleSize ((Handle) PIXMAP_TABLE (patmap), ctab_size); */

    HLock((Handle)patmap);
    PIXMAP_TABLE_X(patmap) = RM((CTabHandle)NewHandle(ctab_size));
    HUnlock((Handle)patmap);

    BlockMoveData((Ptr)ctab_ptr,
                  (Ptr)STARH(PIXMAP_TABLE(patmap)),
                  ctab_size);

    /* ctab_ptr->ctSeed = CL (GetCTSeed ()); */
    CTAB_SEED_X(PIXMAP_TABLE(patmap)) = CL(GetCTSeed());

#if 0
  gui_assert (GetHandleSize (pixpat_res)
	      == (sizeof (struct pixpat_res)
		  + pixpat_data_size
		  + ctab_size));
#endif /* 0 */

    return pixpat;
}

P1(PUBLIC pascal trap, void, DisposPixPat,
   PixPatHandle, pixpat_h)
{
    if(pixpat_h)
    {
        /* ##### determine which of these checks are necessary, and which
	 should be asserts that the handles are non-NULL */
        if(PIXPAT_MAP_X(pixpat_h))
            DisposPixMap(PIXPAT_MAP(pixpat_h));
        if(PIXPAT_DATA_X(pixpat_h))
            DisposHandle(PIXPAT_DATA(pixpat_h));
        /* We ignore the xmap field, so no need to free it. */
        if(PIXPAT_XDATA_X(pixpat_h))
            xdata_free((xdata_handle_t)PIXPAT_XDATA(pixpat_h));

        DisposHandle((Handle)pixpat_h);
    }
}

P2(PUBLIC pascal trap, void, CopyPixPat,
   PixPatHandle, src,
   PixPatHandle, dst)
{
    int data_size;

    PIXPAT_TYPE_X(dst) = PIXPAT_TYPE_X(src);
    CopyPixMap(PIXPAT_MAP(src), PIXPAT_MAP(dst));

    data_size = GetHandleSize(PIXPAT_DATA(src));
    SetHandleSize(PIXPAT_DATA(dst), data_size);
    memcpy(STARH(PIXPAT_DATA(dst)), STARH(PIXPAT_DATA(src)), data_size);
    PIXPAT_XVALID_X(dst) = CWC(-1);
    PATASSIGN(PIXPAT_1DATA(dst), PIXPAT_1DATA(src));
}

P2(PUBLIC pascal trap, void, MakeRGBPat,
   PixPatHandle, pixpat,
   RGBColor *, color)
{
    PixMapHandle patmap;

    PIXPAT_TYPE_X(pixpat) = CWC(pixpat_rgb_pattern);
    PIXPAT_XVALID_X(pixpat) = CWC(-1);

    /* ##### resolve the meaning of the actual PixPat fields */

    patmap = PIXPAT_MAP(pixpat);
    PIXMAP_SET_ROWBYTES_X(patmap, CWC(2));
    PIXMAP_BOUNDS(patmap) = ROMlib_pattern_bounds;
    /* create a table with 5 entries, the last of which
     will be the desired rgb color */
    SetHandleSize((Handle)PIXMAP_TABLE(patmap),
                  (Size)(sizeof(ColorTable) + (4 * sizeof(ColorSpec))));

    CTAB_SEED_X(PIXMAP_TABLE(patmap)) = CL(GetCTSeed());
    CTAB_SIZE_X(PIXMAP_TABLE(patmap)) = CWC(5);
    CTAB_TABLE(PIXMAP_TABLE(patmap))
    [4].rgb
        = *color;
}

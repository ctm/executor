/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "WindowMgr.h"
#include "MenuMgr.h"
#include "SysErr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/host.h"
#include "rsys/vdriver.h"
#include "rsys/blockinterrupts.h"
#include "rsys/flags.h"
#include "rsys/autorefresh.h"
#include "rsys/redrawscreen.h"
#include "rsys/executor.h"
#include "rsys/functions.impl.h"

using namespace Executor;

/*
 * determined experimentally -- this fix is needed for Energy Scheming because
 * they put a bogus test for mode >= 8 in their code to detect color.  NOTE:
 * the value that will get stored is most definitely not 8 or anything near
 * 8, since 8bpp will be translated to 0x83.
 */

static LONGINT
mode_from_bpp(int bpp)
{
    LONGINT retval;
    int log2;

    if(bpp >= 1 && bpp <= 32)
        log2 = ROMlib_log2[bpp];
    else
        log2 = -1;

    if(log2 != -1)
        retval = 0x80 | ROMlib_log2[bpp];
    else
    {
        warning_unexpected("bpp = %d", bpp);
        retval = 0x86; /* ??? */
    }
    return retval;
}

void Executor::gd_allocate_main_device(void)
{
    GDHandle graphics_device;

    if(vdriver_fbuf == NULL)
        gui_fatal("vdriver not initialized, unable to allocate `LM(MainDevice)'");

    TheZoneGuard guard(LM(SysZone));

    PixMapHandle gd_pixmap;
    Rect *gd_rect;

    graphics_device = NewGDevice(/* no driver */ 0,
                                 mode_from_bpp(vdriver_bpp));

    /* we are the main device, since there are currently no others */
    LM(TheGDevice) = LM(MainDevice) = RM(graphics_device);

    /* set gd flags reflective of the main device */
    GD_FLAGS_X(graphics_device).raw_or(CWC((1 << mainScreen) | (1 << screenDevice) | (1 << screenActive)
                                           /* PacMan Deluxe avoids
						 GDevices with noDriver
						 set.  Looking around
						 Apple's site shows that
						 people set this bit when
						 they're creating offscreen
						 gDevices.  It's not clear
						 whether or not we should
						 be setting this bit.
					    | (1 << noDriver) */));

    gd_set_bpp(graphics_device, !vdriver_grayscale_p, vdriver_fixed_clut_p,
               vdriver_bpp);

    gd_pixmap = GD_PMAP(graphics_device);
    PIXMAP_SET_ROWBYTES_X(gd_pixmap, CW(vdriver_row_bytes));
    PIXMAP_BASEADDR_X(gd_pixmap) = RM((Ptr)vdriver_fbuf);

    gd_rect = &GD_RECT(graphics_device);
    gd_rect->top = gd_rect->left = CWC(0);
    gd_rect->bottom = CW(vdriver_height);
    gd_rect->right = CW(vdriver_width);
    PIXMAP_BOUNDS(gd_pixmap) = *gd_rect;

    /* add ourselves to the device list */
    GD_NEXT_GD_X(graphics_device) = LM(DeviceList);
    LM(DeviceList) = RM(graphics_device);

    /* Assure that we're using the correct colors. */
    vdriver_set_colors(0, 1 << vdriver_bpp,
                       CTAB_TABLE(PIXMAP_TABLE(GD_PMAP(graphics_device))));
}

GDHandle Executor::C_NewGDevice(INTEGER ref_num, LONGINT mode)
{
    GDHandle this2;
    GUEST<Handle> h;
    GUEST<PixMapHandle> pmh;

    TheZoneGuard guard(LM(SysZone));

    this2 = (GDHandle)NewHandle((Size)sizeof(GDevice));

    if(this2 == NULL)
        return this2;

    /* initialize fields; for some of these, i'm not sure what
	  the value should be */
    GD_ID_X(this2) = CWC(0); /* ??? */

    /* CLUT graphics device by default */
    GD_TYPE_X(this2) = CWC(clutType);

    /* how do i allocate a new inverse color table? */
    h = RM(NewHandle(0));
    GD_ITABLE_X(this2) = guest_cast<ITabHandle>(h);
    GD_RES_PREF_X(this2) = CWC(DEFAULT_ITABLE_RESOLUTION);

    GD_SEARCH_PROC_X(this2) = nullptr;
    GD_COMP_PROC_X(this2) = nullptr;

    GD_FLAGS_X(this2) = CWC(0);
    /* mode_from_bpp (1)  indicates b/w hardware */
    if(mode != mode_from_bpp(1))
        GD_FLAGS_X(this2).raw_or(CWC(1 << gdDevType));

    pmh = RM(NewPixMap());
    GD_PMAP_X(this2) = pmh;
    CTAB_FLAGS_X(PIXMAP_TABLE(GD_PMAP(this2))) |= CTAB_GDEVICE_BIT_X;

    GD_REF_CON_X(this2) = CLC(0); /* ??? */
    GD_REF_NUM_X(this2) = CW(ref_num); /* ??? */
    GD_MODE_X(this2) = CL(mode); /* ??? */

    GD_NEXT_GD_X(this2) = nullptr;

    GD_RECT(this2).top = CWC(0);
    GD_RECT(this2).left = CWC(0);
    GD_RECT(this2).bottom = CWC(0);
    GD_RECT(this2).right = CWC(0);

    /* handle to cursor's expanded data/mask? */
    GD_CCBYTES_X(this2) = CWC(0);
    GD_CCDEPTH_X(this2) = CWC(0);
    GD_CCXDATA_X(this2) = nullptr;
    GD_CCXMASK_X(this2) = nullptr;

    GD_RESERVED_X(this2) = CLC(0);

    /* if mode is -1, this2 is a user created gdevice,
	  and InitGDevice () should not be called (IMV-122) */
    if(mode != -1)
        InitGDevice(ref_num, mode, this2);

    return this2;
}

void Executor::gd_set_bpp(GDHandle gd, bool color_p, bool fixed_p, int bpp)
{
    PixMapHandle gd_pixmap;
    bool main_device_p = (gd == MR(LM(MainDevice)));

    /* set the color bit, all other flag bits should be the same */
    if(color_p)
        GD_FLAGS_X(gd).raw_or(CWC(1 << gdDevType));
    else
        GD_FLAGS_X(gd).raw_and(CWC(~(1 << gdDevType)));

    GD_TYPE_X(gd) = (bpp > 8
                         ? CWC(directType)
                         : (fixed_p ? CWC(fixedType) : CWC(clutType)));

    gd_pixmap = GD_PMAP(gd);
    pixmap_set_pixel_fields(STARH(gd_pixmap), bpp);

    if(bpp <= 8)
    {
        CTabHandle gd_color_table;

        gd_color_table = PIXMAP_TABLE(gd_pixmap);

        if(fixed_p)
        {
            if(!main_device_p)
                gui_fatal("unable to set bpp of gd not the screen");
            else
            {
                CTabHandle gd_color_table;

                gd_color_table = PIXMAP_TABLE(gd_pixmap);
                SetHandleSize((Handle)gd_color_table,
                              CTAB_STORAGE_FOR_SIZE((1 << bpp) - 1));
                CTAB_SIZE_X(gd_color_table) = CW((1 << bpp) - 1);
                vdriver_get_colors(0, 1 << bpp,
                                   CTAB_TABLE(gd_color_table));

                CTAB_SEED_X(gd_color_table) = CL(GetCTSeed());
            }
        }
        else
        {
            CTabHandle temp_color_table;

            temp_color_table = GetCTable(color_p
                                             ? bpp
                                             : (bpp + 32));
            if(temp_color_table == NULL)
                gui_fatal("unable to get color table `%d'",
                          color_p ? bpp : (bpp + 32));
            ROMlib_copy_ctab(temp_color_table, gd_color_table);
            DisposCTable(temp_color_table);
        }

        CTAB_FLAGS_X(gd_color_table) = CTAB_GDEVICE_BIT_X;
        MakeITable(gd_color_table, GD_ITABLE(gd), GD_RES_PREF(gd));

        if(main_device_p && !fixed_p)
            vdriver_set_colors(0, 1 << bpp, CTAB_TABLE(gd_color_table));
    }
}

/* it seems that `gd_ref_num' describes which device to initialize,
   and `mode' tells it what mode to start it in

   i'm not sure how this2 relates to NeXT/vga video hardware, for now,
   we do nothing */
void Executor::C_InitGDevice(INTEGER gd_ref_num, LONGINT mode, GDHandle gdh)
{
}

void Executor::C_SetDeviceAttribute(GDHandle gdh, INTEGER attribute,
                                    BOOLEAN value)
{
    if(value)
        GD_FLAGS_X(gdh).raw_or(CW(1 << attribute));
    else
        GD_FLAGS_X(gdh).raw_and(CW(~(1 << attribute)));
}

void Executor::C_SetGDevice(GDHandle gdh)
{
    if(LM(TheGDevice) != RM(gdh))
    {
        LM(TheGDevice) = RM(gdh);
        ROMlib_invalidate_conversion_tables();
    }
}

void Executor::C_DisposeGDevice(GDHandle gdh)
{
    DisposHandle((Handle)GD_ITABLE(gdh));
    DisposPixMap(GD_PMAP(gdh));

    /* FIXME: do other cleanup */
    DisposHandle((Handle)gdh);
}

GDHandle Executor::C_GetGDevice()
{
    return MR(LM(TheGDevice));
}

GDHandle Executor::C_GetDeviceList()
{
    return MR(LM(DeviceList));
}

GDHandle Executor::C_GetMainDevice()
{
    GDHandle retval;

    retval = MR(LM(MainDevice));

    /* Unfortunately, Realmz winds up dereferencing non-existent
     memory unless noDriver is set, but PacMan Deluxe will have
     trouble if that bit is set. */

    if(ROMlib_creator == TICK("RLMZ"))
        GD_FLAGS_X(retval).raw_or(CWC(1 << noDriver));
    else
        GD_FLAGS_X(retval).raw_and(~CWC(1 << noDriver));

    return retval;
}

GDHandle Executor::C_GetMaxDevice(Rect *globalRect)
{
    /* FIXME:
     currently we have only a single device, so that has
     the max pixel depth for any given region (tho we would
     probably see if it intersects the main screen and return
     NULL otherwise */
    return MR(LM(MainDevice));
}

GDHandle Executor::C_GetNextDevice(GDHandle cur_device)
{
    return GD_NEXT_GD(cur_device);
}

void Executor::C_DeviceLoop(RgnHandle rgn,
                            DeviceLoopDrawingProcPtr drawing_proc,
                            LONGINT user_data, DeviceLoopFlags flags)
{
    GDHandle gd;
    GUEST<RgnHandle> save_vis_rgn_x;
    RgnHandle sect_rgn, gd_rect_rgn;

    save_vis_rgn_x = PORT_VIS_REGION_X(thePort);

    sect_rgn = NewRgn();
    gd_rect_rgn = NewRgn();

    /* Loop over all GDevices, looking for active screens. */
    for(gd = MR(LM(DeviceList)); gd; gd = GD_NEXT_GD(gd))
        if((GD_FLAGS_X(gd) & CWC((1 << screenDevice)
                                 | (1 << screenActive)))
           == CWC((1 << screenDevice) | (1 << screenActive)))
        {
            /* NOTE: I'm blowing off some flags that come into play when
	 * you have multiple screens.  I don't think anything terribly
	 * bad would happen even if we had multiple screens, but we
	 * don't.  We can worry about it later.
	 */

            if(!(flags & allDevices))
            {
                Rect gd_rect, *port_bounds;

                /* Map the GDevice rect into thePort local coordinates. */
                gd_rect = GD_RECT(gd);
                port_bounds = &(PORT_BOUNDS(thePort));
                OffsetRect(&gd_rect,
                           CW(port_bounds->left),
                           CW(port_bounds->top));

                /* Intersect GDevice rect with the specified region. */
                RectRgn(gd_rect_rgn, &gd_rect);
                SectRgn(gd_rect_rgn, rgn, sect_rgn);

                SectRgn(sect_rgn, PORT_VIS_REGION(thePort), sect_rgn);

                /* Save it away in thePort. */
                PORT_VIS_REGION_X(thePort) = RM(sect_rgn);
            }

            if((flags & allDevices) || !EmptyRgn(sect_rgn))
            {
                drawing_proc(PIXMAP_PIXEL_SIZE(GD_PMAP(gd)),
                             GD_FLAGS(gd), gd, user_data);
            }
        }

    PORT_VIS_REGION_X(thePort) = save_vis_rgn_x;

    DisposeRgn(gd_rect_rgn);
    DisposeRgn(sect_rgn);
}

BOOLEAN Executor::C_TestDeviceAttribute(GDHandle gdh, INTEGER attribute)
{
    BOOLEAN retval;

    retval = (GD_FLAGS(gdh) & (1 << attribute)) != 0;
    return retval;
}

// FIXME: #warning ScreenRes is duplicate with toolutil.cpp
void Executor::C_ScreenRes(GUEST<INTEGER> *h_res, GUEST<INTEGER> *v_res)
{
    *h_res = CW(PIXMAP_HRES(GD_PMAP(MR(LM(MainDevice)))) >> 16);
    *v_res = CW(PIXMAP_VRES(GD_PMAP(MR(LM(MainDevice)))) >> 16);
}

INTEGER Executor::C_HasDepth(GDHandle gdh, INTEGER bpp, INTEGER which_flags,
                             INTEGER flags)
{
    flags &= ~1;
    which_flags &= ~1;

    if(gdh != MR(LM(MainDevice))
       || bpp == 0)
        return false;

    return (vdriver_acceptable_mode_p(0, 0, bpp, ((which_flags & (1 << gdDevType))
                                                      ? (flags & (1 << gdDevType)) == (1 << gdDevType)
                                                      : vdriver_grayscale_p),
                                      false));
}

OSErr Executor::C_SetDepth(GDHandle gdh, INTEGER bpp, INTEGER which_flags,
                           INTEGER flags)
{
    PixMapHandle gd_pixmap;
    WindowPeek tw;
    virtual_int_state_t int_state;

    if(gdh != MR(LM(MainDevice)))
    {
        warning_unexpected("Setting the depth of a device not the screen; "
                           "this violates bogus assumptions in SetDepth.");
    }

    gd_pixmap = GD_PMAP(gdh);

    if(bpp == PIXMAP_PIXEL_SIZE(gd_pixmap))
        return noErr;

    int_state = block_virtual_ints();
    HideCursor();

    note_executor_changed_screen(0, vdriver_height);

    if(bpp == 0 || !vdriver_set_mode(0, 0, bpp, ((which_flags & (1 << gdDevType)) ? !(flags & (1 << gdDevType)) : vdriver_grayscale_p)))
    {
        /* IMV says this2 returns non-zero in error case; not positive
	 cDepthErr is correct; need to verify */
        ShowCursor();
        restore_virtual_ints(int_state);
        return cDepthErr;
    }

    if(vdriver_fbuf == NULL)
        gui_fatal("vdriver not initialized, unable to change bpp");
    gd_set_bpp(gdh, !vdriver_grayscale_p, vdriver_fixed_clut_p, bpp);

    PIXMAP_SET_ROWBYTES_X(gd_pixmap, CW(vdriver_row_bytes));
    screenBitsX.rowBytes = CW(vdriver_row_bytes);

    cursor_reset_current_cursor();

    ShowCursor();
    restore_virtual_ints(int_state);

    /* FIXME: assuming (1) all windows are on the current
     graphics device, and (2) the rowbytes and baseaddr
     of the gdevice cannot change */
    /* set the pixel size, rowbytes, etc
     of windows and the window manager color graphics port */

    if(LM(WWExist) == EXIST_YES)
    {
        for(tw = MR(LM(WindowList)); tw; tw = WINDOW_NEXT_WINDOW(tw))
        {
            GrafPtr gp;

            gp = WINDOW_PORT(tw);

            if(CGrafPort_p(gp))
            {
                PixMapHandle window_pixmap;

                window_pixmap = CPORT_PIXMAP(gp);
                PIXMAP_PIXEL_SIZE_X(window_pixmap)
                    = PIXMAP_PIXEL_SIZE_X(gd_pixmap);
                PIXMAP_CMP_SIZE_X(window_pixmap)
                    = PIXMAP_PIXEL_SIZE_X(gd_pixmap);
                PIXMAP_SET_ROWBYTES_X(window_pixmap,
                                      PIXMAP_ROWBYTES_X(gd_pixmap));

                ROMlib_copy_ctab(PIXMAP_TABLE(gd_pixmap),
                                 PIXMAP_TABLE(window_pixmap));

                ThePortGuard guard(gp);
                RGBForeColor(&CPORT_RGB_FG_COLOR(gp));
                RGBBackColor(&CPORT_RGB_BK_COLOR(gp));
            }
            else
            {
                BITMAP_SET_ROWBYTES_X(&PORT_BITS(gp),
                                      PIXMAP_ROWBYTES_X(gd_pixmap));
            }
        }

        /* do the same for the LM(WMgrCPort) */
        {
            PixMapHandle wmgr_cport_pixmap;

            wmgr_cport_pixmap = CPORT_PIXMAP(MR(LM(WMgrCPort)));

            PIXMAP_PIXEL_SIZE_X(wmgr_cport_pixmap)
                = PIXMAP_PIXEL_SIZE_X(gd_pixmap);
            PIXMAP_CMP_SIZE_X(wmgr_cport_pixmap)
                = PIXMAP_PIXEL_SIZE_X(gd_pixmap);
            PIXMAP_SET_ROWBYTES_X(wmgr_cport_pixmap,
                                  PIXMAP_ROWBYTES_X(gd_pixmap));

            ROMlib_copy_ctab(PIXMAP_TABLE(gd_pixmap),
                             PIXMAP_TABLE(wmgr_cport_pixmap));
        }

        ThePortGuard guard(MR(wmgr_port));
        RGBForeColor(&ROMlib_black_rgb_color);
        RGBBackColor(&ROMlib_white_rgb_color);
    }

    /* Redraw the screen if that's what changed. */
    if(gdh == MR(LM(MainDevice)))
        redraw_screen();

    return noErr;
}

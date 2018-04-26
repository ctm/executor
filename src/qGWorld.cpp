/* Copyright 1994 - 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/gworld.h"
#include "rsys/host.h"
#include "rsys/mman.h"
#include "rsys/qcolor.h"

using namespace Executor;

gw_info_t *gw_info_head, *gw_info_free;

void Executor::ROMlib_InitGWorlds(void)
{
    gw_info_head = NULL;
    gw_info_free = NULL;
}

#define LOOKUP_GW_INFO_BY_EXPR(expr, value)   \
    {                                         \
        gw_info_t *t;                         \
                                              \
        for(t = gw_info_head; t; t = t->next) \
        {                                     \
            if(expr == value)                 \
            {                                 \
                return t;                     \
            }                                 \
        }                                     \
        return NULL;                          \
    }

gw_info_t *
Executor::lookup_gw_info_by_gw(GWorldPtr gw)
{
    LOOKUP_GW_INFO_BY_EXPR(t->gw, gw);
}

gw_info_t *
Executor::lookup_gw_info_by_gw_pixmap(PixMapHandle gw_pixmap)
{
    LOOKUP_GW_INFO_BY_EXPR(t->gw_pixmap, gw_pixmap);
}

gw_info_t *
Executor::lookup_gw_info_by_gw_pixmap_baseaddr(void *baseaddr)
{
    LOOKUP_GW_INFO_BY_EXPR(PIXMAP_BASEADDR(t->gw_pixmap),
                           baseaddr);
}

gw_info_t *
Executor::lookup_gw_info_by_gw_gd_pixmap(PixMapHandle gw_gd_pixmap)
{
    LOOKUP_GW_INFO_BY_EXPR(t->gw_gd_pixmap,
                           gw_gd_pixmap);
}

static void
install_new_gw_info(GWorldPtr graphics_world,
                    GDHandle graphics_world_device,
                    GWorldFlags flags,
                    int gd_allocated_p)
{
    gw_info_t *new_gw_info;

    if(gw_info_free)
    {
        new_gw_info = gw_info_free;
        gw_info_free = gw_info_free->next;
    }
    else
    {
        new_gw_info = (gw_info_t *)NewPtr(sizeof *new_gw_info);
    }

    new_gw_info->next = gw_info_head;
    gw_info_head = new_gw_info;

    new_gw_info->gw = graphics_world;
    new_gw_info->gw_pixmap = CPORT_PIXMAP(graphics_world);
    new_gw_info->gw_gd = graphics_world_device;
    new_gw_info->gw_gd_pixmap = GD_PMAP(graphics_world_device);
    new_gw_info->flags = flags;
    new_gw_info->pixel_lock_count = 0;
    new_gw_info->gd_allocated_p = gd_allocated_p;
}

void delete_gw_info(gw_info_t *gw_info)
{
    gw_info_t *t;

    if(gw_info == gw_info_head)
    {
        gw_info_head = gw_info->next;
    }
    else
    {
        for(t = gw_info_head; t; t = t->next)
        {
            if(t->next == gw_info)
            {
                t->next = t->next->next;
                break;
            }
        }
        if(t == NULL)
            gui_fatal("gw_info `%p' not in info chain", gw_info);
    }

    gw_info->next = gw_info_free;
    gw_info_free = gw_info;
}

QDErr Executor::C_NewGWorld(GUEST<GWorldPtr> *graphics_world_out,
                            INTEGER depth, Rect *bounds, CTabHandle ctab,
                            GDHandle gw_gd, GWorldFlags flags)
{
    PixMapHandle gw_pixmap, gd_pixmap;
    GWorldPtr graphics_world;
    GUEST<GrafPtr> save_portX;
    int gd_allocated_p = false;

    save_portX = thePortX;

    if(!depth)
    {
        GDHandle max_gd;

        max_gd = GetMaxDevice(bounds);
        if(max_gd == NULL)
            return paramErr;
        gw_gd = max_gd;
        depth = PIXMAP_PIXEL_SIZE(GD_PMAP(max_gd));

        if(bounds->top || bounds->left)
        {
            Rect *temp_bounds = (Rect *)alloca(sizeof *temp_bounds);

            temp_bounds->bottom = CW(RECT_HEIGHT(bounds));
            temp_bounds->right = CW(RECT_WIDTH(bounds));

            temp_bounds->top = temp_bounds->left = CWC(0);

            bounds = temp_bounds;
        }
    }

#if defined(BASIC_QD_ONLY)
    gui_assert(depth == 1 && gw_gd == NULL);
#endif /* BASIC_QD_ONLY */

    /* tests show that if we allocate the gdevice, then the gdevice's
     baseAddr is the graphics world's locked handle.

     but if the user passes the gdevice in, the pixmap is allocated
     its own handle */

    if(!(flags & noNewDevice))
    {
        Handle gd_pixmap_baseaddr;
        int row_bytes;

        gw_gd = NewGDevice(/* dummy */ 1,
                           /* don't initialize */ -1);
        gd_allocated_p = true;
        /* set the various fields of the newly created graphics
	 device */
        GD_RECT(gw_gd) = *bounds;

        gd_pixmap = GD_PMAP(gw_gd);

        row_bytes = ((RECT_WIDTH(bounds) * depth + 31) / 32) * 4;
        PIXMAP_SET_ROWBYTES_X(gd_pixmap, CW(row_bytes));

        gd_pixmap_baseaddr = NewHandle(row_bytes * RECT_HEIGHT(bounds));
        if(MemError() != noErr)
            return MemError();

        PIXMAP_BASEADDR_X(gd_pixmap) = RM((Ptr)gd_pixmap_baseaddr);

        PIXMAP_BOUNDS(gd_pixmap) = *bounds;
        if(depth > 8)
        {
            PIXMAP_PIXEL_TYPE_X(gd_pixmap) = CWC(RGBDirect);
            PIXMAP_CMP_COUNT_X(gd_pixmap) = CWC(3);
            switch(depth)
            {
                case 16:
                    PIXMAP_CMP_SIZE_X(gd_pixmap) = CWC(5);
                    break;
                case 32:
                    PIXMAP_CMP_SIZE_X(gd_pixmap) = CWC(8);
                    break;
                default:
                    gui_fatal("unknown pixel size");
            }
        }
        else
        {
            PIXMAP_PIXEL_TYPE_X(gd_pixmap) = CWC(0);
            PIXMAP_CMP_COUNT_X(gd_pixmap) = CWC(1);
            PIXMAP_CMP_SIZE_X(gd_pixmap) = CW(depth);
        }
        PIXMAP_PIXEL_SIZE_X(gd_pixmap) = CW(depth);
        if(ctab == NULL && depth <= 8)
        {
            int ctab_max_elt = (1 << depth) - 1;

            ctab = PIXMAP_TABLE(gd_pixmap);

            SetHandleSize((Handle)ctab,
                          CTAB_STORAGE_FOR_SIZE(ctab_max_elt));
            CTAB_SIZE_X(ctab) = CW(ctab_max_elt);
            CTAB_SEED_X(ctab) = CL((int32_t)depth);
            CTAB_FLAGS_X(ctab) = CTAB_GDEVICE_BIT_X;
            memcpy(CTAB_TABLE(ctab),
                   default_ctab_colors[ROMlib_log2[depth]],
                   sizeof(ColorSpec) << depth);
        }
        else if(ctab && depth <= 8)
        {
            ROMlib_copy_ctab(ctab, PIXMAP_TABLE(gd_pixmap));
        }

        /* #### if color table in the main device matches `ctab', then
	 we should copy the inverse color table from the main device */
        MakeITable(PIXMAP_TABLE(gd_pixmap), GD_ITABLE(gw_gd),
                   GD_RES_PREF(gw_gd));
    }
    else
    {
        if(!gw_gd)
            return paramErr;

        gd_pixmap = GD_PMAP(gw_gd);
        depth = PIXMAP_PIXEL_SIZE(gd_pixmap);
    }

    TheGDeviceGuard guard(gw_gd);
    /* allocate and initialize a new graphics world structure */
    graphics_world = (GWorldPtr)NewPtr(sizeof(GWorld));
    OpenCPort(GW_CPORT(graphics_world));

    gw_pixmap = CPORT_PIXMAP(graphics_world);

    if(!gd_allocated_p)
    {
        Handle gw_pixmap_baseaddr;
        int row_bytes;

        /* #### the `OpenCPort ()' initialization of `gw' sets the
	      default depth of `gw_pixmap' to that of the gworld,
	      which may not match `depth' */
        row_bytes = ((RECT_WIDTH(bounds) * depth + 31) / 32) * 4;

        PIXMAP_SET_ROWBYTES_X(gw_pixmap, CW(row_bytes));

        gw_pixmap_baseaddr = NewHandle(row_bytes * RECT_HEIGHT(bounds));
        if(MemError() != noErr)
            /* ### err, i'm not sure what the mac does when this
		allocation fails */
            gw_pixmap_baseaddr = NewHandle(0);
        PIXMAP_BASEADDR_X(gw_pixmap)
            = RM((Ptr)gw_pixmap_baseaddr);

        PORT_RECT(graphics_world) = PIXMAP_BOUNDS(gw_pixmap) = *bounds;
    }

    /* mark as a GWorld and not a CGrafPort */
    CPORT_VERSION_X(graphics_world).raw_or(GW_FLAG_BIT_X);

    install_new_gw_info(graphics_world, gw_gd, 0, gd_allocated_p);

    if(flags & pixelsLocked)
        LockPixels(gw_pixmap);

    *graphics_world_out = RM(graphics_world);

    thePortX = save_portX;
    return noErr;
}

Boolean Executor::C_LockPixels(PixMapHandle pixels)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw_pixmap(pixels);
    Handle pixels_baseaddr_h;

    if(gw_info)
    {
        if(gw_info->flags & pixelsLocked)
        {
            gw_info->pixel_lock_count++;
            return true;
        }

        gw_info->flags |= pixelsLocked;
        gui_assert(!gw_info->pixel_lock_count);
        gw_info->pixel_lock_count = 1;

        pixels_baseaddr_h = (Handle)PIXMAP_BASEADDR(pixels);
        /* lock the baseaddr handle memory */
        HSetState(pixels_baseaddr_h, HGetState(pixels_baseaddr_h) | LOCKBIT);
        PIXMAP_BASEADDR_X(pixels) = *pixels_baseaddr_h;

        HSetState((Handle)pixels, HGetState((Handle)pixels) | LOCKBIT);
    }

    return true;
}

void Executor::C_UnlockPixels(PixMapHandle pixels)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw_pixmap(pixels);
    Handle pixels_baseaddr_h;
    Ptr pixels_baseaddr;

    if(gw_info)
    {
        if(!(gw_info->flags & pixelsLocked))
        {
            warning_unexpected("attemp to unlock unlocked pixels");
            return;
        }

        gw_info->pixel_lock_count--;
        if(gw_info->flags & pixelsLocked
           && !gw_info->pixel_lock_count)
        {
            gw_info->flags &= ~pixelsLocked;

            pixels_baseaddr = PIXMAP_BASEADDR(pixels);
            pixels_baseaddr_h = RecoverHandle(pixels_baseaddr);
            HSetState(pixels_baseaddr_h,
                      HGetState(pixels_baseaddr_h) & ~LOCKBIT);
            PIXMAP_BASEADDR_X(pixels) = RM((Ptr)pixels_baseaddr_h);

            HSetState((Handle)pixels, HGetState((Handle)pixels) & ~LOCKBIT);
        }
    }
}

GWorldFlags Executor::C_UpdateGWorld(GUEST<GWorldPtr> *graphics_world,
                                     INTEGER depth, Rect *bounds,
                                     CTabHandle ctab, GDHandle a_gdevice,
                                     GWorldFlags flags)
{
    gw_info_t *gw_info;
    GWorldPtr gw;
    PixMapHandle gw_pmap;
    CTabHandle gw_pmap_ctab;
    GDHandle gw_gd;
    GDHandle max_gd = NULL;
    QDErr err;
    GWorldFlags retval = 0;

    gw_info = lookup_gw_info_by_gw(MR(*graphics_world));

    gw = gw_info->gw;
    gw_pmap = gw_info->gw_pixmap;
    gw_gd = gw_info->gw_gd;
    gw_pmap_ctab = PIXMAP_TABLE(gw_pmap);

    if(flags & ~(keepLocal | clipPix | stretchPix | ditherPix))
        return paramErr;

    if(!depth)
    {
        PixMapHandle max_gd_pixmap;

        max_gd = GetMaxDevice(bounds);
        max_gd_pixmap = GD_PMAP(max_gd);

        ctab = PIXMAP_TABLE(max_gd_pixmap);
        depth = PIXMAP_PIXEL_SIZE(GD_PMAP(max_gd));
        gw_gd = max_gd;

        /* #### unclear if this is correct, but it is what `NewGWorld ()'
	 does */
        if(bounds->top || bounds->left)
        {
            Rect *temp_bounds = (Rect *)alloca(sizeof *temp_bounds);

            *temp_bounds = *bounds;
            OffsetRect(temp_bounds,
                       -CW(temp_bounds->left), -CW(temp_bounds->top));
            bounds = temp_bounds;
        }
    }

    if(ctab
       && CTAB_SEED_X(ctab) != CTAB_SEED_X(gw_pmap_ctab))
        retval |= mapPix;

    if(depth != PIXMAP_PIXEL_SIZE(gw_pmap))
        retval |= newDepth;

    if(memcmp(bounds, &PIXMAP_BOUNDS(gw_pmap),
              sizeof *bounds))
    {
        if(flags & clipPix)
            retval |= clipPix;
        else if(flags & stretchPix)
            retval |= stretchPix;
        else
            retval |= reallocPix;
    }
    if(retval & (mapPix | newDepth | clipPix | stretchPix | reallocPix))
    {
        GWorldPtr gw_ret;
        GUEST<GWorldPtr> gw_ret_x;
        PixMapHandle gw_pmap_ret;
        gw_info_t *gw_info_ret;
        Rect src_rect, dst_rect;

        err = NewGWorld(&gw_ret_x, depth, bounds, ctab, gw_gd, flags);
        if(err != noErr)
            return err;

        gw_ret = MR(gw_ret_x);
        gw_info_ret = lookup_gw_info_by_gw(gw_ret);
        gw_pmap_ret = CPORT_PIXMAP(gw_ret);

        if(PIXMAP_ROWBYTES_X(gw_pmap_ret) != PIXMAP_ROWBYTES_X(gw_pmap))
            retval |= newRowBytes;

        LockPixels(gw_pmap_ret);

        if(flags & (stretchPix | clipPix))
        {
            if(flags & stretchPix)
            {
                src_rect = PIXMAP_BOUNDS(gw_pmap);
                dst_rect = PIXMAP_BOUNDS(gw_pmap_ret);
            }
            else /* if (flags & clipPix) */
            {
                Rect *gw_bounds_ret, *gw_bounds;
                Rect temp_rect;

                gw_bounds_ret = &PIXMAP_BOUNDS(gw_pmap_ret);
                gw_bounds = &PIXMAP_BOUNDS(gw_pmap);

                src_rect = *gw_bounds;
                dst_rect = *gw_bounds_ret;

                OffsetRect(&src_rect,
                           -CW(src_rect.left), -CW(src_rect.top));
                OffsetRect(&dst_rect,
                           -CW(dst_rect.left), -CW(dst_rect.top));

                SectRect(&src_rect, &dst_rect, &temp_rect);
                src_rect = dst_rect = temp_rect;

                OffsetRect(&src_rect,
                           CW(gw_bounds->left), CW(gw_bounds->top));
                OffsetRect(&dst_rect,
                           CW(gw_bounds_ret->left), CW(gw_bounds_ret->top));
            }

            CopyBits(PORT_BITS_FOR_COPY(gw),
                     PORT_BITS_FOR_COPY(gw_ret),
                     &src_rect, &dst_rect,
                     srcCopy, NULL);
        }
        if(gw_info->flags & pixelsLocked)
        {
            gw_info_ret->flags |= pixelsLocked;
            gw_info_ret->pixel_lock_count = gw_info->pixel_lock_count;
        }
        else
            UnlockPixels(gw_pmap_ret);

        DisposeGWorld(gw);
        *graphics_world = gw_ret_x;
    }

    return retval;
}

void Executor::C_DisposeGWorld(GWorldPtr graphics_world)
{
    gw_info_t *gw_info;

    if((GrafPtr)graphics_world == thePort)
        thePortX = LM(WMgrPort);
    /* FIXME: set the gdevice to something sane as well? */

    gw_info = lookup_gw_info_by_gw(graphics_world);

    if(!gw_info)
    {
        warning_unexpected("no gw_info, no deallocation done");
        return;
    }

    /* dispose the fbuf */
    if(gw_info->flags & pixelsLocked)
    {
        Handle h;

        h = RecoverHandle((Ptr)(PIXMAP_BASEADDR(gw_info->gw_pixmap)));
        DisposHandle(h);
    }
    else
        DisposHandle((Handle)PIXMAP_BASEADDR(gw_info->gw_pixmap));

    if(gw_info->gd_allocated_p)
    {
        DisposeGDevice(gw_info->gw_gd);
    }

    ClosePort((GrafPtr)graphics_world);
    DisposPtr((Ptr)graphics_world);

    delete_gw_info(gw_info);
}

void Executor::C_GetGWorld(GUEST<CGrafPtr> *port,
                           GUEST<GDHandle> *graphics_device)
{
    *port = theCPortX;
    *graphics_device = LM(TheGDevice);
}

void Executor::C_SetGWorld(CGrafPtr port, GDHandle graphics_device)
{
    if(port && GRAPHICS_WORLD_P(port))
    {
        gw_info_t *gw_info;

        gw_info = lookup_gw_info_by_gw((GWorldPtr)port);
        gui_assert(gw_info);

        LM(TheGDevice) = RM(gw_info->gw_gd);
    }
    else
    {
        if(graphics_device)
            LM(TheGDevice) = RM(graphics_device);
        else
        {
            PixMapHandle gd_pixmap;

            gd_pixmap = GD_PMAP(MR(LM(MainDevice)));

            if(port
               && (PIXMAP_BASEADDR_X(gd_pixmap) == PORT_BASEADDR_X(port)))
                LM(TheGDevice) = LM(MainDevice);
        }
    }

    theCPortX = RM(port);
}

void Executor::C_AllowPurgePixels(PixMapHandle pixels)
{
    gw_info_t *gw_info;
    Handle pixels_baseaddr_h;

    gw_info = lookup_gw_info_by_gw_pixmap(pixels);
    if(gw_info)
    {
        pixels_baseaddr_h = (Handle)PIXMAP_BASEADDR(pixels);
        if(gw_info->flags & pixelsLocked)
            pixels_baseaddr_h = RecoverHandle((Ptr)pixels_baseaddr_h);
        HPurge(pixels_baseaddr_h);
        gw_info->flags |= pixelsPurgeable;
    }
}

void Executor::C_NoPurgePixels(PixMapHandle pixels)
{
    gw_info_t *gw_info;
    Handle pixels_baseaddr_h;

    gw_info = lookup_gw_info_by_gw_pixmap(pixels);
    if(gw_info)
    {
        pixels_baseaddr_h = (Handle)PIXMAP_BASEADDR(pixels);
        if(gw_info->flags & pixelsLocked)
            pixels_baseaddr_h = RecoverHandle((Ptr)pixels_baseaddr_h);
        HNoPurge(pixels_baseaddr_h);
        gw_info->flags &= ~pixelsPurgeable;
    }
}

#define FLAGMASK (pixelsPurgeable | pixelsLocked | keepLocal)

GWorldFlags Executor::C_GetPixelsState(PixMapHandle pixels)
{
    gw_info_t *gw_info;

    gw_info = lookup_gw_info_by_gw_pixmap(pixels);

    return gw_info ? (gw_info->flags & FLAGMASK) : 0;
}

void Executor::C_SetPixelsState(PixMapHandle pixels, GWorldFlags state)
{
    gw_info_t *gw_info;

    if(state & pixelsPurgeable)
        AllowPurgePixels(pixels);
    else
        NoPurgePixels(pixels);

    if(state & pixelsLocked)
        LockPixels(pixels);
    else
        UnlockPixels(pixels);

    gw_info = lookup_gw_info_by_gw_pixmap(pixels);
    if(gw_info)
    {
        if(state & keepLocal)
            gw_info->flags |= keepLocal;
        else
            gw_info->flags &= ~keepLocal;
    }
}

Ptr Executor::C_GetPixBaseAddr(PixMapHandle pixels)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw_pixmap(pixels);

    if(gw_info == NULL)
    {
        gw_info = lookup_gw_info_by_gw_gd_pixmap(pixels);

        if(gw_info == NULL
           /* don't dereference the screen baseaddr! */
           || gw_info->gw_gd == MR(LM(MainDevice)))
            return PIXMAP_BASEADDR(pixels);
        else
            return STARH((Handle)PIXMAP_BASEADDR(pixels));
    }
    else if(gw_info->flags & pixelsLocked)
        return PIXMAP_BASEADDR(pixels);
    else
        return STARH((Handle)PIXMAP_BASEADDR(pixels));
}

QDErr Executor::C_NewScreenBuffer(Rect *global_rect, Boolean purgeable_p,
                                  GUEST<GDHandle> *graphics_device,
                                  GUEST<PixMapHandle> *offscreen_pixmap)
{
    GDHandle max_graphics_device;
    PixMapHandle pixels, gd_pixmap;
    int width, height;
    short rowbytes;
    int bpp;
    GUEST<Ptr> p;

    max_graphics_device = GetMaxDevice(global_rect);
    gd_pixmap = GD_PMAP(max_graphics_device);

    pixels = NewPixMap();
    if(pixels == NULL)
        return cNoMemErr;

    bpp = PIXMAP_PIXEL_SIZE(gd_pixmap);
    PIXMAP_PIXEL_SIZE_X(pixels) = PIXMAP_PIXEL_SIZE_X(gd_pixmap);
    PIXMAP_CMP_COUNT_X(pixels) = PIXMAP_CMP_COUNT_X(gd_pixmap);
    PIXMAP_CMP_SIZE_X(pixels) = PIXMAP_CMP_SIZE_X(gd_pixmap);

    ROMlib_copy_ctab(PIXMAP_TABLE(gd_pixmap), PIXMAP_TABLE(pixels));

    PIXMAP_BOUNDS(pixels) = *global_rect;
    height = RECT_HEIGHT(global_rect);
    width = RECT_WIDTH(global_rect);

    rowbytes = ((width * bpp + 31) / 32) * 4;

    PIXMAP_SET_ROWBYTES_X(pixels, CW(rowbytes));

    /* not clear if we should be allocating a ptr or a handle for the
     pixmap baseaddr */
    warning_unexpected("may be incorrectly allocating pointer");

    /* an unlocked pixel map for a graphics world contains a handle to
     the pixel data; not a pointer */
    p = RM((Ptr)NewPtr(rowbytes * height));

    if(p == NULL)
    {
        DisposPixMap(pixels);
        return cNoMemErr;
    }

    if(purgeable_p)
        AllowPurgePixels(pixels);
    else
        NoPurgePixels(pixels);
    PIXMAP_BASEADDR_X(pixels) = p;

    *graphics_device = RM(max_graphics_device);
    *offscreen_pixmap = RM(pixels);
    return noErr;
}

void Executor::C_DisposeScreenBuffer(PixMapHandle pixels)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw_pixmap(pixels);

    /* It's not clear whether we should do more if no gw_info */
    if(gw_info)
    {

        /* dispose of the actual offscreen buffer */
        if(gw_info->flags & pixelsLocked)
        {
            Handle baseaddr;

            baseaddr = RecoverHandle(PIXMAP_BASEADDR(pixels));
            DisposHandle(baseaddr);
        }
        else
            DisposHandle((Handle)PIXMAP_BASEADDR(pixels));

        /* and the pixmap */
        DisposPixMap(pixels);
    }
}

GDHandle Executor::C_GetGWorldDevice(GWorldPtr graphics_world)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw(graphics_world);

    return gw_info->gw_gd;
}

Boolean Executor::C_PixMap32Bit(PixMapHandle pixels)
{
    warning_unimplemented("poorly implemented");
    return true;
    /* #warning "Haven't verified that true is better than 0xff or 0 in PixMap32Bit" */
}

PixMapHandle Executor::C_GetGWorldPixMap(GWorldPtr port)
{
    gw_info_t *gw_info = lookup_gw_info_by_gw(port);

    if(gw_info)
        return gw_info->gw_pixmap;
    else
        /* ultima (and others) call `GetGWorldPixMap ()' with a non-gworld
       port */
        return CPORT_PIXMAP(port);
}

QDErr Executor::C_NewTempScreenBuffer(Rect *global_rect, Boolean purgeable_p,
                                      GUEST<GDHandle> *graphics_device,
                                      GUEST<PixMapHandle> *offscreen_pixmap)
{
    gui_fatal("unimplemented");
#if !defined(LETGCCWAIL)
    return paramErr;
#endif
}

LONGINT Executor::C_OffscreenVersion()
{
    /* #warning OffscreenVersion not properly implemented */
    warning_unimplemented("poorly implemented");
    return 0x130;
}

void Executor::C_GDeviceChanged(GDHandle graphics_device)
{
    /* #warning GDeviceChanged not implemented */
    ROMlib_invalidate_conversion_tables();
    warning_unimplemented(NULL_STRING);
}

void Executor::C_PortChanged(GrafPtr port)
{
    /* #warning "PortChanged not implemented; worked around for hypercard" */
    ROMlib_invalidate_conversion_tables();
    warning_unimplemented(NULL_STRING);
}

void Executor::C_PixPatChanged(PixPatHandle pixpat)
{
    /* #warning "PixPatChanged not implemented" */
    ROMlib_invalidate_conversion_tables();
    warning_unimplemented(NULL_STRING);
}

void Executor::C_CTabChanged(CTabHandle ctab)
{
    /* #warning "CTabChanged not implemented" */
    ROMlib_invalidate_conversion_tables();
    warning_unimplemented(NULL_STRING);
}

Boolean Executor::C_QDDone(GrafPtr port)
{
    /* #warning "QDDone not implemented" */
    warning_unimplemented(NULL_STRING);
    return true;
}

/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in WindowMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "ToolboxUtil.h"
#include "ResourceMgr.h"
#include "FontMgr.h"
#include "MemoryMgr.h"
#include "SegmentLdr.h"
#include "OSUtil.h"
#include "OSEvent.h"
#include "ControlMgr.h"
#include "MenuMgr.h"
#include "SysErr.h"
#include "DialogMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"
#include "rsys/menu.h"
#include "rsys/resource.h"
#include "rsys/system_error.h"

#include "rsys/prefs.h"
#include "rsys/flags.h"

#include "rsys/segment.h"
#include "rsys/file.h"
#include "rsys/executor.h"
#include "rsys/custom.h"
#include "rsys/options.h"
#include "rsys/launch.h"

using namespace Executor;

PUBLIC BOOLEAN Executor::ROMlib_dirtyvariant = false;

bool Executor::system_file_version_skew_p;

static void
exit_executor(void)
{
    ROMlib_exit = true;
    ExitToShell();
}

#if !defined(MSDOS)
PRIVATE std::string reinstall = "System and %System";
#else
PRIVATE char *reinstall = "EXSYSTEM.HFV";
#endif

P0(PUBLIC pascal trap, void, InitWindows)
{
    PatHandle ph;
    PixPatHandle new_ph;
    RgnHandle mrgn, corners;

    AuxWinHead = RM(default_aux_win);
    SaveVisRgn = NULL;

    {
        ThePortGuard guard(thePort);

        /* FIXME: is this a memory leak, to just call InitPort () again? */
        InitPort(MR(WMgrPort));
        InitCPort(MR(WMgrCPort));

        ph = GetPattern(deskPatID);
        if(ph == NULL)
        {
            fprintf(stderr, "Can't open System file.\n"
                            "This is very bad.  You will have to reinstall %s"
                            " before\n"
                            "Executor will work again.\n",
                    reinstall.c_str());
            ROMlib_exit = true;
            ExitToShell();
        }
        new_ph = GetPixPat(deskPatID);
        if(new_ph)
            DeskCPat = RM(new_ph);
        else
            USE_DESKCPAT_VAR &= ~USE_DESKCPAT_BIT;
        InitPalettes();
        InitMenus();
        PATASSIGN(DeskPattern, STARH(ph));
        GrayRgn = RM(NewRgn());

        OpenRgn();
        if(ROMlib_creator && !(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
            FrameRoundRect(&GD_BOUNDS(MR(TheGDevice)), 16, 16);
        else
            FrameRect(&GD_BOUNDS(MR(TheGDevice)));
        CloseRgn(MR(GrayRgn));
        mrgn = NewRgn();
        SetRectRgn(mrgn, 0, 0, CW(GD_BOUNDS(MR(TheGDevice)).right),
                   CW(MBarHeight));
        SectRgn(MR(GrayRgn), mrgn, mrgn);
        corners = NewRgn();
        SetRectRgn(corners, 0, 0, CW(GD_BOUNDS(MR(TheGDevice)).right),
                   CW(GD_BOUNDS(MR(TheGDevice)).bottom));
        DiffRgn(corners, MR(GrayRgn), corners);
        PaintRgn(corners);
        CopyRgn(MR(GrayRgn), PORT_VIS_REGION(MR(wmgr_port)));
        DiffRgn(MR(GrayRgn), mrgn, MR(GrayRgn));
        PenPat(white);
        PaintRgn(mrgn);
        PenPat(black);
        MoveTo(0, CW(MBarHeight) - 1);
        Line(CW(GD_BOUNDS(MR(TheGDevice)).right), 0);
        if((USE_DESKCPAT_VAR & USE_DESKCPAT_BIT)
           && PIXMAP_PIXEL_SIZE(GD_PMAP(MR(MainDevice))) > 2)
            FillCRgn(MR(GrayRgn), MR(DeskCPat));
        else
            FillRgn(MR(GrayRgn), DeskPattern);
        DisposeRgn(mrgn);
        DisposeRgn(corners);
        CopyRgn(MR(GrayRgn), PORT_CLIP_REGION(MR(wmgr_port)));
        WindowList = NULL;
        SaveUpdate = CWC(-1);
        PaintWhite = CWC(-1);
        DeskHook = NULL;
        GhostWindow = NULL;
        PATASSIGN(DragPattern, gray);
    }

    /* since there is no `InitControls ()', we do this here */
    ctl_color_init();

    WWExist = EXIST_YES;

    if(ROMlib_creator && ROMlib_creatorsp)
    {
        int i;
        bool found_p;
        int n_vals;

        n_vals = (ROMlib_creatorsp->head.length / sizeof ROMlib_creatorsp->vals[0]);
        for(found_p = false, i = 0; !found_p && i < n_vals; ++i)
            if(ROMlib_creatorsp->vals[i] == (uint32)ROMlib_creator)
                found_p = true;

        if(!found_p)
        {
            char msg_buf[1024];
            sprintf(msg_buf, "You are trying to run a Macintosh Application "
                             "for which this copy of Executor isn't licensed.  This "
                             "copy of Executor is licensed only to run specific "
                             "applications.  "
                             "Please choose the \"About Executor...\" menu item and "
                             "click on the \"License\" button for more information.");

            system_error(msg_buf, 0,
                         "Exit", NULL, NULL,
                         C_ExitToShell, NULL, NULL);
        }
    }

    {
        static bool issued_system_file_version_skew_warning_p = false;

        if(system_file_version_skew_p
           && !issued_system_file_version_skew_warning_p)
        {
            system_error("\
The system file you have installed appears to be too old. \
Executor may die without warning because of this mismatch",
                         0,
                         "Continue", "Exit", NULL,
                         NULL, exit_executor, NULL);
        }
        issued_system_file_version_skew_warning_p = true;
    }

#if defined(MSDOS)
    {
        static bool issued_cd_warning_p = false;

        if(cd_mounted_by_trickery_p && !issued_cd_warning_p)
        {
            char *warning_file;
            struct stat sbuf;

            warning_file = copystr("+/cdinfo.txt");
            if(warning_file)
            {
                if(stat(warning_file, &sbuf) == 0)
                {
                    char buf[1024];
                    int i;

                    for(i = strlen(warning_file) - 1; i >= 0; --i)
                        if(warning_file[i] == '/')
                            warning_file[i] = '\\';

                    sprintf(buf, "From DOS or Windows, please read the "
                                 "file \"%s\".  "
                                 "If you don't, Executor won't be "
                                 "able to read Mac CD-ROMS (except "
                                 "the Executor CD-ROM, which is special).",
                            warning_file);
                    system_error(buf, 0,
                                 "Exit", "Continue", NULL,
                                 exit_executor, NULL, NULL);
                }
                free(warning_file);
            }
            issued_cd_warning_p = true;
        }
    }
#endif

    switch(ROMlib_launch_failure)
    {
        case launch_no_failure:
            break;
        case launch_cfm_requiring:
            system_error("CFM-requiring applications are not currently supported.",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
            break;
        case launch_ppc_only:
#if !defined(powerpc) && !defined(__ppc__)
            system_error("That application is PowerPC-only.  This version of "
                         "Executor doesn't run PowerPC applications.  "
                         "You need to find an M68k version of that application.",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
#else
            system_error("That application is PowerPC-only.  To attempt to run "
                         "it, Executor must be started using the \"-ppc\" "
                         "command-line switch.",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
#endif
            break;
        case launch_damaged:
            system_error("That application appears damaged (lacks CODE and cfrg).",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
            break;

        case launch_compressed_ge7:
            system_error("That application has a compressed CODE 0.  "
                         "It is probably unusable under Executor but "
                         "might work in System 6 mode",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
            break;
        case launch_compressed_lt7:
            system_error("That application has a compressed CODE 0.  "
                         "It will not run under this version of Executor.",
                         0, "OK", NULL, NULL, NULL, NULL, NULL);
            break;

        default:
            warning_unexpected("%d", ROMlib_launch_failure);
            break;
    }
    ROMlib_launch_failure = launch_no_failure;

    if(!size_info.application_p)
        return;

    /* only issue warnings once */
    size_info.application_p = false;

    if((!size_info.size_resource_present_p
        || (size_info.size_flags & SZis32BitCompatible) != SZis32BitCompatible)
       && !ROMlib_nowarn32)
    {
        system_error("This application doesn't claim to be \"32 bit clean\".  "
                     "It is quite possible that this program will not work "
                     "under Executor.",
                     0,
                     "Continue", "Restart", NULL,
                     NULL, C_ExitToShell, NULL);
    }

    if(size_info.size_resource_present_p
       && ROMlib_applzone_size < size_info.preferred_size)
    {
        char msg_buf[1024];
        int applzone_in_k, preferred_size_in_k;

        applzone_in_k = ROMlib_applzone_size / 1024;
        preferred_size_in_k = (size_info.preferred_size + 1023) / 1024;

        sprintf(msg_buf, "This application prefers `%dk' of memory, "
                         "but only '%dk' of memory is available in the application "
                         "zone.  You should exit Executor and run it again "
                         "with \"-applzone %dk\".",
                preferred_size_in_k, applzone_in_k,
                preferred_size_in_k);

        system_error(msg_buf, 0,
                     "Continue", "Browser", "Exit",
                     NULL, C_ExitToShell, exit_executor);
    }
}

P1(PUBLIC pascal trap, void, GetWMgrPort, GUEST<GrafPtr> *, wp)
{
    *wp = WMgrPort;
}

P1(PUBLIC pascal trap, void, GetCWMgrPort, GUEST<CGrafPtr> *, wp)
{
    *wp = WMgrCPort;
}

P1(PUBLIC pascal trap, void, SetDeskCPat, PixPatHandle, ph)
{
    PatHandle bw_ph;

    if(ph)
    {
        DeskCPat = RM(ph);
        USE_DESKCPAT_VAR |= USE_DESKCPAT_BIT;
    }
    else
    {
        bw_ph = GetPattern(deskPatID);
        PATASSIGN(DeskPattern, STARH(bw_ph));
        USE_DESKCPAT_VAR &= ~USE_DESKCPAT_BIT;
    }
    PaintOne((WindowPeek)0, MR(GrayRgn));
}

static void
ROMlib_new_window_common(WindowPeek w,
                         int allocated_p, int cwindow_p,
                         Rect *bounds, StringPtr title, BOOLEAN visible_p,
                         INTEGER proc_id, WindowPtr behind,
                         BOOLEAN go_away_flag, LONGINT ref_con)
{
    WindowPeek t_w;
    AuxWinHandle t_aux_w;
    GrafPtr save_port;

    save_port = thePort;
    if(!title)
        title = (StringPtr) ""; /* thank MS Word for pointing this out */
    if(!behind)
    {
        WINDOW_NEXT_WINDOW_X(w) = nullptr;
        if(WindowList)
        {
            for(t_w = MR(WindowList);
                WINDOW_NEXT_WINDOW_X(t_w);
                t_w = WINDOW_NEXT_WINDOW(t_w))
                ;
            WINDOW_NEXT_WINDOW_X(t_w) = RM(w);
        }
        else
        {
            WindowList = RM(w);
            if(visible_p)
            {
                /* notify the palette manager that the `FrontWindow ()'
		 may have changed */
                pm_front_window_maybe_changed_hook();
            }
        }
    }
    else if(behind == (WindowPtr)-1L)
    {
        WINDOW_NEXT_WINDOW_X(w) = WindowList;
        WindowList = RM((WindowPeek)w);
        if(visible_p)
        {
            /* notify the palette manager that the `FrontWindow ()' may have
	     changed */
            pm_front_window_maybe_changed_hook();
        }
    }
    else
    {
        WINDOW_NEXT_WINDOW_X(w) = WINDOW_NEXT_WINDOW_X(behind);
        WINDOW_NEXT_WINDOW_X(behind) = RM((WindowPeek)w);
    }
    WINDOW_KIND_X(w) = CWC(userKind);
    WINDOW_VISIBLE_X(w) = visible_p;
    for(t_w = MR(WindowList);
        t_w && !WINDOW_VISIBLE(t_w);
        t_w = WINDOW_NEXT_WINDOW(t_w))
        ;
    WINDOW_HILITED_X(w) = visible_p && (t_w == w);
    if(WINDOW_HILITED_X(w))
    {
        CurActivate = RM((WindowPtr)w);
        for(t_w = WINDOW_NEXT_WINDOW(t_w);
            t_w && !WINDOW_HILITED_X(t_w);
            t_w = WINDOW_NEXT_WINDOW(t_w))
            ;
    }
    else
        t_w = 0; /* t_w will be used later */
    WINDOW_GO_AWAY_FLAG_X(w) = go_away_flag;
    WINDOW_SPARE_FLAG_X(w) = 0; /* will be used zoombox (wNew) */
    WINDOW_DATA_X(w) = 0;
    WINDOW_STRUCT_REGION_X(w) = RM(NewRgn());
    WINDOW_CONT_REGION_X(w) = RM(NewRgn());
    WINDOW_UPDATE_REGION_X(w) = RM(NewRgn());
    WINDOW_DEF_PROC_X(w) = RM(GetResource(TICK("WDEF"), proc_id >> 4));
    if(!WINDOW_DEF_PROC_X(w))
    {
        WINDOW_DEF_PROC_X(w) = RM(GetResource(TICK("WDEF"), 0));
        if(!WINDOW_DEF_PROC_X(w))
        {
            if(allocated_p)
                DisposPtr((Ptr)w);
            /* fatal_error ("no window (?)"); */
            gui_fatal("Unable to find WDEF.");
        }
    }

    t_aux_w = (AuxWinHandle)NewHandle(sizeof(AuxWinRec));
    HxX(t_aux_w, awNext) = AuxWinHead;
    HxX(t_aux_w, awOwner) = RM((WindowPtr)w);
    HxX(t_aux_w, awCTable) = RM((CTabHandle)GetResource(TICK("wctb"), 0));
    HxX(t_aux_w, dialogCItem) = 0;
    HxX(t_aux_w, awFlags) = CL((proc_id & 0xF) << 24);
    HxX(t_aux_w, awReserved) = 0;
    HxX(t_aux_w, awRefCon) = 0;
    AuxWinHead = RM(t_aux_w);

    {
        Handle t;

        PtrToHand((Ptr)title, &t, (LONGINT)title[0] + 1);
        WINDOW_TITLE_X(w) = RM((StringHandle)t);
    }

    if(cwindow_p)
        OpenCPort((CGrafPtr)w);
    else
        OpenPort((GrafPtr)w);
    OffsetRect(&PORT_BOUNDS(w), -CW(bounds->left), -CW(bounds->top));
    PORT_RECT(w) = *bounds;
    OffsetRect(&PORT_RECT(w), -CW(bounds->left), -CW(bounds->top));
    {
        HLockGuard guard(WINDOW_TITLE(w));
        WINDOW_TITLE_WIDTH_X(w) = CW(StringWidth(STARH(WINDOW_TITLE(w))));
    }

    TextFont(applFont);
    WINDOW_CONTROL_LIST_X(w) = nullptr;
    WINDOW_PIC_X(w) = nullptr;
    WINDOW_REF_CON_X(w) = CL(ref_con);
    WINDCALL((WindowPtr)w, wNew, 0);
    if(WINDOW_VISIBLE_X(w))
    {
        ThePortGuard guard(MR(wmgr_port));
        WINDCALL((WindowPtr)w, wCalcRgns, 0);
        SetClip(WINDOW_STRUCT_REGION(w));
        ClipAbove(w);
        PenPat(black);
        WINDCALL((WindowPtr)w, wDraw, 0);
        CalcVis(w);
        EraseRgn(WINDOW_CONT_REGION(w));
        CopyRgn(WINDOW_CONT_REGION(w), WINDOW_UPDATE_REGION(w));
        if(WINDOW_NEXT_WINDOW_X(w))
            CalcVisBehind(WINDOW_NEXT_WINDOW(w), WINDOW_STRUCT_REGION(w));
    }
    else
        SetEmptyRgn(PORT_VIS_REGION(w));
    if(t_w)
    {
        HiliteWindow((WindowPtr)t_w, false);
        CurDeactive = RM((WindowPtr)t_w);
    }

    SetPort(save_port);
}

P8(PUBLIC pascal trap, WindowPtr, NewWindow,
   Ptr, window_storage, Rect *, bounds, StringPtr, title,
   BOOLEAN, visible_p, INTEGER, proc_id, WindowPtr, behind,
   BOOLEAN, go_away_flag, LONGINT, ref_con)
{
    WindowPeek w;
    int allocated_p = 0;

    if(!window_storage)
    {
        allocated_p = 1;
        /* Hack for Dark Castle Demo.  They call NewWindow and expect us to
	 create the storage.  Immediately after calling NewWindow they set
	 the windowKind field to dialogKind.  Later they call UpdateDialog
	 on this window and we die a horrible death since we try to refer
	 to a field that isn't present.  I don't know how they get away with
	 it on the Mac, but I doubt that this particular hack will hurt us
	 elsewhere.  At some point we should find out why it works on the
	 Mac and then get rid of this evil hack.  ctm 97/06/01 */
        {
            int size;

#define DARK_CASTLE_HACK
#if defined(DARK_CASTLE_HACK)
// FIXME: #warning DARK_CASTLE_HACK
            if(strncmp((char *)title + 1, "Modal", 5) == 0)
                size = sizeof(DialogRecord);
            else
                size = sizeof *w;
            w = (WindowPeek)_NewPtr_flags(size, false, true);
#else
            w = (WindowPeek)NewPtr(sizeof *w);
#endif
        }
    }
    else
        w = (WindowPeek)window_storage;

    ROMlib_new_window_common(w, allocated_p, 0,
                             bounds, title, visible_p, proc_id, behind,
                             go_away_flag, ref_con);
    return (WindowPtr)w;
}

P8(PUBLIC pascal trap, CWindowPtr, NewCWindow,
   Ptr, window_storage, Rect *, bounds, StringPtr, title,
   BOOLEAN, visible_p, INTEGER, proc_id, CWindowPtr, behind,
   BOOLEAN, go_away_flag, LONGINT, ref_con)
{
    WindowPeek w;
    int allocated_p = 0;

    if(!window_storage)
    {
        allocated_p = 1;
        w = (WindowPeek)NewPtr(sizeof *w);
    }
    else
        w = (WindowPeek)window_storage;

    ROMlib_new_window_common(w, allocated_p, 1,
                             bounds, title, visible_p, proc_id,
                             (WindowPtr)behind,
                             go_away_flag, ref_con);
    return (CWindowPtr)w;
}

typedef windrestype *windrestypeptr;

typedef GUEST<windrestypeptr> *windrestypehand;

P3(PUBLIC pascal trap, CWindowPtr, GetNewCWindow,
   INTEGER, window_id,
   Ptr, window_storage,
   CWindowPtr, behind)
{

    CWindowPtr new_cwin;
    windrestypehand win_res;
    Handle win_ctab_res;
    PaletteHandle palette;

    win_res = (windrestypehand)ROMlib_getrestid(TICK("WIND"), window_id);
    if(win_res == NULL)
        return (CWindowPtr)NULL;

    new_cwin = NewCWindow(window_storage, &HxX(win_res, _wrect),
                          (StringPtr)((char *)&HxX(win_res, _wrect) + 18),
                          Hx(win_res, _wvisible) != 0,
                          Hx(win_res, _wprocid),
                          behind, Hx(win_res, _wgoaway) != 0,
                          CL(*(GUEST<LONGINT> *)((char *)&HxX(win_res, _wrect) + 14)));

    win_ctab_res = ROMlib_getrestid(TICK("wctb"), window_id);
    if(win_ctab_res != NULL)
    {
        ThePortGuard guard(thePort);
        SetWinColor((WindowPtr)new_cwin, (CTabHandle)win_ctab_res);
    }

    /* if this is a color window we must check if a palette
     corresponding to this window id exists */

    palette = GetNewPalette(window_id);
    if(palette)
        NSetPalette((WindowPtr)new_cwin, palette, pmAllUpdates);

    return new_cwin;
}

P3(PUBLIC pascal trap, WindowPtr, GetNewWindow, INTEGER, wid, Ptr, wst,
   WindowPtr, behind)
{
    windrestypehand wh;
    WindowPtr tp;

    wh = (windrestypehand)GetResource(TICK("WIND"), wid);
    if(!wh)
        return (0);
    if(!*wh)
        LoadResource((Handle)wh);
    tp = NewWindow(wst, &(HxX(wh, _wrect)),
                   (StringPtr)((char *)&HxX(wh, _wrect) + 18),
                   Hx(wh, _wvisible) != 0, Hx(wh, _wprocid), (WindowPtr)behind,
                   Hx(wh, _wgoaway) != 0,
                   CL(*(GUEST<LONGINT> *)((char *)&HxX(wh, _wrect) + 14)));
    return (tp);
}

/*
 * NOTE below:  On the Mac+ if after you close a window, the top most
 *		window is non-visible, it will shuffle things.
 */

P1(PUBLIC pascal trap, void, CloseWindow, WindowPtr, w)
{
    WindowPeek wptmp;
    GrafPtr savgp;

    AuxWinHandle saveauxh;
    GUEST<AuxWinHandle> *auxhp;
    ControlHandle c, t;

    if(FrontWindow() == w)
    {
        wptmp = ROMlib_firstvisible((WindowPtr)WINDOW_NEXT_WINDOW(w));
        if(wptmp)
        {
            HiliteWindow((WindowPtr)wptmp, true);
            CurActivate = RM((WindowPtr)wptmp);
        }
    }
    if(MR(WindowList) == (WindowPeek)w)
    {
        WindowList = WINDOW_NEXT_WINDOW_X(w);
        wptmp = MR(WindowList);
    }
    else
    {
        for(wptmp = MR(WindowList);
            wptmp && WINDOW_NEXT_WINDOW(wptmp) != (WindowPeek)w;
            wptmp = WINDOW_NEXT_WINDOW(wptmp))
            ;
        if(wptmp)
            WINDOW_NEXT_WINDOW_X(wptmp) = WINDOW_NEXT_WINDOW_X(w);
    }

    /* notify the palette manager this window has been deleted */
    pm_window_closed(w);

    /* notify the palette manager that the `FrontWindow ()' may have
       changed */
    pm_front_window_maybe_changed_hook();

    /* NOTE: tests have shown that the behaviour implemented below is
       indeed what a Mac+ does */

    /* NOTE: we can't use THEPORT_SAVE_EXCURSION here, becuase of this
       odd behavior */
    savgp = thePort == (GrafPtr)w ? (GrafPtr)MR(wmgr_port) : thePort;
    SetPort(MR(wmgr_port));
    SetClip(MR(GrayRgn));
    PaintBehind(WINDOW_NEXT_WINDOW(w), WINDOW_STRUCT_REGION(w));
    if(WINDOW_NEXT_WINDOW_X(w))
        CalcVisBehind(WINDOW_NEXT_WINDOW(w), WINDOW_STRUCT_REGION(w));

    DisposeRgn(WINDOW_STRUCT_REGION(w));
    DisposeRgn(WINDOW_CONT_REGION(w));
    DisposeRgn(WINDOW_UPDATE_REGION(w));
    DisposHandle((Handle)WINDOW_TITLE(w));
    for(auxhp = (GUEST<AuxWinHandle> *)&AuxWinHead;
        *auxhp && STARH(STARH(auxhp))->awOwner != RM(w);
        auxhp = (GUEST<AuxWinHandle> *)&STARH(STARH(auxhp))->awNext)
        ;
    if(*auxhp)
    {
        saveauxh = STARH(auxhp);
        *auxhp = STARH(STARH(auxhp))->awNext;
        DisposHandle((Handle)saveauxh);
    }

#if defined(NOTAGOODIDEA)
// FIXME: #warning "what the hell does this mean?! DANGER WILL ROBINSON!"
    Cx (*(Ptr *)Cx)(((WindowPeek)w)->windowDefProc) = 0;
    DisposHandle(Cx(((WindowPeek)w)->windowDefProc));
#endif /* NOTAGOODIDEA */

/*
 * TODO: Fix this.  Tests on the mac show that KillControls is called,
 * but just replacing the for loop causes many apps to die.  It could
 * be because some window information that DisposeControl wants is
 * destroyed already, or it could be DisposeControl or KillControl
 * makes some false assumptions.  More tests need to be written.
 */
#if 1
    for(c = WINDOW_CONTROL_LIST(w); c;)
    {
        t = c;
        c = HxP(c, nextControl);
#if 0
	DisposHandle(Hx(t, contrlDefProc));
#endif /* 0 */
        DisposHandle((Handle)t);
    }
#else /* 0 */
    KillControls(w);
#endif /* 0 */

    if(WINDOW_PIC_X(w))
        KillPicture(WINDOW_PIC(w));
    ClosePort((GrafPtr)w);
    SetPort(savgp);
    if(MR(CurActivate) == w)
        CurActivate = 0;
    if(MR(CurDeactive) == w)
        CurDeactive = 0;
    WINDCALL((WindowPtr)w, wDispose, 0);
}

P1(PUBLIC pascal trap, void, DisposeWindow, WindowPtr, w)
{
    CloseWindow(w);
    DisposPtr((Ptr)w);
}

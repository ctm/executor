/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* color window manager functions; introduced in IM-V or beyond */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "WindowMgr.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/wind.h"

using namespace Executor;

/* return a pointer to the auxilary window record
   associated with `w' */

AuxWinHandle Executor::default_aux_win = NULL;

static const RGBColor BLACK_RGB = {
    CWC(0), CWC(0), CWC(0),
};
static const RGBColor WHITE_RGB = {
    CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF),
};

/* bw window colortable */
const ColorSpec default_bw_win_ctab[] = {
    { CWC(wContentColor), WHITE_RGB },
    { CWC(wFrameColor), BLACK_RGB },
    { CWC(wTextColor), BLACK_RGB },
    { CWC(wHiliteColor), BLACK_RGB },
    { CWC(wTitleBarColor), WHITE_RGB },
    { CWC(wHiliteColorLight), WHITE_RGB },
    { CWC(wHiliteColorDark), BLACK_RGB },
    { CWC(wTitleBarLight), WHITE_RGB },
    { CWC(wTitleBarDark), BLACK_RGB },
    { CWC(wDialogLight), BLACK_RGB },
    { CWC(wDialogDark), BLACK_RGB },
    { CWC(wTingeLight), BLACK_RGB },
    { CWC(wTingeDark), BLACK_RGB },
};

static const RGBColor LT_BLUISH_RGB = { CWC((unsigned short)0xCCCC), CWC((unsigned short)0xCCCC), CWC((unsigned short)0xFFFF) };
static const RGBColor DK_BLUISH_RGB = { CWC((unsigned short)0x3333), CWC((unsigned short)0x3333), CWC((unsigned short)0x6666) };
/* the default `bluish' window color table */
const ColorSpec Executor::default_color_win_ctab[] = {
    { CWC(wContentColor), WHITE_RGB },
    { CWC(wFrameColor), BLACK_RGB },
    { CWC(wTextColor), BLACK_RGB },
    { CWC(wHiliteColor), BLACK_RGB },
    { CWC(wTitleBarColor), WHITE_RGB },
    { CWC(wHiliteColorLight), WHITE_RGB },
    { CWC(wHiliteColorDark), BLACK_RGB },
    { CWC(wTitleBarLight), WHITE_RGB },
    { CWC(wTitleBarDark), BLACK_RGB },
    { CWC(wDialogLight), LT_BLUISH_RGB },
    { CWC(wDialogDark), BLACK_RGB },
    { CWC(wTingeLight), LT_BLUISH_RGB },
    { CWC(wTingeDark), DK_BLUISH_RGB },
};
#undef DK_BLUISH_RGB
#undef LT_BLUISH_RGB

static const RGBColor GRAY_RGB = { CWC((unsigned short)0x8888), CWC((unsigned short)0x8888), CWC((unsigned short)0x8888) };
/* stolen from the default colortable excel
   tries to install */
const ColorSpec default_system6_color_win_ctab[] = {
    { CWC(wContentColor), WHITE_RGB },
    { CWC(wFrameColor), BLACK_RGB },
    { CWC(wTextColor), BLACK_RGB },
    { CWC(wHiliteColor), BLACK_RGB },
    { CWC(wTitleBarColor), WHITE_RGB },
};
#undef GRAY_RGB
#undef WHITE_RGB
#undef BLACK_RGB

void Executor::wind_color_init(void)
{
    /* initalize the default window colortable */
    TheZoneGuard guard(LM(SysZone));

    default_aux_win = (AuxWinHandle)NewHandle(sizeof(AuxWinRec));
    HxX(default_aux_win, awNext) = 0;
    HxX(default_aux_win, awOwner) = 0;
    HxX(default_aux_win, awCTable)
        = RM((CTabHandle)GetResource(TICK("wctb"), 0));
    HxX(default_aux_win, dialogCItem) = 0;
    HxX(default_aux_win, awFlags) = 0;
    HxX(default_aux_win, awReserved) = 0;
    HxX(default_aux_win, awRefCon) = 0;
}

GUEST<AuxWinHandle> *
Executor::lookup_aux_win(WindowPtr w)
{
    GUEST<AuxWinHandle> *t;

    for(t = &LM(AuxWinHead);
        *t && HxP(MR(*t), awOwner) != w;
        t = &HxX(MR(*t), awNext))
        ;
    return t;
}

void Executor::C_SetWinColor(WindowPtr w, CTabHandle new_w_ctab)
{
    if(w)
    {
        AuxWinHandle aux_w;

        aux_w = MR(*lookup_aux_win(w));

        if(!aux_w)
        {
            GUEST<AuxWinHandle> t_aux_w;

            t_aux_w = LM(AuxWinHead);
            aux_w = (AuxWinHandle)NewHandle(sizeof(AuxWinRec));
            LM(AuxWinHead) = RM(aux_w);
            HxX(aux_w, awNext) = t_aux_w;
            HxX(aux_w, awOwner) = RM((WindowPtr)w);
            /* FIXME: copy? */
            HxX(aux_w, awCTable) = RM(new_w_ctab);
            HxX(aux_w, dialogCItem) = 0;
            HxX(aux_w, awFlags) = /* CL (proc_id & 0x0F) */ 0;
            HxX(aux_w, awReserved) = 0;
            HxX(aux_w, awRefCon) = 0;
        }
        else
            HxX(aux_w, awCTable) = RM(new_w_ctab);

        if(CGrafPort_p(w))
        {
            ColorSpec *w_ctab_table;
            RGBColor *color;

            w_ctab_table = CTAB_TABLE(new_w_ctab);
            color = &w_ctab_table[wContentColor].rgb;

            CPORT_RGB_BK_COLOR(w) = *color;

            /* pick the best color and store it into window's port's
	     bkColor field */
            PORT_BK_COLOR_X(w) = CL(Color2Index(color));

            if(WINDOW_VISIBLE_X(w))
            {
                ThePortGuard guard(w);
                RgnHandle t;
                t = NewRgn();

                CopyRgn(WINDOW_CONT_REGION(w), t);
                OffsetRgn(t,
                          CW(PORT_BOUNDS(w).left),
                          CW(PORT_BOUNDS(w).top));
                EraseRgn(t);
                DisposeRgn(t);
            }
        }

        if(WINDOW_VISIBLE_X(w))
        {
            /* set the port here, don't just save it while drawing the
	     window
	     FIXME: i'm not sure to what extent this should be the
	     case, at the bare minimum at least here */
            SetPort(MR(wmgr_port));

            SetClip(WINDOW_STRUCT_REGION(w));
            ClipAbove((WindowPeek)w);
            WINDCALL(w, wDraw, 0);
        }
    }
    else
    {
        /* modify the default color window table */
        abort();
    }
}

/* NOTE: all documentation on this function is either vague to the
   point of certain mis-interpretation, or plain wrong.

   it's second argument is `AuxWinHandle *' (or `AuxWinHndl *' in
   Think C speak), and returns the aux window entry for the given
   window; if NULL is passed, an handle to a `AuxWinRec' is placed in
   the aux_w_out argument which contains the default window color
   table

   they named it correctly, but noone got the documentation right */

BOOLEAN Executor::C_GetAuxWin(WindowPtr w, GUEST<AuxWinHandle> *aux_win_out)
{
    if(!w)
    {
        /* return default window color table */
        *aux_win_out = RM(default_aux_win);
        return true;
    }
    else
    {
        GUEST<AuxWinHandle> t;

        t = *lookup_aux_win(w);
        if(t)
        {
            /* return this window's color table */
            *aux_win_out = t;
            return true;
        }
        else
        {
            /* return default window color table */
            *aux_win_out = RM(default_aux_win);
            return false;
        }
    }
}

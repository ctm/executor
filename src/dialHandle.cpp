/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

/* HLock checked by ctm on Mon May 13 17:54:22 MDT 1991 */

#include "rsys/common.h"
#include "EventMgr.h"
#include "DialogMgr.h"
#include "ControlMgr.h"
#include "MemoryMgr.h"
#include "rsys/osutil.h"
#include "OSUtil.h"
#include "ToolboxUtil.h"
#include "ToolboxEvent.h"
#include "Iconutil.h"
#include "PrintMgr.h"
#include "StdFilePkg.h"
#include "FontMgr.h"

#include "rsys/cquick.h"
#include "rsys/ctl.h"
#include "rsys/mman.h"
#include "rsys/itm.h"
#include "rsys/prefs.h"
#include "rsys/pstuff.h"
#include "rsys/stdfile.h"
#include "rsys/print.h"
#include "rsys/hook.h"
#include "rsys/executor.h"
#include "rsys/osevent.h"

using namespace Executor;

BOOLEAN Executor::C_ROMlib_myfilt(DialogPeek dp, EventRecord *evt,
                                  GUEST<INTEGER> *ith) /* IMI-415 */
{
    itmp ip;
    ControlHandle c;
    WriteWhenType when;
    SignedByte flags;

    if(Cx(evt->what) == keyDown && ((Cx(evt->message) & 0xFF) == '\r' || (Cx(evt->message) & 0xFF) == NUMPAD_ENTER))
    {
        ip = ROMlib_dpnotoip(dp, CW(*ith = dp->aDefItem), &flags);
        if(ip && (CB(ip->itmtype) & ctrlItem))
        {
            c = (ControlHandle)MR(ip->itmhand);
            if(Hx(c, contrlVis) && U(Hx(c, contrlHilite)) != INACTIVE)
            {
                if((when = ROMlib_when) != WriteNever)
                    ROMlib_WriteWhen(WriteInBltrgn);
                HiliteControl(c, inButton);
                Delay((LONGINT)5, (LONGINT *)0);
                HiliteControl(c, 0);
                HSetState(MR(((DialogPeek)dp)->items), flags);
                ROMlib_WriteWhen(when);
                /*-->*/ return -1;
            }
        }
        HSetState(MR(((DialogPeek)dp)->items), flags);
    }
    return false;
}

#define DIALOGEVTS \
    (mDownMask | mUpMask | keyDownMask | autoKeyMask | updateMask | activMask)

typedef BOOLEAN (*modalprocp)(DialogPtr dial, EventRecord *evtp,
                                     GUEST<INTEGER> *iht);

#define CALLMODALPROC(dp, evtp, ip, fp2) \
    ROMlib_CALLMODALPROC((dp), (evtp), (ip), (modalprocp)(fp2))

static inline BOOLEAN
ROMlib_CALLMODALPROC(DialogPtr dp,
                     EventRecord *evtp, GUEST<INTEGER> *ip, modalprocp fp)
{
    BOOLEAN retval;

    if(fp == (modalprocp)P_ROMlib_myfilt)
        retval = C_ROMlib_myfilt((DialogPeek)dp, evtp, ip);
    else if(fp == (modalprocp)P_ROMlib_stdffilt)
        retval = C_ROMlib_stdffilt((DialogPeek)dp, evtp, ip);
    else if(fp == (modalprocp)P_ROMlib_numsonlyfilterproc)
        retval = C_ROMlib_numsonlyfilterproc((DialogPeek)dp, evtp, ip);
    else if(fp == (modalprocp)P_ROMlib_stlfilterproc)
        retval = C_ROMlib_stlfilterproc((DialogPeek)dp, evtp, ip);
    else
    {
        ROMlib_hook(dial_modalnumber);
        HOOKSAVEREGS();
        retval = CToPascalCall((void *)fp, ctop(&C_ROMlib_myfilt), dp, evtp, ip);
        HOOKRESTOREREGS();
    }
    return retval;
}

typedef void (*useritemp)(WindowPtr wp, INTEGER item);

#define CALLUSERITEM(dp, inum, temph) \
    ROMlib_CALLUSERITEM(dp, inum, (useritemp)(temph))

static inline void
ROMlib_CALLUSERITEM(DialogPtr dp,
                    INTEGER inum, useritemp temph)
{
    if(temph == (useritemp)P_ROMlib_filebox)
        C_ROMlib_filebox((DialogPeek)dp, inum);
    else if(temph == (useritemp)P_ROMlib_circle_ok)
        C_ROMlib_circle_ok((DialogPeek)dp, inum);
    else if(temph == (useritemp)P_ROMlib_orientation)
        C_ROMlib_orientation((DialogPeek)dp, inum);
    else
    {
        ROMlib_hook(dial_usernumber);
        CToPascalCall((void *)temph, ctop(&C_ROMlib_filebox), dp, inum);
    }
}

#define _FindWindow(pt, wp)             \
    ({                                  \
        GUEST<WindowPtr> __wp;          \
        int retval;                     \
                                        \
        retval = FindWindow(pt, &__wp); \
        *(wp) = MR(__wp);               \
                                        \
        retval;                         \
    })

#define ALLOW_MOVABLE_MODAL /* we've made so many other changes, we \
                   may as well go whole hog */

#if !defined(ALLOW_MOVABLE_MODAL)

/* NOTE: the changes between #if #else and #else #endif should be very
   small, but THEGDEVICE_SAVE_EXCURSION prevents us from using #if, so
   we have a lot of replicated code.  This is scary and should be fixed. */

void Executor::C_ModalDialog(ProcPtr fp, GUEST<INTEGER> *item) /* IMI-415 */
{
    /*
   * The code used to save thePort and restore it at the end of the
   * function, but CALLMODALPROC expects thePort to be unchanged which
   * caused a bug in Macwrite II when size/fontsize... and clicking on
   * a size on the left.
   */
    TheGDeviceGuard guard(MR(MainDevice));

    EventRecord evt;
    DialogPeek dp;
    GUEST<DialogPtr> ndp;
    TEHandle idle;
    ProcPtr fp2;
    Point whereunswapped;
    bool done;

    dp = (DialogPeek)FrontWindow();
    if(dp->window.windowKind != CWC(dialogKind) && CW(dp->window.windowKind) >= 0)
        *item = CWC(-1);
    else
    {
        idle = (Cx(dp->editField) == -1) ? 0 : MR(dp->textH);
        if(fp)
            fp2 = fp;
        else
            fp2 = (ProcPtr)P_ROMlib_myfilt;
        for(done = false; !done;)
        {
            WindowPtr temp_wp;
            bool mousedown_p;
            int mousedown_where;

            if(idle)
                TEIdle(idle);
            GetNextEvent(DIALOGEVTS, &evt);
            whereunswapped.h = CW(evt.where.h);
            whereunswapped.v = CW(evt.where.v);

            mousedown_p = (CW(evt.what) == mouseDown);

            /* dummy initializations to keep gcc happy */
            temp_wp = NULL;
            mousedown_where = inContent;

            if(mousedown_p)
                mousedown_where = _FindWindow(whereunswapped, &temp_wp);

            if(mousedown_p
               && mousedown_where != inMenuBar
               && !(mousedown_where == inContent
                    && temp_wp == (WindowPtr)dp))
                BEEP(1);
            else if(CALLMODALPROC((DialogPtr)dp, &evt, item, fp2))
            {
                /* #### not sure what this means */
                /* above callpascal might need to be `& 0xF0' */
                done = true;
                break;
            }
            else
            {
                if(IsDialogEvent(&evt)
                   && DialogSelect(&evt, &ndp, item))
                    done = true;
            }
        }
    }
}

#else /* defined (ALLOW_MOVABLE_MODAL) */

void Executor::C_ModalDialog(ProcPtr fp, GUEST<INTEGER> *item) /* IMI-415 */
{
    /*
   * The code used to save thePort and restore it at the end of the
   * function, but CALLMODALPROC expects thePort to be unchanged which
   * caused a bug in Macwrite II when size/fontsize... and clicking on
   * a size on the left.
   */

    TheGDeviceGuard guard(MR(MainDevice));

    EventRecord evt;
    DialogPeek dp;
    GUEST<DialogPtr> ndp;
    TEHandle idle;
    ProcPtr fp2;
    Point whereunswapped;
    bool done;

    dp = (DialogPeek)FrontWindow();
    if(dp->window.windowKind != CWC(dialogKind) && CW(dp->window.windowKind) >= 0)
        *item = CWC(-1);
    else
    {
        idle = (Cx(dp->editField) == -1) ? TEHandle(nullptr) : MR(dp->textH);
        if(fp)
            fp2 = fp;
        else
            fp2 = (ProcPtr)P_ROMlib_myfilt;
        for(done = false; !done;)
        {
            WindowPtr temp_wp;
            bool mousedown_p;
            int mousedown_where;

            if(idle)
                TEIdle(idle);
            GetNextEvent(DIALOGEVTS, &evt);
            whereunswapped.h = CW(evt.where.h);
            whereunswapped.v = CW(evt.where.v);

            mousedown_p = (CW(evt.what) == mouseDown);

            /* dummy initializations to keep gcc happy */
            temp_wp = NULL;
            mousedown_where = inContent;

            if(mousedown_p)
                mousedown_where = _FindWindow(whereunswapped, &temp_wp);

            if(CALLMODALPROC((DialogPtr)dp, &evt, item, fp2))
            {
                /* #### not sure what this means */
                /* above callpascal might need to be `& 0xF0' */

                done = true;
                break;
            }
            else if(mousedown_p
                    && mousedown_where != inMenuBar
                    && !(mousedown_where == inContent
                         && temp_wp == (WindowPtr)dp))
                BEEP(1);
            else
            {
                if(IsDialogEvent(&evt)
                   && DialogSelect(&evt, &ndp, item))
                    done = true;
            }
        }
    }
}
#endif

BOOLEAN Executor::C_IsDialogEvent(EventRecord *evt) /* IMI-416 */
{
    GUEST<WindowPtr> wp;
    DialogPeek dp;
    Point p;

    if(evt->what == CWC(activateEvt) || evt->what == CWC(updateEvt))
        /*-->*/ return MR(guest_cast<WindowPeek>(evt->message))->windowKind == CWC(dialogKind);
    dp = (DialogPeek)FrontWindow();
    if(dp && dp->window.windowKind == CWC(dialogKind))
    {
        if(dp->editField != CWC(-1))
            TEIdle(MR(dp->textH));
        p.h = CW(evt->where.h);
        p.v = CW(evt->where.v);
        /*-->*/ return evt->what != CWC(mouseDown) || (FindWindow(p,
                                                                  &wp)
                                                           == inContent
                                                       && MR(wp) == (WindowPtr)dp);
    }
    return false;
}

bool Executor::get_item_style_info(DialogPtr dp, int item_no,
                                   uint16_t *flags_return, item_style_info_t *style_info)
{
    AuxWinHandle aux_win_h;

    aux_win_h = MR(*lookup_aux_win(dp));
    if(aux_win_h && HxX(aux_win_h, dialogCItem))
    {
        Handle items_color_info_h;
        item_color_info_t *items_color_info, *item_color_info;

        items_color_info_h = HxP(aux_win_h, dialogCItem);
        items_color_info = (item_color_info_t *)STARH(items_color_info_h);

        item_color_info = &items_color_info[item_no - 1];
        if(item_color_info->data || item_color_info->offset)
        {
            uint16_t flags;
            int style_info_offset;

            flags = CW(item_color_info->data);
            style_info_offset = CW(item_color_info->offset);

            *style_info = *(item_style_info_t *)((char *)items_color_info
                                                 + style_info_offset);
            if(flags & doFontName)
            {
                char *font_name;

                font_name = (char *)items_color_info + CW(style_info->font);
                GetFNum((StringPtr)font_name, &style_info->font);
            }

            *flags_return = flags;
            return true;
        }
    }
    return false;
}

void Executor::ROMlib_drawiptext(DialogPtr dp, itmp ip, int item_no)
{
    bool restore_draw_state_p = false;
    draw_state_t draw_state;
    uint16_t flags;
    item_style_info_t style_info;
    Rect r;

    if(get_item_style_info(dp, item_no, &flags, &style_info))
    {
        draw_state_save(&draw_state);
        restore_draw_state_p = true;

        if(flags & TEdoFont)
            TextFont(CW(style_info.font));
        if(flags & TEdoFace)
            TextFace(CB(style_info.face));
        if(flags & TEdoSize)
            TextSize(CW(style_info.size));
        if(flags & TEdoColor)
            RGBForeColor(&style_info.foreground);
#if 1
        /* NOTE: this code has been "#if 0"d out since it was first written,
	 but testing on the Mac leads me to believe that this works properly.
	 Perhaps we should only do this if we're pretending to be  running
	 System 7, but other than that possibility, I don't see a downside
	 to including this. */

        if(flags & doBColor)
            RGBBackColor(&style_info.background);
#endif
    }

    r = ip->itmr;
    if(CB(ip->itmtype) & statText)
    {
        Handle nh;
        LONGINT l;
        char subsrc[2], *sp;
        GUEST<Handle> *hp;

        *subsrc = '^';
        sp = subsrc + 1;
        nh = (Handle)MR(ip->itmhand);

        HandToHand(&nh);

        for(*sp = '0', hp = (GUEST<Handle> *)DAStrings;
            *sp != '4'; ++*sp, hp++)
        {
            if(hp)
            {
                for(l = 0; l >= 0;
                    l = Munger(nh, l,
                               (Ptr)subsrc, (LONGINT)2, STARH(STARH(hp)) + 1,
                               (LONGINT)(unsigned char)*STARH(STARH(hp))))
                    ;
            }
        }
        HLock(nh);
        TextBox(STARH(nh), GetHandleSize(nh), &r, teFlushDefault);
        HUnlock(nh);
        DisposHandle(nh);
    }
    else if(CB(ip->itmtype) & editText)
    {
        Handle text_h;

        text_h = MR(ip->itmhand);
        {
            HLockGuard guard(text_h);
            TextBox(STARH(text_h), GetHandleSize(text_h),
                    &r, teFlushDefault);
        }

        PORT_PEN_SIZE(thePort).h = PORT_PEN_SIZE(thePort).v = CWC(1);
        InsetRect(&r, -3, -3);
        FrameRect(&r);
    }

    if(restore_draw_state_p)
        draw_state_restore(&draw_state);
}

void Executor::dialog_draw_item(DialogPtr dp, itmp itemp, int itemno)
{
    if(itemp->itmtype & ctrlItem)
    {
        /* controls will already have been drawn */
    }
    else if(itemp->itmtype & (statText | editText))
    {
        Rect r;

        // FIXME: #warning This fix helps Energy Scheming, but we really should find out the
        // FIXME: #warning exact semantics for when we should try to draw items, different
        // FIXME: #warning item types may have different behaviors, we also might want to
        // FIXME: #warning look at visRgn and clipRgn.  BTW, we should also test to see
        // FIXME: #warning whether SectRect will really write to location 0

        if(SectRect(&itemp->itmr, &dp->portRect, &r))
            ROMlib_drawiptext(dp, itemp, itemno);
    }
    else if(itemp->itmtype & iconItem)
    {
        Handle icon;

        icon = MR(itemp->itmhand);
        if(CICON_P(icon))
            PlotCIcon(&itemp->itmr, (CIconHandle)icon);
        else
            PlotIcon(&itemp->itmr, icon);
    }
    else if(itemp->itmtype & picItem)
    {
        DrawPicture((PicHandle)MR(itemp->itmhand), &itemp->itmr);
    }
    else
    {
        Handle h;

        /* useritem */
        h = MR(itemp->itmhand);
        if(h)
            CALLUSERITEM(dp, itemno, h);
    }
}

/* #### look into having DrawDialog not draw stuff that can't be seen */

void Executor::C_DrawDialog(DialogPtr dp) /* IMI-418 */
{
    GUEST<INTEGER> *intp;
    INTEGER i, inum;
    itmp ip;
    GrafPtr gp;
    SignedByte state;

    if(dp)
    {
        gp = thePort;
        SetPort((GrafPtr)dp);
        if(Cx(((DialogPeek)dp)->editField) != -1)
            TEDeactivate(MR(((DialogPeek)dp)->textH));
        DrawControls((WindowPtr)dp);
        state = HGetState(MR(((DialogPeek)dp)->items));
        HSetState(MR(((DialogPeek)dp)->items), state | LOCKBIT);
        intp = (GUEST<INTEGER> *)STARH(MR(((DialogPeek)dp)->items));
        ip = (itmp)(intp + 1);
        for(i = Cx(*intp), inum = 1; i-- >= 0; inum++, BUMPIP(ip))
        {
            dialog_draw_item(dp, ip, inum);
        }
        if(Cx(((DialogPeek)dp)->editField) != -1)
            TEActivate(MR(((DialogPeek)dp)->textH));
        HSetState(MR(((DialogPeek)dp)->items), state);
        SetPort(gp);
    }
}

INTEGER Executor::C_FindDItem(DialogPtr dp, Point pt) /* IMIV-60 */
{
    GUEST<INTEGER> *intp;
    INTEGER i, inum;
    itmp ip;

    intp = (GUEST<INTEGER> *)STARH(MR(((DialogPeek)dp)->items));
    ip = (itmp)(intp + 1);
    for(i = Cx(*intp), inum = 0; i-- >= 0; inum++, BUMPIP(ip))
        if(PtInRect(pt, &ip->itmr))
            /*-->*/ return inum;
    return -1;
}

void Executor::C_UpdtDialog(DialogPtr dp, RgnHandle rgn) /* IMIV-60 */
{
    GUEST<INTEGER> *intp;
    INTEGER i, inum;
    itmp ip;
    GrafPtr gp;
    SignedByte state;

    gp = thePort;
    SetPort((GrafPtr)dp);
    ShowWindow((WindowPtr)dp);
    DrawControls((WindowPtr)dp);
    state = HGetState(MR(((DialogPeek)dp)->items));
    HSetState(MR(((DialogPeek)dp)->items), state | LOCKBIT);
    intp = (GUEST<INTEGER> *)STARH(MR(((DialogPeek)dp)->items));
    ip = (itmp)(intp + 1);
    for(i = Cx(*intp), inum = 1; i-- >= 0; inum++, BUMPIP(ip))
    {
        if(RectInRgn(&ip->itmr, rgn))
        {
            dialog_draw_item(dp, ip, inum);
        }
    }
    HSetState(MR(((DialogPeek)dp)->items), state);
    SetPort(gp);
}

BOOLEAN Executor::C_DialogSelect(EventRecord *evt, GUEST<DialogPtr> *dpp,
                                 GUEST<INTEGER> *itemp) /* IMI-417 */
{
    DialogPeek dp;
    Byte c;
    itmp ip;
    GUEST<INTEGER> *intp;
    INTEGER i, iend;
    Point localp;
    GUEST<Point> glocalp;
    GrafPtr gp;
    BOOLEAN itemenabled, retval;
    SignedByte flags;

    dp = (DialogPeek)FrontWindow();
    retval = false;
    *itemp = CWC(-1);
    switch(Cx(evt->what))
    {
        case mouseDown:
            glocalp = evt->where;
            gp = thePort;
            SetPort((GrafPtr)dp);
            GlobalToLocal(&glocalp);
            localp = glocalp.get();
            SetPort(gp);
            intp = (GUEST<INTEGER> *)STARH(MR(dp->items));
            iend = Cx(*intp) + 2;
            ip = (itmp)(intp + 1);
            for(i = 0;
                ++i != iend && !PtInRect(localp, &(ip->itmr));
                BUMPIP(ip))
                ;
            itemenabled = !(CB(ip->itmtype) & itemDisable);
            if(i == iend)
                break;
            if(CB(ip->itmtype) & editText)
            {
                if(Cx(dp->editField) != i - 1)
                    ROMlib_dpntoteh(dp, i);
                TEClick(localp, (Cx(evt->modifiers) & shiftKey) ? true : false,
                        MR(dp->textH));
            }
            else if(CB(ip->itmtype) & ctrlItem)
            {
                ControlHandle c;

                c = (ControlHandle)MR(ip->itmhand);
                if(CTL_HILITE(c) == INACTIVE
                   || !TrackControl(c, localp,
                                    CTL_ACTION(c)))
                    break;
            }
            if(itemenabled)
            {
                *itemp = CW(i);
                retval = true;
                break;
            }
            break;
        case keyDown:
        case autoKey:
            if(Cx(dp->editField) == -1)
                break;
            c = Cx(evt->message) & 0xff;
            switch(c)
            {
                case '\t':
                    ROMlib_dpntoteh(dp, 0);
                    TESetSelect((LONGINT)0, (LONGINT)32767,
                                DIALOG_TEXTH(dp));
                    break;
                default:
                    TEKey(c, DIALOG_TEXTH(dp));
            }
            *itemp = CW(CW(dp->editField) + 1);
            ip = ROMlib_dpnotoip(dp, CW(*itemp), &flags);
            if(ip)
                retval = !(CB(ip->itmtype) & itemDisable);
            else
            {
                warning_unexpected("couldn't resolve editField -- dp = %p, "
                                   "CW (*itemp) = %d",
                                   dp, CW(*itemp));
                retval = false;
            }
            HSetState(MR(((DialogPeek)dp)->items), flags);
            break;
        case updateEvt:
            dp = MR(guest_cast<DialogPeek>(evt->message));
            BeginUpdate((WindowPtr)dp);
            DrawDialog((DialogPtr)dp);
            if(dp->editField != CWC(-1))
                TEUpdate(&dp->window.port.portRect, MR(dp->textH));
            EndUpdate((WindowPtr)dp);
            break;
        case activateEvt:
            dp = MR(guest_cast<DialogPeek>(evt->message));
            if(dp->editField != CWC(-1))
            {
                if(Cx(evt->modifiers) & activeFlag)
                    TEActivate(MR(dp->textH));
                else
                    TEDeactivate(MR(dp->textH));
            }
            break;
    }
    *dpp = RM((DialogPtr)dp);
    return retval;
}

void Executor::DlgCut(DialogPtr dp) /* IMI-418 */
{
    if((((DialogPeek)dp)->editField) != CWC(-1))
        TECut(MR(((DialogPeek)dp)->textH));
}

void Executor::DlgCopy(DialogPtr dp) /* IMI-418 */
{
    if((((DialogPeek)dp)->editField) != CWC(-1))
        TECopy(MR(((DialogPeek)dp)->textH));
}

void Executor::DlgPaste(DialogPtr dp) /* IMI-418 */
{
    if((((DialogPeek)dp)->editField) != CWC(-1))
        TEPaste(MR(((DialogPeek)dp)->textH));
}

void Executor::DlgDelete(DialogPtr dp) /* IMI-418 */
{
    if((((DialogPeek)dp)->editField) != CWC(-1))
        TEDelete(MR(((DialogPeek)dp)->textH));
}

void Executor::BEEPER(INTEGER n)
{
    if(DABeeper)
    {
        if((void (*)(INTEGER))MR(DABeeper) == P_ROMlib_mysound)
            C_ROMlib_mysound((n));
        else
        {
            HOOKSAVEREGS();
            ROMlib_hook(dial_soundprocnumber);
            Executor::CToPascalCall((void *)(soundprocp)MR(DABeeper), ctop(&C_ROMlib_mysound), n);
            HOOKRESTOREREGS();
        }
    }
}

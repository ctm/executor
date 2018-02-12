/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "EventMgr.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/ctl.h"
#include "rsys/resource.h"
#include "rsys/hook.h"
#include "rsys/functions.impl.h"

/* cheat, and make this a plain ctab, so we can use our old accessor
   macros */

using namespace Executor;

AuxCtlHandle Executor::default_aux_ctl;

#define BLACK_RGB         \
    {                     \
        CWC(0)            \
        , CWC(0), CWC(0), \
    }
#define WHITE_RGB                                                   \
    {                                                               \
        CWC((unsigned short)0xFFFF)                                 \
        , CWC((unsigned short)0xFFFF), CWC((unsigned short)0xFFFF), \
    }

#define LT_BLUISH_RGB                                              \
    {                                                              \
        CWC((unsigned short)0xCCCC)                                \
        , CWC((unsigned short)0xCCCC), CWC((unsigned short)0xFFFF) \
    }
#define DK_BLUISH_RGB              \
    {                              \
        CWC(0x3333)                \
        , CWC(0x3333), CWC(0x6666) \
    }

#define DK_GRAY                                                    \
    {                                                              \
        CWC((unsigned short)0x5555)                                \
        , CWC((unsigned short)0x5555), CWC((unsigned short)0x5555) \
    }

const ColorSpec Executor::default_ctl_colors[] = {
    { CWC(cFrameColor), BLACK_RGB },
    { CWC(cBodyColor), WHITE_RGB },
    { CWC(cTextColor), BLACK_RGB },
    { CWC(cThumbColor), WHITE_RGB },
    { CWC(4), DK_GRAY },
    { CWC(cArrowsColorLight), WHITE_RGB },
    { CWC(cArrowsColorDark), BLACK_RGB },
    { CWC(cThumbLight), WHITE_RGB },
    { CWC(cThumbDark), BLACK_RGB },
    /* used same as the w... color component */
    { CWC(cHiliteLight), WHITE_RGB },
    { CWC(cHiliteDark), BLACK_RGB },
    { CWC(cTitleBarLight), WHITE_RGB },
    { CWC(cTitleBarDark), BLACK_RGB },
    { CWC(cTingeLight), LT_BLUISH_RGB },
    { CWC(cTingeDark), DK_BLUISH_RGB },
};

#define KEEP_DEFAULT_CTL_CTAB_AROUND_FOR_OLD_SYSTEM_FILE_USERS

#if defined(KEEP_DEFAULT_CTL_CTAB_AROUND_FOR_OLD_SYSTEM_FILE_USERS)
CTabHandle Executor::default_ctl_ctab;
#endif

void Executor::ctl_color_init(void)
{
    default_aux_ctl = (AuxCtlHandle)NewHandle(sizeof(AuxCtlRec));

#if defined(KEEP_DEFAULT_CTL_CTAB_AROUND_FOR_OLD_SYSTEM_FILE_USERS)
    default_ctl_ctab = (CTabHandle)ROMlib_getrestid(TICK("cctb"), 0);

    if(!default_ctl_ctab)
    {
        default_ctl_ctab
            = (CTabHandle)NewHandle(CTAB_STORAGE_FOR_SIZE(14));
        CTAB_SIZE_X(default_ctl_ctab) = CWC(14);
        CTAB_SEED_X(default_ctl_ctab) = CLC(0);
        CTAB_FLAGS_X(default_ctl_ctab) = CWC(0);
        memcpy(&CTAB_TABLE(default_ctl_ctab)[0],
               &default_ctl_colors[0],
               15 * sizeof default_ctl_colors[0]);
    }
#endif

    HxX(default_aux_ctl, acNext) = CLC_NULL;
    HxX(default_aux_ctl, acOwner) = CLC_NULL;
    HxX(default_aux_ctl, acCTable) = RM((CCTabHandle)GetResource(TICK("cctb"), 0));
    HxX(default_aux_ctl, acFlags) = CWC(0);
    HxX(default_aux_ctl, acReserved) = CLC(0);
    HxX(default_aux_ctl, acRefCon) = CLC(0);
}

GUEST<AuxCtlHandle> *
Executor::lookup_aux_ctl(ControlHandle ctl)
{
    GUEST<AuxCtlHandle> *t;

    for(t = &LM(AuxCtlHead);
        *t && HxP(MR(*t), acOwner) != ctl;
        t = &HxX(MR(*t), acNext))
        ;
    return t;
}

void Executor::C_SetCRefCon(ControlHandle c, LONGINT data) /* IMI-327 */
{
    HxX(c, contrlRfCon) = CL(data);
}

LONGINT Executor::C_GetCRefCon(ControlHandle c) /* IMI-327 */
{
    return Hx(c, contrlRfCon);
}

void Executor::C_SetCtlAction(ControlHandle c, ControlActionUPP a) /* IMI-328 */
{
    if(a != (ControlActionUPP)-1)
        HxX(c, contrlAction) = RM(a);
    else
        HxX(c, contrlAction) = guest_cast<ControlActionUPP>(CLC(-1));
}

ControlActionUPP Executor::C_GetCtlAction(ControlHandle c) /* IMI-328 */
{
    return HxP(c, contrlAction);
}

INTEGER Executor::C_GetCVariant(ControlHandle c) /* IMV-222 */
{
    AuxCtlHandle h;

    for(h = MR(LM(AuxCtlHead)); h != 0 && HxP(h, acOwner) != c; h = HxP(h, acNext))
        ;
    return h != 0 ? Hx(h, acFlags) : (INTEGER)0;
}

/* according to IM-MTE; this has been renamed
   `GetAuxiliaryControlRecord ()', possibly because of the
   inconsistency below, i can only assume they have the same trap word */
BOOLEAN Executor::C_GetAuxCtl(ControlHandle ctl,
                              GUEST<AuxCtlHandle> *aux_ctl) /* IMV-222 */
{
    /* according to testing on the Mac+
     `GetAuxCtl ()' returns false (not true) and leaves
     aux_ctl untouched; this is not the case of later
     mac version, such as the color classic v7.1 */

    if(!ctl)
    {
        *aux_ctl = RM(default_aux_ctl);
        return true;
    }
    else
    {
        GUEST<AuxCtlHandle> t;

        t = *lookup_aux_ctl(ctl);
        if(t)
        {
            *aux_ctl = t;
            return false;
        }
        else
        {
            *aux_ctl = RM(default_aux_ctl);
            return true;
        }
    }
}

int32_t Executor::ROMlib_ctlcall(ControlHandle c, int16_t i, int32_t l)
{
    Handle defproc;
    int32_t retval;
    ctlfuncp cp;

    defproc = CTL_DEFPROC(c);

    if(*defproc == NULL)
        LoadResource(defproc);

    cp = (ctlfuncp)STARH(defproc);

    if(cp == &cdef0)
        retval = cdef0(VAR(c), c, i, l);
    else if(cp == &cdef16)
        retval = cdef16(VAR(c), c, i, l);
    else if(cp == &cdef1008)
        retval = cdef1008(VAR(c), c, i, l);
    else
    {
        ROMlib_hook(ctl_cdefnumber);
        HLockGuard guard(defproc);
        retval = cp(VAR(c), c, i, l);
    }

    return retval;
}

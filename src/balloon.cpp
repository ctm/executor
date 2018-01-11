/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "HelpMgr.h"

using namespace Executor;

BOOLEAN Executor::C_HMGetBalloons()
{
    warning_unimplemented(NULL_STRING);
    return false;
}

OSErr Executor::C_HMSetBalloons(BOOLEAN flag)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

BOOLEAN Executor::C_HMIsBalloon()
{
    warning_unimplemented(NULL_STRING);
    return false;
}

OSErr Executor::C_HMShowBalloon(HMMessageRecord *msgp, Point tip,
                                RectPtr alternaterectp, Ptr tipprocptr,
                                INTEGER proc, INTEGER variant, INTEGER method)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMShowMenuBalloon(INTEGER item, INTEGER menuid,
                                    LONGINT flags, LONGINT itemreserved,
                                    Point tip, RectPtr alternaterectp,
                                    Ptr tipproc, INTEGER proc, INTEGER variant)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMRemoveBalloon()
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetHelpMenuHandle(GUEST<MenuHandle> *mhp)
{
    warning_unimplemented(NULL_STRING);
    *mhp = nullptr;
    return noErr;
}

OSErr Executor::C_HMGetFont(GUEST<INTEGER> *fontp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetFontSize(GUEST<INTEGER> *sizep)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMSetFont(INTEGER font)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMSetFontSize(INTEGER size)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMSetDialogResID(INTEGER resid)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetDialogResID(GUEST<INTEGER> *residp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMSetMenuResID(INTEGER menuid, INTEGER resid)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetMenuResID(GUEST<INTEGER> *menuidp,
                                 GUEST<INTEGER> *residp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMScanTemplateItems(INTEGER whichid, INTEGER whicresfile,
                                      ResType whictype)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMBalloonRect(HMMessageRecord *messp, Rect *rectp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMBalloonPict(HMMessageRecord *messp, GUEST<PicHandle> *pictp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetBalloonWindow(GUEST<WindowPtr> *windowpp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMExtractHelpMsg(ResType type, INTEGER resid, INTEGER msg,
                                   INTEGER state, HMMessageRecord *helpmsgp)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

OSErr Executor::C_HMGetIndHelpMsg(ResType type, INTEGER resid, INTEGER msg,
                                  INTEGER state, GUEST<LONGINT> *options,
                                  Point tip, Rect *altrectp,
                                  GUEST<INTEGER> *theprocp,
                                  GUEST<INTEGER> *variantp,
                                  HMMessageRecord *helpmsgp,
                                  GUEST<INTEGER> *count)
{
    warning_unimplemented(NULL_STRING);
    return hmHelpManagerNotInited;
}

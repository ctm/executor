#if !defined(_HELPMGR_H_)
#define _HELPMGR_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "TextEdit.h"
#include "MenuMgr.h"
#include "WindowMgr.h"

namespace Executor
{
struct HMStringResType
{
    GUEST_STRUCT;
    GUEST<INTEGER> hmmResID;
    GUEST<INTEGER> hmmIndex;
};

typedef struct HMMessageRecord
{
    GUEST_STRUCT;
    GUEST<INTEGER> hmmHelpType;
    union {
        GUEST<Str255> hmmString;
        GUEST<INTEGER> hmmPict;
        GUEST<HMStringResType> hmmStringRes;
        GUEST<TEHandle> hmmTEHandle;
        GUEST<PicHandle> hmmPictHandle;
        GUEST<INTEGER> hmmTERes;
        GUEST<INTEGER> hmmSTRRes;
    } u;
} HMMessageRecord, *HMMessageRecPtr;

enum
{
    hmHelpDisabled = -850,
    hmBalloonAborted = -853,
    hmSameAsLastBalloon = -854,
    hmHelpManagerNotInited = -855,
    hmSkippedBalloon = -857,
    hmWrongVersion = -858,
    hmUnknownHelpType = -859,
    hmOperationUnsupported = -861,
    hmNoBalloonUp = -862,
    hmCloseViewActive = -863
};

extern BOOLEAN C_HMGetBalloons(void);
PASCAL_SUBTRAP(HMGetBalloons, 0xA830, Pack14);

extern OSErr C_HMSetBalloons(BOOLEAN flag);
PASCAL_SUBTRAP(HMSetBalloons, 0xA830, Pack14);

extern BOOLEAN C_HMIsBalloon(void);
PASCAL_SUBTRAP(HMIsBalloon, 0xA830, Pack14);

extern OSErr C_HMShowBalloon(HMMessageRecord *msgp, Point tip,
                                  RectPtr alternaterectp, Ptr tipprocptr,
                                  INTEGER proc, INTEGER variant,
                                  INTEGER method);
PASCAL_SUBTRAP(HMShowBalloon, 0xA830, Pack14);

extern OSErr C_HMShowMenuBalloon(INTEGER item, INTEGER menuid,
                                      LONGINT flags,
                                      LONGINT itemreserved, Point tip,
                                      RectPtr alternaterectp,
                                      Ptr tipproc, INTEGER proc,
                                      INTEGER variant);
PASCAL_SUBTRAP(HMShowMenuBalloon, 0xA830, Pack14);

extern OSErr C_HMRemoveBalloon(void);
PASCAL_SUBTRAP(HMRemoveBalloon, 0xA830, Pack14);

extern OSErr C_HMGetHelpMenuHandle(GUEST<MenuHandle> *mhp);
PASCAL_SUBTRAP(HMGetHelpMenuHandle, 0xA830, Pack14);

extern OSErr C_HMGetFont(GUEST<INTEGER> *fontp);
PASCAL_SUBTRAP(HMGetFont, 0xA830, Pack14);

extern OSErr C_HMGetFontSize(GUEST<INTEGER> *sizep);
PASCAL_SUBTRAP(HMGetFontSize, 0xA830, Pack14);

extern OSErr C_HMSetFont(INTEGER font);
PASCAL_SUBTRAP(HMSetFont, 0xA830, Pack14);

extern OSErr C_HMSetFontSize(INTEGER size);
PASCAL_SUBTRAP(HMSetFontSize, 0xA830, Pack14);

extern OSErr C_HMSetDialogResID(INTEGER resid);
PASCAL_SUBTRAP(HMSetDialogResID, 0xA830, Pack14);

extern OSErr C_HMGetDialogResID(GUEST<INTEGER> *residp);
PASCAL_SUBTRAP(HMGetDialogResID, 0xA830, Pack14);

extern OSErr C_HMSetMenuResID(INTEGER menuid, INTEGER resid);
PASCAL_SUBTRAP(HMSetMenuResID, 0xA830, Pack14);

extern OSErr C_HMGetMenuResID(GUEST<INTEGER> *menuidp, GUEST<INTEGER> *residp);
PASCAL_SUBTRAP(HMGetMenuResID, 0xA830, Pack14);

extern OSErr C_HMScanTemplateItems(INTEGER whichid,
                                        INTEGER whicresfile,
                                        ResType whictype);
PASCAL_SUBTRAP(HMScanTemplateItems, 0xA830, Pack14);

extern OSErr C_HMBalloonRect(HMMessageRecord *messp, Rect *rectp);
PASCAL_SUBTRAP(HMBalloonRect, 0xA830, Pack14);

extern OSErr C_HMBalloonPict(HMMessageRecord *messp,
                                  GUEST<PicHandle> *pictp);
PASCAL_SUBTRAP(HMBalloonPict, 0xA830, Pack14);

extern OSErr C_HMGetBalloonWindow(GUEST<WindowPtr> *windowpp);
PASCAL_SUBTRAP(HMGetBalloonWindow, 0xA830, Pack14);

extern OSErr C_HMExtractHelpMsg(ResType type, INTEGER resid,
                                     INTEGER msg, INTEGER state,
                                     HMMessageRecord *helpmsgp);
PASCAL_SUBTRAP(HMExtractHelpMsg, 0xA830, Pack14);

extern OSErr C_HMGetIndHelpMsg(ResType type, INTEGER resid,
                               INTEGER msg, INTEGER state,
                               GUEST<LONGINT> *options, Point tip,
                               Rect *altrectp, GUEST<INTEGER> *theprocp,
                               GUEST<INTEGER> *variantp,
                               HMMessageRecord *helpmsgp,
                               GUEST<INTEGER> *count);

PASCAL_SUBTRAP(HMGetIndHelpMsg, 0xA830, Pack14);
}

#endif /* !_HELPMGR_H_ */

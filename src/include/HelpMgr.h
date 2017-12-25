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

extern trap BOOLEAN C_HMGetBalloons(void);

extern trap OSErr C_HMSetBalloons(BOOLEAN flag);

extern trap BOOLEAN C_HMIsBalloon(void);

extern trap OSErr C_HMShowBalloon(HMMessageRecord *msgp, Point tip,
                                  RectPtr alternaterectp, Ptr tipprocptr,
                                  INTEGER proc, INTEGER variant,
                                  INTEGER method);

extern trap OSErr C_HMShowMenuBalloon(INTEGER item, INTEGER menuid,
                                      LONGINT flags,
                                      LONGINT itemreserved, Point tip,
                                      RectPtr alternaterectp,
                                      Ptr tipproc, INTEGER proc,
                                      INTEGER variant);

extern trap OSErr C_HMRemoveBalloon(void);

extern trap OSErr C_HMGetHelpMenuHandle(GUEST<MenuHandle> *mhp);

extern trap OSErr C_HMGetFont(GUEST<INTEGER> *fontp);

extern trap OSErr C_HMGetFontSize(GUEST<INTEGER> *sizep);

extern trap OSErr C_HMSetFont(INTEGER font);

extern trap OSErr C_HMSetFontSize(INTEGER size);

extern trap OSErr C_HMSetDialogResID(INTEGER resid);

extern trap OSErr C_HMGetDialogResID(GUEST<INTEGER> *residp);

extern trap OSErr C_HMSetMenuResID(INTEGER menuid, INTEGER resid);

extern trap OSErr C_HMGetMenuResID(GUEST<INTEGER> *menuidp, GUEST<INTEGER> *residp);

extern trap OSErr C_HMScanTemplateItems(INTEGER whichid,
                                        INTEGER whicresfile,
                                        ResType whictype);

extern trap OSErr C_HMBalloonRect(HMMessageRecord *messp, Rect *rectp);

extern trap OSErr C_HMBalloonPict(HMMessageRecord *messp,
                                  GUEST<PicHandle> *pictp);

extern trap OSErr C_HMGetBalloonWindow(GUEST<WindowPtr> *windowpp);

extern trap OSErr C_HMExtractHelpMsg(ResType type, INTEGER resid,
                                     INTEGER msg, INTEGER state,
                                     HMMessageRecord *helpmsgp);

extern OSErr HMGetIndHelpMsg(ResType type, INTEGER resid,
                             INTEGER msg, INTEGER state,
                             GUEST<LONGINT> *options, Point tip,
                             Rect *altrectp, GUEST<INTEGER> *theprocp,
                             INTEGER *variantp,
                             HMMessageRecord *helpmsgp,
                             GUEST<INTEGER> *count);
}

#endif /* !_HELPMGR_H_ */

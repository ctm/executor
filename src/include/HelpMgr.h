#if !defined (_HELPMGR_H_)
#define _HELPMGR_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: HelpMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "TextEdit.h"
#include "MenuMgr.h"
#include "WindowMgr.h"

namespace Executor {
typedef struct PACKED HMStringResType
{
  INTEGER hmmResID;
  INTEGER hmmIndex;
} HMStringResType;

typedef struct PACKED HMMessageRecord
{
  INTEGER hmmHelpType;
  union
  {
    Str255 hmmString;
    INTEGER hmmPict;
    HMStringResType hmmStringRes;
    PACKED_MEMBER(TEHandle, hmmTEHandle);
    PACKED_MEMBER(PicHandle, hmmPictHandle);
    INTEGER hmmTERes;
    INTEGER hmmSTRRes;
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

extern trap BOOLEAN C_HMGetBalloons (void);

extern trap OSErr C_HMSetBalloons (BOOLEAN flag);

extern trap BOOLEAN C_HMIsBalloon (void);

extern trap OSErr C_HMShowBalloon (HMMessageRecord *msgp, Point tip,
					RectPtr alternaterectp, Ptr tipprocptr,
					INTEGER proc, INTEGER variant,
					INTEGER method);

extern trap OSErr C_HMShowMenuBalloon (INTEGER item, INTEGER menuid,
					    LONGINT flags,
					    LONGINT itemreserved, Point tip,
					    RectPtr alternaterectp,
					    Ptr tipproc, INTEGER proc,
					    INTEGER variant);

extern trap OSErr C_HMRemoveBalloon (void);

extern trap OSErr C_HMGetHelpMenuHandle (MenuHandle *mhp);

extern trap OSErr C_HMGetFont (INTEGER *fontp);

extern trap OSErr C_HMGetFontSize (INTEGER *sizep);

extern trap OSErr C_HMSetFont (INTEGER font);

extern trap OSErr C_HMSetFontSize (INTEGER size);

extern trap OSErr C_HMSetDialogResID (INTEGER resid);

extern trap OSErr C_HMGetDialogResID (INTEGER *residp);

extern trap OSErr C_HMSetMenuResID (INTEGER menuid, INTEGER resid);

extern trap OSErr C_HMGetMenuResID (INTEGER *menuidp, INTEGER *residp);

extern trap OSErr C_HMScanTemplateItems (INTEGER whichid,
					      INTEGER whicresfile,
					      ResType whictype);

extern trap OSErr C_HMBalloonRect (HMMessageRecord *messp, Rect *rectp);

extern trap OSErr C_HMBalloonPict (HMMessageRecord *messp,
					PicHandle *pictp);

extern trap OSErr C_HMGetBalloonWindow (WindowPtr *windowpp);

extern trap OSErr C_HMExtractHelpMsg (ResType type, INTEGER resid,
					   INTEGER msg, INTEGER state,
					   HMMessageRecord *helpmsgp);

extern OSErr HMGetIndHelpMsg (ResType type, INTEGER resid,
			      INTEGER msg, INTEGER state,
			      LONGINT *options, Point tip,
			      Rect *altrectp, INTEGER *theprocp,
			      INTEGER *variantp,
			      HMMessageRecord *helpmsgp,
			      INTEGER *count);
}

#endif /* !_HELPMGR_H_ */

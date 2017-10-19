#if !defined (__NOTIFYMGR__)
#define __NOTIFYMGR__

/*
 * Copyright 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: NotifyMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
    /* value of -1 means remove queue element
                           automatically */
typedef struct NMRec { GUEST_STRUCT;
    GUEST< QElemPtr> qLink;
    GUEST< INTEGER> qType;
    GUEST< INTEGER> nmFlags;
    GUEST< LONGINT> nmPrivate;
    GUEST< INTEGER> nmReserved;
    GUEST< INTEGER> nmMark;
    GUEST< Handle> nmIcon;
    GUEST< Handle> nmSound;
    GUEST< StringPtr> nmStr;
    GUEST< ProcPtr> nmResp;    /* pascal void myresponse(NMRecPtr foo) */
    GUEST< LONGINT> nmRefCon;
} *NMRecPtr;

extern trap OSErrRET NMInstall( NMRecPtr nmptr );
extern trap OSErrRET NMRemove( NMRecPtr nmptr );
}
#endif /* __NOTIFYMGR__ */

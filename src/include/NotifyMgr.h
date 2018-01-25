#if !defined(__NOTIFYMGR__)
#define __NOTIFYMGR__

/*
 * Copyright 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
/* value of -1 means remove queue element
                           automatically */
typedef struct NMRec
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<INTEGER> nmFlags;
    GUEST<LONGINT> nmPrivate;
    GUEST<INTEGER> nmReserved;
    GUEST<INTEGER> nmMark;
    GUEST<Handle> nmIcon;
    GUEST<Handle> nmSound;
    GUEST<StringPtr> nmStr;
    GUEST<ProcPtr> nmResp; /* void myresponse(NMRecPtr foo) */
    GUEST<LONGINT> nmRefCon;
} * NMRecPtr;

extern OSErr NMInstall(NMRecPtr nmptr);
extern OSErr NMRemove(NMRecPtr nmptr);
}
#endif /* __NOTIFYMGR__ */

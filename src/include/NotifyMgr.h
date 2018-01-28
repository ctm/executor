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
REGISTER_TRAP2(NMInstall, 0xA05E, D0(A0));
extern OSErr NMRemove(NMRecPtr nmptr);
REGISTER_TRAP2(NMRemove, 0xA05F, D0(A0));
}
#endif /* __NOTIFYMGR__ */

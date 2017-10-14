#if !defined (__COMMTOOL__)
#define __COMMTOOL__

/*
 * Copyright 1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: CommTool.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
typedef struct CRMRec : GuestStruct {
    GUEST< QElemPtr> qLink;
    GUEST< INTEGER> qType;
    GUEST< INTEGER> crmVersion;
    GUEST< LONGINT> crmPrivate;
    GUEST< INTEGER> crmReserved;
    GUEST< LONGINT> crmDeviceType;
    GUEST< LONGINT> crmDeviceID;
    GUEST< LONGINT> crmAttributes;
    GUEST< LONGINT> crmStatus;
    GUEST< LONGINT> crmRefCon;
} *CRMRecPtr;

typedef OSErr CRMErr;

enum { crmGenericError = -1, crmNoErr = 0 };
enum { curCRMVersion = 1 };
enum { crmType = 9 };
enum { crmRecVersion = 1 };
enum { curCRMSerRecVer = 0 };
enum { crmSerialDevice = 1 };

typedef struct CRMSerialRecord : GuestStruct {
    GUEST< INTEGER> version;
    GUEST< StringHandle> inputDriverName;
    GUEST< StringHandle> outputDriverName;
    GUEST< StringHandle> name;
    GUEST< Handle> deviceIcon;
    GUEST< LONGINT> ratedSpeed;
    GUEST< LONGINT> maxSpeed;
    GUEST< LONGINT> reserved;
} *CRMSerialPtr;

extern INTEGER CRMGetCRMVersion (void);

extern QHdrPtr CRMGetHeader (void);

extern void CRMInstall (QElemPtr);

extern OSErr CRMRemove (QElemPtr);

extern QElemPtr CRMSearch (QElemPtr);

extern CRMErr InitCRM (void);
}
#endif

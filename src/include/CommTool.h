#if !defined (__COMMTOOL__)
#define __COMMTOOL__

/*
 * Copyright 1999 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: CommTool.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct
{
  QElemPtr qLink PACKED_P;
  INTEGER qType PACKED;
  INTEGER crmVersion PACKED;
  LONGINT crmPrivate PACKED;
  INTEGER crmReserved PACKED;
  LONGINT crmDeviceType PACKED;
  LONGINT crmDeviceID PACKED;
  LONGINT crmAttributes PACKED;
  LONGINT crmStatus PACKED;
  LONGINT crmRefCon PACKED;
}
CRMRec, *CRMRecPtr;

typedef OSErr CRMErr;

enum { crmGenericError = -1, crmNoErr = 0 };
enum { curCRMVersion = 1 };
enum { crmType = 9 };
enum { crmRecVersion = 1 };
enum { curCRMSerRecVer = 0 };
enum { crmSerialDevice = 1 };

typedef struct
{
  INTEGER version PACKED;
  StringHandle inputDriverName PACKED_P;
  StringHandle outputDriverName PACKED_P;
  StringHandle name PACKED_P;
  Handle deviceIcon PACKED;
  LONGINT ratedSpeed PACKED;
  LONGINT maxSpeed PACKED;
  LONGINT reserved PACKED;
}
CRMSerialRecord, *CRMSerialPtr;

extern INTEGER CRMGetCRMVersion (void);

extern QHdrPtr CRMGetHeader (void);

extern void CRMInstall (QElemPtr);

extern OSErr CRMRemove (QElemPtr);

extern QElemPtr CRMSearch (QElemPtr);

extern CRMErr InitCRM (void);

#endif

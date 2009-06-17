#if !defined (__NOTIFYMGR__)
#define __NOTIFYMGR__

/*
 * Copyright 1993 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: NotifyMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct PACKED {
  QElemPtr qLink	PACKED_P;
  INTEGER qType;
  INTEGER nmFlags;
  LONGINT nmPrivate;
  INTEGER nmReserved;
  INTEGER nmMark;
  Handle nmIcon	PACKED_P;
  Handle nmSound	PACKED_P;
  StringPtr nmStr	PACKED_P;
  ProcPtr nmResp	PACKED_P;     /* pascal void myresponse(NMRecPtr foo) */
                        /* value of -1 means remove queue element
                           automatically */
  LONGINT nmRefCon;
} NMRec, *NMRecPtr;

extern trap OSErrRET NMInstall( NMRecPtr nmptr );
extern trap OSErrRET NMRemove( NMRecPtr nmptr );

#endif /* __NOTIFYMGR__ */

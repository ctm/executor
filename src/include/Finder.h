#if !defined (_FINDER_H_)
#define _FINDER_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Finder.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct PACKED
{
  QElemPtr qLink	PACKED_P;
  INTEGER qType;
  Ptr ioCmdAddr		PACKED_P;
  ProcPtr ioCompletion	PACKED_P;
  OSErr ioResult;
  StringPtr ioNamePtr	PACKED_P;
  INTEGER ioVRefNum;
  INTEGER ioDTRefNum;
  INTEGER ioIndex;
  LONGINT ioTagInfo;
  Ptr ioDTBuffer	PACKED_P;
  LONGINT ioDTReqCount;
  LONGINT ioDTActCount;
  SignedByte filler1;
  SignedByte ioIconType;
  INTEGER filler2;
  LONGINT ioDirID;
  OSType ioFileCreator;
  OSType ioFileType;
  LONGINT ioFiller3;
  LONGINT ioDTLgLen;
  LONGINT ioDTPyLen;
  INTEGER ioFiller4[14];
  LONGINT ioAPPLParID;
}
DTPBRec, *DTPBRecPtr, *DTPBPtr;

extern OSErr PBDTGetPath (DTPBPtr dtp);
extern OSErr PBDTOpenInform (DTPBPtr dtp);
extern OSErr PBDTCloseDown (DTPBPtr dtp);
extern OSErr PBDTGetIcon (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetIconInfo (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetAPPL (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetComment (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTAddIcon (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTAddAPPL (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTSetComment (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTRemoveAPPL (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTRemoveComment (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTFlush (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetInfo (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTReset (DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTDelete (DTPBPtr dtp, BOOLEAN async);

#endif

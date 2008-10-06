#if !defined (_FINDER_H_)
#define _FINDER_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Finder.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct
{
  QElemPtr qLink	PACKED;
  INTEGER qType		PACKED;
  Ptr ioCmdAddr		PACKED;
  ProcPtr ioCompletion	PACKED;
  OSErr ioResult	PACKED;
  StringPtr ioNamePtr	PACKED;
  INTEGER ioVRefNum	PACKED;
  INTEGER ioDTRefNum	PACKED;
  INTEGER ioIndex	PACKED;
  LONGINT ioTagInfo	PACKED;
  Ptr ioDTBuffer	PACKED;
  LONGINT ioDTReqCount	PACKED;
  LONGINT ioDTActCount	PACKED;
  SignedByte filler1	PACKED;
  SignedByte ioIconType	PACKED;
  INTEGER filler2	PACKED;
  LONGINT ioDirID	PACKED;
  OSType ioFileCreator	PACKED;
  OSType ioFileType	PACKED;
  LONGINT ioFiller3	PACKED;
  LONGINT ioDTLgLen	PACKED;
  LONGINT ioDTPyLen	PACKED;
  INTEGER ioFiller4[14]	PACKED;
  LONGINT ioAPPLParID	PACKED;
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

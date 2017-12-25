#if !defined(_FINDER_H_)
#define _FINDER_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef struct DTPBRec
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<Ptr> ioCmdAddr;
    GUEST<ProcPtr> ioCompletion;
    GUEST<OSErr> ioResult;
    GUEST<StringPtr> ioNamePtr;
    GUEST<INTEGER> ioVRefNum;
    GUEST<INTEGER> ioDTRefNum;
    GUEST<INTEGER> ioIndex;
    GUEST<LONGINT> ioTagInfo;
    GUEST<Ptr> ioDTBuffer;
    GUEST<LONGINT> ioDTReqCount;
    GUEST<LONGINT> ioDTActCount;
    GUEST<SignedByte> filler1;
    GUEST<SignedByte> ioIconType;
    GUEST<INTEGER> filler2;
    GUEST<LONGINT> ioDirID;
    GUEST<OSType> ioFileCreator;
    GUEST<OSType> ioFileType;
    GUEST<LONGINT> ioFiller3;
    GUEST<LONGINT> ioDTLgLen;
    GUEST<LONGINT> ioDTPyLen;
    GUEST<INTEGER[14]> ioFiller4;
    GUEST<LONGINT> ioAPPLParID;
} * DTPBRecPtr, *DTPBPtr;

extern OSErr PBDTGetPath(DTPBPtr dtp);
extern OSErr PBDTOpenInform(DTPBPtr dtp);
extern OSErr PBDTCloseDown(DTPBPtr dtp);
extern OSErr PBDTGetIcon(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetIconInfo(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetAPPL(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetComment(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTAddIcon(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTAddAPPL(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTSetComment(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTRemoveAPPL(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTRemoveComment(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTFlush(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTGetInfo(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTReset(DTPBPtr dtp, BOOLEAN async);
extern OSErr PBDTDelete(DTPBPtr dtp, BOOLEAN async);
}
#endif

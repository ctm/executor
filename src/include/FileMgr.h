#if !defined (_FILEMGR_H_)
#define _FILEMGR_H_

#include "MacTypes.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: FileMgr.h 86 2005-05-25 00:47:12Z ctm $
 */

//So as to not conflict with OS X defines
#undef PBGetVInfo
#undef PBXGetVolInfo
#undef PBGetVol
#undef PBSetVol
#undef PBFlushVol
#undef PBCreate
#undef PBDelete
#undef PBOpenDF
#undef PBOpenRF
#undef PBRename
#undef PBGetFInfo
#undef PBSetFInf
#undef PBSetFLock
#undef PBRstFLock
#undef PBSetFVers
#undef PBAllocate
#undef PBGetEOF
#undef PBSetEOF
#undef PBGetFPos
#undef PBSetFPos
#undef PBFlushFile
#undef PBCatSearch
#undef PBOpenWD
#undef PBCloseWD
#undef PBHSetVol
#undef PBHGetVol
#undef PBCatMove
#undef PBDirCreate
#undef PBGetWDInfo
#undef PBGetFCBInfo
#undef PBGetCatInfo
#undef PBSetCatInfo
#undef PBAllocContig
#undef PBLockRange
#undef PBUnlockRange
#undef PBSetVInfo
#undef PBHGetVInfo
#undef PBHOpen
#undef PBHOpenRF
#undef PBHOpenDF
#undef PBHCreate
#undef PBHDelete
#undef PBHRename
#undef PBHRstFLock
#undef PBHSetFLock
#undef PBHGetFInfo
#undef PBHSetFInfo
#undef PBMakeFSSpec
#undef PBHGetVolParms
#undef PBHGetLogInInfo
#undef PBHGetDirAccess
#undef PBHSetDirAccess
#undef PBHMapID
#undef PBHMapName
#undef PBHCopyFile
#undef PBHMoveRename
#undef PBHOpenDeny
#undef PBHOpenRFDeny
#undef PBExchangeFiles
#undef PBCreateFileIDRef
#undef PBResolveFileIDRef
#undef PBDeleteFileIDRef
#undef PBGetForeignPrivs
#undef PBSetForeignPrivs
#undef PBDTAddIcon
#undef PBDTGetIcon
#undef PBDTGetIconInfo
#undef PBDTAddAPPL
#undef PBDTRemoveAPPL
#undef PBDTGetAPPL
#undef PBDTSetComment
#undef PBDTRemoveComment
#undef PBDTGetComment
#undef PBDTFlush
#undef PBDTReset
#undef PBDTGetInfo
#undef PBDTDelete
#undef PBSetFInfo
#undef PBGetFInfo

namespace Executor {
#define fOnDesk		1
#define fHasBundle	8192
#define fInvisible	16384
#define fTrash		(-3)
#define fDesktop	(-2)
#define fDisk		0

#define fsCurPerm	0
#define fsRdPerm	1
#define fsWrPerm	2
#define fsRdWrPerm	3
#define fsRdWrShPerm	4

#define fsAtMark	0
#define fsFromStart	1
#define fsFromLEOF	2
#define fsFromMark	3
#define rdVerify	64

#define badMDBErr	(-60)
#define badMovErr	(-122)
#define bdNamErr	(-37)
#define dirFulErr	(-33)
#define dskFulErr	(-34)
#define dupFNErr	(-48)
#define eofErr		(-39)
#define extFSErr	(-58)
#define fBsyErr		(-47)
#define fLckdErr	(-45)
#define fnfErr		(-43)
#define fnOpnErr	(-38)
#define fsRnErr		(-59)
#define gfpErr		(-52)
#define ioErr		(-36)
#define memFullErr	(-108)
#define noMacDskErr	(-57)
#define nsDrvErr	(-56)
#define nsvErr		(-35)
#define opWrErr		(-49)
#define paramErr	(-50)
#define permErr		(-54)
#define posErr		(-40)
#define rfNumErr	(-51)
#define tmfoErr		(-42)
#define volOffLinErr	(-53)
#define volOnLinErr	(-55)
#define vLckdErr	(-46)
#define wrgVolTypErr	(-123)
#define wrPermErr	(-61)
#define wPrErr		(-44)
#define tmwdoErr	(-121)
#define dirNFErr	(-120)
#define fsDSIntErr	(-127)

enum
{
  wrgVolTypeErr = -123,
  diffVolErr = -1303,
};

#pragma pack(push, 2)

typedef struct {
  OSType fdType;
  OSType fdCreator;
  uint16_t fdFlags;
  Point fdLocation;
  uint16_t fdFldr;
} FInfo;

typedef struct {
  uint16_t fdIconID;
  uint16_t fdUnused[4];
  uint16_t fdComment;
  LONGINT fdPutAway;
} FXInfo;

typedef struct {
  Rect frRect;
  uint16_t frFlags;
  Point frLocation;
  uint16_t frView;
} DInfo;

typedef struct {
  Point frScroll;
  LONGINT frOpenChain;
  uint16_t frUnused;
  uint16_t frComment;
  LONGINT frPutAway;
} DXInfo;

typedef enum {
    ioParamType,
    fileParamType,
    volumeParamType,
    cntrlParamType
} ParamBlkType;

#define COMMONFSQUEUEDEFS	        \
  PACKED_MEMBER(QElemPtr, qLink);       \
  INTEGER qType;                        \
  INTEGER ioTrap;                       \
  PACKED_MEMBER(Ptr, ioCmdAddr);        \
  PACKED_MEMBER(ProcPtr, ioCompletion); \
  OSErr ioResult;                       \
  PACKED_MEMBER(StringPtr, ioNamePtr);  \
  INTEGER ioVRefNum

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioRefNum;
  SignedByte ioVersNum;
  SignedByte ioPermssn;
  LONGINT ioMisc;		/* should be largest of Ptr, LONGINT */
  PACKED_MEMBER(Ptr, ioBuffer);
  LONGINT ioReqCount;
  LONGINT ioActCount;
  INTEGER ioPosMode;
  LONGINT ioPosOffset;
} IOParam;

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioFRefNum;
  SignedByte ioFVersNum;
  SignedByte filler1;
  INTEGER ioFDirIndex;
  SignedByte ioFlAttrib;
  SignedByte ioFlVersNum;
  FInfo ioFlFndrInfo;
  LONGINT ioFlNum;
  INTEGER ioFlStBlk;
  LONGINT ioFlLgLen;
  LONGINT ioFlPyLen;
  INTEGER ioFlRStBlk;
  LONGINT ioFlRLgLen;
  LONGINT ioFlRPyLen;
  LONGINT ioFlCrDat;
  LONGINT ioFlMdDat;
} FileParam;

typedef struct {
  COMMONFSQUEUEDEFS;
  LONGINT filler2;
  INTEGER ioVolIndex;
  LONGINT ioVCrDate;
  LONGINT ioVLsBkUp;
  uint16_t ioVAtrb;
  uint16_t ioVNmFls;
  uint16_t ioVDirSt;
  uint16_t ioVBlLn;
  uint16_t ioVNmAlBlks;
  LONGINT ioVAlBlkSiz;
  LONGINT ioVClpSiz;
  uint16_t ioAlBlSt;
  LONGINT ioVNxtFNum;
  uint16_t ioVFrBlk;
} VolumeParam;

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioCRefNum;
  INTEGER csCode;
  INTEGER csParam[11];
} CntrlParam;

typedef union {
    IOParam ioParam;
    FileParam fileParam;
    VolumeParam volumeParam;
    CntrlParam cntrlParam;
} ParamBlockRec;
typedef ParamBlockRec *ParmBlkPtr;

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioRefNum;
  SignedByte ioVersNum;
  SignedByte ioPermssn;
  LONGINT ioMisc;		/* should be largest of Ptr, LONGINT */
  PACKED_MEMBER(Ptr, ioBuffer);
  LONGINT ioReqCount;
  LONGINT ioActCount;
  INTEGER ioPosMode;
  LONGINT ioPosOffset;
} HIoParam;

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioFRefNum;
  SignedByte ioFVersNum;
  SignedByte filler1;
  INTEGER ioFDirIndex;
  SignedByte ioFlAttrib;
  SignedByte ioFlVersNum;
  FInfo ioFlFndrInfo;
  /*-->*/ LONGINT ioDirID;
  INTEGER ioFlStBlk;
  LONGINT ioFlLgLen;
  LONGINT ioFlPyLen;
  INTEGER ioFlRStBlk;
  LONGINT ioFlRLgLen;
  LONGINT ioFlRPyLen;
  LONGINT ioFlCrDat;
  LONGINT ioFlMdDat;
} HFileParam;

typedef struct {
  COMMONFSQUEUEDEFS;
  LONGINT pfiller2;
  INTEGER ioVolIndex;
  LONGINT ioVCrDate;
  /*-->*/ LONGINT ioVLsMod;
  INTEGER ioVAtrb;
  uint16_t ioVNmFls;
  /*-->*/ uint16_t ioVBitMap;
  /*-->*/ uint16_t ioVAllocPtr;
  uint16_t ioVNmAlBlks;
  LONGINT ioVAlBlkSiz;
  LONGINT ioVClpSiz;
  uint16_t ioAlBlSt;
  /*-->*/ LONGINT ioVNxtCNID;
  uint16_t ioVFrBlk;
  /*-->*/ uint16_t ioVSigWord;
  /*-->*/ INTEGER ioVDrvInfo;
  /*-->*/ INTEGER ioVDRefNum;
  /*-->*/ INTEGER ioVFSID;
  /*-->*/ LONGINT ioVBkUp;
  /*-->*/ uint16_t ioVSeqNum;
  /*-->*/ LONGINT ioVWrCnt;
  /*-->*/ LONGINT ioVFilCnt;
  /*-->*/ LONGINT ioVDirCnt;
  /*-->*/ LONGINT ioVFndrInfo[8];
} HVolumeParam;

typedef union {
    HIoParam ioParam;
    HFileParam fileParam;
    HVolumeParam volumeParam;
} HParamBlockRec;
typedef HParamBlockRec *HParmBlkPtr;

typedef enum { hfileInfo, dirInfo } CInfoType;

#define COMMONCINFODEFS			\
    COMMONFSQUEUEDEFS;	\
    INTEGER ioFRefNum;	\
    SignedByte ioFVersNum;	\
    SignedByte filler1;	\
    INTEGER ioFDirIndex;	\
    SignedByte ioFlAttrib;	\
    SignedByte ioACUser

typedef struct {
  COMMONCINFODEFS;
  FInfo ioFlFndrInfo;
  LONGINT ioDirID;
  INTEGER ioFlStBlk;
  LONGINT ioFlLgLen;
  LONGINT ioFlPyLen;
  INTEGER ioFlRStBlk;
  LONGINT ioFlRLgLen;
  LONGINT ioFlRPyLen;
  LONGINT ioFlCrDat;
  LONGINT ioFlMdDat;
  LONGINT ioFlBkDat;
  FXInfo ioFlXFndrInfo;
  LONGINT ioFlParID;
  LONGINT ioFlClpSiz;
} HFileInfo;

typedef struct {
  COMMONCINFODEFS;
  DInfo ioDrUsrWds;
  LONGINT ioDrDirID;
  uint16_t ioDrNmFls;
  uint16_t filler3[9];
  LONGINT ioDrCrDat;
  LONGINT ioDrMdDat;
  LONGINT ioDrBkDat;
  DXInfo ioDrFndrInfo;
  LONGINT ioDrParID;
} DirInfo;

typedef union {
    HFileInfo hFileInfo;
    DirInfo dirInfo;
} CInfoPBRec;
typedef CInfoPBRec *CInfoPBPtr;

typedef struct {
  COMMONFSQUEUEDEFS;
  LONGINT filler1;
  PACKED_MEMBER(StringPtr, ioNewName);
  LONGINT filler2;
  LONGINT ioNewDirID;
  LONGINT filler3[2];
  LONGINT ioDirID;
} CMovePBRec;
typedef CMovePBRec *CMovePBPtr;

typedef struct {
  COMMONFSQUEUEDEFS;
  uint16_t filler1;
  INTEGER ioWDIndex;
  LONGINT ioWDProcID;
  INTEGER ioWDVRefNum;
  INTEGER filler2[7];
  LONGINT ioWDDirID;
} WDPBRec;
typedef WDPBRec *WDPBPtr;

typedef struct {
  COMMONFSQUEUEDEFS;
  INTEGER ioRefNum;
  uint16_t filler;
  INTEGER ioFCBIndx;
  INTEGER ioFCBobnoxiousfiller;	/* ACK! not in IMIV, but ThinkC+MPW */
  LONGINT ioFCBFlNm;
  uint16_t ioFCBFlags;
  INTEGER ioFCBStBlk;
  LONGINT ioFCBEOF;
  LONGINT ioFCBPLen;
  LONGINT ioFCBCrPs;
  INTEGER ioFCBVRefNum;
  LONGINT ioFCBClpSiz;
  LONGINT ioFCBParID;
} FCBPBRec;
typedef FCBPBRec *FCBPBPtr;

typedef struct {
  PACKED_MEMBER(QElemPtr, qLink); /* 0 */
  INTEGER qType;		/*  4 */
  uint16_t vcbFlags;		/*  6 */
  uint16_t vcbSigWord;		/*  8 */
  LONGINT vcbCrDate;		/* 10 */
  LONGINT vcbLsMod;		/* 14 */
  uint16_t vcbAtrb;		/* 18 */
  uint16_t vcbNmFls;		/* 20 */
  uint16_t vcbVBMSt;		/* 22 */
  uint16_t vcbAllocPtr;	/* 24 */
  uint16_t vcbNmAlBlks;	/* 26 */
  LONGINT vcbAlBlkSiz;	/* 28 */
  LONGINT vcbClpSiz;		/* 32 */
  uint16_t vcbAlBlSt;		/* 36 */
  LONGINT vcbNxtCNID;		/* 38 */
  uint16_t vcbFreeBks;		/* 42 */
  Byte vcbVN[28];		/* 44 */
  INTEGER vcbDrvNum;		/* 72 */
  INTEGER vcbDRefNum;		/* 74 */
  INTEGER vcbFSID;		/* 76 */
  INTEGER vcbVRefNum;		/* 78 */
  PACKED_MEMBER(Ptr, vcbMAdr);
  PACKED_MEMBER(Ptr, vcbBufAdr);
  uint16_t vcbMLen;
  INTEGER vcbDirIndex;
  uint16_t vcbDirBlk;
  LONGINT vcbVolBkUp;
  uint16_t vcbVSeqNum;
  LONGINT vcbWrCnt;
  LONGINT vcbXTClpSiz;
  LONGINT vcbCTClpSiz;
  uint16_t vcbNmRtDirs;
  LONGINT vcbFilCnt;
  LONGINT vcbDirCnt;
  LONGINT vcbFndrInfo[8];
  uint16_t vcbVCSize;
  uint16_t vcbVBMCSiz;
  uint16_t vcbCtlCSiz;
  uint16_t vcbXTAlBlks;
  uint16_t vcbCTAlBlks;
  INTEGER vcbXTRef;
  INTEGER vcbCTRef;
  PACKED_MEMBER(Ptr, vcbCtlBuf);
  LONGINT vcbDirIDM;
  uint16_t vcbOffsM;
} VCB;

typedef VCB *VCBPtr;
MAKE_HIDDEN(VCBPtr);

typedef struct {
  PACKED_MEMBER(QElemPtr, qLink);
  INTEGER qType;
  INTEGER dQDrive;
  INTEGER dQRefNum;
  INTEGER dQFSID;
  uint16_t dQDrvSz;
  uint16_t dQDrvSz2;
} DrvQEl;

/* data types introduced by the new high level file system dispatch
   traps */

struct FSSpec
{
  INTEGER vRefNum;
  LONGINT parID;
  Str63 name;
};

typedef struct FSSpec FSSpec;
typedef FSSpec *FSSpecPtr;
typedef FSSpecPtr FSSpecArrayPtr;

#if 0
#if !defined (FCBSPtr_H)
extern HIDDEN_Ptr 	FCBSPtr_H;
extern HIDDEN_VCBPtr 	DefVCBPtr_H;
extern HIDDEN_Ptr 	WDCBsPtr_H;
extern INTEGER 	BootDrive;
extern QHdr 	DrvQHdr;
extern QHdr 	VCBQHdr;
extern QHdr 	FSQHdr;
extern INTEGER 	DefVRefNum;
extern INTEGER 	FSFCBLen;
#endif

#define FCBSPtr		(FCBSPtr_H.p)
#define DefVCBPtr	(DefVCBPtr_H.p)
#define WDCBsPtr	(WDCBsPtr_H.p)
#endif

#pragma pack(pop)

extern OSErr FSOpen( StringPtr filen, INTEGER vrn, INTEGER *rn ); 
extern OSErr OpenRF( StringPtr filen, INTEGER vrn, INTEGER *rn ); 
extern OSErr FSRead( INTEGER rn, LONGINT *count, Ptr buffp ); 
extern OSErr FSWrite( INTEGER rn, LONGINT *count, Ptr buffp ); 
extern OSErr GetFPos( INTEGER rn, LONGINT *filep ); 
extern OSErr SetFPos( INTEGER rn, INTEGER posmode, LONGINT possoff ); 
extern OSErr GetEOF( INTEGER rn, LONGINT *eof ); 
extern OSErr SetEOF( INTEGER rn, LONGINT eof ); 
extern OSErr Allocate( INTEGER rn, LONGINT *count ); 
extern OSErr AllocContig( INTEGER rn, LONGINT *count ); 
extern OSErr FSClose( INTEGER rn ); 
extern void ROMlib_rewinddir( void  ); 
extern char *ROMlib_newunixfrommac( char *ip, INTEGER n ); 
extern trap OSErrRET OpenDeny( HParmBlkPtr pb, BOOLEAN a );
extern trap OSErrRET PBHGetLogInInfo( HParmBlkPtr pb, BOOLEAN a );
extern trap OSErrRET PBHGetDirAccess( HParmBlkPtr pb, BOOLEAN a ); 
extern trap OSErrRET PBHCopyFile( HParmBlkPtr pb, BOOLEAN a ); 
extern trap OSErrRET PBHMapID( HParmBlkPtr pb, BOOLEAN a ); 
extern trap OSErrRET PBHMapName( HParmBlkPtr pb, BOOLEAN a ); 
extern trap OSErrRET PBHSetDirAccess( HParmBlkPtr pb, BOOLEAN a ); 
extern trap OSErrRET PBHMoveRename( HParmBlkPtr pb, BOOLEAN a ); 
extern OSErr Create (StringPtr filen, INTEGER vrn, OSType creator,
		     OSType filtyp ); 
extern OSErr FSDelete( StringPtr filen, INTEGER vrn ); 
extern OSErr GetFInfo( StringPtr filen, INTEGER vrn, FInfo *fndrinfo ); 
extern OSErr HGetFInfo (INTEGER vref, LONGINT dirid, Str255 name,
			FInfo *fndrinfo);
extern OSErr SetFInfo( StringPtr filen, INTEGER vrn, 
 FInfo *fndrinfo ); 
extern OSErr SetFLock( StringPtr filen, INTEGER vrn ); 
extern OSErr RstFLock( StringPtr filen, INTEGER vrn ); 
extern OSErr Rename( StringPtr filen, INTEGER vrn, 
 StringPtr newf ); 
extern unsigned char ROMlib_fromhex( unsigned char c ); 
extern INTEGER ROMlib_UNIX7_to_Mac( char *name, INTEGER length ); 
extern trap void FInitQueue( void  ); 
extern trap QHdrPtr GetFSQHdr( void  ); 
extern trap QHdrPtr GetVCBQHdr( void  ); 
extern trap QHdrPtr GetDrvQHdr( void  ); 
extern OSErr GetVInfo( INTEGER drv, StringPtr voln, 
 INTEGER *vrn, LONGINT *freeb ); 
extern OSErr GetVRefNum( INTEGER prn, INTEGER *vrn ); 
extern OSErr GetVol( StringPtr voln, INTEGER *vrn ); 
extern OSErr SetVol( StringPtr voln, INTEGER vrn ); 
extern OSErr FlushVol( StringPtr voln, INTEGER vrn ); 
extern OSErr UnmountVol( StringPtr voln, INTEGER vrn ); 
extern OSErr Eject( StringPtr voln, INTEGER vrn );
extern trap OSErrRET PBHRename( HParmBlkPtr pb, BOOLEAN async );
extern trap OSErrRET PBHCreate( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBDirCreate( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHDelete( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBRead( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBWrite( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBClose( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHOpen( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBOpenDF( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHOpenRF( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetCatInfo( CInfoPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetCatInfo( CInfoPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBCatMove( CMovePBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetVInfo( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBUnmountVol( ParmBlkPtr pb ); 
extern trap OSErrRET PBEject( ParmBlkPtr pb ); 
extern trap OSErrRET PBAllocate( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBAllocContig( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHGetFInfo( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetEOF( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBOpen( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBOpenRF( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBLockRange( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBUnlockRange( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetFPos( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetFPos( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetEOF( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBFlushFile( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBCreate( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBDelete( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBOpenWD( WDPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBCloseWD( WDPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetWDInfo( WDPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetFInfo( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetFInfo( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHSetFInfo( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetFLock( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHSetFLock( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBRstFLock( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHRstFLock( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetFVers( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBRename( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetFCBInfo( FCBPBPtr pb, BOOLEAN async ); 
extern trap OSErr PBMountVol( ParmBlkPtr pb ); 
extern trap OSErrRET PBHGetVInfo( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHGetVolParms( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetVInfo( HParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBGetVol( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHGetVol( WDPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBSetVol( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBHSetVol( WDPBPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBFlushVol( ParmBlkPtr pb, BOOLEAN async ); 
extern trap OSErrRET PBOffLine( ParmBlkPtr pb ); 
extern trap OSErrRET PBExchangeFiles (ParmBlkPtr pb, BOOLEAN async);
extern trap OSErrRET PBCatSearch (ParmBlkPtr pb, BOOLEAN async);
extern trap OSErrRET PBCreateFileIDRef (ParmBlkPtr pb, BOOLEAN async);
extern trap OSErrRET PBDeleteFileIDRef (ParmBlkPtr pb, BOOLEAN async);
extern trap OSErrRET PBResolveFileIDRef (ParmBlkPtr pb, BOOLEAN async);

/* prototypes for the high level filesystem dispatch traps */
extern pascal trap OSErr C_FSMakeFSSpec (int16 vRefNum, int32 dir_id,
					 Str255 file_name, FSSpecPtr spec);
extern pascal trap OSErr C_FSpExchangeFiles (FSSpecPtr src, FSSpecPtr dst);
extern pascal trap OSErr C_FSpOpenDF (FSSpecPtr spec, SignedByte perms,
				      int16 *refNum_out);
extern pascal trap OSErr C_FSpOpenRF (FSSpecPtr spec, SignedByte perms,
				      int16 *refNum_out);
extern pascal trap OSErr C_FSpCreate (FSSpecPtr spec,
				      OSType creator, OSType file_type,
				      ScriptCode script);
extern pascal trap OSErr C_FSpDirCreate (FSSpecPtr spec, ScriptCode script,
					 int32 *created_dir_id);
extern pascal trap OSErr C_FSpDelete (FSSpecPtr spec);

extern pascal trap OSErr C_FSpGetFInfo (FSSpecPtr spec, FInfo *fndr_info);
extern pascal trap OSErr C_FSpSetFInfo (FSSpecPtr spec, FInfo *fndr_info);
extern pascal trap OSErr C_FSpSetFLock (FSSpecPtr spec);
extern pascal trap OSErr C_FSpRstFLock (FSSpecPtr spec);
extern pascal trap OSErr C_FSpRename (FSSpecPtr spec, Str255 new_name);
extern pascal trap OSErr C_FSpCatMove (FSSpecPtr src, FSSpecPtr dst);
extern pascal trap void C_FSpCreateResFile (FSSpecPtr spec,
					     OSType creator, OSType file_type,
					     ScriptCode script);
extern pascal trap INTEGER C_FSpOpenResFile (FSSpecPtr spec, SignedByte perms);
extern pascal trap INTEGER C_HOpenResFile (INTEGER vref, LONGINT dirid,
					   Str255 file_name, SignedByte perm);
extern pascal trap void C_HCreateResFile (INTEGER vrefnum, LONGINT parid,
					  Str255 name);

extern OSErr HCreate (INTEGER vref, LONGINT dirid, Str255 name, OSType creator,
		      OSType type);
extern OSErr HOpenRF (INTEGER vref, LONGINT dirid, Str255 name,
		      SignedByte perm, INTEGER *refp);

extern OSErr GetWDInfo (INTEGER wd, INTEGER *vrefp, LONGINT *dirp,
			LONGINT *procp);
}
#endif /* _FILEMGR_H_ */

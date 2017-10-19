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

typedef struct { GUEST_STRUCT;
  GUEST<OSType> fdType;
  GUEST<OSType> fdCreator;
  GUEST<uint16_t> fdFlags;
  Point fdLocation;
  GUEST<uint16_t> fdFldr;
} FInfo;

typedef struct { GUEST_STRUCT;
    GUEST<uint16_t> fdIconID;
    GUEST<uint16_t> fdUnused[4];
    GUEST<uint16_t> fdComment;
    GUEST<LONGINT> fdPutAway;
} FXInfo;

typedef struct { GUEST_STRUCT;
  Rect frRect;
  GUEST<uint16_t> frFlags;
  Point frLocation;
  GUEST<uint16_t> frView;
} DInfo;

typedef struct { GUEST_STRUCT;
  Point frScroll;
  GUEST<LONGINT> frOpenChain;
  GUEST<uint16_t> frUnused;
  GUEST<uint16_t> frComment;
  GUEST<LONGINT> frPutAway;
} DXInfo;

typedef enum {
    ioParamType,
    fileParamType,
    volumeParamType,
    cntrlParamType
} ParamBlkType;

#define COMMONFSQUEUEDEFS	        \
    GUEST<QElemPtr> qLink;       \
    GUEST<INTEGER> qType;                        \
    GUEST<INTEGER> ioTrap;                       \
    GUEST<Ptr> ioCmdAddr;        \
    GUEST<ProcPtr> ioCompletion; \
    GUEST<OSErr> ioResult;                       \
    GUEST<StringPtr> ioNamePtr;  \
    GUEST<INTEGER> ioVRefNum

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioRefNum;
    GUEST< SignedByte> ioVersNum;
    GUEST< SignedByte> ioPermssn;
    GUEST< LONGINT> ioMisc;    /* should be largest of Ptr, LONGINT */
    GUEST< Ptr> ioBuffer;
    GUEST< LONGINT> ioReqCount;
    GUEST< LONGINT> ioActCount;
    GUEST< INTEGER> ioPosMode;
    GUEST< LONGINT> ioPosOffset;
} IOParam;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioFRefNum;
    GUEST< SignedByte> ioFVersNum;
    GUEST< SignedByte> filler1;
    GUEST< INTEGER> ioFDirIndex;
    GUEST< SignedByte> ioFlAttrib;
    GUEST< SignedByte> ioFlVersNum;
    GUEST< FInfo> ioFlFndrInfo;
    GUEST< LONGINT> ioFlNum;
    GUEST< INTEGER> ioFlStBlk;
    GUEST< LONGINT> ioFlLgLen;
    GUEST< LONGINT> ioFlPyLen;
    GUEST< INTEGER> ioFlRStBlk;
    GUEST< LONGINT> ioFlRLgLen;
    GUEST< LONGINT> ioFlRPyLen;
    GUEST< LONGINT> ioFlCrDat;
    GUEST< LONGINT> ioFlMdDat;
} FileParam;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< LONGINT> filler2;
    GUEST< INTEGER> ioVolIndex;
    GUEST< LONGINT> ioVCrDate;
    GUEST< LONGINT> ioVLsBkUp;
    GUEST< uint16_t> ioVAtrb;
    GUEST< uint16_t> ioVNmFls;
    GUEST< uint16_t> ioVDirSt;
    GUEST< uint16_t> ioVBlLn;
    GUEST< uint16_t> ioVNmAlBlks;
    GUEST< LONGINT> ioVAlBlkSiz;
    GUEST< LONGINT> ioVClpSiz;
    GUEST< uint16_t> ioAlBlSt;
    GUEST< LONGINT> ioVNxtFNum;
    GUEST< uint16_t> ioVFrBlk;
} VolumeParam;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioCRefNum;
    GUEST< INTEGER> csCode;
    GUEST< INTEGER[11]> csParam;
} CntrlParam;

typedef union {
    IOParam ioParam;
    FileParam fileParam;
    VolumeParam volumeParam;
    CntrlParam cntrlParam;
} ParamBlockRec;
typedef ParamBlockRec *ParmBlkPtr;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioRefNum;
    GUEST< SignedByte> ioVersNum;
    GUEST< SignedByte> ioPermssn;
    GUEST< LONGINT> ioMisc;    /* should be largest of Ptr, LONGINT */
    GUEST< Ptr> ioBuffer;
    GUEST< LONGINT> ioReqCount;
    GUEST< LONGINT> ioActCount;
    GUEST< INTEGER> ioPosMode;
    GUEST< LONGINT> ioPosOffset;
} HIoParam;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioFRefNum;
    GUEST< SignedByte> ioFVersNum;
    GUEST< SignedByte> filler1;
    GUEST< INTEGER> ioFDirIndex;
    GUEST< SignedByte> ioFlAttrib;
    GUEST< SignedByte> ioFlVersNum;
    GUEST< FInfo> ioFlFndrInfo;
    GUEST< LONGINT> ioDirID;    /*-->*/
    GUEST< INTEGER> ioFlStBlk;
    GUEST< LONGINT> ioFlLgLen;
    GUEST< LONGINT> ioFlPyLen;
    GUEST< INTEGER> ioFlRStBlk;
    GUEST< LONGINT> ioFlRLgLen;
    GUEST< LONGINT> ioFlRPyLen;
    GUEST< LONGINT> ioFlCrDat;
    GUEST< LONGINT> ioFlMdDat;
} HFileParam;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< LONGINT> pfiller2;
    GUEST< INTEGER> ioVolIndex;
    GUEST< LONGINT> ioVCrDate;
    GUEST< LONGINT> ioVLsMod;    /*-->*/
    GUEST< INTEGER> ioVAtrb;
    GUEST< uint16_t> ioVNmFls;
    GUEST< uint16_t> ioVBitMap;    /*-->*/
    GUEST< uint16_t> ioVAllocPtr;    /*-->*/
    GUEST< uint16_t> ioVNmAlBlks;
    GUEST< LONGINT> ioVAlBlkSiz;
    GUEST< LONGINT> ioVClpSiz;
    GUEST< uint16_t> ioAlBlSt;
    GUEST< LONGINT> ioVNxtCNID;    /*-->*/
    GUEST< uint16_t> ioVFrBlk;
    GUEST< uint16_t> ioVSigWord;    /*-->*/
    GUEST< INTEGER> ioVDrvInfo;    /*-->*/
    GUEST< INTEGER> ioVDRefNum;    /*-->*/
    GUEST< INTEGER> ioVFSID;    /*-->*/
    GUEST< LONGINT> ioVBkUp;    /*-->*/
    GUEST< uint16_t> ioVSeqNum;    /*-->*/
    GUEST< LONGINT> ioVWrCnt;    /*-->*/
    GUEST< LONGINT> ioVFilCnt;    /*-->*/
    GUEST< LONGINT> ioVDirCnt;    /*-->*/
    GUEST< LONGINT[8]> ioVFndrInfo;    /*-->*/
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
    GUEST<INTEGER> ioFRefNum;	\
    GUEST<SignedByte> ioFVersNum;	\
    GUEST<SignedByte> filler1;	\
    GUEST<INTEGER> ioFDirIndex;	\
    GUEST<SignedByte> ioFlAttrib;	\
    GUEST<SignedByte> ioACUser

typedef struct { GUEST_STRUCT;
  COMMONCINFODEFS;
    GUEST< FInfo> ioFlFndrInfo;
    GUEST< LONGINT> ioDirID;
    GUEST< INTEGER> ioFlStBlk;
    GUEST< LONGINT> ioFlLgLen;
    GUEST< LONGINT> ioFlPyLen;
    GUEST< INTEGER> ioFlRStBlk;
    GUEST< LONGINT> ioFlRLgLen;
    GUEST< LONGINT> ioFlRPyLen;
    GUEST< LONGINT> ioFlCrDat;
    GUEST< LONGINT> ioFlMdDat;
    GUEST< LONGINT> ioFlBkDat;
    GUEST< FXInfo> ioFlXFndrInfo;
    GUEST< LONGINT> ioFlParID;
    GUEST< LONGINT> ioFlClpSiz;
} HFileInfo;

typedef struct { GUEST_STRUCT;
  COMMONCINFODEFS;
    GUEST< DInfo> ioDrUsrWds;
    GUEST< LONGINT> ioDrDirID;
    GUEST< uint16_t> ioDrNmFls;
    GUEST< uint16_t[9]> filler3;
    GUEST< LONGINT> ioDrCrDat;
    GUEST< LONGINT> ioDrMdDat;
    GUEST< LONGINT> ioDrBkDat;
    GUEST< DXInfo> ioDrFndrInfo;
    GUEST< LONGINT> ioDrParID;
} DirInfo;

typedef union {
    HFileInfo hFileInfo;
    DirInfo dirInfo;
} CInfoPBRec;
typedef CInfoPBRec *CInfoPBPtr;

typedef struct { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< LONGINT> filler1;
    GUEST< StringPtr> ioNewName;
    GUEST< LONGINT> filler2;
    GUEST< LONGINT> ioNewDirID;
    GUEST< LONGINT[2]> filler3;
    GUEST< LONGINT> ioDirID;
} CMovePBRec;
typedef CMovePBRec *CMovePBPtr;

struct WDPBRec { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< uint16_t> filler1;
    GUEST< INTEGER> ioWDIndex;
    GUEST< LONGINT> ioWDProcID;
    GUEST< INTEGER> ioWDVRefNum;
    GUEST< INTEGER[7]> filler2;
    GUEST< LONGINT> ioWDDirID;
};
typedef WDPBRec *WDPBPtr;

struct FCBPBRec { GUEST_STRUCT;
  COMMONFSQUEUEDEFS;
    GUEST< INTEGER> ioRefNum;
    GUEST< uint16_t> filler;
    GUEST< INTEGER> ioFCBIndx;
    GUEST< INTEGER> ioFCBobnoxiousfiller;    /* ACK! not in IMIV, but ThinkC+MPW */
    GUEST< LONGINT> ioFCBFlNm;
    GUEST< uint16_t> ioFCBFlags;
    GUEST< INTEGER> ioFCBStBlk;
    GUEST< LONGINT> ioFCBEOF;
    GUEST< LONGINT> ioFCBPLen;
    GUEST< LONGINT> ioFCBCrPs;
    GUEST< INTEGER> ioFCBVRefNum;
    GUEST< LONGINT> ioFCBClpSiz;
    GUEST< LONGINT> ioFCBParID;
};
typedef FCBPBRec *FCBPBPtr;

struct VCB { GUEST_STRUCT;
    GUEST< QElemPtr> qLink;    /* 0 */
    GUEST< INTEGER> qType;    /*  4 */
    GUEST< uint16_t> vcbFlags;    /*  6 */
    GUEST< uint16_t> vcbSigWord;    /*  8 */
    GUEST< LONGINT> vcbCrDate;    /* 10 */
    GUEST< LONGINT> vcbLsMod;    /* 14 */
    GUEST< uint16_t> vcbAtrb;    /* 18 */
    GUEST< uint16_t> vcbNmFls;    /* 20 */
    GUEST< uint16_t> vcbVBMSt;    /* 22 */
    GUEST< uint16_t> vcbAllocPtr;    /* 24 */
    GUEST< uint16_t> vcbNmAlBlks;    /* 26 */
    GUEST< LONGINT> vcbAlBlkSiz;    /* 28 */
    GUEST< LONGINT> vcbClpSiz;    /* 32 */
    GUEST< uint16_t> vcbAlBlSt;    /* 36 */
    GUEST< LONGINT> vcbNxtCNID;    /* 38 */
    GUEST< uint16_t> vcbFreeBks;    /* 42 */
    GUEST< Byte[28]> vcbVN;    /* 44 */
    GUEST< INTEGER> vcbDrvNum;    /* 72 */
    GUEST< INTEGER> vcbDRefNum;    /* 74 */
    GUEST< INTEGER> vcbFSID;    /* 76 */
    GUEST< INTEGER> vcbVRefNum;    /* 78 */
    GUEST< Ptr> vcbMAdr;
    GUEST< Ptr> vcbBufAdr;
    GUEST< uint16_t> vcbMLen;
    GUEST< INTEGER> vcbDirIndex;
    GUEST< uint16_t> vcbDirBlk;
    GUEST< LONGINT> vcbVolBkUp;
    GUEST< uint16_t> vcbVSeqNum;
    GUEST< LONGINT> vcbWrCnt;
    GUEST< LONGINT> vcbXTClpSiz;
    GUEST< LONGINT> vcbCTClpSiz;
    GUEST< uint16_t> vcbNmRtDirs;
    GUEST< LONGINT> vcbFilCnt;
    GUEST< LONGINT> vcbDirCnt;
    GUEST< LONGINT[8]> vcbFndrInfo;
    GUEST< uint16_t> vcbVCSize;
    GUEST< uint16_t> vcbVBMCSiz;
    GUEST< uint16_t> vcbCtlCSiz;
    GUEST< uint16_t> vcbXTAlBlks;
    GUEST< uint16_t> vcbCTAlBlks;
    GUEST< INTEGER> vcbXTRef;
    GUEST< INTEGER> vcbCTRef;
    GUEST< Ptr> vcbCtlBuf;
    GUEST< LONGINT> vcbDirIDM;
    GUEST< uint16_t> vcbOffsM;
};

typedef VCB *VCBPtr;


struct DrvQEl {
    GUEST_STRUCT;
    GUEST< QElemPtr> qLink;
    GUEST< INTEGER> qType;
    GUEST< INTEGER> dQDrive;
    GUEST< INTEGER> dQRefNum;
    GUEST< INTEGER> dQFSID;
    GUEST< uint16_t> dQDrvSz;
    GUEST< uint16_t> dQDrvSz2;
};

/* data types introduced by the new high level file system dispatch
   traps */

struct FSSpec
{
    GUEST_STRUCT;
    GUEST< INTEGER> vRefNum;
    GUEST< LONGINT> parID;
    GUEST< Str63> name;
};

typedef struct FSSpec FSSpec;
typedef FSSpec *FSSpecPtr;
typedef FSSpecPtr FSSpecArrayPtr;

#if 0
#if !defined (FCBSPtr_H)
extern GUEST<Ptr> 	FCBSPtr_H;
extern GUEST<VCBPtr> 	DefVCBPtr_H;
extern GUEST<Ptr> 	WDCBsPtr_H;
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

        // the output parameters point to NATIVE values
        // there are no traps for these functions,
        // they are implemented again in glue code in Mac apps
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
 GUEST<INTEGER> *vrn, GUEST<LONGINT> *freeb ); 
extern OSErr GetVRefNum( INTEGER prn, GUEST<INTEGER> *vrn ); 
extern OSErr GetVol( StringPtr voln, GUEST<INTEGER> *vrn ); 
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
				      GUEST<int16> *refNum_out);
extern pascal trap OSErr C_FSpOpenRF (FSSpecPtr spec, SignedByte perms,
				      GUEST<int16> *refNum_out);
extern pascal trap OSErr C_FSpCreate (FSSpecPtr spec,
				      OSType creator, OSType file_type,
				      ScriptCode script);
extern pascal trap OSErr C_FSpDirCreate (FSSpecPtr spec, ScriptCode script,
					 GUEST<int32> *created_dir_id);
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
		      SignedByte perm, /*NATIVE*/ INTEGER *refp); 

extern OSErr GetWDInfo (INTEGER wd, GUEST<INTEGER> *vrefp, GUEST<LONGINT> *dirp,
			GUEST<LONGINT> *procp);
}
#endif /* _FILEMGR_H_ */

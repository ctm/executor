#if !defined(_FILEMGR_H_)
#define _FILEMGR_H_

#include "ExMacTypes.h"
#include <rsys/lowglobals.h>

#define MODULE_NAME FileMgr
#include <rsys/api-module.h>

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

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

namespace Executor
{
enum
{
    fOnDesk = 1,
    fHasBundle = 8192,
    fInvisible = 16384,
    fTrash = (-3),
    fDesktop = (-2),
    fDisk = 0,
};

enum
{
    fsCurPerm = 0,
    fsRdPerm = 1,
    fsWrPerm = 2,
    fsRdWrPerm = 3,
    fsRdWrShPerm = 4,
};

enum
{
    fsAtMark = 0,
    fsFromStart = 1,
    fsFromLEOF = 2,
    fsFromMark = 3,
    rdVerify = 64,
};

enum
{
    badMDBErr = (-60),
    badMovErr = (-122),
    bdNamErr = (-37),
    dirFulErr = (-33),
    dskFulErr = (-34),
    dupFNErr = (-48),
    eofErr = (-39),
    extFSErr = (-58),
    fBsyErr = (-47),
    fLckdErr = (-45),
    fnfErr = (-43),
    fnOpnErr = (-38),
    fsRnErr = (-59),
    gfpErr = (-52),
    ioErr = (-36),
    noMacDskErr = (-57),
    nsDrvErr = (-56),
    nsvErr = (-35),
    opWrErr = (-49),
    permErr = (-54),
    posErr = (-40),
    rfNumErr = (-51),
    tmfoErr = (-42),
    volOffLinErr = (-53),
    volOnLinErr = (-55),
    vLckdErr = (-46),
    wrgVolTypErr = (-123),
    wrPermErr = (-61),
    wPrErr = (-44),
    tmwdoErr = (-121),
    dirNFErr = (-120),
    fsDSIntErr = (-127),
};

enum
{
    wrgVolTypeErr = -123,
    diffVolErr = -1303,
};

#pragma pack(push, 2)

typedef struct
{
    GUEST_STRUCT;
    GUEST<OSType> fdType;
    GUEST<OSType> fdCreator;
    GUEST<uint16_t> fdFlags;
    GUEST<Point> fdLocation;
    GUEST<uint16_t> fdFldr;
} FInfo;

typedef struct
{
    GUEST_STRUCT;
    GUEST<uint16_t> fdIconID;
    GUEST<uint16_t> fdUnused[4];
    GUEST<uint16_t> fdComment;
    GUEST<LONGINT> fdPutAway;
} FXInfo;

typedef struct
{
    GUEST_STRUCT;
    GUEST<Rect> frRect;
    GUEST<uint16_t> frFlags;
    GUEST<Point> frLocation;
    GUEST<uint16_t> frView;
} DInfo;

typedef struct
{
    GUEST_STRUCT;
    GUEST<Point> frScroll;
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

#define COMMONFSQUEUEDEFS        \
    GUEST<QElemPtr> qLink;       \
    GUEST<INTEGER> qType;        \
    GUEST<INTEGER> ioTrap;       \
    GUEST<Ptr> ioCmdAddr;        \
    GUEST<ProcPtr> ioCompletion; \
    GUEST<OSErr> ioResult;       \
    GUEST<StringPtr> ioNamePtr;  \
    GUEST<INTEGER> ioVRefNum

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioRefNum;
    GUEST<SignedByte> ioVersNum;
    GUEST<SignedByte> ioPermssn;
    GUEST<LONGINT> ioMisc; /* should be largest of Ptr, LONGINT */
    GUEST<Ptr> ioBuffer;
    GUEST<LONGINT> ioReqCount;
    GUEST<LONGINT> ioActCount;
    GUEST<INTEGER> ioPosMode;
    GUEST<LONGINT> ioPosOffset;
} IOParam;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioFRefNum;
    GUEST<SignedByte> ioFVersNum;
    GUEST<SignedByte> filler1;
    GUEST<INTEGER> ioFDirIndex;
    GUEST<SignedByte> ioFlAttrib;
    GUEST<SignedByte> ioFlVersNum;
    GUEST<FInfo> ioFlFndrInfo;
    GUEST<LONGINT> ioFlNum;
    GUEST<INTEGER> ioFlStBlk;
    GUEST<LONGINT> ioFlLgLen;
    GUEST<LONGINT> ioFlPyLen;
    GUEST<INTEGER> ioFlRStBlk;
    GUEST<LONGINT> ioFlRLgLen;
    GUEST<LONGINT> ioFlRPyLen;
    GUEST<LONGINT> ioFlCrDat;
    GUEST<LONGINT> ioFlMdDat;
} FileParam;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<LONGINT> filler2;
    GUEST<INTEGER> ioVolIndex;
    GUEST<LONGINT> ioVCrDate;
    GUEST<LONGINT> ioVLsBkUp;
    GUEST<uint16_t> ioVAtrb;
    GUEST<uint16_t> ioVNmFls;
    GUEST<uint16_t> ioVDirSt;
    GUEST<uint16_t> ioVBlLn;
    GUEST<uint16_t> ioVNmAlBlks;
    GUEST<LONGINT> ioVAlBlkSiz;
    GUEST<LONGINT> ioVClpSiz;
    GUEST<uint16_t> ioAlBlSt;
    GUEST<LONGINT> ioVNxtFNum;
    GUEST<uint16_t> ioVFrBlk;
} VolumeParam;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioCRefNum;
    GUEST<INTEGER> csCode;
    GUEST<INTEGER[11]> csParam;
} CntrlParam;

typedef union {
    IOParam ioParam;
    FileParam fileParam;
    VolumeParam volumeParam;
    CntrlParam cntrlParam;
} ParamBlockRec;
typedef ParamBlockRec *ParmBlkPtr;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioRefNum;
    GUEST<SignedByte> ioVersNum;
    GUEST<SignedByte> ioPermssn;
    GUEST<LONGINT> ioMisc; /* should be largest of Ptr, LONGINT */
    GUEST<Ptr> ioBuffer;
    GUEST<LONGINT> ioReqCount;
    GUEST<LONGINT> ioActCount;
    GUEST<INTEGER> ioPosMode;
    GUEST<LONGINT> ioPosOffset;
} HIoParam;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioFRefNum;
    GUEST<SignedByte> ioFVersNum;
    GUEST<SignedByte> filler1;
    GUEST<INTEGER> ioFDirIndex;
    GUEST<SignedByte> ioFlAttrib;
    GUEST<SignedByte> ioFlVersNum;
    GUEST<FInfo> ioFlFndrInfo;
    GUEST<LONGINT> ioDirID; /*-->*/
    GUEST<INTEGER> ioFlStBlk;
    GUEST<LONGINT> ioFlLgLen;
    GUEST<LONGINT> ioFlPyLen;
    GUEST<INTEGER> ioFlRStBlk;
    GUEST<LONGINT> ioFlRLgLen;
    GUEST<LONGINT> ioFlRPyLen;
    GUEST<LONGINT> ioFlCrDat;
    GUEST<LONGINT> ioFlMdDat;
} HFileParam;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<LONGINT> pfiller2;
    GUEST<INTEGER> ioVolIndex;
    GUEST<LONGINT> ioVCrDate;
    GUEST<LONGINT> ioVLsMod; /*-->*/
    GUEST<INTEGER> ioVAtrb;
    GUEST<uint16_t> ioVNmFls;
    GUEST<uint16_t> ioVBitMap; /*-->*/
    GUEST<uint16_t> ioVAllocPtr; /*-->*/
    GUEST<uint16_t> ioVNmAlBlks;
    GUEST<LONGINT> ioVAlBlkSiz;
    GUEST<LONGINT> ioVClpSiz;
    GUEST<uint16_t> ioAlBlSt;
    GUEST<LONGINT> ioVNxtCNID; /*-->*/
    GUEST<uint16_t> ioVFrBlk;
    GUEST<uint16_t> ioVSigWord; /*-->*/
    GUEST<INTEGER> ioVDrvInfo; /*-->*/
    GUEST<INTEGER> ioVDRefNum; /*-->*/
    GUEST<INTEGER> ioVFSID; /*-->*/
    GUEST<LONGINT> ioVBkUp; /*-->*/
    GUEST<uint16_t> ioVSeqNum; /*-->*/
    GUEST<LONGINT> ioVWrCnt; /*-->*/
    GUEST<LONGINT> ioVFilCnt; /*-->*/
    GUEST<LONGINT> ioVDirCnt; /*-->*/
    GUEST<LONGINT[8]> ioVFndrInfo; /*-->*/
} HVolumeParam;

typedef union {
    HIoParam ioParam;
    HFileParam fileParam;
    HVolumeParam volumeParam;
} HParamBlockRec;
typedef HParamBlockRec *HParmBlkPtr;

typedef enum { hfileInfo,
               dirInfo } CInfoType;

#define COMMONCINFODEFS           \
    COMMONFSQUEUEDEFS;            \
    GUEST<INTEGER> ioFRefNum;     \
    GUEST<SignedByte> ioFVersNum; \
    GUEST<SignedByte> filler1;    \
    GUEST<INTEGER> ioFDirIndex;   \
    GUEST<SignedByte> ioFlAttrib; \
    GUEST<SignedByte> ioACUser

typedef struct
{
    GUEST_STRUCT;
    COMMONCINFODEFS;
    GUEST<FInfo> ioFlFndrInfo;
    GUEST<LONGINT> ioDirID;
    GUEST<INTEGER> ioFlStBlk;
    GUEST<LONGINT> ioFlLgLen;
    GUEST<LONGINT> ioFlPyLen;
    GUEST<INTEGER> ioFlRStBlk;
    GUEST<LONGINT> ioFlRLgLen;
    GUEST<LONGINT> ioFlRPyLen;
    GUEST<LONGINT> ioFlCrDat;
    GUEST<LONGINT> ioFlMdDat;
    GUEST<LONGINT> ioFlBkDat;
    GUEST<FXInfo> ioFlXFndrInfo;
    GUEST<LONGINT> ioFlParID;
    GUEST<LONGINT> ioFlClpSiz;
} HFileInfo;

typedef struct
{
    GUEST_STRUCT;
    COMMONCINFODEFS;
    GUEST<DInfo> ioDrUsrWds;
    GUEST<LONGINT> ioDrDirID;
    GUEST<uint16_t> ioDrNmFls;
    GUEST<uint16_t[9]> filler3;
    GUEST<LONGINT> ioDrCrDat;
    GUEST<LONGINT> ioDrMdDat;
    GUEST<LONGINT> ioDrBkDat;
    GUEST<DXInfo> ioDrFndrInfo;
    GUEST<LONGINT> ioDrParID;
} DirInfo;

typedef union {
    HFileInfo hFileInfo;
    DirInfo dirInfo;
} CInfoPBRec;
typedef CInfoPBRec *CInfoPBPtr;

typedef struct
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<LONGINT> filler1;
    GUEST<StringPtr> ioNewName;
    GUEST<LONGINT> filler2;
    GUEST<LONGINT> ioNewDirID;
    GUEST<LONGINT[2]> filler3;
    GUEST<LONGINT> ioDirID;
} CMovePBRec;
typedef CMovePBRec *CMovePBPtr;

struct WDPBRec
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<uint16_t> filler1;
    GUEST<INTEGER> ioWDIndex;
    GUEST<LONGINT> ioWDProcID;
    GUEST<INTEGER> ioWDVRefNum;
    GUEST<INTEGER[7]> filler2;
    GUEST<LONGINT> ioWDDirID;
};
typedef WDPBRec *WDPBPtr;

struct FCBPBRec
{
    GUEST_STRUCT;
    COMMONFSQUEUEDEFS;
    GUEST<INTEGER> ioRefNum;
    GUEST<uint16_t> filler;
    GUEST<INTEGER> ioFCBIndx;
    GUEST<INTEGER> ioFCBobnoxiousfiller; /* ACK! not in IMIV, but ThinkC+MPW */
    GUEST<LONGINT> ioFCBFlNm;
    GUEST<uint16_t> ioFCBFlags;
    GUEST<INTEGER> ioFCBStBlk;
    GUEST<LONGINT> ioFCBEOF;
    GUEST<LONGINT> ioFCBPLen;
    GUEST<LONGINT> ioFCBCrPs;
    GUEST<INTEGER> ioFCBVRefNum;
    GUEST<LONGINT> ioFCBClpSiz;
    GUEST<LONGINT> ioFCBParID;
};
typedef FCBPBRec *FCBPBPtr;

struct VCB
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink; /* 0 */
    GUEST<INTEGER> qType; /*  4 */
    GUEST<uint16_t> vcbFlags; /*  6 */
    GUEST<uint16_t> vcbSigWord; /*  8 */
    GUEST<LONGINT> vcbCrDate; /* 10 */
    GUEST<LONGINT> vcbLsMod; /* 14 */
    GUEST<uint16_t> vcbAtrb; /* 18 */
    GUEST<uint16_t> vcbNmFls; /* 20 */
    GUEST<uint16_t> vcbVBMSt; /* 22 */
    GUEST<uint16_t> vcbAllocPtr; /* 24 */
    GUEST<uint16_t> vcbNmAlBlks; /* 26 */
    GUEST<LONGINT> vcbAlBlkSiz; /* 28 */
    GUEST<LONGINT> vcbClpSiz; /* 32 */
    GUEST<uint16_t> vcbAlBlSt; /* 36 */
    GUEST<LONGINT> vcbNxtCNID; /* 38 */
    GUEST<uint16_t> vcbFreeBks; /* 42 */
    GUEST<Byte[28]> vcbVN; /* 44 */
    GUEST<INTEGER> vcbDrvNum; /* 72 */
    GUEST<INTEGER> vcbDRefNum; /* 74 */
    GUEST<INTEGER> vcbFSID; /* 76 */
    GUEST<INTEGER> vcbVRefNum; /* 78 */
    GUEST<Ptr> vcbMAdr;
    GUEST<Ptr> vcbBufAdr;
    GUEST<uint16_t> vcbMLen;
    GUEST<INTEGER> vcbDirIndex;
    GUEST<uint16_t> vcbDirBlk;
    GUEST<LONGINT> vcbVolBkUp;
    GUEST<uint16_t> vcbVSeqNum;
    GUEST<LONGINT> vcbWrCnt;
    GUEST<LONGINT> vcbXTClpSiz;
    GUEST<LONGINT> vcbCTClpSiz;
    GUEST<uint16_t> vcbNmRtDirs;
    GUEST<LONGINT> vcbFilCnt;
    GUEST<LONGINT> vcbDirCnt;
    GUEST<LONGINT[8]> vcbFndrInfo;
    GUEST<uint16_t> vcbVCSize;
    GUEST<uint16_t> vcbVBMCSiz;
    GUEST<uint16_t> vcbCtlCSiz;
    GUEST<uint16_t> vcbXTAlBlks;
    GUEST<uint16_t> vcbCTAlBlks;
    GUEST<INTEGER> vcbXTRef;
    GUEST<INTEGER> vcbCTRef;
    GUEST<Ptr> vcbCtlBuf;
    GUEST<LONGINT> vcbDirIDM;
    GUEST<uint16_t> vcbOffsM;
};

typedef VCB *VCBPtr;

struct DrvQEl
{
    GUEST_STRUCT;
    GUEST<QElemPtr> qLink;
    GUEST<INTEGER> qType;
    GUEST<INTEGER> dQDrive;
    GUEST<INTEGER> dQRefNum;
    GUEST<INTEGER> dQFSID;
    GUEST<uint16_t> dQDrvSz;
    GUEST<uint16_t> dQDrvSz2;
};

/* data types introduced by the new high level file system dispatch
   traps */

struct FSSpec
{
    GUEST_STRUCT;
    GUEST<INTEGER> vRefNum;
    GUEST<LONGINT> parID;
    GUEST<Str63> name;
};

typedef struct FSSpec FSSpec;
typedef FSSpec *FSSpecPtr;
typedef FSSpecPtr FSSpecArrayPtr;

const LowMemGlobal<INTEGER> BootDrive { 0x210 }; // FileMgr IMIV-212 (true);
const LowMemGlobal<QHdr> DrvQHdr { 0x308 }; // FileMgr IMIV-182 (true);
const LowMemGlobal<ProcPtr> EjectNotify { 0x338 }; // FileMgr ThinkC (false);
const LowMemGlobal<Ptr> FCBSPtr { 0x34E }; // FileMgr IMIV-179 (true);
const LowMemGlobal<VCBPtr> DefVCBPtr { 0x352 }; // FileMgr IMIV-178 (true);
const LowMemGlobal<QHdr> VCBQHdr { 0x356 }; // FileMgr IMIV-178 (true);
const LowMemGlobal<QHdr> FSQHdr { 0x360 }; // FileMgr IMIV-176 (true);
const LowMemGlobal<Ptr> WDCBsPtr { 0x372 }; // FileMgr idunno (true);
const LowMemGlobal<INTEGER> DefVRefNum { 0x384 }; // FileMgr MPW (true);

/*
 * Note: MacLinkPC+ loads 0x358 into a register (i.e. the address of the
 * pointer to the first element on the VCB queue) and then uses
 * 72 off of it (0x3A0) and 78 off of it (0x3A6). As LONGINT as
 * there are zeros there, that doesn't hurt us, but normally,
 * we'd have negative ones in there. Hence we describe them
 * here and set them to zero in executor.
 */
/* const LowMemGlobal<Ptr> FmtDefaults { 0x39E }; // FileMgr ThinkC (false); */
const LowMemGlobal<Ptr> ToExtFS { 0x3F2 }; // FileMgr IMIV-212 (false);
const LowMemGlobal<INTEGER> FSFCBLen { 0x3F6 }; // FileMgr IMIV-97 (true);


#pragma pack(pop)

// the output parameters point to NATIVE values
// there are no traps for these functions,
// they are implemented again in glue code in Mac apps
extern OSErr FSOpen(StringPtr filen, INTEGER vrn, INTEGER *rn);
extern OSErr OpenRF(StringPtr filen, INTEGER vrn, INTEGER *rn);
extern OSErr FSRead(INTEGER rn, LONGINT *count, Ptr buffp);
extern OSErr FSWrite(INTEGER rn, LONGINT *count, Ptr buffp);
extern OSErr GetFPos(INTEGER rn, LONGINT *filep);
extern OSErr SetFPos(INTEGER rn, INTEGER posmode, LONGINT possoff);
extern OSErr GetEOF(INTEGER rn, LONGINT *eof);
extern OSErr SetEOF(INTEGER rn, LONGINT eof);
extern OSErr Allocate(INTEGER rn, GUEST<LONGINT> *count);
extern OSErr AllocContig(INTEGER rn, GUEST<LONGINT> *count);
extern OSErr FSClose(INTEGER rn);

extern void ROMlib_rewinddir(void);
extern OSErr Create(StringPtr filen, INTEGER vrn, OSType creator,
                    OSType filtyp);
extern OSErr FSDelete(StringPtr filen, INTEGER vrn);
extern OSErr GetFInfo(StringPtr filen, INTEGER vrn, FInfo *fndrinfo);
extern OSErr HGetFInfo(INTEGER vref, LONGINT dirid, Str255 name,
                       FInfo *fndrinfo);
extern OSErr SetFInfo(StringPtr filen, INTEGER vrn,
                      FInfo *fndrinfo);
extern OSErr SetFLock(StringPtr filen, INTEGER vrn);
extern OSErr RstFLock(StringPtr filen, INTEGER vrn);
extern OSErr Rename(StringPtr filen, INTEGER vrn,
                    StringPtr newf);
extern unsigned char ROMlib_fromhex(unsigned char c);
extern INTEGER ROMlib_UNIX7_to_Mac(char *name, INTEGER length);
extern void FInitQueue(void);
extern QHdrPtr GetFSQHdr(void);
extern QHdrPtr GetVCBQHdr(void);
extern QHdrPtr GetDrvQHdr(void);
extern OSErr GetVInfo(INTEGER drv, StringPtr voln,
                      GUEST<INTEGER> *vrn, GUEST<LONGINT> *freeb);
extern OSErr GetVRefNum(INTEGER prn, GUEST<INTEGER> *vrn);
extern OSErr GetVol(StringPtr voln, GUEST<INTEGER> *vrn);
extern OSErr SetVol(StringPtr voln, INTEGER vrn);
extern OSErr FlushVol(StringPtr voln, INTEGER vrn);
extern OSErr UnmountVol(StringPtr voln, INTEGER vrn);
extern OSErr Eject(StringPtr voln, INTEGER vrn);

extern OSErr PBAllocContig(ParmBlkPtr pb, BOOLEAN async);


extern OSErr PBOpen(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHOpen(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBOpen, PBHOpen, HParmBlkPtr, 0xA000);

extern OSErr PBClose(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBClose, 0xA001);

extern OSErr PBRead(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBRead, 0xA002);

extern OSErr PBWrite(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBWrite, 0xA003);

extern OSErr PBGetVInfo(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHGetVInfo(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBGetVInfo, PBHGetVInfo, HParmBlkPtr, 0xA007);

extern OSErr PBCreate(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHCreate(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBCreate, PBHCreate, HParmBlkPtr, 0xA008);

extern OSErr PBDelete(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHDelete(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBDelete, PBHDelete, HParmBlkPtr, 0xA009);

extern OSErr PBOpenRF(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHOpenRF(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBOpenRF, PBHOpenRF, HParmBlkPtr, 0xA00A);

extern OSErr PBRename(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHRename(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBRename, PBHRename, HParmBlkPtr, 0xA00B);

extern OSErr PBGetFInfo(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHGetFInfo(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBGetFInfo, PBHGetFInfo, HParmBlkPtr, 0xA00C);

extern OSErr PBSetFInfo(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHSetFInfo(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBSetFInfo, PBHSetFInfo, HParmBlkPtr, 0xA00D);

extern OSErr PBUnmountVol(ParmBlkPtr pb);
REGISTER_TRAP2(PBUnmountVol, 0xA00E, D0(A0));

extern OSErr PBMountVol(ParmBlkPtr pb);
REGISTER_TRAP2(PBMountVol, 0xA00F, D0(A0));

extern OSErr PBAllocate(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBAllocate, 0xA010);

extern OSErr PBGetEOF(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBGetEOF, 0xA011);

extern OSErr PBSetEOF(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBSetEOF, 0xA012);

extern OSErr PBFlushVol(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBFlushVol, 0xA013);

extern OSErr PBGetVol(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHGetVol(WDPBPtr pb, BOOLEAN async);
HFS_TRAP(PBGetVol, PBHGetVol, WDPBPtr, 0xA014);

extern OSErr PBSetVol(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHSetVol(WDPBPtr pb, BOOLEAN async);
HFS_TRAP(PBSetVol, PBHSetVol, WDPBPtr, 0xA015);

extern OSErr PBEject(ParmBlkPtr pb);
REGISTER_TRAP2(PBEject, 0xA017, D0(A0));

extern OSErr PBGetFPos(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBGetFPos, 0xA018);

extern OSErr PBOffLine(ParmBlkPtr pb);
REGISTER_TRAP2(PBOffLine, 0xA035, D0(A0));

extern OSErr PBSetFLock(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHSetFLock(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBSetFLock, PBHSetFLock, HParmBlkPtr, 0xA041);

extern OSErr PBRstFLock(ParmBlkPtr pb, BOOLEAN async);
extern OSErr PBHRstFLock(HParmBlkPtr pb, BOOLEAN async);
HFS_TRAP(PBRstFLock, PBHRstFLock, HParmBlkPtr, 0xA042);

extern OSErr PBSetFVers(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBSetFVers, 0xA043);

extern OSErr PBSetFPos(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBSetFPos, 0xA044);

extern OSErr PBFlushFile(ParmBlkPtr pb, BOOLEAN async);
FILE_TRAP(PBFlushFile, 0xA045);


DISPATCHER_TRAP(FSDispatch, 0xA060, D0W);

extern OSErr PBOpenWD(WDPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBOpenWD, 0xA260, 0x0001, FSDispatch);

extern OSErr PBCloseWD(WDPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBCloseWD, 0xA260, 0x0002, FSDispatch);

extern OSErr PBCatMove(CMovePBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBCatMove, 0xA260, 0x0005, FSDispatch);

extern OSErr PBDirCreate(HParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBDirCreate, 0xA260, 0x0006, FSDispatch);

extern OSErr PBGetWDInfo(WDPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBGetWDInfo, 0xA260, 0x0007, FSDispatch);

extern OSErr PBGetFCBInfo(FCBPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBGetFCBInfo, 0xA260, 0x0008, FSDispatch);

extern OSErr PBGetCatInfo(CInfoPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBGetCatInfo, 0xA260, 9, FSDispatch);

extern OSErr PBSetCatInfo(CInfoPBPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBSetCatInfo, 0xA260, 10, FSDispatch);

extern OSErr PBSetVInfo(HParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBSetVInfo, 0xA260, 11, FSDispatch);

extern OSErr PBLockRange(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBLockRange, 0xA260, 0x10, FSDispatch);

extern OSErr PBUnlockRange(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBUnlockRange, 0xA260, 0x11, FSDispatch);

extern OSErr PBCreateFileIDRef(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBCreateFileIDRef, 0xA260, 0x14, FSDispatch);

extern OSErr PBDeleteFileIDRef(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBDeleteFileIDRef, 0xA260, 0x15, FSDispatch);

extern OSErr PBResolveFileIDRef(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBResolveFileIDRef, 0xA260, 0x16, FSDispatch);

extern OSErr PBExchangeFiles(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBExchangeFiles, 0xA260, 0x17, FSDispatch);

extern OSErr PBCatSearch(ParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBCatSearch, 0xA260, 0x18, FSDispatch);

extern OSErr PBOpenDF(HParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBOpenDF, 0xA260, 0x1A, FSDispatch);


extern OSErr PBHGetVolParms(HParmBlkPtr pb, BOOLEAN async);
FILE_SUBTRAP(PBHGetVolParms, 0xA260, 0x30, FSDispatch);

extern OSErr PBHGetLogInInfo(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHGetLogInInfo, 0xA260, 0x31, FSDispatch);

extern OSErr PBHGetDirAccess(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHGetDirAccess, 0xA260, 0x32, FSDispatch);

extern OSErr PBHSetDirAccess(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHSetDirAccess, 0xA260, 0x33, FSDispatch);

extern OSErr PBHMapID(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHMapID, 0xA260, 0x34, FSDispatch);

extern OSErr PBHMapName(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHMapName, 0xA260, 0x35, FSDispatch);

extern OSErr PBHCopyFile(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHCopyFile, 0xA260, 0x36, FSDispatch);

extern OSErr PBHMoveRename(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(PBHMoveRename, 0xA260, 0x37, FSDispatch);

extern OSErr OpenDeny(HParmBlkPtr pb, BOOLEAN a);
FILE_SUBTRAP(OpenDeny, 0xA260, 0x38, FSDispatch);

/* prototypes for the high level filesystem dispatch traps */
DISPATCHER_TRAP(HighLevelFSDispatch, 0xAA52, D0<0xF>);

extern OSErr C_FSMakeFSSpec(int16_t vRefNum, int32_t dir_id,
                                        Str255 file_name, FSSpecPtr spec);
PASCAL_SUBTRAP(FSMakeFSSpec, 0xAA52, 0x0001, HighLevelFSDispatch);
extern OSErr C_FSpExchangeFiles(FSSpecPtr src, FSSpecPtr dst);
PASCAL_SUBTRAP(FSpExchangeFiles, 0xAA52, 0x000F, HighLevelFSDispatch);
extern OSErr C_FSpOpenDF(FSSpecPtr spec, SignedByte perms,
                                     GUEST<int16_t> *refNum_out);
PASCAL_SUBTRAP(FSpOpenDF, 0xAA52, 0x0002, HighLevelFSDispatch);
extern OSErr C_FSpOpenRF(FSSpecPtr spec, SignedByte perms,
                                     GUEST<int16_t> *refNum_out);
PASCAL_SUBTRAP(FSpOpenRF, 0xAA52, 0x0003, HighLevelFSDispatch);
extern OSErr C_FSpCreate(FSSpecPtr spec,
                                     OSType creator, OSType file_type,
                                     ScriptCode script);
PASCAL_SUBTRAP(FSpCreate, 0xAA52, 0x0004, HighLevelFSDispatch);
extern OSErr C_FSpDirCreate(FSSpecPtr spec, ScriptCode script,
                                        GUEST<int32_t> *created_dir_id);
PASCAL_SUBTRAP(FSpDirCreate, 0xAA52, 0x0005, HighLevelFSDispatch);
extern OSErr C_FSpDelete(FSSpecPtr spec);
PASCAL_SUBTRAP(FSpDelete, 0xAA52, 0x0006, HighLevelFSDispatch);

extern OSErr C_FSpGetFInfo(FSSpecPtr spec, FInfo *fndr_info);
PASCAL_SUBTRAP(FSpGetFInfo, 0xAA52, 0x0007, HighLevelFSDispatch);
extern OSErr C_FSpSetFInfo(FSSpecPtr spec, FInfo *fndr_info);
PASCAL_SUBTRAP(FSpSetFInfo, 0xAA52, 0x0008, HighLevelFSDispatch);
extern OSErr C_FSpSetFLock(FSSpecPtr spec);
PASCAL_SUBTRAP(FSpSetFLock, 0xAA52, 0x0009, HighLevelFSDispatch);
extern OSErr C_FSpRstFLock(FSSpecPtr spec);
PASCAL_SUBTRAP(FSpRstFLock, 0xAA52, 0x000A, HighLevelFSDispatch);
extern OSErr C_FSpRename(FSSpecPtr spec, Str255 new_name);
PASCAL_SUBTRAP(FSpRename, 0xAA52, 0x000B, HighLevelFSDispatch);
extern OSErr C_FSpCatMove(FSSpecPtr src, FSSpecPtr dst);
PASCAL_SUBTRAP(FSpCatMove, 0xAA52, 0x000C, HighLevelFSDispatch);
extern void C_FSpCreateResFile(FSSpecPtr spec,
                                           OSType creator, OSType file_type,
                                           ScriptCode script);
PASCAL_SUBTRAP(FSpCreateResFile, 0xAA52, 0x000E, HighLevelFSDispatch);
extern INTEGER C_FSpOpenResFile(FSSpecPtr spec, SignedByte perms);
PASCAL_SUBTRAP(FSpOpenResFile, 0xAA52, 0x000D, HighLevelFSDispatch);
extern INTEGER C_HOpenResFile(INTEGER vref, LONGINT dirid,
                                          Str255 file_name, SignedByte perm);
PASCAL_TRAP(HOpenResFile, 0xA81A);
extern void C_HCreateResFile(INTEGER vrefnum, LONGINT parid,
                                         Str255 name);
PASCAL_TRAP(HCreateResFile, 0xA81B);

extern OSErr HCreate(INTEGER vref, LONGINT dirid, Str255 name, OSType creator,
                     OSType type);
extern OSErr HOpenRF(INTEGER vref, LONGINT dirid, Str255 name,
                     SignedByte perm, /*NATIVE*/ INTEGER *refp);

extern OSErr GetWDInfo(INTEGER wd, GUEST<INTEGER> *vrefp, GUEST<LONGINT> *dirp,
                       GUEST<LONGINT> *procp);
}
#endif /* _FILEMGR_H_ */

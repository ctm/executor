#if !defined (_FILEMGR_H_)
#define _FILEMGR_H_

#include "MacTypes.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: FileMgr.h 86 2005-05-25 00:47:12Z ctm $
 */

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

typedef struct {
    OSType fdType	PACKED;
    OSType fdCreator	PACKED;
    unsigned short fdFlags	PACKED;
    Point fdLocation	LPACKED;
    unsigned short fdFldr	PACKED;
} FInfo;

typedef struct {
    unsigned short fdIconID	PACKED;
    unsigned short fdUnused[4]	PACKED;
    unsigned short fdComment	PACKED;
    LONGINT fdPutAway	PACKED;
} FXInfo;

typedef struct {
    Rect frRect	LPACKED;
    unsigned short frFlags	PACKED;
    Point frLocation	LPACKED;
    unsigned short frView	PACKED;
} DInfo;

typedef struct {
    Point frScroll	LPACKED;
    LONGINT frOpenChain	PACKED;
    unsigned short frUnused	PACKED;
    unsigned short frComment	PACKED;
    LONGINT frPutAway	PACKED;
} DXInfo;

typedef enum {
    ioParamType,
    fileParamType,
    volumeParamType,
    cntrlParamType
} ParamBlkType;

#define COMMONFSQUEUEDEFS	\
    QElemPtr qLink       PACKED_P;	\
    INTEGER qType        PACKED;	\
    INTEGER ioTrap       PACKED;	\
    Ptr ioCmdAddr        PACKED_P;	\
    ProcPtr ioCompletion PACKED_P;	\
    OSErr ioResult       PACKED;	\
    StringPtr ioNamePtr  PACKED_P;	\
    INTEGER ioVRefNum   /* PACKED gets added in by guys who use this macro. */

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioRefNum	PACKED;
    SignedByte ioVersNum	LPACKED;
    SignedByte ioPermssn	LPACKED;
    LONGINT ioMisc	PACKED;		/* should be largest of Ptr, LONGINT */
    Ptr ioBuffer	PACKED_P;
    LONGINT ioReqCount	PACKED;
    LONGINT ioActCount	PACKED;
    INTEGER ioPosMode	PACKED;
    LONGINT ioPosOffset	PACKED;
} ioParam;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioFRefNum	PACKED;
    SignedByte ioFVersNum	LPACKED;
    SignedByte filler1	LPACKED;
    INTEGER ioFDirIndex	PACKED;
    SignedByte ioFlAttrib	LPACKED;
    SignedByte ioFlVersNum	LPACKED;
    FInfo ioFlFndrInfo	LPACKED;
    LONGINT ioFlNum	PACKED;
    INTEGER ioFlStBlk	PACKED;
    LONGINT ioFlLgLen	PACKED;
    LONGINT ioFlPyLen	PACKED;
    INTEGER ioFlRStBlk	PACKED;
    LONGINT ioFlRLgLen	PACKED;
    LONGINT ioFlRPyLen	PACKED;
    LONGINT ioFlCrDat	PACKED;
    LONGINT ioFlMdDat	PACKED;
} fileParam;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    LONGINT filler2	PACKED;
    INTEGER ioVolIndex	PACKED;
    LONGINT ioVCrDate	PACKED;
    LONGINT ioVLsBkUp	PACKED;
    unsigned short ioVAtrb	PACKED;
    unsigned short ioVNmFls	PACKED;
    unsigned short ioVDirSt	PACKED;
    unsigned short ioVBlLn	PACKED;
    unsigned short ioVNmAlBlks	PACKED;
    LONGINT ioVAlBlkSiz	PACKED;
    LONGINT ioVClpSiz	PACKED;
    unsigned short ioAlBlSt	PACKED;
    LONGINT ioVNxtFNum	PACKED;
    unsigned short ioVFrBlk	PACKED;
} volumeParam;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioCRefNum	PACKED;
    INTEGER csCode	PACKED;
    INTEGER csParam[11]	PACKED;
} cntrlParam;

typedef union {
    ioParam ioParam;
    fileParam fileParam;
    volumeParam volumeParam;
    cntrlParam cntrlParam;
} ParamBlockRec;
typedef ParamBlockRec *ParmBlkPtr;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioRefNum	PACKED;
    SignedByte ioVersNum	LPACKED;
    SignedByte ioPermssn	LPACKED;
    LONGINT ioMisc	PACKED;		/* should be largest of Ptr, LONGINT */
    Ptr ioBuffer	PACKED_P;
    LONGINT ioReqCount	PACKED;
    LONGINT ioActCount	PACKED;
    INTEGER ioPosMode	PACKED;
    LONGINT ioPosOffset	PACKED;
} HIoParam;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioFRefNum	PACKED;
    SignedByte ioFVersNum	LPACKED;
    SignedByte filler1	LPACKED;
    INTEGER ioFDirIndex	PACKED;
    SignedByte ioFlAttrib	LPACKED;
    SignedByte ioFlVersNum	LPACKED;
    FInfo ioFlFndrInfo	LPACKED;
/*-->*/ LONGINT ioDirID	PACKED;
    INTEGER ioFlStBlk	PACKED;
    LONGINT ioFlLgLen	PACKED;
    LONGINT ioFlPyLen	PACKED;
    INTEGER ioFlRStBlk	PACKED;
    LONGINT ioFlRLgLen	PACKED;
    LONGINT ioFlRPyLen	PACKED;
    LONGINT ioFlCrDat	PACKED;
    LONGINT ioFlMdDat	PACKED;
} HFileParam;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    LONGINT pfiller2	PACKED;
    INTEGER ioVolIndex	PACKED;
    LONGINT ioVCrDate	PACKED;
/*-->*/ LONGINT ioVLsMod	PACKED;
    INTEGER ioVAtrb	PACKED;
    unsigned short ioVNmFls	PACKED;
/*-->*/ unsigned short ioVBitMap	PACKED;
/*-->*/ unsigned short ioVAllocPtr	PACKED;
    unsigned short ioVNmAlBlks	PACKED;
    LONGINT ioVAlBlkSiz	PACKED;
    LONGINT ioVClpSiz	PACKED;
    unsigned short ioAlBlSt	PACKED;
/*-->*/ LONGINT ioVNxtCNID	PACKED;
    unsigned short ioVFrBlk	PACKED;
/*-->*/ unsigned short ioVSigWord	PACKED;
/*-->*/ INTEGER ioVDrvInfo	PACKED;
/*-->*/ INTEGER ioVDRefNum	PACKED;
/*-->*/ INTEGER ioVFSID	PACKED;
/*-->*/ LONGINT ioVBkUp	PACKED;
/*-->*/ unsigned short ioVSeqNum	PACKED;
/*-->*/ LONGINT ioVWrCnt	PACKED;
/*-->*/ LONGINT ioVFilCnt	PACKED;
/*-->*/ LONGINT ioVDirCnt	PACKED;
/*-->*/ LONGINT ioVFndrInfo[8]	PACKED;
} HVolumeParam;

typedef union {
    HIoParam ioParam;
    HFileParam fileParam;
    HVolumeParam volumeParam;
} HParamBlockRec;
typedef HParamBlockRec *HParmBlkPtr;

typedef enum { hfileInfo, dirInfo } CInfoType;

#define COMMONCINFODEFS			\
    COMMONFSQUEUEDEFS		PACKED;	\
    INTEGER ioFRefNum		PACKED;	\
    SignedByte ioFVersNum	LPACKED;	\
    SignedByte filler1		LPACKED;	\
    INTEGER ioFDirIndex		PACKED;	\
    SignedByte ioFlAttrib	LPACKED;	\
    SignedByte ioACUser   /* PACKED gets added in by guys who use this macro. */

typedef struct {
    COMMONCINFODEFS	LPACKED;
    FInfo ioFlFndrInfo	LPACKED;
    LONGINT ioDirID	PACKED;
    INTEGER ioFlStBlk	PACKED;
    LONGINT ioFlLgLen	PACKED;
    LONGINT ioFlPyLen	PACKED;
    INTEGER ioFlRStBlk	PACKED;
    LONGINT ioFlRLgLen	PACKED;
    LONGINT ioFlRPyLen	PACKED;
    LONGINT ioFlCrDat	PACKED;
    LONGINT ioFlMdDat	PACKED;
    LONGINT ioFlBkDat	PACKED;
    FXInfo ioFlXFndrInfo	LPACKED;
    LONGINT ioFlParID	PACKED;
    LONGINT ioFlClpSiz	PACKED;
} HFileInfo;

typedef struct {
    COMMONCINFODEFS	LPACKED;
    DInfo ioDrUsrWds	LPACKED;
    LONGINT ioDrDirID	PACKED;
    unsigned short ioDrNmFls	PACKED;
    unsigned short filler3[9]	PACKED;
    LONGINT ioDrCrDat	PACKED;
    LONGINT ioDrMdDat	PACKED;
    LONGINT ioDrBkDat	PACKED;
    DXInfo ioDrFndrInfo	LPACKED;
    LONGINT ioDrParID	PACKED;
} DirInfo;

typedef union {
    HFileInfo hFileInfo;
    DirInfo dirInfo;
} CInfoPBRec;
typedef CInfoPBRec *CInfoPBPtr;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    LONGINT filler1	PACKED;
    StringPtr ioNewName	PACKED_P;
    LONGINT filler2	PACKED;
    LONGINT ioNewDirID	PACKED;
    LONGINT filler3[2]	PACKED;
    LONGINT ioDirID	PACKED;
} CMovePBRec;
typedef CMovePBRec *CMovePBPtr;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    unsigned short filler1	PACKED;	/* should be INTEGER */
    INTEGER ioWDIndex	PACKED;
    LONGINT ioWDProcID	PACKED;
    INTEGER ioWDVRefNum	PACKED;
    INTEGER filler2[7]	PACKED;
    LONGINT ioWDDirID	PACKED;
} WDPBRec;
typedef WDPBRec *WDPBPtr;

typedef struct {
    COMMONFSQUEUEDEFS	PACKED;
    INTEGER ioRefNum	PACKED;
    unsigned short filler	PACKED;
    INTEGER ioFCBIndx	PACKED;
    INTEGER ioFCBobnoxiousfiller	PACKED;	/* ACK! not in IMIV	PACKED, but ThinkC+MPW */
    LONGINT ioFCBFlNm	PACKED;
    unsigned short ioFCBFlags	PACKED;
    INTEGER ioFCBStBlk	PACKED;
    LONGINT ioFCBEOF	PACKED;
    LONGINT ioFCBPLen	PACKED;
    LONGINT ioFCBCrPs	PACKED;
    INTEGER ioFCBVRefNum	PACKED;
    LONGINT ioFCBClpSiz	PACKED;
    LONGINT ioFCBParID	PACKED;
} FCBPBRec;
typedef FCBPBRec *FCBPBPtr;

typedef struct {
    QElemPtr qLink	PACKED_P;		/*  0 */
    INTEGER qType	PACKED;		/*  4 */
    unsigned short vcbFlags	PACKED;		/*  6 */
    unsigned short vcbSigWord	PACKED;		/*  8 */
    LONGINT vcbCrDate	PACKED;		/* 10 */
    LONGINT vcbLsMod	PACKED;		/* 14 */
    unsigned short vcbAtrb	PACKED;		/* 18 */
    unsigned short vcbNmFls	PACKED;		/* 20 */
    unsigned short vcbVBMSt	PACKED;		/* 22 */
    unsigned short vcbAllocPtr	PACKED;	/* 24 */
    unsigned short vcbNmAlBlks	PACKED;	/* 26 */
    LONGINT vcbAlBlkSiz	PACKED;	/* 28 */
    LONGINT vcbClpSiz	PACKED;		/* 32 */
    unsigned short vcbAlBlSt	PACKED;		/* 36 */
    LONGINT vcbNxtCNID	PACKED;		/* 38 */
    unsigned short vcbFreeBks	PACKED;		/* 42 */
    Byte vcbVN[28]	LPACKED;		/* 44 */
    INTEGER vcbDrvNum	PACKED;		/* 72 */
    INTEGER vcbDRefNum	PACKED;		/* 74 */
    INTEGER vcbFSID	PACKED;		/* 76 */
    INTEGER vcbVRefNum	PACKED;		/* 78 */
    Ptr vcbMAdr	PACKED_P;
    Ptr vcbBufAdr	PACKED_P;
    unsigned short vcbMLen	PACKED;
    INTEGER vcbDirIndex	PACKED;
    unsigned short vcbDirBlk	PACKED;
    LONGINT vcbVolBkUp	PACKED;
    unsigned short vcbVSeqNum	PACKED;
    LONGINT vcbWrCnt	PACKED;
    LONGINT vcbXTClpSiz	PACKED;
    LONGINT vcbCTClpSiz	PACKED;
    unsigned short vcbNmRtDirs	PACKED;
    LONGINT vcbFilCnt	PACKED;
    LONGINT vcbDirCnt	PACKED;
    LONGINT vcbFndrInfo[8]	PACKED;
    unsigned short vcbVCSize	PACKED;
    unsigned short vcbVBMCSiz	PACKED;
    unsigned short vcbCtlCSiz	PACKED;
    unsigned short vcbXTAlBlks	PACKED;
    unsigned short vcbCTAlBlks	PACKED;
    INTEGER vcbXTRef	PACKED;
    INTEGER vcbCTRef	PACKED;
    Ptr vcbCtlBuf	PACKED_P;
    LONGINT vcbDirIDM	PACKED;
    unsigned short vcbOffsM	PACKED;
} VCB;

typedef VCB *VCBPtr;
typedef struct { VCBPtr p PACKED_P; } HIDDEN_VCBPtr;

typedef struct {
    QElemPtr qLink	PACKED_P;
    INTEGER qType	PACKED;
    INTEGER dQDrive	PACKED;
    INTEGER dQRefNum	PACKED;
    INTEGER dQFSID	PACKED;
    unsigned short dQDrvSz	PACKED;
    unsigned short dQDrvSz2	PACKED;
} DrvQEl;

/* data types introduced by the new high level file system dispatch
   traps */

struct FSSpec
{
  INTEGER vRefNum PACKED;
  LONGINT parID PACKED;
  Str63 name LPACKED;
};

typedef struct FSSpec FSSpec;
typedef FSSpec *FSSpecPtr;
typedef FSSpecPtr FSSpecArrayPtr;

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

#endif /* _FILEMGR_H_ */

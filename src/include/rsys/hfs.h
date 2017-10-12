#if !defined(__RSYS_HFS__)
#define __RSYS_HFS__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: hfs.h 87 2005-05-25 01:57:33Z ctm $
 */

#define DOSFDBIT 0x40000000	/* or this into a fd to show that we
				   need to use Mat's DOS routines */
#define ASPIFDBIT 0x20000000	/* or this into a fd to show that we
				   need to use ctm's ASPI routines */

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#include "rsys/cruft.h"

#include "OSUtil.h"
#include "FileMgr.h"

#include "rsys/file.h"

#if !defined(UNIX)
#include <string.h>
#endif

#include "ThinkC.h"
#define HVCB VCB

namespace Executor {

#define DEVNAMELEN	50		/* much bigger than necessary */

#define FLOPPYDREF	-5
#define OURHFSDREF	(-101)
#define PHYSBSIZE	512

#define MADROFFSET	40

typedef struct PACKED {
  unsigned short blockstart;
  unsigned short blockcount;
} xtntdesc, xtntrec[3];	/* WILL NEED Cx() */

typedef struct PACKED {
  unsigned short drSigWord;	/* 0 */
  LONGINT drCrDate;		/* 2 */
  LONGINT drLsMod;		/* 6 */
  unsigned short drAtrb;	/* 10 */
  unsigned short drNmFls;	/* 12 */
  unsigned short drVBMSt;	/* 14 */
  unsigned short drAllocPtr;	/* 16 */
  unsigned short drNmAlBlks;	/* 18 */
  LONGINT drAlBlkSiz;		/* 20 */
  LONGINT drClpSiz;
  unsigned short drAlBlSt;
  LONGINT drNxtCNID;
  unsigned short drFreeBks;
  unsigned char drVN[28];
  LONGINT drVolBkUp;
  unsigned short drVSeqNum;
  LONGINT drWrCnt;
  LONGINT drXTClpSiz;
  LONGINT drCTClpSiz;
  unsigned short drNmRtDirs;
  LONGINT drFilCnt;
  LONGINT drDirCnt;
  LONGINT drFndrInfo[8];
  unsigned short drVCSize;
  unsigned short drVCBMSize;
  unsigned short drCtlCSize;
  LONGINT drXTFlSize;
  xtntrec drXTExtRec;
  LONGINT drCTFlSize;
  xtntrec drCTExtRec;
} volumeinfo, *volumeinfoPtr;
MAKE_HIDDEN(volumeinfoPtr);
typedef HIDDEN_volumeinfoPtr *volumeinfoHandle;

#define VOLUMEINFOBLOCKNO   2
#define InternalDrive   1
#define ROUNDUP8(x) ((x+7)/8*8)
#define NPHYSREQ(x) ((x+PHYSBSIZE-1)/PHYSBSIZE)

typedef struct PACKED {
  LONGINT ndFLink;
  LONGINT ndBLink;
  unsigned char ndType;
  char ndLevel;
  short ndNRecs;
  unsigned short idunno;
} btnode;

typedef enum { indexnode, mapnode = 2, leafnode = 0xFF } btnodetype;

typedef struct PACKED {
  unsigned char ckrKeyLen;
  char ckrResrv1;
  LONGINT ckrParID;
  unsigned char ckrCName[32];
} catkey;

typedef struct PACKED {
  unsigned char xkrKeyLen;
  unsigned char xkrFkType;
  LONGINT xkrFNum;
  unsigned short xkrFABN;
} xtntkey;

typedef union {
    unsigned char keylen;
    catkey catk;
    xtntkey xtntk;
} anykey;

#define FILETYPE    2

typedef struct PACKED {
  char cdrType;
  char cdrResrv2;
  char filFlags;
  char filTyp;
  FInfo filUsrWds;    /* not sure what form */
  LONGINT filFlNum;
  unsigned short filStBlk; /* I don't think this is used */
  LONGINT filLgLen;
  LONGINT filPyLen;
  unsigned short filRStBlk;    /* not used? */
  LONGINT filRLgLen;
  LONGINT filRPyLen;
  LONGINT filCrDat;
  LONGINT filMdDat;
  LONGINT filBkDat;
  LONGINT filFndrInfo[4];
  unsigned short filClpSize;
  xtntrec filExtRec;
  xtntrec filRExtRec;
  LONGINT filResrv;
} filerec;

#define DIRTYPE 1

typedef struct PACKED {
  char cdrType;
  char cdrResrv2;
  unsigned short dirFlags;
  unsigned short dirVal;
  LONGINT dirDirID;
  LONGINT dirCrDat;
  LONGINT dirMdDat;
  LONGINT dirBkDat;
  LONGINT dirUsrInfo[4];
  LONGINT dirFndrInfo[4];
  LONGINT dirResrv[4];
} directoryrec;

#define THREADTYPE  3

typedef struct PACKED {
  char cdrType;
  char cdrResrv2;
  char thdResrv[8];
  LONGINT thdParID;
  unsigned char thdCName[32];
} threadrec;

typedef enum { firstisless = -1, same, firstisgreater } compretval;

typedef compretval (*compfp)(void *first, void *second);

#define WRITEBIT    (1<<0)
#define RESOURCEBIT (1<<1)
#define SHAREDBIT   (1<<4)
#define FLOCKEDBIT  (1<<5)
#define DIRTYBIT    (1<<7)

typedef HVCB *HVCBPtr;

typedef struct PACKED {
  LONGINT fcbFlNum;
  Byte fcbMdRByt;
  Byte fcbTypByt;
  unsigned short fcbSBlk;
  LONGINT fcbEOF;
  LONGINT fcbPLen;
  LONGINT fcbCrPs;
  PACKED_MEMBER(HVCBPtr, fcbVPtr);
  PACKED_MEMBER(Ptr, fcbBfAdr);
  unsigned short fcbFlPos;
  LONGINT fcbClmpSize;
  LONGINT fcbBTCBPtr;
  xtntrec fcbExtRec;
  LONGINT fcbFType;
  ULONGINT fcbCatPos;
  LONGINT fcbDirID;
  unsigned char fcbCName[32];
} filecontrolblock;

enum { datafork, resourcefork = 0xFF };
typedef unsigned char Forktype;

typedef enum { databusy, resourcebusy, eitherbusy } busyconcern_t;

typedef enum { reading, writing } accesstype;

#define VSOFTLOCKBIT    (1<<15)
#define VHARDLOCKBIT    (1<<7)
#define VNONEJECTABLEBIT	(1<<5)
#define FSOFTLOCKBIT    (1<<0)
#define FILEFLAGSUSERSETTABLEMASK   FSOFTLOCKBIT

typedef struct PACKED {
    LONGINT    flink;              /* 0 */
    LONGINT    blink;              /* 4 */
    unsigned char type;         /* 8 */
    unsigned char dummy;        /* 9 */
    unsigned short   hesthreejim;        /* 10 */
    INTEGER macdisk_uses_it; /* 12 */
    INTEGER    height;             /* 14 */
    LONGINT    root;               /* 16 */
    LONGINT    numentries;         /* 20 */
    ULONGINT    firstleaf;          /* 24 */
    ULONGINT    lastleaf;           /* 28 */
    unsigned short   btnodesize;         /* 32 */
    unsigned short   indexkeylen;        /* 34 */
    LONGINT    nnodes;             /* 36 */
    LONGINT    nfreenodes;         /* 40 */
    unsigned char reserved[72]; /* 44 */
    unsigned char dummy2[132];  /* 116 */
    unsigned char map[256];     /* 248 */
    LONGINT unknown2[2];           /* 504 */
} btblock0;

#define DATAPFROMKEY(p) ((char *)(p) + ((((catkey *)p)->ckrKeyLen + 2) & ~1))


typedef enum { mfs, hfs, hfsp } fstype;
typedef enum { regular = 1, directory = 2, thread = 4 } filekind;

#if defined(MAC)
#define THINKCMESSED
#endif /* defined(MAC) */

#if defined (THINKCMESSED)
#define vcbClpSiz   vcbClpSIz
#endif /* THINKCMESSED */

/*
 * TODO: do the ioCompletion routine when necessary
 */
 
#define PBRETURN(pb, x) return (((ParmBlkPtr) (pb))->ioParam.ioResult = CW(x), (x))

typedef struct _cacheentry *cacheentry_ptr;

typedef struct PACKED _cacheentry {
  PACKED_MEMBER(cacheentry_ptr, flink);
  PACKED_MEMBER(cacheentry_ptr, blink);
  PACKED_MEMBER(HVCBPtr, vptr);
  LONGINT fileno;
  uint16 refnum;
  ULONGINT physblock;
  ULONGINT logblk;
  unsigned char flags;
  Forktype forktype;
  char buf[PHYSBSIZE];
} cacheentry;

#define CACHEDIRTY  (1 << 7)
#define CACHEBUSY   (1 << 6)
#define CACHEFREE   (1 << 5)

typedef struct PACKED {
  PACKED_MEMBER(cacheentry_ptr, flink);
  PACKED_MEMBER(cacheentry_ptr, blink);
  unsigned short   nitems;
  uint16  flags;
} cachehead;

#define NCACHEENTRIES   16

#define MAXTRAILS   8

typedef struct {
  cacheentry *cachep;
  unsigned short logbno;
  unsigned short after;
} trailentry;

/*
 * a btblock is used to save the state as a b-tree is walked to search for
 * a node.  The saved state can then be used to do an insert, rename or delete.
 * The first cut of code had all these arguments being pushed and popped on
 * the stack as arguments.
 */
 
typedef struct {
    HVCB *vcbp;                     /* in */
    anykey tofind;                  /* in */
    compfp fp;                      /* in */
    unsigned short refnum;                   /* in */
    BOOLEAN success;                /* out */
    anykey *foundp;                 /* out */
    short leafindex;                /* out */
    trailentry trail[MAXTRAILS];    /* out */
} btparam;

typedef struct PACKED {    /* from MPW equates */
    PACKED_MEMBER(HVCBPtr, vcbp);
    LONGINT dirid;
    LONGINT cathint;    /* ??? */
    LONGINT procid;
} wdentry;

#if defined(MAC)
extern Ptr WDCBsPtr : 0x372;
extern LONGINT BufTgFNum : 0x2FC;
extern INTEGER BufTgFFlag : 0x300;
extern INTEGER BufTgFBkNum : 0x302;
extern LONGINT BufTgDate : 0x304;
extern LONGINT TFSTagData0 : 0x38A;
extern LONGINT TFSTagData1 : 0x38E;
#else

#if !defined (WDCBsPtr_H)
extern HIDDEN_Ptr WDCBsPtr_H;
extern LONGINT BufTgFNum;
extern INTEGER BufTgFFlag;
extern INTEGER BufTgFBkNum;
extern LONGINT BufTgDate;
extern LONGINT TFSTagData0;
extern LONGINT TFSTagData1;
extern INTEGER SCSIFlags;
#endif

#define WDCBsPtr	(WDCBsPtr_H.p)

#endif

#define WDMASK  0xC001
#define WDMAGIC 0x8001

#define ISWDNUM(v)						\
({								\
  uint16 _v;							\
								\
  _v = (v);							\
  (_v & WDMASK) == WDMAGIC ?					\
    (_v ^ WDMAGIC) % sizeof (wdentry) == sizeof (INTEGER)	\
  :								\
    FALSE;							\
})

#define WDNUMTOWDP(v) ((wdentry *)(MR(WDCBsPtr) + (INTEGER) ((v) ^ WDMAGIC)))
#define WDPTOWDNUM(p)   (((char *) (p) - (char *)MR(WDCBsPtr)) ^ WDMAGIC)

typedef enum { seteof, allocany, alloccontig } alloctype;

extern compretval ROMlib_catcompare(void *firstp, void *secondp);

typedef enum { GETCACHESAVE = 1, GETCACHENOREAD = 2 } cacheflagtype;

#define VCBDIRTY    (1 << 15)

typedef enum {NOTE, CAUTION, STOP} alerttype;

enum { INHERITED_FLAG_BITS = ATTRIB_ISLOCKED }; 

#if !defined(MAC)
#define CurTime	(GetDateTime(&Time), Cx(Time))
#else
#define CurTime Cx(Time)
#endif

#define EJECTALERTID	(-4061)

/* public entities in btree.c */

extern cacheentry *ROMlib_addrtocachep(Ptr addr, HVCB *vcbp);
extern BOOLEAN ROMlib_searchnode(btnode *btp, void *key, compfp fp, anykey **keypp,
							       INTEGER *afterp);
extern OSErr ROMlib_getcache(cacheentry **retpp, uint16 refnum, ULONGINT logbno,
							   cacheflagtype flags);
extern OSErr ROMlib_putcache(cacheentry *cachep);
extern OSErr ROMlib_keyfind(btparam *btpb);
extern OSErr ROMlib_btnext(anykey **nextpp, anykey *keyp, HVCB *vcbp);
extern OSErr ROMlib_btdelete(btparam *btpb);
extern OSErr ROMlib_makecatparam(btparam *btpb, HVCB *vcbp, LONGINT dirid,
						    INTEGER namelen, Ptr namep);
extern OSErr ROMlib_errortype(btparam *btpb);
extern OSErr ROMlib_filedelete(btparam *btpb, filekind kind);
extern OSErr ROMlib_dirdelete(btparam *btpb);
extern OSErr ROMlib_dirtyleaf(void *p, HVCB *vcbp);
extern OSErr ROMlib_filecreate(btparam *btpb, void *data, filekind kind);
extern OSErr ROMlib_dircreate(btparam *btpb, directoryrec *data);
extern OSErr ROMlib_dircreate(btparam *btpb, directoryrec *data);
extern xtntkey *ROMlib_newextentrecord(filecontrolblock *fcbp, uint16 newabn);
extern OSErr ROMlib_btrename(btparam *btpb, StringPtr newnamep);
extern OSErr ROMlib_btcreateemptyfile(btparam *btpb);
extern OSErr ROMlib_btcreateemptydir(btparam *btpb, LONGINT *newidp);
extern OSErr ROMlib_btpbindex (IOParam *pb, LONGINT dirid, HVCB **vcbpp,
			  filerec **frpp, catkey **catkeypp, BOOLEAN onlyfiles);
extern OSErr ROMlib_cleancache(HVCB *vcbp);
extern OSErr ROMlib_flushcachevcbp(HVCB *vcbp);

/* public entries in changing.c */

extern INTEGER ROMlib_flnumtorefnum(ULONGINT flnum, VCB *vcbp);

/* public entries in file.c */

extern filecontrolblock *ROMlib_getfreefcbp( void );
extern filecontrolblock *ROMlib_refnumtofcbp(uint16 refnum);
extern compretval ROMlib_xtntcompare(void *firstp, void *secondp);
extern compretval ROMlib_catcompare(void *firstp, void *secondp);
extern void ROMlib_makextntkey(xtntkey *keyp, Forktype forkwanted, LONGINT flnum,
								    uint16 bno);
extern void ROMlib_makextntparam(btparam *btpb, HVCB *vcbp, Forktype forkwanted,
						     LONGINT flnum, uint16 bno);
extern LONGINT ROMlib_logtophys(filecontrolblock *fcbp, LONGINT absoffset,
							   LONGINT *nphyscontigp);
extern OSErr ROMlib_makecatkey(catkey *keyp, LONGINT dirid, INTEGER namelen, Ptr namep);
extern OSErr ROMlib_findvcbandfile(IOParam *pb, LONGINT dirid, btparam *btpb,
					   filekind *kindp, BOOLEAN ignorename);
extern OSErr ROMlib_alreadyopen(HVCB *vcbp, LONGINT flnum, SignedByte *permp,
					 INTEGER *refnump, busyconcern_t busy);
extern OSErr ROMlib_allochelper(IOParam *pb, BOOLEAN async, alloctype alloc,
						     BOOLEAN ROMlib_writefcbp);

/* public entries in helper.c */

extern void OurExit( void );
extern OSErr ROMlib_transphysblk(hfs_access_t *hfsp, LONGINT physblock,
				 short nphysblocks,
				 Ptr bufp, accesstype rw, LONGINT *actp);
extern char *ROMlib_indexn(char *str, char tofind, INTEGER length);
#if defined(MAC)
extern void str255assign(StringPtr dstp, StringPtr srcp);
#endif
extern void *ROMlib_indexqueue(QHdr *qp, short index);
extern OSErr ROMlib_writefcbp(filecontrolblock *fcbp);
extern OSErr ROMlib_writevcbp(HVCB *vcbp);
extern void ROMlib_openfloppy( const char *dname, LONGINT *messp );
extern void ROMlib_openharddisk( const char *dname, LONGINT *messp );
extern void ROMlib_hfsinit( void );
extern OSErr ROMlib_ejectfloppy( LONGINT floppyfd );
extern OSErr ROMlib_readwrite( LONGINT fd, char *buffer, LONGINT count, LONGINT off,
			     accesstype rw, LONGINT blocksize, LONGINT maxtransfer );

extern LONGINT ROMlib_sock;

/* public entries in volume.c */

extern HVCB *ROMlib_findvcb(short vrefnum, StringPtr name, LONGINT *diridp,
							   BOOLEAN usedefault);
extern OSErr ROMlib_mkwd(WDPBPtr pb, HVCB *vcbp, LONGINT dirid,
							        LONGINT procid);
extern OSErr ROMlib_pbvolrename(IOParam *pb, StringPtr newnamep);
extern OSErr ROMlib_flushvcbp(HVCB *vcbp);
extern HVCB *ROMlib_vcbbyvrn(short vrefnum);
extern VCBExtra *ROMlib_vcbbyunixname(char *uname);
extern HVCB *ROMlib_vcbbybiggestunixname(const char *uname);
extern HVCB *ROMlib_vcbbydrive(short vrefnum);

extern DrvQExtra *ROMlib_dqbydrive (short vrefnum);

/* public entries in workingdir.c */

extern OSErr ROMlib_dirbusy(LONGINT dirid, HVCB *vcbp);
extern void ROMlib_adjustdirid(LONGINT *diridp, HVCB *vcbp, INTEGER vrefnum);

#if defined(MAC)
/* public entries in misc.c */
extern void bcopy(void *srcp, void *dstp, LONGINT length);
extern void bzero(void *dstp, LONGINT ntozero);
#endif

/* public entry in error.c */
extern void errormessage(StringPtr msg, alerttype severity);

enum { NEWLINEMODE = 1 << 7 };

#if !defined (__STDC__)
extern OSErrRET hfsPBOpen(); 
extern OSErrRET hfsPBHOpen(); 
extern OSErrRET hfsPBOpenRF(); 
extern OSErrRET hfsPBHOpenRF();
extern OSErrRET hfsPBLockRange();
extern OSErrRET hfsPBUnlockRange();
extern OSErrRET hfsPBRead(); 
extern OSErrRET hfsPBWrite(); 
extern OSErrRET hfsPBGetFPos();
extern OSErrRET hfsPBSetFPos();
extern OSErrRET hfsPBGetEOF(); 
extern OSErrRET hfsPBSetEOF(); 
extern OSErrRET hfsPBAllocate();
extern OSErrRET hfsPBAllocContig();
extern OSErrRET hfsPBFlushFile();
extern OSErrRET hfsPBClose(); 
extern OSErrRET hfsPBCreate(); 
extern OSErrRET hfsPBHCreate();
extern OSErrRET hfsPBDirCreate();
extern OSErrRET hfsPBDelete(); 
extern OSErrRET hfsPBHDelete();
extern OSErrRET hfsPBGetCatInfo();
extern OSErrRET hfsPBSetCatInfo();
extern OSErrRET hfsPBCatMove();
extern OSErrRET hfsPBOpenWD(); 
extern OSErrRET hfsPBCloseWD(); 
extern OSErrRET hfsPBGetWDInfo(); 
extern OSErrRET hfsPBGetFInfo();
extern OSErrRET hfsPBHGetFInfo();
extern OSErrRET hfsPBSetFInfo();
extern OSErrRET hfsPBHSetFInfo();
extern OSErrRET hfsPBSetFLock();
extern OSErrRET hfsPBHSetFLock();
extern OSErrRET hfsPBRstFLock();
extern OSErrRET hfsPBHRstFLock();
extern OSErrRET hfsPBSetFVers();
extern OSErrRET hfsPBRename();
extern OSErrRET hfsPBHRename();
extern OSErrRET hfsPBGetFCBInfo();
extern OSErr hfsPBMountVol(); 
extern OSErrRET hfsPBGetVInfo();
extern OSErrRET hfsPBHGetVInfo();
extern OSErrRET hfsPBSetVInfo();
extern OSErrRET hfsPBGetVol(); 
extern OSErrRET hfsPBHGetVol(); 
extern OSErrRET hfsPBSetVol(); 
extern OSErrRET hfsPBHSetVol(); 
extern OSErrRET hfsPBFlushVol();
extern OSErrRET hfsPBUnmountVol(); 
extern OSErrRET hfsPBOffLine(); 
extern OSErrRET hfsPBEject(); 
#else /* __STDC__ */
extern OSErr hfsPBOpen( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHOpen( HParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBOpenRF( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHOpenRF( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBLockRange( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBUnlockRange( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBRead( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBWrite( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBGetFPos( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetFPos( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBGetEOF( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBSetEOF( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBAllocate( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBAllocContig( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBFlushFile( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBClose( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBCreate( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHCreate( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBDirCreate( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBDelete( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHDelete( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBGetCatInfo( CInfoPBPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetCatInfo( CInfoPBPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBCatMove( CMovePBPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBOpenWD( WDPBPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBCloseWD( WDPBPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBGetWDInfo( WDPBPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBGetFInfo( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHGetFInfo( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetFInfo( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHSetFInfo( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetFLock( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHSetFLock( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBRstFLock( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHRstFLock( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetFVers( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBRename( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHRename( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBGetFCBInfo( FCBPBPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBMountVol( ParmBlkPtr ufsPB, LONGINT floppyfd, LONGINT off,
			    LONGINT bsize, LONGINT maxbytes,
			   drive_flags_t flags, DrvQExtra *dqp );
extern OSErr hfsPBGetVInfo( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBHGetVInfo( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBSetVInfo( HParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBGetVol( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHGetVol( WDPBPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBSetVol( ParmBlkPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBHSetVol( WDPBPtr ufsPB, BOOLEAN a ); 
extern OSErr hfsPBFlushVol( ParmBlkPtr ufsPB, BOOLEAN a );
extern OSErr hfsPBUnmountVol( ParmBlkPtr ufsPB ); 
extern OSErr hfsPBOffLine( ParmBlkPtr ufsPB ); 
extern OSErr hfsPBEject( ParmBlkPtr ufsPB ); 

extern void try_to_mount_disk( const char *dname, LONGINT floppyfd,
			       LONGINT *messp, LONGINT bsize,
			       LONGINT maxbytes, drive_flags_t flags,
			       uint32 offset );

extern void ROMlib_OurClose( void );

extern long ROMlib_priv_open(const char *filename, long mode);
extern void vcbsync(HVCB *vcbp);

#endif /* __STDC__ */

#endif
}
#endif /* !defined(__RSYS_HFS__) */

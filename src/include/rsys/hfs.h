#if !defined(__RSYS_HFS__)
#define __RSYS_HFS__

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define DOSFDBIT 0x40000000 /* or this into a fd to show that we \
                   need to use Mat's DOS routines */
#define ASPIFDBIT 0x20000000 /* or this into a fd to show that we \
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

namespace Executor
{

#define DEVNAMELEN 50 /* much bigger than necessary */

#define FLOPPYDREF -5
#define OURHFSDREF (-101)
#define PHYSBSIZE 512

#define MADROFFSET 40

typedef struct xtntdesc
{
    GUEST_STRUCT;
    GUEST<unsigned short> blockstart;
    GUEST<unsigned short> blockcount;
} xtntrec[3];

typedef struct volumeinfo
{
    GUEST_STRUCT;
    GUEST<unsigned short> drSigWord; /* 0 */
    GUEST<LONGINT> drCrDate; /* 2 */
    GUEST<LONGINT> drLsMod; /* 6 */
    GUEST<unsigned short> drAtrb; /* 10 */
    GUEST<unsigned short> drNmFls; /* 12 */
    GUEST<unsigned short> drVBMSt; /* 14 */
    GUEST<unsigned short> drAllocPtr; /* 16 */
    GUEST<unsigned short> drNmAlBlks; /* 18 */
    GUEST<LONGINT> drAlBlkSiz; /* 20 */
    GUEST<LONGINT> drClpSiz;
    GUEST<unsigned short> drAlBlSt;
    GUEST<LONGINT> drNxtCNID;
    GUEST<unsigned short> drFreeBks;
    GUEST<unsigned char[28]> drVN;
    GUEST<LONGINT> drVolBkUp;
    GUEST<unsigned short> drVSeqNum;
    GUEST<LONGINT> drWrCnt;
    GUEST<LONGINT> drXTClpSiz;
    GUEST<LONGINT> drCTClpSiz;
    GUEST<unsigned short> drNmRtDirs;
    GUEST<LONGINT> drFilCnt;
    GUEST<LONGINT> drDirCnt;
    GUEST<LONGINT[8]> drFndrInfo;
    GUEST<unsigned short> drVCSize;
    GUEST<unsigned short> drVCBMSize;
    GUEST<unsigned short> drCtlCSize;
    GUEST<LONGINT> drXTFlSize;
    GUEST<xtntrec> drXTExtRec;
    GUEST<LONGINT> drCTFlSize;
    GUEST<xtntrec> drCTExtRec;
} * volumeinfoPtr;

typedef GUEST<volumeinfoPtr> *volumeinfoHandle;

#define VOLUMEINFOBLOCKNO 2
#define InternalDrive 1
#define ROUNDUP8(x) ((x + 7) / 8 * 8)
#define NPHYSREQ(x) ((x + PHYSBSIZE - 1) / PHYSBSIZE)

struct btnode
{
    GUEST_STRUCT;
    GUEST<LONGINT> ndFLink;
    GUEST<LONGINT> ndBLink;
    GUEST<unsigned char> ndType;
    GUEST<char> ndLevel;
    GUEST<short> ndNRecs;
    GUEST<unsigned short> idunno;
};

typedef enum { indexnode,
               mapnode = 2,
               leafnode = 0xFF } btnodetype;

struct catkey
{
    GUEST_STRUCT;
    GUEST<unsigned char> ckrKeyLen;
    GUEST<char> ckrResrv1;
    GUEST<LONGINT> ckrParID;
    GUEST<unsigned char[32]> ckrCName;
};

struct xtntkey
{
    GUEST_STRUCT;
    GUEST<unsigned char> xkrKeyLen;
    GUEST<unsigned char> xkrFkType;
    GUEST<LONGINT> xkrFNum;
    GUEST<unsigned short> xkrFABN;
};

typedef union {
    unsigned char keylen;
    catkey catk;
    xtntkey xtntk;
} anykey;

#define FILETYPE 2

struct filerec
{
    GUEST_STRUCT;
    GUEST<char> cdrType;
    GUEST<char> cdrResrv2;
    GUEST<char> filFlags;
    GUEST<char> filTyp;
    GUEST<FInfo> filUsrWds; /* not sure what form */
    GUEST<LONGINT> filFlNum;
    GUEST<unsigned short> filStBlk; /* I don't think this is used */
    GUEST<LONGINT> filLgLen;
    GUEST<LONGINT> filPyLen;
    GUEST<unsigned short> filRStBlk; /* not used? */
    GUEST<LONGINT> filRLgLen;
    GUEST<LONGINT> filRPyLen;
    GUEST<LONGINT> filCrDat;
    GUEST<LONGINT> filMdDat;
    GUEST<LONGINT> filBkDat;
    GUEST<LONGINT[4]> filFndrInfo;
    GUEST<unsigned short> filClpSize;
    GUEST<xtntrec> filExtRec;
    GUEST<xtntrec> filRExtRec;
    GUEST<LONGINT> filResrv;
};

#define DIRTYPE 1

struct directoryrec
{
    GUEST_STRUCT;
    GUEST<char> cdrType;
    GUEST<char> cdrResrv2;
    GUEST<unsigned short> dirFlags;
    GUEST<unsigned short> dirVal;
    GUEST<LONGINT> dirDirID;
    GUEST<LONGINT> dirCrDat;
    GUEST<LONGINT> dirMdDat;
    GUEST<LONGINT> dirBkDat;
    GUEST<LONGINT[4]> dirUsrInfo;
    GUEST<LONGINT[4]> dirFndrInfo;
    GUEST<LONGINT[4]> dirResrv;
};

#define THREADTYPE 3

struct threadrec
{
    GUEST_STRUCT;
    GUEST<char> cdrType;
    GUEST<char> cdrResrv2;
    GUEST<char[8]> thdResrv;
    GUEST<LONGINT> thdParID;
    GUEST<unsigned char[32]> thdCName;
};

typedef enum { firstisless = -1,
               same,
               firstisgreater } compretval;

typedef compretval (*compfp)(void *first, void *second);

#define WRITEBIT (1 << 0)
#define RESOURCEBIT (1 << 1)
#define SHAREDBIT (1 << 4)
#define FLOCKEDBIT (1 << 5)
#define DIRTYBIT (1 << 7)

typedef HVCB *HVCBPtr;

struct filecontrolblock
{
    GUEST_STRUCT;
    GUEST<LONGINT> fcbFlNum;
    GUEST<Byte> fcbMdRByt;
    GUEST<Byte> fcbTypByt;
    GUEST<unsigned short> fcbSBlk;
    GUEST<LONGINT> fcbEOF;
    GUEST<LONGINT> fcbPLen;
    GUEST<LONGINT> fcbCrPs;
    GUEST<HVCBPtr> fcbVPtr;
    GUEST<Ptr> fcbBfAdr;
    GUEST<unsigned short> fcbFlPos;
    GUEST<LONGINT> fcbClmpSize;
    GUEST<LONGINT> fcbBTCBPtr;
    GUEST<xtntrec> fcbExtRec;
    GUEST<LONGINT> fcbFType;
    GUEST<ULONGINT> fcbCatPos;
    GUEST<LONGINT> fcbDirID;
    GUEST<unsigned char[32]> fcbCName;
};

enum
{
    datafork,
    resourcefork = 0xFF
};
typedef unsigned char Forktype;

typedef enum { databusy,
               resourcebusy,
               eitherbusy } busyconcern_t;

typedef enum { reading,
               writing } accesstype;

#define VSOFTLOCKBIT (1 << 15)
#define VHARDLOCKBIT (1 << 7)
#define VNONEJECTABLEBIT (1 << 5)
#define FSOFTLOCKBIT (1 << 0)
#define FILEFLAGSUSERSETTABLEMASK FSOFTLOCKBIT

struct btblock0
{
    GUEST_STRUCT;
    GUEST<LONGINT> flink; /* 0 */
    GUEST<LONGINT> blink; /* 4 */
    GUEST<unsigned char> type; /* 8 */
    GUEST<unsigned char> dummy; /* 9 */
    GUEST<unsigned short> hesthreejim; /* 10 */
    GUEST<INTEGER> macdisk_uses_it; /* 12 */
    GUEST<INTEGER> height; /* 14 */
    GUEST<LONGINT> root; /* 16 */
    GUEST<LONGINT> numentries; /* 20 */
    GUEST<ULONGINT> firstleaf; /* 24 */
    GUEST<ULONGINT> lastleaf; /* 28 */
    GUEST<unsigned short> btnodesize; /* 32 */
    GUEST<unsigned short> indexkeylen; /* 34 */
    GUEST<LONGINT> nnodes; /* 36 */
    GUEST<LONGINT> nfreenodes; /* 40 */
    GUEST<unsigned char[72]> reserved; /* 44 */
    GUEST<unsigned char[132]> dummy2; /* 116 */
    GUEST<unsigned char[256]> map; /* 248 */
    GUEST<LONGINT[2]> unknown2; /* 504 */
};

#define DATAPFROMKEY(p) ((char *)(p) + ((((catkey *)p)->ckrKeyLen + 2) & ~1))

typedef enum { mfs,
               hfs,
               hfsp } fstype;
typedef enum { regular = 1,
               directory = 2,
               thread = 4 } filekind;

#if defined(MAC)
#define THINKCMESSED
#endif /* defined(MAC) */

#if defined(THINKCMESSED)
#define vcbClpSiz vcbClpSIz
#endif /* THINKCMESSED */

/*
 * TODO: do the ioCompletion routine when necessary
 */

#define PBRETURN(pb, x) return (((ParmBlkPtr)(pb))->ioParam.ioResult = CW(x), (x))

typedef struct _cacheentry *cacheentry_ptr;

typedef struct _cacheentry
{
    GUEST_STRUCT;
    GUEST<cacheentry_ptr> flink;
    GUEST<cacheentry_ptr> blink;
    GUEST<HVCBPtr> vptr;
    GUEST<LONGINT> fileno;
    GUEST<uint16_t> refnum;
    GUEST<ULONGINT> physblock;
    GUEST<ULONGINT> logblk;
    GUEST<unsigned char> flags;
    GUEST<Forktype> forktype;
    GUEST<char[PHYSBSIZE]> buf;
} cacheentry;

#define CACHEDIRTY (1 << 7)
#define CACHEBUSY (1 << 6)
#define CACHEFREE (1 << 5)

struct cachehead
{
    GUEST_STRUCT;
    GUEST<cacheentry_ptr> flink;
    GUEST<cacheentry_ptr> blink;
    GUEST<unsigned short> nitems;
    GUEST<uint16_t> flags;
};

#define NCACHEENTRIES 16

#define MAXTRAILS 8

typedef struct
{
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

typedef struct
{
    HVCB *vcbp; /* in */
    anykey tofind; /* in */
    compfp fp; /* in */
    unsigned short refnum; /* in */
    BOOLEAN success; /* out */
    anykey *foundp; /* out */
    short leafindex; /* out */
    trailentry trail[MAXTRAILS]; /* out */
} btparam;

struct wdentry
{
    GUEST_STRUCT;
// FIXME: #warning vcbp is stored as a native pointer.
    GUEST<HVCBPtr> vcbp; // stored as native pointer!
    GUEST<LONGINT> dirid;
    GUEST<LONGINT> cathint; /* ??? */
    GUEST<LONGINT> procid;
};

#if 0
#if defined(MAC)
extern Ptr WDCBsPtr : 0x372;
extern LONGINT BufTgFNum : 0x2FC;
extern INTEGER BufTgFFlag : 0x300;
extern INTEGER BufTgFBkNum : 0x302;
extern LONGINT BufTgDate : 0x304;
extern LONGINT TFSTagData0 : 0x38A;
extern LONGINT TFSTagData1 : 0x38E;
#else

#if !defined(WDCBsPtr_H)
extern GUEST<Ptr> WDCBsPtr_H;
extern LONGINT BufTgFNum;
extern INTEGER BufTgFFlag;
extern INTEGER BufTgFBkNum;
extern LONGINT BufTgDate;
extern LONGINT TFSTagData0;
extern LONGINT TFSTagData1;
extern INTEGER SCSIFlags;
#endif

#define WDCBsPtr (WDCBsPtr_H.p)

#endif
#endif

#define WDMASK 0xC001
#define WDMAGIC 0x8001

#define ISWDNUM(v)                                                                     \
    ({                                                                                 \
        uint16_t _v;                                                                     \
                                                                                       \
        _v = (v);                                                                      \
        (_v & WDMASK) == WDMAGIC ? (_v ^ WDMAGIC) % sizeof(wdentry) == sizeof(INTEGER) \
                                 : false;                                              \
    })

#define WDNUMTOWDP(v) ((wdentry *)(MR(WDCBsPtr) + (INTEGER)((v) ^ WDMAGIC)))
#define WDPTOWDNUM(p) (((char *)(p) - (char *)MR(WDCBsPtr)) ^ WDMAGIC)

typedef enum { seteof,
               allocany,
               alloccontig } alloctype;

extern compretval ROMlib_catcompare(void *firstp, void *secondp);

typedef enum { GETCACHESAVE = 1,
               GETCACHENOREAD = 2 } cacheflagtype;

#define VCBDIRTY (1 << 15)

typedef enum { NOTE,
               CAUTION,
               STOP } alerttype;

enum
{
    INHERITED_FLAG_BITS = ATTRIB_ISLOCKED
};

#if !defined(MAC)
#define CurTime (GetDateTime(&Time), Cx(Time))
#else
#define CurTime Cx(Time)
#endif

#define EJECTALERTID (-4061)

/* public entities in btree.c */

extern cacheentry *ROMlib_addrtocachep(Ptr addr, HVCB *vcbp);
extern BOOLEAN ROMlib_searchnode(btnode *btp, void *key, compfp fp, anykey **keypp,
                                 INTEGER *afterp);
extern OSErr ROMlib_getcache(cacheentry **retpp, uint16_t refnum, ULONGINT logbno,
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
extern xtntkey *ROMlib_newextentrecord(filecontrolblock *fcbp, uint16_t newabn);
extern OSErr ROMlib_btrename(btparam *btpb, StringPtr newnamep);
extern OSErr ROMlib_btcreateemptyfile(btparam *btpb);
extern OSErr ROMlib_btcreateemptydir(btparam *btpb, GUEST<LONGINT> *newidp);
extern OSErr ROMlib_btpbindex(IOParam *pb, LONGINT dirid, HVCB **vcbpp,
                              filerec **frpp, catkey **catkeypp, BOOLEAN onlyfiles);
extern OSErr ROMlib_cleancache(HVCB *vcbp);
extern OSErr ROMlib_flushcachevcbp(HVCB *vcbp);

/* public entries in changing.c */

extern INTEGER ROMlib_flnumtorefnum(ULONGINT flnum, VCB *vcbp);

/* public entries in file.c */

extern filecontrolblock *ROMlib_getfreefcbp(void);
extern filecontrolblock *ROMlib_refnumtofcbp(uint16_t refnum);
extern compretval ROMlib_xtntcompare(void *firstp, void *secondp);
extern compretval ROMlib_catcompare(void *firstp, void *secondp);
extern void ROMlib_makextntkey(xtntkey *keyp, Forktype forkwanted, LONGINT flnum,
                               uint16_t bno);
extern void ROMlib_makextntparam(btparam *btpb, HVCB *vcbp, Forktype forkwanted,
                                 LONGINT flnum, uint16_t bno);
extern LONGINT ROMlib_logtophys(filecontrolblock *fcbp, LONGINT absoffset,
                                LONGINT *nphyscontigp);
extern OSErr ROMlib_makecatkey(catkey *keyp, LONGINT dirid, INTEGER namelen, Ptr namep);
extern OSErr ROMlib_findvcbandfile(IOParam *pb, LONGINT dirid, btparam *btpb,
                                   filekind *kindp, BOOLEAN ignorename);
extern OSErr ROMlib_alreadyopen(HVCB *vcbp, LONGINT flnum, SignedByte *permp,
                                GUEST<INTEGER> *refnump, busyconcern_t busy);
extern OSErr ROMlib_allochelper(IOParam *pb, BOOLEAN async, alloctype alloc,
                                BOOLEAN ROMlib_writefcbp);

/* public entries in helper.c */

extern void OurExit(void);
extern OSErr ROMlib_transphysblk(hfs_access_t *hfsp, LONGINT physblock,
                                 short nphysblocks,
                                 Ptr bufp, accesstype rw, GUEST<LONGINT> *actp);
extern char *ROMlib_indexn(char *str, char tofind, INTEGER length);
#if defined(MAC)
extern void str255assign(StringPtr dstp, StringPtr srcp);
#endif
extern void *ROMlib_indexqueue(QHdr *qp, short index);
extern OSErr ROMlib_writefcbp(filecontrolblock *fcbp);
extern OSErr ROMlib_writevcbp(HVCB *vcbp);
extern void ROMlib_openfloppy(const char *dname, GUEST<LONGINT> *messp);
extern void ROMlib_openharddisk(const char *dname, GUEST<LONGINT> *messp);
extern void ROMlib_hfsinit(void);
extern OSErr ROMlib_ejectfloppy(LONGINT floppyfd);
extern OSErr ROMlib_readwrite(LONGINT fd, char *buffer, LONGINT count, LONGINT off,
                              accesstype rw, LONGINT blocksize, LONGINT maxtransfer);

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

extern DrvQExtra *ROMlib_dqbydrive(short vrefnum);

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

enum
{
    NEWLINEMODE = 1 << 7
};

extern OSErr hfsPBOpen(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHOpen(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBOpenRF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHOpenRF(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBLockRange(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBUnlockRange(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBRead(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBWrite(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetFPos(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetFPos(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetEOF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetEOF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBAllocate(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBAllocContig(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBFlushFile(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBClose(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBCreate(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHCreate(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBDirCreate(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBDelete(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHDelete(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetCatInfo(CInfoPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetCatInfo(CInfoPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBCatMove(CMovePBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBOpenWD(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBCloseWD(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetWDInfo(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetFInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHGetFInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetFInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHSetFInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetFLock(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHSetFLock(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBRstFLock(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHRstFLock(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetFVers(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBRename(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHRename(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetFCBInfo(FCBPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBMountVol(ParmBlkPtr ufsPB, LONGINT floppyfd, LONGINT off,
                           LONGINT bsize, LONGINT maxbytes,
                           drive_flags_t flags, DrvQExtra *dqp);
extern OSErr hfsPBGetVInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHGetVInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetVInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBGetVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHGetVol(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBSetVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBHSetVol(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBFlushVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr hfsPBUnmountVol(ParmBlkPtr ufsPB);
extern OSErr hfsPBOffLine(ParmBlkPtr ufsPB);
extern OSErr hfsPBEject(ParmBlkPtr ufsPB);

extern void try_to_mount_disk(const char *dname, LONGINT floppyfd,
                              GUEST<LONGINT> *messp, LONGINT bsize,
                              LONGINT maxbytes, drive_flags_t flags,
                              uint32_t offset);

extern void ROMlib_OurClose(void);

extern long ROMlib_priv_open(const char *filename, long mode);
extern void vcbsync(HVCB *vcbp);

#endif
}
#endif /* !defined(__RSYS_HFS__) */

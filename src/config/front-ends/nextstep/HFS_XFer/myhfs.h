#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include <string.h>
#include "mytype.h"
#include "xbar.h"

#if !defined(UNIX)
#include "HFS.h"
#else
#include "ThinkC.h"
#define HVCB VCB
#endif

#define FLOPPYREF   -5
#define PHYSBSIZE   512

#define MADROFFSET	40

typedef struct {
    unsigned short blockstart;
    unsigned short blockcount;
} xtntdesc, xtntrec[3];

typedef struct {
    short drSigWord;
    long drCrDate;
    long drLsMod;
    short drAtrb;
    short drNmFls;
    short drVBMSt;
    short drAllocPtr;
    short drNmAlBlks;
    long drAlBlkSiz;
    long drClpSiz;
    short drAlBlSt;
    long drNxtCNID;
    short drFreeBks;
    unsigned char drVN[28];
    long drVolBkUp;
    short drVSeqNum;
    long drWrCnt;
    long drXTClpSiz;
    long drCTClpSiz;
    short drNmRtDirs;
    long drFilCnt;
    long drDirCnt;
    long drFndrInfo[8];
    short drVCSize;
    short drVCBMSize;
    short drCtlCSize;
    long drXTFlSize;
    xtntrec drXTExtRec;
    long drCTFlSize;
    xtntrec drCTExtRec;
} volumeinfo, *volumeinfoPtr, **volumeinfoHandle;

#define VOLUMEINFOBLOCKNO   2
#define InternalDrive   1
#define ROUNDUP8(x) ((x+7)/8*8)
#define NPHYSREQ(x) ((x+PHYSBSIZE-1)/PHYSBSIZE)

typedef struct {
    long ndFLink;
    long ndBLink;
    unsigned char ndType;
    char ndLevel;
    short ndNRecs;
    short idunno;
} btnode;

typedef enum { indexnode, leafnode = 0xFF } btnodetype;

typedef struct {
    unsigned char ckrKeyLen;
    char ckrResrv1;
    long ckrParID;
    unsigned char ckrCName[32];
} catkey;

typedef struct {
    unsigned char xkrKeyLen;
    unsigned char xkrFkType;
    long xkrFNum;
    unsigned short xkrFABN;
} xtntkey;

typedef union {
    unsigned char keylen;
    catkey catk;
    xtntkey xtntk;
} anykey;

#define FILETYPE    2

typedef struct {
    char cdrType;
    char cdrResrv2;
    char filFlags;
    char filTyp;
    FInfo filUsrWds;    /* not sure what form */
    long filFlNum;
    short filStBlk; /* I don't think this is used */
    long filLgLen;
    long filPyLen;
    short filRStBlk;    /* not used? */
    long filRLgLen;
    long filRPyLen;
    long filCrDat;
    long filMdDat;
    long filBkDat;
    long filFndrInfo[4];
    short filClpSize;
    xtntrec filExtRec;
    xtntrec filRExtRec;
    long filResrv;
} filerec;

#define DIRTYPE 1

typedef struct {
    char cdrType;
    char cdrResrv2;
    short dirFlags;
    short dirVal;
    long dirDirID;
    long dirCrDat;
    long dirMdDat;
    long dirBkDat;
    long dirUsrInfo[4];
    long dirFndrInfo[4];
    long dirResrv[4];
} directoryrec;

#define THREADTYPE  3

typedef struct {
    char cdrType;
    char cdrResrv2;
    char thdResrv[8];
    long thdParID;
    unsigned char thdCName[32];
} threadrec;

typedef enum { firstisless = -1, same, firstisgreater } compretval;

typedef compretval (*compfp)(void *first, void *second);

#define WRITEBIT    (1<<0)
#define RESOURCEBIT (1<<1)
#define SHAREDBIT   (1<<4)
#define FLOCKEDBIT  (1<<5)
#define DIRTYBIT    (1<<7)

typedef struct {
    long fcbFlNum;
    Byte fcbMdRByt;
    Byte fcbTypByt;
    short fcbSBlk;
    long fcbEOF;
    long fcbPLen;
    long fcbCrPs;
    HVCB *fcbVPtr;
    Ptr fcbBfAdr;
    short fcbFlPos;
    long fcbClmpSize;
    long fcbBTCBPtr;
    xtntrec fcbExtRec;
    long fcbFType;
    ulong fcbCatPos;
    long fcbDirID;
    unsigned char fcbCName[32];
} filecontrolblock;

typedef enum { datafork, resourcefork = 0xFF } forktype;

typedef enum { reading, writing } accesstype;

#define VSOFTLOCKBIT    (1<<15)
#define VHARDLOCKBIT    (1<<7)
#define FSOFTLOCKBIT    (1<<0)
#define FILEFLAGSUSERSETTABLEMASK   FSOFTLOCKBIT

typedef struct {
    long    flink;              /* 0 */
    long    blink;              /* 4 */
    unsigned char type;         /* 8 */
    unsigned char dummy;        /* 9 */
    short   hesthreejim;        /* 10 */
    long    height;             /* 12 */
    long    root;               /* 16 */
    long    numentries;         /* 20 */
    long    firstleaf;          /* 24 */
    long    lastleaf;           /* 28 */
    short   btnodesize;         /* 32 */
    short   indexkeylen;        /* 34 */
    long    nnodes;             /* 36 */
    long    nfreenodes;         /* 40 */
    unsigned char reserved[72]; /* 44 */
    unsigned char dummy2[132];  /* 116 */
    unsigned char map[256];     /* 248 */
    long unknown2[2];           /* 504 */
} btblock0;

#define DATAPFROMKEY(p) ((char *)(p) + ((((catkey *)p)->ckrKeyLen + 2) & ~1))

typedef enum { mfs, hfs } fstype;
typedef enum { regular = 1, directory = 2, thread = 4 } filekind;

#if !defined(UNIX)
#define THINKCMESSED
#endif /* !defined(UNIX) */

#if defined (THINKCMESSED)
#define vcbClpSiz   vcbClpSIz
#endif /* THINKCMESSED */

/*
 * TODO: do the ioCompletion routine when necessary
 */
 
#define PBRETURN(pb, x) return (pb)->ioResult = (x)

typedef struct _cacheentry {
    struct _cacheentry *flink;
    struct _cacheentry *blink;
    HVCB *vptr;
    ulong fileno;
    ushort refnum;
    ulong physblock;
    ulong logblk;
    unsigned char flags;
    unsigned char forktype;
    char buf[PHYSBSIZE];
} cacheentry;

#define CACHEDIRTY  (1 << 7)
#define CACHEBUSY   (1 << 6)
#define CACHEFREE   (1 << 5)

typedef struct {
    cacheentry *flink;
    cacheentry *blink;
    short   nitems;
    ushort  flags;
} cachehead;

#define NCACHEENTRIES   16

#define MAXTRAILS   8

typedef struct {
    cacheentry *cachep;
    short logbno;
    short after;
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
    short refnum;                   /* in */
    BOOLEAN success;                /* out */
    anykey *foundp;                 /* out */
    short leafindex;                /* out */
    trailentry trail[MAXTRAILS];    /* out */
} btparam;

typedef struct {    /* from MPW equates */
    HVCB *vcbp;
    LONGINT dirid;
    LONGINT cathint;    /* ??? */
    LONGINT procid;
} wdentry;

#if !defined(UNIX)
extern Ptr WDCBsPtr : 0x372;
extern LONGINT BufTgFNum : 0x2FC;
extern INTEGER BufTgFFlag : 0x300;
extern INTEGER BufTgFBkNum : 0x302;
extern LONGINT BufTgDate : 0x304;
extern LONGINT TFSTagData0 : 0x38A;
extern LONGINT TFSTagData1 : 0x38E;
#else
extern Ptr WDCBsPtr;
extern LONGINT BufTgFNum;
extern INTEGER BufTgFFlag;
extern INTEGER BufTgFBkNum;
extern LONGINT BufTgDate;
extern LONGINT TFSTagData0;
extern LONGINT TFSTagData1;
#endif

#define WDMASK  0xC001
#define WDMAGIC 0x8001
#define ISWDNUM(v)  (((v) & WDMASK) == WDMAGIC)
#define WDNUMTOWDP(v) ((wdentry *)(WDCBsPtr + ((v) ^ 0x8001)))
#define WDPTOWDNUM(p)   (((char *) (p) - (char *)WDCBsPtr) ^ 0x8001)

typedef enum { seteof, allocany, alloccontig } alloctype;

extern compretval catcompare(void *firstp, void *secondp);

typedef enum { GETCACHESAVE = 1, GETCACHENOREAD = 2 } cacheflagtype;

#define VCBDIRTY    (1 << 15)

typedef enum {NOTE, CAUTION, STOP} alerttype;

#if defined(UNIX)
#define CurTime	(GetDateTime(&Time), Time)
#else
#define CurTime Time
#endif

/* public entities in btree.c */

extern cacheentry *addrtocachep(Ptr addr, HVCB *vcbp);
extern BOOLEAN searchnode(btnode *btp, void *key, compfp fp, anykey **keypp,
							       INTEGER *afterp);
extern OSErr getcache(cacheentry **retpp, short refnum, ulong logbno,
							   cacheflagtype flags);
extern OSErr putcache(cacheentry *cachep);
extern OSErr keyfind(btparam *btpb);
extern OSErr btnext(anykey **nextpp, anykey *keyp, HVCB *vcbp);
extern OSErr btdelete(btparam *btpb);
extern void makecatparam(btparam *btpb, HVCB *vcbp, LONGINT dirid,
						    INTEGER namelen, Ptr namep);
extern OSErr filedelete(btparam *btpb, filekind kind);
extern OSErr dirdelete(btparam *btpb);
extern OSErr dirtyleaf(void *p, HVCB *vcbp);
extern OSErr filecreate(btparam *btpb, void *data, filekind kind);
extern OSErr dircreate(btparam *btpb, directoryrec *data);
extern OSErr dircreate(btparam *btpb, directoryrec *data);
extern xtntkey *newextentrecord(filecontrolblock *fcbp, ushort newabn);
extern OSErr btrename(btparam *btpb, StringPtr newnamep);
extern OSErr btcreateemptyfile(btparam *btpb);
extern OSErr btcreateemptydir(btparam *btpb, LONGINT *newidp);
extern OSErr btpbindex (ioParam *pb, LONGINT dirid, HVCB **vcbpp,
			  filerec **frpp, catkey **catkeypp, BOOLEAN onlyfiles);
extern OSErr cleancache(HVCB *vcbp);
extern OSErr flushcachevcbp(HVCB *vcbp);

/* public entries in changing.c */

extern INTEGER flnumtorefnum(ulong flnum);

/* public entries in file.c */

extern filecontrolblock *getfreefcbp( void );
extern filecontrolblock *refnumtofcbp(short refnum);
extern compretval xtntcompare(void *firstp, void *secondp);
extern compretval catcompare(void *firstp, void *secondp);
extern void makextntkey(xtntkey *keyp, forktype forkwanted, LONGINT flnum,
								    ushort bno);
extern void makextntparam(btparam *btpb, HVCB *vcbp, forktype forkwanted,
						     LONGINT flnum, ushort bno);
extern long logtophys(filecontrolblock *fcbp, long absoffset,
							   short *nphyscontigp);
extern void makecatkey(catkey *keyp, LONGINT dirid, INTEGER namelen, Ptr namep);
extern OSErr findvcbandfile(ioParam *pb, LONGINT dirid, btparam *btpb,
					   filekind *kindp, BOOLEAN ignorename);
extern OSErr alreadyopen(HVCB *vcbp, ulong flnum, SignedByte *permp,
								short *refnump);
extern OSErr dirtyfcbp(filecontrolblock *fcbp);
extern OSErr AllocHelper(ioParam *pb, BOOLEAN async, alloctype alloc,
							     BOOLEAN writefcbp);

/* public entries in helper.c */

extern void OurExit( void );
extern OSErr TransPhysBlk(HVCB *vcbp, long physblock, short nphysblocks,
					   Ptr bufp, accesstype rw, long *actp);
extern char *indexn(char *str, char tofind, INTEGER length);
#if !defined(UNIX)
extern void str255assign(StringPtr dstp, StringPtr srcp);
#endif
extern void *indexqueue(QHdr *qp, short index);
extern OSErr writefcbp(filecontrolblock *fcbp);
extern OSErr writevcbp(HVCB *vcbp);

/* public entries in volume.c */

extern HVCB *findvcb(short vrefnum, StringPtr name, LONGINT *diridp);
extern OSErr pbvolrename(ioParam *pb, StringPtr newnamep);
extern OSErr flushvcbp(HVCB *vcbp);

/* public entries in workingdir.c */

extern OSErr dirbusy(LONGINT dirid, HVCB *vcbp);
extern void adjustdirid(LONGINT *diridp, HVCB *vcbp, INTEGER vrefnum);

#if !defined(UNIX)
/* public entries in misc.c */
extern void bcopy(void *srcp, void *dstp, LONGINT length);
extern void bzero(void *dstp, LONGINT ntozero);
#endif

/* public entry in error.c */
extern void errormessage(StringPtr msg, alerttype severity);

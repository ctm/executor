#if !defined(__rsys_file__)
#define __rsys_file__

#include <string>

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#include "rsys/filedouble.h"

/* #warning ioCompletion code isn't being called */

/*
 * Copyright 1986 - 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "DeviceMgr.h"
#include "rsys/hook.h"
#include "rsys/drive_flags.h"

/* relative paths of the system folder */

#define SYSMACNAME "\006System"
namespace Executor
{

typedef struct hashlink_str
{
    struct hashlink_str *next;
    LONGINT dirid;
    LONGINT parid;
    char *dirname;
} hashlink_t;

#pragma pack(push, 2)

typedef struct
{
    LONGINT fd;
    LONGINT offset;
    LONGINT bsize;
    LONGINT maxbytes;
} hfs_access_t;

/*
 * Evil guest/host mixed structure.
 * The devicename is a host memory pointer.
 * partition is in big-endian guest byte order.
 * #ifdef alpha seems to mean that alpha is the only 64 bit platform in the world,
 * and that this structure should have a fixed size? Why?
 */
typedef struct
{
    GUEST<LONGINT> flags;
    DrvQEl dq;
    Ptr devicename; /* "/usr"	"/dev/rfd0"	whatever */
#if !defined(__alpha)
    char *filler;
#endif
    GUEST<INTEGER> partition; /* for multiply partitioned drives */
    hfs_access_t hfs; /* currently only for floppies -- ick */
} DrvQExtra;

#define OURUFSDREF (-102)

extern int ROMlib_nosync;

#if 0
#if !defined(VCBQHdr)
extern QHdr VCBQHdr, DrvQHdr;
extern GUEST<VCBPtr> DefVCBPtr_H;
#endif
#endif

extern GUEST<LONGINT> DefDirID;

#if defined(MSDOS)
extern bool cd_mounted_by_trickery_p;
#endif

/* Internal structure of access path info */

#define fcdirty (1 << 7)
#define fcclump (1 << 6)
#define fcwriteprot (1 << 5)
#define fcsharedwrite (1 << 4)
#define fclockrange (1 << 2)
#define fcfisres (1 << 1)
#define fcwriteperm (1 << 0)

/*
 * This struct is confusing.
 * Some of it is big-endian guest data,
 * some of it is native host data.
 * Not entirely sure yet which is which.
 */
typedef struct
{
    GUEST_STRUCT;
    GUEST<LONGINT> fdfnum; /* LONGINT fcbFlNum */
    GUEST<Byte> fcflags; /* Byte fcbMdRByt */
    GUEST<Byte> fcbTypByt;
    GUEST<INTEGER> fcbSBlk;
    GUEST<LONGINT> fcleof; /* LONGINT fcbEOF */
    GUEST<LONGINT> fcPLen;
    GUEST<LONGINT> fcbCrPs;
    GUEST<VCB *> fcvptr; /* VCB *fcbVPtr */
    GUEST<Ptr> fcbBfAdr;
    GUEST<INTEGER> fcbFlPos;
    GUEST<LONGINT> fcbClmpSize;
    LONGINT fcfd; /* instead of: LONGINT fcbBTCBPtr	PACKED; */
    GUEST<LONGINT> zero[3]; /* these three fields are fcbExtRec */
    GUEST<LONGINT> fcbFType;
    LONGINT hiddenfd; /* instead of LONGINT fcbCatPos */
    GUEST<LONGINT> fcparid; /* LONGINT fcbDirID */
    GUEST<Byte[32]> fcname; /* Str31 fcbCName */
} fcbrec;

#define NFCB 348 /* should be related to NOFILE */

typedef struct
{
    GUEST_STRUCT;
    GUEST<INTEGER> nbytes;
    GUEST<fcbrec[NFCB]> fc;
} fcbhidden;

#define ROMlib_fcblocks (((fcbhidden *)MR(FCBSPtr))->fc)

typedef struct
{
    GUEST_STRUCT;
    GUEST<LONGINT> dirid;
    GUEST<INTEGER> filesystemlen;
    unsigned char hostnamelen;
    char hostnameandroot[1]; /* potentially many more */
} rkey_t;

typedef struct
{
    GUEST_STRUCT;
    GUEST<LONGINT> parid;
    char path[1]; /* potentially many more */
} rcontent_t;

#define FILLOCK 1 /* dirflags & FILLOCK tell whether file is locked */

#define VOLCHAR ':'

typedef enum { DataFork,
               ResourceFork } ForkType;
typedef enum { File,
               Directory } FOrDType;
typedef enum { Get,
               Set } GetOrSetType;
typedef enum { CatMove,
               FRename,
               HRename } MoveOrRenameType;
typedef enum { Lock,
               Unlock } LockOrUnlockType;

/* Note below:  No WDIndex because the paramBlock for PBGetWDInfo is weird */
typedef enum { NoIndex,
               VolIndex,
               FDirIndex,
               FCBIndex,
               IGNORENAME } IndexType;

#if defined(NDEBUG)
#define BADRETURNHOOK(err)
#else /* !defined(NDEBUG) */
#define BADRETURNHOOK(err) \
    if(err != noErr)       \
    ROMlib_hook(file_badreturn)
#endif /* !defined(NDEBUG) */

#define CALLCOMPLETION(pb, compp, err)            \
    do                                            \
    {                                             \
        LONGINT saved0, saved1, saved2, saved3,   \
            savea0, savea1, savea2, savea3;       \
                                                  \
        saved0 = EM_D0;                           \
        saved1 = EM_D1;                           \
        saved2 = EM_D2;                           \
        saved3 = EM_D3;                           \
        savea0 = EM_A0;                           \
        savea1 = EM_A1;                           \
        savea2 = EM_A2;                           \
        savea3 = EM_A3;                           \
        EM_A0 = US_TO_SYN68K(pb);                 \
        EM_D0 = err;                              \
        CALL_EMULATOR(US_TO_SYN68K((long)compp)); \
        EM_D0 = saved0;                           \
        EM_D1 = saved1;                           \
        EM_D2 = saved2;                           \
        EM_D3 = saved3;                           \
        EM_A0 = savea0;                           \
        EM_A1 = savea1;                           \
        EM_A2 = savea2;                           \
        EM_A3 = savea3;                           \
    } while(0)

#define FAKEASYNC(pb, a, err)                                          \
    do                                                                 \
    {                                                                  \
        ((ParmBlkPtr)(pb))->ioParam.ioResult = CW(err);                \
        if(err != noErr)                                               \
            warning_trap_failure("%d", err);                           \
        BADRETURNHOOK(err);                                            \
        if(a)                                                          \
        {                                                              \
            ProcPtr compp;                                             \
                                                                       \
            if((compp = MR(((ParmBlkPtr)(pb))->ioParam.ioCompletion))) \
            {                                                          \
                CALLCOMPLETION(pb, compp, err);                        \
            }                                                          \
        }                                                              \
        return err;                                                    \
    } while(0)

extern fcbrec *PRNTOFPERR(INTEGER prn, OSErr *errp);

#define ATTRIB_ISLOCKED (1 << 0)
#define ATTRIB_RESOPEN (1 << 2)
#define ATTRIB_DATAOPEN (1 << 3)
#define ATTRIB_ISADIR (1 << 4)
#define ATTRIB_ISOPEN (1 << 7)

#define RESOURCEPREAMBLE 512
#define MAGICWORD 0x41524449

#define CHEATDIR __cheatdir

#define INODEMAP "inodemap"

typedef struct
{
    VCB vcb;
    char *unixname;
#if !defined(__alpha)
    char *filler;
#endif
    union {
        struct
        {
            LONGINT ino;
            LONGINT nhashentries;
            hashlink_t **hashtable;
#if !defined(__alpha)
            char *filler2;
#endif
        } ufs;
        hfs_access_t hfs;
    } u;
} VCBExtra;

enum
{
    bHasBlankAccessPrivileges = 4,
    bHasBtreeMgr,
    bHasFileIDs,
    bHasCatSearch,
    bHasUserGroupList,
    bHasPersonalAccessPrivileges,
    bHasFolderLock,
    bHasShortName,
    bHasDesktopMgr,
    bHasMoveRename,
    bHasCopyFile,
    bHasOpenDeny,
    bHasExtFSVol,
    bNoSysDir,
    bAccessCntl,
    bNoBootBlks,
    bNoDeskItems,
    bNoSwitchTo = 25,
    bTrshOffLine,
    bNoLclSync,
    bNoVNEdit,
    bNoMiniFndr,
    bLocalWList,
    bLimitFCBs,
};

typedef struct
{
    GUEST_STRUCT;
    GUEST<INTEGER> vMVersion;
    GUEST<ULONGINT> vMAttrib;
    GUEST<LONGINT> vMLocalHand;
    GUEST<LONGINT> vMServerAdr;
    GUEST<LONGINT> vMVolumeGrade;
    GUEST<INTEGER> vMForeignPrivID;
} getvolparams_info_t;

#define HARDLOCKED (1 << 7)
#define SOFTLOCKED (1 << 15)
#define volumenotlocked(vp) (Cx(((VCB *)vp)->vcbAtrb) & SOFTLOCKED ? vLckdErr : (Cx(((VCB *)vp)->vcbAtrb) & HARDLOCKED ? wPrErr : noErr))

#define FORKOFFSET ROMlib_FORKOFFSET

#if !defined(L_INCR)
#define L_INCR 1
#endif /* L_INCR */

#if !defined(L_SET)
#define L_SET 0
#endif /* L_SET */

#if !defined(L_XTND)
#define L_XTND 2
#endif /* L_XTND */

/*
 * TODO: Below we check for non-zero
 *	 ioFDirIndex.  It should probably be a check for positive.  We
 *	 should also check into what we do when the first byte is zero.
 */

#define UPDATE_IONAMEPTR_P(pb) \
    ((pb).ioNamePtr && ((pb).ioFDirIndex != CWC(0) || !MR((pb).ioNamePtr)[0]))

/* After a pathname has been normalized, the offset of the first
   slash.  It's 2 under DOS because a normalized path is, e.g., C:/etc */

#if defined(MSDOS) || defined(CYGWIN32)
enum
{
    SLASH_CHAR_OFFSET = 2
};
#else
enum
{
    SLASH_CHAR_OFFSET = 0
};
#endif

extern StringPtr ROMlib_exefname;
extern char *ROMlib_exeuname;

#pragma pack(pop)

extern LONGINT ROMlib_FORKOFFSET(fcbrec *fp);
extern OSErr ROMlib_seteof(fcbrec *fp);
extern OSErr ROMlib_geteofostype(fcbrec *fp);
extern OSErr ROMlib_nami(ParmBlkPtr pb, LONGINT dir, IndexType indextype,
                         char **pathname, char **filename, char **endname,
                         BOOLEAN nodirs, VCBExtra **vcbpp, struct stat *sbufp);
extern OSErr ROMlib_maperrno(void);
extern VCB *ROMlib_breakoutioname(ParmBlkPtr pb, LONGINT *diridp,
                                  char **therestp, BOOLEAN *fullpathp, BOOLEAN usedefault);

extern void ROMlib_fillkeycontent(datum *keyp, datum *contentp,
                                  char *path, VCBExtra *vcbp);
extern datum ROMlib_dbm_fetch(VCBExtra *vcbp, LONGINT dir);
extern BOOLEAN ROMlib_dbm_store(VCBExtra *vcbp, char *pathname,
                                LONGINT *dirp, BOOLEAN verify_p);
extern void ROMlib_dbm_delete_inode(VCBExtra *vcbp, LONGINT inode);
extern void ROMlib_dbm_open(VCBExtra *vcbp);
extern void ROMlib_dbm_close(VCBExtra *vcbp);

extern OSErr ROMlib_PBMoveOrRename(ParmBlkPtr pb, BOOLEAN a, LONGINT dir,
                                   LONGINT newdir, char *newname, MoveOrRenameType op);
extern OSErr ROMlib_PBGetSetFInfoD(ParmBlkPtr pb, BOOLEAN a,
                                   GetOrSetType op, GUEST<LONGINT> *dir, BOOLEAN dodirs);
extern OSErr ROMlib_driveropen(ParmBlkPtr pbp, BOOLEAN a);
extern OSErr ROMlib_dispatch(ParmBlkPtr p, BOOLEAN async,
                             DriverRoutineType routine, INTEGER trap);

extern DrvQExtra *ROMlib_addtodq(ULONGINT drvsize, const char *devicename,
                                 INTEGER partition, INTEGER drefnum,
                                 drive_flags_t flags, hfs_access_t *hfsp);

extern unsigned long ROMlib_destroy_blocks(syn68k_addr_t start, uint32_t count,
                                           BOOLEAN flush_only_faulty_checksums);
extern void ROMlib_automount(const char *path);

extern Byte open_attrib_bits(LONGINT file_id, VCB *vcbp, GUEST<INTEGER> *refnump);

extern VCB *vlookupbyname(const char *namep, const char *endp);

extern OSErr ufsPBOpen(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHOpen(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBOpenRF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHOpenRF(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBLockRange(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBUnlockRange(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBRead(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBWrite(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetFPos(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetFPos(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetEOF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetEOF(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBAllocate(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBAllocContig(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBFlushFile(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBClose(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBCreate(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHCreate(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBDirCreate(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBDelete(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHDelete(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetCatInfo(CInfoPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetCatInfo(CInfoPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBCatMove(CMovePBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBOpenWD(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBCloseWD(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetWDInfo(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetFInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHGetFInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetFInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHSetFInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetFLock(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHSetFLock(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBRstFLock(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHRstFLock(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetFVers(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBRename(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHRename(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetFCBInfo(FCBPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBMountVol(ParmBlkPtr ufsPB);
extern OSErr ufsPBGetVInfo(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHGetVInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetVInfo(HParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBGetVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHGetVol(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBSetVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBHSetVol(WDPBPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBFlushVol(ParmBlkPtr ufsPB, BOOLEAN a);
extern OSErr ufsPBUnmountVol(ParmBlkPtr ufsPB);
extern OSErr ufsPBOffLine(ParmBlkPtr ufsPB);
extern OSErr ufsPBEject(ParmBlkPtr ufsPB);

extern void ROMlib_fileinit(void);

#if !defined(NDEBUG)

extern void fs_err_hook(OSErr);

#else

#define fs_err_hook(x)

#endif

extern void HCreateResFile_helper(INTEGER vrefnum, LONGINT parid, Str255 name,
                                  OSType creator, OSType type,
                                  ScriptCode script);

extern OSErr FSReadAll(INTEGER rn, LONGINT *count, Ptr buffp);
extern OSErr FSWriteAll(INTEGER rn, LONGINT *count, Ptr buffp);

extern int ROMlib_no_dot_files;
extern LONGINT ROMlib_magic_offset;

extern void convert_slashs_to_backslashs(char *p);

extern OSErr ROMlib_hiddenbyname(GetOrSetType gors, char *pathname,
                                 char *rpathname, Single_dates *datep,
                                 FInfo *finfop, FXInfo *fxinfop,
                                 GUEST<LONGINT> *lenp, GUEST<LONGINT> *rlenp);

extern unsigned char ROMlib_fromhex(unsigned char c);
#endif

#if !defined(ST_INO)
#define ST_INO(buf) ((uint32_t)((buf).st_ino))
#endif

extern std::string ROMlib_volumename;
extern INTEGER ROMlib_nextvrn;

std::string expandPath(std::string);
}
#endif /* !defined(__rsys_file__) */

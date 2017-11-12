#if !defined(__RSYS_NEW_MMANSTUBS__)
#define __RSYS_NEW_MMANSTUBS__
namespace Executor
{
extern OSErrRET HSetStateError(Handle h, SignedByte flags);
#define HSetState(h, flags) \
    (MemErr = CW(HSetStateError(h, flags)), (void)0)

extern OSErrRET HLockError(Handle h);
#define HLock(h) (MemErr = CW(HLockError(h)), (void)0)

extern OSErrRET HUnlockError(Handle h);
#define HUnlock(h) (MemErr = CW(HUnlockError(h)), (void)0)

extern OSErrRET HPurgeError(Handle h);
#define HPurge(h) (MemErr = CW(HPurgeError(h)), (void)0)

extern OSErrRET HNoPurgeError(Handle h);
#define HNoPurge(h) (MemErr = CW(HNoPurgeError(h)), (void)0)

extern OSErrRET HSetRBitError(Handle h);
#define HSetRBit(h) (MemErr = CW(HSetRBitError(h)), (void)0)

extern OSErrRET HClrRBitError(Handle h);
#define HClrRBit(h) (MemErr = CW(HClrRBitError(h)), (void)0)

extern OSErrRET InitApplZoneError(void);
#define InitApplZone() (MemErr = CW(InitApplZoneError()), (void)0)

extern OSErrRET SetApplBaseError(Ptr newbase);
#define SetApplBase(nb) (MemErr = CW(SetApplBaseError(nb)), (void)0)

extern OSErrRET MoreMastersError(void);
#define MoreMasters() (MemErr = CW(MoreMastersError()), (void)0)

extern OSErrRET InitZoneError(ProcPtr pGrowZone, INTEGER cMoreMasters,
                              Ptr limitPtr, Zone *startPtr);
#define InitZone(pg, cm, lp, sp) \
    (MemErr = CW(InitZoneError(pg, cm, lp, sp)), (void)0)

extern OSErrRET SetZoneError(THz hz);
#define SetZone(hz) (MemErr = CW(SetZoneError(hz)), (void)0)

extern OSErrRET DisposHandleError(Handle h);
#define DisposHandle(h) (MemErr = CW(DisposHandleError(h)), (void)0)

extern Size GetHandleSizeError(Handle h);

#if defined(TRUSTNEXTGCC)
#define GetHandleSize(h)                  \
    ({                                    \
        LONGINT size;                     \
        size = GetHandleSizeError(h);     \
        size < 0 ? (MemErr = CW(size), 0) \
                 : size;                  \
    })
#else /* !defined(TRUSTNEXTGCC) */
#if !defined(NO_ROMLIB)
static inline LONGINT GetHandleSize(Handle h)
{
    LONGINT size;

    size = GetHandleSizeError(h);
    return size < 0 ? (MemErr = CW(size), 0)
                    : size;
}
#endif /* !NO_ROMLIB */
#endif /* !defined(TRUSTNEXTGCC) */

extern OSErrRET SetHandleSizeError(Handle h, ULONGINT newsize);
#define SetHandleSize(h, ns) \
    (MemErr = CW(SetHandleSizeError(h, ns)), (void)0)

extern OSErrRET ReallocHandleError(Handle h, ULONGINT size);
#define ReallocHandle(h, s) \
    (MemErr = CW(ReallocHandleError(h, s)), (void)0)

extern OSErrRET DisposPtrError(Ptr p);
#define DisposPtr(p) (MemErr = CW(DisposPtrError(p)), (void)0)

extern Size GetPtrSizeError(Ptr p);
#if defined(TRUSTNEXTGCC)
#define GetPtrSize(p)                     \
    ({                                    \
        LONGINT size;                     \
        size = GetPtrSizeError(p);        \
        size < 0 ? (MemErr = CW(size), 0) \
                 : size;                  \
    })
#else /* !defined(TRUSTNEXTGCC) */
#if !defined(NO_ROMLIB)
static inline LONGINT GetPtrSize(Ptr p)
{
    LONGINT size;

    size = GetPtrSizeError(p);
    return size < 0 ? (MemErr = CW(size), 0)
                    : size;
}
#endif /* !NO_ROMLIB */
#endif /* !defined(TRUSTNEXTGCC) */

extern OSErrRET SetPtrSizeError(Ptr p, ULONGINT newsize);
#define SetPtrSize(p, ns) \
    (MemErr = CW(SetPtrSizeError(p, ns)), (void)0)

extern OSErrRET BlockMoveError(Ptr src, Ptr dst, Size cnt);
#define BlockMove(src, dst, cnt) \
    (MemErr = CW(BlockMoveError(src, dst, cnt)), (void)0)

extern OSErrRET MaxApplZoneError(void);
#define MaxApplZone() (MemErr = CW(MaxApplZoneError()), (void)0)

extern OSErrRET MoveHHiError(Handle h);
#define MoveHHi(h) (MemErr = CW(MoveHHiError(h)), (void)0)

extern OSErrRET SetApplLimitError(Ptr newlimit);
#define SetApplLimit(nl) (MemErr = CW(SetApplLimitError(nl)), (void)0)

extern OSErrRET SetGrowZoneError(ProcPtr newgz);
#define SetGrowZone(ngz) (MemErr = CW(SetGrowZoneError(ngz)), (void)0)

extern OSErrRET EmptyHandleError(Handle h);
#define EmptyHandle(h) (MemErr = CW(EmptyHandleError(h)), (void)0)

extern Handle RecoverHandleFlag(Ptr p, short trapno);
#define RecoverHandle(p) (RecoverHandleFlag(p, 0))

extern LONGINT FreeMemFlag(short trapno);
#define FreeMem() (FreeMemFlag(0))

extern Size MaxMemFlag(Size *growp, short trapno);
#define MaxMem(gp) (MaxMemFlag(gp, 0))

extern Size CompactMemFlag(ULONGINT sizeneeded, short trapno);
#define CompactMem(size) (CompactMemFlag(size, 0))

extern LONGINT MaxBlockFlag(short trapno);
#define MaxBlock() (MaxBlockFlag(0))

extern void PurgeSpaceFlag(LONGINT *totalp, LONGINT *contigp, short trapno);
#define PurgeSpace(totalp, contigp) (PurgeSpaceFlag(totalp, contigp, 0))

extern OSErrRET ResrvMemFlagError(ULONGINT needed, short trapno);
#define ResrvMem(size) (MemErr = CW(ResrvMemFlagError(size, 0)), (void)0)

extern OSErrRET PurgeMemFlagError(ULONGINT sizeneeded, short trapno);
#define PurgeMem(size) (MemErr = CW(PurgeMemFlagError(size, 0)), (void)0)

extern THz GetZoneErrorP(OSErrRET *errp);
#define GetZonePtr() (GetZonePtrErrorP(0))

extern THz HandleZoneErrorP(Handle h, OSErrRET *errp);
#define HandleZone(h) (HandleZoneErrorP(h, 0))

extern THz PtrZoneErrorP(Ptr p, OSErrRET *errp);
#define PtrZone(p) (PtrZoneErrorP(p, 0))

extern Handle NewEmptyHandleFlagErrorP(short trapno, OSErrRET *errp);
#define NewEmptyHandle() (NewEmptyHandleFlagErrorP(0, 0))

extern Handle NewHandleFlagErrorP(ULONGINT size, short trapno, OSErrRET *errp);
#define NewHandle(size) (NewHandleFlagErrorP(size, 0, 0))

extern Ptr NewPtrFlagErrorP(ULONGINT size, short trapno, OSErrRET *errp);
#define NewPtr(size) (NewPtrFlagErrorP(size, 0, 0))
}
#endif /* !defined(__RSYS_NEW_MMANSTUBS__) */

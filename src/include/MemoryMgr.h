#if !defined(_MEMORY_MGR_H_)
#define _MEMORY_MGR_H_

#include "ExMacTypes.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
enum
{
    memFullErr = (-108),
    memLockedErr = (-117),
    memPurErr = (-112),
    memWZErr = (-111),
};
//enum { memAZErr	 = -113 };
enum
{
    nilHandleErr = (-109),
};

enum
{
    memROZErr = (-99),
    memAdrErr = (-110),
    memAZErr = (-113),
    memPCErr = (-114),
    memBCErr = (-115),
    memSCErr = (-116),
};

typedef UPP<LONGINT(Size)> GrowZoneProcPtr;

struct Zone
{
    GUEST_STRUCT;
    GUEST<Ptr> bkLim;
    GUEST<Ptr> purgePtr;
    GUEST<Ptr> hFstFree;
    GUEST<LONGINT> zcbFree;
    GUEST<GrowZoneProcPtr> gzProc;
    GUEST<INTEGER> moreMast;
    GUEST<INTEGER> flags;
    GUEST<INTEGER> cntRel;
    GUEST<INTEGER> maxRel;
    GUEST<INTEGER> cntNRel;
    GUEST<INTEGER> maxNRel;
    GUEST<INTEGER> cntEmpty;
    GUEST<INTEGER> cntHandles;
    GUEST<LONGINT> minCBFree;
    GUEST<ProcPtr> purgeProc;
    GUEST<Ptr> sparePtr;
    GUEST<Ptr> allocPtr;
    GUEST<INTEGER> heapData;
};
typedef Zone *THz;

#if 0
#if !defined(MemErr)
extern int16_t MemErr;
extern GUEST<Ptr> 	MemTop_H;
extern GUEST<Ptr> 	BufPtr_H;
extern GUEST<Ptr> 	HeapEnd_H;
extern GUEST<THz> 	TheZone_H;
extern GUEST<Ptr> 	ApplLimit_H;
extern GUEST<THz> 	SysZone_H;
extern GUEST<THz> 	ApplZone_H;
extern GUEST<Ptr> 	ROMBase_H;
extern GUEST<Ptr> 	heapcheck_H;
extern GUEST<Handle> 	GZRootHnd_H;
extern GUEST<ProcPtr> 	IAZNotify_H;
extern GUEST<Ptr> 	CurrentA5_H;
extern GUEST<Ptr> 	CurStackBase_H;
extern Byte 	Scratch20[20];
extern LONGINT 	Lo3Bytes;
extern LONGINT 	MinStack;
extern LONGINT 	DefltStack;
extern Byte 	ToolScratch[8];
extern Byte 	Scratch8[8];
extern LONGINT 	OneOne;
extern LONGINT 	MinusOne;
extern Byte 	ApplScratch[12];
#endif

enum
{
    MemTop = (MemTop_H.p),
    BufPtr = (BufPtr_H.p),
    HeapEnd = (HeapEnd_H.p),
    TheZone = (TheZone_H.p),
    ApplLimit = (ApplLimit_H.p),
    SysZone = (SysZone_H.p),
    ApplZone = (ApplZone_H.p),
    ROMBase = (ROMBase_H.p),
    heapcheck = (heapcheck_H.p),
    GZRootHnd = (GZRootHnd_H.p),
    IAZNotify = (IAZNotify_H.p),
    CurrentA5 = (CurrentA5_H.p),
    CurStackBase = (CurStackBase_H.p),
};
#endif
/* traps which can have a `sys' or `clear' bit set */

#define NewEmptyHandle() (_NewEmptyHandle_flags(false))
#define NewEmptyHandleSys() (_NewEmptyHandle_flags(true))
extern Handle _NewEmptyHandle_flags(bool sys_p);

#define NewHandle(size) (_NewHandle_flags(size, false, false))
#define NewHandleSys(size) (_NewHandle_flags(size, true, false))
#define NewHandleClear(size) (_NewHandle_flags(size, false, true))
#define NewHandleSysClear(size) (_NewHandle_flags(size, true, true))
extern Handle _NewHandle_flags(Size size, bool sys_p, bool clear_p);

#define RecoverHandle(ptr) (_RecoverHandle_flags(ptr, false))
#define RecoverHandleSys(ptr) (_RecoverHandle_flags(ptr, true))
extern Handle _RecoverHandle_flags(Ptr p, bool sys_p);

#define NewPtr(size) (_NewPtr_flags(size, false, false))
#define NewPtrSys(size) (_NewPtr_flags(size, true, false))
#define NewPtrClear(size) (_NewPtr_flags(size, false, true))
#define NewPtrSysClear(size) (_NewPtr_flags(size, true, true))
extern Ptr _NewPtr_flags(Size size, bool sys_p, bool clear_p);

#define FreeMem() (_FreeMem_flags(false))
#define FreeMemSys() (_FreeMem_flags(true))
extern int32_t _FreeMem_flags(bool sys_p);

#define MaxMem(growp) (_MaxMem_flags(growp, false))
#define MaxMemSys(growp) (_MaxMem_flags(growp, true))
extern Size _MaxMem_flags(Size *growp, bool sys_p);

#define CompactMem(needed) (_CompactMem_flags(needed, false))
#define CompactMemSys(needed) (_CompactMem_flags(needed, true))
extern Size _CompactMem_flags(Size sizeneeded, bool sys_p);

#define ResrvMem(needed) (_ResrvMem_flags(needed, false))
#define ResrvMemSys(needed) (_ResrvMem_flags(needed, true))
extern void _ResrvMem_flags(Size needed, bool sys_p);

#define PurgeMem(needed) (_PurgeMem_flags(needed, false))
#define PurgeMemSys(needed) (_PurgeMem_flags(needed, true))
extern void _PurgeMem_flags(Size needed, bool sys_p);

#define MaxBlock() (_MaxBlock_flags(false))
#define MaxBlockSys() (_MaxBlock_flags(true))
extern Size _MaxBlock_flags(bool sys_p);

#define PurgeSpace(totalp, congtigp) \
    (_PurgeSpace_flags(totalp, contigp, false))
#define PurgeSpaceSys(totalp, congtigp) \
    (_PurgeSpace_flags(totalp, contigp, true))
extern void _PurgeSpace_flags(Size *totalp, Size *contigp, bool sys_p);

/* ### cliff bogofunc; should go away */
extern void ROMlib_installhandle(Handle sh, Handle dh);

extern void ROMlib_InitZones();
extern OSErr MemError(void);

extern SignedByte HGetState(Handle h);
extern void HSetState(Handle h, SignedByte flags);
extern void HLock(Handle h);
extern void HUnlock(Handle h);
extern void HPurge(Handle h);
extern void HNoPurge(Handle h);
extern void HSetRBit(Handle h);
extern void HClrRBit(Handle h);
extern void InitApplZone(void);
extern void SetApplBase(Ptr newbase);
extern void MoreMasters(void);
extern void InitZone(GrowZoneProcPtr pGrowZone, int16_t cMoreMasters,
                     Ptr limitPtr, THz startPtr);
extern THz GetZone(void);
extern void SetZone(THz hz);
extern void DisposHandle(Handle h);
extern Size GetHandleSize(Handle h);
extern void SetHandleSize(Handle h, Size newsize);
extern THz HandleZone(Handle h);
extern void ReallocHandle(Handle h, Size size);
extern void DisposPtr(Ptr p);
extern Size GetPtrSize(Ptr p);
extern void SetPtrSize(Ptr p, Size newsize);
extern THz PtrZone(Ptr p);
extern void BlockMove(Ptr src, Ptr dst, Size cnt);
extern void BlockMoveData(Ptr src, Ptr dst, Size cnt);
extern void MaxApplZone(void);
extern void MoveHHi(Handle h);
extern void SetApplLimit(Ptr newlimit);
extern void SetGrowZone(GrowZoneProcPtr newgz);
extern void EmptyHandle(Handle h);
extern THz SystemZone(void);
extern THz ApplicZone(void);
extern Size StackSpace(void);

/* temporary memory functions; see tempmem.c */
extern pascal trap int32_t C_TempFreeMem(void);
PASCAL_FUNCTION(TempFreeMem);
extern pascal trap Size C_TempMaxMem(GUEST<Size> *grow);
PASCAL_FUNCTION(TempMaxMem);
extern pascal trap Ptr C_TempTopMem(void);
PASCAL_FUNCTION(TempTopMem);
extern pascal trap Handle C_TempNewHandle(Size logical_size, GUEST<OSErr> *result_code);
PASCAL_FUNCTION(TempNewHandle);
extern pascal trap void C_TempHLock(Handle h, GUEST<OSErr> *result_code);
PASCAL_FUNCTION(TempHLock);
extern pascal trap void C_TempHUnlock(Handle h, GUEST<OSErr> *result_code);
PASCAL_FUNCTION(TempHUnlock);
extern pascal trap void C_TempDisposeHandle(Handle h, GUEST<OSErr> *result_code);
PASCAL_FUNCTION(TempDisposeHandle);
}
#endif /* _MEMORY_MGR_H_ */

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

const LowMemGlobal<Ptr> MemTop { 0x108 }; // MemoryMgr IMII-19 (true);
const LowMemGlobal<Ptr> BufPtr { 0x10C }; // MemoryMgr IMII-19 (true-b);
const LowMemGlobal<Ptr> HeapEnd { 0x114 }; // MemoryMgr IMII-19 (true);
const LowMemGlobal<THz> TheZone { 0x118 }; // MemoryMgr IMII-31 (true);
const LowMemGlobal<Ptr> ApplLimit { 0x130 }; // MemoryMgr IMII-19 (true);
const LowMemGlobal<INTEGER> MemErr { 0x220 }; // MemoryMgr IMIV-80 (true);
const LowMemGlobal<THz> SysZone { 0x2A6 }; // MemoryMgr IMII-19 (true);
const LowMemGlobal<THz> ApplZone { 0x2AA }; // MemoryMgr IMII-19 (true);
const LowMemGlobal<Ptr> ROMBase { 0x2AE }; // MemoryMgr IMIV-236 (true-b);
const LowMemGlobal<Ptr> RAMBase { 0x2B2 }; // MemoryMgr IMI-87 (false);
const LowMemGlobal<Ptr> heapcheck { 0x316 }; // MemoryMgr SysEqu.a (true-b);
const LowMemGlobal<LONGINT> Lo3Bytes { 0x31A }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<LONGINT> MinStack { 0x31E }; // MemoryMgr IMII-17 (true-b);
const LowMemGlobal<LONGINT> DefltStack { 0x322 }; // MemoryMgr IMII-17 (true-b);
const LowMemGlobal<Handle> GZRootHnd { 0x328 }; // MemoryMgr IMI-43 (true);
const LowMemGlobal<Handle> GZMoveHnd { 0x330 }; // MemoryMgr LowMem.h (false);
const LowMemGlobal<ProcPtr> IAZNotify { 0x33C }; // MemoryMgr ThinkC (true-b);
const LowMemGlobal<Ptr> CurrentA5 { 0x904 }; // MemoryMgr IMI-95 (true);
const LowMemGlobal<Ptr> CurStackBase { 0x908 }; // MemoryMgr IMII-19 (true-b);

const LowMemGlobal<Byte[20]> Scratch20 { 0x1E4 }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<Byte[8]> ToolScratch { 0x9CE }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<Byte[8]> Scratch8 { 0x9FA }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<LONGINT> OneOne { 0xA02 }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<LONGINT> MinusOne { 0xA06 }; // MemoryMgr IMI-85 (true);
const LowMemGlobal<Byte[12]> ApplScratch { 0xA78 }; // MemoryMgr IMI-85 (true);

/* traps which can have a `sys' or `clear' bit set */
#define SYSBIT (1 << 10)
#define CLRBIT (1 << 9)


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
/*
CREATE_FUNCTION_WRAPPER(
    PascalTrap<
        decltype(_NewPtr_flags) COMMA &_NewPtr_flags COMMA 0xA11E COMMA
        callconv::Register<callconv::A<0> (callconv::D<0> COMMA callconv::TrapBit<SYSBIT> COMMA callconv::TrapBit<CLRBIT>)>
    >, 
    stub_NewPtr, &_NewPtr_flags, "NewPtr");*/

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

DISPATCHER_TRAP(OSDispatch, 0xA88F, StackW);

/* temporary memory functions; see tempmem.c */
extern int32_t C_TempFreeMem(void);
PASCAL_SUBTRAP(TempFreeMem, 0xA88F, 0x0018, OSDispatch);
extern Size C_TempMaxMem(GUEST<Size> *grow);
PASCAL_SUBTRAP(TempMaxMem, 0xA88F, 0x0015, OSDispatch);
extern Ptr C_TempTopMem(void);
PASCAL_SUBTRAP(TempTopMem, 0xA88F, 0x0016, OSDispatch);
extern Handle C_TempNewHandle(Size logical_size, GUEST<OSErr> *result_code);
PASCAL_SUBTRAP(TempNewHandle, 0xA88F, 0x001D, OSDispatch);
extern void C_TempHLock(Handle h, GUEST<OSErr> *result_code);
PASCAL_SUBTRAP(TempHLock, 0xA88F, 0x001E, OSDispatch);
extern void C_TempHUnlock(Handle h, GUEST<OSErr> *result_code);
PASCAL_SUBTRAP(TempHUnlock, 0xA88F, 0x001F, OSDispatch);
extern void C_TempDisposeHandle(Handle h, GUEST<OSErr> *result_code);
PASCAL_SUBTRAP(TempDisposeHandle, 0xA88F, 0x0020, OSDispatch);
}
#endif /* _MEMORY_MGR_H_ */

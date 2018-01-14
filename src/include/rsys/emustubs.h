#if !defined(_RSYS_EMUSTUBS_H_)
#define _RSYS_EMUSTUBS_H_

/*
 * Copyright 1995, 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
struct adbop_t
{
    GUEST_STRUCT;
    GUEST<Ptr> buffer;
    GUEST<ProcPtr> proc;
    GUEST<Ptr> data;
};

typedef struct comm_toolbox_dispatch_args
{
    GUEST_STRUCT;
    GUEST<int16_t> selector;

    union {
        struct
        {
            GUEST<int16_t> n_items;
            GUEST<DialogPtr> dp;
        } shorten_args;
        struct
        {
            GUEST<DITLMethod> method;
            GUEST<Handle> new_items_h;
            GUEST<DialogPtr> dp;
        } append_args;
        struct
        {
            GUEST<DialogPtr> dp;
        } count_args;
        struct
        {
            GUEST<QElemPtr> qp;
        } crm_args;
    } args;
} comm_toolbox_dispatch_args_t;

typedef void *voidptr;

struct initzonehiddenargs_t
{
    GUEST_STRUCT;
    GUEST<voidptr> startPtr;
    GUEST<voidptr> limitPtr;
    GUEST<INTEGER> cMoreMasters;
    GUEST<GrowZoneProcPtr> pGrowZone;
};

extern void ROMlib_GetTrapAddress_helper(uint32_t *d0p, uint32_t d1, uint32_t *a0p);
extern void ROMlib_reset_bad_trap_addresses(void);
extern void C_pack8_unknown_selector(void);

extern syn68k_addr_t _Unimplemented(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SwapMMUMode(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Launch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Chain(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _FInitQueue(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _InitZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _FreeMem(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MaxMem(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _NewPtr(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _DisposPtr(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetPtrSize(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetPtrSize(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _NewHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _DisposHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetHandleSize(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetHandleSize(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HandleZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ReallocHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _RecoverHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HLock(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HUnlock(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _EmptyHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _InitApplZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetApplLimit(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _BlockMove(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PostEvent(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _OSEventAvail(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetOSEvent(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _FlushEvents(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _VInstall(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _VRemove(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SlotManager(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SlotVInstall(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SlotVRemove(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MoreMasters(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ReadDateTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetDateTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Delay(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _EqualString(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _DrvrInstall(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _DrvrRemove(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ResrvMem(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetTrapAddress(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetTrapAddress(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PtrZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HPurge(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HNoPurge(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetGrowZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _CompactMem(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PurgeMem(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _RelString(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _UprString(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _StripAddress(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetApplBase(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _InsTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _RmvTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PrimeTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _NMInstall(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _NMRemove(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HFSDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MaxBlock(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PurgeSpace(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MaxApplZone(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MoveHHi(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _StackSpace(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _NewEmptyHandle(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HSetRBit(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HClrRBit(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HGetState(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HSetState(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _CountADBs(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetIndADB(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetADBInfo(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetADBInfo(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ADBReInit(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ADBOp(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SysEnvirons(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HWPriv(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Microseconds(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Gestalt(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ResourceStub(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _TEDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ScriptUtil(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PrGlue(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Dequeue(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Enqueue(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Secs2Date(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Date2Secs(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HandToHand(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PtrToXHand(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PtrToHand(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HandAndHand(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack0(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack2(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack3(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack4(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack5(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack6(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack7(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack8(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack11(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack12(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PtrAndHand(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _LoadSeg(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack14(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Pack15(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _CommToolboxDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _OSDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _FontDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HighLevelFSDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ResourceDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _DialogDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _modeswitch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _WackyQD32Trap(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PaletteDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _QDExtensions(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _ShutDown(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _AliasDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _WriteParam(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _InitUtil(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _flushcache(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Key1Trans(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Key2Trans(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Fix2X(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _Frac2X(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SCSIDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _IMVI_LowerText(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SoundDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _QuickTime(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _IconDispatch(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetDefaultStartup(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetDefaultStartup(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetVideoDefault(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetVideoDefault(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _GetOSDefault(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _SetOSDefault(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _IMVI_ReadXPRam(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _bad_trap_unimplemented(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _PhysicalGestalt(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _IMVI_PPC(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _HFSRoutines(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _CodeFragment(syn68k_addr_t ignoreme, void **ignoreme2);
extern syn68k_addr_t _MixedMode(syn68k_addr_t ignoreme, void **ignoreme2);
}
#endif

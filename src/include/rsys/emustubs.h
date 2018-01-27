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
PASCAL_FUNCTION(pack8_unknown_selector);

RAW_68K_TRAP(Unimplemented, 0xA89F);
RAW_68K_TRAP(SwapMMUMode, 0xA05D);
RAW_68K_TRAP(Launch, 0xA9F2);
RAW_68K_TRAP(Chain, 0xA9F3);
RAW_68K_TRAP(FInitQueue, 0xA016);
RAW_68K_TRAP(InitZone, 0xA019);
RAW_68K_TRAP(GetZone, 0xA11A);
RAW_68K_TRAP(SetZone, 0xA01B);
RAW_68K_TRAP(FreeMem, 0xA01C);
RAW_68K_TRAP(MaxMem, 0xA11D);
RAW_68K_TRAP(NewPtr, 0xA11E);
RAW_68K_TRAP(DisposPtr, 0xA01F);
RAW_68K_TRAP(SetPtrSize, 0xA020);
RAW_68K_TRAP(GetPtrSize, 0xA021);
RAW_68K_TRAP(NewHandle, 0xA122);
RAW_68K_TRAP(DisposHandle, 0xA023);
RAW_68K_TRAP(SetHandleSize, 0xA024);
RAW_68K_TRAP(GetHandleSize, 0xA025);
RAW_68K_TRAP(HandleZone, 0xA126);
RAW_68K_TRAP(ReallocHandle, 0xA027);
RAW_68K_TRAP(RecoverHandle, 0xA128);
RAW_68K_TRAP(HLock, 0xA029);
RAW_68K_TRAP(HUnlock, 0xA02A);
RAW_68K_TRAP(EmptyHandle, 0xA02B);
RAW_68K_TRAP(InitApplZone, 0xA02C);
RAW_68K_TRAP(SetApplLimit, 0xA02D);
RAW_68K_TRAP(BlockMove, 0xA02E);
RAW_68K_TRAP(PostEvent, 0xA02F);
RAW_68K_TRAP(OSEventAvail, 0xA030);
RAW_68K_TRAP(GetOSEvent, 0xA031);
RAW_68K_TRAP(FlushEvents, 0xA032);
RAW_68K_TRAP(VInstall, 0xA033);
RAW_68K_TRAP(VRemove, 0xA034);
RAW_68K_TRAP(SlotManager, 0xA06E);
RAW_68K_TRAP(SlotVInstall, 0xA06F);
RAW_68K_TRAP(SlotVRemove, 0xA070);
RAW_68K_TRAP(MoreMasters, 0xA036);
RAW_68K_TRAP(ReadDateTime, 0xA039);
RAW_68K_TRAP(SetDateTime, 0xA03A);
RAW_68K_TRAP(Delay, 0xA03B);
RAW_68K_TRAP(EqualString, 0xA03C);
RAW_68K_TRAP(DrvrInstall, 0xA03D);
RAW_68K_TRAP(DrvrRemove, 0xA03E);
RAW_68K_TRAP(ResrvMem, 0xA040);
RAW_68K_TRAP(GetTrapAddress, 0xA146);
RAW_68K_TRAP(SetTrapAddress, 0xA047);
RAW_68K_TRAP(PtrZone, 0xA148);
RAW_68K_TRAP(HPurge, 0xA049);
RAW_68K_TRAP(HNoPurge, 0xA04A);
RAW_68K_TRAP(SetGrowZone, 0xA04B);
RAW_68K_TRAP(CompactMem, 0xA04C);
RAW_68K_TRAP(PurgeMem, 0xA04D);
RAW_68K_TRAP(RelString, 0xA050);
RAW_68K_TRAP(UprString, 0xA054);
RAW_68K_TRAP(StripAddress, 0xA055);
RAW_68K_TRAP(SetApplBase, 0xA057);
RAW_68K_TRAP(InsTime, 0xA058);
RAW_68K_TRAP(RmvTime, 0xA059);
RAW_68K_TRAP(PrimeTime, 0xA05A);
RAW_68K_TRAP(NMInstall, 0xA05E);
RAW_68K_TRAP(NMRemove, 0xA05F);
RAW_68K_TRAP(HFSDispatch, 0xA260);
RAW_68K_TRAP(MaxBlock, 0xA061);
RAW_68K_TRAP(PurgeSpace, 0xA062);
RAW_68K_TRAP(MaxApplZone, 0xA063);
RAW_68K_TRAP(MoveHHi, 0xA064);
RAW_68K_TRAP(StackSpace, 0xA065);
RAW_68K_TRAP(NewEmptyHandle, 0xA166);
RAW_68K_TRAP(HSetRBit, 0xA067);
RAW_68K_TRAP(HClrRBit, 0xA068);
RAW_68K_TRAP(HGetState, 0xA069);
RAW_68K_TRAP(HSetState, 0xA06A);
RAW_68K_TRAP(CountADBs, 0xA077);
RAW_68K_TRAP(GetIndADB, 0xA078);
RAW_68K_TRAP(GetADBInfo, 0xA079);
RAW_68K_TRAP(SetADBInfo, 0xA07A);
RAW_68K_TRAP(ADBReInit, 0xA07B);
RAW_68K_TRAP(ADBOp, 0xA07C);
RAW_68K_TRAP(SysEnvirons, 0xA090);
RAW_68K_TRAP(HWPriv, 0xA198);
RAW_68K_TRAP(Microseconds, 0xA193);
RAW_68K_TRAP(Gestalt, 0xA1AD);
RAW_68K_TRAP(ResourceStub, 0xA0FC);
RAW_68K_TRAP(ScriptUtil, 0xA8B5);
RAW_68K_TRAP(PrGlue, 0xA8FD);
RAW_68K_TRAP(Dequeue, 0xA96E);
RAW_68K_TRAP(Enqueue, 0xA96F);
RAW_68K_TRAP(Secs2Date, 0xA9C6);
RAW_68K_TRAP(Date2Secs, 0xA9C7);
RAW_68K_TRAP(HandToHand, 0xA9E1);
RAW_68K_TRAP(PtrToXHand, 0xA9E2);
RAW_68K_TRAP(PtrToHand, 0xA9E3);
RAW_68K_TRAP(HandAndHand, 0xA9E4);
RAW_68K_TRAP(Pack0, 0xA9E7);
RAW_68K_TRAP(Pack2, 0xA9E9);
RAW_68K_TRAP(Pack4, 0xA9EB);
RAW_68K_TRAP(Pack5, 0xA9EC);
RAW_68K_TRAP(Pack8, 0xA816);
RAW_68K_TRAP(PtrAndHand, 0xA9EF);
RAW_68K_TRAP(LoadSeg, 0xA9F0);
RAW_68K_TRAP(CommToolboxDispatch, 0xA08B);
RAW_68K_TRAP(OSDispatch, 0xA88F);
//RAW_68K_FUNCTION(modeswitch);
RAW_68K_TRAP(WackyQD32Trap, 0xAB03);
RAW_68K_TRAP(PaletteDispatch, 0xAAA2);
RAW_68K_TRAP(QDExtensions, 0xAB1D);
RAW_68K_TRAP(AliasDispatch, 0xA823);
RAW_68K_TRAP(WriteParam, 0xA038);
RAW_68K_TRAP(InitUtil, 0xA03F);
RAW_68K_TRAP(flushcache, 0xA0BD);
RAW_68K_FUNCTION(Key1Trans);
RAW_68K_FUNCTION(Key2Trans);
RAW_68K_TRAP(Fix2X, 0xA843);
RAW_68K_TRAP(Frac2X, 0xA845);
RAW_68K_TRAP(SCSIDispatch, 0xA815);
RAW_68K_TRAP(IMVI_LowerText, 0xA056);
RAW_68K_TRAP(SoundDispatch, 0xA800);
RAW_68K_TRAP(QuickTime, 0xAAAA);
RAW_68K_TRAP(GetDefaultStartup, 0xA07D);
RAW_68K_TRAP(SetDefaultStartup, 0xA07E);
RAW_68K_TRAP(GetVideoDefault, 0xA080);
RAW_68K_TRAP(SetVideoDefault, 0xA081);
RAW_68K_TRAP(GetOSDefault, 0xA084);
RAW_68K_TRAP(SetOSDefault, 0xA083);
RAW_68K_TRAP(IMVI_ReadXPRam, 0xA051);
RAW_68K_FUNCTION(bad_trap_unimplemented);
RAW_68K_TRAP(IMVI_PPC, 0xA0DD);
RAW_68K_FUNCTION(HFSRoutines);
//RAW_68K_FUNCTION(CodeFragment);
//RAW_68K_FUNCTION(MixedMode);
}
#endif

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
RAW_68K_FUNCTION(SwapMMUMode);
RAW_68K_TRAP(Launch, 0xA9F2);
RAW_68K_TRAP(Chain, 0xA9F3);
RAW_68K_FUNCTION(FInitQueue);
RAW_68K_FUNCTION(InitZone);
RAW_68K_FUNCTION(GetZone);
RAW_68K_FUNCTION(SetZone);
RAW_68K_FUNCTION(FreeMem);
RAW_68K_FUNCTION(MaxMem);
RAW_68K_FUNCTION(NewPtr);
RAW_68K_FUNCTION(DisposPtr);
RAW_68K_FUNCTION(SetPtrSize);
RAW_68K_FUNCTION(GetPtrSize);
RAW_68K_FUNCTION(NewHandle);
RAW_68K_FUNCTION(DisposHandle);
RAW_68K_FUNCTION(SetHandleSize);
RAW_68K_FUNCTION(GetHandleSize);
RAW_68K_FUNCTION(HandleZone);
RAW_68K_FUNCTION(ReallocHandle);
RAW_68K_FUNCTION(RecoverHandle);
RAW_68K_FUNCTION(HLock);
RAW_68K_FUNCTION(HUnlock);
RAW_68K_FUNCTION(EmptyHandle);
RAW_68K_FUNCTION(InitApplZone);
RAW_68K_FUNCTION(SetApplLimit);
RAW_68K_FUNCTION(BlockMove);
RAW_68K_FUNCTION(PostEvent);
RAW_68K_FUNCTION(OSEventAvail);
RAW_68K_FUNCTION(GetOSEvent);
RAW_68K_FUNCTION(FlushEvents);
RAW_68K_FUNCTION(VInstall);
RAW_68K_FUNCTION(VRemove);
RAW_68K_FUNCTION(SlotManager);
RAW_68K_FUNCTION(SlotVInstall);
RAW_68K_FUNCTION(SlotVRemove);
RAW_68K_FUNCTION(MoreMasters);
RAW_68K_FUNCTION(ReadDateTime);
RAW_68K_FUNCTION(SetDateTime);
RAW_68K_FUNCTION(Delay);
RAW_68K_FUNCTION(EqualString);
RAW_68K_FUNCTION(DrvrInstall);
RAW_68K_FUNCTION(DrvrRemove);
RAW_68K_FUNCTION(ResrvMem);
RAW_68K_FUNCTION(GetTrapAddress);
RAW_68K_FUNCTION(SetTrapAddress);
RAW_68K_FUNCTION(PtrZone);
RAW_68K_FUNCTION(HPurge);
RAW_68K_FUNCTION(HNoPurge);
RAW_68K_FUNCTION(SetGrowZone);
RAW_68K_FUNCTION(CompactMem);
RAW_68K_FUNCTION(PurgeMem);
RAW_68K_FUNCTION(RelString);
RAW_68K_FUNCTION(UprString);
RAW_68K_FUNCTION(StripAddress);
RAW_68K_FUNCTION(SetApplBase);
RAW_68K_FUNCTION(InsTime);
RAW_68K_FUNCTION(RmvTime);
RAW_68K_FUNCTION(PrimeTime);
RAW_68K_FUNCTION(NMInstall);
RAW_68K_FUNCTION(NMRemove);
RAW_68K_FUNCTION(HFSDispatch);
RAW_68K_FUNCTION(MaxBlock);
RAW_68K_FUNCTION(PurgeSpace);
RAW_68K_FUNCTION(MaxApplZone);
RAW_68K_FUNCTION(MoveHHi);
RAW_68K_FUNCTION(StackSpace);
RAW_68K_FUNCTION(NewEmptyHandle);
RAW_68K_FUNCTION(HSetRBit);
RAW_68K_FUNCTION(HClrRBit);
RAW_68K_FUNCTION(HGetState);
RAW_68K_FUNCTION(HSetState);
RAW_68K_FUNCTION(CountADBs);
RAW_68K_FUNCTION(GetIndADB);
RAW_68K_FUNCTION(GetADBInfo);
RAW_68K_FUNCTION(SetADBInfo);
RAW_68K_FUNCTION(ADBReInit);
RAW_68K_FUNCTION(ADBOp);
RAW_68K_FUNCTION(SysEnvirons);
RAW_68K_FUNCTION(HWPriv);
RAW_68K_TRAP(Microseconds, 0xA193);
RAW_68K_FUNCTION(Gestalt);
RAW_68K_FUNCTION(ResourceStub);
RAW_68K_TRAP(TEDispatch, 0xA83D);
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
RAW_68K_TRAP(Pack3, 0xA9EA);
RAW_68K_TRAP(Pack4, 0xA9EB);
RAW_68K_TRAP(Pack5, 0xA9EC);
RAW_68K_TRAP(Pack6, 0xA9ED);
RAW_68K_TRAP(Pack7, 0xA9EE);
RAW_68K_TRAP(Pack8, 0xA816);
RAW_68K_TRAP(Pack11, 0xA82D);
RAW_68K_TRAP(Pack12, 0xA82E);
RAW_68K_TRAP(PtrAndHand, 0xA9EF);
RAW_68K_TRAP(LoadSeg, 0xA9F0);
RAW_68K_TRAP(Pack14, 0xA830);
RAW_68K_TRAP(Pack15, 0xA831);
RAW_68K_TRAP(CommToolboxDispatch, 0xA08B);
RAW_68K_TRAP(OSDispatch, 0xA88F);
RAW_68K_TRAP(FontDispatch, 0xA854);
RAW_68K_TRAP(HighLevelFSDispatch, 0xAA52);
RAW_68K_TRAP(ResourceDispatch, 0xA822);
RAW_68K_TRAP(DialogDispatch, 0xAA68);
//RAW_68K_FUNCTION(modeswitch);
RAW_68K_TRAP(WackyQD32Trap, 0xAB03);
RAW_68K_TRAP(PaletteDispatch, 0xAAA2);
RAW_68K_TRAP(QDExtensions, 0xAB1D);
RAW_68K_TRAP(ShutDown, 0xA895);
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
RAW_68K_TRAP(IconDispatch, 0xABC9);
RAW_68K_FUNCTION(GetDefaultStartup);
RAW_68K_FUNCTION(SetDefaultStartup);
RAW_68K_FUNCTION(GetVideoDefault);
RAW_68K_FUNCTION(SetVideoDefault);
RAW_68K_FUNCTION(GetOSDefault);
RAW_68K_FUNCTION(SetOSDefault);
RAW_68K_TRAP(IMVI_ReadXPRam, 0xA051);
RAW_68K_FUNCTION(bad_trap_unimplemented);
RAW_68K_TRAP(IMVI_PPC, 0xA0DD);
RAW_68K_FUNCTION(HFSRoutines);
//RAW_68K_FUNCTION(CodeFragment);
//RAW_68K_FUNCTION(MixedMode);
}
#endif

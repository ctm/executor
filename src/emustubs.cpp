/* Copyright 1994, 1995, 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include <stdarg.h>

#include "ResourceMgr.h"
#include "ScriptMgr.h"
#include "TextEdit.h"
#include "ToolboxUtil.h"
#include "ListMgr.h"
#include "SANE.h"
#include "BinaryDecimal.h"
#include "FileMgr.h"
#include "StdFilePkg.h"
#include "DiskInit.h"
#include "PrintMgr.h"
#include "IntlUtil.h"
#include "MemoryMgr.h"
#include "CQuickDraw.h"
#include "SoundMgr.h"
#include "HelpMgr.h"
#include "ADB.h"

#include "SegmentLdr.h"
#include "ToolboxUtil.h"
#include "OSUtil.h"
#include "VRetraceMgr.h"
#include "TimeMgr.h"
#include "ToolboxEvent.h"
#include "OSEvent.h"
#include "Gestalt.h"
#include "rsys/segment.h"
#include "rsys/resource.h"
#include "rsys/toolutil.h"
#include "NotifyMgr.h"
#include "ShutDown.h"
#include "AppleEvents.h"
#include "ProcessMgr.h"
#include "AliasMgr.h"
#include "EditionMgr.h"
#include "FontMgr.h"
#include "Finder.h"
#include "Iconutil.h"
#include "QuickTime.h"
#include "CommTool.h"
#include "SpeechManager.h"

#include "rsys/file.h"
#include "rsys/time.h"
#include "rsys/mman.h"
#include "rsys/prefs.h"
#include "rsys/soundopts.h"
#include "rsys/system_error.h"
#include "rsys/emustubs.h"
#include "rsys/gestalt.h"
#include "rsys/executor.h"
#include "rsys/mixed_mode.h"
#include "rsys/cfm.h"
#include "rsys/mixed_mode.h"

namespace Executor
{
#define RTS() return POPADDR()

#define STUB(x) syn68k_addr_t _##x(syn68k_addr_t ignoreme, \
                                          void *ignoreme2)
void C_unknown574()
{
}

STUB(GetDefaultStartup)
{
    (SYN68K_TO_US(EM_A0))[0] = -1;
    (SYN68K_TO_US(EM_A0))[1] = -1;
    (SYN68K_TO_US(EM_A0))[2] = -1;
    (SYN68K_TO_US(EM_A0))[3] = -33; /* That's what Q610 has */
    RTS();
}

STUB(SetDefaultStartup)
{
    RTS();
}

STUB(GetVideoDefault)
{
    (SYN68K_TO_US(EM_A0))[0] = 0; /* Q610 values */
    (SYN68K_TO_US(EM_A0))[1] = -56;
    RTS();
}

STUB(SetVideoDefault)
{
    RTS();
}

STUB(GetOSDefault)
{
    (SYN68K_TO_US(EM_A0))[0] = 0; /* Q610 values */
    (SYN68K_TO_US(EM_A0))[1] = 1;
    RTS();
}

STUB(SetOSDefault)
{
    RTS();
}

STUB(SwapMMUMode)
{
    EM_D0 &= 0xFFFFFF00;
    EM_D0 |= 0x00000001;
    LM(MMU32Bit) = 0x01; /* TRUE32b */
    RTS();
}

STUB(Launch)
{
    LaunchParamBlockRec *lpbp;
    StringPtr strp;

    lpbp = (LaunchParamBlockRec *)SYN68K_TO_US(EM_A0);
    if(lpbp->launchBlockID == CWC(extendedBlock))
        strp = 0;
    else
        strp = MR(*(GUEST<StringPtr> *)lpbp);
    EM_D0 = NewLaunch(strp, 0, lpbp);
    RTS();
}

STUB(Chain)
{
    Chain(MR(*(GUEST<StringPtr> *)SYN68K_TO_US(EM_A0)), 0);
    RTS();
}

STUB(IMVI_LowerText)
{
    EM_D0 = resNotFound;
    RTS();
}

STUB(SCSIDispatch)
{
    syn68k_addr_t retaddr;

    retaddr = POPADDR();
    EM_A7 += 4; /* get rid of selector and location for errorvalue */
#define scMgrBusyErr 7
    PUSHUW(scMgrBusyErr);
    PUSHADDR(retaddr);
    RTS();
}

STUB(LoadSeg)
{
    syn68k_addr_t retaddr;

    retaddr = POPADDR();

    C_LoadSeg(POPUW());
    PUSHADDR(retaddr - 6);
    RTS();
}

STUB(ResourceStub)
{
    EM_A0 = US_TO_SYN68K_CHECK0(ROMlib_mgetres2(
        (resmaphand)SYN68K_TO_US_CHECK0(EM_A4),
        (resref *)SYN68K_TO_US_CHECK0(EM_A2)));
    RTS();
}

STUB(DrvrInstall)
{
    EM_D0 = -1;
    RTS();
}

STUB(DrvrRemove)
{
    EM_D0 = -1;
    RTS();    
}

STUB(ADBOp)
{
    adbop_t *p;

    p = (adbop_t *)SYN68K_TO_US(EM_A0);

    cpu_state.regs[0].sw.n
        = ADBOp(MR(p->data), MR(p->proc), MR(p->buffer), EM_D0);
    RTS();
}

STUB(Fix2X)
{
    syn68k_addr_t retaddr;
    syn68k_addr_t retp;
    Fixed f;

    retaddr = POPADDR();
    f = (Fixed)POPUL();
    retp = POPADDR();

    R_Fix2X((void *)SYN68K_TO_US(retaddr), f,
            (extended80 *)SYN68K_TO_US(retp));

    PUSHADDR(retp);
    return retaddr;
}

STUB(Frac2X)
{
    syn68k_addr_t retaddr;
    syn68k_addr_t retp;
    Fract f;

    retaddr = POPADDR();
    f = (Fract)POPUL();
    retp = POPADDR();

    R_Frac2X((void *)SYN68K_TO_US(retaddr), f,
             (extended80 *)SYN68K_TO_US(retp));

    PUSHADDR(retp);
    return retaddr;
}


/*
 * NOTE: The LM(Key1Trans) and LM(Key2Trans) implementations are just transcriptions
 *	 of what I had in stubs.s.  I'm still not satisified that we have
 *	 the real semantics of these two routines down.
 */

#define KEYTRANSMACRO()                                      \
    unsigned short uw;                                       \
                                                             \
    uw = EM_D1 << 8;                                         \
    uw |= cpu_state.regs[2].uw.n;                            \
    EM_D0 = (KeyTrans(NULL, uw, (LONGINT *)0) >> 16) & 0xFF; \
    RTS();

STUB(Key1Trans);
STUB(Key2Trans);

STUB(Key1Trans)
{
    KEYTRANSMACRO();
}

STUB(Key2Trans)
{
    KEYTRANSMACRO();
}

STUB(IMVI_PPC)
{
    EM_D0 = paramErr; /* this is good enough for NetScape */
    RTS();
}

STUB(CommToolboxDispatch)
{
    comm_toolbox_dispatch_args_t *arg_block;
    int selector;

    arg_block = (comm_toolbox_dispatch_args_t *)SYN68K_TO_US(EM_A0);

    selector = CW(arg_block->selector);
    switch(selector)
    {
        case 0x0402:
            AppendDITL(MR(arg_block->args.append_args.dp),
                       MR(arg_block->args.append_args.new_items_h),
                       CW(arg_block->args.append_args.method));
            break;
        case 0x0403:
            EM_D0 = CountDITL(MR(arg_block->args.count_args.dp));
            break;
        case 0x0404:
            ShortenDITL(MR(arg_block->args.shorten_args.dp),
                        CW(arg_block->args.shorten_args.n_items));
            break;
        case 1286:
            EM_D0 = CRMGetCRMVersion();
            break;
        case 1282:
            EM_D0 = US_TO_SYN68K_CHECK0(CRMGetHeader());
            break;
        case 1283:
            CRMInstall(MR(arg_block->args.crm_args.qp));
            break;
        case 1284:
            EM_D0 = CRMRemove(MR(arg_block->args.crm_args.qp));
            break;
        case 1285:
            EM_D0 = US_TO_SYN68K_CHECK0(CRMSearch(MR(arg_block->args.crm_args.qp)));
            break;
        case 1281:
            EM_D0 = InitCRM();
            break;
        default:
            warning_unimplemented(NULL_STRING); /* now Quicken 6 will limp */
            EM_D0 = 0;
            break;
    }
    RTS();
}

STUB(PostEvent)
{
    GUEST<EvQElPtr> qelemp;

    // FIXME: #warning the first argument to PPostEvent looks suspicious
    EM_D0 = PPostEvent(EM_A0, EM_D0,
                       (GUEST<EvQElPtr> *)&qelemp);
    EM_A0 = qelemp.raw();
    RTS();
}


#define DIACBIT (1 << 9)
#define CASEBIT (1 << 10)

STUB(EqualString)
{
    EM_D0 = !!ROMlib_RelString((unsigned char *)SYN68K_TO_US_CHECK0(EM_A0),
                               (unsigned char *)SYN68K_TO_US_CHECK0(EM_A1),
                               !!(EM_D1 & CASEBIT),
                               !(EM_D1 & DIACBIT), EM_D0);
    RTS();
}

STUB(RelString)
{
    EM_D0 = ROMlib_RelString((unsigned char *)SYN68K_TO_US_CHECK0(EM_A0),
                             (unsigned char *)SYN68K_TO_US_CHECK0(EM_A1),
                             !!(EM_D1 & CASEBIT),
                             !(EM_D1 & DIACBIT), EM_D0);
    RTS();
}

STUB(UprString)
{
    long savea0;

    savea0 = EM_A0;
    ROMlib_UprString((StringPtr)SYN68K_TO_US_CHECK0(EM_A0),
                     !(EM_D1 & DIACBIT), EM_D0);
    EM_A0 = savea0;
    RTS();
}

STUB(StripAddress)
{
    RTS();
}

STUB(FInitQueue)
{
    RTS();
}

#define GETTRAPNEWBIT (1 << 9)
#define GETTRAPTOOLBIT (1 << 10)

#define UNIMPLEMENTEDINDEX (0x9F)

static long istool(uint32_t *d0p, uint32_t d1)
{
    long retval;

    if(d1 & GETTRAPNEWBIT)
    {
        retval = d1 & GETTRAPTOOLBIT;
        if(retval)
            *d0p &= 0x3FF;
        else
            *d0p &= 0xFF;
    }
    else
    {
        *d0p &= 0x1FF;
        retval = (*d0p > 0x4F) && (*d0p != 0x54) && (*d0p != 0x57);
    }
    return retval;
}

static long unimplementedos(long d0)
{
    long retval;

    switch(d0)
    {
        case 0x77: /* CountADBs */
        case 0x78: /* GetIndADB */
        case 0x79: /* GetADBInfo */
        case 0x7A: /* SetADBInfo */
        case 0x7B: /* ADBReInit */
        case 0x7C: /* ADBOp */
        case 0x3D: /* DrvrInstall */
        case 0x3E: /* DrvrRemove */
        case 0x4F: /* RDrvrInstall */
            retval = 1;
            break;
        case 0x8B: /* Communications Toolbox */
            retval = ROMlib_creator == TICK("KR09"); /* kermit */
            break;
        default:
            retval = 0;
            break;
    }
    return retval;
}

static long unimplementedtool(long d0)
{
    long retval;

    switch(d0)
    {
        case 0x00: /* SoundDispatch -- if sound is off, soundispatch is unimpl */
            retval = ROMlib_PretendSound == soundoff;
            break;
        case 0x8F: /* OSDispatch (Word uses old, undocumented selectors) */
            retval = system_version < 0x700;
            break;
        case 0x30: /* Pack14 */
            retval = 1;
            break;
        case 0xB5: /* ScriptUtil */
            retval = ROMlib_pretend_script ? 0 : 1;
            break;
        default:
            retval = 0;
            break;
    }
    return retval;
}

/*
 * Cheezoid implementation, but we don't have to worry about running out
 * of memory and if we have more than 10 traps displaying them all doesn't
 * buy us much, anyway
 */

static uint16_t bad_traps[10];
static int n_bad_traps = 0;

void
ROMlib_reset_bad_trap_addresses(void)
{
    n_bad_traps = 0;
}

static void
add_to_bad_trap_addresses(bool tool_p, unsigned short index)
{
    int i;
    uint16_t aline_trap;

    aline_trap = 0xA000 + index;
    if(tool_p)
        aline_trap += 0x800;

    for(i = 0; i < n_bad_traps && i < NELEM(bad_traps) && bad_traps[i] != aline_trap; ++i)
        ;
    if(i >= n_bad_traps)
    {
        bad_traps[n_bad_traps % NELEM(bad_traps)] = aline_trap;
        ++n_bad_traps;
    }
}

STUB(bad_trap_unimplemented)
{
    char buf[1024];

    /* TODO: more */
    switch(mostrecenttrap)
    {
        default:
            sprintf(buf,
                    "Fatal error.\r"
                    "Jumped to unimplemented trap handler, "
                    "probably by getting the address of one of these traps: [");
            {
                int i;
                bool need_comma_p;

                need_comma_p = false;
                for(i = 0; i < (int)NELEM(bad_traps) && i < n_bad_traps; ++i)
                {
                    if(need_comma_p)
                        strcat(buf, ",");
                    {
                        char trap_buf[7];
                        sprintf(trap_buf, "0x%04x", bad_traps[i]);
                        gui_assert(trap_buf[6] == 0);
                        strcat(buf, trap_buf);
                    }
                    need_comma_p = true;
                }
            }
            strcat(buf, "].");
            system_error(buf, 0,
                         "Restart", NULL, NULL,
                         NULL, NULL, NULL);
            break;
    }

    ExitToShell();
    return /* dummy */ -1;
}

void
ROMlib_GetTrapAddress_helper(uint32_t *d0p, uint32_t d1, uint32_t *a0p)
{
    bool tool_p;

    if(*d0p == READUL((syn68k_addr_t)0xA198))
        *d0p = 0xA198;
    if(*d0p == READUL((syn68k_addr_t)0xA89F))
        *d0p = 0xA89F;
    tool_p = istool(d0p, d1);
    if(tool_p)
    {
        *a0p = (tooltraptable[unimplementedtool(*d0p) ? UNIMPLEMENTEDINDEX
                                                      : *d0p]);
    }
    else
    {
        *a0p = (unimplementedos(*d0p) ? tooltraptable[UNIMPLEMENTEDINDEX]
                                      : ostraptable[*d0p]);
    }
    if(*a0p == (uint32_t)tooltraptable[UNIMPLEMENTEDINDEX])
    {
        add_to_bad_trap_addresses(tool_p, *d0p);
        *a0p = US_TO_SYN68K_CHECK0((Ptr)&stub_bad_trap_unimplemented);
    }
    *d0p = 0;
}

STUB(GetTrapAddress)
{
    ROMlib_GetTrapAddress_helper(&EM_D0, EM_D1, &EM_A0);
    RTS();
}

STUB(SetTrapAddress)
{
    syn68k_addr_t *tablep;

    if(istool(&EM_D0, EM_D1))
        tablep = tooltraptable;
    else
        tablep = ostraptable;
    tablep[EM_D0] = EM_A0;

    if(EM_D0 != 0xED) /* Temporary MacWrite hack */
        ROMlib_destroy_blocks(0, ~0, true);
    EM_D0 = 0;
    RTS();
}

STUB(Gestalt)
{
    GUEST<LONGINT> l;
    SelectorFunctionUPP oldp;

    switch(EM_D1 & 0xFFFF)
    {
        case 0xA1AD:
        default:
            l = CLC(0);
            EM_D0 = Gestalt(EM_D0, &l);
            EM_A0 = CL(l);
            break;
        case 0xA3AD:
            if(EM_D0 == DONGLE_GESTALT)
                EM_D0 = Gestalt(EM_D0, (GUEST<LONGINT> *)SYN68K_TO_US_CHECK0(EM_A0));
            else
                EM_D0 = NewGestalt(EM_D0, (SelectorFunctionUPP)SYN68K_TO_US_CHECK0(EM_A0));
            break;
        case 0xA5AD:
            EM_D0 = ReplaceGestalt(EM_D0, (SelectorFunctionUPP)SYN68K_TO_US_CHECK0(EM_A0),
                                   &oldp);
            EM_A0 = US_TO_SYN68K_CHECK0((void *)oldp);
            break;
        case 0xA7AD:
            gui_abort();
            /* GetGestaltProcPtr(); no docs on this call */
            break;
    }
    RTS();
}

/* unlike the 68k version, every unknown trap gets vectored to
   `Unimplemented ()' */

STUB(Unimplemented)
{
    char buf[1024];

    switch(mostrecenttrap)
    {
        default:
            sprintf(buf,
                    "Fatal error.\r"
                    "encountered unknown, unimplemented trap `%X'.",
                    mostrecenttrap);
            system_error(buf, 0,
                         "Restart", NULL, NULL,
                         NULL, NULL, NULL);
            break;
    }

    ExitToShell();
    RTS(); /* in case we want to get return from within gdb */
    return /* dummy */ -1;
}

/*
 * This is just to trick out NIH Image... it's really not supported
 */

/* #warning SlotManager not properly implemented */

STUB(SlotManager)
{
    EM_D0 = -300; /* smEmptySlot */
    RTS();
}

STUB(WackyQD32Trap)
{
    gui_fatal("This trap shouldn't be called");
}

STUB(InitZone)
{
    initzonehiddenargs_t *ip;

    ip = (initzonehiddenargs_t *)SYN68K_TO_US(EM_A0);
    InitZone(MR(ip->pGrowZone), CW(ip->cMoreMasters),
             (Ptr)MR(ip->limitPtr), (THz)MR(ip->startPtr));
    EM_D0 = CW(LM(MemErr));
    RTS();
}

STUB(Microseconds)
{
    unsigned long ms = msecs_elapsed();
    EM_D0 = ms * 1000;
    EM_A0 = ((uint64_t)ms * 1000) >> 32;
    RTS();
}

STUB(IMVI_ReadXPRam)
{
    /* I, ctm, don't have the specifics for ReadXPram, but Bolo suggests that
     when d0 is the value below that a 12 byte block is filled in, with some
     sort of time info at offset 8 off of a0. */

    if(EM_D0 == 786660)
    {
        /* memset((char *)SYN68K_TO_US(EM_A0), 0, 12); not needed */
        *(long *)((char *)SYN68K_TO_US(EM_A0) + 8) = 0;
    }
    RTS();
}


/*
 * modeswitch is special; we don't return to from where we came.
 * instead we pick up the return address from the stack
 */

STUB(modeswitch)
{
    syn68k_addr_t retaddr;
    RoutineDescriptor *rp;
    va_list unused;
    ProcInfoType procinfo;
    int convention;
    int n_routines;
    int i;

    EM_A7 += 4;
    retaddr = POPADDR();
    rp = (RoutineDescriptor *)(SYN68K_TO_US(ignoreme)); /* UGH! */

    n_routines = CW(rp->routineCount) + 1;
    for(i = 0;
        i < n_routines && rp->routineRecords[i].ISA != CBC(kPowerPCISA);
        ++i)
        ;
    if(i == n_routines)
    {
        fprintf(stderr, "*** bad modeswitch***\n");
        return retaddr;
    }

    procinfo = CL(rp->routineRecords[i].procInfo);
    convention = procinfo & 0xf;

    if(convention == kRegisterBased)
    {
        uint32_t retval;
        int retwidth;
        int ret_reg;
        uint32_t mask;

        warning_trace_info("calling universal from mixed mode using "
                           "register conventions");
#if 0
        retval = CallUniversalProc_from_native_common(unused, args_via_68k_regs,
                                                      MR(rp->routineRecords[i].procDescriptor),
                                                      procinfo);
#else
        retval = 0;
#endif
        retwidth = (procinfo >> 4) & 3;
        ret_reg = (procinfo >> 6) & 31;
        switch(retwidth)
        {
            default:
            case 0:
                mask = 0;
                break;
            case 1:
                mask = 0xff;
                break;
            case 2:
                mask = 0xffff;
                break;
            case 3:
                mask = 0xffffffff;
                break;
        }
        if(ret_reg <= kRegisterA6)
        {
            if(mask)
            {
                uint32_t *regp;
                static int map[] = {
                    0, /* kRegisterD0,  0 */
                    1, /* kRegisterD1,  1 */
                    2, /* kRegisterD2,  2 */
                    3, /* kRegisterD3,  3 */
                    8, /* kRegisterA0,  4 */
                    9, /* kRegisterA1,  5 */
                    10, /* kRegisterA2,  6 */
                    11, /* kRegisterA3,  7 */
                    4, /* kRegisterD4,  8 */
                    5, /* kRegisterD5,  9 */
                    6, /* kRegisterD6, 10 */
                    7, /* kREgisterD7, 11 */
                    12, /* kRegisterA4, 12 */
                    13, /* kRegisterA5, 13 */
                    14, /* kRegisterA6, 14 */
                };

                regp = &cpu_state.regs[map[ret_reg]].ul.n;
                *regp &= ~mask;
                *regp |= retval & mask;
            }
        }
        else
        {
            switch(ret_reg)
            {
                case kCCRegisterCBit:
                case kCCRegisterVBit:
                case kCCRegisterZBit:
                case kCCRegisterNBit:
                case kCCRegisterXBit:
                    warning_unimplemented("ret_reg = %d", ret_reg);
                    break;
            }
        }
    }
    else if(convention > kRegisterBased)
    {
        warning_unimplemented("ignoring convention %d\n", convention);
    }
    else
    {
        int rettype;
        uint32_t retval;

        warning_trace_info("calling universal from mixed mode");
#if 0
        retval = CallUniversalProc_from_native_common(unused, args_via_68k_stack,
                                                      MR(rp->routineRecords[i].procDescriptor),
                                                      procinfo);
#else
        retval = 0;
#endif
        warning_trace_info("just got back from calling universal from mixed mode");
        rettype = (procinfo >> kCallingConventionWidth)
            & ((1 << kResultSizeWidth) - 1);
        switch(rettype)
        {
            case kOneByteCode:
                WRITEUW(EM_A7, 0);
                WRITEUB(EM_A7, retval);
                break;
            case kTwoByteCode:
                WRITEUW(EM_A7, retval);
                break;
            case kFourByteCode:
                WRITEUL(EM_A7, retval);
                break;
        }
    }
    return retaddr;
}

}

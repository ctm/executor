/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if defined(powerpc) || defined(__ppc__)

#include "rsys/common.h"

#include "FileMgr.h"
#include "MemoryMgr.h"

#include "rsys/mixed_mode.h"

#include "rsys/file.h"

using namespace Executor;

UniversalProcPtr Executor::C_NewRoutineDescriptor(
    ProcPtr proc, ProcInfoType info, ISAType isa)
{
    UniversalProcPtr retval;

    if(!proc)
        retval = NULL;
    else
    {
        RoutineDescriptor *p;

        p = (RoutineDescriptor *)NewPtr(sizeof *p);
        p->goMixedModeTrap = CWC(MIXED_MODE_TRAP);
        ROMlib_destroy_blocks((syn68k_addr_t)
                                  US_TO_SYN68K(&p->goMixedModeTrap),
                              2, true);
        p->version = CBC(kRoutineDescriptorVersion);
        p->routineDescriptorFlags = CBC(0);
        p->reserved1 = CLC(0);
        p->reserved2 = CBC(0);
        p->selectorInfo = CBC(0);
        p->routineCount = CWC(0);
        p->routineRecords[0].procInfo = CL(info);
        p->routineRecords[0].reserved1 = CBC(0);
        p->routineRecords[0].ISA = CB(isa);
        p->routineRecords[0].routineFlags = CWC(kSelectorsAreNotIndexable);
        p->routineRecords[0].procDescriptor = RM(proc);
        p->routineRecords[0].reserved2 = CLC(0);
        p->routineRecords[0].selector = CLC(0);
        retval = (UniversalProcPtr)p;
    }
    return retval;
}

void Executor::C_DisposeRoutineDescriptor(UniversalProcPtr ptr)
{
    DisposPtr((Ptr)ptr);
    warning_trace_info(NULL_STRING);
}

UniversalProcPtr Executor::C_NewFatRoutineDescriptor(
    ProcPtr m68k, ProcPtr ppc, ProcInfoType info)
{
    warning_unimplemented(NULL_STRING);
    *(long *)-1 = -1; /* abort */
    return NULL;
}

OSErr Executor::C_SaveMixedModeState(void *statep, uint32_t vers)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

OSErr Executor::C_RestoreMixedModeState(void *statep, uint32_t vers)
{
    warning_unimplemented(NULL_STRING);
    return paramErr;
}

#endif

/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if defined (powerpc) || defined (__ppc__)

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mixed_mode[] =
	    "$Id: mixed_mode.c 63 2004-12-24 18:19:43Z ctm $";
#endif


#include "rsys/common.h"

#include "FileMgr.h"
#include "MemoryMgr.h"

#include "rsys/mixed_mode.h"

#include "rsys/file.h"

using namespace Executor;

P3 (PUBLIC pascal trap, UniversalProcPtr, NewRoutineDescriptor, ProcPtr, proc,
    ProcInfoType, info, ISAType, isa)
{
  UniversalProcPtr retval;

  if (!proc)
    retval = NULL;
  else
    {
      RoutineDescriptor *p;

      p = (RoutineDescriptor *) NewPtr (sizeof *p);
      p->goMixedModeTrap = CWC (MIXED_MODE_TRAP);
      ROMlib_destroy_blocks ((syn68k_addr_t)
			     US_TO_SYN68K (&p->goMixedModeTrap), 2, true);
      p->version = CBC (kRoutineDescriptorVersion);
      p->routineDescriptorFlags = CBC (0);
      p->reserved1 = CLC (0);
      p->reserved2 = CBC (0);
      p->selectorInfo = CBC (0);
      p->routineCount = CWC (0);
      p->routineRecords[0].procInfo = CL (info);
      p->routineRecords[0].reserved1 = CBC (0);
      p->routineRecords[0].ISA = CB (isa);
      p->routineRecords[0].routineFlags = CWC (kSelectorsAreNotIndexable);
      p->routineRecords[0].procDescriptor = RM (proc);
      p->routineRecords[0].reserved2 = CLC (0);
      p->routineRecords[0].selector = CLC (0);
      retval = (UniversalProcPtr) p;
    }
  return retval;
}

P1 (PUBLIC pascal trap, void, DisposeRoutineDescriptor, UniversalProcPtr, ptr)
{
  DisposPtr ((Ptr) ptr);
  warning_trace_info (NULL_STRING);
}

P3 (PUBLIC pascal trap, UniversalProcPtr, NewFatRoutineDescriptor,
    ProcPtr, m68k, ProcPtr, ppc, ProcInfoType, info)
{
  warning_unimplemented (NULL_STRING);
  *(long *)-1 = -1; /* abort */
  return NULL;
}

P2 (PUBLIC pascal trap, OSErr, SaveMixedModeState, void *, statep,
    uint32, vers)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

P2 (PUBLIC pascal trap, OSErr, RestoreMixedModeState, void *, statep,
    uint32, vers)
{
  warning_unimplemented (NULL_STRING);
  return paramErr;
}

#endif

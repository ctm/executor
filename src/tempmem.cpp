/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_tempmem[] =
"$Id: tempmem.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

#include "MemoryMgr.h"
#include "rsys/mman.h"

using namespace Executor;

/* #define TEMP_MEM_FAIL */
#define paramErr	(-50)

P0 (PUBLIC pascal trap, int32, TempFreeMem)
{
#if defined (TEMP_MEM_FAIL)
  return 0;
#else
  int32 sysfree, applfree, retval;

  ZONE_SAVE_EXCURSION
    (ApplZone,
     {
       applfree = FreeMem ();
     });
  sysfree = FreeMemSys ();
  retval = MAX (applfree, sysfree);
  return retval;
#endif
}

P1 (PUBLIC pascal trap, Size, TempMaxMem,
    Size *, grow)
{
#if defined (TEMP_MEM_FAIL)
  return 0;
#else
  int32 sysfree, applmax, retval;

  ZONE_SAVE_EXCURSION
    (ApplZone,
     {
       applmax = MaxMem (grow);
     });
  sysfree = FreeMemSys ();
  retval = MAX (applmax, sysfree);
  return retval;
#endif
}

P0 (PUBLIC pascal trap, Ptr, TempTopMem)
{
  warning_unimplemented (NULL_STRING);
  return NULL;
}

P2 (PUBLIC pascal trap, Handle, TempNewHandle,
    Size, logical_size, OSErr *, result_code)
{
#if defined (TEMP_MEM_FAIL)
  *result_code = CWC (memFullErr);
  return NULL;
#else
  {
    Handle retval;

    ZONE_SAVE_EXCURSION
      (ApplZone,
       {
	 if (FreeMemSys () >= FreeMem ())
	   TheZone = SysZone;
	 
	 retval = NewHandle (logical_size);
	 if (result_code)
	   *result_code = MemErr;
       });
    return retval;
  }
#endif
}

P2 (PUBLIC pascal trap, void, TempHLock,
    Handle, h, OSErr *, result_code)
{
  HLock (h);
  *result_code = MemErr;
}

P2 (PUBLIC pascal trap, void, TempHUnlock,
    Handle, h, OSErr *, result_code)
{
  HUnlock (h);
  *result_code = MemErr;
}

P2 (PUBLIC pascal trap, void, TempDisposeHandle,
    Handle, h, OSErr *, result_code)
{
  DisposHandle (h);
  *result_code = MemErr;
}

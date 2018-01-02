/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "MemoryMgr.h"
#include "rsys/mman.h"

using namespace Executor;

/* #define TEMP_MEM_FAIL */
#define paramErr (-50)

PUBLIC pascal trap int32_t Executor::C_TempFreeMem()
{
#if defined(TEMP_MEM_FAIL)
    return 0;
#else
    int32_t sysfree, applfree, retval;

    {
        TheZoneGuard guard(ApplZone);

        applfree = FreeMem();
    }
    sysfree = FreeMemSys();
    retval = MAX(applfree, sysfree);
    return retval;
#endif
}

PUBLIC pascal trap Size Executor::C_TempMaxMem(GUEST<Size> * grow_s)
{
#if defined(TEMP_MEM_FAIL)
    return 0;
#else
    int32_t sysfree, applmax, retval;
    Size grow;

    {
        TheZoneGuard guard(ApplZone);

        applmax = MaxMem(&grow);
    }
    if(grow_s)
        *grow_s = CL(grow);
    sysfree = FreeMemSys();
    retval = MAX(applmax, sysfree);
    return retval;
#endif
}

PUBLIC pascal trap Ptr Executor::C_TempTopMem()
{
    warning_unimplemented(NULL_STRING);
    return NULL;
}

PUBLIC pascal trap Handle Executor::C_TempNewHandle(Size logical_size, GUEST<OSErr> * result_code)
{
#if defined(TEMP_MEM_FAIL)
    *result_code = CWC(memFullErr);
    return NULL;
#else
    {
        Handle retval;

        TheZoneGuard guard(ApplZone);
        if(FreeMemSys() >= FreeMem())
            TheZone = SysZone;

        retval = NewHandle(logical_size);
        if(result_code)
            *result_code = MemErr;
        return retval;
    }
#endif
}

PUBLIC pascal trap void Executor::C_TempHLock(Handle h, GUEST<OSErr> * result_code)
{
    HLock(h);
    *result_code = MemErr;
}

PUBLIC pascal trap void Executor::C_TempHUnlock(Handle h, GUEST<OSErr> * result_code)
{
    HUnlock(h);
    *result_code = MemErr;
}

PUBLIC pascal trap void Executor::C_TempDisposeHandle(Handle h, GUEST<OSErr> * result_code)
{
    DisposHandle(h);
    *result_code = MemErr;
}

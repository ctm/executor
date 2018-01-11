/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 *
 * Derived from public domain source code written by Sam Lantinga
 */

/* Separate the memory management routines because they don't compile with
   USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES enabled
*/

#include "rsys/common.h"
#include "MemoryMgr.h"

using namespace Executor;

char *sdl_ReallocHandle(Executor::Handle mem, int len)
{
    ReallocHandle(mem, len);
    if(LM(MemErr) != CWC(noErr))
        return NULL;
    else
        return (char *)STARH(mem);
}

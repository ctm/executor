/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/drive_flags.h"
#include "rsys/error.h"
#include "Gestalt.h"
#include "rsys/gestalt.h"

#include <windows.h>

PUBLIC ULONGINT
physical_memory(void)
{
    MEMORYSTATUS status;
    ULONGINT retval;

    GlobalMemoryStatus(&status);
    retval = status.dwTotalPhys;
    replace_physgestalt_selector(gestaltPhysicalRAMSize, retval);
    return retval;
}

/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_memory[] = "$Id: win_memory.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/drive_flags.h"
#include "rsys/error.h"
#include "Gestalt.h"
#include "rsys/gestalt.h"

#include <windows.h>

PUBLIC ULONGINT
physical_memory (void)
{
  MEMORYSTATUS status;
  ULONGINT retval;

  GlobalMemoryStatus (&status);
  retval = status.dwTotalPhys;
  replace_physgestalt_selector (gestaltPhysicalRAMSize, retval);
  return retval;
}

/* Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_time[] = "$Id: win_time.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"

#include "rsys/time.h"

#include <windows.h>
#include "mmsystem.h"

PRIVATE uint64
system_time_to_micro_time (const SYSTEMTIME *timep)
{
  FILETIME file_time;
  uint64_t retval;

  SystemTimeToFileTime (timep, &file_time);
  retval = ((((uint64_t) file_time.dwHighDateTime) << 32) |
	    (uint32) file_time.dwLowDateTime) / 10;
  return retval;
}

PRIVATE void
gettimeofday (struct timeval *tvp, void *ignored)
{
  SYSTEMTIME system_time, unix_epoch;
  uint64_t now_micro_time;

  unix_epoch.wYear = 1970;
  unix_epoch.wMonth = 1;
  unix_epoch.wDayOfWeek = 4;
  unix_epoch.wDay = 1;
  unix_epoch.wHour = 0;
  unix_epoch.wMinute = 0;
  unix_epoch.wSecond = 0;
  unix_epoch.wMilliseconds = 0;
  GetSystemTime (&system_time);
  now_micro_time  = (system_time_to_micro_time (&system_time) -
		     system_time_to_micro_time (&unix_epoch));
  tvp->tv_usec = now_micro_time % 1000000;
  tvp->tv_sec  = now_micro_time / 1000000;
}

unsigned long
msecs_elapsed (void)
{
  static unsigned long start_msecs = 0;
  unsigned long retval;
  unsigned long msecs;

  msecs = timeGetTime();
  if (start_msecs == 0)
    {
      start_msecs = msecs;
      gettimeofday (&ROMlib_start_time, NULL);
    }

  retval = msecs - start_msecs;
  return retval;
}

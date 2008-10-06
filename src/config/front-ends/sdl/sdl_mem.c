/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 *
 * Derived from public domain source code written by Sam Lantinga
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_sdl_mem[] = "$Id: sdl_mem.c 88 2005-05-25 03:59:37Z ctm $";
#endif

/* Separate the memory management routines because they don't compile with
   USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES enabled
*/
   
#include "rsys/common.h"
#include "MemoryMgr.h"


char *sdl_ReallocHandle(char **mem, int len)
{
  ReallocHandle((Handle)mem, len);
  if ( MemErr != noErr )
    return NULL;
  else
    return (char *) STARH((Handle)mem);
}

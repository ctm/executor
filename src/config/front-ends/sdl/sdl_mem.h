/* 
 * Copyright 1998 by Abacus Research and
 * Development, Inc.  All rights reserved.
 *
 * Derived from public domain source code written by Sam Lantinga
 */

/* Separate the memory management routines because they don't compile with
   USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES enabled
*/
   
extern char *sdl_ReallocHandle(char **mem, int len);

/* Copyright 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_stubs[] =
		"$Id: romlib_stubs.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "rsys/glue.h"

#if !defined(NDEBUG) && !defined(MSDOS)

#undef putchar

A1(PUBLIC, void, ROMlib_printstring, unsigned char *, p)
{
    int i;

    for (i = *p++; --i >= 0 ; putchar(*p++))
	;
    putchar('\n');
}

A1(PUBLIC, void, ROMlib_printostype, OSType, t)
{
    putchar(t >> 24);
    putchar(t >> 16);
    putchar(t >>  8);
    putchar(t >>  0);
    putchar('\n');
}
#endif /* !defined(NDEBUG) && !defined(MSDOS) */

#if !defined (NDEBUG)
A1(PUBLIC, void, ROMlib_hook, LONGINT, hn)
{
	/* don't do anything; This is just here for gdbing */
}
#endif /* !defined (NDEBUG) */

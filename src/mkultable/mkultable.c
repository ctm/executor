/* Copyright 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mkultable[] =
		    "$Id: mkultable.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include <stdio.h>
#include <stdlib.h>

/*
 * This program builds the replacement table used for quick underlining
 * of text.
 */

typedef unsigned int ULONGINT;

int main(int argc, const char * argv[])
{
    ULONGINT i, j;
    ULONGINT out;

    printf("static unsigned char ultable[256] = {\n");
    for (i = 0; i < 256; i++) {
	if (!(i & 7))
	    printf("    ");
	out = (i << 8) ^ (0xFF << 8);
	for (j = 0; j < 8; j++) {
	    if (i & (1 << j)) {
		out &= ~((1 << (j + 9))|(1 << (j + 7)));
	    }
	}
	printf("0x%02x,", out >> 8);
	putchar(((i & 7) != 7) ? ' ' : '\n');
    }
    printf("};\n");
    exit(0);
}

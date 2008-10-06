/* Copyright 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mkseedtables[] =
		    "$Id: mkseedtables.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include <stdio.h>
#include <stdlib.h>

#define EIGHTHPOWEROF(x) ((x)*(x)*(x)*(x)*(x)*(x)*(x)*(x))

typedef int LONGINT;

static void buildbinarytotrinary()
{
    LONGINT i, retval;
    
    printf("PRIVATE INTEGER binarytotrinary[] = {\n");
    for (i = 0; i < EIGHTHPOWEROF(2); i++) {
	retval = 0;
	if (i & (1 << 0))
	    retval += 1;

	if (i & (1 << 1))
	    retval += 3;

	if (i & (1 << 2))
	    retval += 3*3;

	if (i & (1 << 3))
	    retval += 3*3*3;

	if (i & (1 << 4))
	    retval += 3*3*3*3;

	if (i & (1 << 5))
	    retval += 3*3*3*3*3;

	if (i & (1 << 6))
	    retval += 3*3*3*3*3*3;

	if (i & (1 << 7))
	    retval += 3*3*3*3*3*3*3;

	if (!(i & 7))
	    printf("    ");
	printf("0x%04X,", retval);
	putchar((i & 7) == 7 ? '\n' : ' ');
    }
    printf("};\n");
}

static void buildexpandtable()
{
    register LONGINT i, n, bit;
    register LONGINT x, y;
    unsigned char toexpand, seed, retval;
    
    printf("PRIVATE unsigned char expandtable[] = {\n");
    for (i = 0; i < EIGHTHPOWEROF(3); i++) {
	toexpand = seed = retval = 0;
	for (n = 1, bit = 1; n < 6561; n *= 3, bit <<= 1) {
	    switch ((i / n) % 3) {
	    case 0:
		toexpand |= bit;
		break;
	    case 2:
		seed |= bit;
		break;
	    }
	}
	x = ~toexpand & seed;
	for (bit = 1; bit < 256; bit <<= 1) {
	    if (x & bit) {
	    	retval |= bit;
		for (y = bit << 1; y < 256 && !(y&toexpand); y <<= 1)
		    retval |= y;
		for (y = bit >> 1; y && !(y&toexpand); y >>= 1)
		    retval |= y;
	    }
	}
	if (!(i & 7))
	    printf("    ");
	printf("0x%02X,", retval);
	putchar((i & 7) == 7 || i == EIGHTHPOWEROF(3)-1 ? '\n' : ' ');
    }
    printf("};\n");
}

int main()
{
    buildbinarytotrinary();
    buildexpandtable();
    exit(0);
}

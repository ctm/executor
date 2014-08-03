/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mksspairtable[] =
		    "$Id: mksspairtable.c 86 2005-05-25 00:47:12Z ctm $";
#endif

#include <stdio.h>

/*
 * This program builds the table taht BitMapToRegion uses to map a byte
 * of bitmap into start stop pairs.
 * 
 * The first 256 entries are for when the preceeding character did not have
 * the low bit (bit 0) set.  The next 256 wntries are for when the preceeding
 * character did have bit 0 set.  Each entry is an arrray of up to 9 elements
 * that represent all the state changes +1 with 0 as a sentinel.
 */

int main(int argc, const char * argv[])
{
  int lowbit, i, j, value, curbit;

  printf("static unsigned char sspairtable[256*2][9] = {\n");
  for (lowbit = 0; lowbit < 2; ++lowbit)
    {
      for (i = 0; i < 256; i++)
	{
	  printf("    {");
	  value = i;
	  curbit = lowbit << 7;
	  for (j = 0; j < 8; ++j)
	    {
	      if ((value & 0x80) != curbit)
		{
		  printf(" %d,", j+1);
		  curbit ^= 0x80;
		}
	      value <<= 1;
	    }
	  printf("},\n");
	}
    }
  printf("};\n");
  return 0;
}

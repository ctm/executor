/* Copyright 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_mkexpand[] = "$Id: mkexpandtables.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include <stdio.h>

#define EIGHTHPOWEROF(x) ((x) * (x) * (x) * (x) * (x) * (x) * (x) * (x))

#undef PRIVATE
#define PRIVATE static
#define PUBLIC

PRIVATE void buildexpandtable4(void)
{
    LONGINT i, j, retval;

    printf("PRIVATE LONGINT expandtable4[] = {\n");
    for(i = 0; i < EIGHTHPOWEROF(2); i++)
    {
        retval = 0;
        for(j = 0; j < 8; j++)
        {
            if(i & (1 << j))
                retval |= ((LONGINT)0xF << j * 4);
        }
        if(!(i & 3))
            printf("    ");

#if defined(LITTLEENDIAN)
        {
            unsigned char b0, b1, b2, b3;
            b0 = retval;
            b1 = retval >> 8;
            b2 = retval >> 16;
            b3 = retval >> 24;
            retval = ((LONGINT)b0 << 24) | ((LONGINT)b1 << 16) | ((LONGINT)b2 << 8) | b3;
        }
#endif /* defined(LITTLEENDIAN) */

        printf("CLC(0x%08X),", retval); /* NOTE: LONGINT is not a long */
        putchar((i & 3) == 3 ? '\n' : ' ');
    }
    printf("};\n");
}

PRIVATE void buildexpandtable2(void)
{
    LONGINT i, j, retval;

    printf("PRIVATE INTEGER expandtable2[] = {\n");
    for(i = 0; i < EIGHTHPOWEROF(2); i++)
    {
        retval = 0;
        for(j = 0; j < 8; j++)
        {
            if(i & (1 << j))
                retval |= (0x3 << j * 2);
        }
        if(!(i & 7))
            printf("    ");

#if defined(LITTLEENDIAN)
        retval = ((unsigned short)retval >> 8) | ((unsigned char)retval << 8);
#endif /* defined(LITTLEENDIAN) */

        printf("CWC(0x%04X),", (LONGINT)(unsigned short)retval);
        putchar((i & 7) == 7 ? '\n' : ' ');
    }
    printf("};\n");
}

PRIVATE void buildshrinktable2(void)
{
    LONGINT i, j, retval;

    printf("PRIVATE unsigned char shrinktable2[] = {\n");
    for(i = 0; i < EIGHTHPOWEROF(2); i++)
    {
        retval = 0;
        for(j = 0; j < 4; j++)
        {
            if(i & (3 << (j * 2)))
                retval |= 1 << j;
        }
        if(!(i & 7))
            printf("    ");
        printf("0x%X,", retval);
        putchar((i & 7) == 7 ? '\n' : ' ');
    }
    printf("};\n");
}

#if defined(BUILDSHRINKTABLE4)
PRIVATE void buildshrinktable4(void)
{
    LONGINT i, j, retval;

    printf("PRIVATE unsigned char shrinktable4[] = {\n");
    for(i = 0; i < EIGHTHPOWEROF(2); i++)
    {
        retval = 0;
        for(j = 0; j < 2; j++)
        {
            if(i & (0xF << (j * 4)))
                retval |= 1 << j;
        }
        if(!(i & 7))
            printf("    ");
        printf("0x%X,", retval);
        putchar((i & 7) == 7 ? '\n' : ' ');
    }
    printf("};\n");
}
#endif /* BUILDSHRINKTABLE4 */

PUBLIC int main(void)
{
    buildexpandtable4();
    buildexpandtable2();
    buildshrinktable2();
#if defined(BUILDSHRINKTABLE4)
    buildshrinktable4();
#endif /* BUILDSHRINKTABLE4 */
    exit(0);
}

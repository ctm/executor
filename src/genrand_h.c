/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#define STOPBITS 0x1820

main()
{
    int i;
    long l;

    printf("#define STOPBITS 0x%04x\n", STOPBITS);
    for(i = 0; i < 5408; ++i)
        random();
    for(i = 0; i < 100; ++i)
    {
        do
            l = random();
        while(!(l & STOPBITS));
        printf("#define RAND%02d 0x%08x\n", i, l);
    }
    for(i = 0; i < 28; ++i)
    {
        l = random();
        printf("#define STOP%02d 0x%08x\n", i, l & ~STOPBITS);
    }
    exit(0);
}

/*
 * Copyright 1988, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

/*
 *  I admit this file needs some comments...
 *  basically I am trying to use the same code twice relatively
 *  efficiently, trading codesize for speed
 */

static
DECL
{
    STATEDECL
    INTEGER x = 0, x1 = 0, x2 = 0, y = 0;
    ITYPE *ip;
    INTEGER *op;
    int rev;
    INTEGER y0, y1;

    INTEGER ends[3][100];
    INTEGER *oldinset, *oldunset, *newunset;

    rev = -1;
    ends[0][0] = ends[1][0] = 32767;
    y1 = -32768;
    SETIO; /* sets ip, op, y, npairs */
    while(y1 != 32767)
    {
        y0 = y;
        /* you say you want a revolution ... */
        oldinset = (INTEGER *)(ends + (++rev) % 3);
        oldunset = (INTEGER *)(ends + (rev + 1) % 3);
        newunset = (INTEGER *)(ends + (rev + 2) % 3);

        /* merge oldunset and input into newunset */
        if(y != 32767)
        {
            while(NEXTPAIR)
            {
                while(*oldunset < x)
                    *newunset++ = *oldunset++;
                if(x < *oldunset)
                    *newunset++ = x;
                else
                    oldunset++;
            }
            while((*newunset++ = *oldunset++) != 32767)
                ;
        }
        else
            *newunset = 32767; /* letting the pipe drain */

        /* inset the old unset inplace (using oldinset) */
        oldinset = oldunset = (INTEGER *)(ends + (rev + 1) % 3);
        if(dh > 0)
        {
            /* (+ -) (+ -) ... (+ -) */
            while((x1 = oldunset[0]) != 32767)
            {
                if((x1 = x1 + dh) < (x2 = oldunset[1] - dh))
                {
                    *oldinset++ = x1;
                    *oldinset++ = x2;
                }
                oldunset += 2;
            }
            *oldinset = 32767;
        }
        else
        {
            /* - (+ -) (+ -) ... (+ -) + */
            /* remember that dh is negative, so +dh is -ABS(dh) */
            if((x2 = oldunset[0]) != 32767)
            {
                *oldinset++ = x2 + dh;
                oldunset++;
                while((x2 = oldunset[1]) != 32767)
                {
                    if((x1 = oldunset[0] - dh) < (x2 = x2 + dh))
                    {
                        *oldinset++ = x1;
                        *oldinset++ = x2;
                    }
                    oldunset += 2;
                }
                *oldinset++ = oldunset[0] - dh;
            }
            *oldinset = 32767;
        }

        /* merge oldinset and oldunset(now inset) */
        oldinset = (INTEGER *)(ends + (rev) % 3);
        oldunset = (INTEGER *)(ends + (rev + 1) % 3);
        while(*oldinset != 32767 || *oldunset != 32767)
        {
            if(*oldinset < *oldunset)
            {
                INCLXY(*oldinset, y1);
                oldinset++;
            }
            else if(*oldunset < *oldinset)
            {
                INCLXY(*oldunset, y1);
                oldunset++;
            }
            else
            { /* equal, blow them both off */
                oldunset++;
                oldinset++;
            }
        }
        y1 = y0;
    }
    UNSETIO;
}

#undef DECL
#undef STATEDECL
#undef SETIO
#undef UNSETIO
#undef NEXTPAIR
#undef INCLXY
#undef PIPEINCXY

/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in BinaryDecimal.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "rsys/glue.h"
#include "BinaryDecimal.h"

using namespace Executor;

void Executor::NumToString(LONGINT l, StringPtr s)
{
    Byte *p = s + 1;
    LONGINT d;

    if(l == 0)
    {
        s[0] = 1;
        *p = '0';
        return;
    }
    else if((uint32_t)l == 0x80000000L)
    {
        const char *q = "-2147483648";
        s[0] = 11;
        while(*q)
            *p++ = *q++; /* DON'T put the null byte in */
        return;
    }
    else if(l < 0)
    {
        *p++ = '-';
        l = -l;
    }
    d = 1000000000L;
    while(!(*p = l / d)) /* must find something */
        d /= 10;
    l -= d * *p;
    d /= 10;
    *p++ += '0';
    while(d)
    {
        l -= (*p = l / d) * d;
        *p++ += '0';
        d /= 10;
    }
    s[0] = p - (s + 1);
}

void Executor::StringToNum(StringPtr s, LONGINT *lp)
{
    char *p = (char *)s + 1;
    char *ep = p + s[0];
    LONGINT l = 0;
    int sign = 1;

    if(p != ep)
    {
        if(*p == '+')
            p++;
        else if(*p == '-')
        {
            p++;
            sign = -1;
        }
    }
    while(p != ep)
        l = l * 10 + (*p++ & 0xF);
    *lp = sign * l;
}

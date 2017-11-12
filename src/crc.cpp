/* Copyright 1992, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "rsys/options.h"
#include "rsys/crc.h"

/*
 * Algorithm from "Computer Networks" by Andrew S. Tanenbaum pp. 129-132.
 */

#define GEN 0x88108000
#define GEN1 (GEN >> 1)
#define GEN2 (GEN >> 2)
#define GEN3 (GEN >> 3)
#define GEN4 (GEN >> 4)
#define GEN5 (GEN >> 5)
#define GEN6 (GEN >> 6)
#define GEN7 (GEN >> 7)

using namespace Executor;

long polydivide(long l)
{
    if(l & 0x80000000)
        l ^= GEN;
    if(l & 0x40000000)
        l ^= GEN1;
    if(l & 0x20000000)
        l ^= GEN2;
    if(l & 0x10000000)
        l ^= GEN3;
    if(l & 0x08000000)
        l ^= GEN4;
    if(l & 0x04000000)
        l ^= GEN5;
    if(l & 0x02000000)
        l ^= GEN6;
    if(l & 0x01000000)
        l ^= GEN7;
    return l;
}

PUBLIC unsigned short Executor::ROMlib_crcccitt(unsigned char *data, long length)
{
    unsigned long l;

    if(length == 0)
        l = 0;
    else if(length == 1)
        l = data[0];
    else if(length == 2)
        l = (data[0] << 8) | data[1];
    else
        l = (data[0] << 16) | (data[1] << 8) | data[2];
    data += 3;
    length -= 3;
    while(--length >= 0)
    {
        l = (l << 8) | *data++;
        l = polydivide(l);
    }
    l <<= 8;
    l = polydivide(l);
    l <<= 8;
    l = polydivide(l);
    return l >> 8;
}

short Executor::getthecrc(ResType typ, long id)
{
    Handle h;

    h = GetResource(typ, id);
    if(h && *h)
        return ROMlib_crcccitt((unsigned char *)STARH(h), GetHandleSize(h));
    else
        return 0;
}

/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in OSUtil.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "FileMgr.h"
#include "EventMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"
#include "Serial.h"
#include "FontMgr.h"
#include "TimeMgr.h"
#include "MenuMgr.h"
#include "ToolboxEvent.h"

#include "rsys/glue.h"
#include "rsys/notmac.h"
#include "rsys/mman.h"
#include "rsys/blockinterrupts.h"
#include "rsys/trapglue.h"
#include "rsys/pstuff.h"
#include "rsys/osutil.h"
#include "rsys/host.h"
#include "rsys/time.h"
#include "rsys/toolevent.h"
#include "rsys/stdfile.h"
#include "rsys/syncint.h"

using namespace Executor;

/*
 * NOTE: HandToHand is not directly called by the outside world.
 *	 Hence, the handle that hp points to is not swapped.
 */

OSErrRET Executor::HandToHand(Handle *hp)
{
    Handle nh;
    Size s;
    OSErr err;

    if(!*hp)
    {
        warning_unexpected("hp = %p", hp);
        /*-->*/ return nilHandleErr;
    }

    s = GetHandleSize(*hp);
    if((err = MemError()))
        /*-->*/ return (err);

    nh = NewHandle(s);
    if((err = MemError()))
        /*-->*/ return (err);

    BlockMove(STARH(*hp), STARH(nh), s);
    *hp = nh;
    return noErr;
}

/*
 * NOTE: PtrToHand is not directly callable, hence the handle that
 *	 h points to isn't swapped.
 */

OSErrRET Executor::PtrToHand(Ptr p, Handle *h, LONGINT s)
{
    Handle nh;
    OSErr err;

    nh = NewHandle(s);
    if((err = MemError()))
        return (err);
    BlockMove(p, STARH(nh), s);
    if((err = MemError()))
        return (err);
    *h = nh;
    return (noErr);
}

OSErrRET Executor::PtrToXHand(Ptr p, Handle h, LONGINT s)
{
    OSErr err;

    /*
 * DO *NOT* use ReallocHandle here.  It will fail if the handle is locked.
 */
    SetHandleSize(h, s);
    if((err = MemError()))
        return (err);
    BlockMove(p, STARH(h), s);
    if((err = MemError()))
        return (err);
    return (noErr);
}

OSErrRET Executor::HandAndHand(Handle h1, Handle h2)
{
    Size s1 = GetHandleSize(h1), s2 = GetHandleSize(h2);
    OSErr err;

    SetHandleSize(h2, s1 + s2);
    if((err = MemError()))
        return (err);
    BlockMove(STARH(h1), STARH(h2) + s2, s1);
    if((err = MemError()))
        return (err);
    return (noErr);
}

OSErrRET Executor::PtrAndHand(Ptr p, Handle h, LONGINT s1)
{
    Size s2 = GetHandleSize(h);
    OSErr err;

    SetHandleSize(h, s1 + s2);
    if((err = MemError()))
        return (err);
    BlockMove(p, STARH(h) + s2, s1);
    if((err = MemError()))
        return (err);
    return (noErr);
}

/* NOTE:  This set of tables was created by running a test program on
	  the Mac.  It replaces the tables that were hand created from
	  (incomplete) information in Inside Macintosh */

static unsigned char casefold[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    'a', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0xCB, 0x89, 0x80, 0xCC, 0x81, 0x82, 0x83, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x84, 0x97, 0x98, 0x99, 0x85, 0xCD, 0x9C, 0x9D, 0x9E, 0x86,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xAE, 0xAF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCE,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

static unsigned char diacfold[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 0x7F,
    'A', 'A', 'C', 'E', 'N', 'O', 'U', 'a', 'a', 'a', 'a', 'a', 'a', 'c', 'e', 'e',
    'e', 'e', 'i', 'i', 'i', 'i', 'n', 'o', 'o', 'o', 'o', 'o', 'u', 'u', 'u', 'u',
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 'O',
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 'a', 'o', 0xBD, 0xBE, 'o',
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0x20, 'A', 'A', 'O', 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 'y', 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

static unsigned char bothfold[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    'a', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 0x7F,
    'A', 'A', 'C', 'E', 'N', 'O', 'U', 'A', 'A', 'A', 'A', 'A', 'A', 'C', 'E', 'E',
    'E', 'E', 'I', 'I', 'I', 'I', 'N', 'O', 'O', 'O', 'O', 'O', 'U', 'U', 'U', 'U',
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 'O',
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 'A', 'O', 0xBD, 0xAE, 'O',
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 'A', 'A', 'O', 0xCE, 0xCE,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 'Y', 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

static unsigned char order[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x21, 0x22, 0x23, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
    0x47, 0x48, 0x57, 0x59, 0x5D, 0x5F, 0x66, 0x68, 0x6A, 0x6C, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7E,
    0x8C, 0x8E, 0x90, 0x92, 0x95, 0x97, 0x9E, 0xA0, 0xA2, 0xA4, 0xA7, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD,
    0xAE, 0x4E, 0x58, 0x5B, 0x5E, 0x61, 0x67, 0x69, 0x6B, 0x6D, 0x73, 0x75, 0x77, 0x79, 0x7C, 0x83,
    0x8D, 0x8F, 0x91, 0x93, 0x96, 0x99, 0x9F, 0xA1, 0xA3, 0xA5, 0xA8, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
    0x4A, 0x4C, 0x5A, 0x60, 0x7B, 0x7F, 0x98, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x5C, 0x62, 0x63,
    0x64, 0x65, 0x6E, 0x6F, 0x70, 0x71, 0x7D, 0x84, 0x85, 0x86, 0x87, 0x88, 0x9A, 0x9B, 0x9C, 0x9D,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0x94, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0x4D, 0x81,
    0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0x55, 0x8A, 0xCC, 0x56, 0x89,
    0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0x26, 0x27, 0xD4, 0x20, 0x49, 0x4B, 0x80, 0x82, 0x8B,
    0xD5, 0xD6, 0x24, 0x25, 0x2D, 0x2E, 0xD7, 0xD8, 0xA6, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

LONGINT Executor::ROMlib_RelString(unsigned char *s1, unsigned char *s2,
                                   BOOLEAN casesig, BOOLEAN diacsig, LONGINT d0)
{
    INTEGER n1, n2;
    unsigned char *s, *t;
    unsigned char c1, c2;

    s = s1;
    t = s2;
    n1 = d0 >> 16;
    n2 = d0 & 0xFFFF;
    if(n1 > n2)
        n1 = n2;
    if(casesig && diacsig)
    {
        while(n1--)
        {
            c1 = *s++;
            c2 = *t++;
            if((c1 == 0x20 && c2 == 0xCA) || (c2 == 0x20 && c1 == 0xCA))
                /*-->*/ continue;
            if(c1 != c2)
                /*-->*/ return order[c1] < order[c2] ? -1 : 1;
        }
    }
    else if(casesig && !diacsig)
    {
        while(n1--)
        {
            c1 = *s++;
            c2 = *t++;
            if((c1 == 0x20 && c2 == 0xCA) || (c2 == 0x20 && c1 == 0xCA))
                /*-->*/ continue;
            if(diacfold[c1] != diacfold[c2])
                /*-->*/ return order[diacfold[c1]] < order[diacfold[c2]] ? -1 : 1;
        }
    }
    else if(!casesig && diacsig)
    {
        while(n1--)
        {
            c1 = *s++;
            c2 = *t++;
            if((c1 == 0x20 && c2 == 0xCA) || (c2 == 0x20 && c1 == 0xCA))
                /*-->*/ continue;
            if(casefold[c1] != casefold[c2])
                /*-->*/ return order[casefold[c1]] < order[casefold[c2]] ? -1 : 1;
        }
    }
    else
    { /*!casesig && !diacsig*/
        while(n1--)
        {
            c1 = *s++;
            c2 = *t++;
            if((c1 == 0x20 && c2 == 0xCA) || (c2 == 0x20 && c1 == 0xCA))
                /*-->*/ continue;
            if(bothfold[c1] != bothfold[c2])
                /*-->*/ return order[bothfold[c1]] < order[bothfold[c2]] ? -1 : 1;
        }
    }
    n1 = d0 >> 16;
    if(n1 != n2)
        /*-->*/ return n1 < n2 ? -1 : 1;
    return 0;
}

INTEGERRET Executor::RelString(StringPtr s1, StringPtr s2, BOOLEAN casesig,
                               BOOLEAN diacsig)
{
    return ROMlib_RelString((unsigned char *)s1 + 1, (unsigned char *)s2 + 1,
                            casesig, diacsig,
                            (LONGINT)(unsigned char)s1[0] << 16 | (unsigned char)s2[0]);
}

BOOLEANRET Executor::EqualString(StringPtr s1, StringPtr s2, BOOLEAN casesig,
                                 BOOLEAN diacsig)
{
    return RelString(s1, s2, casesig, diacsig) ? false : true;
}

int Executor::ROMlib_strcmp(const Byte *s1, const Byte *s2) /* INTERNAL */
{
    int n1 = U(s1[0]), n2 = U(s2[0]);
    unsigned char *p1 = (unsigned char *)s1 + 1,
                  *p2 = (unsigned char *)s2 + 1, c1, c2;

#if !defined(LETGCCWAIL)
    c1 = 0;
    c2 = 0;
#endif /* LETGCCWAIL */

    while(n1 && n2 && ((c1 = bothfold[U(*p1)]) == (c2 = bothfold[U(*p2)])))
    {
        n1--;
        n2--;
        p1++;
        p2++;
    }
    if(!n1)
        return n2 ? -1 : 0;
    if(!n2)
        return 1;
    else
        return c1 < c2 ? -1 : 1;
}

void Executor::ROMlib_UprString(StringPtr s, BOOLEAN diac, INTEGER len)
{
    unsigned char *p, *ep, *base;

    if(diac)
        base = casefold;
    else
        base = bothfold;

    for(p = s, ep = p + len; p != ep; p++)
        *p = base[U(*p)];
}

void Executor::UprString(StringPtr s, BOOLEAN diac)
{
    ROMlib_UprString(s + 1, diac, (INTEGER)(unsigned char)s[0]);
}

void Executor::GetDateTime(GUEST<ULONGINT> *mactimepointer)
{
    if(mactimepointer)
    {
        unsigned long msecs;
        msecs = msecs_elapsed();
        *mactimepointer = CL(UNIXTIMETOMACTIME(ROMlib_start_time.tv_sec)
                             + (((ROMlib_start_time.tv_usec / 1000) + msecs)
                                / 1000));
    }
}

OSErrRET Executor::ReadDateTime(GUEST<ULONGINT> *secs)
{
    GetDateTime(secs);
    return (noErr);
}

OSErrRET Executor::SetDateTime(ULONGINT mactime)
{
#if !defined(SYSV) && !defined(WIN32)
    struct timeval thetime;

    thetime.tv_sec = MACTIMETOGUNIXTIME(mactime);
    thetime.tv_usec = 0;

    if(settimeofday(&thetime, 0) < 0)
        return (clkWrErr);
    else
        return (noErr);
#else /* defined(SYSV) */
    // FIXME: #warning "SetDateTime not implemented"
    return clkWrErr;
#endif /* defined(SYSV) */
}

/* beginning of code to test */

static unsigned long long
secsinminutes(unsigned long long nminutes)
{
    return nminutes * 60;
}

static unsigned long long
secsinhours(unsigned long long nhours)
{
    return nhours * secsinminutes((LONGINT)60);
}

static unsigned long long
secsindays(unsigned long long ndays)
{
    return ndays * secsinhours((LONGINT)24);
}

static INTEGER daysinmonths[13] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

static INTEGER daysinleapmonths[13] = {
    0,
    31,
    31 + 29,
    31 + 29 + 31,
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
};

static unsigned long long
secsinmonths(ULONGINT nmonths)
{
    return secsindays((ULONGINT)daysinmonths[nmonths]);
}

static unsigned long long
secsinleapmonths(ULONGINT nmonths)
{
    return secsindays((ULONGINT)daysinleapmonths[nmonths]);
}

static unsigned long long
daysinyears(ULONGINT year)
{
    return (ULONGINT)365 * year + year / 4 - year / 100 + year / 400;
}

static BOOLEAN isleap(ULONGINT);
static void setdefaults();
static OSErr openparam(INTEGER *);
static void deriveglobals();

static BOOLEAN isleap(ULONGINT year)
{
    return !(year % 4) && (year % 100 || !(year % 400));
}

/* month January = 1, hour Midnight = 0 */

long long
Executor::ROMlib_long_long_secs(INTEGER year, INTEGER month, INTEGER day, INTEGER hour,
                                INTEGER minute, INTEGER second)
{
    /* #warning Make this work for years < 1904 ... it's probably wrong */
    long long retval;

    if(year < 1904)
        warning_unexpected("year = %d", year);
    retval = secsindays(daysinyears(year - 1) - daysinyears(1903));
    retval += secsinmonths(month - 1);
    retval += secsindays(day - 1);
    retval += secsinhours(hour);
    retval += secsinminutes(minute);
    retval += second;
    if(isleap(year) && month > 2)
        retval += secsindays((ULONGINT)1);
    return retval;
}

#define N_SECS_IN_MINUTE (60L)
#define N_SECS_IN_HOUR (60L * N_SECS_IN_MINUTE)
#define N_SECS_IN_DAY (24L * N_SECS_IN_HOUR)
#define N_SECS_IN_YEAR (365L * N_SECS_IN_DAY)
#define N_SECS_IN_LEAP_YEAR (366L * N_SECS_IN_DAY)

static long long cutoffs[(1LL << 32) / N_SECS_IN_YEAR + 1];

static void
init_cutoffs(void)
{
    int i;
    long long cutoff;

    i = 0;
    cutoff = N_SECS_IN_LEAP_YEAR;

    while(cutoff < (1LL << 32))
    {
        cutoffs[i] = cutoff;
        if(i % 4)
            cutoff += N_SECS_IN_YEAR;
        else
            cutoff += N_SECS_IN_LEAP_YEAR;
        ++i;
    }
    cutoffs[i] = cutoff;
}

void
Executor::date_to_swapped_fields(long long mactime, GUEST<INTEGER> *yearp, GUEST<INTEGER> *monthp,
                                 GUEST<INTEGER> *dayp, GUEST<INTEGER> *hourp, GUEST<INTEGER> *minutep,
                                 GUEST<INTEGER> *secondp, GUEST<INTEGER> *dayofweekp,
                                 GUEST<INTEGER> *dayofyearp, GUEST<INTEGER> *weekofyearp)
{

    /*
 * NOTE:  the function below relies on the start date being 1/1/1904.  It is
 *	  clear that Apple picked that date so there would be no funky non-
 *	  leap years (1900 was one).
 */
    long long secs_left;
    ULONGINT approx_year, days, secs_approx_year;
    INTEGER month, day, hour, minute;
    static BOOLEAN cutoffs_inited_p = false;

    if(!cutoffs_inited_p)
    {
        init_cutoffs();
        cutoffs_inited_p = true;
    }

    if(mactime < 0)
        warning_unexpected(NULL_STRING);

    if(dayofweekp)
        *dayofweekp = CW((mactime / N_SECS_IN_DAY + 5) % 7 + 1);

    if(mactime >= (1LL << 32))
    {
        warning_unexpected("impossible mactime = %lld", mactime);
        approx_year = 0;
    }
    else
    {
        for(approx_year = 0; mactime >= cutoffs[approx_year]; ++approx_year)
            ;
    }

    days = 365 * approx_year + (approx_year + 3) / 4;
    secs_left = mactime - (days * N_SECS_IN_DAY);
    secs_approx_year = isleap(approx_year + 1904) ? N_SECS_IN_LEAP_YEAR : N_SECS_IN_YEAR;
    if(secs_left >= secs_approx_year)
    {
        approx_year++;
        secs_left -= secs_approx_year;
    }
    *yearp = CW(approx_year + 1904);

    if(secs_left < 0)
    {
        warning_unimplemented("This code is flat out wrong");
        secs_left %= N_SECS_IN_LEAP_YEAR;
        if(secs_left < 0)
            secs_left += N_SECS_IN_LEAP_YEAR;
        warning_unexpected("bad date");
    }

    if(dayofyearp)
        *dayofyearp = CW(secs_left / N_SECS_IN_DAY + 1);

    if(weekofyearp)
        *weekofyearp = CW(secs_left / N_SECS_IN_DAY / 7 + 1);

    if(isleap(approx_year + 1904))
    {
        for(month = 0;
            secsinleapmonths(month + 1) <= (unsigned long long)secs_left;
            month++)
            ;
        secs_left -= secsinleapmonths(month);
    }
    else
    {
        for(month = 0;
            secsinmonths(month + 1) <= (unsigned long long)secs_left;
            month++)
            ;
        secs_left -= secsinmonths(month);
    }
    *monthp = CW(month + 1);

    day = secs_left / N_SECS_IN_DAY;
    *dayp = CW(day + 1);
    secs_left -= day * N_SECS_IN_DAY;

    hour = secs_left / N_SECS_IN_HOUR;
    *hourp = CW(hour);
    secs_left -= hour * N_SECS_IN_HOUR;

    minute = secs_left / N_SECS_IN_MINUTE;
    *minutep = CW(minute);
    secs_left -= minute * N_SECS_IN_MINUTE;

    *secondp = CW(secs_left);
}

/* end of code to test */

/*
 * NOTE: not callable from the outside world directly
 */

void Executor::Date2Secs(DateTimeRec *d, ULONGINT *s)
{
    long long l;

    l = ROMlib_long_long_secs(CW(d->year), CW(d->month), CW(d->day),
                              CW(d->hour), CW(d->minute), CW(d->second));
    *s = (ULONGINT)l;
}

void Executor::Secs2Date(ULONGINT mactime, DateTimeRec *d)
{
    date_to_swapped_fields((unsigned long)mactime, &d->year, &d->month,
                           &d->day, &d->hour, &d->minute, &d->second,
                           &d->dayOfWeek, 0, 0);
}

void Executor::GetTime(DateTimeRec *d)
{
    GUEST<ULONGINT> secs;

    GetDateTime(&secs);
    Secs2Date(CL(secs), d);
}

void Executor::SetTime(DateTimeRec *d)
{
    ULONGINT secs;

    Date2Secs(d, &secs);
    SetDateTime(secs);
}

typedef enum { Read,
               Write } ReadWriteType;

#define VALID 0xA8

static void setdefaults()
{
    LM(SPValid) = VALID;
    LM(SPAlarm) = CLC(0);
    LM(SPATalkB) = LM(SPATalkA) = LM(SPConfig) = 0;
    LM(SPPortB) = LM(SPPortA) = CW(baud9600 | stop10 | data8 | noParity);
    LM(SPPrint) = 0;
    LM(SPFont) = CW(geneva - 1);
    LM(SPKbd) = 0x63;
    LM(SPVolCtl) = 3;
    LM(SPClikCaret) = 0x88;
    LM(SPMisc2) = 0x4C;
}

static OSErr openparam(INTEGER *rnp)
{
    static char paramname[] = PARAMRAMMACNAME;
    OSErr err;

    err = FSOpen((StringPtr)paramname, Cx(LM(BootDrive)), rnp);
    if(err == fnfErr)
    {
        if((err = Create((StringPtr)paramname, Cx(LM(BootDrive)), TICK("unix"),
                         TICK("pram")))
           == noErr)
            err = FSOpen((StringPtr)paramname, Cx(LM(BootDrive)), rnp);
    }
    return err;
}

LONGINT Executor::ROMlib_GMTcorrect;

static void deriveglobals()
{
    struct tm *tm, tml, tmg, *tmlater, *tmearlier;
    time_t unixtimenow, gmtimenow, ltimenow;

    unixtimenow = (ROMlib_start_time.tv_sec
                   + ((ROMlib_start_time.tv_usec / 1000 + msecs_elapsed())
                      / 1000));
    tm = localtime(&unixtimenow);
    BlockMove((Ptr)tm, (Ptr)&tml, (Size)sizeof(tml));
    tm = gmtime(&unixtimenow);
    BlockMove((Ptr)tm, (Ptr)&tmg, (Size)sizeof(tmg));
    if(tml.tm_year != tmg.tm_year)
    {
        tmlater = tml.tm_year > tmg.tm_year ? &tml : &tmg;
        tmearlier = tml.tm_year > tmg.tm_year ? &tmg : &tml;
        /* Adjust the day of year */
        tmlater->tm_yday = tmearlier->tm_yday + 1;
    }
    ltimenow = (tml.tm_yday * 24 * 60 * 60) + /* seconds per day */
        (tml.tm_hour * 60 * 60) + /* seconds per hour */
        (tml.tm_min * 60) + tml.tm_sec;
    gmtimenow = (tmg.tm_yday * 24 * 60 * 60) + (tmg.tm_hour * 60 * 60) + (tmg.tm_min * 60) + tmg.tm_sec;
    ROMlib_GMTcorrect = gmtimenow - ltimenow;

    LM(KeyThresh) = CW((short)((LM(SPKbd) >> 4) & 0xF) * 4);
    LM(KeyRepThresh) = CW((short)(LM(SPKbd) & 0xF) * 4);
    LM(MenuFlash) = CW((short)(LM(SPMisc2) >> 2) & 3);
    LM(CaretTime) = CL((short)(LM(SPClikCaret) & 0xF) * 4);
    LM(DoubleTime) = CL((short)(LM(SPClikCaret) & 0xF0) / 4);
}

OSErrRET Executor::InitUtil() /* IMII-380 */
{
    INTEGER rn;
    SysParmType sp;
    LONGINT count;
    OSErr err;
    BOOLEAN badread;

#if !defined(LETGCCWAIL)
    badread = false;
#endif
    if((err = openparam(&rn)) == noErr)
    {
        count = sizeof(sp);
        if(FSRead(rn, &count, (Ptr)&sp) == noErr && sp.valid == VALID && count == sizeof(sp))
        {
            LM(SPValid) = sp.valid;
            LM(SPATalkA) = sp.aTalkA;
            LM(SPATalkB) = sp.aTalkB;
            LM(SPConfig) = sp.config;
            LM(SPPortA) = sp.portA;
            LM(SPPortB) = sp.portB;
            LM(SPAlarm) = sp.alarm;
            LM(SPFont) = sp.font;
            LM(SPKbd) = CW(sp.kbdPrint) >> 8;
            LM(SPPrint) = CW(sp.kbdPrint);
            LM(SPVolCtl) = CW(sp.volClik) >> 8;
            LM(SPClikCaret) = CW(sp.volClik);
            LM(SPMisc2) = CW(sp.misc);
            badread = false;
        }
        else
            badread = true;
    }
    if(err != noErr || badread)
        setdefaults();
    deriveglobals();
    if(err)
        err = prInitErr;
    else
        err = FSClose(rn);
    return err;
}

SysPPtr Executor::GetSysPPtr() /* IMII-381 */
{
    return (SysPPtr)&LM(SPValid);
}

OSErrRET Executor::WriteParam() /* IMII-382 */
{
    INTEGER rn;
    SysParmType sp;
    LONGINT count;
    OSErr err, err2;

    err = prWrErr;
    if(openparam(&rn) == noErr)
    {
        sp.valid = LM(SPValid);
        sp.aTalkA = LM(SPATalkA);
        sp.aTalkB = LM(SPATalkB);
        sp.config = LM(SPConfig);
        sp.portA = LM(SPPortA);
        sp.portB = LM(SPPortB);
        sp.alarm = LM(SPAlarm);
        sp.font = LM(SPFont);
        sp.kbdPrint = CW((short)(LM(SPKbd) << 8) | (LM(SPPrint) & 0xff));
        sp.volClik = CW((short)(LM(SPVolCtl) << 8) | (LM(SPClikCaret) & 0xff));
        sp.misc = CW(LM(SPMisc2));
        count = sizeof(sp);
        if(FSWrite(rn, &count, (Ptr)&sp) == noErr && count == sizeof(sp))
            err = noErr;
        deriveglobals();
        ROMlib_beepedonce = false;
        err2 = FSClose(rn);
        if(err == noErr)
            err = err2;
    }
    return err;
}

void Executor::Enqueue(QElemPtr e, QHdrPtr h)
{
    GUEST<QElemPtr> *qpp;
    virtual_int_state_t block;

    block = block_virtual_ints();
    for(qpp = (GUEST<QElemPtr> *)&h->qHead; *qpp && MR(*qpp) != e;
        qpp = (GUEST<QElemPtr> *)MR(*qpp))
        ;
    if(!*qpp)
    {
        e->evQElem.qLink = 0;
        if(h->qTail)
            MR(h->qTail)->evQElem.qLink = RM(e);
        else
            h->qHead = RM(e);
        h->qTail = RM(e);
    }
    restore_virtual_ints(block);
}

OSErrRET Executor::Dequeue(QElemPtr e, QHdrPtr h)
{
    GUEST<QElemPtr> *qpp;
    OSErr retval;
    virtual_int_state_t block;

    retval = qErr;
    block = block_virtual_ints();
    for(qpp = (GUEST<QElemPtr> *)&h->qHead; *qpp && MR(*qpp) != e;
        qpp = (GUEST<QElemPtr> *)MR(*qpp))
        ;
    if(*qpp)
    {
        *qpp = e->evQElem.qLink;
        if(MR(h->qTail) == e)
            h->qTail = qpp == (GUEST<QElemPtr> *)&h->qHead ? nullptr : RM((QElemPtr)qpp);
        retval = noErr;
    }
    restore_virtual_ints(block);
    return retval;
}

LONGINT Executor::NGetTrapAddress(INTEGER n, INTEGER ttype) /* IMII-384 */
{
    LONGINT retval;

    retval = (LONGINT)((ttype == OSTrap) ? ostraptable[n & (NOSENTRIES - 1)]
                                         : tooltraptable[n & (NTOOLENTRIES - 1)]);
    warning_trace_info("n = 0x%x, ttype = %d, retval = %8x", (uint16_t)n, ttype,
                       (unsigned)retval);
    return retval;
}


static BOOLEAN shouldbeawake;

void Executor::C_ROMlib_wakeup()
{
    shouldbeawake = true;
}

/* argument n is in 1/60ths of a second */

void Executor::Delay(LONGINT n, LONGINT *ftp) /* IMII-384 */
{
    if(n > 0)
    {
        TMTask tm;
        shouldbeawake = false;

        tm.tmAddr = RM((ProcPtr)&ROMlib_wakeup);
        InsTime((QElemPtr)&tm);

        //      fprintf (stderr, "p");
        //      fflush (stderr);
        PrimeTime((QElemPtr)&tm, n * 1000 / 60);
        while(!shouldbeawake)
        {
            syncint_wait();
            check_virtual_interrupt();
        }
        RmvTime((QElemPtr)&tm);
    }

    /* Note:  we're really called from a stub, so no CL() is needed here. */
    if(ftp)
        *ftp = TickCount();
}

void Executor::C_SysBeep(INTEGER i) /* SYSTEM DEPENDENT */
{
#if defined(MAC)
    DebugStr("\004Beep");
#elif defined(X) || defined(MACOSX_) || defined(CYGWIN32)
    host_beep_at_user();
#else
    write(1, "\7", 1);
#endif
}

char Executor::ROMlib_phoneyrom[10] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0x06, 0x7C,
};

void Executor::Environs(GUEST<INTEGER> *rom, GUEST<INTEGER> *machine)
{
    unsigned char rom8, rom9;

    rom8 = ((unsigned char *)ROMlib_phoneyrom)[8];
    rom9 = ((unsigned char *)ROMlib_phoneyrom)[9];

    *rom = CW(rom9);
    *machine = CW(rom8 + 1);
}

INTEGER ROMlib_processor = env68040;

OSErrRET Executor::SysEnvirons(INTEGER vers, SysEnvRecPtr p)
{
    if(vers <= 0)
        /*-->*/ return envBadVers;
    p->environsVersion = CW(vers);
    p->machineType = CWC(53);
    p->systemVersion = LM(SysVersion);

    p->processor = CWC(env68040);
    p->hasFPU = false;
    p->hasColorQD = true;
    p->keyBoardType = CWC(envAExtendKbd);
    p->atDrvrVersNum = 0;
    p->sysVRefNum = LM(BootDrive);

    return vers <= SYSRECVNUM ? noErr : envVersTooBig;
}

void Executor::SetUpA5()
{
}

void Executor::RestoreA5()
{
}

#define TRUE32b 1

void Executor::GetMMUMode(GUEST<INTEGER> *ip) /* IMV-592 */
{
    *ip = CWC(TRUE32b);
}

void Executor::SwapMMUMode(Byte *bp) /* IMV-593 */
{
    *bp = CB(TRUE32b);
}

LONGINT Executor::StripAddress(LONGINT l) /* IMV-593 */
{
    return l;
}

void Executor::C_DebugStr(StringPtr p)
{
    int i;

    fprintf(stderr, "debugstr: ");
    for(i = *p++; i-- > 0; fprintf(stderr, "%c", (LONGINT)*p++))
        ;
    fprintf(stderr, "\n");
}

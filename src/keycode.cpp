/* Copyright 1995 - 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/*
 * This file handles the mechanics of verifying serial number, authorization
 * key pairs and also extracting out the expiration date and numbe of CPUS
 * that is encoded into them.  It doesn't handle the opposite direction, given
 * a serial number, an expiration date and a number of CPUs, a valid
 * authorization key can be created.  That functionality is in a separate
 * file -- one that's not part of Executor -- since we want to make it harder
 * for hackers to reverse engineer their own "getcode" program.  We do make
 * a bunch of intermediate routines available for the getcode program so that
 * we don't have duplicated source code.
 *
 * A serial numbers is a 32-bit quantity that is displayed as a 10-digit
 * unsigned decimal number.  Internally we use the first seven digits as
 * a customer number and the last three digits as encoding of the class of
 * license:
 *
 *         c,ccc,ccc,soo, c = customer number, s = sub-class (e.g. student,
 *		faculty, commercial, etc.), o = operating sytem.
 *
 * This encoding of the serial number is not important to the routines that
 * are in this file.
 *
 * An authorization key decrypts into a serial number, a number of CPUs
 *	supported, an Executor major revision number, and an expiration date.
 *
 * These fields aren't represented as decimal digits, since the end-user never
 * sees them.
 *	
 *	serial number:			32 bits
 *	number of CPUs supported:	16 bits
 *	Executor major revision number:  4 bits
 *	Expiration date:		 8 bits (number of months
 *						 beyond 95-01-01, 0 = none)
 *	Unassigned (must be zero):	 5 bits
 *	_______________________________________
 *	Total:				65 bits
 */

#include "rsys/common.h"
#include "rsys/next.h"

#ifdef MACOSX_
#import "MacAppClass.h"
#import "MacViewClass.h"
#endif

#include "rsys/soundopts.h"
#include "rsys/blockinterrupts.h"
#include "rsys/prefs.h"
#include <sys/types.h>
#include "rsys/version.h"

#include "rsys/keycode.h"

using namespace Executor;

PUBLIC int valid_key_format(const unsigned char *key)
{
    int retval;

    if(strlen((const char *)key) != 13)
        retval = false;
    else
    {
        retval = true;
        while(*key)
        {
            if((*key < '0' || *key > '9') && (*key < 'a' || *key > 'z'))
            {
                retval = false;
                /*-->*/ break;
            }
            ++key;
        }
    }
    return retval;
}

/*
 * Implementation of DES as given in Computer Networks by Andrew S. Tanenbaum
 * 1981 Prentice-Hall, Inc.          -----------------
 *
 * undes particulars figured out by Bill Goldman
 *
 * OK, not quite DES anymore.
 */

PUBLIC ordering_t Executor::InitialTr = {
    0,
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7,
};

PUBLIC ordering_t Executor::FinalTr = {
    0,
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25,
};

PUBLIC ordering_t Executor::swap = {
    0,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
};

PUBLIC ordering_t Executor::KeyTr1 = {
    0,
    57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4,
};

PRIVATE ordering_t KeyTr2 = {
    0,
    14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32,
};

PRIVATE ordering_t etr = {
    0,
    32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9,
    8, 9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1,
};

PRIVATE ordering_t ptr = {
    0,
    16, 7, 20, 21, 29, 12, 28, 17, 1, 15, 23, 26, 5, 18, 31, 10,
    2, 8, 24, 14, 32, 27, 3, 9, 19, 13, 30, 6, 22, 11, 4, 25,
};

PRIVATE unsigned char s[8][65] = {
    {
        0,
        14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
        0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
        4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
        15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13,
    },
    {
        0,
        15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
        3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
        0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
        13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9,
    },
    {
        0,
        10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
        13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
        13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
        1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12,
    },
    {
        0,
        7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
        13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
        10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
        3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14,
    },
    {
        0,
        2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
        14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
        4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
        11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3,
    },
    {
        0,
        12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
        10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
        9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
        4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13,
    },
    {
        0,
        4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
#if 0
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
#else
        13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 2, 12, 15, 8, 6,
#endif
        1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
        6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12,
    },
    {

        0,
        13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
        1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
        7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
        2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11,
    },
};

PRIVATE unsigned char rots[33] = {
    0,
    1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1,
    28, 55, 54, 54, 54, 54, 54, 54, 55, 54, 54, 54, 54, 54, 54, 55
};

PUBLIC void Executor::transpose(block_t data, ordering_t t, int n)
{
    block_t x;
    int i;

    memcpy(x, data, sizeof(x));
    for(i = 1; i <= n; ++i)
        data[i] = x[t[i]];
}

PRIVATE void rotate(block_t key)
{
    int i;
    block_t x;

    memcpy(x, key, sizeof(x));
    for(i = 1; i <= 55; ++i)
        x[i] = x[i + 1];
    x[28] = key[1];
    x[56] = key[29];
    memcpy(key, x, sizeof(block_t));
}

PUBLIC void Executor::f(int i, block_t key, block_t a, block_t x)
{
    block_t e, ikey, y;
    int r, k, j;

    memcpy(e, a, sizeof(e));
    transpose(e, etr, 48);
    for(j = 1; j <= rots[i]; ++j)
        rotate(key);
    memcpy(ikey, key, sizeof(ikey));
    transpose(ikey, KeyTr2, 48);
    for(j = 1; j <= 48; ++j)
        if(e[j] + ikey[j] == 1)
            y[j] = 1;
        else
            y[j] = 0;
    for(k = 1; k <= 8; ++k)
    {
        r = 32 * y[6 * k - 5] + 16 * y[6 * k] + 8 * y[6 * k - 4] + 4 * y[6 * k - 3] + 2 * y[6 * k - 2] + y[6 * k - 1] + 1;
        x[4 * k - 3] = (s[k - 1][r] / 8) & 1;
        x[4 * k - 2] = (s[k - 1][r] / 4) & 1;
        x[4 * k - 1] = (s[k - 1][r] / 2) & 1;
        x[4 * k - 0] = (s[k - 1][r] / 1) & 1;
    }
    transpose(x, ptr, 32);
}

PUBLIC void Executor::undes(block_t ciphertext, block_t keyin, block_t plaintext)
{
    int i, j;
    block_t a, b, x, key;

    memcpy(key, keyin, sizeof(key));
    memcpy(a, ciphertext, sizeof(a));
    transpose(a, InitialTr, 64);
    transpose(key, KeyTr1, 56);

    for(i = 17; i <= 32; ++i)
    {
        memcpy(b, a, sizeof(b));
        for(j = 1; j <= 32; ++j)
            a[j] = b[j + 32];
        f(i, key, a, x);
        for(j = 1; j <= 32; ++j)
            a[j + 32] = b[j] + x[j] == 1;
    }

    transpose(a, swap, 64);
    transpose(a, FinalTr, 64);
    memcpy(plaintext, a, sizeof(block_t));
}

/*
 * 32-bits that make up the serial number.
 * FIXME: need 32 bits here, and need to change user, app and special
 */

PUBLIC int Executor::serial_bits[32] = {
#if defined(RELEASE_PRE_BETA)
    46, 20, 36, 23, 5, 45, 12, 3,
    7, 44, 53, 43, 11, 16, 31, 8,
    41, 27, 55, 24, 64, 1, 2, 9,
    10, 13, 14, 18, 19, 22, 29, 30
#else
    10, 13, 14, 18, 19, 22, 29, 30,
    41, 27, 55, 24, 64, 1, 2, 9,
    7, 44, 53, 43, 11, 16, 31, 8,
    46, 20, 36, 23, 5, 45, 12, 3,
#endif
};

PUBLIC int Executor::n_cpu_bits[16] = {
#if defined(RELEASE_PRE_BETA)
    56, 25, 32, 61, 37, 34, 49, 42,
    15, 28, 6, 26, 17, 4, 21, 60
#else
    15, 28, 6, 26, 17, 4, 21, 60,
    56, 25, 32, 61, 37, 34, 49, 42,
#endif
};

PUBLIC int Executor::revision_number_bits[4] = {
#if defined(RELEASE_PRE_BETA)
    33,
    35,
    38,
    39
#else
    39,
    38,
    35,
    33,
#endif
};

PUBLIC int Executor::expiration_date_bits[8] = {
    40, 47, 48, 50, 51, 52, 54, 57
};

PUBLIC int Executor::unassigned_bits[5] = {
    0, 58, 59, 62, 63, /* NOTE: 0 must be included here, because currently we */
}; /* don't even encode bit 0 in our key */

#if defined(RELEASE_PRE_BETA)
PUBLIC unsigned char Executor::key32[] = "gq2kt8ziwjc5x6vde79f3b4nysphmaur";
#else
PUBLIC unsigned char Executor::key32[] = "5x6vde79f3b4nysphmaurgq2kt8ziwjc";
#endif

PUBLIC block_t Executor::key = {
    0,
    1, 0, 0, 1, 0, 0, 0, 1,
    0, 0, 1, 0, 1, 1, 1, 0,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 1, 0, 0, 1, 0, 1, 0,
    0, 1, 1, 1, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 1, 0,
    0, 1, 0, 1, 1, 1, 0, 1,
    0, 1, 0, 1, 1, 1, 0, 1,
};

PUBLIC void Executor::texttoblock(const unsigned char *theirkey, block_t text)
{
    static int beenhere = 0;
    static unsigned char translate[256];
    unsigned char *textloc;
    int smallnum;
    int i;
    unsigned char *sp;

    if(!beenhere)
    {
        for(i = 0; i < 256; ++i)
            translate[i] = 0;
        for(i = 0, sp = key32; *sp; ++sp, ++i)
            translate[*sp] = i;
    }
    textloc = text;
    for(i = 0; i < 13; ++i)
    {
        smallnum = translate[*theirkey++];
        *textloc++ = (smallnum & (1 << 4)) ? 1 : 0;
        *textloc++ = (smallnum & (1 << 3)) ? 1 : 0;
        *textloc++ = (smallnum & (1 << 2)) ? 1 : 0;
        *textloc++ = (smallnum & (1 << 1)) ? 1 : 0;
        *textloc++ = (smallnum & (1 << 0)) ? 1 : 0;
    }
}

PUBLIC void Executor::bitstonum(unsigned long *result, const int *locs, int numbits,
                                const block_t text)
{
    unsigned long l;

    *result = 0;
    for(l = 1 << (numbits - 1); l; l >>= 1)
        if(text[*locs++])
            *result |= l;
}

PUBLIC int Executor::decode(const unsigned char *theirkey, decoded_info_t *infop)
{
    block_t ciphertext, plaintext;
    unsigned long templong;
    int retval;

    texttoblock(theirkey, ciphertext);
    undes(ciphertext, key, plaintext);
    bitstonum(&templong, unassigned_bits, NELEM(unassigned_bits), plaintext);
    if(templong == 0)
    {
        bitstonum(&templong, serial_bits, NELEM(serial_bits), plaintext);
        infop->serial_number = templong;
        bitstonum(&templong, n_cpu_bits, NELEM(n_cpu_bits), plaintext);
        infop->n_cpu = templong;
        bitstonum(&templong, revision_number_bits,
                  NELEM(revision_number_bits), plaintext);
        infop->major_revision = BASE_REVISION + (templong >> 1);
        infop->updates_p = templong & 1;
        bitstonum(&templong, expiration_date_bits,
                  NELEM(expiration_date_bits), plaintext);
        infop->expires_p = templong != 0;
        infop->last_month = BASE_MONTH + (templong % MONTHS_IN_YEAR);
        infop->last_year = BASE_YEAR + (templong / MONTHS_IN_YEAR);
        retval = true;
    }
    else
        retval = false;
    return retval;
}

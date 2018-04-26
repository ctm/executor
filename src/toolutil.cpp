/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ToolboxUtil.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ToolboxUtil.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/glue.h"
#include "SANE.h"
#include "rsys/float.h"
#include "rsys/floatconv.h"
#include "rsys/string.h"
#include "rsys/mman_private.h"
#include "rsys/resource.h"
#include "rsys/toolutil.h"
#include <string>

using namespace Executor;

static Fixed minvert(Fixed);
static int cmpstrings(char *p, char *ep, char *p1, LONGINT len);

Fixed Executor::C_FixRatio(INTEGER n, INTEGER d)
{
    if(!d)
        return (n < 0 ? 0x80000001 : 0x7fffffff);
    return (((LONGINT)n << 16) / d);
}

Fixed Executor::C_FixMul(Fixed a, Fixed b)
{
    int sign = 1;
    ULONGINT ms1, ms2, ls1, ls2, mm, ml, lm, ll;

    if(a < 0)
    {
        a = -a;
        sign = -sign;
    }
    if(b < 0)
    {
        b = -b;
        sign = -sign;
    }
    ms1 = (ULONGINT)a >> 16;
    ms2 = (ULONGINT)b >> 16;
    ls1 = (ULONGINT)a & 0xffff;
    ls2 = (ULONGINT)b & 0xffff;

    mm = ms1 * ms2;
    if(mm >= (1L << 15))
        return sign == 1 ? 0x7FFFFFFF : 0x80000000;
    ml = ms1 * ls2;
    lm = ls1 * ms2;
    ll = ls1 * ls2;

    mm <<= 16;
    mm += ml + lm + (ll >> 16);

    if(mm & 0x80000000)
        return sign == 1 ? 0x7FFFFFFF : 0x80000000;
    return (sign * mm);
}

INTEGER Executor::C_FixRound(Fixed x)
{
    return ((x >> 16) + ((x & 0x8000L) != 0));
}

StringHandle Executor::C_NewString(StringPtr s)
{
    Handle retval;

    PtrToHand((Ptr)s, &retval, (LONGINT)U(s[0]) + 1);
    return ((StringHandle)retval);
}

void Executor::C_SetString(StringHandle h, StringPtr s)
{
    PtrToXHand((Ptr)s, (Handle)h, (LONGINT)U(s[0]) + 1);
}

Handle Executor::ROMlib_getrestid(ResType rest, INTEGER id) /* INTERNAL */
{
    Handle retval;

    retval = GetResource(rest, id);
    if(retval)
        LoadResource(retval);
    return retval;
}

StringHandle Executor::ROMlib_phoney_name_string;

static StringHandle
get_phoney_name_resource(void)
{
    using namespace std;
    if(!ROMlib_phoney_name_string)
    {
        string name = "";
#if defined(linux) || defined(MACOSX_)
        name = string(getlogin());
        if(!name.length())
            name = string(getenv("LOGNAME"));
        if(!name.length())
            name = "";
#endif
        int len;

        len = name.length();
        ROMlib_phoney_name_string = (StringHandle)NewHandleSys(len + 1);
        if(ROMlib_phoney_name_string)
        {
            HSetRBit((Handle)ROMlib_phoney_name_string);
            str255_from_c_string(STARH(ROMlib_phoney_name_string), name.c_str());
        }
    }
    return ROMlib_phoney_name_string;
}

StringHandle Executor::C_GetString(INTEGER i)
{
    StringHandle retval;

    retval = (StringHandle)ROMlib_getrestid(TICK("STR "), i);
    if(i == -16096 && !retval)
        retval = get_phoney_name_resource();
    return retval;
}

void Executor::GetIndString(StringPtr s, INTEGER sid, INTEGER index)
{
    Handle retval;
    char *p, *ep, *op;

    retval = GetResource(TICK("STR#"), sid);
    LoadResource(retval);
    if(ResError() != noErr || *(INTEGER *)STARH(retval) < index)
    {
        s[0] = 0;
        /*-->*/ return;
    }
    p = (char *)STARH(retval) + 2;
    while(--index)
        p += 1 + U(*p);
    for(ep = p + 1 + U(*p), op = (char *)s; p != ep; *op++ = *p++)
        ;
}

static int cmpstrings(char *p, char *ep, char *p1, LONGINT len)
{
    int retval = true;

    while(retval && p != ep && len--)
        if(*p++ != *p1++)
            retval = false;
    return (retval);
}

/*
 * TODO: find out mroe about what Munger does.  If it's returning an
 * undocumented error return in D0, does that mean that the global memory
 * error location isn't changed?
 */

// FIXME: #warning Munger returns undocumented error return in D0

#define RETURN(x)     \
    do                \
    {                 \
        retval = (x); \
        goto DONE;    \
    } while(false)

// FIXME: #warning We never check for offset greater than HandleSize -- check and fix

// FIXME: #warning We also return -1 even though the Mac may return something different

LONGINT Executor::C_Munger(Handle h, LONGINT off, Ptr p1, LONGINT len1, Ptr p2,
                           LONGINT len2)
{
    char *p, *ep;
    LONGINT hs, tomove;
    LONGINT retval;

    MM_SLAM("entry");
    p = (char *)STARH(h) + off;
    ep = (char *)STARH(h) + (hs = GetHandleSize(h));
    if(p1 && len1)
    {
        while(!cmpstrings(p, ep, (char *)p1, len1))
            p++;
        if(p == ep)
            RETURN(-1);
        if(ep - p < len1)
        {
            if(p != (char *)STARH(h) + off)
                RETURN(-1);
            else
                len1 = ep - p;
        }
        off = p - (char *)STARH(h);
    }
    else if(len1 < 0)
        len1 = hs - off;
    if(!p2 && p1)
        RETURN(off);
    tomove = ep - p - len1;
    if(len1 > len2)
    {
        BlockMoveData((Ptr)p + len1, (Ptr)p + len2, tomove);
        SetHandleSize(h, hs + len2 - len1);
        p = (char *)STARH(h) + off;
    }
    else if(len1 < len2)
    {
        SetHandleSize(h, hs + len2 - len1);
        p = (char *)STARH(h) + off;
        BlockMoveData((Ptr)p + len1, (Ptr)p + len2, tomove);
    }
    while(len2--)
        *p++ = *p2++;
    RETURN(p - (char *)STARH(h));
DONE:
    MM_SLAM("exit");
    EM_D0 &= 0xFFFF0000; /* for now ... we never have an error */
    return retval;
}
#undef RETURN

void Executor::C_PackBits(GUEST<Ptr> *sp, GUEST<Ptr> *dp, INTEGER len)
{
    char *ip, *op, *ep, *erp, *markp, c;

    ip = (char *)MR(*sp);
    op = (char *)MR(*dp);
    ep = ip + len;
    erp = ip + len - 2;
    markp = op++;
    while(ip != ep)
    {
        while(ip != ep
              && (ip >= erp || *ip != *(ip + 1) || *ip != *(ip + 2))
              && op - markp - 2 < 127)
            *op++ = *ip++;
        if(op != markp + 1)
        {
            *markp = op - markp - 2;
        }
        else
            op--;
        if(ip != ep)
        {
            markp = ip;
            c = *ip++;
            while(ip != ep && c == *ip && ip - markp - 1 < 127)
                ip++;
            *op++ = -(ip - markp - 1);
            *op++ = c;
        }
        markp = op++;
    }
    *sp = RM((Ptr)ip);
    *dp = RM((Ptr)op - 1);
}

#define UNPACK_BITS_BODY(out_type)                                           \
    do                                                                       \
    {                                                                        \
        const int8_t *ip;                                                      \
        out_type *op, *ep;                                                   \
                                                                             \
        ip = (const int8_t *)MR(*sp);                                          \
        op = (out_type *)MR(*dp);                                            \
        ep = (out_type *)((int8_t *)op + len);                                 \
                                                                             \
        while(op < ep)                                                       \
        {                                                                    \
            int count = *ip++;                                               \
            if(count < 0)                                                    \
            {                                                                \
                out_type v = *(out_type *)ip;                                \
                ip += sizeof(out_type);                                      \
                for(count = MIN(1 - count, ep - op); count > 0; count--)     \
                    *op++ = v;                                               \
            }                                                                \
            else                                                             \
            {                                                                \
                unsigned bytes = MIN(count + 1, ep - op) * sizeof(out_type); \
                memmove(op, ip, bytes);                                      \
                op += bytes / sizeof *op;                                    \
                ip += bytes;                                                 \
            }                                                                \
        }                                                                    \
                                                                             \
        *sp = RM((Ptr)ip);                                                   \
        *dp = RM((Ptr)op);                                                   \
    } while(false)

void Executor::unpack_int16_t_bits(GUEST<Ptr> *sp, GUEST<Ptr> *dp, INTEGER len)
{
    /* This is used when unpacking 16 bpp PICT bitmaps. */
    UNPACK_BITS_BODY(int16_t);
}

void Executor::C_UnpackBits(GUEST<Ptr> *sp, GUEST<Ptr> *dp, INTEGER len)
{
    UNPACK_BITS_BODY(int8_t);
}

BOOLEAN Executor::C_BitTst(Ptr bp, LONGINT bn)
{
    bp += bn / 8;
    return ((*bp & (1 << ((7 - bn) & 7))) != 0);
}

void Executor::C_BitSet(Ptr bp, LONGINT bn)
{
    bp += bn / 8;
    *bp |= 1 << ((7 - bn) & 7);
}

void Executor::C_BitClr(Ptr bp, LONGINT bn)
{
    bp += bn / 8;
    *bp &= ~(1 << ((7 - bn) & 7));
}

LONGINT Executor::C_BitAnd(LONGINT a, LONGINT b)
{
    return (a & b);
}

LONGINT Executor::C_BitOr(LONGINT a, LONGINT b)
{
    return (a | b);
}

LONGINT Executor::C_BitXor(LONGINT a, LONGINT b)
{
    return (a ^ b);
}

LONGINT Executor::C_BitNot(LONGINT a)
{
    return (~a);
}

LONGINT Executor::C_BitShift(LONGINT a, INTEGER n)
{
    if(n < 0)
    {
        n = -n;
        return ((n %= 64) & 32) ? 0 : (ULONGINT)a >> n;
    }
    else
        return ((n %= 64) & 32) ? 0 : (ULONGINT)a << n;
}

INTEGER Executor::C_HiWord(LONGINT a)
{
    return (a >> 16);
}

INTEGER Executor::C_LoWord(LONGINT a)
{
    return (a & 0xFFFF);
}

void Executor::C_LongMul(LONGINT a, LONGINT b, Int64Bit *c)
{
    int sign;
    ULONGINT ha, hb, la, lb, halb, lahb;
    int carry;

    if((a > 0 && b < 0) || (a < 0 && b > 0))
        sign = -1;
    else
        sign = 1;
    if(a < 0)
        a = -a;
    if(b < 0)
        b = -b;
    ha = a >> 16;
    hb = b >> 16;
    la = a & 0xFFFF;
    lb = b & 0xFFFF;
    halb = ha * lb;
    lahb = la * hb;
    c->hiLong = CL(ha * hb);
    c->loLong = CL(la * lb);
    c->hiLong = CL(CL(c->hiLong) + (halb >> 16));
    c->hiLong = CL(CL(c->hiLong) + (lahb >> 16));
    carry = CL(c->loLong) >> 31;
    c->loLong = CL(CL(c->loLong) + (halb << 16));
    carry += (halb >> 15) & 1;
    c->loLong = CL(CL(c->loLong) + (lahb << 16));
    carry += (lahb >> 15) & 1;
    carry >>= 1;
    c->hiLong = CL(CL(c->hiLong) + (carry));
    if(sign == -1)
    {
        c->hiLong = ~c->hiLong;
        if(c->loLong)
            c->loLong = CL(~CL(c->loLong) + 1);
        else
            c->hiLong = CL(CL(c->hiLong) + 1);
    }
}

PatHandle Executor::C_GetPattern(INTEGER id)
{
    return ((PatHandle)ROMlib_getrestid(TICK("PAT "), id));
}

void Executor::GetIndPattern(Byte *op, INTEGER plistid, INTEGER index)
{
    Handle retval;
    char *p, *ep;

    retval = GetResource(TICK("PAT#"), plistid);
    LoadResource(retval);
    if(ResError() != noErr || *(INTEGER *)STARH(retval) < index)
    {
        return;
    }
    p = (char *)STARH(retval) + 2 + 8 * (index - 1);
    for(ep = p + 8; p != ep; *op++ = *p++)
        ;
}

CursHandle Executor::C_GetCursor(INTEGER id)
{
    return ((CursHandle)ROMlib_getrestid(TICK("CURS"), id));
}

/*
 * Note:  ShieldCursor is found in qd/qCursor.c
 */

PicHandle Executor::C_GetPicture(INTEGER id)
{
    PicHandle retval;

    retval = (PicHandle)ROMlib_getrestid(TICK("PICT"), id);
    return retval;
}

LONGINT Executor::C_DeltaPoint(Point a, Point b)
{
    return (((LONGINT)a.v - b.v) << 16) | (unsigned short)(a.h - b.h);
}

static Fixed minvert(Fixed f) /* wants postive number */
{
    return (((0x80000000 / (ULONGINT)f) << 1) & 0x7FFFFFFF);
}

static Fixed sloptab[46] = {
    static_cast<Fixed>(0x80000001), 0x00394a30, 0x001ca2d7, 0x001314bd, 0x000e4cf5,
    0x000b6e17, 0x000983ad, 0x000824f3, 0x00071d88, 0x00065051,
    0x0005abd9, 0x00052501, 0x00046462, 0x000454db, 0x000402c2,
    0x0003bb68, 0x00037cc7, 0x00034556, 0x000313e3, 0x0002e77a,
    0x0002bf5b, 0x00029ae7, 0x000279af, 0x00025b19, 0x00023efc,
    0x000224fe, 0x00020ce1, 0x0001f66e, 0x0001e177, 0x0001cdd6,
    0x0001bb68, 0x0001aa0e, 0x000199af, 0x00018a35, 0x00017689,
    0x00016dab, 0x0001605b, 0x000153b9, 0x000147aa, 0x00013c22,
    0x00013117, 0x0001267f, 0x00011c51, 0x00011287, 0x00010919,
    0x00010000
};

Fixed Executor::C_SlopeFromAngle(INTEGER a)
{
    int sign, recip;
    Fixed retval; /* cc -a problems */

    if(a < 0)
        a -= 360 * ((a / 360) - 1);
    if(a > 360)
        a %= 360;
    if(a > 180)
        a -= 180;
    if(a < 90)
    {
        sign = -1;
        a = 180 - a;
    }
    else
        sign = 1;
    if(a > 135)
    {
        recip = true;
        a = 270 - a;
    }
    else
        recip = false;
    retval = sloptab[a - 90];
    if(recip)
        retval = minvert(retval);
    if(sign == -1)
        retval = -retval;
    return retval;
}

INTEGER Executor::C_AngleFromSlope(Fixed s)
{
    int neg, inv, retval;
    ULONGINT *ulp;

    if(s < 0)
    {
        s = -s;
        neg = true;
    }
    else
        neg = false;
    if(s < 0x10000)
    {
        if(s)
            s = minvert(s);
        else
            s = 0x7fffffff;
        inv = true;
    }
    else
        inv = false;
    for(ulp = (ULONGINT *)sloptab + 1; *ulp > (ULONGINT)s; ulp++)
        ;
    if(*(ulp - 1) - s < s - *ulp)
        ulp--;
#if !defined(__GNUC__) || defined(FIXEDGCC)
    retval = ulp - (ULONGINT *)sloptab + 90;
#else
    {
        ULONGINT *volatile temp;

        temp = (ULONGINT *)sloptab;
        retval = ulp - temp + 90;
    }
#endif
    if(inv)
        retval = 270 - retval;
    if(neg)
        retval = 180 - retval;
    return (retval ? retval : 180);
}

Fract Executor::C_FracMul(Fract x, Fract y) /* IMIV-64 */
{
    Extended z;

    if((uint32_t)x == 0x80000000 && (uint32_t)y == 0x80000000)
        /*-->*/ return 0; /* this is a deliberate "bug" according to IMIV-63
				    but nevertheless the mac+ returns 0 here */
    z = Frac2X(x) * Frac2X(y);
    return X2Frac(&z);
}

Fixed Executor::C_FixDiv(Fixed x, Fixed y) /* IMIV-64 */
{
    Extended z;

    z = Fix2X(x) / Fix2X(y);
    return X2Fix(&z);
}

Fract Executor::C_FracDiv(Fract x, Fract y) /* IMIV-64 */
{
    Extended z;

    z = Frac2X(x) / Frac2X(y);
    return X2Frac(&z);
}

#define MAXLONG2FIX 0x7FFF
#define MINLONG2FIX -((LONGINT)MAXLONG2FIX + 1)
#define POSOVERFLOW 0x7FFFFFFF
#define NEGOVERFLOW 0x80000000

Fixed Executor::C_Long2Fix(LONGINT x) /* IMIV-65 */
{
    if(x > MAXLONG2FIX)
        /*-->*/ return POSOVERFLOW;
    else if(x < MINLONG2FIX)
        /*-->*/ return NEGOVERFLOW;
    else
        /*-->*/ return x << 16;
}

LONGINT Executor::C_Fix2Long(Fixed x) /* IMIV-65 */
{
    return x >= 0 ? (x + (1L << 15)) >> 16
                  : 0xFFFF0000 | ((x + (1L << 15)) >> 16);
}

#define MAXFIX2FRAC 0x1FFFF
#define MINFIX2FRAC -((LONGINT)MAXFIX2FRAC + 1)

Fract Executor::C_Fix2Frac(Fixed x) /* IMIV-65 */
{
    if(x > MAXFIX2FRAC)
        /*-->*/ return POSOVERFLOW;
    else if(x < MINFIX2FRAC)
        /*-->*/ return NEGOVERFLOW;
    else
        /*-->*/ return x << 14;
}

Fixed Executor::C_Frac2Fix(Fract x) /* IMIV-65 */
{
    return x >= 0 ? (x + (1 << 13)) >> 14
                  : 0xFFFC0000 | ((x + (1 << 13)) >> 14);
}

Extended Executor::Fix2X(Fixed x) /* IMIV-65 */
{
    return (Extended)x / (1L << 16);
}

#define MAXX2FIX MAXLONG2FIX
#define MINX2FIX MINLONG2FIX

Fixed Executor::X2Fix(Extended *xp) /* IMIV-65 */
{
    Extended x;

    if((x = *xp) >= 0)
    {
        if(x > MAXX2FIX)
            /*-->*/ return POSOVERFLOW;
        else
            /*-->*/ return (Fixed)(x * (1L << 16) + .5);
    }
    else
    {
        if(x < MINX2FIX)
            /*-->*/ return NEGOVERFLOW;
        else
            /*-->*/ return (Fixed)(x * (1L << 16) - .5);
    }
}

Extended Executor::Frac2X(Fract x) /* IMIV-65 */
{
    return (Extended)x / (1L << 30);
}

#define BEYONDMAXX2FRAC 2
#define MINX2FRAC -2

Fract Executor::X2Frac(Extended *xp) /* IMIV-65 */
{
    Extended x;

    if((x = *xp) >= 0)
    {
        if(x >= BEYONDMAXX2FRAC)
            /*-->*/ return POSOVERFLOW;
        else
            /*-->*/ return (Fract)(x * (1L << 30) + .5);
    }
    else
    {
        if(x < MINX2FRAC)
            /*-->*/ return NEGOVERFLOW;
        else
            /*-->*/ return (Fract)(x * (1L << 30) - .5);
    }
}


void Executor::R_Fix2X(void *dummyretpc, Fixed x, extended80 *ret) /* INTERNAL */
{
#if 0
    extended96 temp;

    asm("fmovel %1, fp0\n\t"
	"fdivl #0x10000, fp0\n\t"
	"fmovex fp0, %0" : "=m" (temp) : "m" (x) : "fp0");
    X96TO80(temp, *ret);
#else
    ieee_t n = x;
    n /= 0x10000;
    ieee_to_x80(n, ret);
#endif
}

void Executor::R_Frac2X(void *dummyretpc, Fract x,
                        extended80 *ret) /* INTERNAL */
{
#if 0
    extended96 temp;

    asm("fmovel %1, fp0\n\t"
	"fdivl #0x40000000, fp0\n\t"
	"fmovex fp0, %0" : "=m" (temp) : "m" (x) : "fp0");
    X96TO80(temp, *ret);
#else
    ieee_t n = x;
    n /= 0x40000000;
    ieee_to_x80(n, ret);
#endif
}

Fixed Executor::C_R_X2Fix(extended80 *x)
{
#if 0
    extended96 temp;
    Fixed retval;

    X80TO96(*x, temp);
    asm("fmovex %1, fp0\n\t"
	"fmull #0x10000, fp0\n\t"
	"fmovel fp0, %0" : "=g" (retval) : "m" (temp) : "fp0");
    return retval;
#else
    ieee_t n = x80_to_ieee(x);
    n *= 0x10000;
    return (Fixed)n;
#endif
}

Fract Executor::C_R_X2Frac(extended80 *x)
{
#if 0
    extended96 temp;
    Fract retval;

    X80TO96(*x, temp);
    asm("fmovex %1, fp0\n\t"
	"fmull #0x40000000, fp0\n\t"
	"fmovel fp0, %0" : "=g" (retval) : "m" (temp) : "fp0");
    return retval;
#else
    ieee_t n = x80_to_ieee(x);
    n *= 0x40000000;
    return (Fract)n;
#endif
}


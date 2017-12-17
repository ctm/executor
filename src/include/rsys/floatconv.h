#if !defined(_floatconv_h_)
#define _floatconv_h_

/* This file contains static inline routines to convert between ieee_t's and
 * SANE representations for various numbers.  The exact type of conversion
 * should be obvious from the function name.
 *
 * If these functions are not inlined, and ieee_t's are converted to 8 byte
 * doubles, precision will be lost.  Therefore all of these are defined
 * static inline.
 *
 * This file contains:
 *
 *     x80_to_ieee           ieee_to_x80          
 *     f64_to_ieee           ieee_to_f64
 *     f32_to_ieee           ieee_to_f32
 *     comp_to_ieee          ieee_to_comp
 *
 * These routines will only work on machines that support IEEE floating point.
 *
 */

#include <string.h>
#include "rsys/float.h"

#if defined(powerpc)
#include <ieee754.h>
#elif defined(__ppc__)

/*
 * This comes from gcc's ieee754.h,
 * which does not come with gcc with Apple's XCode.
 */

union ieee754_double {
    long double d;
    struct
    {
        unsigned int negative : 1;
        unsigned int exponent : 11;
        /* Together these comprise the mantissa.  */
        unsigned int mantissa0 : 20;
        unsigned int mantissa1 : 32;

    } ieee;
    struct
    {
        unsigned int negative : 1;
        unsigned int exponent : 11;
        unsigned int quiet_nan : 1;
        /* Together these comprise the mantissa.  */
        unsigned int mantissa0 : 19;
        unsigned int mantissa1 : 32;
    } ieee_nan;
};

#define IEEE754_DOUBLE_BIAS 0x3ff /* Added to exponent.  */

#endif

namespace Executor
{
EXTERN_INLINE ieee_t
x80_to_ieee(const x80_t *x) ALWAYS_INLINE;

EXTERN_INLINE ieee_t
x80_to_ieee(const x80_t *x)
{
    ieee_t retval;

#if defined(mc68000)
    volatile m68k_x96_t temp96;

    /* Construct the 96 bit FP memory representation. */
    temp96.fields.sgn_and_exp = x->se.sgn_and_exp;
    temp96.fields.zero = 0;
    temp96.fields.man = x->man.man;

    /* If LONGINT doubles are implemented, just grab it; else use inline asm. */
    if(sizeof(ieee_t) == 12)
        retval = temp96.val;
    else
        asm("fmovex %1,%0"
            : "=f"(retval)
            : "m"(*&temp96));

#elif defined(i386)
    volatile i386_x80_t temp80;

    /* Swap x's byte order. */
    temp80.man_lo = CL_RAW(x->man.hilo.man_lo);
    temp80.man_hi = CL_RAW(x->man.hilo.man_hi);
    temp80.sgn_and_exp = CW_RAW(x->se.sgn_and_exp);

    /* If it's an Inf or NaN, be sure to set the "normalized" bit.
   * 80387 requires this bit to be set (else Inf's become NaN's)
   * but SANE doesn't care about it either way.
   */
    if((temp80.sgn_and_exp & 0x7FFF) == 0x7FFF)
        temp80.man_hi |= 0x80000000;

    /* Load the 10 byte value into our local variable. */
    asm("fldt %1"
        : "=t"(retval)
        : "m"(*&temp80));

#elif defined(__alpha)
    volatile alpha_x64_t temp64;
    uint32 templ;

    temp64.sgn = GET_X80_SGN(x);
    if(GET_X80_EXP(x) == 0 && x->man.man == 0)
    {
        temp64.exp = 0;
        temp64.man_hi = 0;
        temp64.man_lo = 0;
    }
    else
    {
        temp64.exp = GET_X80_EXP(x) - 16383 + 1023;
        templ = CL(x->man.hilo.man_hi);
        temp64.man_hi = templ >> 11;
        temp64.man_lo = ((templ & 0xFFF) << 21) | (CL(x->man.hilo.man_lo) >> 11);
    }
    retval = *(double *)&temp64;

#elif defined(powerpc) || defined(__ppc__)

    union ieee754_double d;
    long long f; /* see SANE 2-18 for names */
    unsigned int e;
    int s;
    int i;

    s = GET_X80_SGN(x);
    e = GET_X80_EXP(x);
    i = x->man.man >> 63;
    f = x->man.man & 0x7fffffffffffffffULL;

    if(e == 32767)
    {
        if(!f) /* infinity */
            d.d = s ? -HUGE_VAL : HUGE_VAL;
        else /* NaN */
        {
            int nan_value;

            nan_value = (f >> 48) & 0xff;
            d.ieee_nan.negative = s;
            d.ieee_nan.exponent = 2047;
            d.ieee_nan.quiet_nan = f >> 63;
            d.ieee_nan.mantissa0 = nan_value << 5;
            d.ieee_nan.mantissa1 = 0;
        }
    }
    else
    {
        if(i) /* Normalized */
        {
            int exp;

            exp = e - 16383;
            if(exp < -1022)
                d.d = 0; /* underflow */
            else if(exp > 1023)
                d.d = s ? -HUGE_VAL : HUGE_VAL;
            else
            {
                d.ieee.negative = s;
                d.ieee.exponent = exp + 1023;
                d.ieee.mantissa0 = f >> 43;
                d.ieee.mantissa1 = f >> 11;
            }
        }
        else
        {
            if(!f) /* zero */
                d.d = 0;
            else /* denormalized */
            {
                int exp;

                exp = e - 16383;
                if(exp != -1022)
                    d.d = 0;
                else
                {
                    d.ieee.negative = s;
                    d.ieee.exponent = 0;
                    d.ieee.mantissa0 = f >> 43;
                    d.ieee.mantissa1 = f >> 11;
                }
            }
        }
    }
// FIXME: #warning using 8 byte to represent x80; precision will be lost
    retval = d.d;
#else
// FIXME: #warning x80_to_ieee not yet supported on this architecture.
#endif

    return retval;
}

EXTERN_INLINE ieee_t
f64_to_ieee(const f64_t *f) ALWAYS_INLINE;

EXTERN_INLINE ieee_t
f64_to_ieee(const f64_t *f)
{
#if defined(QUADALIGN)
#ifdef LITTLEENDIAN
    volatile f64_t temp1 __attribute__((aligned(8)));
    volatile native_f64_t temp2 __attribute__((aligned(8)));
    memcpy(&temp1, f, sizeof temp1);
    temp2.hilo.hi = CL(temp1.hilo.lo);
    temp2.hilo.lo = CL(temp1.hilo.hi);
    return *(double *)&temp2;
#else /* !LITTLEENDIAN */
    double d;
    memcpy(&d, f, sizeof d);
    return d;
#endif /* !LITTLEENDIAN */
#else /* !QUADALIGN */
#ifdef LITTLEENDIAN
    volatile native_f64_t temp;
    temp.hilo.hi = CL_RAW(f->hilo.lo);
    temp.hilo.lo = CL_RAW(f->hilo.hi);
    return temp.d;
#else /* !LITTLEENDIAN */
    return f->d;
#endif /* !LITTLEENDIAN */
#endif /* !QUADALIGN */
}

EXTERN_INLINE ieee_t
f32_to_ieee(const f32_t *f) ALWAYS_INLINE;

EXTERN_INLINE ieee_t
f32_to_ieee(const f32_t *f)
{
#if !defined(LITTLEENDIAN) && !defined(QUADALIGN)
    return *(float *)f;
#else
    volatile native_f32_t temp;
    temp.n = CL_RAW(*(uint32 *)f); /* This byte swaps on LITTLEENDIAN machines. */
    return temp.f;
#endif /* !QUADALIGN */
}

EXTERN_INLINE ieee_t
comp_to_ieee(const comp_t *cp) ALWAYS_INLINE;

EXTERN_INLINE ieee_t
comp_to_ieee(const comp_t *cp)
{
    ieee_t retval;
    native_comp_t c;

#if defined(LITTLEENDIAN)
    c.hilo.hi = CL_RAW(*(uint32 *)(&cp->hilo.hi));
    c.hilo.lo = CL_RAW(*(uint32 *)(&cp->hilo.lo));
#else /* Not LITTLEENDIAN */
#ifndef QUADALIGN
    c = *cp;
#else /* QUADALIGN */
    memcpy(&c, cp, sizeof c);
#endif /* QUADALIGN */
#endif /* Not LITTLEENDIAN */

    if(c.val == 0) /* zero maps to zero. */
        retval = 0.0;
    else if(NATIVE_COMP_IS_NAN(c)) /* NaN maps to special NaN value. */
    {
        /* Magical value for SANE NaN derived from comp NaN. */
        static const unsigned char f64_nan_comp[] __attribute__((aligned(2)))
        = { 0x7F, 0xFF, 0x40, 0x00, 0x00, 0x00, 0x00, 0x14 };
        retval = f64_to_ieee((const f64_t *)&f64_nan_comp);
    }
    else
    {
#if 0
      int is_negative;
      ULONGINT search_val, mask;
      int first_one_bit;
      volatile x80_t temp_x80;

      /* Take absolute value of c, but remember its sign. */
      if (c.val < 0)
	c.val = -c.val, is_negative = 1;
      else
	is_negative = 0;

      /* Find out which 32 bits in which the highest bit set can be found. */
      if (c.hilo.hi != 0)
	search_val = c.hilo.hi, first_one_bit = 0;
      else
	search_val = c.hilo.lo, first_one_bit = 32;

      /* Find the highest bit set. */
      for (mask = 0x80000000; !(search_val & mask);first_one_bit++, mask >>= 1)
	;

      /* Create the value. */
      c.val <<= first_one_bit;
      SET_X80_SGN (&temp_x80, is_negative);
      SET_X80_EXP (&temp_x80, 16383 - first_one_bit);
      temp_x80.man.hilo.man_hi = CL (c.hilo.hi);
      temp_x80.man.hilo.man_lo = CL (c.hilo.lo);
      retval = x80_to_ieee (&temp_x80);
#else
        retval = c.val;
#endif
    }

    return retval;
}

EXTERN_INLINE void
ieee_to_x80(ieee_t n, x80_t *x) ALWAYS_INLINE;

EXTERN_INLINE void
ieee_to_x80(ieee_t n, x80_t *x)
{
#if defined(mc68000)
    volatile m68k_x96_t temp96;

    /* Move the 96 bit representation to memory. */
    if(sizeof(ieee_t) == 12)
        temp96.val = n;
    else
        asm("fmovex %1,%0"
            : "=m"(*&temp96)
            : "f"(n));

    /* Convert the memory representation to an x80_t. */
    x->se.sgn_and_exp = temp96.fields.sgn_and_exp;
    x->man.man = temp96.fields.man;

#elif defined(i386)
    volatile i386_x80_t temp80;

    /* Move the 80 bit representation to memory. */
    asm("fstpt %0\n\t"
        "fwait"
        : "=m"(*&temp80)
        : "t"(n)
        : "st"); /* FIXME - st??? */

    /* Byte swap the memory representation and store it in x. */
    x->se.sgn_and_exp = CW_RAW(temp80.sgn_and_exp);
    x->man.hilo.man_lo = CL_RAW(temp80.man_lo);
    x->man.hilo.man_hi = CL_RAW(temp80.man_hi);
#elif defined(__alpha)
    volatile alpha_x64_t *temp64p;
    uint32 templ;

    temp64p = (alpha_x64_t *)&n;

    SET_X80_SGN(x, temp64p->sgn);
    SET_X80_EXP(x, temp64p->exp + 16383 - 1023);
    templ = temp64p->man_lo;
    x->man.hilo.man_hi = CL(0x80000000 | ((temp64p->man_hi) << 11) | (templ >> 21));
    x->man.hilo.man_lo = CL(templ << 11);
#elif defined(powerpc) || defined(__ppc__)
    union ieee754_double d;

    d.d = n;
    SET_X80_SGN(x, d.ieee.negative);
    if(d.ieee.exponent == 0 && d.ieee.mantissa0 == 0 && d.ieee.mantissa1 == 0)
    {
        SET_X80_EXP(x, 0);
        x->man.man = 0;
    }
    else
    {
// FIXME: #warning Infinities and NaNs not supported here
        SET_X80_EXP(x, d.ieee.exponent + 16383 - IEEE754_DOUBLE_BIAS);
        x->man.man = (((long long)d.ieee.mantissa0 << (32 + 11)) | ((long long)d.ieee.mantissa1 << 11) | ((long long)1 << 63));
    }
#else
// FIXME: #warning ieee_to_x80 not yet supported on this architecture.
#endif
}

EXTERN_INLINE void
ieee_to_f64(ieee_t val, f64_t *dest) ALWAYS_INLINE;

EXTERN_INLINE void
ieee_to_f64(ieee_t val, f64_t *dest)
{
#if !defined(LITTLEENDIAN)
#if !defined(QUADALIGN)
    dest->d = (double)val;
#else /* QUADALIGN */
    volatile f64_t temp;
    temp.d = (double)val;
    *(uint32 *)&dest->hi = CL_RAW(temp.hilo.hi);
    *(uint32 *)&dest->lo = CL_RAW(temp.hilo.lo);
#endif /* QUADALIGN */
#else /* LITTLEENDIAN */
    volatile f64_t temp;
    temp.d = (double)val;
    *(uint32 *)&dest->hilo.lo = CL_RAW(temp.hilo.hi);
    *(uint32 *)&dest->hilo.hi = CL_RAW(temp.hilo.lo);
#endif /* LITTLEENDIAN */
}

EXTERN_INLINE void
ieee_to_f32(ieee_t val, f32_t *dest) ALWAYS_INLINE;

EXTERN_INLINE void
ieee_to_f32(ieee_t val, f32_t *dest)
{
#if !defined(QUADALIGN)
    dest->f = (float)val;
#ifdef LITTLEENDIAN
    dest->n = CL_RAW(dest->n);
#endif
#else /* QUADALIGN */
    volatile f32_t temp;
    temp.f = (float)val;
    *(uint32 *)dest = CL(temp.n);
#endif
}

EXTERN_INLINE void
ieee_to_comp(ieee_t val, comp_t *dest) ALWAYS_INLINE;

EXTERN_INLINE void
ieee_to_comp(ieee_t val, comp_t *dest)
{
    /* FIXME - does not handle range errors! */

    /* Check to see if val is NaN. */
    if(val != val)
    {
        *(uint32 *)&dest->hilo.hi = CL_RAW(0x80000000); /* NaN high 32 bits. */
        *(uint32 *)&dest->hilo.lo = CL_RAW(0x00000000); /* NaN low 32 bits. */
    }
    else /* Not NaN */
    {
#if !defined(LITTLEENDIAN) && !defined(QUADALIGN)
        dest->val = val; /* Let C compiler cast the ieee_t to a LONGINT LONGINT. */
#else
        volatile native_comp_t temp;

        temp.val = val; /* Let C compiler cast the ieee_t to a LONGINT LONGINT. */

        /* Write the value out. */
        *(uint32 *)&dest->hilo.hi = CL_RAW(temp.hilo.hi);
        *(uint32 *)&dest->hilo.lo = CL_RAW(temp.hilo.lo);
#endif
    }
}
}
#endif

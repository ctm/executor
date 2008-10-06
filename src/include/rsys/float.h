#if !defined (_romlib_float_h_)
#define _romlib_float_h_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: float.h 63 2004-12-24 18:19:43Z ctm $
 */

#include <syn68k_public.h>
#include "SANE.h"
#include <float.h>
#include <math.h>

#if !defined (macfpstate)
extern Byte macfpstate[6];
#endif

#define NATIVE_COMP_IS_NAN(c) \
   ((c).val == (signed long long) 0x8000000000000000ULL)

/* Macros for manipulating the sgn and exponent fields of an x80_t. */
#if !defined (LITTLEENDIAN)
# define GET_X80_SGN(x)    ((x)->se.s.sgn)
# define SET_X80_SGN(x, v) ((x)->se.s.sgn = (v))
# define GET_X80_EXP(x)    ((x)->se.s.exp)
# define SET_X80_EXP(x, v) ((x)->se.s.exp = (v))
#else /* LITTLEENDIAN */
#define EXP_BIT_MASK 0x80
# define GET_X80_SGN(x) (((x)->se.sgn_and_exp >> 7) & 1)
# define SET_X80_SGN(x, v) \
  ((x)->se.sgn_and_exp = (((x)->se.sgn_and_exp \
			   & (~ EXP_BIT_MASK)) | ((v) << 7)))
# define GET_X80_EXP(x) (SWAPUW ((x)->se.sgn_and_exp) & 0x7FFF)
# define SET_X80_EXP(x, v) \
  ((x)->se.sgn_and_exp = ((SWAPUW (v) & (~ EXP_BIT_MASK)) \
			 | ((x)->se.sgn_and_exp & EXP_BIT_MASK)))
#endif /* LITTLEENDIAN */


/* This is meant to represent an IEEE FP value with 80 bits of precision.
 * Unfortunately, there may be no C type with that many bits available; in
 * this case, we must compile -O and hope all computations are performed in FP
 * registers so that no precision is lost.
 */
#if 1   /* long double seems to work */
typedef long double ieee_t;
#else
typedef double ieee_t;
#endif


/* 68k 96 bit IEEE FP memory representation. */
#if defined (mc68000)
typedef union {
  struct {
    unsigned short sgn_and_exp PACKED;
    unsigned short zero PACKED;
    unsigned long long man PACKED;
  } fields PACKED;
  ieee_t val PACKED;
} m68k_x96_t;
#endif


/* i386 80 bit IEEE FP memory representation. */
#if defined (i386)
typedef struct {
  ULONGINT man_lo PACKED;        /* Little endian. */
  ULONGINT man_hi PACKED;        /* Little endian. */
  unsigned short sgn_and_exp PACKED;  /* Little endian. */
} i386_x80_t;
#endif

#if defined(__alpha)
typedef struct {
    ULONGINT man_lo PACKED;
    ULONGINT man_hi:20 PACKED;
    ULONGINT exp:11 PACKED;
    ULONGINT sgn:1 PACKED;
} alpha_x64_t;
#endif

/* Define asm constants for various CC bit combinations resulting from
 * compares.
 */
#if !defined (SYN68K)
# define F_GREATER_CODES   "#0x00"
# define F_EQUAL_CODES     "#0x04"
# define F_LESS_CODES      "#0x19"
# define F_UNORDERED_CODES "#0x02"
#endif


/* m68k "double". */
typedef union {
  struct {
    ULONGINT hi;
    ULONGINT lo;
  } hilo;
  double d;
} f64_t;

typedef f64_t native_f64_t;

/* m68k "float". */
typedef union {
  ULONGINT n;
  float f;
} f32_t;

typedef f32_t native_f32_t;


#define OPCODE_MASK 0x3F00

#if defined (NEED_LOGB)
#if defined (HAVE_LOG2)
#define logb(x) log2(x)
#else /* !HAVE_LOG2 */

#if !defined (M_LOG2E)
#define M_LOG2E 1.4426950408889634074
#endif

#define logb ROMlib_logb  /* Gross hack to avoid warnings w/djgpp. */
#define ROMlib_logb(n) (log (n) * M_LOG2E)
#endif /* !HAVE_LOG2 */
#endif /* NEED_LOGB */

#define DECLAREIN()	ieee_t in
#define DECLAREIN2()	ieee_t in1, in2
#define DECLAREIN2OUT()	ieee_t in1, in2, out
#define DECLAREINOUT()	ieee_t in, out

#endif /* Not _romlib_float_h_ */

#if !defined (_romlib_float_h_2_)

#if !defined (EXTERN_INLINE)
#define EXTERN_INLINE extern inline
#endif

#define ALWAYS_INLINE __attribute__((always_inline))

#if defined (NEED_RINT)
#define rint ROMlib_rint  /* Gross hack to avoid warnings w/djgpp. */
EXTERN_INLINE double
rint (double n) ALWAYS_INLINE;

EXTERN_INLINE double
rint (double n)
{
#if defined (i386)
  asm ("frndint" : "=t" (n) : "0" (n));
#else
# error "You're out of luck; you need to write rint!"
#endif
  return n;
}
#endif /* NEED_RINT */

#if defined (NEED_SCALB)
#define scalb ROMlib_scalb  /* Gross hack to avoid warnings w/djgpp. */
EXTERN_INLINE double
scalb (double x, int n) ALWAYS_INLINE ;

EXTERN_INLINE double
scalb (double x, int n) 
{
/* #warning scalb is implemented poorly */
  warning_unimplemented ("poorly implemented");
  return x * pow (2.0, n);
}
#endif /* NEED_SCALB */


#if defined (NEED_LOG1P)
#define log1p ROMlib_log1p  /* Gross hack to avoid warnings w/djgpp. */
EXTERN_INLINE double
log1p (double x) ALWAYS_INLINE;

EXTERN_INLINE double
log1p (double x) 
{
/* #warning log1p is implemented poorly */
  /* This isn't technically correct, since it will fail for small x. */
  return log (x + 1);
}
#endif /* NEED_LOG1P */

#define _romlib_float_h_2_

#endif

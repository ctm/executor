#if !defined(__SANE__)
#define __SANE__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SANE.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
/* Big-endian 64 bit "comp" data type.  Note that this has a NaN value! */
typedef union {
    struct
    {
        ULONGINT hi;
        ULONGINT lo;
    } hilo;
    signed long long val;
} comp_t;

/* Now we have a version of this in "native" byte order. */
#if defined(LITTLEENDIAN)
typedef union {
    struct
    {
        ULONGINT lo;
        ULONGINT hi;
    } hilo;
    signed long long val;
} native_comp_t;
#else /* Not LITTLEENDIAN */
typedef comp_t native_comp_t;
#endif /* Not LITTLEENDIAN */

/* "Packed" IEEE 80 bit FP representation (zero field omitted). */
/* Sign and exponent. */

/* Mantissa. */
typedef struct PACKED
{
    /* Sign and exponent. */
    union {
#if !defined(LITTLEENDIAN)
        struct PACKED
        { /* Here for added efficiency when BIGENDIAN. */
            unsigned short sgn : 1;
            unsigned short exp : 15;
        } s;
#endif
        unsigned short sgn_and_exp;
    } se;

    /* Mantissa. */
    union {
        struct PACKED
        {
            ULONGINT man_hi;
            ULONGINT man_lo;
        } hilo;
        unsigned long long man;
    } man;
} x80_t;

/* For backwards compatibility with old stuff. */
typedef x80_t extended80;
typedef comp_t comp;

/* Begin old stuff: */

/* This only makes sense on the 68k. */
#if defined(mc68000)
struct extended96
{
    GUEST_STRUCT;
    GUEST<INTEGER> exp;
    GUEST<INTEGER> zero;
    GUEST<INTEGER[4]> man;
};
#endif

#define SIGDIGLEN 20

struct Decimal
{
    GUEST_STRUCT;
    GUEST<unsigned char> sgn;
    GUEST<unsigned char> unused_filler;
    GUEST<INTEGER> exp;
    GUEST<unsigned char[SIGDIGLEN]> sig;
};

typedef enum { FloatDecimal,
               FixedDecimal = 256 } toobigdecformstyle_t;

typedef INTEGER DecFormStyle;

#define DECIMALTYPEMASK 0x0100

typedef enum { SNaN = 1,
               QNaN,
               Infinite,
               ZeroNum,
               NormalNum,
               DenormalNum } NumClass;

struct DecForm
{
    GUEST_STRUCT;
    GUEST<DecFormStyle> style;
    GUEST<INTEGER> digits;
};

#define Decstr char *

#define FX_OPERAND 0x0000
#define FD_OPERAND 0x0800
#define FS_OPERAND 0x1000
#define FC_OPERAND 0x3000
#define FI_OPERAND 0x2000
#define FL_OPERAND 0x2800

#define Fx2X_OPCODE 0x000E

#define FI2X (FI_OPERAND + Fx2X_OPCODE)

#define FCMP_RETURN_TYPE void

extern pascal void C_ROMlib_Fcomp2X(comp sp, extended80 *
                                                 dp);
extern pascal void P_ROMlib_Fcomp2X(comp sp, extended80 *
                                                 dp);
extern pascal trap void C_ROMlib_Fsetenv(INTEGER *dp,
                                         INTEGER sel);
extern pascal trap void P_ROMlib_Fsetenv(INTEGER *dp,
                                         INTEGER sel);
extern pascal trap void C_ROMlib_Fgetenv(INTEGER *dp,
                                         INTEGER sel);
extern pascal trap void P_ROMlib_Fgetenv(INTEGER *dp,
                                         INTEGER sel);
extern pascal trap void C_ROMlib_Fprocentry(INTEGER *dp,
                                            INTEGER sel);
extern pascal trap void P_ROMlib_Fprocentry(INTEGER *dp,
                                            INTEGER sel);
extern pascal trap void C_ROMlib_Fprocexit(INTEGER *dp,
                                           INTEGER sel);
extern pascal trap void P_ROMlib_Fprocexit(INTEGER *dp,
                                           INTEGER sel);
extern pascal trap void C_ROMlib_Ftestxcp(INTEGER *dp,
                                          INTEGER sel);
extern pascal trap void P_ROMlib_Ftestxcp(INTEGER *dp,
                                          INTEGER sel);
extern pascal trap void C_ROMlib_FsqrtX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void P_ROMlib_FsqrtX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void C_ROMlib_FscalbX(INTEGER *sp,
                                         extended80 *dp, unsigned short sel);
extern pascal trap void P_ROMlib_FscalbX(INTEGER *sp,
                                         extended80 *dp, unsigned short sel);
extern pascal trap void C_ROMlib_FlogbX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void P_ROMlib_FlogbX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void C_ROMlib_FabsX(extended80 *dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_FabsX(extended80 *dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_FnegX(extended80 *dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_FnegX(extended80 *dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_FrintX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void P_ROMlib_FrintX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void C_ROMlib_FtintX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void P_ROMlib_FtintX(extended80 *dp,
                                        unsigned short sel);
extern pascal trap void C_ROMlib_Fcpysgnx(x80_t *sp,
                                          x80_t *dp, unsigned short sel);
extern pascal trap void P_ROMlib_Fcpysgnx(x80_t *sp,
                                          x80_t *dp, unsigned short sel);
extern pascal trap void C_ROMlib_Faddx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_Faddx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_Fsubx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_Fsubx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_Fmulx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_Fmulx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_Fdivx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void P_ROMlib_Fdivx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_Fremx(void *sp,
                                       extended80 *dp, unsigned short sel);
extern pascal trap void P_ROMlib_Fremx(void *sp,
                                       extended80 *dp, unsigned short sel);
extern pascal trap FCMP_RETURN_TYPE C_ROMlib_Fcmpx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
extern pascal trap void P_ROMlib_Fcmpx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap FCMP_RETURN_TYPE C_ROMlib_FcpXx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
extern pascal trap void P_ROMlib_FcpXx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
extern pascal trap void C_ROMlib_FX2x(extended80 *sp, void *
                                                          dp,
                                      unsigned short sel);
extern pascal trap void P_ROMlib_FX2x(extended80 *sp, void *
                                                          dp,
                                      unsigned short sel);
extern pascal trap void C_ROMlib_Fx2X(void *sp, extended80 *
                                                    dp,
                                      unsigned short sel);
extern pascal trap void P_ROMlib_Fx2X(void *sp, extended80 *
                                                    dp,
                                      unsigned short sel);
extern pascal trap void C_ROMlib_Fx2dec(DecForm *sp2, void *
                                                          sp,
                                        Decimal *dp, unsigned short sel);
extern pascal trap void P_ROMlib_Fx2dec(DecForm *sp2, void *
                                                          sp,
                                        Decimal *dp, unsigned short sel);
extern pascal trap void C_ROMlib_Fdec2x(Decimal *sp, void *
                                                         dp,
                                        unsigned short sel);
extern pascal trap void P_ROMlib_Fdec2x(Decimal *sp, void *
                                                         dp,
                                        unsigned short sel);
extern pascal trap void C_ROMlib_Fclassx(void *sp, INTEGER *
                                                       dp,
                                         unsigned short sel);
extern pascal trap void P_ROMlib_Fclassx(void *sp, INTEGER *
                                                       dp,
                                         unsigned short sel);
extern pascal trap void C_ROMlib_FlnX(extended80 *dp);
extern pascal trap void P_ROMlib_FlnX(extended80 *dp);
extern pascal trap void C_ROMlib_Flog2X(extended80 *dp);
extern pascal trap void P_ROMlib_Flog2X(extended80 *dp);
extern pascal trap void C_ROMlib_Fln1X(extended80 *dp);
extern pascal trap void P_ROMlib_Fln1X(extended80 *dp);
extern pascal trap void C_ROMlib_Flog21X(extended80 *dp);
extern pascal trap void P_ROMlib_Flog21X(extended80 *dp);
extern pascal trap void C_ROMlib_FexpX(extended80 *dp);
extern pascal trap void P_ROMlib_FexpX(extended80 *dp);
extern pascal trap void C_ROMlib_Fexp2X(extended80 *dp);
extern pascal trap void P_ROMlib_Fexp2X(extended80 *dp);
extern pascal trap void C_ROMlib_Fexp1X(extended80 *dp);
extern pascal trap void P_ROMlib_Fexp1X(extended80 *dp);
extern pascal trap void C_ROMlib_Fexp21X(extended80 *dp);
extern pascal trap void P_ROMlib_Fexp21X(extended80 *dp);
extern pascal trap void C_ROMlib_Fxpwri(INTEGER *sp,
                                        extended80 *dp);
extern pascal trap void P_ROMlib_Fxpwri(INTEGER *sp,
                                        extended80 *dp);
extern pascal trap void C_ROMlib_Fxpwry(extended80 *sp,
                                        extended80 *dp);
extern pascal trap void P_ROMlib_Fxpwry(extended80 *sp,
                                        extended80 *dp);
extern pascal trap void C_ROMlib_Fcompound(extended80 *sp2,
                                           extended80 *sp, extended80 *dp);
extern pascal trap void P_ROMlib_Fcompound(extended80 *sp2,
                                           extended80 *sp, extended80 *dp);
extern pascal trap void C_ROMlib_Fannuity(extended80 *sp2,
                                          extended80 *sp, extended80 *dp);
extern pascal trap void P_ROMlib_Fannuity(extended80 *sp2,
                                          extended80 *sp, extended80 *dp);
extern pascal trap void C_ROMlib_FsinX(extended80 *dp);
extern pascal trap void P_ROMlib_FsinX(extended80 *dp);
extern pascal trap void C_ROMlib_FcosX(extended80 *dp);
extern pascal trap void P_ROMlib_FcosX(extended80 *dp);
extern pascal trap void C_ROMlib_FtanX(extended80 *dp);
extern pascal trap void P_ROMlib_FtanX(extended80 *dp);
extern pascal trap void C_ROMlib_FatanX(extended80 *dp);
extern pascal trap void P_ROMlib_FatanX(extended80 *dp);
extern pascal trap void C_ROMlib_FrandX(extended80 *dp);
extern pascal trap void P_ROMlib_FrandX(extended80 *dp);
extern pascal trap void C_ROMlib_Fdec2str(DecForm *sp2,
                                          Decimal *sp, Decstr dp);
extern pascal trap void P_ROMlib_Fdec2str(DecForm *sp2,
                                          Decimal *sp, Decstr dp);
extern pascal trap void C_ROMlib_Fxstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp,
                                           INTEGER lastchar);
extern pascal trap void P_ROMlib_Fxstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp,
                                           INTEGER lastchar);
extern pascal trap void C_ROMlib_Fcstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
extern pascal trap void P_ROMlib_Fcstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
extern pascal trap void C_ROMlib_Fpstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
extern pascal trap void P_ROMlib_Fpstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
extern pascal trap void C_ROMlib_Fsethv(LONGINT *hvp, unsigned short sel);
extern pascal trap void C_ROMlib_Fgethv(LONGINT *hvp, unsigned short sel);

extern pascal trap void C_ROMlib_FnextX(uint8 *x, uint8 *y,
                                        unsigned short sel);
}
#endif

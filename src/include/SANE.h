#if !defined(__SANE__)
#define __SANE__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

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

enum
{
    SIGDIGLEN = 20,
};

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

enum
{
    DECIMALTYPEMASK = 0x0100,
};

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

enum
{
    FX_OPERAND = 0x0000,
    FD_OPERAND = 0x0800,
    FS_OPERAND = 0x1000,
    FC_OPERAND = 0x3000,
    FI_OPERAND = 0x2000,
    FL_OPERAND = 0x2800,
};

enum
{
    Fx2X_OPCODE = 0x000E,
};

enum
{
    FI2X = (FI_OPERAND + Fx2X_OPCODE),
};

#define FCMP_RETURN_TYPE void

extern void C_ROMlib_Fcomp2X(comp sp, extended80 *
                                                 dp);
extern void P_ROMlib_Fcomp2X(comp sp, extended80 *
                                                 dp);
extern void C_ROMlib_Fsetenv(INTEGER *dp,
                                         INTEGER sel);
PASCAL_FUNCTION(ROMlib_Fsetenv);
extern void C_ROMlib_Fgetenv(INTEGER *dp,
                                         INTEGER sel);
PASCAL_FUNCTION(ROMlib_Fgetenv);
extern void C_ROMlib_Fprocentry(INTEGER *dp,
                                            INTEGER sel);
PASCAL_FUNCTION(ROMlib_Fprocentry);
extern void C_ROMlib_Fprocexit(INTEGER *dp,
                                           INTEGER sel);
PASCAL_FUNCTION(ROMlib_Fprocexit);
extern void C_ROMlib_Ftestxcp(INTEGER *dp,
                                          INTEGER sel);
PASCAL_FUNCTION(ROMlib_Ftestxcp);
extern void C_ROMlib_FsqrtX(extended80 *dp,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_FsqrtX);
extern void C_ROMlib_FscalbX(INTEGER *sp,
                                         extended80 *dp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_FscalbX);
extern void C_ROMlib_FlogbX(extended80 *dp,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_FlogbX);
extern void C_ROMlib_FabsX(extended80 *dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_FabsX);
extern void C_ROMlib_FnegX(extended80 *dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_FnegX);
extern void C_ROMlib_FrintX(extended80 *dp,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_FrintX);
extern void C_ROMlib_FtintX(extended80 *dp,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_FtintX);
extern void C_ROMlib_Fcpysgnx(x80_t *sp,
                                          x80_t *dp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fcpysgnx);
extern void C_ROMlib_Faddx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_Faddx);
extern void C_ROMlib_Fsubx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fsubx);
extern void C_ROMlib_Fmulx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fmulx);
extern void C_ROMlib_Fdivx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fdivx);
extern void C_ROMlib_Fremx(void *sp,
                                       extended80 *dp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fremx);
extern FCMP_RETURN_TYPE C_ROMlib_Fcmpx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fcmpx);
extern FCMP_RETURN_TYPE C_ROMlib_FcpXx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
PASCAL_FUNCTION(ROMlib_FcpXx);
extern void C_ROMlib_FX2x(extended80 *sp, void *
                                                          dp,
                                      unsigned short sel);
PASCAL_FUNCTION(ROMlib_FX2x);
extern void C_ROMlib_Fx2X(void *sp, extended80 *
                                                    dp,
                                      unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fx2X);
extern void C_ROMlib_Fx2dec(DecForm *sp2, void *
                                                          sp,
                                        Decimal *dp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fx2dec);
extern void C_ROMlib_Fdec2x(Decimal *sp, void *
                                                         dp,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fdec2x);
extern void C_ROMlib_Fclassx(void *sp, INTEGER *
                                                       dp,
                                         unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fclassx);
extern void C_ROMlib_FlnX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FlnX);

extern void C_ROMlib_Flog2X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Flog2X);

extern void C_ROMlib_Fln1X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fln1X);

extern void C_ROMlib_Flog21X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Flog21X);

extern void C_ROMlib_FexpX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FexpX);

extern void C_ROMlib_Fexp2X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fexp2X);

extern void C_ROMlib_Fexp1X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fexp1X);

extern void C_ROMlib_Fexp21X(extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fexp21X);

extern void C_ROMlib_Fxpwri(INTEGER *sp,
                                        extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fxpwri);
extern void C_ROMlib_Fxpwry(extended80 *sp,
                                        extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fxpwry);
extern void C_ROMlib_Fcompound(extended80 *sp2,
                                           extended80 *sp, extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fcompound);
extern void C_ROMlib_Fannuity(extended80 *sp2,
                                          extended80 *sp, extended80 *dp);
PASCAL_FUNCTION(ROMlib_Fannuity);
extern void C_ROMlib_FsinX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FsinX);

extern void C_ROMlib_FcosX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FcosX);

extern void C_ROMlib_FtanX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FtanX);

extern void C_ROMlib_FatanX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FatanX);

extern void C_ROMlib_FrandX(extended80 *dp);
PASCAL_FUNCTION(ROMlib_FrandX);

extern void C_ROMlib_Fdec2str(DecForm *sp2,
                                          Decimal *sp, Decstr dp);
PASCAL_FUNCTION(ROMlib_Fdec2str);
extern void C_ROMlib_Fxstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp,
                                           INTEGER lastchar);
PASCAL_FUNCTION(ROMlib_Fxstr2dec);
extern void C_ROMlib_Fcstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
PASCAL_FUNCTION(ROMlib_Fcstr2dec);
extern void C_ROMlib_Fpstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
PASCAL_FUNCTION(ROMlib_Fpstr2dec);
extern void C_ROMlib_Fsethv(LONGINT *hvp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fsethv);
extern void C_ROMlib_Fgethv(LONGINT *hvp, unsigned short sel);
PASCAL_FUNCTION(ROMlib_Fgethv);

extern void C_ROMlib_FnextX(uint8 *x, uint8 *y,
                                        unsigned short sel);
PASCAL_FUNCTION(ROMlib_FnextX);
}
#endif

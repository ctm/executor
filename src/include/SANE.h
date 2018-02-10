#if !defined(__SANE__)
#define __SANE__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 */

#include "ExMacTypes.h"
#include "rsys/macros.h"

#define MODULE_NAME SANE
#include <rsys/api-module.h>

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

DISPATCHER_TRAP(Pack4, 0xA9EB, StackWLookahead<0xFF>);
DISPATCHER_TRAP(Pack5, 0xA9EC, StackWMasked<0xFF>);
DISPATCHER_TRAP(Pack7, 0xA9EE, StackW);

extern void C_ROMlib_Fcomp2X(comp sp, extended80 *
                                                 dp);
extern void C_ROMlib_Fsetenv(INTEGER *dp,
                                         INTEGER sel);
PASCAL_SUBTRAP(ROMlib_Fsetenv, 0xA9EB, 0x01, Pack4);
extern void C_ROMlib_Fgetenv(INTEGER *dp,
                                         INTEGER sel);
PASCAL_SUBTRAP(ROMlib_Fgetenv, 0xA9EB, 0x03, Pack4);
extern void C_ROMlib_Fprocentry(INTEGER *dp,
                                            INTEGER sel);
PASCAL_SUBTRAP(ROMlib_Fprocentry, 0xA9EB, 0x17, Pack4);
extern void C_ROMlib_Fprocexit(INTEGER *dp,
                                           INTEGER sel);
PASCAL_SUBTRAP(ROMlib_Fprocexit, 0xA9EB, 0x19, Pack4);
extern void C_ROMlib_Ftestxcp(INTEGER *dp,
                                          INTEGER sel);
PASCAL_SUBTRAP(ROMlib_Ftestxcp, 0xA9EB, 0x1B, Pack4);
extern void C_ROMlib_FsqrtX(extended80 *dp,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FsqrtX, 0xA9EB, 0x12, Pack4);
extern void C_ROMlib_FscalbX(INTEGER *sp,
                                         extended80 *dp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FscalbX, 0xA9EB, 0x18, Pack4);
extern void C_ROMlib_FlogbX(extended80 *dp,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FlogbX, 0xA9EB, 0x1A, Pack4);
extern void C_ROMlib_FabsX(extended80 *dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FabsX, 0xA9EB, 0x0F, Pack4);
extern void C_ROMlib_FnegX(extended80 *dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FnegX, 0xA9EB, 0x0D, Pack4);
extern void C_ROMlib_FrintX(extended80 *dp,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FrintX, 0xA9EB, 0x14, Pack4);
extern void C_ROMlib_FtintX(extended80 *dp,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FtintX, 0xA9EB, 0x16, Pack4);
extern void C_ROMlib_Fcpysgnx(x80_t *sp,
                                          x80_t *dp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fcpysgnx, 0xA9EB, 0x11, Pack4);
extern void C_ROMlib_Faddx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Faddx, 0xA9EB, 0x00, Pack4);
extern void C_ROMlib_Fsubx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fsubx, 0xA9EB, 0x02, Pack4);
extern void C_ROMlib_Fmulx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fmulx, 0xA9EB, 0x04, Pack4);
extern void C_ROMlib_Fdivx(void *sp, extended80 *
                                                     dp,
                                       unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fdivx, 0xA9EB, 0x06, Pack4);
extern void C_ROMlib_Fremx(void *sp,
                                       extended80 *dp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fremx, 0xA9EB, 0x0C, Pack4);
extern FCMP_RETURN_TYPE C_ROMlib_Fcmpx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fcmpx, 0xA9EB, 0x08, Pack4);
extern FCMP_RETURN_TYPE C_ROMlib_FcpXx(void *sp, extended80 *
                                                                 dp,
                                                   unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FcpXx, 0xA9EB, 0x0A, Pack4);
extern void C_ROMlib_FX2x(extended80 *sp, void *
                                                          dp,
                                      unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FX2x, 0xA9EB, 0x10, Pack4);
extern void C_ROMlib_Fx2X(void *sp, extended80 *
                                                    dp,
                                      unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fx2X, 0xA9EB, 0x0E, Pack4);
extern void C_ROMlib_Fx2dec(DecForm *sp2, void *
                                                          sp,
                                        Decimal *dp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fx2dec, 0xA9EB, 0x0B, Pack4);
extern void C_ROMlib_Fdec2x(Decimal *sp, void *
                                                         dp,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fdec2x, 0xA9EB, 0x09, Pack4);
extern void C_ROMlib_Fclassx(void *sp, INTEGER *
                                                       dp,
                                         unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fclassx, 0xA9EB, 0x1C, Pack4);
extern void C_ROMlib_FlnX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FlnX, 0xA9EC, 0x00, Pack5);

extern void C_ROMlib_Flog2X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Flog2X, 0xA9EC, 0x02, Pack5);

extern void C_ROMlib_Fln1X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fln1X, 0xA9EC, 0x04, Pack5);

extern void C_ROMlib_Flog21X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Flog21X, 0xA9EC, 0x06, Pack5);

extern void C_ROMlib_FexpX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FexpX, 0xA9EC, 0x08, Pack5);

extern void C_ROMlib_Fexp2X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fexp2X, 0xA9EC, 0x0A, Pack5);

extern void C_ROMlib_Fexp1X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fexp1X, 0xA9EC, 0x0C, Pack5);

extern void C_ROMlib_Fexp21X(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fexp21X, 0xA9EC, 0x0E, Pack5);

extern void C_ROMlib_Fxpwri(INTEGER *sp,
                                        extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fxpwri, 0xA9EC, 0x10, Pack5);
extern void C_ROMlib_Fxpwry(extended80 *sp,
                                        extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fxpwry, 0xA9EC, 0x12, Pack5);
extern void C_ROMlib_Fcompound(extended80 *sp2,
                                           extended80 *sp, extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fcompound, 0xA9EC, 0x14, Pack5);
extern void C_ROMlib_Fannuity(extended80 *sp2,
                                          extended80 *sp, extended80 *dp);
PASCAL_SUBTRAP(ROMlib_Fannuity, 0xA9EC, 0x16, Pack5);
extern void C_ROMlib_FsinX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FsinX, 0xA9EC, 0x18, Pack5);

extern void C_ROMlib_FcosX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FcosX, 0xA9EC, 0x1A, Pack5);

extern void C_ROMlib_FtanX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FtanX, 0xA9EC, 0x1C, Pack5);

extern void C_ROMlib_FatanX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FatanX, 0xA9EC, 0x1E, Pack5);

extern void C_ROMlib_FrandX(extended80 *dp);
PASCAL_SUBTRAP(ROMlib_FrandX, 0xA9EC, 0x20, Pack5);

extern void C_ROMlib_Fdec2str(DecForm *sp2,
                                          Decimal *sp, Decstr dp);
PASCAL_SUBTRAP(ROMlib_Fdec2str, 0xA9EE, 0x03, Pack7);
extern void C_ROMlib_Fxstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp,
                                           INTEGER lastchar);
extern void C_ROMlib_Fcstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
PASCAL_SUBTRAP(ROMlib_Fcstr2dec, 0xA9EE, 0x04, Pack7);
extern void C_ROMlib_Fpstr2dec(Decstr sp2,
                                           INTEGER *sp, Decimal *dp2, Byte *dp);
PASCAL_SUBTRAP(ROMlib_Fpstr2dec, 0xA9EE, 0x02, Pack7);
extern void C_ROMlib_Fsethv(LONGINT *hvp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fsethv, 0xA9EB, 0x05, Pack4);
extern void C_ROMlib_Fgethv(LONGINT *hvp, unsigned short sel);
PASCAL_SUBTRAP(ROMlib_Fgethv, 0xA9EB, 0x07, Pack4);

extern void C_ROMlib_FnextX(uint8_t *x, uint8_t *y,
                                        unsigned short sel);
PASCAL_SUBTRAP(ROMlib_FnextX, 0xA9EB, 0x13, Pack4);
}
#endif

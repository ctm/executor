/* Copyright 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in SANE.h (DO NOT DELETE THIS LINE) */

/* 
 * TODO:figure out what the two highest bits of sel do.
 *      they are mentioned in think C's <sane.h>, and have come
 *      up in FreeHand 1.0
 */

#include "rsys/common.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include "SANE.h"
#include "rsys/float.h"
#include "rsys/floatconv.h"
#include "rsys/float_fcw.h"

using namespace Executor;

#if !defined(CYGWIN32)

#define IEEE_T_FORMAT "%.20Lf"
#define IEEE_T_PRINT_CAST ieee_t

#else

#define IEEE_T_FORMAT "%.20f"
#define IEEE_T_PRINT_CAST double

#endif

#if defined(mc68000)

/* This table maps bits in the 68881 FPCR to bits in the SANE environment
 * word.  This bit number of the fpcr bit is the index into the table.
 */
static unsigned short m68k_fpcr_envword_table[16] = {
    0, 0, 0, 0, /* These bits are unused.                   */
    (1 << 13), (1 << 14), /* Rounding mode; map wrong, fixed below.   */
    (1 << 6), (1 << 5), /* Rounding precision.                      */
    (1 << 4), (1 << 4), /* Inexact exceptions enabled.              */
    (1 << 3), /* Divide by zero exceptions enabled.       */
    (1 << 1), /* Underflow exceptions enabled.            */
    (1 << 2), /* Overflow excpetions enabled.             */
    (1 << 0), (1 << 0), (1 << 0) /* Invalid exceptions enabled (inexact map!)*/
};

/* This table maps bits in the 68881 FPSR to bits in the SANE environment
 * word.  This bit number of the fpsr bit is the index into the table.
 * Only the low 8 bits of the FPSR map to anything in the SANE word.
 */
static unsigned short m68k_fpsr_envword_table[8] = {
    0, 0, 0, /* These bits are unused.              */
    (1 << 12), /* Inexact exception raised.           */
    (1 << 11), /* Divide by zero exception raised.    */
    (1 << 9), /* Underflow exception raised.         */
    (1 << 10), /* Overflow exception raised.          */
    (1 << 8) /* Invalid operation exception raised. */
};

/* Bits we affect.  Preserve all of the other ones. */
#define FPCR_BITS_WE_USE 0x0000FFF0
#define FPSR_BITS_WE_USE 0x000000F8

/* We need these to compensate for differing bit patterns for the four
 * rounding modes.
 */
#define ROUNDING_MODE_HIGH_BIT (1 << 14)
#define ROUNDING_MODE_LOW_BIT (1 << 13)

#elif defined(i386)
static unsigned short i387_fcw_envword_table[] = {
    (1 << 0), /* Invalid operation MASKED (backwards from SANE). */
    0, /* Denormal exception masked; no analog in SANE.   */
    (1 << 3), /* Division by zero exception MASKED.              */
    (1 << 2), /* Overflow exception MASKED.                      */
    (1 << 1), /* Underflow exception MASKED.                     */
    (1 << 4), /* Precision -> inexact MASKED.                    */
    0, 0, /* Not used.                                       */
    (1 << 5), (1 << 6), /* Precision control; mapping wrong!  Fixed below. */
    (1 << 14), (1 << 13) /* Rounding control.                               */
};

static unsigned short i387_fsw_envword_table[] = {
    (1 << 8), /* Invalid operation exception signaled.        */
    0, /* Denormal exception signaled; no SANE analog. */
    (1 << 11), /* Division by zero exception signaled.         */
    (1 << 10), /* Overflow exception signaled.                 */
    (1 << 9), /* Underflow exception signaled.                */
    (1 << 12) /* Inexact operation exception signaled.        */
};

/* Bits we affect.  Preserve all of the other ones. */
#define FCW_BITS_WE_USE 0x0F3D
#define FSW_BITS_WE_USE 0x003D

/* We need these to compensate for differing bit patterns for the four
 * precision modes.
 */
#define PRECISION_MODE_HIGH_BIT (1 << 6)
#define PRECISION_MODE_LOW_BIT (1 << 5)

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

typedef struct
{
    unsigned short fcw PACKED;
    unsigned short unused1 PACKED;
    unsigned short fsw PACKED;
    unsigned short unused2 PACKED;
    unsigned short tag_word PACKED;
    unsigned short unused3 PACKED;
    ULONGINT fip PACKED;
    unsigned short fcs PACKED;
    unsigned short opcode PACKED;
    ULONGINT foo PACKED;
    unsigned short fos PACKED;
    unsigned short unused4 PACKED;
} i387_env_t;

PUBLIC uint32_t
ROMlib_get_fcw_fsw(void)
{
    i387_env_t i387_env;

    asm("fnstenv %0 ; fwait"
        : "=m"(i387_env));
    return (i387_env.fcw << 16) | i387_env.fsw;
}

PUBLIC void
ROMlib_set_fcw_fsw(uint32_t fcwfsw)
{
    i387_env_t i387_env;

    asm("fnstenv %0 ; fwait"
        : "=m"(i387_env));
    i387_env.fcw = fcwfsw >> 16;
    i387_env.fsw = fcwfsw;
    asm("fldenv %0"
        :
        : "m"(i387_env));
}

PUBLIC void
ROMlib_compare_fcw_fsw(uint32_t fcwfsw, const char *func, int line)
{
    uint16_t old_fcw;
    uint16_t old_fsw;
    i387_env_t i387_env;

    old_fcw = fcwfsw >> 16;
    old_fsw = fcwfsw;
    asm("fnstenv %0 ; fwait"
        : "=m"(i387_env));
    if(old_fcw != i387_env.fcw)
        fprintf(stderr, "%s(%d) old fcw = %x, new fcw = %x\n", func, line,
                old_fcw, i387_env.fcw);
    if(old_fsw != i387_env.fsw)
        fprintf(stderr, "%s(%d) old fsw = %x, new fsw = %x\n", func, line,
                old_fsw, i387_env.fsw);
}

#endif /* defined (i386) */

/* We keep track of SANE's idea of what halts are enabled separately
 * from what halts are actually enabled in hardware (e.g. 80387).
 * This way Fgetenv will return the same information as Fsetenv,
 * without forcing us to be able to generate a real floating point
 * exception.  Eventually this needs to be changed so we *do* get
 * a real exception, and signal appropriately.
 */
static uint8 halts_enabled;

void Executor::C_ROMlib_Fsetenv(INTEGER *dp, INTEGER sel)
{
    unsigned short env;
    int i;

    /* Note which halts are enabled, but don't enable them in hardware.
   * This is a hack so we don't get real floating point exceptions
   * and should go away when we properly handle SANE exception
   * handling.
   */

    env = CW_RAW(*dp);
    halts_enabled = env & 0x1F;
    env &= ~0x1F;

#if defined(mc68000)
    {
        ULONGINT fpcr, fpsr;

        /* Rearrange rounding mode bits into 68881 format. */
        if(env & ROUNDING_MODE_LOW_BIT)
            env ^= ROUNDING_MODE_HIGH_BIT;

        /* Grab the fpcr + fpsr and zero out the bits we might set.  We'll
     * preserve all of the bits we never affect.
     */
        asm("fmovel fpcr, %0 ; fmovel fpsr, %0"
            : "=g"(fpcr), "=g"(fpsr));
        fpcr &= ~FPCR_BITS_WE_USE;
        fpsr &= ~FPSR_BITS_WE_USE;

        /* Compute bits for the fpcr. */
        for(i = NELEM(m68k_fpcr_envword_table) - 1; i >= 0; i--)
            if(env & m68k_fpcr_envword_table[i])
                fpcr |= (1 << i);

        /* Compute bits for the fpsr. */
        for(i = NELEM(m68k_fpsr_envword_table) - 1; i >= 0; i--)
            if(env & m68k_fpsr_envword_table[i])
                fpsr |= (1 << i);

        /* Save the computed fpcr and fpsr. */
        asm("fmovel %0, fpcr ; fmovel %1, fpsr"
            :
            : "g"(fpcr), "g"(fpsr));
    }
#elif defined(i386)
    {
        unsigned short fcw, fsw;
        i387_env_t i387_env;

        /* Grab the fcw + fsw and zero out the bits we might set.  We'll
     * preserve all of the bits we never affect.  We use fnstenv so we
     * can save the fsw at the end.
     */
        asm("fnstenv %0 ; fwait"
            : "=m"(i387_env));
        fcw = i387_env.fcw;
        fsw = i387_env.fsw;

        /* Mask out bit we are going to set up. */
        fcw &= ~FCW_BITS_WE_USE;
        fsw &= ~FSW_BITS_WE_USE;

#if 0
    /* No longer necessary, since we prevent them from enabling
     * *any* halts as far as the 80387 is concerned.
     */
    
    /* 80387 semantics are that exceptions are "sticky" and stay around
     * until explicitly cleared.  If that exception becomes unmasked,
     * and is still pending, we'll get an exception then.  Panorama 3
   * does an intentional division by zero with division-by-zero
   * exceptions masked.  Then later it reenables division-by-zero
   * faults.  On the 80387, this causes the pending division by zero
   * exception to be triggered by the next floating point opcode!
   * As a workaround, we see which exceptions we're about to unmask
   * and make sure none of them are pending.  Otherwise, we'll
   * end up getting an exception.  A better approach would be
   * to keep track of this information separately elsewhere,
   * so it doesn't trip up the 80387 but the information isn't
   * lost.  Oh well; this SANE implementation would fail a rigorous
   * SANE test suite in many other ways, and this should be good
   * enough for almost all programs.
   */
    if (env & ((env & 0x1F) << 8))
      {
	warning_unimplemented ("About to clear pending SANE exception "
			       "information to avoid an 80387 exception.  "
			       "Technically we should not be discarding "
			       "this information, and should allow "
			       "unmasked exceptions to be set here.");
	env &= ~((env & 0x1F) << 8);
      }
#endif

        /* Compensate for different bit patterns for precision control.
     * [SANE -> 80387]: 00 -> 11, 01 -> 10, 10 -> 00, 11 -> 01.  */
        if(!(env & PRECISION_MODE_HIGH_BIT))
            env ^= (PRECISION_MODE_HIGH_BIT | PRECISION_MODE_LOW_BIT);
        else
            env &= ~PRECISION_MODE_HIGH_BIT;

        /* Compute bits for the fcw. */
        fcw |= 0x3D; /* Start out with exceptions masked, so ^ 1 turns them "on". */
        for(i = NELEM(i387_fcw_envword_table) - 1; i >= 0; i--)
            if(env & i387_fcw_envword_table[i])
                fcw ^= (1 << i);

        /* Compute bits for the fsw. */
        for(i = NELEM(i387_fsw_envword_table) - 1; i >= 0; i--)
            if(env & i387_fsw_envword_table[i])
                fsw |= (1 << i);

        /* Save the computed fcw and fsw. */
        i387_env.fcw = fcw;
        i387_env.fsw = fsw;
        asm("fldenv %0"
            :
            : "m"(i387_env));
    }
#elif !defined(WIN32)
    // FIXME: #warning ROMlib_Fsetenv not implemented!
    signal(SIGFPE, SIG_IGN);
#else
    // FIXME: #warning ROMlib_Fsetenv not implemented!
    gui_abort();
#endif

    warning_floating_point("setenv(0x%04X)", (unsigned)(uint16_t)CW_RAW(*dp));
}

void Executor::C_ROMlib_Fgetenv(INTEGER *dp, INTEGER sel)
{
    unsigned short env;
    int i;

#if defined(mc68000)
    ULONGINT fpcr, fpsr;

    /* Default to all zero bits. */
    env = 0;

    /* Grab FP status and control registers. */
    asm("fmovel fpcr,%0 ; fmovel fpsr,%1"
        : "=g"(fpcr), "=g"(fpsr));

    /* Grab bits from the fpcr. */
    for(i = NELEM(m68k_fpcr_envword_table) - 1; i >= 0; i--)
        if(fpcr & (1 << i))
            env |= m68k_fpcr_envword_table[i];

    /* Grab bits from the fpsr. */
    for(i = NELEM(m68k_fpsr_envword_table) - 1; i >= 0; i--)
        if(fpsr & (1 << i))
            env |= m68k_fpsr_envword_table[i];

    /* Correct for different 2 bit patterns for rounding modes between
   * SANE and the 68881.
   */
    if(env & ROUNDING_MODE_LOW_BIT)
        env ^= ROUNDING_MODE_HIGH_BIT;

#elif defined(i386)
    /* Volatile because targets of fnstcw and fnstsw have to be in memory. */
    volatile unsigned short mem_fcw, mem_fsw;
    unsigned short fcw, fsw;

    /* Default to all zero bits but with all exceptions enabled.  We do it this
   * way because the meanings of the 387 exception enable bits are reversed
   * from those of SANE.  By using XOR and starting these bits out as 1,
   * we set these back to zero when exceptions are "masked", which is what
   * SANE expects.
   */
    env = 0x1F; /* Low 5 bits set. */

    /* Fetch the floating control word and the floating status word. */
    asm("fnstcw %0 ; fnstsw %1 ; fwait"
        : "=m"(mem_fcw), "=m"(mem_fsw));
    fcw = mem_fcw;
    fsw = mem_fsw;

    /* Grab bits from the fcw. */
    for(i = NELEM(i387_fcw_envword_table) - 1; i >= 0; i--)
        if(fcw & (1 << i))
            env ^= i387_fcw_envword_table[i];

    /* Grab bits from the fsw. */
    for(i = NELEM(i387_fsw_envword_table) - 1; i >= 0; i--)
        if(fsw & (1 << i))
            env ^= i387_fsw_envword_table[i];

    /* Compensate for different bit patterns for precision control.
   * [80387 -> SANE]: 00 -> 10, 01 -> 11, 10 -> 01, 11 -> 00.
   */
    if(env & PRECISION_MODE_HIGH_BIT)
        env ^= (PRECISION_MODE_HIGH_BIT | PRECISION_MODE_LOW_BIT);
    else
        env |= PRECISION_MODE_HIGH_BIT;

#elif defined(__alpha) || defined(powerpc) || defined(__ppc__)
    // FIXME: #warning ROMlib_Fgetenv not properly implemented!
    env = 0;
#else
    // FIXME: #warning ROMlib_Fgetenv not implemented!
    gui_abort();
#endif

    /* We now ignore what exceptions the hardware thinks is enabled,
   * and just faithfully report whatever the program set up in the
   * last setenv.
   */
    env = (env & ~0x1F) | halts_enabled;

    /* Return the computed environment word. */
    *(unsigned short *)dp = CW_RAW(env);
    warning_floating_point("Returning 0x%04X", (unsigned)env);
}

void Executor::C_ROMlib_Fprocentry(INTEGER *dp, INTEGER sel)
{
    static INTEGER default_environment = 0; /* Always == 0. */

    warning_floating_point(NULL_STRING);
    /* Save the old environment. */
    C_ROMlib_Fgetenv(dp, 0);

    /* Set up the default environment. */
    C_ROMlib_Fsetenv(&default_environment, 0);
}

void Executor::C_ROMlib_Fprocexit(INTEGER *dp, INTEGER sel)
{
    INTEGER swapped_old_env;
    INTEGER swapped_new_env;

    /* FIXME - the behavior of this function is not likely to be correct for
   * the cases where exceptions are lurking, waiting to be signaled.
   */

    warning_floating_point(NULL_STRING);
#define EXCEPTION_BITS_MASK 0x1F00

    /* Get the old environment. */
    C_ROMlib_Fgetenv(&swapped_old_env, 0);

    /* Compute the new environment (which is swapped). */
    swapped_new_env = ((*dp & ~CWC_RAW(EXCEPTION_BITS_MASK))
                       | (swapped_old_env & CWC_RAW(EXCEPTION_BITS_MASK)));

    /* Set up the new environment. */
    C_ROMlib_Fsetenv(&swapped_new_env, 0);
}

void Executor::C_ROMlib_Ftestxcp(INTEGER *dp, INTEGER sel)
{
    INTEGER env;

    warning_floating_point(NULL_STRING);
    /* Fetch the current environment. */
    C_ROMlib_Fgetenv(&env, 0);
    env = CW_RAW(env);

    /* Clear dp's high byte. */
    *dp &= CWC_RAW(0x00FF);

    /* See if any of the specified exception bits are set. */
    if((env >> 8) & ((unsigned char *)dp)[1])
        *dp |= CWC_RAW(0x100); /* Return 1 in the high byte. */
}

void Executor::C_ROMlib_FsqrtX(x80_t *dp, unsigned short sel)
{
    DECLAREINOUT();
    /* FIXME - may lose precision! */
    ieee_to_x80(out = sqrt(in = x80_to_ieee(dp)), dp);

    warning_floating_point("sqrt(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "",
                           (IEEE_T_PRINT_CAST)in, (IEEE_T_PRINT_CAST)out);
}

void Executor::C_ROMlib_FscalbX(INTEGER *sp, x80_t *dp, unsigned short sel)
{
    int scale;
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    scale = CW_RAW(*(short *)sp);
    ieee_to_x80(out = scalb(in = x80_to_ieee(dp), scale), dp);

    warning_floating_point("scalb(" IEEE_T_FORMAT ", %d) == " IEEE_T_FORMAT "",
                           (IEEE_T_PRINT_CAST)in, scale, (IEEE_T_PRINT_CAST)out);
}

void Executor::C_ROMlib_FlogbX(x80_t *dp, unsigned short sel)
{
    DECLAREINOUT();
    /* FIXME - may lose precision! */
    ieee_to_x80(out = logb(in = x80_to_ieee(dp)), dp);
    warning_floating_point("logb(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "", (IEEE_T_PRINT_CAST)in, (IEEE_T_PRINT_CAST)out);
}

void Executor::C_ROMlib_FabsX(x80_t *dp, unsigned short sel)
{
    warning_floating_point(NULL_STRING);
    SET_X80_SGN(dp, 0);
}

void Executor::C_ROMlib_FnegX(x80_t *dp, unsigned short sel)
{
#if ERROR_SUPPORTED_P(ERROR_FLOATING_POINT)
    ieee_t before;
    before = x80_to_ieee(dp);
#endif
    SET_X80_SGN(dp, !(GET_X80_SGN(dp)));
    warning_floating_point("neg(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "",
                           (IEEE_T_PRINT_CAST)before, (IEEE_T_PRINT_CAST)x80_to_ieee(dp));
}

void Executor::C_ROMlib_Fcpysgnx(x80_t *sp, x80_t *dp, unsigned short sel)
{
    warning_floating_point(NULL_STRING);
    /* This looks strange because we are copying dst's sign to src, but
   * that is what the Apple Numerics Manual specifies (p. 150), and is
   * what the old float4.c used to do.
   */
    SET_X80_SGN(sp, GET_X80_SGN(dp));
}

void Executor::C_ROMlib_FrintX(x80_t *dp, unsigned short sel)
{
    DECLAREINOUT();
    /* FIXME - may lose precision! */
    ieee_to_x80(out = rint(in = x80_to_ieee(dp)), dp);
    warning_floating_point("rint(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "", (IEEE_T_PRINT_CAST)in, (IEEE_T_PRINT_CAST)out);
}

void Executor::C_ROMlib_FtintX(x80_t *dp, unsigned short sel)
{
    DECLAREINOUT();
    ieee_t n = x80_to_ieee(dp);

    in = n;
    /* FIXME - may lose precision! */
    if(n >= 0)
        ieee_to_x80(out = floor(n), dp);
    else
        ieee_to_x80(out = -floor(-n), dp);

    warning_floating_point("tint(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "", (IEEE_T_PRINT_CAST)in, (IEEE_T_PRINT_CAST)out);
}

/* The SIMPLE_OP macro extracts the commonality from several functions. */
#define NOT_BINARY_OP 0
#define IS_BINARY_OP 1
#define SIMPLE_OP(name, op, is_binary_op)                                                                   \
    do                                                                                                      \
    {                                                                                                       \
        DECLAREIN2OUT();                                                                                    \
        ieee_t dest;                                                                                        \
                                                                                                            \
        if(is_binary_op)                                                                                    \
        {                                                                                                   \
            dest = x80_to_ieee(dp);                                                                         \
            in2 = dest;                                                                                     \
        }                                                                                                   \
        else                                                                                                \
            in2 = 0.0;                                                                                      \
        /* Add the appropriate value. */                                                                    \
        switch(sel & OPCODE_MASK)                                                                           \
        {                                                                                                   \
            case FX_OPERAND:                                                                                \
                dest op(in1 = x80_to_ieee((const x80_t *)sp));                                              \
                break;                                                                                      \
            case FD_OPERAND:                                                                                \
                dest op(in1 = f64_to_ieee((const f64_t *)sp));                                              \
                break;                                                                                      \
            case FS_OPERAND:                                                                                \
                dest op(in1 = f32_to_ieee((const f32_t *)sp));                                              \
                break;                                                                                      \
            case FI_OPERAND:                                                                                \
                dest op(in1 = CW_RAW(*(short *)sp));                                                        \
                break;                                                                                      \
            case FL_OPERAND:                                                                                \
                dest op(in1 = CL_RAW(*(long *)(sp)));                                                       \
                break;                                                                                      \
            case FC_OPERAND:                                                                                \
                dest op(in1 = comp_to_ieee((const comp_t *)sp));                                            \
                break;                                                                                      \
            default:                                                                                        \
                dest = 0.0; /* Here to avoid compiler warnings. */                                          \
                gui_abort();                                                                                \
        }                                                                                                   \
                                                                                                            \
        /* Write out the value. */                                                                          \
        ieee_to_x80(dest, dp);                                                                              \
        out = dest;                                                                                         \
        if(is_binary_op)                                                                                    \
            warning_floating_point(name "(" IEEE_T_FORMAT ", " IEEE_T_FORMAT ") == " IEEE_T_FORMAT "",      \
                                   (IEEE_T_PRINT_CAST)in2, (IEEE_T_PRINT_CAST)in1, (IEEE_T_PRINT_CAST)out); \
        else                                                                                                \
            warning_floating_point(name "(" IEEE_T_FORMAT ") == " IEEE_T_FORMAT "",                         \
                                   (IEEE_T_PRINT_CAST)in1, (IEEE_T_PRINT_CAST)out);                         \
    } while(0)

void Executor::C_ROMlib_Faddx(void *sp, x80_t *dp, unsigned short sel)
{
    SIMPLE_OP("add", +=, IS_BINARY_OP);
}

void Executor::C_ROMlib_Fsubx(void *sp, x80_t *dp, unsigned short sel)
{
    SIMPLE_OP("sub", -=, IS_BINARY_OP);
}

void Executor::C_ROMlib_Fmulx(void *sp, x80_t *dp, unsigned short sel)
{
    SIMPLE_OP("mul", *=, IS_BINARY_OP);
}

PRIVATE LONGINT halt_vec;

void Executor::C_ROMlib_Fsethv(LONGINT *hvp, unsigned short sel)
{
    halt_vec = *hvp;
}

void Executor::C_ROMlib_Fgethv(LONGINT *hvp, unsigned short sel)
{
    *hvp = halt_vec;
}

void Executor::C_ROMlib_Fdivx(void *sp, x80_t *dp, unsigned short sel)
{
    SIMPLE_OP("div", /=, IS_BINARY_OP);
}

void Executor::C_ROMlib_Fx2X(void *sp, x80_t *dp, unsigned short sel)
{
    /* NOTE: this is way slower than it needs to be for the case where we are
   * assigning one x80 to another.
   */
    SIMPLE_OP("assign", =, NOT_BINARY_OP);
}

void Executor::C_ROMlib_FX2x(x80_t *sp, void *dp, unsigned short sel)
{
    DECLAREIN();
    ieee_t val = x80_to_ieee(sp);

    /* FIXME - should trigger exceptions if val cannot be accurately
   * represented in the new type.
   */

    in = val;
    warning_floating_point("X2x(" IEEE_T_FORMAT ")", (IEEE_T_PRINT_CAST)in);

    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
#if defined(QUADALIGN)
            memmove(dp, sp, sizeof *sp); /* struct assign may assume alignment. */
#else
            *(x80_t *)dp = *sp;
#endif
            break;
        case FD_OPERAND:
            ieee_to_f64(val, (f64_t *)dp);
            break;
        case FS_OPERAND:
            ieee_to_f32(val, (f32_t *)dp);
            break;
        case FI_OPERAND:
            *(short *)dp = CW_RAW((signed short)rint(val));
            break;
        case FL_OPERAND:
            *(long *)dp = CL_RAW((LONGINT)rint(val));
            break;
        case FC_OPERAND:
            ieee_to_comp(val, (comp_t *)dp);
            break;
        default:
            gui_abort();
    }
}

void Executor::C_ROMlib_Fremx(void *sp, x80_t *dp, unsigned short sel)
{
    DECLAREIN2OUT();
    ieee_t n1, n2, ratio;
    int n;

    /* Fetch first argument. */
    n1 = x80_to_ieee(dp);
    in1 = n1;

    /* Fetch second argument. */
    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            n2 = x80_to_ieee((const x80_t *)sp);
            break;
        case FD_OPERAND:
            n2 = f64_to_ieee((const f64_t *)sp);
            break;
        case FS_OPERAND:
            n2 = f32_to_ieee((const f32_t *)sp);
            break;
        case FI_OPERAND:
            n2 = CW_RAW(*(short *)sp);
            break;
        case FL_OPERAND:
            n2 = CL_RAW(*(long *)sp);
            break;
        case FC_OPERAND:
            n2 = comp_to_ieee((const comp_t *)sp);
            break;
        default:
            n2 = 0.0; /* Here to avoid compiler warnings. */
            gui_abort();
    }
    in2 = n2;

/* m68k specific stuff commented out so we can test one consistent version. */
#if 0 && defined(mc68000)
  asm ("fremx %3,%0 ; fmovel fpsr, %1" : "=f" (n1), "=g" (n)
       : "0" (n1), "f" (n2));
  n = (n >> 16);
#else /* Not mc68000 */
    /* FIXME - may lose precision! */
    n = ratio = rint(n1 / n2);
    if(n < 0)
        n = -n;
    n1 = /* drem (n1, n2); */ n1 - ratio * n2; /* djgpp has no drem. */
#endif /* Not mc68000 */

    /* Save the frem value. */
    ieee_to_x80(out = n1, dp);

    warning_floating_point("remx(" IEEE_T_FORMAT ", " IEEE_T_FORMAT ") == " IEEE_T_FORMAT "",
                           (IEEE_T_PRINT_CAST)in1, (IEEE_T_PRINT_CAST)in2, (IEEE_T_PRINT_CAST)out);

    /* Put the low-order 7 bits of n into d0.w. */
    cpu_state.regs[0].uw.n = (n & 0x7F);
}

#define CCC cpu_state.ccc
#define CCV cpu_state.ccv
#define CCN cpu_state.ccn
#define CCX cpu_state.ccx
#define CCNZ cpu_state.ccnz

FCMP_RETURN_TYPE Executor::C_ROMlib_Fcmpx(
    void *sp, x80_t *dp, unsigned short sel)
{
    DECLAREIN2();
    ieee_t n1, n2;

    /* Fetch first value to compare. */
    n1 = x80_to_ieee(dp);

    /* Fetch other value to compare. */
    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            n2 = x80_to_ieee((const x80_t *)sp);
            break;
        case FD_OPERAND:
            n2 = f64_to_ieee((const f64_t *)sp);
            break;
        case FS_OPERAND:
            n2 = f32_to_ieee((const f32_t *)sp);
            break;
        case FI_OPERAND:
            n2 = CW_RAW(*(short *)sp);
            break;
        case FL_OPERAND:
            n2 = CL_RAW(*(long *)sp);
            break;
        case FC_OPERAND:
            n2 = comp_to_ieee((const comp_t *)sp);
            break;
        default:
            n2 = 0.0; /* Here to avoid compiler warnings. */
            gui_abort();
    }
    in1 = n1;
    in2 = n2;

/* Compare the two values & set CC bits accordingly. */

#define SET_CCS(x, n, z, v, c) \
    do                         \
    {                          \
        CCC = (c);             \
        CCN = (n);             \
        CCV = (v);             \
        CCX = (x);             \
        CCNZ = !(z);           \
    } while(0)

    /* Set up the CC bits appropriately. */
    if(n1 <= n2)
    {
        if(n1 == n2)
            SET_CCS(0, 0, 1, 0, 0);
        else /* n1 < n2 */
            SET_CCS(1, 1, 0, 0, 1);
    }
    else if(n1 > n2)
        SET_CCS(0, 0, 0, 0, 0);
    else /* Unordered - FIXME; will it actually get here? */
        SET_CCS(0, 0, 0, 1, 0);

    warning_floating_point("cmp(" IEEE_T_FORMAT ", " IEEE_T_FORMAT ") == (cnvxz=%d%d%d%d%d)",
                           (IEEE_T_PRINT_CAST)in1, (IEEE_T_PRINT_CAST)in2,
                           CCC, CCN, CCV, CCX, !CCNZ);
}

FCMP_RETURN_TYPE Executor::C_ROMlib_FcpXx(
    void *sp, x80_t *dp, unsigned short sel)
{
    warning_floating_point(NULL_STRING);
    /* FIXME - this should signal; calling Fcmpx is only a stopgap hack
   * so we can keep testing stuff.
   */
    C_ROMlib_Fcmpx(sp, dp, sel);
}

/* Rounds an ASCII string of digits to a string of `desired_digits'
 * length, and returns the order-of-magnitude difference between
 * this string and the original.
 */
static int
round_string(const char *in, char *out, bool negative_p,
             int desired_digits)
{
    int in_len, out_len, exponent_change, len_diff;

    in_len = strlen(in);

    /* First check for the easy case of no rounding. */
    if(desired_digits >= in_len)
    {
        strcpy(out, in);
        exponent_change = 0;
    }
    else
    {
        char temp[256], *dot;
        double d;

        /* We'll convert the number to a double, and then round it
       * with rint.  This should use the rounding mode we
       * set in Fsetenv.
       */
        sprintf(temp, "%s%.*s.%s", negative_p ? "-" : "", desired_digits, in,
                in + desired_digits);
        d = atof(temp);
        sprintf(out, "%f", rint(d));

        /* Axe any floating point residual that may have appeared. */
        dot = strchr(out, '.');
        if(dot)
            *dot = '\0';

        /* Strip any leading zeros or minus signs that may have crept in. */
        while(out[0] == '0' || out[0] == '-')
            memmove(out, out + 1, strlen(out));

        exponent_change = in_len - desired_digits;
    }

    out_len = strlen(out);
    len_diff = desired_digits - out_len;
    if(len_diff > 0)
        memset(out + out_len, '0', len_diff);
    out[desired_digits] = '\0';
    exponent_change -= len_diff;

    return exponent_change;
}

void Executor::C_ROMlib_Fx2dec(DecForm *sp2, void *sp, Decimal *dp,
                               unsigned short sel)
{
    DECLAREIN();
    ieee_t n;
    int digits;
    char c_string[256] = "?"; /* Big enough to be safe. */

    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            n = x80_to_ieee((const x80_t *)sp);
            break;
        case FD_OPERAND:
            n = f64_to_ieee((const f64_t *)sp);
            break;
        case FS_OPERAND:
            n = f32_to_ieee((const f32_t *)sp);
            break;
        case FI_OPERAND:
            n = CW_RAW(*(short *)sp);
            break;
        case FL_OPERAND:
            n = CL_RAW(*(long *)sp);
            break;
        case FC_OPERAND:
            n = comp_to_ieee((const comp_t *)sp);
            break;
        default:
            n = 0.0; /* Here to avoid compiler warnings. */
            gui_abort();
    }
    in = n;

    /* Fetch the number of digits they are interested in. */
    digits = CW_RAW(*(short *)(&sp2->digits));

    /* Compute sign. */
    if(n < 0)
    {
        dp->sgn = CB(1);
        n = -n;
    }
    else
        dp->sgn = CB(0);

    /* Default to 0 exp, in case of infinity, etc, just to be consistent. */
    dp->exp = CWC(0);

    if(n == 0)
        strcpy(c_string, "0");
    else if(n != n) /* Check for NaN */
        strcpy(c_string, "N"); /* FIXME - more digits?  See SANE p. 28 */
    else if(n * 2 == n) /* Check for +-Inf (0 handled above). */
        strcpy(c_string, "I");
    else /* Normal number. */
    {
        char *digit_string, *dot;
        int exponent, digits_to_keep;

        /* Convert the number to ASCII. */
        digit_string = (char *)alloca(SIGDIGLEN + 256); /* tons of extra space */

#if !defined(CYGWIN32)
        sprintf(digit_string, "%.80Lf", n);
#else
        // FIXME: #warning MAY LOSE PRECISION HERE
        sprintf(digit_string, "%.80f", (double)n);
#endif

        dot = strchr(digit_string, '.');
        if(dot == NULL)
            exponent = 0;
        else
        {
            exponent = dot - &digit_string[strlen(digit_string) - 1];
            memmove(dot, dot + 1, strlen(dot)); /* nuke the decimal point. */
        }

        /* Nuke any leading zeros. */
        while(*digit_string == '0')
            digit_string++;

        /* Check which format they want and act accordingly. */
        if((sp2->style & CWC(DECIMALTYPEMASK)) == CWC(FloatDecimal))
        {
            /* Floating style. */
            digits_to_keep = digits;
        }
        else
        {
            /* Fixed style. */
            digits_to_keep = strlen(digit_string) + exponent + digits;
        }

        /* Round the value appropriately. */
        if(digits_to_keep <= 0)
            c_string[0] = '\0'; /* zero */
        else
            exponent += round_string(digit_string, c_string,
                                     CB(dp->sgn), digits_to_keep);

        /* Make sure the string is short enough. */
        if((int)strlen(c_string) > digits_to_keep)
            c_string[digits_to_keep] = '\0';

        /* Replace the empty string with "0". */
        if(c_string[0] == '\0')
            strcpy(c_string, "0");

        dp->exp = CW(exponent);
    }

    /* See if the generated string is too LONGINT. */

    if(strlen(c_string) >= SIGDIGLEN)
#if 0
    strcpy (c_string, "?");   /* SANE specs say this is OK. */
#else
    {
        int old_len, new_len;

        old_len = strlen(c_string);
        c_string[SIGDIGLEN] = 0;
        new_len = SIGDIGLEN;
        dp->exp = CW(CW(dp->exp) + old_len - new_len);
    }
#endif

        /* Copy the generated string out to their Decimal record. */
        dp->sig[0] = strlen(c_string);
    strncpy((char *)dp->sig + 1,
            c_string, strlen(c_string)); /* Don't copy '\0' */

    warning_floating_point("Fx2dec(" IEEE_T_FORMAT ", digits=%d) == %s%s * 10**%d",
                           (IEEE_T_PRINT_CAST)in, digits, dp->sgn ? "-" : "",
                           c_string, CW(dp->exp));
}

#if defined(CYGWIN32)
#define pow(a, b) my_pow10(b)

PRIVATE double
my_pow10(int i)
{
    double retval;

    retval = 1;
    while(i-- > 0)
        retval *= 10;
    return retval;
}
#endif

void Executor::C_ROMlib_Fdec2x(Decimal *sp, void *dp, unsigned short sel)
{
    DECLAREIN();
    long double n;
    INTEGER exp;
#if 0
  char c_str[SIGDIGLEN + 1];
#endif
    char *p, *last_char;

    /* Parse the value. */
    switch(sp->sig[1])
    {
        case '0':
            n = 0.0;
            break;
        case 'N':
            /* #warning "Expect an invalid operation error here; we're just making a NaN" */
            n = (0.0 / 0.0); /* NaN */

            /* FIXME - grab subsequent bytes of sig and stick them in significand;
     * see SANE book p.28.
     */

            break;
        case 'I':
            /* #warning "Expect an invalid operation error here; we're just making a +Inf" */
            n = (1.0 / 0.0); /* Infinity. */
            break;
        default:
            /* Parse the digits (a potentially large integer) into an ieee_t.
     * May lose precision in the boundary cases.
     */
            last_char = (char *)sp->sig + sp->sig[0];
            for(p = (char *)sp->sig + 1, n = 0; p <= last_char; p++)
                n = (n * 10) + (*p - '0');

            exp = CW(sp->exp);

            if(exp > 0)
                n *= pow(10.0, exp);
            else
                n /= pow(10.0, -exp);

            if(sp->sgn)
                n = -n;

            break;
    }
    in = n;

    /* Write out the value. */
    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:
            ieee_to_x80(n, (x80_t *)dp);
            break;
        case FD_OPERAND:
            ieee_to_f64(n, (f64_t *)dp);
            break;
        case FS_OPERAND:
            ieee_to_f32(n, (f32_t *)dp);
            break;
        case FI_OPERAND:
            *(short *)dp = CW_RAW((signed short)rint(n));
            break;
        case FL_OPERAND:
            *(long *)dp = CL_RAW((LONGINT)rint(n));
            break;
        case FC_OPERAND:
            ieee_to_comp(n, (comp_t *)dp);
            break;
        default:
            gui_abort();
    }

    warning_floating_point("Fdec2x(%s%.*s * 10**%d) == " IEEE_T_FORMAT "",
                           sp->sgn ? "-" : "",
                           (uint8)sp->sig[0], &sp->sig[1],
                           CW(sp->exp), (IEEE_T_PRINT_CAST)in);
}

void Executor::C_ROMlib_Fclassx(void *sp, INTEGER *dp, unsigned short sel)
{
    static const unsigned char eight_zeros[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned short first_word = CW_RAW(*(unsigned short *)sp);

    warning_floating_point(NULL_STRING);

    /* Default to normal number. */
    *dp = CWC_RAW(NormalNum);

    warning_floating_point(NULL_STRING);

    switch(sel & OPCODE_MASK)
    {
        case FX_OPERAND:

#define X_INF_OR_NAN ((unsigned short)0x7FFF)
#define X_NORMNUM_MASK ((unsigned short)0x8000)
#define X_QNAN_MASK ((unsigned short)0x4000)

            if((first_word & X_INF_OR_NAN) == X_INF_OR_NAN)
            {
                if((((unsigned char *)sp)[2] & 0x7F) == 0
                   && !memcmp(((unsigned char *)sp) + 3, eight_zeros, 7))
                    *dp = CWC_RAW(Infinite);
                else if(((unsigned short *)sp)[1] & CWC_RAW(X_QNAN_MASK))
                    *dp = CWC_RAW(QNaN);
                else
                    *dp = CWC_RAW(SNaN);
            }
            else if((((unsigned short *)sp)[1] & CWC_RAW(X_NORMNUM_MASK)) == 0)
            {
                if(!memcmp(((char *)sp) + 2, eight_zeros, 8))
                    *dp = CWC_RAW(ZeroNum);
                else
                    *dp = CWC_RAW(DenormalNum);
            }
            break;
        case FD_OPERAND:

#define D_SIGN_MASK ((unsigned short)0x8000)
#define D_INF_OR_NAN ((unsigned short)0x7FF0)
#define D_NORMNUM_MASK ((unsigned short)0x7FF0)
#define D_QNAN_MASK ((unsigned short)0x0008)

            if((first_word & D_INF_OR_NAN) == D_INF_OR_NAN)
            {
                if((first_word & 0xF) == 0
                   && !memcmp(((char *)sp) + 2, eight_zeros, 6))
                    *dp = CWC_RAW(Infinite);
                else if(first_word & D_QNAN_MASK)
                    *dp = CWC_RAW(QNaN);
                else
                    *dp = CWC_RAW(SNaN);
            }
            else if((first_word & D_NORMNUM_MASK) == 0)
            {
                if((first_word & 0xF) == 0
                   && !memcmp(((char *)sp) + 2, eight_zeros, 6))
                    *dp = CWC_RAW(ZeroNum);
                else
                    *dp = CWC_RAW(DenormalNum);
            }
            break;
        case FS_OPERAND:
        {
#define S_INF_OR_NAN 0x78000000
#define S_NORMNUM_MASK 0x78000000
#define S_FRAC_MASK 0x07FFFFFF
#define S_QNAN_MASK 0x04000000
            ULONGINT v = CL_RAW(*(uint32_t *)sp);
            if((v & S_INF_OR_NAN) == S_INF_OR_NAN)
            {
                if((v & S_FRAC_MASK) == 0)
                    *dp = CWC_RAW(Infinite);
                else if(v & S_QNAN_MASK)
                    *dp = CWC_RAW(QNaN);
                else
                    *dp = CWC_RAW(SNaN);
            }
            else if((v & S_NORMNUM_MASK) == 0)
            {
                if((v & S_FRAC_MASK) == 0)
                    *dp = CWC_RAW(ZeroNum);
                else
                    *dp = CWC_RAW(DenormalNum);
            }
        }
        break;
        case FC_OPERAND:
        {
            static const unsigned char comp_nan[] = { 0x80, 0, 0, 0, 0, 0, 0, 0 };

            if(!memcmp(sp, eight_zeros, sizeof(comp_t)))
                *dp = CWC_RAW(ZeroNum);
            else if(!memcmp(sp, comp_nan, sizeof comp_nan))
                *dp = CWC_RAW(SNaN); /* FIXME - should this be signaling?  Bill sez so */

            /* FIXME - should we let the SNaN get negated, below? */
        }
        break;
        default:
            gui_abort();
            break;
    }

    /* Negate *dp if *sp is negative.  Since all types share the same
   * sign bit, we only need to check the first byte of the type.
   */
    if(*(signed char *)sp < 0)
        *dp = CW_RAW(0 - CW_RAW(*dp));
}

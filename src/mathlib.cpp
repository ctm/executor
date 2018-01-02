/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if defined(powerpc) || defined(__ppc__)

#include "rsys/common.h"
#include "rsys/mathlib.h"

#include "rsys/cfm.h"
#include "rsys/pef.h"

#include <math.h>

#include "SANE.h"

using namespace Executor;

PRIVATE void
num2dec(/* const */ DecForm *f, double d, uint32_t unused1, uint32_t unused2,
        Decimal *dp)
{
    // #warning mystery args
    warning_unexpected("0x%x 0x%x", unused1, unused2);
    warning_trace_info(NULL_STRING);
    ROMlib_Fx2dec(f, &d, dp, FD_OPERAND);
}

PRIVATE double
dec2num(const Decimal *decp)
{
    double retval;

    warning_trace_info(NULL_STRING);
    ROMlib_Fdec2x((Decimal *)decp, &retval, FD_OPERAND);
    return retval;
}

PRIVATE void
str2dec(const char *str, INTEGER *indexp, Decimal *decp, INTEGER *successp)
{
    Byte success;

    warning_trace_info(NULL_STRING);
    ROMlib_Fcstr2dec((char *)str, indexp, decp, &success);
    *successp = success;
}

PRIVATE void
dec2str(const DecForm *sp2, const Decimal *sp, char *dp)
{
    C_ROMlib_Fdec2str((DecForm *)sp2, (Decimal *)sp, dp);
    warning_trace_info(NULL_STRING);
    ROMlib_p2cstr(dp);
}

PRIVATE double
ceil_wrapper(double x)
{
    warning_trace_info("%f", x);
    return ceil(x);
}

PRIVATE double
asin_wrapper(double x)
{
    warning_trace_info("%f", x);
    return asin(x);
}

PRIVATE double
fmod_wrapper(double x, double y)
{
    warning_trace_info("%f %f", x, y);
    return fmod(x, y);
}

PRIVATE double
acos_wrapper(double x)
{
    warning_trace_info("%f", x);
    return acos(x);
}

PRIVATE double
fmin_wrapper(double x, double y)
{
    double retval;

    retval = x < y ? x : y;
    warning_trace_info("%f %f", x, y);
    return retval;
}

PRIVATE double
fmax_wrapper(double x, double y)
{
    double retval;

    retval = x < y ? y : x;
    warning_trace_info("%f %f", x, y);
    return retval;
}

PRIVATE double
atan_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return atan(x);
}

PRIVATE double
atan2_wrapper(double x, double y)
{
    warning_trace_info(NULL_STRING);
    return atan2(x, y);
}

PRIVATE double
cos_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return cos(x);
}

PRIVATE double
tan_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return tan(x);
}

PRIVATE long
__isfinited_wrapper(double x)
{
    long retval;

    retval = isfinite(x);
    warning_trace_info("%f, %ld", x, retval);
    return retval;
}

PRIVATE long
__isnand_wrapper(double x)
{
    long retval;

    retval = isnan(x);
    warning_trace_info("%f, %ld", x, retval);
    return retval;
}

PRIVATE double
round_wrapper(double x)
{
    double retval;

    switch(fpclassify(x))
    {
        case FP_NAN:
        case FP_INFINITE:
        case FP_ZERO:
            retval = x;
            break;
        default:
            if(x < 0)
                retval = floor(x - .5);
            else
                retval = floor(x + .5);
            break;
    }

    // #warning we do not set inexact exceptions
    warning_trace_info("%f, %f", x, retval);
    return retval;
}

PRIVATE double
sqrt_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return sqrt(x);
}

PRIVATE double
pow_wrapper(double x, double y)
{
    warning_trace_info(NULL_STRING);
    return pow(x, y);
}

PRIVATE double
sin_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return sin(x);
}

PRIVATE double
exp_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return exp(x);
}

PRIVATE double
log_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return log(x);
}

PRIVATE double
__inf_wrapper(void)
{
    warning_trace_info(NULL_STRING);
    return HUGE_VAL;
}

PRIVATE double
floor_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return floor(x);
}

PRIVATE double
fabs_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return fabs(x);
}

PRIVATE double
log10_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return log10(x);
}

PRIVATE double
nan_wrapper(void)
{
    x80_t x;
    double retval;

    memset(&x, 0xff, sizeof x);
    ROMlib_FX2x(&x, &retval, FD_OPERAND);

    warning_trace_info(NULL_STRING);
    return retval;
}

PRIVATE void
feclearexcept_stub(uint32_t bits)
{
    warning_trace_info("bits = 0x%x", bits);
    warning_unimplemented("bits = 0x%x", bits);
}

typedef uint32_t fexcept_t;

PRIVATE void
fesetexcept_stub(const fexcept_t *flagp, int bits)
{
    warning_trace_info(NULL_STRING);
    warning_unimplemented(NULL_STRING);
}

PRIVATE uint32_t
fetestexcept_stub(uint32_t bits)
{
    warning_trace_info(NULL_STRING);
    warning_unimplemented(NULL_STRING);
    return 0;
}

PRIVATE double
frexp_wrapper(double x, int *exp)
{
    warning_trace_info(NULL_STRING);
    return frexp(x, exp);
}

PRIVATE double
ldexp_wrapper(double x, int exp)
{
    warning_trace_info(NULL_STRING);
    return ldexp(x, exp);
}

PRIVATE double
cosh_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return cosh(x);
}

PRIVATE double
sinh_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return sinh(x);
}

PRIVATE double
tanh_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return tanh(x);
}

PRIVATE double
trunc_wrapper(double x)
{
    warning_trace_info(NULL_STRING);
    return trunc(x);
}

PRIVATE map_entry_t
    mathlib_map[]
    = {
        {
            "log", log_wrapper,
        },
        {
            "num2dec", num2dec,
        },
        //  { "x80told", C_x80told, },
        {
            "atan", atan_wrapper,
        },
        {
            "cos", cos_wrapper,
        },
        {
            "dec2str", dec2str,
        },
        {
            "sqrt", sqrt_wrapper,
        },
        {
            "sin", sin_wrapper,
        },
        {
            "exp", exp_wrapper,
        },
        {
            "pow", pow_wrapper,
        },
        {
            "__inf", __inf_wrapper,
        },
        {
            "dec2num", dec2num,
        },
        {
            "str2dec", str2dec,
        },
        {
            "floor", floor_wrapper,
        },
        {
            "fabs", fabs_wrapper,
        },
        {
            "log10", log10_wrapper,
        },
        {
            "nan", nan_wrapper,
        },
        {
            "ceil", ceil_wrapper,
        },
        {
            "asin", asin_wrapper,
        },
        {
            "fmod", fmod_wrapper,
        },
        {
            "acos", acos_wrapper,
        },
        {
            "fmin", fmin_wrapper,
        },
        {
            "fmax", fmax_wrapper,
        },
        {
            "tan", tan_wrapper,
        },
        {
            "atan2", atan2_wrapper,
        },
        {
            "__isfinited", __isfinited_wrapper,
        },
        {
            "__isnand", __isnand_wrapper,
        },
        {
            "round", round_wrapper,
        },

        {
            "feclearexcept", feclearexcept_stub,
        },
        {
            "fesetexcept", fesetexcept_stub,
        },
        {
            "fetestexcept", fetestexcept_stub,
        },

        {
            "frexp", frexp_wrapper,
        },
        {
            "ldexp", ldexp_wrapper,
        },
        {
            "cosh", cosh_wrapper,
        },
        {
            "sinh", sinh_wrapper,
        },
        {
            "tanh", tanh_wrapper,
        },
        {
            "trunc", trunc_wrapper,
        },

      };

PUBLIC OSErr
ROMlib_GetMathLib(Str63 library, OSType arch, LoadFlags loadflags,
                  ConnectionID *cidp, Ptr *mainaddrp, Str255 errName)
{
    static ConnectionID cid;
    OSErr retval;

    if(cid)
    {
        *cidp = cid;
        retval = noErr;
    }
    else
    {
#if !defined(CFM_PROBLEMS)
        cid = ROMlib_new_connection(1);
#else
        // FIXME: #warning "Will not work until CFM is viable"
        cid = 0;
#endif
        if(!cid)
            retval = fragNoMem;
        else
        {
            cid->lihp = ROMlib_build_pef_hash(mathlib_map, NELEM(mathlib_map));
            cid->ref_count = 1;
            retval = noErr;
            *cidp = cid;
        }
    }
    if(retval == noErr)
        *mainaddrp = 0;
    return retval;
}

#endif

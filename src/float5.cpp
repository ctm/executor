/* Copyright 1990-1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include <math.h>

#include "SANE.h"
#include "rsys/float.h"
#include "rsys/floatconv.h"

using namespace Executor;

void Executor::C_ROMlib_FlnX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = log(in = x80_to_ieee(dp)), dp);
    warning_floating_point("ln(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Flog2X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = log(in = x80_to_ieee(dp)) * M_LOG2E, dp);
    warning_floating_point("log2(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Fln1X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = log1p(in = x80_to_ieee(dp)), dp);
    warning_floating_point("ln1(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Flog21X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = log1p(in = x80_to_ieee(dp)) * M_LOG2E, dp);
    warning_floating_point("log21(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_FexpX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = exp(in = x80_to_ieee(dp)), dp);
    warning_floating_point("exp(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Fexp2X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = pow(2.0, in = x80_to_ieee(dp)), dp);
    warning_floating_point("exp2(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Fexp1X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = exp(in = x80_to_ieee(dp)) - 1, dp);
    warning_floating_point("exp1(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Fexp21X(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = pow(2.0, in = x80_to_ieee(dp)) - 1, dp);
    warning_floating_point("exp21(%f) == %f", (double)in, (double)out);
}

void Executor::C_ROMlib_Fxpwri(INTEGER *sp, x80_t *dp)
{
    DECLAREIN2OUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = pow(in1 = x80_to_ieee(dp), in2 = CW_RAW(*(short *)sp)), dp);
    warning_floating_point("xpwri(%f, %f) == %f",
                           (double)in1, (double)in2, (double)out);
}

void Executor::C_ROMlib_Fxpwry(x80_t *sp, x80_t *dp)
{
    ieee_t r, n;
    DECLAREIN2OUT();

    /* FIXME - may lose precision! */
    r = x80_to_ieee(dp);
    n = x80_to_ieee(sp);
    in1 = r;
    in2 = n;
    ieee_to_x80(out = pow(r, n), dp);
    warning_floating_point("xpwry(%f, %f) == %f",
                           (double)in1, (double)in2, (double)out);
}

void Executor::C_ROMlib_Fcompound(x80_t *sp2, x80_t *sp, x80_t *dp)
{
    DECLAREIN2OUT();

    /* FIXME - may lose precision! */
    ieee_t r = x80_to_ieee(sp2), n = x80_to_ieee(sp);
    in1 = r;
    in2 = n;
    ieee_to_x80(out = pow(1 + r, n), dp);
    warning_floating_point("compound(%f, %f) == %f",
                           (double)in1, (double)in2, (double)out);
}

void Executor::C_ROMlib_Fannuity(x80_t *sp2, x80_t *sp, x80_t *dp)
{
    DECLAREIN2OUT();

    ieee_t r = x80_to_ieee(sp2), n = x80_to_ieee(sp);
    in1 = r;
    in2 = n;
    ieee_to_x80(out = (1 - (pow(1 + r, -n))) / r, dp);
    warning_floating_point("annuity(%f, %f) == %f",
                           (double)in1, (double)in2, (double)out);
}

void Executor::C_ROMlib_FsinX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = sin(in = x80_to_ieee(dp)), dp);
    warning_floating_point("sin(%f) == %f",
                           (double)in, (double)out);
}

void Executor::C_ROMlib_FcosX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = cos(in = x80_to_ieee(dp)), dp);
    warning_floating_point("cos(%f) == %f",
                           (double)in, (double)out);
}

void Executor::C_ROMlib_FtanX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = tan(in = x80_to_ieee(dp)), dp);
    warning_floating_point("tan(%f) == %f",
                           (double)in, (double)out);
}

void Executor::C_ROMlib_FatanX(x80_t *dp)
{
    DECLAREINOUT();

    /* FIXME - may lose precision! */
    ieee_to_x80(out = atan(in = x80_to_ieee(dp)), dp);
    warning_floating_point("atan(%f) == %f",
                           (double)in, (double)out);
}

void Executor::C_ROMlib_FrandX(x80_t *dp)
{
    DECLAREINOUT();

/* Derived from Bill Goldman's old code. */
#define SEVEN_TO_FIFTH 16807
    in = x80_to_ieee(dp);
    out = fmod(in * SEVEN_TO_FIFTH, 0x7FFFFFFF);
    ieee_to_x80(out, dp);
    warning_floating_point("rand(%f) == %f",
                           (double)in, (double)out);
}

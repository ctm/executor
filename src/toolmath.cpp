/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_toolmath[] =
	    "$Id: toolmath.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ToolboxUtil.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ToolboxUtil.h"
#include <float.h>
#include <math.h>

using namespace Executor;

P1(PUBLIC pascal trap, Fract, FracSqrt, Fract, x)	/* IMIV-64 */
{
    Extended z;

    z = sqrt(Frac2X(x));
    return X2Frac(&z);
}

#define POVER4 (ULONGINT) 0x0000C910	/* IMIV-64 */

P1(PUBLIC pascal trap, Fract, FracSin, Fixed, x)	/* IMIV-64 */
{
    Extended z;
    LONGINT oct;

    if (x < 0)
	x += (-(x+1) / (POVER4 * 8) + 1) * POVER4 * 8;
    oct = x / POVER4 % 8;
    x %= POVER4 * 4;
    switch (oct) {
    case 0:
	z = sin(Fix2X(x));
	break;
    case 1:
	z = cos(Fix2X(POVER4 * 2 - x));
	break;
    case 2:
	z = cos(Fix2X(x - POVER4 * 2));
	break;
    case 3:
	z = sin(Fix2X(POVER4 * 4 - x));
	break;
    case 4:
	z = -sin(Fix2X(x));
	break;
    case 5:
	z = -cos(Fix2X(POVER4 * 2 - x));
	break;
    case 6:
	z = -cos(Fix2X(x - POVER4 * 2));
	break;
    case 7:
	z = -sin(Fix2X(POVER4 * 4 - x));
	break;
    }
    return X2Frac(&z);
}

A2(PUBLIC, Fixed, FixATan2, LONGINT, x, LONGINT, y)	/* IMIV-65 */
{
    Extended z;

    if (y == 0) {
	if (x & 0x80000000)
/*-->*/	    return x & 0x7FFFFFFF ? 205888 : 234019;	/* trial and error */
/*-->*/	return 0;
    }
    z = atan2((double) y, (double) x);
    return X2Fix(&z);
}
P2(PUBLIC pascal trap, Fixed, FixAtan2, LONGINT, x, LONGINT, y)	/* IMIV-65 */
{
    return FixATan2(x, y);
}

P1(PUBLIC pascal trap, Fract, FracCos, Fixed, x)	/* IMIV-64 */
{
    return FracSin(x + POVER4 * 2);	/* could produce overflow ! */
}

/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "ToolboxUtil.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/glue.h"

using namespace Executor;

void Executor::C_TextFont(INTEGER f)
{
    if(thePort)
        PORT_TX_FONT_X(thePort) = CW(f);
}

void Executor::C_TextFace(INTEGER thef)
{
    if(thePort)
        PORT_TX_FACE_X(thePort) = thef;
}

void Executor::C_TextMode(INTEGER m)
{
    if(thePort)
        PORT_TX_MODE_X(thePort) = CW(m);
}

void Executor::C_TextSize(INTEGER s)
{
    if(thePort)
        PORT_TX_SIZE_X(thePort) = CW(s);
}

void Executor::C_SpaceExtra(Fixed e)
{
    if(thePort)
        PORT_SP_EXTRA_X(thePort) = CL(e);
}

void Executor::C_DrawChar(CharParameter thec)
{
    Point p;
    Byte c;

    c = thec;
    p.h = 1;
    p.v = 1;
    CALLTEXT(1, (Ptr)&c, p, p);
}

void Executor::C_DrawString(StringPtr s)
{
    Point p;

    p.h = 1;
    p.v = 1;
    CALLTEXT((INTEGER)U(s[0]), (Ptr)(s + 1), p, p);
}

void Executor::C_DrawText(Ptr tb, INTEGER fb, INTEGER bc)
{
    Point p;

    p.h = 1;
    p.v = 1;
    CALLTEXT(bc, tb + fb, p, p);
}

/* This is a convenience function so we can more easily call DrawText
 * with C strings.
 */
void Executor::DrawText_c_string(const char *string)
{
    DrawText((Ptr)string, 0, strlen(string));
}

INTEGER Executor::C_CharWidth(CharParameter thec)
{
    GUEST<Point> np, dp;
    INTEGER retval;
    FontInfo fi;
    Byte c;

    c = thec;
    np.h = np.v = dp.h = dp.v = CWC(256);
    retval = CALLTXMEAS(1, (Ptr)&c, &np, &dp, &fi);
    return FixMul((LONGINT)retval << 16, (Fixed)CW(np.h) << 8) / ((LONGINT)CW(dp.h) << 8);
}

INTEGER Executor::C_StringWidth(StringPtr s)
{
    GUEST<Point> np, dp;
    INTEGER retval;
    FontInfo fi;

    np.h = np.v = dp.h = dp.v = CWC(256);
    retval = CALLTXMEAS((INTEGER)U(s[0]), (Ptr)s + 1, &np, &dp, &fi);
    return FixMul((LONGINT)retval << 16,
                  (Fixed)CW(np.h) << 8)
        / ((LONGINT)CW(dp.h) << 8);
}

INTEGER Executor::C_TextWidth(Ptr tb, INTEGER fb, INTEGER bc)
{
    GUEST<Point> np, dp;
    INTEGER retval;
    FontInfo fi;

    np.h = np.v = dp.h = dp.v = CWC(256);
    retval = CALLTXMEAS(bc, tb + fb, &np, &dp, &fi);
    return FixMul((LONGINT)retval << 16,
                  (Fixed)CW(np.h) << 8)
        / ((LONGINT)CW(dp.h) << 8);
}

void Executor::C_GetFontInfo(FontInfo *ip)
{
    GUEST<Point> pn, pd;

    pn.h = CWC(1);
    pn.v = CWC(1);
    pd.v = CWC(1);
    pd.h = CWC(1);
    CALLTXMEAS(0, (Ptr) "", &pn, &pd, ip);
    ip->ascent = CW(CW(ip->ascent) * CW(pn.v) / CW(pd.v));
    ip->descent = CW(CW(ip->descent) * CW(pn.v) / CW(pd.v));
    ip->leading = CW(CW(ip->leading) * CW(pn.v) / CW(pd.v));
}

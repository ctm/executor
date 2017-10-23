/* Copyright 1986, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qText[] =
	    "$Id: qText.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "ToolboxUtil.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/glue.h"

using namespace Executor;

P1(PUBLIC pascal trap, void, TextFont, INTEGER, f)
{
  if (thePort)
    PORT_TX_FONT_X (thePort) = CW(f);
}

P1(PUBLIC pascal trap, void, TextFace, INTEGER, thef)
{
  if (thePort)
    PORT_TX_FACE_X (thePort) = thef;
}

P1(PUBLIC pascal trap, void, TextMode, INTEGER, m)
{
  if (thePort)
    PORT_TX_MODE_X (thePort) = CW(m);
}

P1(PUBLIC pascal trap, void, TextSize, INTEGER, s)
{
  if (thePort)
    PORT_TX_SIZE_X (thePort) = CW(s);
}

P1(PUBLIC pascal trap, void, SpaceExtra, Fixed, e)
{
  if (thePort)
    PORT_SP_EXTRA_X (thePort) = CL(e);
}

P1(PUBLIC pascal trap, void, DrawChar, CHAR, thec)
{
    Point p;
    Byte c;
    
    c = thec;
    p.h = 1;
    p.v = 1;
    CALLTEXT(1, (Ptr) &c, p, p);
}

P1(PUBLIC pascal trap, void, DrawString, StringPtr, s)
{
    Point p;
    
    p.h = 1;
    p.v = 1;
    CALLTEXT( (INTEGER)U(s[0]), (Ptr) (s+1), p, p);
}

P3(PUBLIC pascal trap, void, DrawText, Ptr, tb, INTEGER, fb, INTEGER, bc)
{
    Point p;
    
    p.h = 1;
    p.v = 1;
    CALLTEXT(bc, tb+fb, p, p);
}

/* This is a convenience function so we can more easily call DrawText
 * with C strings.
 */
void
Executor::DrawText_c_string (char *string)
{
  DrawText ((Ptr) string, 0, strlen (string));
}

P1(PUBLIC pascal trap, INTEGER, CharWidth, CHAR, thec)
{
    GUEST<Point> np, dp;
    INTEGER retval;
    FontInfo fi;
    Byte c;
    
    c = thec;
    np.h = np.v = dp.h = dp.v = CWC(256);
    retval =  CALLTXMEAS(1, (Ptr) &c, &np, &dp, &fi);
    return FixMul((LONGINT) retval << 16, (Fixed) CW(np.h) << 8) / ((LONGINT) CW(dp.h) << 8);
}

P1(PUBLIC pascal trap, INTEGER, StringWidth, StringPtr, s)
{
  GUEST<Point> np, dp;
  INTEGER retval;
  FontInfo fi;
  
  np.h = np.v = dp.h = dp.v = CWC (256);
  retval =  CALLTXMEAS((INTEGER)U(s[0]), (Ptr) s+1, &np, &dp, &fi);
  return FixMul ((LONGINT) retval << 16,
		 (Fixed) CW (np.h) << 8) / ((LONGINT) CW (dp.h) << 8);
}

P3(PUBLIC pascal trap, INTEGER, TextWidth, Ptr, tb, INTEGER, fb, INTEGER, bc)
{
  GUEST<Point> np, dp;
  INTEGER retval;
  FontInfo fi;
  
  np.h = np.v = dp.h = dp.v = CWC(256);
  retval =  CALLTXMEAS(bc, tb+fb, &np, &dp, &fi);
  return FixMul ((LONGINT) retval << 16,
		 (Fixed) CW(np.h) << 8) / ((LONGINT) CW(dp.h) << 8);
}

P1(PUBLIC pascal trap, void, GetFontInfo, FontInfo *, ip)
{
    GUEST<Point> pn, pd;
    
    pn.h = CWC(1);
    pn.v = CWC(1);
    pd.v = CWC(1);
    pd.h = CWC(1);
    CALLTXMEAS(0, (Ptr) "", &pn, &pd, ip);
    ip->ascent  = CW(CW(ip->ascent ) * CW(pn.v) / CW(pd.v));
    ip->descent = CW(CW(ip->descent) * CW(pn.v) / CW(pd.v));
    ip->leading = CW(CW(ip->leading) * CW(pn.v) / CW(pd.v));
}

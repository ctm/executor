/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_ctlSet[] =
		"$Id: ctlSet.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ControlMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"

#include "rsys/ctl.h"

using namespace Executor;

P2(PUBLIC pascal trap, void, SetCtlValue, ControlHandle, c,	/* IMI-326 */
								  INTEGER, v)
{
  CTL_CALL_EXCURSION
    (c,
     {
       SetPort(HxP(c, contrlOwner));
       if (v < Hx(c, contrlMin))
	 HxX(c, contrlValue) = HxX(c, contrlMin);
       else if (v > Hx(c, contrlMax))
	 HxX(c, contrlValue) = HxX(c, contrlMax);
       else
	 HxX(c, contrlValue) = CW(v);
       CTLCALL (c, drawCntl, ALLINDICATORS);
     });
    
    EM_D0 = 0;
}

P1(PUBLIC pascal trap, INTEGER, GetCtlValue,			/* IMI-326 */
					        ControlHandle, c)
{
  return Hx(c, contrlValue);
}

P2(PUBLIC pascal trap, void, SetCtlMin, ControlHandle, c,	/* IMI-326 */
								  INTEGER, v)
{
  CTL_CALL_EXCURSION
    (c,
     {
       SetPort(HxP(c, contrlOwner));
       HxX(c, contrlMin) = CW(v);
       if (Hx(c, contrlValue) < v)
	 HxX(c, contrlValue) = CW(v);
       CTLCALL(c, drawCntl, ALLINDICATORS);
     });
}

P1(PUBLIC pascal trap, INTEGER, GetCtlMin, ControlHandle, c)	/* IMI-327 */
{
    return Hx(c, contrlMin);
}

P2(PUBLIC pascal trap, void, SetCtlMax, ControlHandle, c,	/* IMI-327 */
								  INTEGER, v)
{
  CTL_CALL_EXCURSION
    (c,
     {
       	/* #### TEST ON MAC MacBreadboard's behaviour suggests that
	   this code is needed. */
       if (v < Hx(c, contrlMin))
	 v = Hx(c, contrlMin);
       
       HxX(c, contrlMax) = CW(v);
       if (Hx(c, contrlValue) > v)
	 HxX(c, contrlValue) = CW(v);
       CTLCALL(c, drawCntl, ALLINDICATORS);
     });
}

P1(PUBLIC pascal trap, INTEGER, GetCtlMax, ControlHandle, c)	/* IMI-327 */
{
  return Hx(c, contrlMax);
}

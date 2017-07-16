/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialInit[] =
	    "$Id: dialInit.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "DialogMgr.h"
#include "FontMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/itm.h"
#include "rsys/mman.h"

using namespace Executor;
using namespace ByteSwap;

P1(PUBLIC, pascal void,  ROMlib_mysound, INTEGER, i)
{
    while (i--)
        SysBeep(5);
}

P1(PUBLIC pascal trap, void, ErrorSound, ProcPtr, sp)	/* IMI-411 */
{
    DABeeper = RM(sp);
}

P1 (PUBLIC pascal trap, void, InitDialogs, ProcPtr, rp)	/* IMI-411 */
{
  Ptr nothing = (Ptr) "";
  
  ZONE_SAVE_EXCURSION
    (SysZone,
     {
       DlgFont = CWC (systemFont);
       ResumeProc = RM (rp);
       ErrorSound ((ProcPtr) P_ROMlib_mysound);
	   
	   
       PtrToHand (nothing, &DAStrings_H[0], (LONGINT) 1);
	   HIDDEN_Ptr *afg = DAStrings_H[0].p;
       DAStrings_H[0].p = RM (afg);
       PtrToHand (nothing, &DAStrings_H[1], (LONGINT) 1);
	   afg = DAStrings_H[1].p;
       DAStrings_H[1].p = RM (afg);
       PtrToHand (nothing, &DAStrings_H[2], (LONGINT) 1);
	   afg = DAStrings_H[2].p;
       DAStrings_H[2].p = RM (afg);
       PtrToHand (nothing, &DAStrings_H[3], (LONGINT) 1);
	   afg = DAStrings_H[2].p;
       DAStrings_H[3].p = RM (afg);
     });
}

A1(PUBLIC, void, SetDAFont, INTEGER, i)	/* IMI-412 */
{
    DlgFont = BigEndianValue(i);
}

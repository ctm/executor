/* Copyright 1987, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_resGettype[] =
	    "$Id: resGettype.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "rsys/resource.h"
#include "rsys/mman.h"

namespace Executor {
  typedef ResType *restypeptr;
  MAKE_HIDDEN(restypeptr);
  
  PRIVATE HIDDEN_restypeptr *ar = 0;
  PRIVATE INTEGER inserttypes(resmaphand, INTEGER, BOOLEAN);
  PRIVATE INTEGER initar(INTEGER);
}
#define ARRN_NOTINITTED	(-1)
#define ARRN_ALL	(-2)

using namespace Executor;

PRIVATE INTEGER arrn = ARRN_NOTINITTED;

A0(PUBLIC, void, ROMlib_invalar)				/* INTERNAL */
{
    if (ar)
	EmptyHandle((Handle) ar);
    arrn = ARRN_NOTINITTED;
}

A3(PRIVATE, INTEGER, inserttypes, resmaphand, map, INTEGER, ninserted,
								BOOLEAN, first)
{
    typref *tr;
    INTEGER i, j;
    ResType *next, *check;
    ResType candidate;

    next = STARH(ar) + ninserted;
    if (first) {
	WALKTR(map, i, tr)
	    *next++ = tr->rtyp;
	EWALKTR(tr)
    } else {
	WALKTR(map, i, tr)
	    candidate = tr->rtyp;
	    check = STARH(ar);
	    for (j = ninserted-1; --j > -1 && *check++ != candidate;)
		;
	    if (j <= -1) {
		*next++ = candidate;
		ninserted++;
	    }
	EWALKTR(tr)
    }
    return next - STARH(ar);
}

A1(PRIVATE, INTEGER, initar, INTEGER, rn)
{
    Size mostbytesneeded;
    INTEGER ninserted;
    resmaphand map;
    BOOLEAN first;

    if (arrn != rn) {
	mostbytesneeded = 0;
	if (rn >= 0) {
	    if ((map = ROMlib_rntohandl(rn, (Handle *) 0)))
		mostbytesneeded = NUMTMINUS1(map) + 1;
	} else if (rn == ARRN_ALL) {
	    WALKMAPTOP(map)
		mostbytesneeded += NUMTMINUS1(map) + 1;
	    EWALKMAP()
	}
	mostbytesneeded *= sizeof(ResType);
	if (ar)
	    ReallocHandle((Handle) ar, mostbytesneeded);
	else
	  {
	    ZONE_SAVE_EXCURSION
	      (SysZone,
	       {
		 ar = (HIDDEN_restypeptr *) NewHandle(mostbytesneeded);
	       });
	  }
	ninserted = 0;
	if (rn >= 0) {
	    if ((map = ROMlib_rntohandl(rn, (Handle *) 0)))
		ninserted = inserttypes(map, ninserted, TRUE);
	} else if (rn == ARRN_ALL) {
	    first = TRUE;
	    WALKMAPTOP(map)
		ninserted = inserttypes(map, ninserted, first);
		first = FALSE;
	    EWALKMAP()
	}
	SetHandleSize((Handle) ar, (Size) ninserted * sizeof(ResType));
	arrn = rn;
    }
    return GetHandleSize((Handle) ar) / sizeof(ResType);
}

P0(PUBLIC pascal trap, INTEGER, CountTypes)
{
    return initar(ARRN_ALL);
}

P0(PUBLIC pascal trap, INTEGER, Count1Types)   /* IMIV-15 */
{
    return initar(Cx(CurMap));
}

P2(PUBLIC pascal trap, void, GetIndType, ResType *, typ, INTEGER, indx)
{
    if (indx <= 0 || indx > initar(ARRN_ALL))
        *typ = 0;
    else
	*typ = STARH(ar)[indx-1];
}

P2(PUBLIC pascal trap, void, Get1IndType, ResType *, typ,	/* IMIV-15 */
						          INTEGER, indx)
{
    if (indx <= 0 || indx > initar(Cx(CurMap)))
        *typ = 0;
    else
	*typ = STARH(ar)[indx-1];
}

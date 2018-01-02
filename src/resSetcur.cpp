/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "rsys/resource.h"

using namespace Executor;

PUBLIC pascal trap INTEGER Executor::C_CurResFile()
{
    ROMlib_setreserr(noErr);
    return (Cx(CurMap));
}

/* ROMlib_findmapres:
      does same as ROMlib_findres (see below) except map is specified */

PUBLIC OSErr Executor::ROMlib_findmapres(resmaphand map, Handle r, /* INTERNAL */
   typref ** trp, resref ** rrp)
{
    INTEGER i, j;
    typref *tr;
    resref *rr;

    WALKTANDR(map, i, tr, j, rr)
    if((Handle)MR(rr->rhand) == r)
    {
        *trp = tr;
        *rrp = rr;
        return (noErr);
    }
    EWALKTANDR(tr, rr)
    return (resNotFound);
}

/* ROMlib_findres:  given a handle to a resource, r,
	    ROMlib_findres fills in *mapp,
             *trp, *rrp with the Handle to the map, and pointers to
             the typref and the resref */

PUBLIC OSErr Executor::ROMlib_findres(Handle r, resmaphand * mapp, /* INTERNAL */
   typref ** trp, resref ** rrp)
{
    resmaphand map;

    if(!r)
        /*-->*/ return resNotFound;
    WALKMAPTOP(map)
    if(ROMlib_findmapres(map, r, trp, rrp) == noErr)
    {
        *mapp = map;
        return (noErr);
    }
    EWALKMAP()
    return (resNotFound);
}

PUBLIC pascal trap INTEGER Executor::C_HomeResFile(Handle res)
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if(ResErr != CWC(noErr))
        return (-1);
    else
        return (Hx(map, resfn));
}

PUBLIC pascal trap void Executor::C_UseResFile(INTEGER rn)
{
    Handle map;

    if(rn == 0)
        rn = Cx(SysMap);
    ROMlib_invalar();
    if(ROMlib_rntohandl(rn, &map))
    {
        CurMap = CW(rn);
        ROMlib_setreserr(noErr);
    }
    else
        ROMlib_setreserr(resFNotFound);
}

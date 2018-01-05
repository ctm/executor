/* Copyright 1987, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "rsys/resource.h"

using namespace Executor;

LONGINT Executor::C_MaxSizeRsrc(Handle h) /* IMIV-16 */
{
    resmaphand map;
    typref *tr;
    resref *rr;
    LONGINT dl, mdl, nl;
    INTEGER i, j;

    ROMlib_setreserr(ROMlib_findres(h, &map, &tr, &rr));
    if(LM(ResErr) != CWC(noErr))
        return (-1);
    if(!rr->rhand || !(*(Handle)MR(rr->rhand)))
    { /* STARH is overkill */
        dl = B3TOLONG(rr->doff);
        mdl = Hx(map, rh.datlen);
        WALKTANDR(map, i, tr, j, rr)
        if((nl = B3TOLONG(rr->doff)) > dl && nl < mdl)
            mdl = nl;
        EWALKTANDR(tr, rr)
        return (mdl - dl);
    }
    else
        return (GetHandleSize((Handle)MR(rr->rhand)));
}

LONGINT Executor::C_RsrcMapEntry(Handle h) /* IMIV-16 */
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(h, &map, &tr, &rr));
    if(LM(ResErr) != CWC(noErr))
        return (0);
    return ((char *)rr - (char *)STARH(map));
}

/* OpenRFPerm is in resOpen.c */

Handle Executor::C_RGetResource(ResType typ, INTEGER id)
{
    return GetResource(typ, id);
}

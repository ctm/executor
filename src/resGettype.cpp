/* Copyright 1987, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"
#include "rsys/resource.h"
#include "rsys/mman.h"

using namespace Executor;

typedef GUEST<ResType> *restypeptr;

static GUEST<restypeptr> *ar = 0;
static INTEGER inserttypes(resmaphand, INTEGER, BOOLEAN);
static INTEGER initar(INTEGER);

#define ARRN_NOTINITTED (-1)
#define ARRN_ALL (-2)

static INTEGER arrn = ARRN_NOTINITTED;

void Executor::ROMlib_invalar() /* INTERNAL */
{
    if(ar)
        EmptyHandle((Handle)ar);
    arrn = ARRN_NOTINITTED;
}

static INTEGER inserttypes(resmaphand map, INTEGER ninserted, BOOLEAN first)
{
    typref *tr;
    INTEGER i, j;
    GUEST<ResType> *next, *check;
    GUEST<ResType> candidate;

    next = STARH(ar) + ninserted;
    if(first)
    {
        WALKTR(map, i, tr)
        *next++ = tr->rtyp;
        EWALKTR(tr)
    }
    else
    {
        WALKTR(map, i, tr)
        candidate = tr->rtyp;
        check = STARH(ar);
        for(j = ninserted - 1; --j > -1 && *check++ != candidate;)
            ;
        if(j <= -1)
        {
            *next++ = candidate;
            ninserted++;
        }
        EWALKTR(tr)
    }
    return next - STARH(ar);
}

static INTEGER initar(INTEGER rn)
{
    Size mostbytesneeded;
    INTEGER ninserted;
    resmaphand map;
    BOOLEAN first;

    if(arrn != rn)
    {
        mostbytesneeded = 0;
        if(rn >= 0)
        {
            if((map = ROMlib_rntohandl(rn, (Handle *)0)))
                mostbytesneeded = NUMTMINUS1(map) + 1;
        }
        else if(rn == ARRN_ALL)
        {
            WALKMAPTOP(map)
            mostbytesneeded += NUMTMINUS1(map) + 1;
            EWALKMAP()
        }
        mostbytesneeded *= sizeof(ResType);
        if(ar)
            ReallocHandle((Handle)ar, mostbytesneeded);
        else
        {
            TheZoneGuard guard(LM(SysZone));

            ar = (GUEST<restypeptr> *)NewHandle(mostbytesneeded);
        }
        ninserted = 0;
        if(rn >= 0)
        {
            if((map = ROMlib_rntohandl(rn, (Handle *)0)))
                ninserted = inserttypes(map, ninserted, true);
        }
        else if(rn == ARRN_ALL)
        {
            first = true;
            WALKMAPTOP(map)
            ninserted = inserttypes(map, ninserted, first);
            first = false;
            EWALKMAP()
        }
        SetHandleSize((Handle)ar, (Size)ninserted * sizeof(ResType));
        arrn = rn;
    }
    return GetHandleSize((Handle)ar) / sizeof(ResType);
}

INTEGER Executor::C_CountTypes()
{
    return initar(ARRN_ALL);
}

INTEGER Executor::C_Count1Types() /* IMIV-15 */
{
    return initar(Cx(LM(CurMap)));
}

void Executor::C_GetIndType(GUEST<ResType> *typ, INTEGER indx)
{
    if(indx <= 0 || indx > initar(ARRN_ALL))
        *typ = 0;
    else
        *typ = STARH(ar)[indx - 1];
}

void Executor::C_Get1IndType(GUEST<ResType> *typ, INTEGER indx) /* IMIV-15 */
{
    if(indx <= 0 || indx > initar(Cx(LM(CurMap))))
        *typ = 0;
    else
        *typ = STARH(ar)[indx - 1];
}

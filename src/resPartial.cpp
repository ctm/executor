/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"

#include "ResourceMgr.h"
#include "FileMgr.h"
#include "rsys/resource.h"
#include "rsys/file.h"

using namespace Executor;

P4(PUBLIC pascal trap, void, ReadPartialResource,
   Handle, res, int32_t, offset, Ptr, buffer, int32_t, count)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    OSErr err;

    err = ROMlib_findres(res, &map, &tr, &rr);
    if(err == noErr)
    {
        if(rr->doff[0] == 0xff && rr->doff[1] == 0xff && rr->doff[2] == 0xff)
            err = resNotFound;
        else
        {
            LONGINT cur_size;
            LONGINT loc;

            cur_size = ROMlib_SizeResource(res, false);
            err = CW(ResErr);
            if(err == noErr && (uint32_t)offset + count > (uint32_t)cur_size)
                err = inputOutOfBounds;
            else
            {
                INTEGER rn;

                rn = Hx(map, resfn);
                loc = (Hx(map, rh.rdatoff) + B3TOLONG(rr->doff)
                       + sizeof(Size) + (uint32_t)offset);
                err = SetFPos(rn, fsFromStart, loc);
                if(err == noErr)
                {
                    LONGINT lcount;

                    lcount = count;
                    err = FSReadAll(rn, &lcount, buffer);
                    if(err == noErr)
                        if(STARH(res))
                            err = resourceInMemory;
                }
            }
        }
    }
    ROMlib_setreserr(err);
}

P4(PUBLIC pascal trap, void, WritePartialResource,
   Handle, resource, int32_t, offset, Ptr, buffer, int32_t, count)
{
    warning_unimplemented(NULL_STRING);
}

P2(PUBLIC pascal trap, void, SetResourceSize,
   Handle, resource, int32_t, size)
{
    warning_unimplemented(NULL_STRING);
}

/* Word6 has this lovely code in it

0xe49a6:	beqs 0xe49ba
0xe49a8:	subqw #4,sp
0xe49aa:	movel fp@(12),sp@-
0xe49ae:	moveq #10,d0
0xe49b0:	movel sp,fp@
0xe49b2:	_ResourceDispatch
0xe49b4:	movel sp@+,fp@(16)
0xe49b8:	bras 0xe49be

**************************************/

P1(PUBLIC pascal trap, Handle, GetNextFOND, Handle, fondHandle)
{
    Handle retval;
    warning_unimplemented(NULL_STRING);

    retval = 0;
    return retval;
}

/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in DiskInit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "MemoryMgr.h"
#include "FileMgr.h"
#include "OSUtil.h"
#include "rsys/common.h"
#include "DiskInit.h"
#include "rsys/glue.h"
#include "mkvol/mkvol.h"
#include "rsys/hfs.h"
#include "rsys/blockinterrupts.h"

using namespace Executor;

P0(PUBLIC pascal trap, void, DILoad)
{
}

P0(PUBLIC pascal trap, void, DIUnload)
{
}

P2(PUBLIC pascal trap, INTEGER, DIBadMount, Point, pt, LONGINT, evtmess)
{
    return paramErr;
}

P1(PUBLIC pascal trap, OSErr, DIFormat, INTEGER, dn)
{
    return noErr; /* We don't do low-level formats right now */
}

enum
{
    FLOPPY_SIDES_PER_DISK = 2,
    FLOPPY_TRACKS_PER_SIDE = 80,
    FLOPPY_SECTORS_PER_TRACK = 18,
    FLOPPY_SECTORS_PER_DISK = FLOPPY_SIDES_PER_DISK * FLOPPY_TRACKS_PER_SIDE
        * FLOPPY_SECTORS_PER_TRACK,
};

PRIVATE OSErr
get_vref_dref(INTEGER rn, INTEGER *vrefp, INTEGER *drefp)
{
    OSErr retval;

    retval = noErr;
    *vrefp = rn;
    *drefp = OURHFSDREF;
    return retval;
}

typedef struct
{
    INTEGER vref;
    INTEGER dref;
    LONGINT pos;
} our_file_info_t;

typedef OSErrRET (*func_t)(ParmBlkPtr pb, BOOLEAN async);

PRIVATE OSErr
raw_read_write(func_t func, our_file_info_t *op, LONGINT *lengthp,
               char buf[])
{
    OSErr retval;
    ParamBlockRec pbr;

    check_virtual_interrupt();
    pbr.ioParam.ioVRefNum = CW(op->vref);
    pbr.ioParam.ioRefNum = CW(op->dref);
    pbr.ioParam.ioBuffer = RM((Ptr)buf);
    pbr.ioParam.ioReqCount = CL(*lengthp);
    pbr.ioParam.ioPosMode = CWC(fsFromStart);
    pbr.ioParam.ioPosOffset = CL(op->pos);
    retval = func(&pbr, false);
    if(retval == noErr)
    {
        *lengthp = CL(pbr.ioParam.ioActCount);
        op->pos += CL(pbr.ioParam.ioActCount);
    }
    return retval;
}

PRIVATE OSErr
raw_read(our_file_info_t *op, LONGINT *lengthp, char buf[])
{
    return raw_read_write(PBRead, op, lengthp, buf);
}

PRIVATE OSErr
raw_write(our_file_info_t *op, LONGINT *lengthp, char buf[])
{
    return raw_read_write(PBWrite, op, lengthp, buf);
}

enum
{
    N_TRACK_BYTES = (PHYSBSIZE * FLOPPY_SIDES_PER_DISK
                     * FLOPPY_SECTORS_PER_TRACK)
};

P1(PUBLIC pascal trap, OSErr, DIVerify, INTEGER, dn)
{
    int i;
    char buf[N_TRACK_BYTES];
    LONGINT length;
    OSErr err;
    our_file_info_t oi;

    err = get_vref_dref(dn, &oi.vref, &oi.dref);
    oi.pos = 0;
    if(err == noErr)
    {
        for(i = 0, err = noErr; err == noErr && i < FLOPPY_TRACKS_PER_SIDE; ++i)
        {
            length = sizeof(buf);
            err = raw_read(&oi, &length, buf);
            if(err == noErr && length != sizeof(buf))
                err = ioErr;
        }
    }

    return err;
}

/*
 * NOTE: This track at a time buffering will only work if there are no
 *       gaps in the data that is being written.  Currently, that's how
 *       we format floppies, but eventually we need something better here.
 */

PRIVATE Ptr track_bufp;
PRIVATE long offset;
PRIVATE long length;

PRIVATE OSErr
begin_track_buffering_for_write(void)
{
    OSErr retval;

    track_bufp = NewPtr(N_TRACK_BYTES);
    if(track_bufp)
    {
        retval = noErr;
        offset = 0;
        length = 0;
    }
    else
        retval = CW(MemErr);
    return retval;
}

PRIVATE OSErr
flush_buffer(our_file_info_t *ofitp)
{
    OSErr retval;
    LONGINT n_to_write;

    n_to_write = length;

    retval = raw_write(ofitp, &n_to_write, (char *)track_bufp);
    if(retval == noErr && n_to_write != length)
        retval = ioErr;
    else
    {
        length = 0;
        offset += n_to_write;
    }
    return retval;
}

PRIVATE size_t
writefunc(int magic, const void *buf, size_t buf_len)
{
    OSErr err = noErr;
    our_file_info_t *ofip;
    char *bufp;
    size_t buf_len_remaining;

    buf_len_remaining = buf_len;
    ofip = (our_file_info_t *)magic;
    if(ofip->pos != offset)
        warning_unexpected("ofip->pos = %d, offset = %ld\n", ofip->pos, offset);

    bufp = (char *)buf;
    while(err == noErr && buf_len_remaining > 0)
    {
        uint32 n_bytes_left, n_to_copy;

        n_bytes_left = N_TRACK_BYTES - length;
        n_to_copy = MIN(n_bytes_left, buf_len_remaining);
        memcpy(track_bufp + length, bufp, n_to_copy);
        length += n_to_copy;
        if(length == N_TRACK_BYTES)
            err = flush_buffer(ofip);
        buf_len_remaining -= n_to_copy;
    }
    return err ? 0 : buf_len;
}

PRIVATE OSErr
end_track_buffering_for_write(our_file_info_t *ofitp)
{
    OSErr retval;

    if(length)
        retval = flush_buffer(ofitp);
    else
        retval = noErr;
    DisposPtr(track_bufp);
    track_bufp = 0;
    return retval;
}

/*
 * Hacky -- assumes 1.4 MB, but it's a start
 */

P2(PUBLIC pascal trap, OSErr, DIZero, INTEGER, dn, StringPtr, vname)
{
    OSErr err;
    GUEST<ULONGINT> time;
    int name_len;
    char *name;
    our_file_info_t oi;

    name_len = vname[0];
    name = (char *)alloca(name_len + 1);
    memcpy(name, vname + 1, name_len);
    name[name_len] = 0;
    GetDateTime(&time);

    err = get_vref_dref(dn, &oi.vref, &oi.dref);
    if(err == noErr)
    {
        oi.pos = 0;
        err = begin_track_buffering_for_write();
        if(err == noErr)
        {
            OSErr err2;

// FIXME: #warning disk init unsupported
            //err = format_disk(time, name, FLOPPY_SECTORS_PER_DISK, writefunc,
            //		    (int) &oi);
            err2 = end_track_buffering_for_write(&oi);
            if(err == noErr)
                err = err2;
        }
    }
    return err;
}

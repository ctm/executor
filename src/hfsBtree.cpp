/*
 * Some modifications we need to make to go from HFS to HFS+:
 *
 * Number of allocation blocks is now 32 bits
 * Filenames can now be up to 255 characters and are unicode
 * The meta information (FInfo, FXinfo) is now more generic
 * System FolderID has been changed (don't recall if we currently support this)
 * Catalog node size is no longer fixed at 512 bytes "typically" 4kb
 * Maximum file size is 2^63 bytes
 *
 * The "Attributes file" is totally new to HFS+
 * The "Startup file" is totally new, but probably fits into everything else
 * fairly easily
 *
 * There is an "alternate volume header"
 */

/*
 * NOTES: They use 32-bit allocation block numbers, an allocation block is
 *        a power of 2 >= 512.  512 is the logical block size, but most
 *        things are probably done in terms of allocation blocks
 *
 * TECHNOTE CONCERNING ALLOCATION:
    ... "Space is not allocated in contiguous clump-sized pieces"
    (this could be outdated)
    ... "Special files ... only have a data fork ... and it's described in
    the volume header" HOWEVER, the tech note later elaborates that the
    special files may spill over into the extents file (although the
    extents file itself can't)

    it looks like a virtual bad-block file is maintained inside the extents
    catalog

    unicode, but still case insensitive... woo hoo

    encodingsBitmap is a 64-bit value of all the different encodings that
    have been seen on a particular volume.  It isn't necessary to clear a
    bit when the last entry is deleted ... since it's only 64 bits, they
    will run out of bits... oh yes, they've already done so... woo hoo

    Most dates are stored in GMT.  The exception is the volume creation date,
    which is stored in local.  That's because they don't want the time shifting
    around as the timezone is changed.  Seems like a crock, but there you
    go

 */

/*
 * Implementation steps:
 *
 *   Recognize HFS+ signature, and make sure that it can only be opened
 *   read-only
 *
 *
 */

/* Copyright 1992-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"
#include "rsys/hfs.h"
#include "rsys/dcache.h"

#include "rsys/hfs_plus.h"

using namespace Executor;

#if 0
ULONGINT blockchecksum(void *blockp)
{
    ULONGINT retval, *ulp;
    INTEGER i;
    
    for (i = 128, retval = 0, ulp = blockp; --i >= 0;)
	retval ^= *ulp++;
    return retval;
}

static void checkcache(short refnum)
{
    cacheentry *cachep;
    cachehead *headp;
    HVCB *vcbp;
    filecontrolblock *fcbp;
    INTEGER i;
    static INTEGER count = 0;
    
    if (++count > 3)
	return;
    
    printf("\n");
    fcbp = (filecontrolblock *)((char *)CL(FCBSPtr) + refnum);
    vcbp = CL(fcbp->fcbVPtr);
    headp = (cachehead *) CL(vcbp->vcbCtlBuf);
    printf("headp = 0x%lx, nitems = %d, flink = 0x%lx, blink = 0x%lx\n",
		headp, CW(headp->nitems), CL(headp->flink), CL(headp->blink));
    for (i = CW(headp->nitems), cachep = CL(headp->flink); --i >= -3;
						   cachep = CL(cachep->flink))
	printf("0x%lx:0x%x ", cachep, cachep->flags);
    printf("\n\n");
    for (i = CW(headp->nitems), cachep = CL(headp->blink); --i >= -3;
						 cachep = CL(cachep->blink))
	printf("0x%lx ", cachep);
    printf("\n");
}
#endif

cacheentry *Executor::ROMlib_addrtocachep(Ptr addr, HVCB *vcbp)
{
    cachehead *headp;
    cacheentry *retval;
    INTEGER i;

    headp = (cachehead *)MR(vcbp->vcbCtlBuf);
    for(i = CW(headp->nitems), retval = MR(headp->flink); --i >= 0 && (addr < (Ptr)retval || addr > (Ptr)retval + sizeof(cacheentry));
        retval = MR(retval->flink))
        ;
    return i >= 0 ? retval : 0;
}

#define BTENTRY(btp, n) \
    ((anykey *)((char *)(btp) + CW(((GUEST<int16_t> *)((char *)(btp) + PHYSBSIZE - sizeof(short)))[-(n)])))

#define BTOFFSET(btp, n) \
    ((GUEST<int16_t> *)((char *)(btp) + PHYSBSIZE - sizeof(short)) - (n))

#define EVENUP(x) (((x) + 1) / 2 * 2)

/*
 * ROMlib_errortype returns dirNFErr or fnfErr depending on whether the cause
 * of failure was a missing directory or a missing file.  This winds up
 * being very important, because our file creation routines need to know
 * whether or not a given directory is present and if so, whether or not
 * the file is already there.
 */

OSErr Executor::ROMlib_errortype(btparam *btpb)
{
    trailentry *tep;
    btnode *btp;
    catkey *catkeyp;
    INTEGER index;
    OSErr retval;

    tep = btpb->trail + btpb->leafindex;
    btp = (btnode *)tep->cachep->buf;
    index = (short)tep->after < 0 ? 0 : tep->after;
    catkeyp = (catkey *)BTENTRY(btp, index);
    retval = catkeyp->ckrParID == btpb->tofind.catk.ckrParID
        ? fnfErr
        : dirNFErr;
    if(retval == dirNFErr)
        warning_trace_info("catkeyp->ckrParID = %d, "
                           "btpb->tofind.catk.ckrParID = %d",
                           CL(catkeyp->ckrParID),
                           CL(btpb->tofind.catk.ckrParID));
    fs_err_hook(retval);
    return retval;
}

/*
 * The test code below assumes that a catalog file is being modified.
 * You can't have CATFILEDEBUG turned on during normal use because as soon
 * as an extents file has to be modified you'll get complaints related to
 * the keysize not being what this code expects.
 */

/* #define CATFILEDEBUG */

#if defined(CATFILEDEBUG)

static void checkbtp(btnode *btp)
{
    ULONGINT flink, blink;
    short *offsetp, expected;
    INTEGER i;
    char keylen;

    flink = CL(btp->ndFLink);
    blink = CL(btp->ndBLink);
    switch(btp->ndType)
    {
        case indexnode:
            if(btp->ndLevel > 5)
                warning_unexpected("level(%d) > 5 on indexnode", btp->ndLevel);
            offsetp = BTOFFSET(btp, 0);
            expected = sizeof(btnode);
            for(i = CW(btp->ndNRecs) + 1; --i >= 0; --offsetp)
            {
                if(CW(*offsetp) != expected)
                    if(CW(*offsetp) < expected)
                        warning_unexpected("unexpected offset");
                    else
                        warning_unexpected("curiously large offset");
                if(i > 0)
                {
                    if(*((char *)btp + expected) != 37)
                        warning_unexpected("unexpected keylen");
                    expected += 38 + sizeof(LONGINT);
                }
            }
            break;
        case leafnode:
#if 0
	if (flink > 100 || blink > 100)         /* could do more checking */
	    warning_unexpected ("flink or blink > 100");
#endif
            if(btp->ndLevel != 1)
                warning_unexpected("level != 1 on leafnode");
            offsetp = BTOFFSET(btp, 0);
            expected = sizeof(btnode);
            for(i = CW(btp->ndNRecs) + 1; --i >= 0; --offsetp)
            {
                if(CW(*offsetp) != expected)
                    if(CW(*offsetp) < expected)
                        warning_unexpected("unexpected offset");
                    else
                        warning_unexpected("curiously large offset\n");
                if(i > 0)
                {
                    if((keylen = *((char *)btp + expected)) > 37)
                        warning_unexpected("unexpected keylen");
                    expected += EVENUP(keylen + 1);
                    switch(*((char *)btp + expected))
                    {
                        case DIRTYPE:
                            expected += sizeof(directoryrec);
                            break;
                        case FILETYPE:
                            expected += sizeof(filerec);
                            break;
                        case THREADTYPE:
                            expected += sizeof(threadrec);
                            break;
                        default:
                            warning_unexpected("unexpected type");
                            break;
                    }
                }
            }
            break;
        default:
            warning_unexpected("unknown node type");
            return;
            break;
    }
}
#endif /* CATFILEDEBUG */

BOOLEAN Executor::ROMlib_searchnode(btnode *btp, void *key, compfp fp,
                                    anykey **keypp, INTEGER *afterp)
{
    INTEGER low, high, mid;
    anykey *totest, *totest2;

#if defined(CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    high = CW(btp->ndNRecs) - 1;
    totest = BTENTRY(btp, high); /* test last one by hand then use as sentinel */
    switch((*fp)(key, totest))
    {
        case firstisless:
            low = 0;
            for(;;)
            {
                mid = (low + high) / 2;
                totest = BTENTRY(btp, mid);
                switch((*fp)(key, totest))
                {
                    case firstisless:
                        if(mid == 0)
                        {
                            *keypp = totest;
                            *afterp = -1;
                            return false;
                        }
                        high = mid;
                        break;
                    case same:
                        *keypp = totest;
                        *afterp = mid;
                        return true;
                    case firstisgreater:
                        totest2 = BTENTRY(btp, mid + 1);
                        switch((*fp)(key, totest2))
                        {
                            case firstisless:
                                *keypp = totest;
                                *afterp = mid;
                                return false;
                            case same:
                                *keypp = totest2;
                                *afterp = mid + 1;
                                return true;
                            case firstisgreater:
                                low = mid + 1;
                                break;
                        }
                }
            }
        case same:
            *keypp = totest;
            *afterp = high;
            return true;
        case firstisgreater:
            *keypp = totest;
            *afterp = high;
            return false;
    }
#if !defined(LETGCCWAIL)
    return false;
#endif
}

static void makefirst(cachehead *headp, cacheentry *entryp)
{
    if(MR(headp->flink) != entryp)
    {
        MR(entryp->blink)->flink = entryp->flink; /* remove link */
        MR(entryp->flink)->blink = entryp->blink;

        entryp->flink = headp->flink;
        entryp->blink = MR(headp->flink)->blink;

        MR(headp->flink)->blink = RM(entryp);
        headp->flink = RM(entryp);
    }
}

OSErr Executor::ROMlib_putcache(cacheentry *cachep)
{
    OSErr err;
    HVCB *vcbp;

    err = noErr;

    vcbp = MR(cachep->vptr);
    if((cachep->flags & (CACHEDIRTY | CACHEFREE)) == CACHEDIRTY)
    {
#if 0
	BufTgFNum = cachep->fileno;
	BufTgFFlag = cachep->forktype == datafork ? 0 : 2;
	BufTgFBkNum = cachep->logblk;
	BufTgDate = Time;
#endif
        err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                  CL(cachep->physblock) * PHYSBSIZE, 1,
                                  (Ptr)cachep->buf, writing, (GUEST<LONGINT> *)0);
        vcbsync(vcbp);
    }
    if(cachep->flags & CACHEFREE)
        warning_unexpected("cache free");
    cachep->flags &= ~CACHEDIRTY;

    if(err == noErr)
        err = dcache_flush(((VCBExtra *)vcbp)->u.hfs.fd) ? (OSErr)noErr : (OSErr)ioErr;
    fs_err_hook(err);
    return err;
}

PUBLIC LONGINT tagfnum;
PUBLIC INTEGER tagflag;
PUBLIC INTEGER tagbknm;
PUBLIC LONGINT tagdate;
PUBLIC LONGINT tagtfs0;
PUBLIC LONGINT tagtfs1;

PRIVATE BOOLEAN ROMlib_index_cached = false;

OSErr Executor::ROMlib_getcache(cacheentry **retpp, uint16_t refnum,
                                ULONGINT logbno, cacheflagtype flags)
{
    cacheentry *retval, *lastp, *lastdirtyp, *lastfreep;
    cachehead *headp;
    HVCB *vcbp;
    filecontrolblock *fcbp;
    INTEGER count;
    LONGINT nphyscontig;
    OSErr err;
    ULONGINT physbyte;
    LONGINT filenum;
    Forktype forkwanted;
#if 1
    INTEGER badnesscount;
#endif

    ROMlib_index_cached = false;
    fcbp = (filecontrolblock *)((char *)MR(FCBSPtr) + refnum);
    vcbp = MR(fcbp->fcbVPtr);
    filenum = CL(fcbp->fcbFlNum);
    forkwanted = fcbp->fcbMdRByt & RESOURCEBIT ? resourcefork : datafork;
    headp = (cachehead *)MR(vcbp->vcbCtlBuf);

    count = CW(headp->nitems);
    lastp = 0;
    lastdirtyp = 0;
    lastfreep = 0;
#if 1
    badnesscount = 0;
#endif
    for(retval = MR(headp->flink); --count >= 0 && (CL(retval->logblk) != logbno || CW(retval->refnum) != refnum || MR(retval->vptr) != vcbp || CL(retval->fileno) != filenum || retval->forktype != forkwanted);
        retval = MR(retval->flink))
    {
        if(!(retval->flags & CACHEBUSY))
        {
            if(retval->flags & CACHEDIRTY)
            {
                if(MR(retval->vptr) == vcbp) /* TODO: take vptr == vcbp out */
                    lastdirtyp = retval;
#if 1
                else
                    badnesscount++;
#endif
            }
            else
                lastp = retval;
        }
        if(retval->flags & CACHEFREE)
            lastfreep = retval;
    }
    if(count < 0)
    {
        if(lastfreep)
            retval = lastfreep;
        else if(lastp)
            retval = lastp;
        else if(lastdirtyp)
        {
            retval = lastdirtyp;
            err = ROMlib_putcache(retval);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
        }
        else
        {
            warning_unexpected("all cache busy");
            err = fsDSIntErr;
            fs_err_hook(err);
            return err;
        }
    }
    makefirst(headp, retval);
    if(count < 0)
    {
        retval->vptr = RM(vcbp);
        retval->fileno = CL(filenum);
        retval->refnum = CW(refnum);
        retval->logblk = CL(logbno);
        retval->flags = CACHEBUSY;
        retval->forktype = forkwanted;

        physbyte = ROMlib_logtophys(fcbp, logbno * PHYSBSIZE, &nphyscontig);
        if(nphyscontig < 1)
        {
            warning_unexpected("nphyscontig < 1");
            err = fsDSIntErr;
            fs_err_hook(err);
            return err;
        }
        retval->physblock = CL(physbyte / PHYSBSIZE);
        if(!(flags & GETCACHENOREAD))
            err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs, physbyte, 1,
                                      (Ptr)retval->buf, reading,
                                      (GUEST<LONGINT> *)0);
        else
            err = noErr;
        retval->flags = 0;
    }
    else
        err = noErr;
    if(flags & GETCACHESAVE)
        retval->flags |= CACHEBUSY;
    retval->flags &= ~CACHEFREE;
    *retpp = retval;
    fs_err_hook(err);
    return err;
}

#if defined(CATFILEDEBUG)
void Executor::ROMlib_checkleaves(INTEGER refnum)
{
    OSErr err;
    cacheentry *block0cachep, *cachep;
    btblock0 *block0p;
    ULONGINT node, expectedblink;
    btnode *btp;

    err = ROMlib_getcache(&block0cachep, refnum, (ULONGINT)0, GETCACHESAVE);
    if(err != noErr)
        warning_unexpected("getcache error");
    block0p = (btblock0 *)block0cachep->buf;
    node = CL(block0p->firstleaf);
    expectedblink = 0;
    while(node != 0)
    {
        err = ROMlib_getcache(&cachep, refnum, node, 0);
        if(err != noErr)
            warning_unexpected("getcache error");
        btp = (btnode *)cachep->buf;
#if defined(CATFILEDEBUG)
        checkbtp(btp);
#endif /* CATFILEDEBUG */
        if(CL(btp->ndBLink) != expectedblink)
            warning_unexpected("bad blink");
        expectedblink = node;
        node = CL(btp->ndFLink);
    }
    if(CL(block0p->lastleaf) != expectedblink)
        warning_unexpected("bad block0p->blink");
}
#endif /* CATFILEDEBUG */

OSErr Executor::ROMlib_cleancache(HVCB *vcbp)
{
    INTEGER i;
    cachehead *headp;
    cacheentry *cachep;
    OSErr err;

    headp = (cachehead *)MR(vcbp->vcbCtlBuf);
    err = noErr;
    for(i = CW(headp->nitems), cachep = (cacheentry *)(headp + 1); --i >= 0;
        ++cachep)
    {
        if(MR(cachep->vptr) == vcbp)
            cachep->flags &= ~CACHEBUSY;
    }
    fs_err_hook(err);
    return err;
}

OSErr Executor::ROMlib_flushcachevcbp(HVCB *vcbp)
{
    INTEGER i;
    cachehead *headp;
    cacheentry *cachep;
    OSErr err;

    headp = (cachehead *)MR(vcbp->vcbCtlBuf);
    err = noErr;
    if(headp)
    {
        for(i = CW(headp->nitems), cachep = (cacheentry *)(headp + 1);
            --i >= 0; ++cachep)
        {
            if(MR(cachep->vptr) == vcbp && (cachep->flags & CACHEDIRTY))
            {
                OSErr err2;

                err2 = ROMlib_putcache(cachep);
                if(err == noErr && err2 != noErr)
                    err = err2;
            }
        }
        fs_err_hook(err);
    }
    if(err == noErr)
        err = dcache_flush(((VCBExtra *)vcbp)->u.hfs.fd) ? noErr : ioErr;

    return err;
}

/*
 * NOTE: an important side effect of ROMlib_keyfind is that the
 * first and last node have the CACHEBUSY bit set.
 */

OSErr Executor::ROMlib_keyfind(btparam *btpb)
{
    cacheentry *cachep;
    OSErr err;
    LONGINT node;
    BOOLEAN found;
    unsigned char type;
    trailentry *tep;

    tep = btpb->trail;
    err = ROMlib_getcache(&cachep, btpb->refnum, (ULONGINT)0, GETCACHESAVE);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    if(((btblock0 *)cachep->buf)->numentries == CLC(0))
    {
        btpb->foundp = 0;
        btpb->success = false;
        btpb->leafindex = 0;
        return noErr;
    }
    node = CL(((btblock0 *)cachep->buf)->root);
    tep->logbno = 0;
    tep++->cachep = cachep;
#if !defined(LETGCCWAIL)
    type = 0;
#endif /* LETGCCWAIL */
    for(;;)
    {
        err = ROMlib_getcache(&cachep, btpb->refnum, node, (cacheflagtype)0);
        if(err == noErr)
        {
            tep->logbno = node;
            tep->cachep = cachep;
            type = ((btnode *)cachep->buf)->ndType;
        }
        if(err != noErr || (type != indexnode && type != leafnode))
        {
            btpb->success = false;
            if(err == noErr)
            {
                warning_unexpected("unknown node");
                err = fsDSIntErr;
            }
            fs_err_hook(err);
            return err;
        }
        found = ROMlib_searchnode((btnode *)cachep->buf, &btpb->tofind,
                                  btpb->fp, &btpb->foundp, (INTEGER *)&tep->after);
        if(type == indexnode)
            node = CL(*(GUEST<LONGINT> *)DATAPFROMKEY(btpb->foundp));
        else
        {
            btpb->leafindex = tep - btpb->trail;
            btpb->success = found;
            tep->cachep->flags |= CACHEBUSY;
            return noErr;
        }
        ++tep;
    }
}

/*
 * NOTE: ROMlib_btnext could be made to use ROMlib_searchnode if we passed in
 *	 a comparator.
 *       Currently I'm not concerned with "speed" since disk i/o overwhelms
 *       computation..
 */

OSErr Executor::ROMlib_btnext(anykey **nextpp, anykey *keyp, HVCB *vcbp)
{
    cacheentry *cachep;
    btnode *btp;
    INTEGER i;
    anykey *retval;
    LONGINT node;
    OSErr err;

    cachep = ROMlib_addrtocachep((Ptr)keyp, vcbp);
    btp = (btnode *)cachep->buf;
    for(i = CW(btp->ndNRecs); --i >= 0 && BTENTRY(btp, i) != keyp;)
        ;
    if(i < 0)
        retval = 0;
    else if(i < CW(btp->ndNRecs) - 1)
        retval = BTENTRY(btp, i + 1);
    else if((node = CL(btp->ndFLink)))
    {
        err = ROMlib_getcache(&cachep, CW(cachep->refnum), node, (cacheflagtype)0);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        btp = (btnode *)cachep->buf;
        retval = BTENTRY(btp, 0);
    }
    else
        retval = 0;
    *nextpp = retval;
    return noErr;
}

/*
 * The first map is part of block 0, successive maps have blocks of their
 * own, but in each case, the map is the last entry in the reverse offset
 * index at the end of a node.
 */

static OSErr MapBitSetOrClr(cacheentry *block0cachep, LONGINT bit, BOOLEAN set)
{
    INTEGER refnum;
    btblock0 *block0p;
    LONGINT nnodes;
    btnode *btp;
    unsigned char *mapstart, *mapend;
    cacheentry *cachep;
    BOOLEAN done;
    INTEGER nrecs, nmapnodes;
    OSErr err;

    refnum = CW(block0cachep->refnum);
    cachep = block0cachep;
    block0p = (btblock0 *)block0cachep->buf;
    nnodes = CL(block0p->nnodes);
    gui_assert(bit < nnodes);
    btp = (btnode *)block0p;
    done = false;
    do
    {
        nrecs = CW(btp->ndNRecs);
        mapstart = (unsigned char *)BTENTRY(btp, nrecs - 1);
        mapend = (unsigned char *)BTENTRY(btp, nrecs);
        nmapnodes = (mapend - mapstart) * 8;
        if(bit < nmapnodes)
        {
            if(set)
                BitSet((Ptr)mapstart, bit);
            else
                BitClr((Ptr)mapstart, bit);
            cachep->flags |= CACHEDIRTY;
            done = true;
        }
        else
        {
            if(btp->ndFLink)
            {
                bit -= nmapnodes;
                err = ROMlib_getcache(&cachep, refnum, CL(btp->ndFLink), (cacheflagtype)0);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    /*-->*/ return err;
                }
                btp = (btnode *)cachep->buf;
            }
            else
            {
                err = fsDSIntErr;
                fs_err_hook(err);
                /*-->*/ return err;
            }
        }
    } while(!done);
    return noErr;
}

enum
{
    MAP_PAGE_MAP_BEGIN = 0xE,
    MAP_PAGE_MAP_END = 0x1FA
};

static OSErr add_free_nodes(cacheentry *block0cachep, ULONGINT n_new_nodes)
{
    btblock0 *block0p;
    LONGINT first_free_node;
    ULONGINT nnodes, nmapnodes;
    btnode *btp, *newbtp;
    unsigned char *mapstart, *mapend;
    cacheentry *newcachep;
    BOOLEAN done;
    INTEGER refnum;
    INTEGER nrecs;
    OSErr err;
    cacheentry *oldcachep;

    refnum = CW(block0cachep->refnum);
    block0p = (btblock0 *)block0cachep->buf;
    nnodes = CL(block0p->nnodes);
    first_free_node = nnodes;
    btp = (btnode *)block0p;
    oldcachep = block0cachep;
    done = false;
    block0p->nnodes = CL(nnodes + n_new_nodes);
    do
    {
        nrecs = CW(btp->ndNRecs);
        mapstart = (unsigned char *)BTENTRY(btp, nrecs - 1);
        mapend = (unsigned char *)BTENTRY(btp, nrecs);
        nmapnodes = (mapend - mapstart) * 8;
        if(nnodes + n_new_nodes > nmapnodes)
        {
            if(btp->ndFLink)
            {
                nnodes -= nmapnodes;
                err = ROMlib_getcache(&newcachep, refnum, CL(btp->ndFLink), (cacheflagtype)0);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    /*-->*/ return err;
                }
                btp = (btnode *)newcachep->buf;
                oldcachep = newcachep;
            }
            else
            {
                err = ROMlib_getcache(&newcachep, refnum, first_free_node,
                                      GETCACHENOREAD);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    /*-->*/ return err;
                }
                newbtp = (btnode *)newcachep->buf;
                btp->ndFLink = CL(first_free_node);
                memset((char *)newbtp, 0, PHYSBSIZE);
                newbtp->ndType = CB(mapnode); /* 2 */
                newbtp->ndNRecs = CWC(1);

                *((GUEST<INTEGER> *)newbtp + 255) = CWC((short)MAP_PAGE_MAP_BEGIN);
                *((GUEST<INTEGER> *)newbtp + 254) = CWC((short)MAP_PAGE_MAP_END);
                --n_new_nodes;
                oldcachep->flags |= CACHEDIRTY;
                newcachep->flags |= CACHEDIRTY;
                MapBitSetOrClr(block0cachep, first_free_node, true);
                done = true;
            }
        }
        else
            done = true;
    } while(!done);
    block0p->nfreenodes = CL(CL(block0p->nfreenodes) + n_new_nodes);
    return noErr;
}

static ULONGINT findfirstzero(unsigned char *cp)
{
    ULONGINT retval;
    unsigned char c, bit;

    retval = 0;
    while(*cp++ == 0xFF)
        retval += 8;
    c = cp[-1];
    for(bit = 0x80; bit & c; bit >>= 1)
        ++retval;
    return retval;
}

static OSErr MapFindFirstBitAndSet(cacheentry *block0cachep,
                                   ULONGINT *newblockpbit)
{
    INTEGER refnum;
    btblock0 *block0p;
    LONGINT nnodes;
    btnode *btp;
    unsigned char *mapstart, *mapend;
    cacheentry *cachep;
    BOOLEAN done;
    INTEGER nrecs;
    ULONGINT nmapnodes;
    OSErr err;
    ULONGINT retval;
    ULONGINT n;

    refnum = CW(block0cachep->refnum);
    cachep = block0cachep;
    block0p = (btblock0 *)block0cachep->buf;
    nnodes = CL(block0p->nnodes);
    btp = (btnode *)block0p;
    done = false;
    retval = 0;
    do
    {
        nrecs = CW(btp->ndNRecs);
        mapstart = (unsigned char *)BTENTRY(btp, nrecs - 1);
        mapend = (unsigned char *)BTENTRY(btp, nrecs);
        nmapnodes = (mapend - mapstart) * 8;
        n = findfirstzero(mapstart);
        if(n < nmapnodes)
        {
            BitSet((Ptr)mapstart, n);
            cachep->flags |= CACHEDIRTY;
            retval += n;
            done = true;
        }
        else
        {
            if(btp->ndFLink)
            {
                retval += nmapnodes;
                err = ROMlib_getcache(&cachep, refnum, CL(btp->ndFLink), (cacheflagtype)0);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    /*-->*/ return err;
                }
                btp = (btnode *)cachep->buf;
            }
            else
            {
                err = fsDSIntErr;
                fs_err_hook(err);
                /*-->*/ return err;
            }
        }
    } while(!done);
    *newblockpbit = retval;
    return noErr;
}

static OSErr deletenode(cacheentry *todeletep)
{
    cacheentry *block0cachep, *linkcachep;
    btblock0 *block0p;
    btnode *btp, *linkbtp;
    ULONGINT node, flink, blink;
    OSErr err;
    INTEGER refnum;

    refnum = CW(todeletep->refnum);
    err = ROMlib_getcache(&block0cachep, refnum, 0L, GETCACHESAVE);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    block0p = (btblock0 *)block0cachep->buf;
    btp = (btnode *)todeletep->buf;
#if defined(CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    node = CL(todeletep->logblk);
#if 0
    if (btp->ndType == leafnode) {
#endif
    flink = CL(btp->ndFLink);
    blink = CL(btp->ndBLink);
    if(CL(block0p->firstleaf) == node)
        block0p->firstleaf = CL(flink);
    if(CL(block0p->lastleaf) == node)
        block0p->lastleaf = CL(blink);
    if(blink)
    {
        err = ROMlib_getcache(&linkcachep, refnum, blink, (cacheflagtype)0);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        linkbtp = (btnode *)linkcachep->buf;
        linkbtp->ndFLink = CL(flink);
        linkcachep->flags |= CACHEDIRTY;
    }
    if(flink)
    {
        err = ROMlib_getcache(&linkcachep, refnum, flink, (cacheflagtype)0);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        linkbtp = (btnode *)linkcachep->buf;
        linkbtp->ndBLink = CL(blink);
        linkcachep->flags |= CACHEDIRTY;
    }
#if 0
    }
#endif
    block0p->nfreenodes = CL(CL(block0p->nfreenodes) + 1);
    MapBitSetOrClr(block0cachep, node, false);
    block0cachep->flags |= CACHEDIRTY;
    memset(todeletep->buf, 0, PHYSBSIZE);
    todeletep->flags |= CACHEDIRTY;
    return noErr;
}

typedef enum { leavealone,
               doleft,
               doright } whichnodetype;

#define FREESIZE(btp) \
    (((char *)(btp) + PHYSBSIZE - (CW((btp)->ndNRecs) + 1) * sizeof(short)) - (char *)BTENTRY((btp), CW((btp)->ndNRecs)))

#define SIZECUTOFF ((PHYSBSIZE - (int)sizeof(btnode)) / 2)

/*
 * NOTE: the code for merge is very similar to the code for shuffle right to
 *       left.  If you find a bug here, look for a corresponding one below.
 */

static OSErr merge(cacheentry *leftp, cacheentry *rightp)
{
    INTEGER n, nrecs, datasize, i;
    btnode *leftbtp, *rightbtp;
    char *datastart, *datastop;
    GUEST<INTEGER> *offsetp;
    OSErr retval;

    leftbtp = (btnode *)leftp->buf;
    rightbtp = (btnode *)rightp->buf;
#if defined(CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    nrecs = CW(rightbtp->ndNRecs);
    datastart = (char *)BTENTRY(rightbtp, 0);
    datastop = (char *)BTENTRY(rightbtp, nrecs);
    datasize = datastop - datastart;

    memmove(BTENTRY(leftbtp, CW(leftbtp->ndNRecs)), datastart, datasize);

    offsetp = BTOFFSET(leftbtp, CW(leftbtp->ndNRecs));
    n = 0;
    for(i = nrecs, n = 0; --i >= 0; ++n)
    {
        offsetp[-1] = CW(CW(offsetp[0]) + (char *)BTENTRY(rightbtp, n + 1) - (char *)BTENTRY(rightbtp, n));
        --offsetp;
    }
    leftbtp->ndNRecs = CW(CW(leftbtp->ndNRecs) + (nrecs));
    if(!(rightp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    if(!(leftp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    leftp->flags |= CACHEDIRTY;
#if defined(CATFILEDEBUG)
    checkbtp(leftbtp);
#endif
    retval = deletenode(rightp);
    fs_err_hook(retval);
    return retval;
}

static OSErr shuffle(cacheentry *leftp, cacheentry *rightp)
{
    btnode *leftbtp, *rightbtp;
    INTEGER leftfreesize, rightfreesize, numtocopy, n, recsize,
        rightdatasize, bytestoshift, i;
    char *rightbtentry0, *recstart, *recend;
    GUEST<INTEGER> *offsetp;

    leftbtp = (btnode *)leftp->buf;
    rightbtp = (btnode *)rightp->buf;
#if defined(CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    leftfreesize = FREESIZE(leftbtp);
    rightfreesize = FREESIZE(rightbtp);
    numtocopy = 0;
    bytestoshift = 0;
    rightbtentry0 = (char *)BTENTRY(rightbtp, 0);
#if !defined(LETGCCWAIL)
    recstart = recend = 0;
#endif /* LETGCCWAIL */
    if(leftfreesize < rightfreesize)
    {
        /* copy from left to right; almost the same code as below */
        /* NOTE:  if you find a bug here, look for a similar bug below */
        n = CW(leftbtp->ndNRecs) - 1;
        while(rightfreesize > SIZECUTOFF)
        {
            numtocopy++;
            recstart = (char *)BTENTRY(leftbtp, n);
            recend = (char *)BTENTRY(leftbtp, n + 1);
            recsize = recend - recstart;
            bytestoshift += recsize;
            rightfreesize -= sizeof(INTEGER) + recsize;
            --n;
        }
        rightdatasize = (char *)BTENTRY(rightbtp, CW(rightbtp->ndNRecs)) - rightbtentry0;
        memmove(rightbtentry0 + bytestoshift, rightbtentry0, rightdatasize);
        memmove(rightbtentry0, recstart, bytestoshift);

        offsetp = BTOFFSET(rightbtp, CW(rightbtp->ndNRecs) + numtocopy);
        for(i = CW(rightbtp->ndNRecs); --i >= 0;)
        {
            offsetp[0] = CW(CW(offsetp[numtocopy]) + bytestoshift);
            ++offsetp;
        }

        offsetp = BTOFFSET(rightbtp, 0);
        ++n;
        for(i = numtocopy; --i >= 0;)
        {
            offsetp[-1] = CW(CW(offsetp[0]) + (char *)BTENTRY(leftbtp, n + 1) - (char *)BTENTRY(leftbtp, n));
            --offsetp;
            ++n;
        }

        leftbtp->ndNRecs = CW(CW(leftbtp->ndNRecs) - (numtocopy));
        rightbtp->ndNRecs = CW(CW(rightbtp->ndNRecs) + (numtocopy));
    }
    else
    {
        /* copy from right to left; almost the same code as above */
        /* NOTE:  if you find a bug here, look for a similar bug above */
        n = 0;
        while(leftfreesize > SIZECUTOFF)
        {
            numtocopy++;
            recstart = (char *)BTENTRY(rightbtp, n);
            recend = (char *)BTENTRY(rightbtp, n + 1);
            recsize = recend - recstart;
            bytestoshift += recsize;
            leftfreesize -= sizeof(INTEGER) + recsize;
            ++n;
        }
        rightdatasize = (char *)BTENTRY(rightbtp, CW(rightbtp->ndNRecs)) - recend;
        memmove(BTENTRY(leftbtp, CW(leftbtp->ndNRecs)),
                rightbtentry0, bytestoshift);
        memmove(rightbtentry0, recend, rightdatasize);

        offsetp = BTOFFSET(leftbtp, CW(leftbtp->ndNRecs));
        n = 0;
        for(i = numtocopy; --i >= 0;)
        {
            offsetp[-1] = CW(CW(offsetp[0]) + (char *)BTENTRY(rightbtp, n + 1) - (char *)BTENTRY(rightbtp, n));
            --offsetp;
            ++n;
        }

        offsetp = BTOFFSET(rightbtp, 1);
        for(i = CW(rightbtp->ndNRecs) - numtocopy; --i >= 0;)
        {
            offsetp[0] = CW(CW(offsetp[-numtocopy]) - bytestoshift);
            --offsetp;
        }

        rightbtp->ndNRecs = CW(CW(rightbtp->ndNRecs) - (numtocopy));
        leftbtp->ndNRecs = CW(CW(leftbtp->ndNRecs) + (numtocopy));
    }
#if defined(CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    if(!(rightp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    if(!(leftp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    leftp->flags |= CACHEDIRTY;
    rightp->flags |= CACHEDIRTY;
    return noErr;
}

static OSErr btsetkey(cacheentry *cachep, INTEGER index, anykey *srckeyp)
{
    anykey *dstkeyp;

#if defined(CATFILEDEBUG)
    checkbtp((btnode *)cachep->buf);
#endif /* CATFILEDEBUG */
    dstkeyp = BTENTRY((btnode *)cachep->buf, index);

    /* NOTE: dstkeyp->keylen is not a bug; it shouldn't be srckeyp->keylen.
	     btsetkey is used only to set a parent's key from a child's key,
	     and the rule is that parent's keys are always a fixed length
	     that are never smaller than children's keys */

    memmove((char *)dstkeyp + 1, (char *)srckeyp + 1, dstkeyp->keylen);
    if(!(cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    cachep->flags |= CACHEDIRTY;
#if defined(CATFILEDEBUG)
    checkbtp((btnode *)cachep->buf);
#endif /* CATFILEDEBUG */
    return noErr;
}

static OSErr pullout(cacheentry *selfcachep, INTEGER selfindex,
                     cacheentry *parentcachep, INTEGER parentindex,
                     INTEGER *todeletep)
{
    cacheentry *leftcachep = NULL, *rightcachep = NULL;
    btnode *btp, *parentbtp, *leftbtp, *rightbtp;
    GUEST<INTEGER> *offsetp;
    INTEGER adjust;
    char *startp, *stopp, *freep;
    INTEGER freesize, ntoadjust;
    whichnodetype whichmerge;
    OSErr err;
    BOOLEAN done, modselfkey, modrightkey;
    LONGINT left, right;

    modselfkey = false;
    modrightkey = false;
    *todeletep = -1;
    btp = (btnode *)selfcachep->buf;
    if(selfindex < 0 || selfindex >= CW(btp->ndNRecs))
    {
        warning_unexpected("fried selfindex");
        err = fsDSIntErr;
        fs_err_hook(err);
        return err;
    }

#if defined(CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    /* delete entry */

    selfcachep->flags |= CACHEDIRTY;
    startp = (char *)BTENTRY(btp, selfindex);
    stopp = (char *)BTENTRY(btp, selfindex + 1);
    freep = (char *)BTENTRY(btp, CW(btp->ndNRecs));
    memmove(startp, stopp, freep - stopp);
    ntoadjust = CW(btp->ndNRecs) - selfindex;
    offsetp = BTOFFSET(btp, selfindex);
    adjust = stopp - startp;
    while(--ntoadjust >= 0)
    {
        *offsetp = CW(CW(offsetp[-1]) - adjust);
        --offsetp;
    }
    btp->ndNRecs = CW(CW(btp->ndNRecs) - 1);
    if(selfindex == 0)
        modselfkey = true;

#if defined(CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
#if !defined(LETGCCWAIL)
    leftbtp = rightbtp = 0;
#endif
    /* check to see if freespace is too big */
    freesize = FREESIZE(btp);
    if(freesize > SIZECUTOFF)
    {
        done = false;
        if(parentcachep)
        {
            parentbtp = (btnode *)parentcachep->buf;
#if defined(CATFILEDEBUG)
            checkbtp(parentbtp);
#endif /* CATFILEDEBUG */
            if(parentindex > 0)
                left = CL(*(GUEST<LONGINT> *)DATAPFROMKEY(BTENTRY(parentbtp, parentindex - 1)));
            else
                left = -1;
            if(parentindex < CW(parentbtp->ndNRecs) - 1)
                right = CL(*(GUEST<LONGINT> *)DATAPFROMKEY(BTENTRY(parentbtp, parentindex + 1)));
            else
                right = -1;
        }
        else
        {
            left = -1;
            right = -1;
        }
        if(left >= 0)
        {
            err = ROMlib_getcache(&leftcachep, CW(selfcachep->refnum), left, GETCACHESAVE);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            leftbtp = (btnode *)leftcachep->buf;
#if defined(CATFILEDEBUG)
            checkbtp(leftbtp);
#endif /* CATFILEDEBUG */
            if(freesize + FREESIZE(leftbtp) < 2 * SIZECUTOFF)
            {
                err = shuffle(leftcachep, selfcachep);
#if 0
		printf("shuffled left, self\n");
#endif
                if(err != noErr)
                {
                    fs_err_hook(err);
                    return err;
                }
                modselfkey = true;
                done = true;
            }
        }
        if(!done && right >= 0)
        {
            err = ROMlib_getcache(&rightcachep, CW(selfcachep->refnum), right,
                                  GETCACHESAVE);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            rightbtp = (btnode *)rightcachep->buf;
#if defined(CATFILEDEBUG)
            checkbtp(rightbtp);
#endif /* CATFILEDEBUG */
            if(freesize + FREESIZE(rightbtp) < 2 * SIZECUTOFF)
            {
                err = shuffle(selfcachep, rightcachep);
#if 0
		printf("shuffled self, right\n");
#endif
                if(err != noErr)
                {
                    fs_err_hook(err);
                    return err;
                }
                modrightkey = true;
                done = true;
            }
        }
        if(!done)
        {
            if(left >= 0)
            {
                if(right >= 0)
                    if(FREESIZE(leftbtp) <= FREESIZE(rightbtp))
                        whichmerge = doleft;
                    else
                        whichmerge = doright;
                else
                    whichmerge = doleft;
            }
            else
            {
                if(right >= 0)
                    whichmerge = doright;
                else
                    whichmerge = leavealone;
            }
            switch(whichmerge)
            {
                case doleft:
                    err = merge(leftcachep, selfcachep);
#if 0
		printf("merged left, self\n");
#endif
                    *todeletep = parentindex;
                    modselfkey = false;
                    break;
                case doright:
                    err = merge(selfcachep, rightcachep);
#if 0
		printf("merged self, right\n");
#endif
                    *todeletep = parentindex + 1;
                    modrightkey = false;
                    break;
                case leavealone: /* do nothing */
                    break;
                default:
                    warning_unexpected("unknown whichmerge");
                    err = fsDSIntErr;
                    fs_err_hook(err);
                    return err;
                    break;
            }
        }
    }
    err = noErr;
    if(modselfkey && parentcachep)
    {
        err = btsetkey(parentcachep, parentindex, BTENTRY(btp, 0));
#if 0
	printf("modded self\n");
#endif
    }
    if(err == noErr && modrightkey)
    {
        err = btsetkey(parentcachep, parentindex + 1, BTENTRY(rightbtp, 0));
#if 0
	printf("modded right\n");
#endif
    }
    fs_err_hook(err);
    return err;
}

static OSErr maketrailentrybusy(trailentry *tep, uint16_t refnum)
{
    OSErr err;
    GUEST<HVCB *> SWvcbp;

    SWvcbp = ((filecontrolblock *)((char *)MR(FCBSPtr) + refnum))->fcbVPtr;
    if(CW(tep->cachep->refnum) != refnum || CL(tep->cachep->logblk) != tep->logbno || tep->cachep->vptr != SWvcbp)
        err = ROMlib_getcache(&tep->cachep, refnum, tep->logbno, GETCACHESAVE);
    else
    {
        err = noErr;
        tep->cachep->flags |= CACHEBUSY;
    }
    fs_err_hook(err);
    return err;
}

static OSErr btlegitimize(btparam *btpb)
{
    OSErr err;

    if(btpb->leafindex < 0)
        err = ROMlib_keyfind(btpb);
    else
        err = noErr;
    fs_err_hook(err);
    return err;
}

static OSErr deleteroot(cacheentry *oldrootp, cacheentry *block0cachep)
{
    btblock0 *block0p;
    OSErr err;
    /* update height, root */

    block0p = (btblock0 *)block0cachep->buf;
    block0p->height = CW(CW(block0p->height) - 1);
    block0p->root = *(GUEST<ULONGINT> *)DATAPFROMKEY(BTENTRY((btnode *)oldrootp->buf, 0));
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    block0cachep->flags |= CACHEDIRTY;
    err = deletenode(oldrootp);
    fs_err_hook(err);
    return err;
}

static OSErr btdeletetree(cacheentry *block0cachep, cacheentry *leafcachep)
{
    OSErr err;
    btblock0 *block0p;

    /* TODO: check this over carefully */
    block0p = (btblock0 *)block0cachep->buf;
    block0p->height = CWC(0);
    block0p->root = 0;
    /* don't set block0p->numentries; it'll be decremented later */
    block0p->firstleaf = block0p->lastleaf = 0;
    block0cachep->flags |= CACHEDIRTY;
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    if(!(leafcachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    err = deletenode(leafcachep);
    fs_err_hook(err);
    return err;
}

static OSErr updatenumentries(cacheentry *block0cachep, INTEGER adjust)
{
    btblock0 *block0p;

    block0p = (btblock0 *)block0cachep->buf;
    block0p->numentries = CL(CL(block0p->numentries) + (adjust));
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    block0cachep->flags |= CACHEDIRTY;
    return noErr;
}

OSErr Executor::ROMlib_btdelete(btparam *btpb)
{
    OSErr err;
    trailentry *tep;
    BOOLEAN done;
    cacheentry *selfcachep, *parentcachep;
    INTEGER selfindex, parentindex, todelete, tomung, refnum;

    err = btlegitimize(btpb);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    if(!btpb->success)
    {
#if 0
        err = ROMlib_keyfind(btpb);
        if (err != noErr)
#endif
        {
            warning_unexpected("no success in ROMlib_btdelete");
            err = fsDSIntErr;
            fs_err_hook(err);
            return err;
        }
    }
    tep = btpb->trail + btpb->leafindex;
    selfindex = tep->after;
    done = false;
    refnum = CW(btpb->trail[0].cachep->refnum);
    if(((btblock0 *)btpb->trail[0].cachep->buf)->numentries == CLC(1))
    {
        err = maketrailentrybusy(&btpb->trail[1], refnum);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        err = btdeletetree(btpb->trail[0].cachep, btpb->trail[1].cachep);
    }
    else
    {
        done = false;
        while(!done)
        {
            err = maketrailentrybusy(tep - 1, refnum);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            err = maketrailentrybusy(tep, refnum);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            selfcachep = tep->cachep;
            if(tep > btpb->trail + 1)
            {
                parentcachep = tep[-1].cachep;
                parentindex = tep[-1].after;
            }
            else
            {
                parentcachep = 0;
#if !defined(LETGCCWAIL)
                parentindex = 0;
#endif
            }
            err = pullout(selfcachep, selfindex, parentcachep, parentindex,
                          &todelete);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            if(todelete == -1 && ((btnode *)selfcachep->buf)->ndNRecs == CWC(1)
               && ((btnode *)selfcachep->buf)->ndType == indexnode)
            {
                err = deleteroot(selfcachep, btpb->trail[0].cachep);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    return err;
                }
            }
            if(todelete == -1)
                done = true;
            else
            {
                selfindex = todelete;
                tep--;
            }
        }
        /*
 * We start the loop off at +leafindex-1 because the bottom most one is
 * going to already be done for us if it is necessary (which it may not
 * be if there was a merge that resulted in our current node being deleted)
 */
        for(tep = btpb->trail + btpb->leafindex - 1;
            tep > btpb->trail + 1 && tep->after == 0; --tep)
        {
            tomung = tep[-1].after;
            if(tomung == -1)
            {
                warning_unexpected("tomung is -1");
                err = fsDSIntErr;
                fs_err_hook(err);
                return err;
            }
            err = maketrailentrybusy(tep, refnum);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            err = maketrailentrybusy(tep - 1, refnum);
            if(err != noErr)
            {
                fs_err_hook(err);
                return err;
            }
            if((*btpb->fp)(&btpb->tofind,
                           BTENTRY((btnode *)tep[-1].cachep->buf, tomung))
               == same)
            {
                btsetkey(tep[-1].cachep, tomung,
                         BTENTRY((btnode *)tep[0].cachep->buf, 0));
#if 0
	        printf("munging %d\n", tomung);
#endif
            }
        }
        err = noErr;
    }
    if(err == noErr)
        err = updatenumentries(btpb->trail[0].cachep, -1);
    fs_err_hook(err);
    return err;
}

OSErr Executor::ROMlib_makecatparam(btparam *btpb, HVCB *vcbp, LONGINT dirid,
                                    INTEGER namelen, Ptr namep)
{
    OSErr retval;

    btpb->vcbp = vcbp;
    retval = ROMlib_makecatkey((catkey *)&btpb->tofind, dirid, namelen, namep);
    btpb->fp = ROMlib_catcompare;
    btpb->refnum = CW(vcbp->vcbCTRef);
    btpb->leafindex = -1;
    fs_err_hook(retval);
    return retval;
}

OSErr Executor::ROMlib_dirtyleaf(void *p, HVCB *vcbp)
{
    OSErr err;
    cacheentry *cachep;

    cachep = ROMlib_addrtocachep((Ptr)p, vcbp);
    if(cachep)
    {
#if 0
	if (!(cachep->flags & CACHEBUSY))
	    warning_unexpected ("not busy");
#endif
        cachep->flags |= CACHEDIRTY;
        err = noErr;
    }
    else
    {
        warning_unexpected("addrtocachep failed");
        err = fsDSIntErr;
    }
    fs_err_hook(err);
    return err;
}

static OSErr valenceadjust(btparam *btpb, INTEGER toadjust, filekind kind)
{
    OSErr err;
    GUEST<LONGINT> *countadj;
    threadrec *thdp;
    directoryrec *drp;
    btparam btparamblock;

    err = noErr;
    switch(kind)
    {
        case regular:
            countadj = &btpb->vcbp->vcbFilCnt;
            break;
        case directory:
            countadj = &btpb->vcbp->vcbDirCnt;
            break;
        default:
#if !defined(LETGCCWAIL)
            countadj = 0;
#endif /* LETGCCWAIL */
            warning_unexpected("unknown valenceadjust");
            err = fsDSIntErr;
            break;
    }
    if(err == noErr)
    {
        err = ROMlib_makecatparam(&btparamblock, btpb->vcbp,
                                  CL(btpb->tofind.catk.ckrParID), 0, (Ptr)0);
        if(err == noErr)
            err = ROMlib_keyfind(&btparamblock);
        if(err == noErr && btparamblock.success)
        {
            thdp = (threadrec *)DATAPFROMKEY(btparamblock.foundp);
            err = ROMlib_makecatkey((catkey *)&btparamblock.tofind, CL(thdp->thdParID),
                                    thdp->thdCName[0], (Ptr)thdp->thdCName + 1);
            /* don't need to remake btparamblock */
            if(err == noErr)
                err = ROMlib_keyfind(&btparamblock);
            if(err == noErr && btparamblock.success)
            {
                drp = (directoryrec *)DATAPFROMKEY(btparamblock.foundp);
                drp->dirVal = CW(CW(drp->dirVal) + (toadjust));
                err = ROMlib_dirtyleaf(drp, btpb->vcbp);
                if(err == noErr)
                {
                    *countadj = CL(CL(*countadj) + (toadjust));
                    if(drp->dirDirID == CLC(2))
                    {
                        if(kind == directory)
                            btpb->vcbp->vcbNmRtDirs = CW(CW(btpb->vcbp->vcbNmRtDirs) + toadjust);
                        else
                            btpb->vcbp->vcbNmFls = CW(CW(btpb->vcbp->vcbNmFls) + toadjust);
                    }
                    btpb->vcbp->vcbFlags |= CW(VCBDIRTY);
                }
            }
            else
            {
                if(err == noErr)
                {
                    warning_unexpected("no success2");
                    err = fsDSIntErr;
                }
            }
        }
        else
        {
            if(err == noErr)
            {
                warning_unexpected("no success1");
                err = fsDSIntErr;
            }
        }
    }
    fs_err_hook(err);
    return err;
}

/*
 * ROMlib_filedelete calls ROMlib_btdelete but adjusts valences (is called by ROMlib_dirdelete)
 */

OSErr Executor::ROMlib_filedelete(btparam *btpb, filekind kind)
{
    OSErr err;

    err = ROMlib_btdelete(btpb);
    if(err == noErr)
        err = valenceadjust(btpb, -1, kind);
    fs_err_hook(err);
    return err;
}

/*
 * ROMlib_dirdelete calls ROMlib_filedelete but also deletes the thread record
 */

OSErr Executor::ROMlib_dirdelete(btparam *btpb)
{
    OSErr err;
    directoryrec *drp;
    LONGINT dirid;

    drp = (directoryrec *)DATAPFROMKEY(btpb->foundp);
    dirid = CL(drp->dirDirID);
    err = ROMlib_filedelete(btpb, directory);
    if(err == noErr)
    { /* nuke the thread record */
        err = ROMlib_makecatkey((catkey *)&btpb->tofind, dirid, 0, (Ptr)0);
        btpb->leafindex = -1;
        if(err == noErr)
            err = ROMlib_btdelete(btpb);
    }
    fs_err_hook(err);
    return err;
}

typedef struct
{
    short refnum;
    ULONGINT logbno;
} saverec_t;

static OSErr savebusybuffers(HVCB *vcbp, GUEST<saverec_t *> **savehandlep)
{
    INTEGER count;
    cacheentry *cachep;
    cachehead *headp;
    saverec_t tempsaverec;
    GUEST<saverec_t *> *retval;
    Size cursize;
    OSErr err;

    headp = (cachehead *)MR(vcbp->vcbCtlBuf);
    count = CW(headp->nitems);

    retval = (GUEST<saverec_t *> *)NewHandle((Size)0);
    if(retval == 0)
    {
        err = MemError();
        fs_err_hook(err);
        return err;
    }
    cursize = 0;
    for(cachep = MR(headp->flink); --count >= 0; cachep = MR(cachep->flink))
    {
        if(cachep->flags & CACHEBUSY)
        {
            tempsaverec.refnum = CW(cachep->refnum);
            tempsaverec.logbno = CL(cachep->logblk);
            SetHandleSize((Handle)retval, cursize + sizeof(tempsaverec));
            memmove((char *)MR(*retval) + cursize,
                    &tempsaverec, sizeof(tempsaverec));
            cursize += sizeof(tempsaverec);
        }
    }
    *savehandlep = retval;
    return noErr;
}

static OSErr restorebusybuffers(GUEST<saverec_t *> *savehandle)
{
    INTEGER nentries;
    saverec_t *savep;
    OSErr err, retval;
    cacheentry *notused;

    retval = noErr;
    nentries = GetHandleSize((Handle)savehandle) / sizeof(saverec_t);
    HLock((Handle)savehandle);
    for(savep = MR(*savehandle); --nentries >= 0; ++savep)
    {
        err = ROMlib_getcache(&notused, savep->refnum, savep->logbno, GETCACHESAVE);
        if(retval == noErr)
            retval = err;
    }
    HUnlock((Handle)savehandle);
    DisposHandle((Handle)savehandle);
    fs_err_hook(retval);
    return retval;
}

static OSErr getfreenode(cacheentry **newcachepp, cacheentry *block0cachep)
{
    OSErr err, err1;
    cacheentry *newcachep;
    btblock0 *block0p;
    ULONGINT nblocksalloced, newblock;
    IOParam iop;
    INTEGER refnum, flags;
    filecontrolblock *fcbp;
    GUEST<saverec_t *> *busysave;

    refnum = CW(block0cachep->refnum);
    block0p = (btblock0 *)block0cachep->buf;
    if(block0p->nfreenodes == CLC(0))
    {
        fcbp = (filecontrolblock *)((char *)MR(FCBSPtr) + refnum);
        iop.ioRefNum = CW(refnum);
        iop.ioReqCount = fcbp->fcbClmpSize;

        /* We never add more than one extra mapping page, so we need
	   to make sure we don't try to allocate more than we can map.
	   max_bytes_we_can_map comes out to about 2 million bytes, so
	   this restriction shouldn't hurt us. */

        {
            LONGINT max_bytes_we_can_map;

            max_bytes_we_can_map = (MAP_PAGE_MAP_END - MAP_PAGE_MAP_BEGIN)
                * 8 * PHYSBSIZE;
            if(CL(iop.ioReqCount) > max_bytes_we_can_map)
                iop.ioReqCount = CLC(max_bytes_we_can_map);
        }

        flags = fcbp->fcbMdRByt;
        fcbp->fcbMdRByt |= WRITEBIT;
        err = savebusybuffers(MR(fcbp->fcbVPtr), &busysave);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        ROMlib_cleancache(MR(fcbp->fcbVPtr));
        err = PBAllocate((ParmBlkPtr)&iop, false); /* yahoo */
        MR(fcbp->fcbVPtr)->vcbFlags |= CWC(VCBDIRTY);
        ROMlib_flushvcbp(MR(fcbp->fcbVPtr)); /* just setting DIRTY isn't safe */
        err1 = restorebusybuffers(busysave);
        if(err == noErr || (err == dskFulErr && CL(iop.ioActCount) > 0))
            err = err1;
        fcbp->fcbMdRByt = flags;
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        nblocksalloced = CL(iop.ioActCount) / PHYSBSIZE;
        if(nblocksalloced <= 0)
        {
            warning_unexpected("nblocksalloced <= 0");
            err = fsDSIntErr;
            fs_err_hook(err);
            return err;
        }
        err = add_free_nodes(block0cachep, nblocksalloced);
        if(err != noErr)
        {
            fs_err_hook(err);
            /*-->*/ return err;
        }
    }
    err = MapFindFirstBitAndSet(block0cachep, &newblock);
    if(err != noErr)
    {
        fs_err_hook(err);
        /*-->*/ return err;
    }
    block0p->nfreenodes = CL(CL(block0p->nfreenodes) - 1);
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    block0cachep->flags |= CACHEDIRTY;
    err = ROMlib_getcache(&newcachep, refnum, newblock, (cacheflagtype)(GETCACHESAVE | GETCACHENOREAD));
    if(err == noErr)
        *newcachepp = newcachep;
    fs_err_hook(err);
    return err;
}

static OSErr getnewnode(cacheentry **newcachepp, cacheentry *leftp)
{
    cacheentry *newcachep, *block0cachep, *linkcachep;
    btblock0 *block0p;
    btnode *leftbtp, *newbtp, *linkbtp;
    OSErr err;
    INTEGER refnum;
    ULONGINT newnode, leftnode, flink;

    refnum = CW(leftp->refnum);
    err = ROMlib_getcache(&block0cachep, refnum, 0L, GETCACHESAVE);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    block0p = (btblock0 *)block0cachep->buf;
    err = getfreenode(&newcachep, block0cachep);
    *newcachepp = newcachep;
    newbtp = (btnode *)newcachep->buf;
    newnode = CL(newcachep->logblk);
    leftbtp = (btnode *)leftp->buf;
#if defined(CATFILEDEBUG)
    checkbtp(leftbtp);
#endif /* CATFILEDEBUG */
    memmove(newbtp, leftbtp, (sizeof(CL(leftbtp->ndFLink))
                              + sizeof(CL(leftbtp->ndBLink))
                              + sizeof(leftbtp->ndType)
                              + sizeof(leftbtp->ndLevel)));
    leftnode = CL(leftp->logblk);
    if(CL(block0p->lastleaf) == leftnode)
        block0p->lastleaf = CL(newnode);
    leftbtp->ndFLink = CL(newnode);
    if(!(leftp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    leftp->flags |= CACHEDIRTY;
    newbtp->ndBLink = CL(leftnode);
    if((flink = CL(newbtp->ndFLink)))
    {
        err = ROMlib_getcache(&linkcachep, refnum, flink, (cacheflagtype)0);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        linkbtp = (btnode *)linkcachep->buf;
        linkbtp->ndBLink = CL(newnode);
        linkcachep->flags |= CACHEDIRTY;
    }
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    block0cachep->flags |= CACHEDIRTY;
    return noErr;
}

/*
 * slipin adds the (key,data) pair to btp, splitting the node if necessary.
 * If the node is split, a pointer to the new node is returned to where
 * cachepp points.  slipin never goes up the tree, but if it splits, or if
 * after is -1 then the caller of slipin will need to update the parent of btp.
 *
 * NOTE: when the split is being done there is some extra copying of memory
 *       going on.  Specifically we do the split as though there is nothing to
 *       be added in and then we call ourselves to do the insert.  This could
 *       be avoided with more code but wouldn't make too much difference since
 *       we're using a write-though cache, which causes the I/O to overwhelm
 *       the computation.
 */

static OSErr slipin(cacheentry *cachep, INTEGER after, anykey *keyp,
                    char *data, INTEGER datasize, cacheentry **cachepp)
{
    INTEGER sizeneeded, nrecs, keysize, freesize, offsetsize, noffsets, i;
    INTEGER newfirst, sizeused, shim;
    char *keylocp, *firstlocp;
    GUEST<INTEGER> *firstoffset, *offsetp;
    uint16_t newnode;
    cacheentry *newcachep;
    btnode *btp, *newbtp;
    HVCB *vcbp;
    BOOLEAN inbtp;
    OSErr err;

    btp = (btnode *)cachep->buf;
#if defined(CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    vcbp = MR(cachep->vptr);
    firstoffset = (GUEST<INTEGER> *)((char *)btp + PHYSBSIZE) - 1;
    nrecs = CW(btp->ndNRecs);
    keysize = EVENUP(((catkey *)keyp)->ckrKeyLen + 1);
    datasize = EVENUP(datasize);
    sizeneeded = keysize + datasize + sizeof(INTEGER);
    freesize = FREESIZE(btp);
    if(sizeneeded <= freesize)
    {
        /* TESTED:  WORKS! */
        keylocp = (char *)BTENTRY(btp, after + 1);
        memmove(keylocp + sizeneeded - sizeof(INTEGER), keylocp,
                (char *)BTENTRY(btp, nrecs) - (char *)BTENTRY(btp, after + 1));
        memmove(keylocp, keyp, keysize);
        memmove(keylocp + keysize, data, datasize);
        for(i = nrecs - after, offsetp = &firstoffset[-nrecs - 1]; --i >= 0;)
        {
            offsetp[0] = CW(CW(offsetp[1]) + datasize + keysize);
            ++offsetp;
        }
        btp->ndNRecs = CW(CW(btp->ndNRecs) + 1);
        if(cachepp)
            *cachepp = 0;
        if(!(cachep->flags & CACHEBUSY))
            warning_unexpected("not busy");
        cachep->flags |= CACHEDIRTY;
        err = noErr;
#if defined(CATFILEDEBUG)
        checkbtp(btp);
#endif /* CATFILEDEBUG */
    }
    else
    {
        /* NOTE: it might be a win to try to shuffle with left and right,
					       but I'm not too concerned now */
        err = getnewnode(&newcachep, cachep);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        newnode = CL(newcachep->logblk);
        newbtp = (btnode *)newcachep->buf;
        if(cachepp)
            *cachepp = newcachep;

        /* find split */
        inbtp = false;
        for(newfirst = 0, sizeused = 0; sizeused < SIZECUTOFF; newfirst++)
        {
            if(after + 1 == newfirst && !inbtp)
            {
                inbtp = true;
                sizeused += sizeneeded;
                /* --> */ --newfirst; /* didn't add in newfirst on this go around */
            }
            else
                sizeused += (char *)BTENTRY(btp, newfirst + 1) - (char *)BTENTRY(btp, newfirst) + sizeof(INTEGER);
        }
        /* copy group from btp to newbtp */
        firstlocp = (char *)BTENTRY(btp, newfirst);
        memmove((char *)newbtp + sizeof(btnode), firstlocp,
                (char *)BTENTRY(btp, nrecs) - firstlocp);
        /* adjust offsets */
        noffsets = nrecs - newfirst + 1;
        offsetsize = noffsets * sizeof(INTEGER);
        memmove((char *)newbtp + PHYSBSIZE - offsetsize,
                &firstoffset[-nrecs], offsetsize);
        shim = (char *)BTENTRY(btp, newfirst) - (char *)btp - sizeof(btnode);
        offsetp = (GUEST<INTEGER> *)((char *)newbtp + PHYSBSIZE);
        for(i = noffsets; --i >= 0;)
        {
            --offsetp;
            *offsetp = CW(CW(*offsetp) - shim);
        }

        newbtp->ndNRecs = CW(noffsets - 1);
        btp->ndNRecs = CW(newfirst);

#if defined(CATFILEDEBUG)
        checkbtp(btp);
        checkbtp(newbtp);
#endif

        if(inbtp)
            err = slipin(cachep, after, keyp, data, datasize,
                         (cacheentry **)0);
        else
            err = slipin(newcachep, after - newfirst, keyp, data, datasize,
                         (cacheentry **)0);
        if(!(cachep->flags & CACHEBUSY))
            warning_unexpected("not busy");
        if(!(newcachep->flags & CACHEBUSY))
            warning_unexpected("not busy");
        cachep->flags |= CACHEDIRTY;
        newcachep->flags |= CACHEDIRTY;
#if defined(CATFILEDEBUG)
        checkbtp(btp);
        checkbtp(newbtp);
#endif
    }
    fs_err_hook(err);
    return err;
}

static OSErr makenewroot(cacheentry *leftp, cacheentry *rightp,
                         cacheentry *block0cachep)
{
    OSErr err;
    btblock0 *block0p;
    cacheentry *newcachep;
    btnode *newbtp, *leftbtp, *rightbtp;
    GUEST<INTEGER> *offsetp;
    INTEGER keylen;
    char *keydst;

    err = getfreenode(&newcachep, block0cachep);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    block0p = (btblock0 *)block0cachep->buf;
    newbtp = (btnode *)newcachep->buf;
    newbtp->ndFLink = 0;
    newbtp->ndBLink = 0;
    newbtp->ndType = indexnode;
    block0p->height = CW(CW(block0p->height) + 1);
    newbtp->ndLevel = CW(block0p->height);
    leftbtp = (btnode *)leftp->buf;
    if(rightp)
    {
        rightbtp = (btnode *)rightp->buf;
        newbtp->ndNRecs = CWC(2);
    }
    else
    {
        newbtp->ndNRecs = CWC(1);
#if !defined(LETGCCWAIL)
        rightbtp = 0;
#endif /* LETGCCWAIL */
    }
    if(!(newcachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    newcachep->flags |= CACHEDIRTY;
    block0p->root = newcachep->logblk;
    block0cachep->flags |= CACHEDIRTY;
    offsetp = BTOFFSET(newbtp, 0);
    keylen = EVENUP(CW(block0p->indexkeylen) + 1);
    offsetp[0] = CW(sizeof(btnode));
    keydst = (char *)BTENTRY(newbtp, 0);
    memmove(keydst, BTENTRY(leftbtp, 0), keylen);
    *keydst = keylen - 1;
    *(GUEST<ULONGINT> *)(keydst + keylen) = leftp->logblk;
    if(rightp)
    {
        --offsetp;
        offsetp[0] = CW(CW(offsetp[1]) + keylen + sizeof(ULONGINT));
        keydst = (char *)BTENTRY(newbtp, 1);
        memmove(keydst, BTENTRY(rightbtp, 0), keylen);
        *keydst = keylen - 1;
        *(GUEST<ULONGINT> *)(keydst + keylen) = rightp->logblk;
    }
    --offsetp;
    offsetp[0] = CW(CW(offsetp[1]) + keylen + sizeof(ULONGINT));
#if defined(CATFILEDEBUG)
    checkbtp(newbtp);
    checkbtp(leftbtp);
    if(rightp)
        checkbtp(rightbtp);
#endif
    return noErr;
}

/* TODO: makefirstentry should create just one leaf node! */

static OSErr makefirstentry(btparam *btpb, char *datap, INTEGER datasize)
{
    OSErr err;
    cacheentry *block0cachep, *leafp;
    btnode *leafbtp;
    GUEST<INTEGER> *offsetp;
    btblock0 *block0p;
    ULONGINT newnode;
    INTEGER keylen;
    char *keydst;

    block0cachep = btpb->trail[0].cachep;
    err = getfreenode(&leafp, block0cachep);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    leafbtp = (btnode *)leafp->buf;
    leafbtp->ndFLink = 0;
    leafbtp->ndBLink = 0;
    leafbtp->ndType = leafnode;
    leafbtp->ndLevel = 1;
    leafbtp->ndNRecs = CWC(1);
    offsetp = BTOFFSET(leafbtp, 0);
    *offsetp = CWC(sizeof(btnode));
    keylen = EVENUP(btpb->tofind.keylen + 1);
    keydst = (char *)BTENTRY(leafbtp, 0);
    memmove(keydst, &btpb->tofind, keylen);
    memmove(keydst + keylen, datap, datasize);
    --offsetp;
    offsetp[0] = CW(CW(offsetp[1]) + keylen + EVENUP(datasize));
    if(!(leafp->flags & CACHEBUSY))
        warning_unexpected("not busy");
    leafp->flags |= CACHEDIRTY;
    block0p = (btblock0 *)block0cachep->buf;
    block0p->height = CWC(1);
    block0p->root = leafp->logblk;
    block0p->numentries = CLC(1);
    newnode = CL(leafp->logblk);
    block0p->firstleaf = CL(newnode);
    block0p->lastleaf = CL(newnode);
    if(!(block0cachep->flags & CACHEBUSY))
        warning_unexpected("not busy");
    block0cachep->flags |= CACHEDIRTY;
    return noErr;
}

static OSErr btcreate(btparam *btpb, void *datap, INTEGER datasize)
{
    OSErr err;
    trailentry *tep;
    cacheentry *newcachep, *block0cachep;
    anykey *keytoinsertp;
    char *datatoinsertp;
    INTEGER sizetoinsert, keylen, tomung, refnum;
    BOOLEAN done;
    btnode *newbtp;
    anykey tempkey;
    btblock0 *block0p;
    HVCB *vcbp;
    ULONGINT potentialfree;

    err = ROMlib_getcache(&block0cachep, btpb->refnum, (ULONGINT)0, GETCACHESAVE);
    if(err != noErr)
    {
        fs_err_hook(err);
        /*-->*/ return err;
    }
    block0p = (btblock0 *)block0cachep->buf;
    vcbp = MR(block0cachep->vptr);
    potentialfree = CL(block0p->nfreenodes);
    potentialfree += CW(vcbp->vcbFreeBks) * (CL(vcbp->vcbAlBlkSiz) / PHYSBSIZE);

    /*
 * It is possible for a split to ripple up the entire b-tree and even force
 * us to have a new root node.  Since once we can't roll back a split if we
 * find out we're out of room, this means we have to have enough spare blocks
 * to add a new node at each level in the tree, and add a new root.
 *
 * Theoretically we could still get in trouble if by adding blocks to the
 * catalog b-tree we wind up needing to add some extents b-tree entries.
 * I think in non-contrived situations this is EXTREMELY unlikely to occur.
 */

    if(potentialfree < (ULONGINT)CW(block0p->height) + 1)
    {
        err = dskFulErr;
        fs_err_hook(err);
        /*-->*/ return err;
    }
    btpb->trail[0].cachep = block0cachep;
    if(block0p->numentries == CLC(0))
    {
        err = ::makefirstentry(btpb, (char *)datap, datasize);
        fs_err_hook(err);
        return err;
    }

    err = btlegitimize(btpb);
    if(err != noErr)
    {
        fs_err_hook(err);
        /*-->*/ return err;
    }
    if(btpb->success)
    {
        err = dupFNErr;
        fs_err_hook(err);
        /*-->*/ return err;
    }
    tep = btpb->trail + btpb->leafindex;
    keytoinsertp = &btpb->tofind;
    datatoinsertp = (char *)datap;
    sizetoinsert = datasize;
    done = false;
    refnum = CW(block0cachep->refnum);
    while(!done)
    {
        err = maketrailentrybusy(tep, refnum);
        err = slipin(tep->cachep, tep->after, keytoinsertp, datatoinsertp,
                     sizetoinsert, &newcachep);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        if(newcachep)
        {
            --tep;
            if(tep == btpb->trail)
            {
                err = makenewroot(tep[1].cachep, newcachep, tep->cachep);
                if(err != noErr)
                {
                    fs_err_hook(err);
                    return err;
                }
                done = true;
            }
            else
            {
                if(block0p->lastleaf == tep->cachep->logblk)
                {
                    block0p->lastleaf = newcachep->logblk;
                    block0cachep->flags |= CACHEDIRTY;
                }
                newbtp = (btnode *)newcachep->buf;
#if 0
		keylen = BTENTRY((btnode *)tep->cachep->buf, 0)->keylen;
#else
                keylen = CW(block0p->indexkeylen);
#endif
                keytoinsertp = BTENTRY(newbtp, 0);
                if(keytoinsertp->keylen < keylen)
                {
                    tempkey = *keytoinsertp;
                    tempkey.keylen = keylen;
                    keytoinsertp = &tempkey;
                }
                else if(keytoinsertp->keylen > keylen)
                {
                    warning_unexpected("keytoinsertp->keylen too big");
                    err = fsDSIntErr;
                    fs_err_hook(err);
                    return err;
                }
                datatoinsertp = (char *)&newcachep->logblk;
                sizetoinsert = sizeof(newcachep->logblk);
            }
        }
        else
            done = true;
    }
    for(tep = btpb->trail + btpb->leafindex;
        tep > btpb->trail + 1 && tep->after == (unsigned short)-1; --tep)
    {
        tomung = tep[-1].after;
        if(tomung == -1)
            tomung = 0;
        err = maketrailentrybusy(tep - 1, refnum);
        if(err != noErr)
        {
            fs_err_hook(err);
            return err;
        }
        btsetkey(tep[-1].cachep, tomung,
                 BTENTRY((btnode *)tep[0].cachep->buf, 0));
    }
    err = updatenumentries(block0cachep, 1);
    fs_err_hook(err);
    return err;
}

static void makethreadrec(threadrec *recp, GUEST<LONGINT> SWparid,
                          StringPtr namep)
{
    memset(recp, 0, sizeof(*recp));
    recp->cdrType = THREADTYPE;
    recp->thdParID = SWparid;
    str255assign(recp->thdCName, namep);
}

/*
 * ROMlib_filecreate calls btcreate but adjusts the valence afterward.
 * NOTE: ROMlib_filecreate IS used to create directories as well (see ROMlib_dircreate below)
 */

OSErr Executor::ROMlib_filecreate(btparam *btpb, void *data, filekind kind)
{
    OSErr err;
    INTEGER datasize;

    datasize = kind == directory ? sizeof(directoryrec) : sizeof(filerec);
    err = btcreate(btpb, data, datasize);
    if(err == noErr)
        err = valenceadjust(btpb, 1, kind);
    fs_err_hook(err);
    return err;
}

/*
 * ROMlib_dircreate calls ROMlib_filecreate but also creates a thread record
 */

OSErr Executor::ROMlib_dircreate(btparam *btpb, directoryrec *data)
{
    OSErr err;
    threadrec rec;

    err = ROMlib_filecreate(btpb, data, directory);
    if(err == noErr)
    {
        makethreadrec(&rec, ((catkey *)&btpb->tofind)->ckrParID,
                      ((catkey *)&btpb->tofind)->ckrCName);
        err = ROMlib_makecatkey((catkey *)&btpb->tofind, CL(data->dirDirID), 0, (Ptr)0);
        btpb->leafindex = -1;
        if(err == noErr)
            err = btcreate(btpb, &rec, sizeof(rec));
    }
    fs_err_hook(err);
    return err;
}

xtntkey *Executor::ROMlib_newextentrecord(
    filecontrolblock *fcbp, uint16_t newabn)
{
    xtntrec rec;
    HVCB *vcbp;
    btparam btparamrec;
    OSErr err;
    Forktype forkwanted;

    vcbp = MR(fcbp->fcbVPtr);
    forkwanted = fcbp->fcbMdRByt & RESOURCEBIT ? resourcefork : datafork;
    memset(&rec, 0, sizeof(rec));
    ROMlib_makextntparam(&btparamrec, vcbp, forkwanted, CL(fcbp->fcbFlNum), newabn);
    if((err = btcreate(&btparamrec, rec, sizeof(rec))) != noErr)
    {
        warning_unexpected("couldn't create new xtntrec");
        return 0;
    }
    btparamrec.leafindex = -1;
    err = btlegitimize(&btparamrec);
    if(err != noErr)
    {
        warning_unexpected("couldn't find new xtntrec");
        return 0;
    }
    return (xtntkey *)btparamrec.foundp;
}

OSErr Executor::ROMlib_btrename(btparam *btpb, StringPtr newnamep)
{
    btparam newbtparam;
    OSErr err;
    char *datap;
    INTEGER datasize;

    newbtparam = *btpb;
    err = ROMlib_makecatkey((catkey *)&newbtparam.tofind,
                            CL(((catkey *)&btpb->tofind)->ckrParID), newnamep[0],
                            (Ptr)newnamep + 1);
    newbtparam.leafindex = -1; /* I don't think this line does anything */
    /* useful, but I noticed it the day that */
    /* I was wrapping up Executor/DOS 1.0 and */
    /* it didn't make sense to take it out then. */
    /* After all, it is harmless. */
    btpb->leafindex = -1;
    if(err == noErr)
        err = btlegitimize(btpb);
    if(err != noErr)
    {
        fs_err_hook(err);
        return err;
    }
    if(!btpb->success)
    {
        warning_unexpected("no success in ROMlib_btrename");
        err = fsDSIntErr;
        fs_err_hook(err);
        return err;
    }
    datap = DATAPFROMKEY(btpb->foundp);
    switch(((filerec *)datap)->cdrType)
    {
        case FILETYPE:
            datasize = sizeof(filerec);
            break;
        case DIRTYPE:
            datasize = sizeof(directoryrec);
            break;
        default:
            warning_unexpected("unknown cdrType in ROMlib_btrename");
            err = fsDSIntErr;
            fs_err_hook(err);
            return err;
    }
    datap = (char *)alloca(datasize);
    memmove(datap, DATAPFROMKEY(btpb->foundp), datasize);
    err = btcreate(&newbtparam, datap, datasize);
    if(err != noErr)
    {
        fs_err_hook(err);
        /*-->*/ return err;
    }
    btpb->leafindex = -1;
    err = ROMlib_btdelete(btpb);
    if(err == noErr && ((filerec *)datap)->cdrType == DIRTYPE)
    {
        err = ROMlib_makecatkey((catkey *)&newbtparam.tofind,
                                CL(((directoryrec *)datap)->dirDirID), 0, (Ptr)0);
        newbtparam.leafindex = -1;
        if(err == noErr)
            err = btlegitimize(&newbtparam);
        datap = DATAPFROMKEY(newbtparam.foundp);
        str255assign(((threadrec *)datap)->thdCName, newnamep);
        newbtparam.trail[newbtparam.leafindex].cachep->flags |= CACHEDIRTY;
    }
    fs_err_hook(err);
    return err;
}

/*
 * IMIV lies ... this bit is *not* set
 *
 * #define STARTFLAGS (1<<7)   IMIV-172 record used
 */

enum
{
    STARTFLAGS = 0
};

OSErr Executor::ROMlib_btcreateemptyfile(btparam *btpb)
{
    OSErr err;
    filerec rec;
    HVCB *vcbp;

    vcbp = btpb->vcbp;
    memset(&rec, 0, sizeof(rec));
    rec.cdrType = FILETYPE;
    rec.filFlags = STARTFLAGS;
    rec.filFlNum = vcbp->vcbNxtCNID;
    vcbp->vcbNxtCNID = CL(CL(vcbp->vcbNxtCNID) + 1);
    vcbp->vcbFlags |= CWC(VCBDIRTY);
    rec.filMdDat = rec.filCrDat = Time;
    err = ROMlib_filecreate(btpb, &rec, regular);
#if defined(CATFILEDEBUG)
    ROMlib_checkleaves(CW(vcbp->vcbCTRef));
#endif /* CATFILEDEBUG */
    fs_err_hook(err);
    return err;
}

OSErr Executor::ROMlib_btcreateemptydir(btparam *btpb, GUEST<LONGINT> *newidp)
{
    directoryrec rec;
    HVCB *vcbp;
    OSErr err;

    vcbp = btpb->vcbp;
    memset(&rec, 0, sizeof(rec));
    rec.cdrType = DIRTYPE;
    rec.dirFlags = CWC(STARTFLAGS);
    *newidp = rec.dirDirID = vcbp->vcbNxtCNID;
    vcbp->vcbNxtCNID = CL(CL(vcbp->vcbNxtCNID) + 1);
    vcbp->vcbFlags |= CWC(VCBDIRTY);
    rec.dirMdDat = rec.dirCrDat = Time;
    err = ROMlib_dircreate(btpb, &rec);
    fs_err_hook(err);
    return err;
}

/*
 * NOTE: ROMlib_getcache clears the ROMlib_index_cached flag
 */

OSErr Executor::ROMlib_btpbindex(IOParam *pb, LONGINT dirid, HVCB **vcbpp,
                                 filerec **frpp, catkey **catkeypp,
                                 BOOLEAN filesonly)
{
    IOParam newpb;
    btparam btparamrec;
    filekind kind;
    OSErr err;
    BOOLEAN done;
    LONGINT count, new_count;
    btnode *btp;
    LONGINT flink;
    INTEGER refnum;
    static HVCB *save_vcbp;
    static LONGINT save_dirid;
    static cacheentry *save_cachep;
    static INTEGER save_index;
    static filerec *save_frp;
    static anykey *save_entryp;
    static LONGINT save_count;
    static GUEST<INTEGER> save_vRefNum;

    if(dirid != save_dirid || pb->ioVRefNum != save_vRefNum)
    {
        ROMlib_index_cached = false;
        save_dirid = dirid;
        save_vRefNum = pb->ioVRefNum;
    }
    new_count = CW(((FileParam *)pb)->ioFDirIndex);
    if(ROMlib_index_cached && new_count >= save_count)
        count = new_count - save_count;
    else
        count = new_count;
    save_count = new_count;
    newpb = *pb;
    newpb.ioNamePtr = RM((StringPtr) "");
    *vcbpp = 0;

    kind = thread;
    if(ROMlib_index_cached)
        err = noErr;
    else
        err = ROMlib_findvcbandfile(&newpb, dirid, &btparamrec, &kind, false);
    if(err == noErr)
    {
        if(!ROMlib_index_cached)
        {
            save_vcbp = btparamrec.vcbp;
            save_dirid = CL(btparamrec.foundp->catk.ckrParID); /* Could have */
            /* changed due to */
            /* Working Dir ID */
        }
        *vcbpp = save_vcbp;
        if(!ROMlib_index_cached && (!btparamrec.success || kind != thread))
        {
            warning_unexpected("didn't find thread");
            err = fsDSIntErr;
        }
        else
        {
            if(!ROMlib_index_cached)
            {
                save_cachep = btparamrec.trail[btparamrec.leafindex].cachep;
                save_index = btparamrec.trail[btparamrec.leafindex].after;
            }
            refnum = CW(save_cachep->refnum);
            btp = (btnode *)save_cachep->buf;
            for(done = count == 0; !done;)
            {
                if(++save_index < CW(btp->ndNRecs))
                {
                    ; /* nothing to do here;
			    bumping save_index was sufficient */
                }
                else if((flink = CL(btp->ndFLink)))
                {
                    save_cachep->flags &= ~CACHEBUSY;
                    err = ROMlib_getcache(&save_cachep, refnum, flink,
                                          GETCACHESAVE);
                    if(err != noErr)
                        done = true;
                    btp = (btnode *)save_cachep->buf;
                    save_index = 0;
                }
                else
                    done = true;
                save_entryp = BTENTRY(btp, save_index);
                if(save_dirid != 1 && CL(save_entryp->catk.ckrParID) != save_dirid)
                    done = true;
                else if(!done)
                {
                    save_frp = (filerec *)DATAPFROMKEY(save_entryp);
                    if(save_frp->cdrType == FILETYPE || (!filesonly && save_frp->cdrType == DIRTYPE))
                    {
                        if(--count == 0)
                            done = true;
                    }
                }
            }
        }
        if(err == noErr)
        {
            if(count == 0)
            {
                *frpp = save_frp;
                *catkeypp = (catkey *)save_entryp;
            }
            else
                err = fnfErr;
        }
    }
    ROMlib_index_cached = (err == noErr);
    fs_err_hook(err);
    return err;
}

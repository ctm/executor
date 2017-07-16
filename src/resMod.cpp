/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_resMod[] =
	    "$Id: resMod.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in ResourceMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "ToolboxUtil.h"
#include "rsys/resource.h"
#include "rsys/glue.h"
#include "rsys/mman.h"
#include "rsys/file.h"


using namespace Executor;
using namespace ByteSwap;

typedef res_sorttype_t *sorttypeptr;
MAKE_HIDDEN(sorttypeptr);
typedef HIDDEN_sorttypeptr *sorttypehand;


namespace Executor {
	PRIVATE LONGINT addtype(resmaphand, ResType);
	PRIVATE LONGINT addname(resmaphand, StringPtr);
	PRIVATE OSErr writemap(resmaphand);
	PRIVATE void fillst(sorttypehand, resref *, resref *);
	PRIVATE void getdat(INTEGER, LONGINT, LONGINT, Handle *);
	PRIVATE void putdat(INTEGER, LONGINT, LONGINT *, Handle);
	PRIVATE LONGINT walkst(res_sorttype_t *, res_sorttype_t *, INTEGER, LONGINT);
	PRIVATE void compactdata(resmaphand);

}

P3(PUBLIC pascal trap, void, SetResInfo, Handle, res, INTEGER, id,
							       StringPtr, name)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    char *sp;
    INTEGER sl;
    OSErr err;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != noErr)
        return;
    if (rr->ratr & resProtected) {
        ROMlib_setreserr(resAttrErr);    /* IV-18 */
        return;
    }
    rr->rid = BigEndianValue(id);
    if (name) {
	sl = U(name[0]);
        if (U(*(sp = (char *)STARH(map) + Hx(map, namoff) + Cx(rr->noff))) < sl ||
							  rr->noff == -1) {
            SetHandleSize((Handle) map, Hx(map, rh.maplen) + sl + 1);
	    err = MemError();
            if (ROMlib_setreserr(err))
                return;
            rr->noff = BigEndianValue(Hx(map, rh.maplen) - Hx(map, namoff));
            HxX(map, rh.maplen) = BigEndianValue(Hx(map, rh.maplen) + sl + 1);
	    sp = (char *)STARH(map) + Hx(map, namoff) + Cx(rr->noff);
	    warning_unimplemented ("we leak space here");
        }
        str255assign(sp, name);
    }
}

P2(PUBLIC pascal trap, void, SetResAttrs, Handle, res, INTEGER, attrs)
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != noErr)
        return;
    rr->ratr = attrs;
}

P1(PUBLIC pascal trap, void, ChangedResource, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    Size oldsize, newsize;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != noErr)
        return;
    if (rr->ratr & resProtected) {
        ROMlib_setreserr(resAttrErr);    /* IV-18 */
        return;
    }
    rr->ratr |= resChanged;
    HxX(map, resfatr) |= CWC(mapChanged);
    if (rr->doff[0] != 0xff || rr->doff[1] != 0xff || rr->doff[2] != 0xff) {
	oldsize = ROMlib_SizeResource(res, FALSE);
	newsize = GetHandleSize((Handle) MR(rr->rhand));
	if (newsize > oldsize) {
	    HxX(map, resfatr) |= CWC(mapCompact);
	    rr->doff[0] = rr->doff[1] = rr->doff[2] = 0xff;
	}
    }
}

A2(PRIVATE, LONGINT, addtype, resmaphand, map, ResType, typ)
{
    typref *tr, t;
    INTEGER i;
    LONGINT off;
    
    WALKTR(map, i, tr)
        if (tr->rtyp == BigEndianValue(typ))
            return((LONGINT)((char *)tr - (char *)STARH(map)));
    EWALKTR(tr)
    ROMlib_invalar();
    t.rtyp = BigEndianValue(typ);
    t.nres = CWC(-1);
    t.rloff = BigEndianValue(NAMEOFF(map) - TYPEOFF(map));
    off = (LONGINT)((char *)tr - (char *) STARH(map));
    Munger((Handle) map, off, (Ptr) "", (LONGINT)0, (Ptr) &t, sizeof(t));
    NUMTMINUS1X(map) = BigEndianValue(NUMTMINUS1(map) + 1);
    MAPLENX(map) = BigEndianValue(MAPLEN(map) + sizeof(t));
    NAMEOFFX(map) = BigEndianValue(NAMEOFF(map) + sizeof(t));
    WALKTR(map, i, tr)
        tr->rloff = BigEndianValue(BigEndianValue(tr->rloff) + sizeof(t));
    EWALKTR(tr)
    return(off);
}

A2(PRIVATE, LONGINT, addname, resmaphand, map, StringPtr, name)
{
  LONGINT retval;
    
  if (!name || !name[0])
    retval = -1;
  else
    {
      LONGINT namelen;
      
      namelen = U(name[0]) + 1;
      
      Munger((Handle) map, MAPLEN(map), (Ptr) "", (LONGINT)0, (Ptr) name,
	     namelen);
      retval = MAPLEN(map) - NAMEOFF(map);
      MAPLENX(map) = BigEndianValue(MAPLEN(map) + namelen);
    }
  return retval;
}

P4(PUBLIC pascal trap, void, AddResource, Handle, data, ResType, typ,
						  INTEGER, id, StringPtr, name)
{
    LONGINT toff, roff;
    typref *tr, *tr2;
    resref r, *rr;
    resmaphand map;
    INTEGER i, j;
    
    if (!data) {
        ROMlib_setreserr(addResFailed);
        return;
    }
    ROMlib_setreserr(noErr);
    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if (!map) {
        ROMlib_setreserr(resFNotFound);
        return;
    }
    if (ROMlib_findmapres(map, data, &tr, &rr) == noErr) {
	ROMlib_setreserr(addResFailed);
/*-->*/	return;
    }
    ROMlib_setreserr(noErr);
    toff = addtype(map, typ);
    r.rid = BigEndianValue(id);
    r.noff = BigEndianValue(addname(map, name));
    r.ratr = CB(resChanged);
    r.doff[0] = r.doff[1] = r.doff[2] = 0xff;
    HxX(map, resfatr) |= CWC(mapChanged);
    r.rhand = RM(data);
    HSetRBit (data);
    tr = (typref *)((char *)STARH(map) + toff);
#if 1
    WALKRR(map, tr, j, rr)
        if (Cx(rr->rid) > id)
            break;
    EWALKRR(rr)
#else /* 0 */
    rr = (resref *)((char *)STARH(map) + Hx(map, typoff) + Cx(tr->rloff));
#endif /* 0 */
    roff = (LONGINT)((char *)rr - (char *) STARH(map));
    /* roff is from the beginning of the map */
    Munger((Handle) map, roff, (Ptr) "", (LONGINT) 0, (Ptr) &r,
							       sizeof(resref));
    roff -= TYPEOFF(map);
    /* roff is from the beginning of the typelist */
    MAPLENX(map)  = BigEndianValue(MAPLEN(map) + sizeof(resref));
    NAMEOFFX(map) = BigEndianValue(NAMEOFF(map) + sizeof(resref));
    tr2 = (typref *)((char *)STARH(map) + toff);
    tr2->nres = BigEndianValue(BigEndianValue(tr2->nres) + 1);
    WALKTR(map, i, tr)
        if (Cx(tr->rloff) >= roff && tr != tr2)
    /*  if (Cx(tr->rloff) > roff)  would probably work, but I'm chicken
								      -- ctm */
            tr->rloff = BigEndianValue(BigEndianValue(tr->rloff) + sizeof(resref));
    EWALKTR(tr)
}

P1(PUBLIC pascal trap, void, RmveResource, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;
    LONGINT troff, rroff, nmoff, nlen;
    INTEGER i, j, tloss;

#if !defined (LETGCCWAIL)
    nlen = 0;
#endif /* LETGCCWAIL */
    
    if (!res) {
        ROMlib_setreserr(rmvResFailed);
/*-->*/ return;
    }
    map = ROMlib_rntohandl(Cx(CurMap), (Handle *)0);
    if (!map) {
        ROMlib_setreserr(resFNotFound);
/*-->*/ return;
    }
    ROMlib_setreserr(ROMlib_findmapres(map, res, &tr, &rr));
    if (ResErr != noErr)
/*-->*/ return;
    if (rr->ratr & resProtected) {
	ROMlib_setreserr(rmvResFailed);
/*-->*/	return;
    }
    HSetState(res, HGetState(res) & ~RSRCBIT);
    troff = (char *)tr - (char *) STARH(map);
    rroff = (char *)rr - (char *) STARH(map);
    if (rr->noff != -1) {
        nmoff = Cx(rr->noff) + NAMEOFF(map);
        nlen = U(*((char *)STARH(map) + nmoff)) + 1;
        Munger((Handle) map, nmoff, (Ptr)0, nlen, (Ptr) "", (LONGINT) 0);
        nmoff -= NAMEOFF(map);
        MAPLENX(map) = BigEndianValue(MAPLEN(map) - nlen);
    } else
        nmoff = 0x7fff;
    HxX(map, resfatr) |= CWC(mapChanged|mapCompact);
    Munger((Handle) map, rroff, (Ptr)0, (LONGINT)sizeof(resref),
						        (Ptr) "", (LONGINT) 0);
    rroff -= TYPEOFF(map);
    MAPLENX(map) = BigEndianValue(MAPLEN(map) - sizeof(resref));
    NAMEOFFX(map) = BigEndianValue(NAMEOFF(map) - sizeof(resref));
    tr = (typref *)((char *) STARH(map) + troff);
    tr->nres = BigEndianValue(BigEndianValue(tr->nres) - 1);
    if (tr->nres == -1) {
        ROMlib_invalar();
        NUMTMINUS1X(map) = BigEndianValue(NUMTMINUS1(map) - 1);
        Munger((Handle) map, troff, (Ptr)0, (LONGINT)sizeof(typref), (Ptr) "",
                                                            (LONGINT) 0);
        tloss = sizeof(typref);
        MAPLENX(map)  = BigEndianValue(MAPLEN(map)  - sizeof(typref));
        NAMEOFFX(map) = BigEndianValue(NAMEOFF(map) - sizeof(typref));
    } else
        tloss = 0;
    WALKTR(map, i, tr)
        if (Cx(tr->rloff) > rroff)
            tr->rloff = BigEndianValue(BigEndianValue(tr->rloff) - sizeof(resref));
	tr->rloff = BigEndianValue(BigEndianValue(tr->rloff) - tloss);
        WALKRR(map, tr, j, rr)
            if (Cx(rr->noff) > nmoff)
                rr->noff = BigEndianValue(BigEndianValue(rr->noff) - nlen);
        EWALKRR(rr)
    EWALKTR(tr)
}

/* writemap:  given a map, writemap writes it out */

A1(PRIVATE, OSErr, writemap, resmaphand, map)
{
    OSErr terr;
    LONGINT lc;

    terr = SetFPos(Hx(map, resfn), fsFromStart, 0L);
    if (terr != noErr)
        return(terr);
    lc = sizeof(reshead);
    terr = FSWriteAll(Hx(map, resfn), &lc, (Ptr) &(HxX(map, rh)));
    if (terr != noErr)
        return(terr);
    terr = SetFPos(Hx(map, resfn), fsFromStart, Hx(map, rh.rmapoff));
    if (terr != noErr)
        return(terr);
    lc = Hx(map, rh.maplen);
    terr = FSWriteAll(Hx(map, resfn), &lc, (Ptr) STARH(map));
    if (terr == noErr)
        HxX(map, resfatr) &= CWC(~(mapChanged));
    return(terr);
}

/* ROMlib_wr: ROMlib_wr is the helper routine for WriteResource, the
	      resource file associated with map is updated to include
	      the resource pointed to by rr */

A2(PUBLIC, void, ROMlib_wr, resmaphand, map, resref *, rr)	/* INTERNAL */
{
    LONGINT rsize, newloc, lc, swappedrsize;
    Handle res;

    res = (Handle) MR(rr->rhand);
    if ((rr->ratr & resChanged) && res) {
        rsize = GetHandleSize(res);
        if (rr->doff[0] == 0xFF && rr->doff[1] == 0xFF &&
						         rr->doff[2] == 0xFF) {
            newloc = Hx(map, rh.rmapoff) - Hx(map, rh.rdatoff);
            HxX(map, rh.rmapoff) = BigEndianValue(Hx(map, rh.rmapoff) + rsize +
							      sizeof(LONGINT));
            HxX(map, rh.datlen)  = BigEndianValue(Hx(map, rh.datlen)  + rsize +
							      sizeof(LONGINT));
            B3ASSIGN(rr->doff, newloc);
        } else
            newloc = B3TOLONG(rr->doff);
        ROMlib_setreserr(SetFPos(Hx(map, resfn), fsFromStart, Hx(map, rh.rdatoff)
                                                                + newloc));
        if (ResErr != noErr)
            return;
        lc = sizeof(LONGINT);
	swappedrsize = BigEndianValue(rsize);
        ROMlib_setreserr(FSWriteAll(Hx(map, resfn), &lc, (Ptr) &swappedrsize));
        if (ResErr != noErr)
            return;
        lc = rsize;
        ROMlib_setreserr(FSWriteAll(Hx(map, resfn), &lc, STARH(res)));
        if (ResErr != noErr)
            return;
        rr->ratr &= ~resChanged;
    } else
	ROMlib_setreserr(noErr);
}

A3(PRIVATE, void, fillst, sorttypehand, st, resref *, rp, resref *, rep)
{
    res_sorttype_t *end = STARH(st), *sp;
    LONGINT newoff;
    
    end++->diskoff = -1;    /* real disk offsets are always > 0 */
    while (rp != rep) {
        newoff = B3TOLONG(rp->doff);
#if 0 /* compiler botch if we leave the next line in */
	gui_assert(newoff >= 0);
#endif
        for (sp = end-1; newoff < sp->diskoff; sp--)
            ;
        sp++;
        BlockMove((Ptr) sp, (Ptr) (sp+1), (Size)(end - sp) * sizeof(res_sorttype_t));
        sp->diskoff = newoff;
        sp->rrptr = rp;
        end++;
        rp++;
    }
}

A4(PRIVATE, void, getdat, INTEGER, fn, LONGINT, datoff, LONGINT, doff,
								Handle *, h)
{
    LONGINT size, lc;
    
    gui_assert(datoff >= 0);
    gui_assert(doff >= 0);
    SetFPos(fn, fsFromStart, datoff + doff);
    lc = sizeof(LONGINT);
    FSReadAll(fn, &lc, (Ptr) &size);
    size = BigEndianValue(size);
    gui_assert(size >= 0);
    *h = NewHandle(size);
    gui_assert(*h);
    HSetState(*h, RSRCBIT);
    lc = size;
    HLock(*h);
    FSReadAll(fn, &lc, STARH(*h));
    gui_assert(lc == size);
    HUnlock(*h);
}

A4(PRIVATE, void, putdat, INTEGER, fn, LONGINT, datoff, LONGINT *, doffp,
								    Handle, h)
{
    LONGINT size = GetHandleSize(h), lc, swappedsize;
    
    gui_assert(size >= 0);
    gui_assert(datoff >= 0);
    gui_assert(*doffp >= 0);
    SetFPos(fn, fsFromStart, datoff + *doffp);
    lc = sizeof(LONGINT);
    swappedsize = BigEndianValue(size);
    FSWriteAll(fn, &lc, (Ptr) &swappedsize);
    lc = size;
    HLock(h);
    FSWriteAll(fn, &lc, STARH(h));
    HUnlock(h);
    *doffp += sizeof(LONGINT) + size;
}

A4(PRIVATE, LONGINT, walkst, res_sorttype_t *, sp, res_sorttype_t *, sep, INTEGER, fn,
							    LONGINT, datoff)
{
    LONGINT size;
    LONGINT doff;
    Handle h;
    LONGINT lc;
    
    doff = 0;
    while (sp != sep) {
        B3ASSIGN(sp->rrptr->doff, doff);
#if 0
        if (sp->rrptr->ratr & resChanged) {
            putdat(fn, datoff, &doff, (Handle) BigEndianValue(sp->rrptr->rhand));
        } else if (!sp->rrptr->rhand || !*(Handle)BigEndianValue(sp->rrptr->rhand)) {
	    getdat(fn, datoff, sp->diskoff, &h);
	    putdat(fn, datoff, &doff,  h);
	    DisposHandle(h);
	} else if (doff != sp->diskoff)
	    putdat(fn, datoff, &doff, (Handle) BigEndianValue(sp->rrptr->rhand));
        else
	    doff += GetHandleSize((Handle) BigEndianValue(sp->rrptr->rhand)) +
							       sizeof(LONGINT);
#else
        if (sp->rrptr->ratr & resChanged)
	  {
            putdat(fn, datoff, &doff, (Handle) MR(sp->rrptr->rhand));
	    sp->rrptr->ratr &= ~resChanged;
	  }
        else if (doff == sp->diskoff) {
	    /* read and advance */
	    SetFPos(fn, fsFromStart, datoff + doff);
	    lc = sizeof(LONGINT);
	    FSReadAll(fn, &lc, (Ptr) &size);
	    gui_assert(lc == sizeof(LONGINT));
	    size = BigEndianValue(size);
	    gui_assert(size >= 0);
	    doff += size + sizeof(LONGINT);
	} else {
	    getdat(fn, datoff, sp->diskoff, &h);
	    putdat(fn, datoff, &doff,  h);
	    HSetState(h, HGetState(h) & ~RSRCBIT);
	    DisposHandle(h);
	}
#endif
        sp++;
    }
    return doff;
}

A1(PRIVATE, void, compactdata, resmaphand, map)
{
    INTEGER nres;
    LONGINT resoff, datlen;
    sorttypehand st;
    resref *rr;
    INTEGER mapstate, ststate;
    
    resoff = TYPEOFF(map) + sizeof(INTEGER) +
                                (NUMTMINUS1(map) + 1) * sizeof(typref);
    nres = (NAMEOFF(map) - resoff) / sizeof(resref);
    st = (sorttypehand) NewHandle((Size)sizeof(res_sorttype_t) * (nres + 1));
    mapstate = HGetState((Handle) map);
    HLock((Handle) map);
    rr = (resref *)((char *) STARH(map) + resoff);
    fillst(st, rr, rr + nres);
    ststate = HGetState((Handle) st);
    HLock((Handle) st);
    datlen = walkst(STARH(st) + 1, STARH(st) + nres + 1, Hx(map, resfn),
                                             Hx(map, rh.rdatoff));
    HSetState((Handle) st, ststate);
    HSetState((Handle) map, mapstate);
    HxX(map, resfatr) |= CWC(mapChanged);
    HxX(map, resfatr) &= CWC(~mapCompact);
    HxX(map, rh.rmapoff) = BigEndianValue(sizeof(reshead) + sizeof(rsrvrec) + datlen);
    HxX(map, rh.datlen) = BigEndianValue(datlen);
    DisposHandle((Handle) st);
}

P1(PUBLIC pascal trap, void, UpdateResFile, INTEGER, rn)
{
    resmaphand map;
    INTEGER i, j;
    typref *tr;
    resref *rr;
    BOOLEAN needtowalk;
    fcbrec *fp;
    OSErr err;
    ioParam iopb;

    map = ROMlib_rntohandl(rn, (Handle *)0);
    if (!map) {
        ROMlib_setreserr(resFNotFound);
        return;
    }
    ROMlib_setreserr (noErr);
/*
 * NOTE:  This implementation looks more TRUE to what IMI-125 implies than
 *	  the one (above) it replaces.
 */
    fp = PRNTOFPERR(Hx(map, resfn), &err);
    if (err == noErr && (fp->fcflags & fcwriteperm)) {
	needtowalk = TRUE;
	if (HxX(map, resfatr) & (CWC (mapCompact))) {
	    compactdata(map);
	    needtowalk = FALSE;
	}
	if (HxX(map, resfatr) & (CWC (mapChanged))) {
	    if (needtowalk) {
		WALKTANDR(map, i, tr, j, rr)
		    ROMlib_wr(map, rr);
		    if (ResErr != noErr)
			return;
		EWALKTANDR(tr, rr)
	    }
	    ROMlib_setreserr(writemap(map));
	    if (ResErr != noErr)
		return;
	}
	iopb.ioRefNum = HxX(map, resfn);
	PBFlushFile((ParmBlkPtr) &iopb, FALSE);
    }
}

P1(PUBLIC pascal trap, void, WriteResource, Handle, res)
{
    resmaphand map;
    typref *tr;
    resref *rr;

    ROMlib_setreserr(ROMlib_findres(res, &map, &tr, &rr));
    if (ResErr != noErr)
        return;
    ROMlib_wr(map, rr);
}

P1(PUBLIC pascal trap, void, SetResPurge, BOOLEAN, install)
{
    /* NOP */
  ROMlib_setreserr (noErr);
}

#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>

PUBLIC filecontrolblock *getfreefcbp( void )
{
    short length;
    filecontrolblock *fcbp, *efcbp;
    
    length = *(short *)FCBSPtr;
    fcbp = (filecontrolblock *) ((short *)FCBSPtr+1);
    efcbp = (filecontrolblock *) ((char *)FCBSPtr + length);
    for (;fcbp < efcbp && fcbp->fcbFlNum;
			  fcbp = (filecontrolblock *) ((char *)fcbp + FSFCBLen))
	;
    return fcbp < efcbp ? fcbp : 0;
}
    
PUBLIC filecontrolblock *refnumtofcbp(short refnum)
{
    INTEGER len;
    
    if (refnum < sizeof(short) || refnum % FSFCBLen != sizeof(short))
	return 0;
    len = *(short *)FCBSPtr;
    if (refnum >= len)
	return 0;
    return (filecontrolblock *)((char *)FCBSPtr + refnum);
}

PRIVATE long pbabsoffset(ioParam *pb, filecontrolblock *fcbp)
{
    switch (pb->ioPosMode) {
    case fsAtMark:
	return fcbp->fcbCrPs;
    case fsFromStart:
	return pb->ioPosOffset;
    case fsFromLEOF:
	return fcbp->fcbEOF + pb->ioPosOffset;
    case fsFromMark:
	return fcbp->fcbCrPs + pb->ioPosOffset;
    default:
	return -1;
    }
}

PRIVATE long xtntbnotophys(xtntrec xtr, unsigned short bno, short *nphyscontigp)
{
    unsigned short bc;
    long retblock;
    
    bc = xtr[0].blockcount;
    if (bno < bc) {
	*nphyscontigp = bc - bno;
	retblock = xtr[0].blockstart + bno;
    } else {
	bno -= bc;
	bc = xtr[1].blockcount;
	if (bno < bc) {
	    *nphyscontigp = bc - bno;
	    retblock = xtr[1].blockstart + bno;
	} else {
	    bno -= bc;
	    bc = xtr[2].blockcount;
	    if (bno < bc) {
		*nphyscontigp = bc - bno;
		retblock = xtr[2].blockstart + bno;
	    } else
		retblock = -1;
	}
    }
    return retblock;
}

PUBLIC compretval xtntcompare(void *firstp, void *secondp)
{
    xtntkey *xp1, *xp2;
    
    xp1 = firstp;
    xp2 = secondp;
    if (xp1->xkrFkType < xp2->xkrFkType)
	return firstisless;
    else if (xp1->xkrFkType > xp2->xkrFkType)
	return firstisgreater;
    else {
	if (xp1->xkrFNum < xp2->xkrFNum)
	    return firstisless;
	else if (xp1->xkrFNum > xp2->xkrFNum)
	    return firstisgreater;
	else {
	    if (xp1->xkrFABN < xp2->xkrFABN)
		return firstisless;
	    else if (xp1->xkrFABN > xp2->xkrFABN)
		return firstisgreater;
	    else
		return same;
	}
    }
}

PUBLIC compretval catcompare(void *firstp, void *secondp)
{
    catkey *ckp1, *ckp2;
    
    ckp1 = firstp;
    ckp2 = secondp;
    if (ckp1->ckrParID < ckp2->ckrParID)
	return firstisless;
    if (ckp1->ckrParID > ckp2->ckrParID)
	return firstisgreater;
    else
	return RelString((StringPtr) ckp1->ckrCName, (StringPtr) ckp2->ckrCName,
								   FALSE, TRUE);
}

PUBLIC void makextntkey(xtntkey *keyp, forktype forkwanted, LONGINT flnum,
								     ushort bno)
{
    keyp->xkrKeyLen = 7;
    keyp->xkrFkType = forkwanted;
    keyp->xkrFNum = flnum;
    keyp->xkrFABN = bno;
}

PUBLIC void makextntparam(btparam *btpb, HVCB *vcbp, forktype forkwanted,
						      LONGINT flnum, ushort bno)
{
    btpb->vcbp = vcbp;
    makextntkey((xtntkey *) &btpb->tofind, forkwanted, flnum, bno);
    btpb->fp = xtntcompare;
    btpb->refnum = vcbp->vcbXTRef;
    btpb->leafindex = -1;
}

PRIVATE xtntkey *fcbpbnotoxkeyp(filecontrolblock *fcbp, ushort bno)
{
    forktype forkwanted;
    xtntkey *xkeyp;
    btparam btparamblock;
    OSErr err;
    
    forkwanted = fcbp->fcbMdRByt & RESOURCEBIT ? resourcefork : datafork;
    makextntparam(&btparamblock, fcbp->fcbVPtr, forkwanted, fcbp->fcbFlNum,
									   bno);
    err = keyfind(&btparamblock);
#if 0
    cleancache(btparamblock.vcbp);
#endif
    xkeyp = (xtntkey *) btparamblock.foundp;
    if (err != noErr || xkeyp->xkrFkType != forkwanted ||
					       xkeyp->xkrFNum != fcbp->fcbFlNum)
	return 0;
    return xkeyp;
}

PUBLIC long logtophys(filecontrolblock *fcbp, long absoffset,
							    short *nphyscontigp)
{
    ushort bno;
    HVCB *vcbp;
    long alblksiz, retblock;
    xtntkey *xkeyp;
    
    vcbp = (HVCB *) fcbp->fcbVPtr;
    alblksiz = vcbp->vcbAlBlkSiz;
    bno = absoffset / alblksiz;
    retblock = xtntbnotophys(fcbp->fcbExtRec, bno, nphyscontigp);
    if (retblock == -1) {
	xkeyp = fcbpbnotoxkeyp(fcbp, bno);
	if (!xkeyp)
	    return -1;
	retblock = xtntbnotophys((xtntdesc *) DATAPFROMKEY(xkeyp),
					    bno - xkeyp->xkrFABN, nphyscontigp);
	if (retblock == -1)
	    return -1;
    }
    return vcbp->vcbAlBlSt * PHYSBSIZE + retblock * alblksiz;
}

PRIVATE OSErr pbtofcbp(ioParam *pb, filecontrolblock **fcbpp, accesstype rw)
{
    OSErr retval;
    
    *fcbpp = refnumtofcbp(pb->ioRefNum);
    if (!*fcbpp)
	retval = rfNumErr;
    else {
	if (rw == writing) {
	    retval = writefcbp(*fcbpp);
	    if (retval == noErr)
		retval = writevcbp((*fcbpp)->fcbVPtr);
	} else
	    retval = noErr;
    }
    return retval;
}

PRIVATE void setbits(HVCB *vcbp, ulong bno, ulong ntoset, unsigned char lookfor)
{
    unsigned char *cp, *ecp;
    ulong ebno;
    INTEGER startbit, stopbit;
    unsigned char mask, want;
    
    if (lookfor)
    	vcbp->vcbFreeBks -= ntoset;
    else
    	vcbp->vcbFreeBks += ntoset;
    vcbp->vcbFlags |= VCBDIRTY;
#if 0
    assert(0);
#endif
    /* bno -= vcbp->vcbAlBlSt; not sure about this */
    ebno = bno + ntoset;
    cp  = (unsigned char *) vcbp->vcbMAdr +  bno / 8 + MADROFFSET;
    ecp = (unsigned char *) vcbp->vcbMAdr + ebno / 8 + MADROFFSET;
    startbit =  bno % 8;
    stopbit  = ebno % 8;
    if (cp == ecp) {
	mask = (0xFF >> startbit) & (0xFF << (8 - stopbit));
	if (lookfor)
	    *cp |= mask;
	else
	    *cp &= ~mask;
    } else {
	if (startbit) {
	    mask = 0xFF >> startbit;
	    if (lookfor)
		*cp++ |= mask;
	    else
		*cp++ &= ~mask;
	}
	want = lookfor ? 0xFF : 0;
	while (cp != ecp)
	    *cp++ = want;
	if (stopbit) {
	    mask = 0xFF << (8 - stopbit);
	    if (lookfor)
		*cp |= mask;
	    else
		*cp &= ~mask;
	}
    }
    TransPhysBlk(vcbp, vcbp->vcbVBMSt * (ulong) PHYSBSIZE, 1,
    			       vcbp->vcbMAdr + MADROFFSET, writing, (long *) 0);

}

PRIVATE ulong countbits(HVCB *vcbp, ulong bno, unsigned char lookfor)
{
    unsigned char *cp, c;
    unsigned char mask, want;
    ulong retval, max;
    INTEGER bit;
    
    /* assert(lookfor <= 1); */
    /* bno -= vcbp->vcbAlBlSt; not sure about this */
    retval = 0;
    max = vcbp->vcbNmAlBlks - bno;
    cp = (unsigned char *) vcbp->vcbMAdr + bno / 8 + MADROFFSET;
#if 0
    {
    	Size madrlen;
    	madrlen = GetPtrSize(vcbp->vcbMAdr);
    	madrlen = madrlen;
    }
#endif
    bit = bno % 8;
    if (bit) {
	c = *cp++;
	mask = (unsigned char) ~0 >> bit;
	want = lookfor ? mask : 0;
	if ((c & mask) == want) {
	    retval += 8 - bit;
	} else {
	    mask = 1 << (7 - bit);
	    if (lookfor)
		for (; c & mask; mask >>= 1)
		    retval++;
	    else
		for (; !(c & mask); mask >>= 1)
		    retval++;
	    return retval;
	}
    }
    want = lookfor ? 0xFF : 0;
    while (retval < max && *cp++ == want) {
	retval += 8;
    }
    if (retval < max) {
	c = cp[-1];
	mask = 1 << 7;
	if (lookfor)
	    for (; c & mask; mask >>= 1)
		retval++;
	else
	    for (; !(c & mask); mask >>= 1)
		retval++;
    }
    return MIN(max, retval);
}

PRIVATE BOOLEAN fillextent(xtntdesc *xp, LONGINT *nallocneededp, HVCB *vcbp,
								ushort *newabnp)
{
    INTEGER nempty;
    ulong toextend;
    ulong needed, max, nfree, search;
    xtntrec tmpxtnt;
    xtntdesc *tmpxp;
    BOOLEAN retval;
    
    needed = *nallocneededp;
    max = vcbp->vcbNmAlBlks;
    for (nempty = 3; nempty > 0 && xp->blockcount; nempty--, xp++)
	*newabnp += xp->blockcount;
    if (nempty != 3) {
	--xp;
	toextend = countbits(vcbp, xp->blockstart + xp->blockcount, 0);
	if (toextend) {
	    toextend = MIN(toextend, needed);
	    needed -= toextend;
	    setbits(vcbp, xp->blockstart + xp->blockcount, toextend, 1);
	    xp->blockcount += toextend;
	}
	++xp;
    }
    if (needed && nempty) {
	tmpxtnt[0].blockcount = 0;
	tmpxtnt[1].blockcount = 0;
	tmpxtnt[2].blockcount = 0;
	nfree = 0;
/*
 * NOTE: the condition in the for loop might look strange.  The point is it
 *       isn't worth counting the number of free blocks from a given point if
 *       even in the best case it couldn't be more than the number you already
 *       know about.
 */
	for (search = countbits(vcbp, 0, 1) /*vcbp->vcbAlBlSt*/; max - search > tmpxtnt[2].blockcount;) {
	    nfree = countbits(vcbp, search, 0);
#if 1
	    if (nfree == 0)
	        DebugStr((StringPtr) "\pnfree == 0");
#endif
	    if (nfree >= needed) {
		tmpxtnt[0].blockcount = nfree;
		tmpxtnt[0].blockstart = search;
		break;
	    }
	    if (nfree > tmpxtnt[1].blockcount) {
		if (nfree > tmpxtnt[0].blockcount) {
		    tmpxtnt[2].blockcount = tmpxtnt[1].blockcount;
		    tmpxtnt[2].blockstart = tmpxtnt[1].blockstart;
		    tmpxtnt[1].blockcount = tmpxtnt[0].blockcount;
		    tmpxtnt[1].blockstart = tmpxtnt[0].blockstart;
		    tmpxtnt[0].blockcount = nfree;
		    tmpxtnt[0].blockstart = search;
		} else {
		    tmpxtnt[2].blockcount = tmpxtnt[1].blockcount;
		    tmpxtnt[2].blockstart = tmpxtnt[1].blockstart;
		    tmpxtnt[1].blockcount = nfree;
		    tmpxtnt[1].blockstart = search;
		}
	    } else if (nfree > tmpxtnt[2].blockcount) {
		tmpxtnt[2].blockcount = nfree;
		tmpxtnt[2].blockstart = search;
	    }
	    search += nfree;
	    search += countbits(vcbp, search, 1);
	}
	tmpxp = tmpxtnt;
	while (needed > 0 && --nempty >= 0) {
	    xp->blockstart = tmpxp->blockstart;
	    xp->blockcount = MIN(tmpxp->blockcount, needed);
#if 1
	    if (xp->blockcount == 0)
	        DebugStr((StringPtr) "\pblockcount = 0");
#endif
	    needed -= xp->blockcount;
	    setbits(vcbp, xp->blockstart, xp->blockcount, 1);
	}
    }
    *newabnp += *nallocneededp - needed;
    retval = needed < *nallocneededp;
    *nallocneededp = needed;
    return retval;
}

PRIVATE void smokexpvcbp(ulong tosmoke, xtntdesc *xp, HVCB *vcbp)
{
    if (tosmoke <= xp[2].blockcount) {
	xp[2].blockcount -= tosmoke;
	setbits(vcbp, xp[2].blockstart + xp[2].blockcount, tosmoke, 0);
    } else {
	setbits(vcbp, xp[2].blockstart, xp[2].blockcount, 0);
	tosmoke -= xp[2].blockcount;
	xp[2].blockcount = 0;
	if (tosmoke <= xp[1].blockcount) {
	    xp[1].blockcount -= tosmoke;
	    setbits(vcbp, xp[1].blockstart + xp[1].blockcount, tosmoke, 0);
	} else {
	    setbits(vcbp, xp[1].blockstart, xp[1].blockcount, 0);
	    tosmoke -= xp[1].blockcount;
	    xp[1].blockcount = 0;
	    xp[0].blockcount -= tosmoke;
	    setbits(vcbp, xp[0].blockstart + xp[0].blockcount, tosmoke, 0);
	}
    }
}

PUBLIC OSErr AllocHelper(ioParam *pb, BOOLEAN async, alloctype alloc,
							      BOOLEAN writefcbp)
{
    filecontrolblock *fcbp;
    OSErr err, err1;
    ulong neweof, totallength;
    ushort clumpsize, newabn;
    HVCB *vcbp;
    ulong tosmoke;
    LONGINT nallocneeded;
    xtntdesc *xp;
    xtntkey *xtkeyp;
    xtntrec saverec;
    Byte savetype;
    btparam btparamrec;
	
    err = pbtofcbp(pb, &fcbp, writing);
    if (err != noErr)
/*-->*/ PBRETURN(pb, err);

    switch (alloc) {
    case seteof:
	neweof = (ulong) pb->ioMisc;
	break;
    case allocany:
    case alloccontig:
	neweof = fcbp->fcbPLen + pb->ioReqCount;
	break;
    default:
        DebugStr((StringPtr) "\punknown allocator1");
	PBRETURN(pb, fsDSIntErr);
    }
    if (neweof < 0)
	neweof = 0;
    clumpsize = fcbp->fcbClmpSize;
    vcbp = (HVCB *) fcbp->fcbVPtr;
    if (!clumpsize)
	clumpsize = vcbp->vcbClpSiz;
    clumpsize = vcbp->vcbAlBlkSiz;	/* i.e. ignore clumpsize; that's what
    					   the Mac really does */
    neweof = (neweof + clumpsize - 1) / clumpsize * clumpsize;
    
    nallocneeded = ((LONGINT) neweof - (LONGINT) fcbp->fcbPLen) /
    							      vcbp->vcbAlBlkSiz;
    xp = fcbp->fcbExtRec;
    totallength = vcbp->vcbAlBlkSiz *
		       (xp[0].blockcount + xp[1].blockcount + xp[2].blockcount);
    if (nallocneeded < 0) {
	if (neweof < totallength) {
	    tosmoke = (totallength - neweof) / vcbp->vcbAlBlkSiz;
	    smokexpvcbp(tosmoke, xp, vcbp);
	    if (writefcbp)
        	fcbp->fcbMdRByt |= DIRTYBIT;
	}
	if (fcbp->fcbPLen > totallength) {
	    newabn = neweof / vcbp->vcbAlBlkSiz;
	    if (!(xtkeyp = fcbpbnotoxkeyp(fcbp, newabn))) {
		neweof = fcbp->fcbPLen;
		DebugStr((StringPtr) "\pcouldn't translate fcbp, newabn 1");
		err = fsDSIntErr;
		goto done;
	    }
	    xp = (xtntdesc *) DATAPFROMKEY(xtkeyp);
	    totallength = vcbp->vcbAlBlkSiz *
		       (xp[0].blockcount + xp[1].blockcount + xp[2].blockcount);
	    tosmoke = xtkeyp->xkrFABN * vcbp->vcbAlBlkSiz + totallength
								       - neweof;
	    savetype = xtkeyp->xkrFkType;
	    if (tosmoke < totallength) {
		smokexpvcbp(tosmoke, xp, vcbp);
		dirtyleaf((anykey *)xtkeyp, vcbp);
		err = btnext((anykey **) &xtkeyp, (anykey *)xtkeyp, vcbp);
		if (err != noErr)
		    goto done;
	    }
	    if (xtkeyp && xtkeyp->xkrFNum == fcbp->fcbFlNum &&
	    				        xtkeyp->xkrFkType == savetype) {
	        memcpy(saverec, DATAPFROMKEY(xtkeyp), (size_t) sizeof(saverec));
	        makextntparam(&btparamrec, vcbp, 0, 0, 0);
	        btparamrec.tofind = *(anykey *) xtkeyp;
	        btparamrec.leafindex = -1;
	        btparamrec.success = TRUE;
	        btparamrec.foundp = (anykey *) xtkeyp;
	    } else
	        btparamrec.success = FALSE;
	    while (btparamrec.success) {
	        xp = (xtntdesc *) DATAPFROMKEY(btparamrec.foundp);
		setbits(vcbp, xp[0].blockstart, xp[0].blockcount, 0);
		setbits(vcbp, xp[1].blockstart, xp[1].blockcount, 0);
		setbits(vcbp, xp[2].blockstart, xp[2].blockcount, 0);
		newabn = btparamrec.tofind.xtntk.xkrFABN + xp[0].blockcount +
							  xp[1].blockcount +
							  xp[2].blockcount;
		err = btdelete(&btparamrec);
		if (err != noErr)
		    goto done;
		btparamrec.tofind.xtntk.xkrFABN = newabn;
		err = keyfind(&btparamrec);
		if (err != noErr)
		    goto done;
	    }
	}
    } else if (nallocneeded > 0) {
	if ((nallocneeded > vcbp->vcbFreeBks) &&
				    (alloc == seteof || alloc == alloccontig)) {
	    neweof = fcbp->fcbPLen;
	    err = dskFulErr;
	    goto done;
	}
	if (fcbp->fcbPLen == totallength) { /* possibly some room in this one */
	    newabn = 0;
	    if (fillextent(xp, &nallocneeded, vcbp, &newabn) && writefcbp)
        	fcbp->fcbMdRByt |= DIRTYBIT;
	} else {
	    newabn = fcbp->fcbPLen / vcbp->vcbAlBlkSiz;
	    if (!(xtkeyp = fcbpbnotoxkeyp(fcbp, newabn))) {
		neweof = fcbp->fcbPLen;
		DebugStr((StringPtr) "\pcouldn't translate fcbp, newabn 2");
		err = fsDSIntErr;
		goto done;
	    }
	    xp = (xtntdesc *) DATAPFROMKEY(xtkeyp);
	    newabn = xtkeyp->xkrFABN;
	    if (fillextent(xp, &nallocneeded, vcbp, &newabn))
		dirtyleaf((anykey *)xtkeyp, vcbp);
	}
	while (nallocneeded > 0) {
	    xtkeyp = newextentrecord(fcbp, newabn);
	    if (!xtkeyp) {
	        DebugStr((StringPtr) "\pnewextent failed");
	        err = fsDSIntErr;
	        goto done;
	    }
	    xp = (xtntdesc *) DATAPFROMKEY(xtkeyp);
	    if (fillextent(xp, &nallocneeded, vcbp, &newabn))
		dirtyleaf((anykey *)xtkeyp, vcbp);
	    /* NOTE: this could loop forever if our count is off or if
	    	     some blocks got eaten when making an extent record.
	       TODO: fix this */
	}
    }
done:
    if (writefcbp)
	err1 = cleancache(vcbp);    /* cleancache must NOT be called if we are trashing */
    else                            /* blocks before we delete a file */
	err1 = noErr;
    switch (alloc) {
    case seteof:
	if (err == noErr)
	    fcbp->fcbEOF = (ulong) pb->ioMisc;
	break;
    case allocany:
    case alloccontig:
	pb->ioActCount = neweof - fcbp->fcbPLen;
	break;
    default:
        DebugStr((StringPtr) "\punknown allocator2");
	PBRETURN(pb, fsDSIntErr);
    }
    fcbp->fcbPLen = neweof;
    fcbp->fcbMdRByt |= 0x80;
    
    if (err == noErr)
	err = err1;
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBSetEOF(ioParam *pb, BOOLEAN async)
{
    return AllocHelper(pb, async, seteof, TRUE);
}

PUBLIC OSErr myPBAllocate(ioParam *pb, BOOLEAN async)
{
    return AllocHelper(pb, async, allocany, TRUE);
}

PUBLIC OSErr myPBAllocContig(ioParam *pb, BOOLEAN async)
{
    return AllocHelper(pb, async, alloccontig, TRUE);
}

#if !defined (MIN)
#define MIN(a, b)   ((a) <= (b) ? (a) : (b))
#endif

#define RETURN(x)   { pb->ioResult = x; goto DONE; }

PRIVATE OSErr PBReadWrite(ioParam *pb, BOOLEAN async, accesstype rw)
{
    filecontrolblock *fcbp;
    long absoffset, totransfer, neweot, templ, physblock, actl;
    OSErr newerr;
    short ntoskip, ntocopy, nphyscontig, nblockstogo, thisrun;
    char tempbuf[PHYSBSIZE];    /* goes away eventually */
    Ptr bufp;
    HVCB *vcbp;
    
    pb->ioResult = noErr;
    pb->ioActCount = 0;
    totransfer = pb->ioReqCount;
    
    newerr = pbtofcbp(pb, &fcbp, rw);
    if (newerr != noErr)
/*-->*/ RETURN(newerr)

    absoffset = pbabsoffset(pb, fcbp);
    if (absoffset < 0)
	RETURN(posErr)
    pb->ioPosOffset = absoffset;
    if (totransfer < 0)
	RETURN(paramErr)
    neweot = absoffset + totransfer;
    if (neweot > fcbp->fcbEOF) {
	if (rw == reading) {
	    totransfer = fcbp->fcbEOF - absoffset;
	    pb->ioResult = eofErr;
	} else {
	    templ = (long) pb->ioMisc;
#if !defined (UNIX)
	    pb->ioMisc = (Ptr) neweot;
#else /* UNIX */
	    pb->ioMisc = (LONGINT) neweot;
#endif /* UNIX */
	    pb->ioResult = myPBSetEOF(pb, FALSE);
	    if (newerr != noErr)
		totransfer = fcbp->fcbPLen - absoffset;
	}
    }
    ntoskip = absoffset % PHYSBSIZE;
    nphyscontig = 0;
    bufp = pb->ioBuffer;
    vcbp = fcbp->fcbVPtr;
#if !defined (LETGCCWAIL)
    physblock = 0;
#endif /* LETGCCWAIL */
    if (ntoskip) {
	absoffset -= ntoskip;
	physblock = logtophys(fcbp, absoffset, &nphyscontig);
	if (nphyscontig < 1)
	    RETURN(ioErr)
	newerr = TransPhysBlk(vcbp, physblock, 1, (Ptr) tempbuf, reading,
								   (long *) 0);
	if (newerr != noErr)
	    RETURN(newerr)
	ntocopy = MIN(totransfer, PHYSBSIZE - ntoskip);
	if (rw == reading)
	    memcpy(bufp, tempbuf + ntoskip, (size_t) ntocopy);
	else {
	    memcpy(tempbuf + ntoskip, bufp, (size_t) ntocopy);
	    newerr = TransPhysBlk(vcbp, physblock, 1, (Ptr) tempbuf, writing,
								    (long *) 0);
	    if (newerr != noErr)
		RETURN(newerr)
	}
	
	pb->ioPosOffset += ntocopy;
	pb->ioActCount += ntocopy;
	totransfer -= ntocopy;
	bufp += ntocopy;
	--nphyscontig;
    }
    if (totransfer >= PHYSBSIZE) {
	nblockstogo = totransfer / PHYSBSIZE;
	while (nblockstogo > 0) {
	    if (nphyscontig == 0) {
		physblock = logtophys(fcbp, absoffset, &nphyscontig);
		if (nphyscontig < 1)
		    RETURN(ioErr)
	    }
	    thisrun = MIN(nphyscontig, nblockstogo);
	    newerr = TransPhysBlk(vcbp, physblock, thisrun, bufp, rw, &actl);
	    pb->ioPosOffset += actl;
	    pb->ioActCount += actl;
	    if (newerr != noErr)
		RETURN(newerr)
	    bufp += thisrun * PHYSBSIZE;
	    absoffset += thisrun * PHYSBSIZE;
	    totransfer -= thisrun * PHYSBSIZE;
	    nblockstogo -= thisrun;
	    nphyscontig = 0;
	}
    }
    if (totransfer > 0) {
	if (nphyscontig == 0) {
	    physblock = logtophys(fcbp, absoffset, &nphyscontig);
	    if (nphyscontig < 1)
		RETURN(ioErr)
	}
	newerr = TransPhysBlk(vcbp, physblock, 1, (Ptr) tempbuf, reading,
							      (long *) 0);
	if (newerr != noErr)
	    RETURN(newerr)
	if (rw == reading)
	    memcpy(bufp, tempbuf, (size_t) totransfer);
	else {
	    memcpy(tempbuf, bufp, (size_t) totransfer);
	    newerr = TransPhysBlk(vcbp, physblock, 1, bufp, writing,
								    (long *) 0);
	    if (newerr != noErr)
		RETURN(newerr)
	}
	pb->ioPosOffset += totransfer;
	pb->ioActCount += totransfer;
    }
    fcbp->fcbCrPs = pb->ioPosOffset;
DONE:
    newerr = cleancache(vcbp);
    if (pb->ioResult == noErr)
        pb->ioResult = newerr;
    return pb->ioResult;
}

#undef RETURN

PUBLIC OSErr myPBRead(ioParam *pb, BOOLEAN async)
{
    return PBReadWrite(pb, async, reading);
}

PUBLIC OSErr myPBWrite(ioParam *pb, BOOLEAN async)
{
    return PBReadWrite(pb, async, writing);
}

PUBLIC OSErr myPBFlushFile(ioParam *pb, BOOLEAN async)
{
    filecontrolblock *fcbp;
    OSErr err;

    fcbp = refnumtofcbp(pb->ioRefNum);
    if (!fcbp)
	err = rfNumErr;
    else
        err = dirtyfcbp(fcbp);
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBClose(ioParam *pb, BOOLEAN async)
{
    filecontrolblock *fcbp;
    OSErr err;

    fcbp = refnumtofcbp(pb->ioRefNum);
    if (!fcbp)
	err = rfNumErr;
    else {
        err = myPBFlushFile(pb, async);
        if (err == noErr)
	    fcbp->fcbFlNum = 0;
    }
    PBRETURN(pb, err);
}

PUBLIC void makecatkey(catkey *keyp, LONGINT dirid, INTEGER namelen, Ptr namep)
{
    keyp->ckrKeyLen = namelen + 1 + sizeof(LONGINT) + 1;
    keyp->ckrResrv1 = 0;
    keyp->ckrParID = dirid;
    keyp->ckrCName[0] = namelen;
    memcpy(keyp->ckrCName+1, namep, (size_t) namelen);
}

PRIVATE OSErr findentry(long dirid, StringPtr name, btparam *btpb,
					    filekind *kindp, BOOLEAN ignorename)
{
    filerec *retval;
    INTEGER namelen;
    unsigned char *namep, *colonp, *endp;
    void *recp;
    short rectype;
    OSErr err;
    
    retval = 0;
    if (ignorename) {
        namelen = 0;
        namep = (StringPtr) "";
    } else {
	namelen = name[0];
	namep = name+1;
    }
    
    for (;;) {
	err = cleancache(btpb->vcbp);
	if (err != noErr)
	    return err;
	colonp = (unsigned char *) indexn((char *) namep, ':', namelen);
	if (colonp)
	    endp = colonp;
	else
	    endp = namep + namelen;
	makecatparam(btpb, btpb->vcbp, dirid, endp-namep, (Ptr) namep);
	err = keyfind(btpb);
	if (err != noErr)
	    return err;
	if (!btpb->success)
	    return colonp ? dirNFErr : fnfErr;
	recp = DATAPFROMKEY(btpb->foundp);
	rectype = *(unsigned char *)recp;
	if (colonp) {   /* expect a directory */
	    switch (rectype) {
	    case DIRTYPE:
		dirid = ((directoryrec *)recp)->dirDirID;
		break;
	    case THREADTYPE:
		if (((catkey *) &btpb->tofind)->ckrCName[0]) {
		    DebugStr((StringPtr) "\pthread with name");
		    return fsDSIntErr;
		}
		dirid = ((threadrec *)recp)->thdParID;
		break;
	    default:
	        DebugStr((StringPtr) "\punknown rectype1 in findentry");
		return fsDSIntErr;
	    }
	    namelen -= ((catkey *) &btpb->tofind)->ckrCName[0]+1;
	    namep   += ((catkey *) &btpb->tofind)->ckrCName[0]+1;
	} else {        /* expect a regular file */
	    switch (rectype) {
	    case FILETYPE:
		if (!(*kindp & regular))
		    return bdNamErr;
		*kindp = regular;
		break;
	    case DIRTYPE:
		if (!(*kindp & directory))
		    return bdNamErr;
		*kindp = directory;
		break;
	    case THREADTYPE:
	        if (ignorename && *kindp == directory) {
		    dirid = ((threadrec *)recp)->thdParID;
		    namep = ((threadrec *)recp)->thdCName+1;
		    namelen = ((threadrec *)recp)->thdCName[0];
		    continue;	/* avoid return below */
	        } else if (!(*kindp & thread))
		    return dirNFErr;
		else
		    *kindp = thread;
		break;
	    default:
	        DebugStr((StringPtr) "\punknown rectype2 in findentry");
		return fsDSIntErr;
	    }
	    return noErr;
	}
    }
}

PUBLIC OSErr findvcbandfile(ioParam *pb, LONGINT dirid, btparam *btpb,
					    filekind *kindp, BOOLEAN ignorename)
{
    OSErr err;
    
    err = noErr;
    btpb->vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, &dirid);
    if (!btpb->vcbp)
	err = nsvErr;
    else {
	adjustdirid(&dirid, btpb->vcbp, pb->ioVRefNum);
	err = findentry(dirid, pb->ioNamePtr, btpb, kindp, ignorename);
    }
    return err;
}

/*
 * alreadyopen checks to see whether a given file is already open and if
 * so whether that causes a conflict (the conflicting refnum is filled in).
 */
 
PUBLIC OSErr alreadyopen(HVCB *vcbp, ulong flnum, SignedByte *permp,
								 short *refnump)
{
    short length;
    filecontrolblock *fcbp, *efcbp;
    SignedByte temp;
    short tempshort;
    
    if (!permp) {
	permp = &temp;
	temp = fsWrPerm;
    }
    
    if (!refnump)
	refnump = &tempshort;
	
    if (*permp == fsRdPerm)
	return noErr;
    length = *(short *)FCBSPtr;
    fcbp = (filecontrolblock *) ((short *)FCBSPtr+1);
    efcbp = (filecontrolblock *) ((char *)FCBSPtr + length);
    for (;fcbp < efcbp; fcbp = (filecontrolblock *) ((char *)fcbp + FSFCBLen))
	if (fcbp->fcbVPtr == vcbp && fcbp->fcbFlNum == flnum &&
			       ((fcbp->fcbMdRByt & WRITEBIT) || permp == &temp))
	    switch (*permp) {
	    case fsCurPerm:
		if (!(fcbp->fcbMdRByt & SHAREDBIT))
		    *permp = fsRdPerm;
		    return noErr;
		break;
	    case fsWrPerm:
	    case fsRdWrPerm:
		*refnump = (char *) fcbp - (char *) FCBSPtr;
		return opWrErr;
		break;
	    case fsRdWrShPerm:
		if (!(fcbp->fcbMdRByt & SHAREDBIT)) {
		    *refnump = (char *) fcbp - (char *) FCBSPtr;
		    return opWrErr;
		}
		break;
	    }
    return noErr;
}

PRIVATE OSErr PBOpenHelper(ioParam *pb, forktype ft, long dirid, BOOLEAN async)
{
    filecontrolblock *fcbp;
    OSErr err;
    SignedByte permssn;
    cacheentry *cachep;
    btparam btparamrec;
    filekind kind;
    filerec *frp;
    catkey *catkeyp;
    
    kind = regular;
    err = findvcbandfile(pb, dirid, &btparamrec, &kind, FALSE);
    if (err != noErr)
	PBRETURN(pb, err);

    err = cleancache(btparamrec.vcbp);
    if (err != noErr)
	PBRETURN(pb, err);
    permssn = pb->ioPermssn;
    frp = (filerec *) DATAPFROMKEY(btparamrec.foundp);
    err = alreadyopen(btparamrec.vcbp, frp->filFlNum, &permssn, &pb->ioRefNum);
    if (err != noErr)
	PBRETURN(pb, err);
    fcbp = getfreefcbp();
    if (!fcbp)
/*-->*/ PBRETURN(pb, tmfoErr);

    fcbp->fcbFlNum = frp->filFlNum;
    if (frp->filFlags & FSOFTLOCKBIT) {
	switch (permssn) {
	case fsCurPerm:
	    permssn = fsRdPerm;
	    break;
	case fsWrPerm:
	case fsRdWrPerm:
	case fsRdWrShPerm:
	    fcbp->fcbFlNum = 0;
	    PBRETURN(pb, permErr);
	}
	fcbp->fcbMdRByt = FLOCKEDBIT;
    } else {
	if (permssn == fsCurPerm)
	    permssn = fsRdWrPerm;
	fcbp->fcbMdRByt = 0;
    }
    switch (permssn) {
    case fsRdPerm:
	break;
    case fsWrPerm:
    case fsRdWrPerm:
	fcbp->fcbMdRByt |= WRITEBIT;
	break;
    case fsRdWrShPerm:
	fcbp->fcbMdRByt |= (WRITEBIT|SHAREDBIT);
	break;
    default:
	fcbp->fcbFlNum = 0;
	DebugStr((StringPtr) "\punknown permission");
	PBRETURN(pb, fsDSIntErr);
    }
    
    if (ft == resourcefork)
	fcbp->fcbMdRByt |= RESOURCEBIT;
    else
	fcbp->fcbMdRByt &= ~RESOURCEBIT;

    if (ft == datafork) {
	fcbp->fcbEOF = frp->filLgLen;
	fcbp->fcbPLen = frp->filPyLen;
	fcbp->fcbSBlk = frp->filStBlk;
	memcpy((char *) fcbp->fcbExtRec, (char *) frp->filExtRec,
					       (size_t) sizeof(frp->filExtRec));
    } else {
	fcbp->fcbEOF = frp->filRLgLen;
	fcbp->fcbPLen = frp->filRPyLen;
	fcbp->fcbSBlk = frp->filRStBlk;
	memcpy((char *) fcbp->fcbExtRec, (char *) frp->filRExtRec,
					      (size_t) sizeof(frp->filRExtRec));
    }
    fcbp->fcbCrPs = 0;
    fcbp->fcbVPtr = btparamrec.vcbp;
#if !defined (UNIX)
    fcbp->fcbBfAdr = pb->ioMisc;
#else /* UNIX */
    fcbp->fcbBfAdr = (Ptr) pb->ioMisc;
#endif /* UNIX */
    fcbp->fcbFlPos = 0;
    
    fcbp->fcbClmpSize = frp->filClpSize;
    if (!fcbp->fcbClmpSize)
	fcbp->fcbClmpSize = btparamrec.vcbp->vcbClpSiz;
	
    fcbp->fcbBTCBPtr = 0;   /* Used only for B-trees I think */
    fcbp->fcbFType = frp->filUsrWds.fdType;
    catkeyp = (catkey *) btparamrec.foundp;
    cachep = addrtocachep((Ptr) catkeyp, btparamrec.vcbp);
    fcbp->fcbCatPos = cachep->logblk;
    fcbp->fcbDirID = catkeyp->ckrParID;
    str255assign(fcbp->fcbCName, catkeyp->ckrCName);
    pb->ioRefNum = (char *) fcbp - (char *) FCBSPtr;
    PBRETURN(pb, noErr);
}

#undef RETURN

PUBLIC OSErr myPBOpen(ioParam *pb, BOOLEAN async)
{
    return PBOpenHelper(pb, datafork, 1L, async);
}

PUBLIC OSErr myPBOpenRF(ioParam *pb, BOOLEAN async)
{
    return PBOpenHelper(pb, resourcefork, 1L, async);
}

PUBLIC OSErr myPBHOpen(HFileParam *pb, BOOLEAN async)
{
    return PBOpenHelper((ioParam *) pb, datafork, pb->ioDirID, async);
}

PUBLIC OSErr myPBHOpenRF(HFileParam *pb, BOOLEAN async)
{
    return PBOpenHelper((ioParam *) pb, resourcefork, pb->ioDirID, async);
}

/*
 * NOTE:  I've tried playing around with the LockRange and UnlockRange calls
 *        and I don't know what they do.  They seem to be unimplemented on
 *        our Mac+.  Perhaps when they were created there was no idea that
 *        multifinder would exist so they don't do anything if you're not
 *        on a network.
 */
 
PUBLIC OSErr myPBLockRange(ioParam *pb, BOOLEAN async)
{
    PBRETURN(pb, noErr);
}

PUBLIC OSErr myPBUnlockRange(ioParam *pb, BOOLEAN async)
{
    PBRETURN(pb, noErr);
}

PUBLIC OSErr myPBGetFPos(ioParam *pb, BOOLEAN async)
{
    filecontrolblock *fcbp;
    
    fcbp = refnumtofcbp(pb->ioRefNum);
    if (!fcbp)
/*-->*/ PBRETURN(pb, rfNumErr);
    pb->ioReqCount = 0;
    pb->ioActCount = 0;
    pb->ioPosMode = 0;
    pb->ioPosOffset = fcbp->fcbCrPs;

    PBRETURN(pb, noErr);
}

PUBLIC OSErr myPBSetFPos(ioParam *pb, BOOLEAN async)
{
    filecontrolblock *fcbp;
    long newpos;
    OSErr retval;
    
    fcbp = refnumtofcbp(pb->ioRefNum);
    if (!fcbp)
/*-->*/ PBRETURN(pb, rfNumErr);
    retval = noErr;
    newpos = pbabsoffset(pb, fcbp);
    if (newpos < 0)
	retval = posErr;
    else if (newpos > fcbp->fcbEOF) {
	retval = eofErr;
	fcbp->fcbCrPs = fcbp->fcbEOF;
    } else
	fcbp->fcbCrPs = newpos;
    pb->ioPosOffset = fcbp->fcbCrPs;
    PBRETURN(pb, retval);
}

PUBLIC OSErr myPBGetEOF(ioParam *pb, BOOLEAN async)
{
    filecontrolblock *fcbp;
    
    fcbp = refnumtofcbp(pb->ioRefNum);
    if (!fcbp)
/*-->*/ PBRETURN(pb, rfNumErr);
#if !defined (UNIX)
    pb->ioMisc = (Ptr) fcbp->fcbEOF;
#else /* UNIX */
    pb->ioMisc = fcbp->fcbEOF;
#endif /* UNIX */

    PBRETURN(pb, noErr);
}

PRIVATE OSErr dirtyfcbp(filecontrolblock *fcbp)
{
    ulong catpos;
    short refnum;
    HVCB *vcbp;
    cacheentry *cachep;
    catkey key;
    unsigned char *namep;
    anykey *retkeyp;
    filerec *frp;
    OSErr err;
    INTEGER dumint;
    
    if (fcbp->fcbMdRByt & DIRTYBIT) {
	refnum = (char *) fcbp - (char *) FCBSPtr;
	vcbp = fcbp->fcbVPtr;
	if (catpos = fcbp->fcbCatPos) {
	    err = getcache(&cachep, fcbp->fcbVPtr->vcbCTRef, catpos, 0);
	    if (err != noErr)
		return err;
	    namep = fcbp->fcbCName;
	    makecatkey(&key, fcbp->fcbDirID, namep[0], (Ptr) namep+1);
	    if (!searchnode((btnode *)cachep->buf, (void *)&key, catcompare,
							  &retkeyp, &dumint)) {
		DebugStr((StringPtr) "\pTODO: just use keyfind!");
		return fsDSIntErr;  /* TODO: just use keyfind! */
	    }
	    frp = (filerec *) DATAPFROMKEY(retkeyp);
	
	    if (fcbp->fcbMdRByt & FLOCKEDBIT)
		frp->filFlags |= FSOFTLOCKBIT;
	    else
		frp->filFlags &= ~FSOFTLOCKBIT;
	
	    frp->filTyp           = fcbp->fcbTypByt;
	    frp->filStBlk         = fcbp->fcbSBlk;
	    frp->filClpSize       = fcbp->fcbClmpSize;
	    frp->filUsrWds.fdType = fcbp->fcbFType;
	
	    if (fcbp->fcbMdRByt & RESOURCEBIT) {
		frp->filRLgLen  = fcbp->fcbEOF;
		frp->filRPyLen  = fcbp->fcbPLen;
		memcpy(frp->filRExtRec, fcbp->fcbExtRec,
					     (size_t) sizeof(frp->filRExtRec));
	    } else {
		frp->filLgLen   = fcbp->fcbEOF;
		frp->filPyLen   = fcbp->fcbPLen;
		memcpy(frp->filExtRec, fcbp->fcbExtRec,
					      (size_t) sizeof(frp->filExtRec));
	    }
	    cachep->flags |= CACHEDIRTY;
	} else if (refnum == vcbp->vcbXTRef || refnum == vcbp->vcbCTRef) {
	    err = noErr;
	    vcbp->vcbFlags |= VCBDIRTY;
#if 0
	    assert(0);
#endif
	} else {
	    DebugStr((StringPtr) "\pno catpos (nor are we XTRef or CTRef)");
	    err = fsDSIntErr;
	}
	if (err == noErr)
	    fcbp->fcbMdRByt &= ~DIRTYBIT;
	return err;
    } else
	return noErr;
}
#endif

#if defined(OUTDATEDCODE)

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#if !defined(UNIX)
#include <string.h>
#endif
#include "MemoryMgr.h"
#include "ToolboxUtil.h"
#include <stdio.h>

PUBLIC ulong blockchecksum(void *blockp)
{
    ulong retval, *ulp;
    INTEGER i;
    
    for (i = 128, retval = 0, ulp = blockp; --i >= 0;)
	retval ^= *ulp++;
    return retval;
}

#if 0
PRIVATE void checkcache(short refnum)
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
    fcbp = (filecontrolblock *)((char *)FCBSPtr + refnum);
    vcbp = fcbp->fcbVPtr;
    headp = (cachehead *) vcbp->vcbCtlBuf;
    printf("headp = 0x%lx, nitems = %d, flink = 0x%lx, blink = 0x%lx\n",
			headp, headp->nitems, headp->flink, headp->blink);
    for (i = headp->nitems, cachep = headp->flink; --i >= -3;
						   cachep = cachep->flink)
	printf("0x%lx:0x%x ", cachep, cachep->flags);
    printf("\n\n");
    for (i = headp->nitems, cachep = headp->blink; --i >= -3;
						 cachep = cachep->blink)
	printf("0x%lx ", cachep);
    printf("\n");
}
#endif

PUBLIC cacheentry *addrtocachep(Ptr addr, HVCB *vcbp)
{
    cachehead *headp;
    cacheentry *retval;
    INTEGER i;

    headp = (cachehead *) vcbp->vcbCtlBuf;
    for (i = headp->nitems, retval = headp->flink; --i >= 0 &&
	      (addr < (Ptr) retval || addr > (Ptr) retval + sizeof(cacheentry));
							 retval = retval->flink)
	;
    return i >= 0 ? retval : 0;
}

#define BTENTRY(btp, n) \
      ((anykey *)((char *) (btp) + \
      (((short *)((char *)(btp) + PHYSBSIZE - sizeof(short)))[-(n)])))
      
#define BTOFFSET(btp, n)    \
		      ((short *)((char *)(btp) + PHYSBSIZE - sizeof(short))-(n))

#define EVENUP(x)   (((x)+1)/2 *2)

/*
 * The test code below assumes that a catalog file is being modified.
 * You can't have CATFILEDEBUG turned on during normal use because as soon
 * as an extents file has to be modified you'll get complaints related to
 * the keysize not being what this code expects.
 */
 
/* #define CATFILEDEBUG */

#if defined (CATFILEDEBUG)

PRIVATE void checkbtp(btnode *btp)
{
    ulong flink, blink;
    short *offsetp, expected;
    INTEGER i;
    char keylen;
    
    flink = btp->ndFLink;
    blink = btp->ndBLink;
    switch (btp->ndType) {
    case indexnode:
	if (btp->ndLevel > 5)
	    errormessage((StringPtr) "\plevel > 5 on indexnode", NOTE);
	offsetp = BTOFFSET(btp, 0);
	expected = sizeof(btnode);
	for (i = btp->ndNRecs+1; --i >= 0; --offsetp) {
	    if (*offsetp != expected)
		if (*offsetp < expected)
		    errormessage((StringPtr) "\punexpected offset", STOP);
		else
		    fprintf(stderr, "curiously large offset\n");
	    if (i > 0) {
		if (*((char *)btp+expected) != 37)
		    errormessage((StringPtr) "\punexpected keylen", STOP);
		expected += 38 + sizeof(long);
	    }
	}
	break;
    case leafnode:
#if 0
	if (flink > 100 || blink > 100)         /* could do more checking */
	    errormessage("\pflink or blink > 100", CAUTION); /* if we looked at block0 */
#endif
	if (btp->ndLevel != 1)
	    errormessage((StringPtr) "\plevel != 1 on leafnode", STOP);
	offsetp = BTOFFSET(btp, 0);
	expected = sizeof(btnode);
	for (i = btp->ndNRecs+1; --i >= 0; --offsetp) {
	    if (*offsetp != expected)
		if (*offsetp < expected)
		    errormessage((StringPtr) "\punexpected offset", CAUTION);
		else
		    fprintf(stderr, "curiously large offset\n");
	    if (i > 0) {
		if ((keylen = *((char *)btp+expected)) > 37)
		    errormessage((StringPtr) "\punexpected keylen", CAUTION);
		expected += EVENUP(keylen+1);
		switch (*((char *)btp+expected)) {
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
		    errormessage((StringPtr) "\punexpected type", STOP);
		    break;
		}
	    }
	}
	break;
    default:
	errormessage((StringPtr) "\punknown node type", STOP);
	return;
	break;
    }
}
#endif /* CATFILEDEBUG */

PUBLIC BOOLEAN searchnode(btnode *btp, void *key, compfp fp, anykey **keypp,
								INTEGER *afterp)
{
    INTEGER low, high, mid;
    anykey *totest, *totest2;
    
#if defined (CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    high = btp->ndNRecs-1;
    totest = BTENTRY(btp, high);        /* test last one by hand then use as sentinel */
    switch ((*fp)(key, totest)) {
    case firstisless:
	low = 0;
	for (;;) {
	    mid = (low + high) / 2;
	    totest = BTENTRY(btp, mid);
	    switch ((*fp)(key, totest)) {
	    case firstisless:
		if (mid == 0) {
		    *keypp = totest;
		    *afterp = -1;
		    return FALSE;
		}
		high = mid;
		break;
	    case same:
		*keypp = totest;
		*afterp = mid;
		return TRUE;
	    case firstisgreater:
		totest2 = BTENTRY(btp, mid+1);
		switch ((*fp)(key, totest2)) {
		case firstisless:
		    *keypp = totest;
		    *afterp = mid;
		    return FALSE;
		case same:
		    *keypp = totest2;
		    *afterp = mid+1;
		    return TRUE;
		case firstisgreater:
		    low = mid+1;
		    break;
		}
	    }
	}
    case same:
	*keypp = totest;
	*afterp = high;
	return TRUE;
    case firstisgreater:
	*keypp = totest;
	*afterp = high;
	return FALSE;
    }
#if !defined (LETGCCWAIL)
    return FALSE;
#endif
}

PRIVATE void makefirst(cachehead *headp, cacheentry *entryp)
{
    if (headp->flink != entryp) {
	entryp->blink->flink = entryp->flink;   /* remove link */
	entryp->flink->blink = entryp->blink;
	
	entryp->flink = headp->flink;
	entryp->blink = headp->flink->blink;
	
	headp->flink->blink = entryp;
	headp->flink = entryp;
    }
}

PUBLIC OSErr putcache(cacheentry *cachep)
{
    OSErr err;
    HVCB *vcbp;
    
    err = noErr;
    if ((cachep->flags & (CACHEDIRTY|CACHEFREE)) == CACHEDIRTY) {
	vcbp = cachep->vptr;
#if !defined(UNIX)
	if (strncmp((char *) vcbp->vcbVN, "\pMyVol", vcbp->vcbVN[0]+1) != 0) {
	    errormessage((StringPtr) "\pDisk not named MyVol", NOTE);
#if 0
	    errormessage((StringPtr) "\pdangerous chemicals", NOTE);
	    printf("Dangerous chemicals, Timmy! (we should stick to MyVol)\n");
#endif /* 0 */
	    DebugStr((StringPtr) "\pabout to exit from putcache");
	    exit(1);
	}
#endif
#if 0
	BufTgFNum = cachep->fileno;
	BufTgFFlag = cachep->forktype == datafork ? 0 : 2;
	BufTgFBkNum = cachep->logblk;
	BufTgDate = Time;
#endif
	err = TransPhysBlk(vcbp, cachep->physblock * PHYSBSIZE, 1,
				       (Ptr) cachep->buf, writing, (long *) 0);
    }
    if (cachep->flags & CACHEFREE)
	errormessage((StringPtr) "\pcache free", NOTE);
    cachep->flags &= ~CACHEDIRTY;
    return err;
}

PUBLIC LONGINT tagfnum;
PUBLIC INTEGER tagflag;
PUBLIC INTEGER tagbknm;
PUBLIC LONGINT tagdate;
PUBLIC LONGINT tagtfs0;
PUBLIC LONGINT tagtfs1;

PUBLIC OSErr getcache(cacheentry **retpp, short refnum, ulong logbno,
							    cacheflagtype flags)
{
    cacheentry *retval, *lastp, *lastdirtyp, *lastfreep;
    cachehead *headp;
    HVCB *vcbp;
    filecontrolblock *fcbp;
    INTEGER count;
    short nphyscontig;
    OSErr err;
    ulong physbyte;
    LONGINT filenum;
    forktype forkwanted;
#if 1
    INTEGER badnesscount;
#endif
    
    fcbp = (filecontrolblock *)((char *)FCBSPtr + refnum);
    vcbp = fcbp->fcbVPtr;
    filenum = fcbp->fcbFlNum;
    forkwanted = fcbp->fcbMdRByt & RESOURCEBIT ? resourcefork : datafork;
    headp = (cachehead *) vcbp->vcbCtlBuf;
    
#if 0 && !defined(UNIX)
    if (strncmp((char *) vcbp->vcbVN, "\pMyVol", vcbp->vcbVN[0]+1) != 0) {
	errormessage((StringPtr) "\pdangerous chemicals", CAUTION);
	printf("Dangerous chemicals, Timmy! (we should stick to MyVol)\n");
	exit(1);
    }
#endif
    
    count = headp->nitems;
    lastp = 0;
    lastdirtyp = 0;
    lastfreep = 0;
#if 1
    badnesscount = 0;
#endif
    for (retval = headp->flink; --count >= 0 && (retval->logblk != logbno ||
		retval->refnum != refnum || retval->vptr != vcbp ||
		retval->fileno != filenum || retval->forktype != forkwanted);
						      retval = retval->flink) {
	if (!(retval->flags & CACHEBUSY)) {
	    if (retval->flags & CACHEDIRTY) {
	        if (retval->vptr == vcbp)   /* TODO: take vptr == vcbp out */
		    lastdirtyp = retval;
#if 1
		else
		    badnesscount++;
#endif
	    } else
		lastp = retval;
	}
	if (retval->flags & CACHEFREE)
	    lastfreep = retval;
    }
    if (count < 0) {
	if (lastfreep)
	    retval = lastfreep;
	else if (lastp)
	    retval = lastp;
	else if (lastdirtyp) {
	    retval = lastdirtyp;
	    err = putcache(retval);
	    if (err != noErr)
		return err;
	} else {
	    DebugStr((StringPtr) "\pall cache busy");
	    return fsDSIntErr;
	}
    }
    makefirst(headp, retval);
    if (count < 0) {
	retval->vptr = vcbp;
	retval->fileno = filenum;
	retval->refnum = refnum;
	retval->logblk = logbno;
	retval->flags = CACHEBUSY;
	retval->forktype = forkwanted;

	physbyte = logtophys(fcbp, logbno * PHYSBSIZE, &nphyscontig);
	if (nphyscontig < 1) {
	    DebugStr((StringPtr) "\pnphyscontig < 1");
	    return fsDSIntErr;
	}
	retval->physblock = physbyte / PHYSBSIZE;
	if (!(flags&GETCACHENOREAD))
	    err = TransPhysBlk(vcbp, physbyte, 1, (Ptr) retval->buf, reading,
								    (long *) 0);
	else
	    err = noErr;
	retval->flags = 0;
#if 0
	if (logbno == 0 || logbno == 1) {
	    tagfnum = BufTgFNum;
	    tagflag = BufTgFFlag;
	    tagbknm = BufTgFBkNum;
	    tagdate = BufTgDate;
	    tagtfs0 = TFSTagData0;
	    tagtfs1 = TFSTagData1;
	    errormessage((StringPtr) "\pgot 'em", NOTE);
	}
#endif
    } else
	err = noErr;
    if (flags & GETCACHESAVE)
	retval->flags |= CACHEBUSY;
    retval->flags &= ~CACHEFREE;
    *retpp = retval;
    return err;
}

#if defined (CATFILEDEBUG)
PUBLIC void checkleaves(INTEGER refnum)
{
    OSErr err;
    cacheentry *block0cachep, *cachep;
    btblock0 *block0p;
    ulong node, expectedblink;
    btnode *btp;
    
    err = getcache(&block0cachep, refnum, (ulong) 0, GETCACHESAVE);
    if (err != noErr)
	errormessage((StringPtr) "\pgetcache error", STOP);
    block0p = (btblock0 *) block0cachep->buf;
    node = block0p->firstleaf;
    expectedblink = 0;
    while (node != 0) {
	err = getcache(&cachep, refnum, node, 0);
	if (err != noErr)
	    errormessage((StringPtr) "\pgetcache error", STOP);
	btp = (btnode *) cachep->buf;
#if defined (CATFILEDEBUG)
	checkbtp(btp);
#endif /* CATFILEDEBUG */
	if (btp->ndBLink != expectedblink)
	    errormessage((StringPtr) "\pbad blink", STOP);
	expectedblink = node;
	node = btp->ndFLink;
    }
    if (block0p->lastleaf != expectedblink)
	errormessage((StringPtr) "\pbad block0p->blink", STOP);
}
#endif /* CATFILEDEBUG */

PUBLIC OSErr cleancache(HVCB *vcbp)
{
    INTEGER i;
    cachehead *headp;
    cacheentry *cachep;
    OSErr err;
    
    headp = (cachehead *) vcbp->vcbCtlBuf;
    err = noErr;
    for (i = headp->nitems, cachep = (cacheentry *) (headp + 1); --i >= 0;
								    ++cachep) {
	if (cachep->vptr == vcbp)
	    cachep->flags &= ~CACHEBUSY;
    }
    return err;
}

PUBLIC OSErr flushcachevcbp(HVCB *vcbp)
{
    INTEGER i;
    cachehead *headp;
    cacheentry *cachep;
    OSErr err;
    
    headp = (cachehead *) vcbp->vcbCtlBuf;
    err = noErr;
    for (i = headp->nitems, cachep = (cacheentry *) (headp + 1); --i >= 0;
								    ++cachep) {
	if (cachep->vptr == vcbp && (cachep->flags & CACHEDIRTY))
	    putcache(cachep);
    }
    return err;
}

/*
 * NOTE: an important side effect of keyfind is that the first and last node
 *       have the CACHEBUSY bit set.
 */
 
PUBLIC OSErr keyfind(btparam *btpb)
{
    cacheentry *cachep;
    OSErr err;
    long node;
    BOOLEAN found;
    unsigned char type;
    trailentry *tep;
    
    tep = btpb->trail;
    err = getcache(&cachep, btpb->refnum, (ulong) 0, GETCACHESAVE);
    if (err != noErr)
	return err;
    if (((btblock0 *)cachep->buf)->numentries == 0) {
        btpb->foundp = 0;
        btpb->success = FALSE;
        btpb->leafindex = 0;
        return noErr;
    }
    node = ((btblock0 *)cachep->buf)->root;
    tep->logbno = 0;
    tep++->cachep = cachep;
#if !defined (LETGCCWAIL)
    type = 0;
#endif /* LETGCCWAIL */
    for (;;) {
	err = getcache(&cachep, btpb->refnum, node, 0);
	if (err == noErr) {
	    tep->logbno = node;
	    tep->cachep = cachep;
	    type = ((btnode *)cachep->buf)->ndType;
	}
	if (err != noErr || (type != indexnode && type != leafnode)) {
	    btpb->success = FALSE;
	    if (err == noErr) {
	        DebugStr((StringPtr) "\punknown node");
	        err = fsDSIntErr;
	    }
	    return err;
	}
	found = searchnode((btnode *)cachep->buf, &btpb->tofind,
					  btpb->fp, &btpb->foundp, &tep->after);
	if (type == indexnode)
	    node = *(long *) DATAPFROMKEY(btpb->foundp);
	else {
	    btpb->leafindex = tep - btpb->trail;
	    btpb->success = found;
	    tep->cachep->flags |= CACHEBUSY;
	    return noErr;
	}
	++tep;
    }
}

/*
 * NOTE: btnext could be made to use searchnode if we passed in a comparator.
 *       Currently I'm not concerned with "speed" since disk i/o overwhelms
 *       computation..
 */

PUBLIC OSErr btnext(anykey **nextpp, anykey *keyp, HVCB *vcbp)
{
    cacheentry *cachep;
    btnode *btp;
    INTEGER i;
    anykey *retval;
    long node;
    OSErr err;
    
    cachep = addrtocachep((Ptr) keyp, vcbp);
    btp = (btnode *) cachep->buf;
    for (i = btp->ndNRecs; --i >= 0 && BTENTRY(btp, i) != keyp;)
	;
    if (i < 0)
	retval = 0;
    else if (i < btp->ndNRecs - 1)
	retval = BTENTRY(btp, i+1);
    else if (node = btp->ndFLink) {
	err = getcache(&cachep, cachep->refnum, node, 0);
	if (err != noErr)
	    return err;
	btp = (btnode *) cachep->buf;
	retval = BTENTRY(btp, 0);
    } else
	retval = 0;
    *nextpp = retval;
    return noErr;
}

PRIVATE OSErr deletenode(cacheentry *todeletep)
{
    cacheentry *block0cachep, *linkcachep;
    btblock0 *block0p;
    btnode *btp, *linkbtp;
    ulong node, flink, blink;
    OSErr err;
    INTEGER refnum;
    
    refnum = todeletep->refnum;
    err = getcache(&block0cachep, refnum, 0L, GETCACHESAVE);
    if (err != noErr)
	return err;
    block0p = (btblock0 *) block0cachep->buf;
    btp = (btnode *) todeletep->buf;
#if defined (CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    node = todeletep->logblk;
#if 0
    if (btp->ndType == leafnode) {
#endif
	flink = btp->ndFLink;
	blink = btp->ndBLink;
	if (block0p->firstleaf == node)
	    block0p->firstleaf = flink;
	if (block0p->lastleaf == node)
	    block0p->lastleaf = blink;
	if (blink) {
	    err = getcache(&linkcachep, refnum, blink, 0);
	    if (err != noErr)
		return err;
	    linkbtp = (btnode *) linkcachep->buf;
	    linkbtp->ndFLink = flink;
	    linkcachep->flags |= CACHEDIRTY;
	}
	if (flink) {
	    err = getcache(&linkcachep, refnum, flink, 0);
	    if (err != noErr)
		return err;
	    linkbtp = (btnode *) linkcachep->buf;
	    linkbtp->ndBLink = blink;
	    linkcachep->flags |= CACHEDIRTY;
	}
#if 0
    }
#endif
    ++block0p->nfreenodes;
    BitClr((Ptr) block0p->map, node);
    block0cachep->flags |= CACHEDIRTY;
    bzero(todeletep->buf, PHYSBSIZE);
    todeletep->flags |= CACHEDIRTY;
    return noErr;
}
    
typedef enum { leavealone, doleft, doright } whichnodetype;

#define FREESIZE(btp)   \
    (((char *) (btp) + PHYSBSIZE - ((btp)->ndNRecs+1)*sizeof(short)) - \
					(char *) BTENTRY((btp), (btp)->ndNRecs))
					
#define SIZECUTOFF  ((PHYSBSIZE - sizeof(btnode)) / 2)

/*
 * NOTE: the code for merge is very similar to the code for shuffle right to
 *       left.  If you find a bug here, look for a corresponding one below.
 */
 
PRIVATE OSErr merge(cacheentry *leftp, cacheentry *rightp)
{
    INTEGER n, nrecs, datasize, i;
    btnode *leftbtp, *rightbtp;
    char *datastart, *datastop;
    INTEGER *offsetp;
    
    leftbtp  = (btnode *) leftp ->buf;
    rightbtp = (btnode *) rightp->buf;
#if defined (CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    nrecs = rightbtp->ndNRecs;
    datastart = (char *) BTENTRY(rightbtp, 0);
    datastop  = (char *) BTENTRY(rightbtp, nrecs);
    datasize  = datastop - datastart;
    
    bcopy(datastart, BTENTRY(leftbtp, leftbtp->ndNRecs), datasize);
    
    offsetp = BTOFFSET(leftbtp, leftbtp->ndNRecs);
    n = 0;
    for (i = nrecs, n = 0; --i >= 0; ++n) {
	offsetp[-1] = offsetp[0] + (char *) BTENTRY(rightbtp, n+1) -
				   (char *) BTENTRY(rightbtp, n);
	--offsetp;
    }
    leftbtp->ndNRecs += nrecs;
    if (!(rightp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    if (!(leftp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    leftp->flags |= CACHEDIRTY;
#if defined (CATFILEDEBUG)
    checkbtp(leftbtp);
#endif
    return deletenode(rightp);
}

PRIVATE OSErr shuffle(cacheentry *leftp, cacheentry *rightp)
{
    btnode *leftbtp, *rightbtp;
    INTEGER leftfreesize, rightfreesize, numtocopy, n, recsize,
	    rightdatasize, bytestoshift, i;
    char *rightbtentry0, *recstart, *recend;
    INTEGER *offsetp;
    
    leftbtp  = (btnode *)  leftp->buf;
    rightbtp = (btnode *) rightp->buf;
#if defined (CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    leftfreesize  = FREESIZE(leftbtp);
    rightfreesize = FREESIZE(rightbtp);
    numtocopy = 0;
    bytestoshift = 0;
    rightbtentry0 = (char *) BTENTRY(rightbtp, 0);
#if !defined (LETGCCWAIL)
    recstart = recend = 0;
#endif /* LETGCCWAIL */
    if (leftfreesize < rightfreesize) {
	/* copy from left to right; almost the same code as below */
	/* NOTE:  if you find a bug here, look for a similar bug below */
	n = leftbtp->ndNRecs - 1;
	while (rightfreesize > SIZECUTOFF) {
	    numtocopy++;
	    recstart = (char *) BTENTRY(leftbtp, n);
	    recend   = (char *) BTENTRY(leftbtp, n+1);
	    recsize = recend - recstart;
	    bytestoshift += recsize;
	    rightfreesize -= sizeof(INTEGER) + recsize;
	    --n;
	}
	rightdatasize = (char *) BTENTRY(rightbtp, rightbtp->ndNRecs) -
								  rightbtentry0;
	bcopy(rightbtentry0, rightbtentry0+bytestoshift, rightdatasize);
	bcopy(recstart, rightbtentry0, bytestoshift);
	
	offsetp = BTOFFSET(rightbtp, rightbtp->ndNRecs + numtocopy);
	for (i = rightbtp->ndNRecs; --i >= 0;) {
	    offsetp[0] = offsetp[numtocopy] + bytestoshift;
	    ++offsetp;
	}
	
	offsetp = BTOFFSET(rightbtp, 0);
	++n;
	for (i = numtocopy; --i >= 0;) {
	    offsetp[-1] = offsetp[0] + (char *) BTENTRY(leftbtp, n+1) -
				       (char *) BTENTRY(leftbtp, n);
	    --offsetp;
	    ++n;
	}
	
	leftbtp ->ndNRecs -= numtocopy;
	rightbtp->ndNRecs += numtocopy;
    } else {
	/* copy from right to left; almost the same code as above */
	/* NOTE:  if you find a bug here, look for a similar bug above */
	n = 0;
	while (leftfreesize > SIZECUTOFF) {
	    numtocopy++;
	    recstart = (char *) BTENTRY(rightbtp, n);
	    recend   = (char *) BTENTRY(rightbtp, n+1);
	    recsize = recend - recstart;
	    bytestoshift += recsize;
	    leftfreesize -= sizeof(INTEGER) + recsize;
	    ++n;
	}
	rightdatasize = (char *) BTENTRY(rightbtp, rightbtp->ndNRecs) -
								  recend;
	bcopy(rightbtentry0, BTENTRY(leftbtp, leftbtp->ndNRecs), bytestoshift);
	bcopy(recend, rightbtentry0, rightdatasize);
	
	offsetp = BTOFFSET(leftbtp, leftbtp->ndNRecs);
	n = 0;
	for (i = numtocopy; --i >= 0;) {
	    offsetp[-1] = offsetp[0] + (char *) BTENTRY(rightbtp, n+1) -
				       (char *) BTENTRY(rightbtp, n);
	    --offsetp;
	    ++n;
	}
	
	offsetp = BTOFFSET(rightbtp, 1);
	for (i = rightbtp->ndNRecs - numtocopy; --i >= 0;) {
	    offsetp[0] = offsetp[-numtocopy] - bytestoshift;
	    --offsetp;
	}
	
	rightbtp->ndNRecs -= numtocopy;
	leftbtp ->ndNRecs += numtocopy;
	
    }
#if defined (CATFILEDEBUG)
    checkbtp(leftbtp);
    checkbtp(rightbtp);
#endif
    if (!(rightp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    if (!(leftp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    leftp ->flags |= CACHEDIRTY;
    rightp->flags |= CACHEDIRTY;
    return noErr;
}

PRIVATE OSErr btsetkey(cacheentry *cachep, INTEGER index, anykey *srckeyp)
{
    anykey *dstkeyp;
    
#if 0
    if (cachep->logblk == 15)
        DebugStr("\psetting key of 15");
#endif
#if defined (CATFILEDEBUG)
    checkbtp((btnode *) cachep->buf);
#endif /* CATFILEDEBUG */
    dstkeyp = BTENTRY((btnode *) cachep->buf, index);
    
    /* NOTE: dstkeyp->keylen is not a bug; it shouldn't be srckeyp->keylen.
	     btsetkey is used only to set a parent's key from a child's key,
	     and the rule is that parent's keys are always a fixed length
	     that are never smaller than children's keys */
	     
    bcopy((char *)srckeyp + 1, (char *)dstkeyp + 1, dstkeyp->keylen);
    if (!(cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    cachep->flags |= CACHEDIRTY;
#if defined (CATFILEDEBUG)
    checkbtp((btnode *) cachep->buf);
#endif /* CATFILEDEBUG */
    return noErr;
}

PRIVATE OSErr pullout(cacheentry *selfcachep, INTEGER selfindex,
	      cacheentry *parentcachep, INTEGER parentindex, INTEGER *todeletep)
{
    cacheentry *leftcachep, *rightcachep;
    btnode *btp, *parentbtp, *leftbtp, *rightbtp;
    short *offsetp;
    INTEGER adjust;
    char *startp, *stopp, *freep;
    INTEGER freesize, ntoadjust;
    whichnodetype whichmerge;
    OSErr err;
    BOOLEAN done, modselfkey, modrightkey;
    LONGINT left, right;
    
#if 0
    if (selfcachep->logblk == 15)
        DebugStr("\ppulling out of 15");
#endif
    modselfkey = FALSE;
    modrightkey = FALSE;
    *todeletep = -1;
    btp = (btnode *) selfcachep->buf;
    if (selfindex < 0 || selfindex >= btp->ndNRecs) {
        DebugStr((StringPtr) "\pfried selfindex");
	return fsDSIntErr;
    }
	
#if defined (CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    /* delete entry */
    
    selfcachep->flags |= CACHEDIRTY;
    startp = (char *) BTENTRY(btp, selfindex);
    stopp  = (char *) BTENTRY(btp, selfindex+1);
    freep  = (char *) BTENTRY(btp, btp->ndNRecs);
    bcopy(stopp, startp, freep - stopp);
    ntoadjust = btp->ndNRecs - selfindex;
    offsetp = BTOFFSET(btp, selfindex);
    adjust = stopp - startp;
    while (--ntoadjust >= 0) {
	*offsetp = offsetp[-1] - adjust;
	--offsetp;
    }
    --btp->ndNRecs;
    if (selfindex == 0)
	modselfkey = TRUE;
    
#if defined (CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
#if !defined (LETGCCWAIL)
    leftbtp = rightbtp = 0;
#endif
    /* check to see if freespace is too big */
    freesize = FREESIZE(btp);
    if (freesize > SIZECUTOFF) {
	done = FALSE;
	if (parentcachep) {
	    parentbtp = (btnode *) parentcachep->buf;
#if defined (CATFILEDEBUG)
	    checkbtp(parentbtp);
#endif /* CATFILEDEBUG */
	    if (parentindex > 0)
		left = *(long *)DATAPFROMKEY(BTENTRY(parentbtp, parentindex-1));
	    else
		left = -1;
	    if (parentindex < parentbtp->ndNRecs - 1)
		right = *(long *)DATAPFROMKEY(BTENTRY(parentbtp, parentindex+1));
	    else
		right = -1;
	} else {
	    left = -1;
	    right = -1;
	}
	if (left >= 0) {
	    err = getcache(&leftcachep, selfcachep->refnum, left, GETCACHESAVE);
	    if (err != noErr)
		return err;
	    leftbtp = (btnode *) leftcachep->buf;
#if defined (CATFILEDEBUG)
	    checkbtp(leftbtp);
#endif /* CATFILEDEBUG */
	    if (freesize + FREESIZE(leftbtp) < 2 * SIZECUTOFF) {
		err = shuffle(leftcachep, selfcachep);
#if 0
		printf("shuffled left, self\n");
#endif
		if (err != noErr)
		    return err;
		modselfkey = TRUE;
		done = TRUE;
	    }
	}
	if (!done && right >= 0) {
	    err = getcache(&rightcachep, selfcachep->refnum, right,
								  GETCACHESAVE);
	    if (err != noErr)
		return err;
	    rightbtp = (btnode *) rightcachep->buf;
#if defined (CATFILEDEBUG)
	    checkbtp(rightbtp);
#endif /* CATFILEDEBUG */
	    if (freesize + FREESIZE(rightbtp) < 2 * SIZECUTOFF) {
		err = shuffle(selfcachep, rightcachep);
#if 0
		printf("shuffled self, right\n");
#endif
		if (err != noErr)
		    return err;
		modrightkey = TRUE;
		done = TRUE;
	    }
	}
	if (!done) {
	    if (left >= 0) {
		if (right >= 0)
		    if (FREESIZE(leftbtp) <= FREESIZE(rightbtp))
			whichmerge = doleft;
		    else
			whichmerge = doright;
		else
		    whichmerge = doleft;
	    } else {
		if (right >= 0)
		    whichmerge = doright;
		else
		    whichmerge = leavealone;
	    }
	    switch (whichmerge) {
	    case doleft:
		err = merge(leftcachep, selfcachep);
#if 0
		printf("merged left, self\n");
#endif
		*todeletep = parentindex;
		modselfkey = FALSE;
		break;
	    case doright:
		err = merge(selfcachep, rightcachep);
#if 0
		printf("merged self, right\n");
#endif
		*todeletep = parentindex+1;
		modrightkey = FALSE;
		break;
	    case leavealone:    /* do nothing */
		break;
	    default:
	    	DebugStr((StringPtr) "\punknown whichmerge");
		return fsDSIntErr;
		break;
	    }
	}
    }
    err = noErr;
    if (modselfkey && parentcachep) {
	err = btsetkey(parentcachep, parentindex, BTENTRY(btp, 0));
#if 0
	printf("modded self\n");
#endif
    }
    if (err == noErr && modrightkey) {
	err = btsetkey(parentcachep, parentindex+1, BTENTRY(rightbtp, 0));
#if 0
	printf("modded right\n");
#endif
    }
    return err;
}

PRIVATE OSErr maketrailentrybusy(trailentry *tep, INTEGER refnum)
{
    OSErr err;
    HVCB *vcbp;
    
    vcbp = ((filecontrolblock *)((char *)FCBSPtr + refnum))->fcbVPtr;
    if (tep->cachep->refnum != refnum || tep->cachep->logblk != tep->logbno ||
						      tep->cachep->vptr != vcbp)
	err = getcache(&tep->cachep, refnum, tep->logbno, GETCACHESAVE);
    else {
	err = noErr;
	tep->cachep->flags |= CACHEBUSY;
    }
    return err;
}

PRIVATE OSErr btlegitimize(btparam *btpb)
{
    OSErr err;
    
    if (btpb->leafindex < 0)
	err = keyfind(btpb);
    else
	err = noErr;
    return err;
}

PRIVATE OSErr deleteroot(cacheentry *oldrootp, cacheentry *block0cachep)
{
    btblock0 *block0p;
    /* update height, root */
    
    block0p = (btblock0 *) block0cachep->buf;
    block0p->height--;
    block0p->root =
		   *(ulong *)DATAPFROMKEY(BTENTRY((btnode *) oldrootp->buf, 0));
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    block0cachep->flags |= CACHEDIRTY;
    return deletenode(oldrootp);
}

PRIVATE OSErr btdeletetree(cacheentry *block0cachep, cacheentry *leafcachep)
{
    OSErr err, err1;
    btblock0 *block0p;
    
    /* TODO: check this over carefully */
    block0p = (btblock0 *) block0cachep->buf;
    block0p->height = 0;
    block0p->root = 0;
    /* don't set block0p->numentries; it'll be decremented later */
    block0p->firstleaf = block0p->lastleaf = 0;
    block0cachep->flags |= CACHEDIRTY;
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    if (!(leafcachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    err = deletenode(leafcachep);
    return err != noErr ? err : err1;
}

PRIVATE OSErr updatenumentries(cacheentry *block0cachep, INTEGER adjust)
{
    btblock0 *block0p;
    
    block0p = (btblock0 *) block0cachep->buf;
    block0p->numentries += adjust;
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    block0cachep->flags |= CACHEDIRTY;
    return noErr;
}

PUBLIC OSErr btdelete(btparam *btpb)
{
    OSErr err;
    trailentry *tep;
    BOOLEAN done;
    cacheentry *selfcachep, *parentcachep;
    INTEGER selfindex, parentindex, todelete, tomung, refnum;
    
    err = btlegitimize(btpb);
    if (err != noErr)
	return err;
    if (!btpb->success) {
#if 0
        err = keyfind(btpb);
        if (err != noErr)
#endif
	    DebugStr((StringPtr) "\pno success in btdelete");
	    return fsDSIntErr;
    }
    tep = btpb->trail + btpb->leafindex;
    selfindex  = tep->after;
    done = FALSE;
    refnum = btpb->trail[0].cachep->refnum;
    if (((btblock0 *)btpb->trail[0].cachep->buf)->numentries == 1) {
	err = maketrailentrybusy(&btpb->trail[1], refnum);
	if (err != noErr)
	    return err;
	err = btdeletetree(btpb->trail[0].cachep, btpb->trail[1].cachep);
    } else {
	done = FALSE;
	while (!done) {
	    err = maketrailentrybusy(tep-1, refnum);
	    if (err != noErr)
		return err;
	    selfcachep = tep->cachep;
	    if (tep > btpb->trail+1) {
		parentcachep = tep[-1].cachep;
		parentindex = tep[-1].after;
	    } else {
		parentcachep = 0;
#if !defined (LETGCCWAIL)
		parentindex = 0;
#endif
	    }
	    err = pullout(selfcachep, selfindex, parentcachep, parentindex,
								     &todelete);
	    if (err != noErr)
		return err;
	    if (todelete == -1 && ((btnode *)selfcachep->buf)->ndNRecs == 1
	    		  && ((btnode *)selfcachep->buf)->ndType == indexnode) {
		err = deleteroot(selfcachep, btpb->trail[0].cachep);
		if (err != noErr)
		    return err;
	    }
	    if (todelete == -1)
		done = TRUE;
	    else {
		selfindex = todelete;
		tep--;
	    }
	}
/*
 * We start the loop off at +leafindex-1 because the bottom most one is
 * going to already be done for us if it is necessary (which it may not
 * be if there was a merge that resulted in our current node being deleted)
 */
	for (tep = btpb->trail + btpb->leafindex - 1;
				tep > btpb->trail+1 && tep->after == 0; --tep) {
	    tomung = tep[-1].after;
	    if (tomung == -1) {
	    	DebugStr((StringPtr) "\ptomung is -1");
		return fsDSIntErr;
	    }
	    err = maketrailentrybusy(tep, refnum);
	    if (err != noErr)
		return err;
	    err = maketrailentrybusy(tep-1, refnum);
	    if (err != noErr)
		return err;
	    if ((*btpb->fp)(&btpb->tofind,
	    		   BTENTRY((btnode *)tep[-1].cachep->buf, 0)) == same) {
	        btsetkey(tep[-1].cachep, tomung,
				      BTENTRY((btnode *)tep[0].cachep->buf, 0));
#if 0
	        printf("munging %d\n", tomung);
#endif
	    }
	}
	err = noErr;
    }
    if (err == noErr)
	err = updatenumentries(btpb->trail[0].cachep, -1);
    return err;
}

PUBLIC void makecatparam(btparam *btpb, HVCB *vcbp, LONGINT dirid,
						     INTEGER namelen, Ptr namep)
{
    btpb->vcbp = vcbp;
    makecatkey((catkey *) &btpb->tofind, dirid, namelen, namep);
    btpb->fp = catcompare;
    btpb->refnum = vcbp->vcbCTRef;
    btpb->leafindex = -1;
}

PUBLIC OSErr dirtyleaf(void *p, HVCB *vcbp)
{
    OSErr err;
    cacheentry *cachep;
    
    cachep = addrtocachep((Ptr) p, vcbp);
    if (cachep) {
#if 0
	if (!(cachep->flags & CACHEBUSY))
	    DebugStr((StringPtr) "\pnot busy");
#endif
	cachep->flags |= CACHEDIRTY;
	err = noErr;
    } else {
        DebugStr((StringPtr) "\paddrtocachep failed");
	err = fsDSIntErr;
    }
    return err; 
}

PRIVATE OSErr valenceadjust(btparam *btpb, INTEGER toadjust, filekind kind)
{
    OSErr err;
    LONGINT *countadj;
    threadrec *thdp;
    directoryrec *drp;
    btparam btparamblock;
    
    err = noErr;
    switch (kind) {
    case regular:
	countadj = &btpb->vcbp->vcbFilCnt;
	break;
    case directory:
	countadj = &btpb->vcbp->vcbDirCnt;
	break;
    default:
#if !defined (LETGCCWAIL)
        countadj = 0;
#endif /* LETGCCWAIL */
	DebugStr((StringPtr) "\punknown valenceadjust");
	err = fsDSIntErr;
	break;
    }
    if (err == noErr) {
	makecatparam(&btparamblock, btpb->vcbp,
					btpb->tofind.catk.ckrParID, 0, (Ptr) 0);
	err = keyfind(&btparamblock);
	if (err == noErr && btparamblock.success) {
	    thdp = (threadrec *) DATAPFROMKEY(btparamblock.foundp);
	    makecatkey((catkey *) &btparamblock.tofind, thdp->thdParID,
				     thdp->thdCName[0], (Ptr) thdp->thdCName+1);
	    /* don't need to remake btparamblock */
	    err = keyfind(&btparamblock);
	    if (err == noErr && btparamblock.success) {
		drp = (directoryrec *) DATAPFROMKEY(btparamblock.foundp);
		drp->dirVal += toadjust;
		err = dirtyleaf(drp, btpb->vcbp);
		if (err == noErr) {
		    *countadj += toadjust;
		    if (drp->dirDirID == 2)
		    if (kind == directory)
			btpb->vcbp->vcbNmRtDirs += toadjust;
		    else
			btpb->vcbp->vcbNmFls += toadjust;
		    btpb->vcbp->vcbFlags |= VCBDIRTY;
#if 0
		    assert(0);
#endif
		}
	    } else {
		if (err == noErr) {
		    DebugStr((StringPtr) "\pno success2");
		    err = fsDSIntErr;
		}
	    }
	} else {
	    if (err == noErr) {
		DebugStr((StringPtr) "\pno success1");
		err = fsDSIntErr;
	    }
	}
    }
    return err;
}

/*
 * filedelete calls btdelete but adjusts valences (is called by dirdelete)
 */
 
PUBLIC OSErr filedelete(btparam *btpb, filekind kind)
{
    OSErr err;
    
    err = btdelete(btpb);
    if (err == noErr)
	err = valenceadjust(btpb, -1, kind);
    return err;
}

/*
 * dirdelete calls filedelete but also deletes the thread record
 */
 
PUBLIC OSErr dirdelete(btparam *btpb)
{
    OSErr err;
    directoryrec *drp;
    LONGINT dirid;
    
    drp = (directoryrec *) DATAPFROMKEY(btpb->foundp);
    dirid = drp->dirDirID;
    err = filedelete(btpb, directory);
    if (err == noErr) { /* nuke the thread record */
	makecatkey((catkey *) &btpb->tofind, dirid, 0, (Ptr) 0);
	btpb->leafindex = -1;
	err = btdelete(btpb);
    }
    return err;
}

PRIVATE ulong findfirstzero(unsigned char *cp)
{
    ulong retval;
    unsigned char c, bit;
    
    retval = 0;
    while (*cp++ == 0xFF)
	retval += 8;
    c = cp[-1];
    for (bit = 0x80; bit & c; bit >>= 1)
	++retval;
    return retval;
}

typedef struct {
    short refnum;
    ulong logbno;
} saverec_t;

PRIVATE OSErr savebusybuffers(HVCB *vcbp, saverec_t ***savehandlep)
{
    INTEGER count;
    cacheentry *cachep;
    cachehead *headp;
    saverec_t tempsaverec, **retval;
    Size cursize;
    
    headp = (cachehead *) vcbp->vcbCtlBuf;
    count = headp->nitems;
    
    retval = (saverec_t **) NewHandle((Size) 0);
    if (retval == 0)
	return MemError();
    cursize = 0;
    for (cachep = headp->flink; --count >= 0; cachep = cachep->flink) {
	if (cachep->flags & CACHEBUSY) {
	    tempsaverec.refnum = cachep->refnum;
	    tempsaverec.logbno = cachep->logblk;
#if 0
	    Munger();
#else
	    SetHandleSize((Handle) retval, cursize + sizeof(tempsaverec));
	    bcopy(&tempsaverec, (char *) *retval + cursize,
							   sizeof(tempsaverec));
	    cursize += sizeof(tempsaverec);
#endif
	}
    }
    *savehandlep = retval;
    return noErr;
}

PRIVATE OSErr restorebusybuffers(saverec_t **savehandle)
{
    INTEGER nentries;
    saverec_t *savep;
    OSErr err, retval;
    cacheentry *notused;
    
    retval = noErr;
    nentries = GetHandleSize((Handle) savehandle) / sizeof(**savehandle);
    HLock((Handle) savehandle);
    for (savep = *savehandle; --nentries >= 0; ++savep) {
	err = getcache(&notused, savep->refnum, savep->logbno, GETCACHESAVE);
	if (retval == noErr)
	    retval = err;
    }
    HUnlock((Handle) savehandle);
    DisposHandle((Handle) savehandle);
    return retval;
}

PRIVATE OSErr getfreenode(cacheentry **newcachepp, cacheentry *block0cachep)
{
    OSErr err, err1;
    cacheentry *newcachep;
    btblock0 *block0p;
    ulong nblocksalloced, newblock;
    ioParam iop;
    INTEGER refnum, flags;
    filecontrolblock *fcbp;
    saverec_t **busysave;
    
    refnum = block0cachep->refnum;
    block0p = (btblock0 *) block0cachep->buf;
    if (block0p->nfreenodes == 0) {
	fcbp = (filecontrolblock *) ((char *)FCBSPtr + refnum);
	iop.ioRefNum = refnum;
#if !defined (UNIX)
	iop.ioMisc = (Ptr) (fcbp->fcbPLen + fcbp->fcbClmpSize);
#else
	iop.ioMisc = (LONGINT) (fcbp->fcbPLen + fcbp->fcbClmpSize);
#endif
	flags = fcbp->fcbMdRByt;
	fcbp->fcbMdRByt |= WRITEBIT;
	err = savebusybuffers(fcbp->fcbVPtr, &busysave);
	if (err != noErr)
	    return err;
	cleancache(fcbp->fcbVPtr);
	err = myPBSetEOF((ioParam *) &iop, FALSE);    /* yahoo */
	fcbp->fcbVPtr->vcbFlags |= VCBDIRTY;
#if 0
	assert(0);
#endif
	flushvcbp(fcbp->fcbVPtr);	/* just setting DIRTY isn't safe */
	err1 = restorebusybuffers(busysave);
	if (err == noErr)
	    err = err1;
	fcbp->fcbMdRByt = flags;
	if (err != noErr)
	    return err;
	nblocksalloced = fcbp->fcbClmpSize / PHYSBSIZE;
	if (nblocksalloced <= 0) {
	    DebugStr((StringPtr) "\pnblocksalloced <= 0");
	    return fsDSIntErr;
	}
	block0p->nfreenodes += nblocksalloced;
	block0p->nnodes     += nblocksalloced;
    }
    newblock = findfirstzero(block0p->map);
    BitSet((Ptr) block0p->map, newblock);
    --block0p->nfreenodes;
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    block0cachep->flags |= CACHEDIRTY;
    err = getcache(&newcachep, refnum, newblock, GETCACHESAVE|GETCACHENOREAD);
    if (err == noErr)
	*newcachepp = newcachep;
    return err;
}

PRIVATE OSErr getnewnode(cacheentry **newcachepp, cacheentry *leftp)
{
    cacheentry *newcachep, *block0cachep, *linkcachep;
    btblock0 *block0p;
    btnode *leftbtp, *newbtp, *linkbtp;
    OSErr err;
    INTEGER refnum;
    ulong newnode, leftnode, flink;
    
    refnum = leftp->refnum;
    err = getcache(&block0cachep, refnum, 0L, GETCACHESAVE);
    if (err != noErr)
	return err;
    block0p = (btblock0 *) block0cachep->buf;
    err = getfreenode(&newcachep, block0cachep);
    *newcachepp = newcachep;
    newbtp = (btnode *) newcachep->buf;
    newnode = newcachep->logblk;
    leftbtp = (btnode *) leftp->buf;
#if defined (CATFILEDEBUG)
    checkbtp(leftbtp);
#endif /* CATFILEDEBUG */
    bcopy(leftbtp, newbtp, sizeof(leftbtp->ndFLink) +
			   sizeof(leftbtp->ndBLink) +
			   sizeof(leftbtp->ndType) +
			   sizeof(leftbtp->ndLevel));
    leftnode = leftp->logblk;
    if (block0p->lastleaf == leftnode)
	block0p->lastleaf = newnode;
    leftbtp->ndFLink = newnode;
    if (!(leftp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    leftp->flags |= CACHEDIRTY;
    newbtp->ndBLink = leftnode;
    if (flink = newbtp->ndFLink) {
	err = getcache(&linkcachep, refnum, flink, 0);
	if (err != noErr)
	    return err;
	linkbtp = (btnode *) linkcachep->buf;
	linkbtp->ndBLink = newnode;
	linkcachep->flags |= CACHEDIRTY;
    }
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
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
 
PRIVATE OSErr slipin(cacheentry *cachep, INTEGER after, anykey *keyp,
			     char *data, INTEGER datasize, cacheentry **cachepp)
{
    INTEGER sizeneeded, nrecs, keysize, freesize, offsetsize, noffsets, i;
    INTEGER newfirst, sizeused, shim;
    char *keylocp, *firstlocp;
    short *firstoffset, *offsetp;
    ushort newnode;
    cacheentry *newcachep;
    btnode *btp, *newbtp;
    HVCB *vcbp;
    BOOLEAN inbtp;
    OSErr err;
    
    btp = (btnode *) cachep->buf;
#if defined (CATFILEDEBUG)
    checkbtp(btp);
#endif /* CATFILEDEBUG */
    vcbp = cachep->vptr;
    firstoffset = (short *)((char *)btp + PHYSBSIZE) - 1;
    nrecs = btp->ndNRecs;
    keysize = EVENUP(((catkey *)keyp)->ckrKeyLen + 1);
    datasize = EVENUP(datasize);
    sizeneeded = keysize + datasize + sizeof(INTEGER);
    freesize = FREESIZE(btp);
    if (sizeneeded <= freesize) {
	/* TESTED:  WORKS! */
	keylocp = (char *) BTENTRY(btp, after+1);
	bcopy(keylocp, keylocp + sizeneeded - sizeof(INTEGER),
		 (char *) BTENTRY(btp, nrecs) - (char *) BTENTRY(btp, after+1));
	bcopy(keyp, keylocp, keysize);
	bcopy(data, keylocp + keysize, datasize);
	for (i = nrecs - after, offsetp = &firstoffset[-nrecs - 1]; --i >= 0;) {
	    offsetp[0] = offsetp[1] + datasize + keysize;
	    ++offsetp;
	}
	++btp->ndNRecs;
	if (cachepp)
	    *cachepp = 0;
	if (!(cachep->flags & CACHEBUSY))
	    errormessage((StringPtr) "\pnot busy", CAUTION);
	cachep->flags |= CACHEDIRTY;
	err = noErr;
#if defined (CATFILEDEBUG)
	checkbtp(btp);
#endif /* CATFILEDEBUG */
    } else {
	/* NOTE: it might be a win to try to shuffle with left and right,
					       but I'm not too concerned now */
	err = getnewnode(&newcachep, cachep);
	if (err != noErr)
	    return err;
	newnode = newcachep->logblk;
	newbtp = (btnode *) newcachep->buf;
	if (cachepp)
	    *cachepp = newcachep;
	
	/* find split */
	inbtp = FALSE;
	for (newfirst = 0, sizeused = 0; sizeused < SIZECUTOFF; newfirst++) {
	    if (after + 1 == newfirst && !inbtp) {
		inbtp = TRUE;
		sizeused += sizeneeded;
/* --> */       --newfirst; /* didn't add in newfirst on this go around */
	    } else
		sizeused += (char *) BTENTRY(btp, newfirst+1) -
			    (char *) BTENTRY(btp, newfirst) + sizeof(INTEGER);
	}
	/* copy group from btp to newbtp */
	firstlocp = (char *) BTENTRY(btp, newfirst);
	bcopy(firstlocp, (char *) newbtp + sizeof(btnode),
				     (char *)  BTENTRY(btp, nrecs) - firstlocp);
	/* adjust offsets */
	noffsets = nrecs - newfirst + 1;
	offsetsize = noffsets * sizeof(INTEGER);
	bcopy(&firstoffset[-nrecs], (char *)newbtp+PHYSBSIZE-offsetsize,
								    offsetsize);
	shim = (char *) BTENTRY(btp, newfirst) - (char *)btp - sizeof(btnode);
	offsetp = (INTEGER *) ((char *)newbtp + PHYSBSIZE); 
	for (i = noffsets; --i >= 0;)
	    *--offsetp -= shim;
	
	newbtp->ndNRecs = noffsets - 1;
	btp->ndNRecs    = newfirst;
	
#if defined (CATFILEDEBUG)
	checkbtp(btp);
	checkbtp(newbtp);
#endif

	if (inbtp)
	    err = slipin(cachep, after, keyp, data, datasize,
							     (cacheentry **) 0);
	else
	    err = slipin(newcachep, after - newfirst, keyp, data, datasize,
							     (cacheentry **) 0);
	if (!(cachep->flags & CACHEBUSY))
	    errormessage((StringPtr) "\pnot busy", CAUTION);
	if (!(newcachep->flags & CACHEBUSY))
	    errormessage((StringPtr) "\pnot busy", CAUTION);
	cachep   ->flags |= CACHEDIRTY;
	newcachep->flags |= CACHEDIRTY;
#if defined (CATFILEDEBUG)
	checkbtp(btp);
	checkbtp(newbtp);
#endif
    }
    return err;
}

PRIVATE OSErr makenewroot(cacheentry *leftp, cacheentry *rightp,
						       cacheentry *block0cachep)
{
    OSErr err;
    btblock0 *block0p;
    cacheentry *newcachep;
    btnode *newbtp, *leftbtp, *rightbtp;
    short *offsetp;
    INTEGER keylen;
    char *keydst;
    
    err = getfreenode(&newcachep, block0cachep);
    if (err != noErr)
	return err;
    block0p = (btblock0 *) block0cachep->buf;
    newbtp   = (btnode *) newcachep->buf;
    newbtp->ndFLink = 0;
    newbtp->ndBLink = 0;
    newbtp->ndType = indexnode;
    newbtp->ndLevel = ++block0p->height;
    leftbtp  = (btnode *) leftp->buf;
    if (rightp) {
	rightbtp = (btnode *) rightp->buf;
	newbtp->ndNRecs = 2;
    } else {
	newbtp->ndNRecs = 1;
#if !defined (LETGCCWAIL)
	rightbtp = 0;
#endif /* LETGCCWAIL */
    }
    if (!(newcachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    newcachep->flags |= CACHEDIRTY;
    block0p->root = newcachep->logblk;
    block0cachep->flags |= CACHEDIRTY;
    offsetp = BTOFFSET(newbtp, 0);
    keylen = EVENUP(block0p->indexkeylen + 1);
    offsetp[ 0] = sizeof(btnode);
    keydst = (char *) BTENTRY(newbtp, 0);
    bcopy(BTENTRY(leftbtp, 0), keydst, keylen);
    *keydst = keylen - 1;
    *(ulong *)(keydst + keylen) = leftp->logblk;
    if (rightp) {
	--offsetp;
	offsetp[0] = offsetp[1] + keylen + sizeof(ulong);
	keydst = (char *) BTENTRY(newbtp, 1);
	bcopy(BTENTRY(rightbtp, 0), keydst, keylen);
	*keydst = keylen-1;
	*(ulong *)(keydst + keylen) = rightp->logblk;
    }
    --offsetp;
    offsetp[0] = offsetp[1] + keylen + sizeof(ulong);
#if defined (CATFILEDEBUG)
    checkbtp(newbtp);
    checkbtp(leftbtp);
    if (rightp)
	checkbtp(rightbtp);
#endif
    return noErr;
}

/* TODO: makefirstentry should create just one leaf node! */

PRIVATE OSErr makefirstentry(btparam *btpb, char *datap, INTEGER datasize)
{
    OSErr err;
    cacheentry *block0cachep, *leafp;
    btnode *leafbtp;
    short *offsetp;
    btblock0 *block0p;
    ulong newnode;
    INTEGER keylen;
    char *keydst;
    
    block0cachep = btpb->trail[0].cachep;
    err = getfreenode(&leafp, block0cachep);
    if (err != noErr)
	return err;
    leafbtp = (btnode *) leafp->buf;
    leafbtp->ndFLink = 0;
    leafbtp->ndBLink = 0;
    leafbtp->ndType = leafnode;
    leafbtp->ndLevel = 1;
    leafbtp->ndNRecs = 1;
    offsetp = BTOFFSET(leafbtp, 0);
    *offsetp = sizeof(btnode);
    keylen = EVENUP(btpb->tofind.keylen + 1);
    keydst = (char *) BTENTRY(leafbtp, 0);
    bcopy(&btpb->tofind, keydst, keylen);
    bcopy(datap, keydst+keylen, datasize);
    --offsetp;
    offsetp[0] = offsetp[1] + keylen + EVENUP(datasize);
    if (!(leafp->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    leafp->flags |= CACHEDIRTY;
    block0p = (btblock0 *) block0cachep->buf;
    block0p->height = 1;
    block0p->root = leafp->logblk;
    block0p->numentries = 1;
    newnode = leafp->logblk;
    block0p->firstleaf = newnode;
    block0p->lastleaf  = newnode;
    if (!(block0cachep->flags & CACHEBUSY))
	errormessage((StringPtr) "\pnot busy", CAUTION);
    block0cachep->flags |= CACHEDIRTY;
    return noErr;
}
	
PRIVATE OSErr btcreate(btparam *btpb, void *datap, INTEGER datasize)
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
    
    err = getcache(&block0cachep, btpb->refnum, (ulong) 0, GETCACHESAVE);
    if (err != noErr)
	return err;
    btpb->trail[0].cachep = block0cachep;
    block0p = (btblock0 *) block0cachep->buf;
    if (block0p->numentries == 0)
	return makefirstentry(btpb, datap, datasize);
    
    err = btlegitimize(btpb);
    if (err != noErr)
	return err;
    tep = btpb->trail + btpb->leafindex;
    keytoinsertp = &btpb->tofind;
    datatoinsertp = datap;
    sizetoinsert = datasize;
    done = FALSE;
    refnum = block0cachep->refnum;
    while (!done) {
	err = maketrailentrybusy(tep, refnum);
	err = slipin(tep->cachep, tep->after, keytoinsertp, datatoinsertp,
						      sizetoinsert, &newcachep);
	if (err != noErr)
	    return err;
	if (newcachep) {
	    --tep;
	    if (tep == btpb->trail) {
		err = makenewroot(tep[1].cachep, newcachep, tep->cachep);
		if (err != noErr)
		    return err;
		done = TRUE;
	    } else {
		if (block0p->lastleaf == tep->cachep->logblk) {
		    block0p->lastleaf = newcachep->logblk;
		    block0cachep->flags |= CACHEDIRTY;
		}
		newbtp = (btnode *) newcachep->buf;
#if 0
		keylen = BTENTRY((btnode *)tep->cachep->buf, 0)->keylen;
#else
		keylen = block0p->indexkeylen;
#endif
		keytoinsertp = BTENTRY(newbtp, 0);
		if (keytoinsertp->keylen < keylen) {
		    tempkey = *keytoinsertp;
		    tempkey.keylen = keylen;
		    keytoinsertp = &tempkey;
		} else if (keytoinsertp->keylen > keylen) {
		    DebugStr((StringPtr) "\pkeytoinsertp->keylen too big");
		    return fsDSIntErr;
		}
		datatoinsertp = (char *) &newcachep->logblk;
		sizetoinsert = sizeof(newcachep->logblk);
	    }
	} else
	    done = TRUE;
    }
    for (tep = btpb->trail + btpb->leafindex;
			       tep > btpb->trail+1 && tep->after == -1; --tep) {
	tomung = tep[-1].after;
	if (tomung == -1)
	    tomung = 0;
	err = maketrailentrybusy(tep-1, refnum);
	if (err != noErr)
	    return err;
	btsetkey(tep[-1].cachep, tomung,
				      BTENTRY((btnode *)tep[0].cachep->buf, 0));
    }
    err = updatenumentries(block0cachep, 1);
    return err;
}

PRIVATE void makethreadrec(threadrec *recp, LONGINT parid, StringPtr namep)
{
    bzero(recp, sizeof(*recp));
    recp->cdrType = THREADTYPE;
    recp->thdParID = parid;
    str255assign(recp->thdCName, namep);
}

/*
 * filecreate calls btcreate but adjusts the valence afterward.
 * NOTE: filecreate IS used to create directories as well (see dircreate below)
 */
 
PUBLIC OSErr filecreate(btparam *btpb, void *data, filekind kind)
{
    OSErr err;
    INTEGER datasize;
    
    datasize = kind == directory ? sizeof(directoryrec) : sizeof(filerec);
    err = btcreate(btpb, data, datasize);
    if (err == noErr)
	err = valenceadjust(btpb, 1, kind);
    return err;
}

/*
 * dircreate calls filecreate but also creates a thread record
 */
 
PUBLIC OSErr dircreate(btparam *btpb, directoryrec *data)
{
    OSErr err;
    threadrec rec;
    
    err = filecreate(btpb, data, directory);
    if (err == noErr) {
	makethreadrec(&rec, ((catkey *) &btpb->tofind)->ckrParID,
					  ((catkey *) &btpb->tofind)->ckrCName);
	makecatkey((catkey *) &btpb->tofind, data->dirDirID, 0, (Ptr) 0);
	btpb->leafindex = -1;
	err = btcreate(btpb, &rec, sizeof(rec));
    }
    return err;
}

PUBLIC xtntkey *newextentrecord(filecontrolblock *fcbp, ushort newabn)
{
    xtntrec rec;
    HVCB *vcbp;
    btparam btparamrec;
    OSErr err;
    forktype forkwanted;
    
    vcbp = fcbp->fcbVPtr;
    forkwanted = fcbp->fcbMdRByt & RESOURCEBIT ? resourcefork : datafork;
    bzero(&rec, sizeof(rec));
    makextntparam(&btparamrec, vcbp, forkwanted, fcbp->fcbFlNum, newabn);
    if ((err = btcreate(&btparamrec, rec, sizeof(rec))) != noErr) {
        DebugStr((StringPtr) "\pcouldn't create new xtntrec");
	return 0;
    }
    btparamrec.leafindex = -1;
    err = btlegitimize(&btparamrec);
    if (err != noErr) {
        DebugStr((StringPtr) "\pcouldn't find new xtntrec");
        return 0;
    }
    return (xtntkey *) btparamrec.foundp;
}

PUBLIC OSErr btrename(btparam *btpb, StringPtr newnamep)
{
    btparam newbtparam;
    OSErr err;
    char *datap;
    INTEGER datasize;
    
    newbtparam = *btpb;
    makecatkey((catkey *) &newbtparam.tofind,
		 ((catkey *) &btpb->tofind)->ckrParID, newnamep[0],
							      (Ptr) newnamep+1);
    newbtparam.leafindex = -1;
    err = btlegitimize(btpb);
    if (err != noErr)
	return err;
    if (!btpb->success) {
        DebugStr((StringPtr) "\pno success in btrename");
	return fsDSIntErr;
    }
    datap = DATAPFROMKEY(btpb->foundp);
    switch (((filerec *)datap)->cdrType) {
    case FILETYPE:
	datasize = sizeof(filerec);
	break;
    case DIRTYPE:
	datasize = sizeof(directoryrec);
	break;
    default:
        DebugStr((StringPtr) "\punknown cdrType in btrename");
	return fsDSIntErr;
    }
    err = btcreate(&newbtparam, datap, datasize);
    if (err != noErr)
	return err;
    btpb->leafindex = -1;
    err = btdelete(btpb);
    return err;
}

#define STARTFLAGS (1<<7)   /* IMIV-172 record used */

PUBLIC OSErr btcreateemptyfile(btparam *btpb)
{
    OSErr err;
    filerec rec;
    HVCB *vcbp;
    
    vcbp = btpb->vcbp;
    bzero(&rec, sizeof(rec));
    rec.cdrType = FILETYPE;
    rec.filFlags = STARTFLAGS;
    rec.filFlNum = vcbp->vcbNxtCNID++;
    vcbp->vcbFlags |= VCBDIRTY;
#if 0
    assert(0);
#endif
    rec.filMdDat = rec.filCrDat = Time;
    err = filecreate(btpb, &rec, regular);
#if defined (CATFILEDEBUG)
    checkleaves(vcbp->vcbCTRef);
#endif /* CATFILEDEBUG */
    return err;
}

PUBLIC OSErr btcreateemptydir(btparam *btpb, LONGINT *newidp)
{
    directoryrec rec;
    HVCB *vcbp;
    
    vcbp = btpb->vcbp;
    bzero(&rec, sizeof(rec));
    rec.cdrType = DIRTYPE;
    rec.dirFlags = STARTFLAGS;
    *newidp = rec.dirDirID = vcbp->vcbNxtCNID++;
    vcbp->vcbFlags |= VCBDIRTY;
#if 0
    assert(0);
#endif
    rec.dirMdDat = rec.dirCrDat = Time;
    return dircreate(btpb, &rec);
}

/*
 * NOTE: we don't do any cacheing below ... this results in bad O()
 */
 
PUBLIC OSErr btpbindex (ioParam *pb, LONGINT dirid, HVCB **vcbpp,
			   filerec **frpp, catkey **catkeypp, BOOLEAN filesonly)
{
    ioParam newpb;
    btparam btparamrec;
    filekind kind;
    HVCB *vcbp;
    OSErr err;
    BOOLEAN done;
    LONGINT count;
    btnode *btp;
    INTEGER index;
    anykey *entryp;
    LONGINT flink;
    cacheentry *cachep;
    INTEGER refnum;
    filerec *frp;
    
    newpb = *pb;
    newpb.ioNamePtr = (StringPtr) "";
    *vcbpp = 0;
    
    kind = thread;
    err = findvcbandfile(&newpb, dirid, &btparamrec, &kind, FALSE);
    if (err == noErr) {
    	vcbp = btparamrec.vcbp;
	*vcbpp = vcbp;
	if (!btparamrec.success || kind != thread) {
	    DebugStr((StringPtr) "\pdidn't find thread");
	    err = fsDSIntErr;
	} else {
	    count = ((fileParam *)pb)->ioFDirIndex;
	    cachep = btparamrec.trail[btparamrec.leafindex].cachep;
	    btp = (btnode *) cachep->buf;
	    index = btparamrec.trail[btparamrec.leafindex].after;
	    refnum = cachep->refnum;
	    for (done = FALSE; !done;) {
	    	if (++index < btp->ndNRecs) {
	    	    ;	/* nothing to do here; bumping index was sufficient */
	    	} else if (flink = btp->ndFLink) {
	    	    cachep->flags &= ~CACHEBUSY;
	   	    err = getcache(&cachep, refnum, flink, GETCACHESAVE);
	   	    if (err != noErr)
	   	        done = TRUE;
	    	    btp = (btnode *) cachep->buf;
	    	    index = 0;
	    	} else
	    	    done = TRUE;
	    	entryp = BTENTRY(btp, index);
	    	if (dirid != 1 && entryp->catk.ckrParID != dirid)
	    	    done = TRUE;
	    	else if (!done) {
	    	    frp = (filerec *) DATAPFROMKEY(entryp);
	    	    if (frp->cdrType == FILETYPE ||
	    	    		      (!filesonly && frp->cdrType == DIRTYPE)) {
	    	        if (--count == 0)
	    	            done = TRUE;
	    	    }
	    	}
	    }
	}
	if (count == 0) {
	    *frpp = frp;
	    *catkeypp = (catkey *) entryp;
	} else
	    err = fnfErr;
    }
    return err;
}

#if defined(UNIX)
#undef Time
PUBLIC LONGINT Time;
#endif

#endif

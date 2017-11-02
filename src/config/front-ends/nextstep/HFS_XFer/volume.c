#if defined(OUTDATEDCODE)
#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>
#include "MemoryMgr.h"

/*
 * NOTE: in the routines below there is no freeing of memory if an error is
 *       detected.  This should be done sometime.
 */
 
PRIVATE OSErr readvolumebitmap(HVCB *vcbp, volumeinfoPtr vp)
{
    OSErr err;
    short nphysrequired;
    
    if (vp->drSigWord != 0x4244)
	err = noMacDskErr;
    else {
	nphysrequired = NPHYSREQ(ROUNDUP8(vp->drNmAlBlks) / 8);
	vcbp->vcbMAdr = NewPtr(PHYSBSIZE * nphysrequired + MADROFFSET);
	vcbp->vcbMLen = nphysrequired + MADROFFSET; /*really add MADROFFSET?*/
	if (!vcbp->vcbMAdr)
	    err = MemError();
	else
	    err = TransPhysBlk(vcbp, vp->drVBMSt * (ulong) PHYSBSIZE,
	       nphysrequired, vcbp->vcbMAdr + MADROFFSET, reading, (long *) 0);
    }
    return err;
}

PRIVATE OSErr initcache(HVCB *vcbp)
{
    GUEST<THz> savezone;
    cachehead *headp;
    cacheentry *cachep;
    
    savezone = TheZone;
    TheZone  = SysZone;
    vcbp->vcbCtlBuf = NewPtr(sizeof(cachehead) +
					    NCACHEENTRIES * sizeof(cacheentry));
    if (!vcbp->vcbCtlBuf)
	return MemError();
    TheZone  = savezone;
    headp = (cachehead *) vcbp->vcbCtlBuf;
    headp->nitems = NCACHEENTRIES;
    headp->flags = CACHEFREE;
    headp->flink = (cacheentry *)(headp + 1);
    headp->blink = headp->flink + NCACHEENTRIES - 1;
    
    for (cachep = headp->flink; cachep <= headp->blink; cachep++) {
	cachep->flink = cachep + 1;
	cachep->blink = cachep - 1;
	cachep->vptr = 0;
	cachep->fileno = 0;
	cachep->flags = 0;
    }
    headp->flink->blink = (cacheentry *) headp;
    headp->blink->flink = (cacheentry *) headp;

    return noErr;
}

PRIVATE OSErr readvolumeinfo(HVCB *vcbp)    /* call once during mounting */
{
    OSErr err;
    
    vcbp->vcbBufAdr = NewPtr((Size) PHYSBSIZE);
    if (!vcbp)
	err = MemError();
    else {
	err = TransPhysBlk(vcbp, (ulong) VOLUMEINFOBLOCKNO * PHYSBSIZE, 1,
					  vcbp->vcbBufAdr, reading, (long *) 0);
	if (err == noErr) {
	    err = readvolumebitmap(vcbp, (volumeinfoPtr) vcbp->vcbBufAdr);
	    if (err == noErr)
		err = initcache(vcbp);
	}
    }
    return err;
}

#define VOLUMEINFOBACKUP(vcbp)  ((vcbp->vcbNmAlBlks * vcbp->vcbAlBlkSiz) + \
				 (vcbp->vcbAlBlSt * PHYSBSIZE))

PRIVATE OSErr writevolumeinfo(HVCB *vcbp, Ptr p)
{
    OSErr err;
    
    err = TransPhysBlk(vcbp, (ulong) VOLUMEINFOBLOCKNO * PHYSBSIZE, 1, p,
							  writing, (long *) 0);
    if (err == noErr)
	err = TransPhysBlk(vcbp, (ulong) VOLUMEINFOBACKUP(vcbp), 1, p, writing,
								   (long *) 0);
    return err;
}

OSErr flushvcbp(HVCB *vcbp)
{
/* TODO: dump the cache */
#if 0
    volumeinfoPtr vip;
    filecontrolblock *fcbp;
    
    vip = (volumeinfoPtr) vcbp->vcbBufAdr;
    memcpy(&vip->drSigWord, &vcbp->vcbSigWord, (size_t) 64);
    memcpy(&vip->drVolBkUp, &vcbp->vcbVolBkUp, (size_t) 66);
    fcbp = (filecontrolblock *)((char *)FCBSPtr + vcbp->vcbXTRef);
    vip->drXTFlSize = fcbp->fcbPLen;
    memcpy(&vip->drXTExtRec, &fcbp->fcbExtRec,
    					      (size_t) sizeof(fcbp->fcbExtRec));
    fcbp = (filecontrolblock *)((char *)FCBSPtr + vcbp->vcbCTRef);
    vip->drCTFlSize = fcbp->fcbPLen;
    memcpy(&vip->drCTExtRec, &fcbp->fcbExtRec,
    					      (size_t) sizeof(fcbp->fcbExtRec));
    return writevolumeinfo(vcbp, (Ptr) vip);
#else
    Ptr p;
    OSErr err;
    volumeinfoPtr vip;
    filecontrolblock *fcbp;
    
    if (vcbp->vcbFlags & VCBDIRTY) {
	p = NewPtr((Size) 512);

	vip = (volumeinfoPtr) p;
	memcpy(&vip->drSigWord, &vcbp->vcbSigWord, (size_t) 64);
	memcpy(&vip->drVolBkUp, &vcbp->vcbVolBkUp, (size_t) 66);
	fcbp = (filecontrolblock *)((char *)FCBSPtr + vcbp->vcbXTRef);
	vip->drXTFlSize = fcbp->fcbPLen;
	memcpy(&vip->drXTExtRec, &fcbp->fcbExtRec,
    					      (size_t) sizeof(fcbp->fcbExtRec));
	fcbp = (filecontrolblock *)((char *)FCBSPtr + vcbp->vcbCTRef);
	vip->drCTFlSize = fcbp->fcbPLen;
	memcpy(&vip->drCTExtRec, &fcbp->fcbExtRec,
    					      (size_t) sizeof(fcbp->fcbExtRec));
	err = writevolumeinfo(vcbp, p);
	DisposPtr(p);
	flushcachevcbp(vcbp);
	vcbp->vcbFlags &= ~VCBDIRTY;
	return err;
    } else
	return noErr;
#endif
}

PRIVATE HVCB *vcbbyname(StringPtr name)
{
    HVCB *vcbp;
    
    for (vcbp = (HVCB *) VCBQHdr.qHead; vcbp &&
	      !EqualString(vcbp->vcbVN, name, false, true) ;
						   vcbp = (HVCB *) vcbp->qLink)
	;
    return vcbp;
}
	
PRIVATE HVCB *vcbbydrive(short vrefnum)
{
    HVCB *vcbp;
    
    for (vcbp = (HVCB *) VCBQHdr.qHead; vcbp && vcbp->vcbDrvNum !=  vrefnum;
						   vcbp = (HVCB *) vcbp->qLink)
	;
    return vcbp;
}

PRIVATE HVCB *vcbbyvrn(short vrefnum)
{
    HVCB *vcbp;
    
    for (vcbp = (HVCB *) VCBQHdr.qHead; vcbp && vcbp->vcbVRefNum !=  vrefnum;
						   vcbp = (HVCB *) vcbp->qLink)
	;
    return vcbp;
}

PUBLIC HVCB *findvcb(short vrefnum, StringPtr name, LONGINT *diridp)
{
    HVCB *vcbp;
    INTEGER namelen;
    Str255 tempname;
    char *colonp;
    wdentry *wdp;
    
    namelen = name ? name[0] : 0;
    vcbp = 0;
    if (namelen && name[1] != ':' &&
			   (colonp = indexn((char *) name+2, ':', namelen-1))) {
	tempname[0] = colonp - (char *) name - 1;
	memcpy((char *)tempname+1, (char *) name+1, (size_t) tempname[0]);
	vcbp = vcbbyname(tempname);
	if (vcbp && diridp)
	    *diridp = 1;
    }
    if (!vcbp) {
	if (vrefnum > 0)
	    vcbp = vcbbydrive(vrefnum);
	else if (vrefnum < 0) {
	    if (ISWDNUM(vrefnum)) {
		wdp = WDNUMTOWDP(vrefnum);
		vcbp = wdp->vcbp;
		*diridp = wdp->dirid;
	    } else
		vcbp = vcbbyvrn(vrefnum);
	} else
	    vcbp = (HVCB *) DefVCBPtr;
    }
    return vcbp;
}

PRIVATE INTEGER drvtodref(INTEGER vref) /* TODO:  flesh this out */
{
    switch (vref) {
    case 1:
    case 2:
	return -5;
    case 3:
    case 4:
	return -2;
    default:
	return 0;
    }
}

PRIVATE INTEGER openxtnt(long filnum, long clpsize, long filsize, xtntrec xtr,
								     HVCB *vcbp)
{
    filecontrolblock *fcbp;
    INTEGER retval;
    
    fcbp = getfreefcbp();
    if (fcbp) {
	fcbp->fcbFlNum = filnum;
	fcbp->fcbMdRByt = 0;
	fcbp->fcbTypByt = 0;
	fcbp->fcbSBlk = 0;
	fcbp->fcbEOF = filsize;
	fcbp->fcbPLen = filsize;
	fcbp->fcbCrPs = 0;
	fcbp->fcbVPtr = vcbp;
	fcbp->fcbBfAdr = 0;
	fcbp->fcbFlPos = 0;
	fcbp->fcbClmpSize = clpsize;
	fcbp->fcbBTCBPtr = 0;
	memcpy(fcbp->fcbExtRec, xtr, (size_t) sizeof(xtntrec));
	fcbp->fcbFType = 0;
	fcbp->fcbCatPos = 0;
	fcbp->fcbDirID = 0;
	fcbp->fcbCName[0] = 0;
	retval = (char *) fcbp - (char *) FCBSPtr;
    } else
	retval = 0;
    return retval;
}

#define XTNUM   3
#define CTNUM   4

static INTEGER defvrn;
static HVCB *defvcbp;

PUBLIC OSErr myPBMountVol(volumeParam *pb)
{
    HVCB *vcbp;
    OSErr err;
#if !defined (UNIX)
    static INTEGER vrn = 0; /* TODO: get low-memory global from MPW equates */
#else
    static INTEGER vrn = -99; /* TODO: get low-memory global from MPW equates */
#endif
    volumeinfoPtr vip;
    
    vcbp = vcbbydrive(pb->ioVRefNum);
    if (vcbp)
	err = volOnLinErr;
    else {
	vcbp = (HVCB *) NewPtr((Size) sizeof(HVCB));
	if (!vcbp)
	    err = MemError();
	else {
	    err = readvolumeinfo(vcbp);
	    if (err == noErr) {
		vip = (volumeinfoPtr) vcbp->vcbBufAdr;
		memcpy(&vcbp->vcbSigWord, &vip->drSigWord, (size_t) 64);
		vcbp->vcbDrvNum = pb->ioVRefNum;
		vcbp->vcbDRefNum = drvtodref(pb->ioVRefNum);
		vcbp->vcbFSID = 0;
		vcbp->vcbVRefNum = --vrn;
		vcbp->vcbDirIndex = 0;
		vcbp->vcbDirBlk = 0;
		vcbp->vcbFlags = 0;
		memcpy(&vcbp->vcbVolBkUp, &vip->drVolBkUp, (size_t) 66);
		
		vcbp->vcbXTAlBlks = vip->drXTFlSize / vip->drAlBlkSiz;
		vcbp->vcbCTAlBlks = vip->drCTFlSize / vip->drAlBlkSiz;
		
		vcbp->vcbXTRef = openxtnt(XTNUM, vip->drXTClpSiz,
					vip->drXTFlSize, vip->drXTExtRec, vcbp);
		vcbp->vcbCTRef = openxtnt(CTNUM, vip->drCTClpSiz,
					vip->drCTFlSize, vip->drCTExtRec, vcbp);
		
		vcbp->vcbDirIDM = 0;
		vcbp->vcbOffsM = 0;
		
		if (!vcbp->vcbCTRef)
		    err = tmfoErr;
		if (err == noErr) {
		    Enqueue((QElemPtr) vcbp, &VCBQHdr);
		    pb->ioVRefNum = vcbp->vcbVRefNum;
		    if (!defvcbp) {
			defvcbp = vcbp;
			defvrn = vcbp->vcbVRefNum;
		    }
		}
	    }
	}
    }
    PBRETURN(pb, err);
}

PRIVATE void goofyclip(unsigned short *up)
{
    if (*up > 0x7C00)   /* IMIV-130 */
	*up = 0x7C00;
}

/*
 * getworkingdir returns the directory id associated with vrefnum
 */
 
PRIVATE LONGINT getworkingdir(INTEGER vrefnum)
{
    LONGINT retval;
    wdentry *wdp;
    
    if (ISWDNUM(vrefnum)) {
	wdp = WDNUMTOWDP(vrefnum);
	retval = wdp->dirid;
    } else
	retval = 0;
    return retval;
}

/*
 * getnmfls finds a directory's valence
 */
 
PRIVATE unsigned short getnmfls(HVCB *vcbp, INTEGER workingdirnum)
{
    LONGINT dirid;
    catkey key;
    threadrec *thp;
    unsigned short retval;
    btparam btparamrec;
    OSErr err;
    
    dirid = getworkingdir(workingdirnum);
    makecatparam(&btparamrec, vcbp, dirid, 0, (Ptr) 0);
    err = keyfind(&btparamrec);
    if (err == noErr && btparamrec.success) {
	thp = (threadrec *) DATAPFROMKEY(btparamrec.foundp);
	key.ckrParID = thp->thdParID;
	str255assign(key.ckrCName, thp->thdCName);
	key.ckrKeyLen = sizeof(LONGINT) + 2 + key.ckrCName[0];
	err = keyfind(&btparamrec);
	if (err == noErr && btparamrec.success)
	    retval = ((directoryrec *)DATAPFROMKEY(btparamrec.foundp))->dirVal;
	else
	    retval = 0;
    } else
	retval = 0;
    return retval;
}

#define RETURN  return
PRIVATE OSErr commonGetVInfo(HVolumeParam *pb, BOOLEAN async, fstype fs)
{
    HVCB *vcbp;
    INTEGER workingdirnum;
    
    if (pb->ioVolIndex > 0) {
	vcbp = (HVCB *) indexqueue(&VCBQHdr, pb->ioVolIndex);
	workingdirnum = 0;
    } else {
	if (pb->ioVolIndex == 0)
	    vcbp = (HVCB *) findvcb(pb->ioVRefNum, (StringPtr) "", (LONGINT *) 0);
	else /* if (pb->ioVolIndex < 0) */
	    vcbp = (HVCB *) findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
	workingdirnum = getworkingdir(pb->ioVRefNum);
    }
	
    if (!vcbp)
/*-->*/ PBRETURN(pb, nsvErr);

    if (pb->ioNamePtr)
	str255assign(pb->ioNamePtr, (StringPtr) vcbp->vcbVN);
    pb->ioVCrDate = vcbp->vcbCrDate;
    pb->ioVAtrb = vcbp->vcbAtrb;
    
    if (workingdirnum)
	pb->ioVNmFls = getnmfls(vcbp, workingdirnum);
    else
	pb->ioVNmFls = vcbp->vcbNmFls;
     
    pb->ioVNmAlBlks = vcbp->vcbNmAlBlks;
    pb->ioVAlBlkSiz = vcbp->vcbAlBlkSiz;
    pb->ioVClpSiz = vcbp->vcbClpSiz;
    pb->ioAlBlSt = vcbp->vcbAlBlSt;
    pb->ioVNxtCNID = vcbp->vcbNxtCNID;
    pb->ioVFrBlk = vcbp->vcbFreeBks;
    switch (fs) {
    case mfs:
	((volumeParam *)pb)->ioVLsBkUp = vcbp->vcbVolBkUp;
	((volumeParam *)pb)->ioVDirSt = 0;
	((volumeParam *)pb)->ioVBlLn = 0;
	if (!workingdirnum)
	    pb->ioVRefNum = vcbp->vcbVRefNum;
	goofyclip((unsigned short *) &pb->ioVNmAlBlks);
	goofyclip((unsigned short *) &pb->ioVFrBlk);
	break;
    case hfs:
	pb->ioVLsMod = vcbp->vcbLsMod;
	pb->ioVBitMap = vcbp->vcbVBMSt;
#if !defined (THINKCMESSED)
	pb->ioVAllocPtr = vcbp->vcbAllocPtr;
#else /* THINKCMESSED */
	pb->ioAllocPtr = vcbp->vcbAllocPtr;
#endif /* THINKCMESSED */
	pb->ioVRefNum = vcbp->vcbVRefNum;
	pb->ioVSigWord = vcbp->vcbSigWord;
	pb->ioVDrvInfo = vcbp->vcbDrvNum;
	pb->ioVDRefNum = vcbp->vcbDRefNum;
	pb->ioVFSID = vcbp->vcbFSID;
	pb->ioVBkUp = vcbp->vcbVolBkUp;
	pb->ioVSeqNum = vcbp->vcbVSeqNum;
	pb->ioVWrCnt = vcbp->vcbWrCnt;
	pb->ioVFilCnt = vcbp->vcbFilCnt;
	pb->ioVDirCnt = vcbp->vcbDirCnt;
	memcpy(pb->ioVFndrInfo, vcbp->vcbFndrInfo,
					      (size_t) sizeof(pb->ioVFndrInfo));
	break;
    }
    PBRETURN(pb, noErr);
}
#undef RETURN

PUBLIC OSErr myPBGetVInfo(volumeParam *pb, BOOLEAN async)
{
    return commonGetVInfo((HVolumeParam *)pb, async, mfs);
}

PUBLIC OSErr myPBHGetVInfo(HVolumeParam *pb, BOOLEAN async)
{
    return commonGetVInfo(pb, async, hfs);
}

#define ATRBMASK    VSOFTLOCKBIT

PUBLIC OSErr myPBSetVInfo(HVolumeParam *pb, BOOLEAN async)
{
    OSErr err;
    HVCB *vcbp;
    
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    if (vcbp) {
	if (vcbp->vcbAtrb & VHARDLOCKBIT)
	    err = wPrErr;
	else {
	    if (pb->ioNamePtr)
		str255assign((StringPtr) vcbp->vcbVN, pb->ioNamePtr);
	    vcbp->vcbCrDate  = pb->ioVCrDate;
	    vcbp->vcbLsMod   = pb->ioVLsMod;
	    vcbp->vcbAtrb    = (vcbp->vcbAtrb & ~ATRBMASK) |
				 (pb->ioVAtrb &  ATRBMASK);
	    vcbp->vcbClpSiz  = pb->ioVClpSiz;
	    vcbp->vcbVolBkUp = pb->ioVBkUp;
	    vcbp->vcbVSeqNum = pb->ioVSeqNum;
	    memcpy(vcbp->vcbFndrInfo, pb->ioVFndrInfo, (size_t) 32);
	    vcbp->vcbFlags |= VCBDIRTY;
#if 0
	    assert(0);
#endif
	}
    } else
	err = nsvErr;
    PBRETURN(pb, err);
}

PRIVATE void getvolcommon(volumeParam *pb)
{
    str255assign(pb->ioNamePtr, (StringPtr) defvcbp->vcbVN);
    pb->ioVRefNum = defvrn;
}

PUBLIC OSErr myPBGetVol(volumeParam *pb, BOOLEAN async)
{
    getvolcommon(pb);
    PBRETURN(pb, noErr);
}

PUBLIC OSErr myPBHGetVol(WDPBPtr pb, BOOLEAN async)
{
    wdentry *wdp;
    
    getvolcommon((volumeParam *) pb);
    if (ISWDNUM(defvrn)) {
	wdp = WDNUMTOWDP(defvrn);
	pb->ioWDProcID = wdp->procid;
	pb->ioWDVRefNum = wdp->vcbp->vcbVRefNum;
	pb->ioWDDirID = wdp->dirid;
    } else {
	pb->ioWDProcID = 0;
	pb->ioWDVRefNum = defvrn;
	pb->ioWDDirID = 2;
    }
    PBRETURN(pb, noErr);
}

PRIVATE OSErr setvolhelper(volumeParam *pb, BOOLEAN aysnc, LONGINT dirid)
{
    HVCB *vcbp;
    OSErr err;
    wdentry *wdp;
    WDPBRec wdpbrec;
    
    err = noErr;
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    if (dirid <= 2)
	defvrn = pb->ioVRefNum;
    else {
	if (ISWDNUM(defvrn)) {
	    wdp = WDNUMTOWDP(defvrn);
	    if (wdp->procid == 'STVL') {
		wdpbrec.ioVRefNum = defvrn;
		err = myPBCloseWD(&wdpbrec, false);
	    }
	}
	if (err == noErr) {
	    wdpbrec.ioNamePtr = 0;
	    wdpbrec.ioVRefNum = vcbp->vcbVRefNum;
	    wdpbrec.ioWDProcID = 'STVL';
	    wdpbrec.ioWDDirID = dirid;
	    err = myPBOpenWD(&wdpbrec, false);
	    if (err == noErr)
		defvrn = wdpbrec.ioVRefNum;
	}
    }
    if (err == noErr)
	defvcbp = vcbp;

    PBRETURN(pb, err);
}

PUBLIC OSErr myPBSetVol(volumeParam *pb, BOOLEAN async)
{
    return setvolhelper(pb, async, 1L);
}

PUBLIC OSErr myPBHSetVol(WDPBPtr pb, BOOLEAN async)
{
    return setvolhelper((volumeParam *) pb, async, pb->ioWDDirID);
}

PUBLIC OSErr myPBFlushVol(volumeParam *pb, BOOLEAN async)
{
    VCB *vcbp;
    OSErr err;

    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    if (vcbp)
	err = flushvcbp(vcbp);
    else
	err = nsvErr;
    PBRETURN(pb, err);
}

PRIVATE void closeallvcbfiles(HVCB *vcbp)
{
    filecontrolblock *fcbp, *efcbp;
    ioParam iopb;
    short length;
    
    length = *(short *)FCBSPtr;
    fcbp = (filecontrolblock *) ((short *)FCBSPtr+1);
    efcbp = (filecontrolblock *) ((char *)FCBSPtr + length);
    for (;fcbp < efcbp; fcbp = (filecontrolblock *) ((char *)fcbp + FSFCBLen))
	if (fcbp->fcbFlNum && fcbp->fcbVPtr == vcbp) {
	    iopb.ioRefNum = (char *) fcbp - (char *) FCBSPtr;
	    myPBFlushFile((ioParam *) &iopb, false);
	}
}

PUBLIC OSErr myPBUnmountVol(volumeParam *pb)
{
    OSErr err;
    HVCB *vcbp;
    
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    if (vcbp) {
	closeallvcbfiles(vcbp);
	err = flushvcbp(vcbp);
	Dequeue((QElemPtr) vcbp, &VCBQHdr);
	DisposPtr(vcbp->vcbMAdr);
	DisposPtr(vcbp->vcbBufAdr);
	DisposPtr(vcbp->vcbCtlBuf);
	DisposPtr((Ptr) vcbp);
    } else
	err = nsvErr;
    PBRETURN(pb, err);
}

PRIVATE OSErr offlinehelper(volumeParam *pb, HVCB *vcbp)
{
    OSErr err;
    extern OSErr updatefloppy( void );
    
    err = myPBFlushVol((volumeParam *) pb, false);
    if (err == noErr) {
	if (vcbp) {
	    DisposPtr(vcbp->vcbMAdr);
	    DisposPtr(vcbp->vcbBufAdr);
	    DisposPtr(vcbp->vcbCtlBuf);
	    vcbp->vcbMAdr = 0;
	    vcbp->vcbBufAdr = 0;
	    vcbp->vcbCtlBuf = 0;
	    vcbp->vcbDrvNum = 0;
	    /* TODO:  look for offline flags in mpw equate files and set them */
	} else
	    err = nsvErr;
    }
#if defined(UNIX)
    if (err == noErr)
	err = updatefloppy();
#endif
    return err;
}

PUBLIC OSErr myPBOffline(volumeParam *pb)
{
    OSErr err;
    HVCB *vcbp;
    
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    err = offlinehelper(pb, vcbp);
    PBRETURN(pb, err);
}

PUBLIC OSErr myPBEject(volumeParam *pb)
{
    OSErr err;
    HVCB *vcbp;
    extern OSErr ejectfloppy( void );
    
    vcbp = findvcb(pb->ioVRefNum, pb->ioNamePtr, (LONGINT *) 0);
    if (vcbp) {
	vcbp->vcbDRefNum = - vcbp->vcbDrvNum;
	err = offlinehelper(pb, vcbp);
    } else
	err = nsvErr;
#if defined(UNIX)
    if (err == noErr)
	err = ejectfloppy();
#endif
    PBRETURN(pb, err);
}

PUBLIC OSErr pbvolrename(ioParam *pb, StringPtr newnamep)
{
    OSErr err;
    HParamBlockRec hpb;
    
    hpb.volumeParam.ioNamePtr = pb->ioNamePtr;
    hpb.volumeParam.ioVRefNum = pb->ioVRefNum;
    hpb.volumeParam.ioVolIndex = 0;
    err = myPBHGetVInfo((HVolumeParam *) &hpb, false);
    if (err == noErr) {
	hpb.volumeParam.ioNamePtr = newnamep;
	err = myPBSetVInfo((HVolumeParam *) &hpb, false);
    }
    return err;
}
#endif

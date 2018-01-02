/* Copyright 1992 - 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "MemoryMgr.h"
#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/hfs_plus.h"

#if defined(CYGWIN32)
#include "winfs.h"
#endif

using namespace Executor;

/*
 * TODO: support read and write count.  This will make it possible to
 *	 discriminate between identically named disks (i.e. when you
 *	 have two Untitled disks that have been ejected).  Currently
 *	 disks are matched by name and name only.
 */

/*
 * NOTE: in the routines below there is no freeing of memory if an error is
 *       detected.  This should be done sometime.
 */

/*
 * NOTE: NewPtr_aligned_4 is so that our miscellaneous buffers will be aligned
 *	 properly for DMA transfers on the NeXT.  Depending on the
 *	implementation of NewPtr, the following loop could
 *	eat all of memory, but it would require a pretty unlikely
 *	implementation of NewPtr.
 */

#define NewPtr_aligned_4(x, y) NewPtr(x)

#if !defined(NewPtr_aligned_4)
static Ptr NewPtr_aligned_4(Size size, INTEGER align)
{
    Ptr p, p2, p3;
    Size shim;

    p = NewPtr(size);
    if(((LONGINT)p & 3) == align)
        return p;
    else
    {
        shim = 0;
        while(p)
        {
            DisposPtr(p);
            p2 = NewPtr(++shim);
            p = NewPtr(size);
            p3 = NewPtr(shim);
            DisposPtr(p3);
            if(((LONGINT)p & 3) == align)
            {
                DisposPtr(p2);
                return p;
            }
            else
            {
                DisposPtr(p);
                DisposPtr(p2);
                p2 = NewPtr(shim + 1);
                p = NewPtr(size);
                p3 = NewPtr(shim + 1);
                DisposPtr(p3);
                if(((LONGINT)p & 3) == align)
                {
                    DisposPtr(p2);
                    return p;
                }
                else
                    DisposPtr(p2);
            }
        }
        return p;
    }
}
#endif

static OSErr readvolumebitmap(HVCB *vcbp, volumeinfoPtr vp)
{
    OSErr err;
    short nphysrequired;

    if(Cx(vp->drSigWord) != 0x4244)
    {
        GUEST<uint32_t> *words;

        err = noMacDskErr;

        words = (GUEST<uint32_t> *)vp;
        warning_fs_log("sigword = 0x%02x (%08x %08x %08x %08x)",
                       CW(vp->drSigWord), (unsigned)CL(words[0]), (unsigned)CL(words[1]),
                       (unsigned)CL(words[2]), (unsigned)CL(words[3]));
    }
    else
    {
        nphysrequired = NPHYSREQ(ROUNDUP8(Cx(vp->drNmAlBlks)) / 8);
        vcbp->vcbMAdr = RM(NewPtr_aligned_4(PHYSBSIZE * nphysrequired + MADROFFSET, 0));
        vcbp->vcbMLen = CW(nphysrequired + MADROFFSET);
        /*really add MADROFFSET?*/
        if(!vcbp->vcbMAdr)
            err = MemError();
        else
            err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                      CW(vp->drVBMSt) * (ULONGINT)PHYSBSIZE,
                                      nphysrequired,
                                      MR(vcbp->vcbMAdr) + MADROFFSET, reading,
                                      nullptr);
    }
    return err;
}

static OSErr initcache(HVCB *vcbp)
{
    GUEST<THz> savezone;
    cachehead *headp;
    cacheentry *cachep;
    INTEGER align;

    savezone = TheZone;
    TheZone = SysZone;
    align = ((char *)&cachep->buf - (char *)&cachep) & 3;
    vcbp->vcbCtlBuf = RM(NewPtr_aligned_4(sizeof(cachehead) + NCACHEENTRIES * sizeof(cacheentry), align));
    if(!vcbp->vcbCtlBuf)
        return MemError();
    TheZone = savezone;
    headp = (cachehead *)MR(vcbp->vcbCtlBuf);
    headp->nitems = CW(NCACHEENTRIES);
    headp->flags = 0;
    headp->flink = RM((cacheentry *)(headp + 1));
    headp->blink = RM(MR(headp->flink) + NCACHEENTRIES - 1);

    for(cachep = MR(headp->flink); cachep <= MR(headp->blink); cachep++)
    {
        cachep->flink = RM(cachep + 1);
        cachep->blink = RM(cachep - 1);
        cachep->vptr = 0;
        cachep->fileno = 0;
        cachep->flags = CACHEFREE;
    }
    MR(headp->flink)->blink = RM((cacheentry *)headp);
    MR(headp->blink)->flink = RM((cacheentry *)headp);

    return noErr;
}

bool Executor::ROMlib_hfs_plus_support = false;

static bool
is_hfs_plus_wrapper(volumeinfoPtr vp)
{
    bool retval;

    retval = vp->drVCSize == CWC(0x482b);
    return retval;
}

static OSErr
check_volume_size(volumeinfoPtr vp)
{
    OSErr retval;

    if(is_hfs_plus_wrapper(vp))
    {
        warning_unexpected("Found wrapped HFS+ volume");
        retval = noErr;
    }
    else
    {
        unsigned short nmalblks;
        unsigned long dralblksiz;

        nmalblks = CW(vp->drNmAlBlks);
        dralblksiz = CL(vp->drAlBlkSiz);
        retval = ((long long)nmalblks * dralblksiz >= 2LL * 1024 * 1024 * 1024
                      ? paramErr
                      : noErr);
        if(retval != noErr)
            warning_unexpected("drNmAlBlks = %d, drAlBlkSiz = %lu",
                               (int)nmalblks, dralblksiz);
    }
    return retval;
}

static OSErr readvolumeinfo(HVCB *vcbp) /* call once during mounting */
{
    OSErr err;

    vcbp->vcbBufAdr = RM(NewPtr_aligned_4((Size)PHYSBSIZE, 0));
    if(!vcbp)
        err = MemError();
    else
    {
        err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                  (ULONGINT)VOLUMEINFOBLOCKNO * PHYSBSIZE,
                                  1, MR(vcbp->vcbBufAdr), reading,
                                  nullptr);
        if(err == noErr)
            err = check_volume_size((volumeinfoPtr)MR(vcbp->vcbBufAdr));
        if(err == noErr)
        {
            err = readvolumebitmap(vcbp, (volumeinfoPtr)MR(vcbp->vcbBufAdr));
            if(err == noErr)
                err = initcache(vcbp);
        }
    }
    return err;
}

#define VOLUMEINFOBACKUP(vcbp) \
    ((CW(vcbp->vcbNmAlBlks) * Cx(vcbp->vcbAlBlkSiz)) + (Cx(vcbp->vcbAlBlSt) * PHYSBSIZE))

void Executor::vcbsync(HVCB *vcbp)
{
    if(!ROMlib_nosync)
        fsync(((VCBExtra *)vcbp)->u.hfs.fd);
}

static OSErr writevolumeinfo(HVCB *vcbp, Ptr p)
{
    OSErr err;

    err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                              (ULONGINT)VOLUMEINFOBLOCKNO * PHYSBSIZE,
                              1, p, writing, nullptr);
    if(err == noErr)
        err = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                  (ULONGINT)VOLUMEINFOBACKUP(vcbp), 1, p,
                                  writing, nullptr);
    vcbsync(vcbp);
    return err;
}

OSErr Executor::ROMlib_flushvcbp(HVCB *vcbp)
{
    Ptr realp, p;
    OSErr retval;
    volumeinfoPtr vip;
    filecontrolblock *fcbp;

    retval = ROMlib_flushcachevcbp(vcbp);
    if(retval == noErr)
    {
        if(Cx(vcbp->vcbFlags) & VCBDIRTY)
        {
            realp = (Ptr)alloca((Size)512 + 4); /* needs to be aligned on unix */

            p = (Ptr)(((uintptr_t)realp + 3) & ~3L);
            vip = (volumeinfoPtr)p;
            memmove(&vip->drSigWord, &vcbp->vcbSigWord, (LONGINT)64);
            memmove(&vip->drVolBkUp, &vcbp->vcbVolBkUp, (LONGINT)66);
            fcbp = (filecontrolblock *)((char *)MR(FCBSPtr)
                                        + Cx(vcbp->vcbXTRef));
            vip->drXTFlSize = fcbp->fcbPLen;
            memmove(&vip->drXTExtRec, &fcbp->fcbExtRec,
                    (LONGINT)sizeof(fcbp->fcbExtRec));
            fcbp = (filecontrolblock *)((char *)MR(FCBSPtr)
                                        + Cx(vcbp->vcbCTRef));
            vip->drCTFlSize = fcbp->fcbPLen;
            memmove(&vip->drCTExtRec, &fcbp->fcbExtRec,
                    (LONGINT)sizeof(fcbp->fcbExtRec));
            retval = writevolumeinfo(vcbp, p);
            vcbp->vcbFlags &= CW(~VCBDIRTY);
        }
    }
    return retval;
}

static HVCB *vcbbyname(StringPtr name)
{
    HVCB *vcbp;

    for(vcbp = (HVCB *)MR(VCBQHdr.qHead); vcbp && !EqualString(vcbp->vcbVN, name, false, true);
        vcbp = (HVCB *)MR(vcbp->qLink))
        ;
    return vcbp;
}

#if defined(CYGWIN32) || defined(MSDOS)
#define VCB_CMPN_FUNC strncasecmp
#define VCB_CMP_FUNC strcasecmp
#else
#define VCB_CMPN_FUNC strncmp
#define VCB_CMP_FUNC strcmp
#endif

HVCB *Executor::ROMlib_vcbbybiggestunixname(const char *name)
{
    HVCB *vcbp, *bestvcbp;
    int namesize, bestsize;

    if(strchr(name, '\\'))
    {
        int len;
        char *new_name, *op, c;
        const char *ip;

        len = strlen(name) + 1;
        new_name = (char *)alloca(len);
        for(op = new_name, ip = name; (c = *ip++);)
            *op++ = c == '\\' ? '/' : c;
        *op++ = 0;
        name = new_name;
    }

    bestvcbp = 0;
    bestsize = -1;
    for(vcbp = (HVCB *)MR(VCBQHdr.qHead); vcbp;
        vcbp = (HVCB *)MR(vcbp->qLink))
    {
        if(!vcbp->vcbCTRef)
        {
            namesize = strlen(((VCBExtra *)vcbp)->unixname);
            if(namesize > bestsize && VCB_CMPN_FUNC(((VCBExtra *)vcbp)->unixname, name, namesize) == 0)
            {
                bestsize = namesize;
                bestvcbp = vcbp;
            }
        }
    }
    return bestvcbp;
}

VCBExtra *
Executor::ROMlib_vcbbyunixname(const char *name)
{
    HVCB *vcbp;

    for(vcbp = (HVCB *)MR(VCBQHdr.qHead); vcbp && (vcbp->vcbCTRef || VCB_CMP_FUNC(((VCBExtra *)vcbp)->unixname, name) != 0);
        vcbp = (HVCB *)MR(vcbp->qLink))
        ;
    return (VCBExtra *)vcbp;
}

HVCB *Executor::ROMlib_vcbbydrive(short vrefnum)
{
    HVCB *vcbp;

    for(vcbp = (HVCB *)MR(VCBQHdr.qHead);
        vcbp && Cx(vcbp->vcbDrvNum) != vrefnum;
        vcbp = (HVCB *)MR(vcbp->qLink))
        ;
    return vcbp;
}

DrvQExtra *
Executor::ROMlib_dqbydrive(short vrefnum)
{
    DrvQEl *dp;
    DrvQExtra *retval;
    GUEST<INTEGER> swapped_vrefnum;

    swapped_vrefnum = CW(vrefnum);
    retval = 0;
    for(dp = (DrvQEl *)MR(DrvQHdr.qHead);
        dp && (retval = (DrvQExtra *)((char *)dp - sizeof(LONGINT)),
              retval->dq.dQDrive != swapped_vrefnum);
        dp = (DrvQEl *)MR(dp->qLink))
        ;
    return dp ? retval : 0;
}

HVCB *Executor::ROMlib_vcbbyvrn(short vrefnum)
{
    HVCB *vcbp;

    for(vcbp = (HVCB *)MR(VCBQHdr.qHead);
        vcbp && Cx(vcbp->vcbVRefNum) != vrefnum;
        vcbp = (HVCB *)MR(vcbp->qLink))
        ;
    return vcbp;
}

HVCB *Executor::ROMlib_findvcb(short vrefnum, StringPtr name, LONGINT *diridp,
                                      BOOLEAN usedefault)
{
    HVCB *vcbp;
    INTEGER namelen;
    Str255 tempname;
    char *colonp;
    wdentry *wdp;

    namelen = name ? name[0] : 0;
    vcbp = 0;
    if(namelen && name[1] != ':' && (colonp = ROMlib_indexn((char *)name + 2, ':', namelen - 1)))
    {
        tempname[0] = colonp - (char *)name - 1;
        memmove((char *)tempname + 1, (char *)name + 1, (LONGINT)tempname[0]);
        vcbp = vcbbyname(tempname);
        if(vcbp && diridp)
            *diridp = 1;
    }
    else
    {
        if(vrefnum > 0)
            vcbp = ROMlib_vcbbydrive(vrefnum);
        else if(vrefnum < 0)
        {
            if(ISWDNUM(vrefnum))
            {
                wdp = WDNUMTOWDP(vrefnum);
                vcbp = MR(wdp->vcbp);
                if(diridp)
                    *diridp = CL(wdp->dirid);
            }
            else
                vcbp = ROMlib_vcbbyvrn(vrefnum);
        }
        else if(usedefault || (!name && !vrefnum))
        {
            vcbp = (HVCB *)MR(DefVCBPtr);
            if(diridp)
                *diridp = CL(DefDirID);
        }
    }
    return vcbp;
}

static INTEGER drvtodref(INTEGER vref) /* TODO:  flesh this out */
{
#if 0
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
#else
    return OURHFSDREF;
#endif
}

static INTEGER openxtnt(LONGINT filnum, LONGINT clpsize, LONGINT filsize,
                        xtntrec xtr, HVCB *vcbp)
{
    filecontrolblock *fcbp;
    INTEGER retval;

    fcbp = ROMlib_getfreefcbp();
    if(fcbp)
    {
        fcbp->fcbFlNum = CL(filnum);
        fcbp->fcbMdRByt = 0;
        fcbp->fcbTypByt = 0;
        fcbp->fcbSBlk = 0;
        fcbp->fcbEOF = CL(filsize);
        fcbp->fcbPLen = CL(filsize);
        fcbp->fcbCrPs = 0;
        fcbp->fcbVPtr = RM(vcbp);
        fcbp->fcbBfAdr = 0;
        fcbp->fcbFlPos = 0;
        fcbp->fcbClmpSize = CL(clpsize);
        fcbp->fcbBTCBPtr = 0;
        memmove(fcbp->fcbExtRec, xtr, (LONGINT)sizeof(xtntrec));
        fcbp->fcbFType = 0;
        fcbp->fcbCatPos = 0;
        fcbp->fcbDirID = 0;
        fcbp->fcbCName[0] = 0;
        retval = (char *)fcbp - (char *)MR(FCBSPtr);
    }
    else
        retval = 0;
    return retval;
}

#define XTNUM 3
#define CTNUM 4

INTEGER Executor::ROMlib_nextvrn = 0; /* TODO: low memory global */

OSErr
Executor::hfsPBMountVol(ParmBlkPtr pb, LONGINT floppyfd, LONGINT offset, LONGINT bsize,
                        LONGINT maxbytes, drive_flags_t flags, DrvQExtra *dqp)
{
    HVCB *vcbp, *vcbp2;
    OSErr err;
    volumeinfoPtr vip;
    BOOLEAN alreadythere;
    ULONGINT nblocks;
    GUEST<THz> saveZone;

    warning_fs_log("floppyfd = 0x%x, offset = %d, bsize = %d, maxbytes = %d "
                   "flags = 0x%x",
                   floppyfd, offset, bsize, maxbytes, flags);
    saveZone = TheZone;
    TheZone = SysZone;
    vcbp = ROMlib_vcbbydrive(CW(pb->volumeParam.ioVRefNum));
    if(vcbp)
        err = volOnLinErr;
    else
    {
        vcbp = (HVCB *)NewPtr((Size)sizeof(VCBExtra));
        ((VCBExtra *)vcbp)->u.hfs.fd = floppyfd;
        ((VCBExtra *)vcbp)->u.hfs.offset = offset;
        ((VCBExtra *)vcbp)->u.hfs.bsize = bsize;
        ((VCBExtra *)vcbp)->u.hfs.maxbytes = maxbytes;
        if(!vcbp)
            err = MemError();
        else
        {
            err = readvolumeinfo(vcbp);
            if(err == noErr)
            {
                vip = (volumeinfoPtr)MR(vcbp->vcbBufAdr);
                alreadythere = false;
                for(vcbp2 = (HVCB *)MR(VCBQHdr.qHead); vcbp2;
                    vcbp2 = (HVCB *)MR(vcbp2->qLink))
                    if(EqualString(vcbp2->vcbVN, vip->drVN, true, true)
                       && vcbp2->vcbDrvNum == CWC(0))
                    {
#if 1
                        vcbp2->vcbBufAdr = vcbp->vcbBufAdr;
                        vcbp2->vcbMAdr = vcbp->vcbMAdr;
                        vcbp2->vcbCtlBuf = vcbp->vcbCtlBuf;
#endif
                        ((VCBExtra *)vcbp2)->unixname = ((VCBExtra *)vcbp)->unixname;
                        ((VCBExtra *)vcbp2)->u.hfs.fd = ((VCBExtra *)vcbp)->u.hfs.fd;
                        DisposPtr((Ptr)vcbp);
                        alreadythere = true;
                        vcbp = vcbp2;
                        break;
                    }
                memmove(&vcbp->vcbSigWord, &vip->drSigWord, (LONGINT)64);

                nblocks = (CL(vcbp->vcbAlBlkSiz) / PHYSBSIZE) * CW(vcbp->vcbNmAlBlks) + CW(vcbp->vcbAlBlSt) + 2;
                dqp->dq.dQDrvSz = CW(nblocks);
                dqp->dq.dQDrvSz2 = CW(nblocks >> 16);
                dqp->dq.qType = CWC(1);

                vcbp->vcbDrvNum = pb->volumeParam.ioVRefNum;
                vcbp->vcbDRefNum = CW(drvtodref(Cx(pb->volumeParam.ioVRefNum)));
                vcbp->vcbFSID = 0;
                if(!alreadythere)
                    vcbp->vcbVRefNum = CW(--ROMlib_nextvrn);
                vcbp->vcbDirIndex = 0;
                vcbp->vcbDirBlk = 0;
                vcbp->vcbFlags = 0;
                memmove(&vcbp->vcbVolBkUp, &vip->drVolBkUp, (LONGINT)66);

                vcbp->vcbXTAlBlks = CW(Cx(vip->drXTFlSize) / Cx(vip->drAlBlkSiz));
                vcbp->vcbCTAlBlks = CW(Cx(vip->drCTFlSize) / Cx(vip->drAlBlkSiz));

                vcbp->vcbXTRef = CW(openxtnt(XTNUM, Cx(vip->drXTClpSiz),
                                             Cx(vip->drXTFlSize), vip->drXTExtRec, vcbp));
                vcbp->vcbCTRef = CW(openxtnt(CTNUM, Cx(vip->drCTClpSiz),
                                             Cx(vip->drCTFlSize), vip->drCTExtRec, vcbp));

                vcbp->vcbDirIDM = 0;
                vcbp->vcbOffsM = 0;
                vcbp->vcbAtrb = 0;
                if(flags & DRIVE_FLAGS_FIXED)
                    vcbp->vcbAtrb |= CW(VNONEJECTABLEBIT);

                if(!vcbp->vcbCTRef)
                    err = tmfoErr;
                if(err == noErr)
                {
                    if(!(flags & DRIVE_FLAGS_LOCKED))
                    {
                        OSErr err2;
                        signed char buffer[PHYSBSIZE + 3];
                        signed char *buf;

                        buf = (signed char *)(((uintptr_t)buffer + 3) & ~3L);
                        err2 = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                                   (ULONGINT)VOLUMEINFOBLOCKNO
                                                       * PHYSBSIZE,
                                                   1, buf, reading,
                                                   nullptr);
                        if(err2 == noErr)
                        {
                            err2 = ROMlib_transphysblk(&((VCBExtra *)vcbp)->u.hfs,
                                                       (ULONGINT)VOLUMEINFOBLOCKNO
                                                           * PHYSBSIZE,
                                                       1, buf,
                                                       writing, nullptr);
                            if(err2 == noErr)
                                err2 = ROMlib_flushvcbp(vcbp);
                        }
                        if(err2 != noErr)
                            flags |= DRIVE_FLAGS_LOCKED;
                    }
                    if(flags & DRIVE_FLAGS_LOCKED)
                        vcbp->vcbAtrb |= CW(VHARDLOCKBIT);
                    if(!alreadythere)
                        Enqueue((QElemPtr)vcbp, &VCBQHdr);
                    pb->volumeParam.ioVRefNum = vcbp->vcbVRefNum;
                    if(!DefVCBPtr)
                    {
                        DefVCBPtr = RM(vcbp);
                        DefVRefNum = vcbp->vcbVRefNum;
                        DefDirID = CLC(2);
                    }
                }
            }
        }
    }
    TheZone = saveZone;
    warning_fs_log("err = %d", err);
    PBRETURN((VolumeParam *)pb, err);
}

static void goofyclip(GUEST<uint16_t> *up)
{
    if(CW(*up) > 0x7C00) /* IMIV-130 */
        *up = CWC(0x7C00);
}

/*
 * getworkingdir returns the directory id associated with vrefnum
 */

static LONGINT getworkingdir(INTEGER vrefnum)
{
    LONGINT retval;
    wdentry *wdp;

    if(ISWDNUM(vrefnum))
    {
        wdp = WDNUMTOWDP(vrefnum);
        retval = CL(wdp->dirid);
    }
    else
        retval = 0;
    return retval;
}

/*
 * getnmfls finds a directory's valence
 */

static unsigned short getnmfls(HVCB *vcbp, INTEGER workingdirnum)
{
    LONGINT dirid;
    catkey key;
    threadrec *thp;
    unsigned short retval;
    btparam btparamrec;
    OSErr err;

    dirid = getworkingdir(workingdirnum);
    err = ROMlib_makecatparam(&btparamrec, vcbp, dirid, 0, (Ptr)0);
    if(err == noErr)
        err = ROMlib_keyfind(&btparamrec);
    if(err == noErr && btparamrec.success)
    {
        thp = (threadrec *)DATAPFROMKEY(btparamrec.foundp);
        key.ckrParID = /*Cx*/ (thp->thdParID);
        str255assign(key.ckrCName, thp->thdCName);
        key.ckrKeyLen = sizeof(LONGINT) + 2 + key.ckrCName[0];
        err = ROMlib_keyfind(&btparamrec);
        // FIXME: #warning autc04: This does not seem right. Added .raw() here to preserve original executor behavior.
        // FIXME: #warning waitwat? "key" is never used again
        if(err == noErr && btparamrec.success)
            retval = ((directoryrec *)DATAPFROMKEY(btparamrec.foundp))->dirVal.raw();
        else
            retval = 0;
    }
    else
        retval = 0;
    return retval;
}

#define RETURN return
static OSErr commonGetVInfo(HVolumeParam *pb, BOOLEAN async, fstype fs)
{
    HVCB *vcbp;
    INTEGER workingdirnum;

    if(Cx(pb->ioVolIndex) > 0)
    {
        vcbp = (HVCB *)ROMlib_indexqueue(&VCBQHdr, Cx(pb->ioVolIndex));
        workingdirnum = 0;
    }
    else
    {
        if(pb->ioVolIndex == CWC(0))
            vcbp = (HVCB *)ROMlib_findvcb(Cx(pb->ioVRefNum), (StringPtr)0,
                                          (LONGINT *)0, false);
        else /* if (Cx(pb->ioVolIndex) < 0) */
            vcbp = (HVCB *)ROMlib_findvcb(Cx(pb->ioVRefNum), MR(pb->ioNamePtr),
                                          (LONGINT *)0, true);
        workingdirnum = getworkingdir(Cx(pb->ioVRefNum));
    }

    if(!vcbp)
        /*-->*/ PBRETURN(pb, nsvErr);

    if(/*CW (pb->ioVolIndex) >= 0 &&*/ pb->ioNamePtr)
        str255assign(MR(pb->ioNamePtr), (StringPtr)vcbp->vcbVN);
    pb->ioVCrDate = vcbp->vcbCrDate;
    pb->ioVAtrb = vcbp->vcbAtrb;

    if(workingdirnum)
        pb->ioVNmFls = CW(getnmfls(vcbp, workingdirnum));
    else
        pb->ioVNmFls = vcbp->vcbNmFls;

    pb->ioVNmAlBlks = vcbp->vcbNmAlBlks;
    pb->ioVAlBlkSiz = vcbp->vcbAlBlkSiz;
    pb->ioVClpSiz = vcbp->vcbClpSiz;
    pb->ioAlBlSt = vcbp->vcbAlBlSt;
    pb->ioVNxtCNID = vcbp->vcbNxtCNID;
    pb->ioVFrBlk = vcbp->vcbFreeBks;
    switch(fs)
    {
        case mfs:
            ((VolumeParam *)pb)->ioVLsBkUp = vcbp->vcbVolBkUp;
            ((VolumeParam *)pb)->ioVDirSt = 0;
            ((VolumeParam *)pb)->ioVBlLn = 0;
            if(!workingdirnum)
                pb->ioVRefNum = vcbp->vcbVRefNum;
            goofyclip((GUEST<uint16_t> *)&pb->ioVNmAlBlks);
            goofyclip((GUEST<uint16_t> *)&pb->ioVFrBlk);
            break;
        case hfs:
            pb->ioVLsMod = vcbp->vcbLsMod;
            pb->ioVBitMap = vcbp->vcbVBMSt;
#if !defined(THINKCMESSED)
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
            memmove(pb->ioVFndrInfo, vcbp->vcbFndrInfo,
                    (LONGINT)sizeof(pb->ioVFndrInfo));
            break;
    }
    PBRETURN(pb, noErr);
}
#undef RETURN

OSErr Executor::hfsPBGetVInfo(ParmBlkPtr pb, BOOLEAN async)
{
    return commonGetVInfo((HVolumeParam *)pb, async, mfs);
}

OSErr Executor::hfsPBHGetVInfo(HParmBlkPtr pb, BOOLEAN async)
{
    return commonGetVInfo((HVolumeParam *)pb, async, hfs);
}

#define ATRBMASK VSOFTLOCKBIT

OSErr Executor::hfsPBSetVInfo(HParmBlkPtr pb, BOOLEAN async)
{
    OSErr err;
    HVCB *vcbp;

    vcbp = ROMlib_findvcb(Cx(pb->volumeParam.ioVRefNum),
                          MR(pb->volumeParam.ioNamePtr), (LONGINT *)0, false);
    if(vcbp)
    {
        if(Cx(vcbp->vcbAtrb) & VHARDLOCKBIT)
            err = wPrErr;
        else
        {
            if(pb->volumeParam.ioNamePtr)
                str255assign((StringPtr)vcbp->vcbVN,
                             MR(pb->volumeParam.ioNamePtr));
            vcbp->vcbCrDate = pb->volumeParam.ioVCrDate;
            vcbp->vcbLsMod = pb->volumeParam.ioVLsMod;
            vcbp->vcbAtrb = CW((Cx(vcbp->vcbAtrb) & ~ATRBMASK) | (Cx(pb->volumeParam.ioVAtrb) & ATRBMASK));
            vcbp->vcbClpSiz = pb->volumeParam.ioVClpSiz;
            vcbp->vcbVolBkUp = pb->volumeParam.ioVBkUp;
            vcbp->vcbVSeqNum = pb->volumeParam.ioVSeqNum;
            memmove(vcbp->vcbFndrInfo, pb->volumeParam.ioVFndrInfo,
                    (LONGINT)32);
            vcbp->vcbFlags |= CW(VCBDIRTY);
            err = noErr;
        }
    }
    else
        err = nsvErr;
    PBRETURN((VolumeParam *)pb, err);
}

static OSErr getvolcommon(VolumeParam *pb)
{
    OSErr err;

    if(!DefVCBPtr)
        err = nsvErr;
    else
    {
        err = noErr;
        if(pb->ioNamePtr)
            str255assign(MR(pb->ioNamePtr), (StringPtr)MR(DefVCBPtr)->vcbVN);
        pb->ioVRefNum = DefVRefNum;
    }
    return err;
}

OSErr Executor::hfsPBGetVol(ParmBlkPtr pb, BOOLEAN async)
{
    OSErr err;

    err = getvolcommon((VolumeParam *)pb);
    PBRETURN((VolumeParam *)pb, err);
}

GUEST<LONGINT> Executor::DefDirID = CLC(2);

OSErr Executor::hfsPBHGetVol(WDPBPtr pb, BOOLEAN async)
{
    wdentry *wdp;
    OSErr err;

    err = getvolcommon((VolumeParam *)pb);
    pb->ioWDDirID = DefDirID;
    if(err == noErr)
    {
        if(ISWDNUM(Cx(DefVRefNum)))
        {
            wdp = WDNUMTOWDP(Cx(DefVRefNum));
            pb->ioWDProcID = wdp->procid;
            pb->ioWDVRefNum = MR(wdp->vcbp)->vcbVRefNum;
        }
        else
        {
            pb->ioWDProcID = 0;
            pb->ioWDVRefNum = DefVRefNum;
        }
    }
    PBRETURN(pb, err);
}

/*
 * NOTE: Considerable change related to PBSetVol, PBHSetVol were made
 *	 just now (Sat Aug  1 16:13:35 MDT 1992).  These routines
 *	 have been giving us trouble for some time.  Tech Note 140
 *	 implies that there is a "DefDirID" buried somewhere in low
 *	 global space.  Sometime we should try to ferret it out.
 *
 */

static OSErr setvolhelper(VolumeParam *pb, BOOLEAN aysnc, LONGINT dirid,
                          BOOLEAN convertzeros)
{
    HVCB *vcbp;
    GUEST<HVCB *> newDefVCBPtr;
    OSErr err, err1;
    LONGINT newdir;
    GUEST<LONGINT> newDefDirID;
    GUEST<INTEGER> newDefVRefNum;
    CInfoPBRec cpb;

    /*
 * CinemationCD hack ... they store a directory as a 2-byte quantity and
 *	sign extend it.  This will only help us recover the sign bit.
 */
    if(dirid < 0)
        dirid = 64 * 1024 + dirid;

    newdir = 0;
    vcbp = ROMlib_findvcb(Cx(pb->ioVRefNum), MR(pb->ioNamePtr),
                          &newdir, false);
    if(!vcbp)
        err = nsvErr;
    else
    {
        err = noErr;
        newDefVCBPtr = RM(vcbp);
        newDefDirID = CLC(0);
        if(newdir > 2)
        { /* picked up working directory */
            newDefDirID = dirid ? CL(dirid) : CL(newdir);
            newDefVRefNum = pb->ioVRefNum;
        }
        else if(newdir == 1)
        { /* picked up by name */
            newDefDirID = CL(newdir);
            newDefVRefNum = vcbp->vcbVRefNum;
        }
        else
        {
            newDefVRefNum = pb->ioVRefNum;
            if(dirid == 0 && convertzeros)
                newDefDirID = CLC(2);
            else
                newDefDirID = CL(dirid);
        }

        if(!convertzeros && pb->ioNamePtr)
        { /* this could change things */
            if(MR(pb->ioNamePtr)[0] == 0)
                cpb.hFileInfo.ioNamePtr = 0; /* otherwise we fill in */
            else
                cpb.hFileInfo.ioNamePtr = pb->ioNamePtr;
            cpb.hFileInfo.ioVRefNum = pb->ioVRefNum;
            cpb.hFileInfo.ioFDirIndex = CWC(0);
            cpb.hFileInfo.ioDirID = CL(dirid);
            /*
 * NOTE: the else case was added after seeing Excel 4 Installer do a setvol
 *	 to a file, presumably with the intent to set it to the parent id.
 *
 *	 Also we make this try twice as a PM PMSP
 */
            do
            {
                if((err1 = PBGetCatInfo(&cpb, false)) == noErr)
                {
                    if(cpb.hFileInfo.ioFlAttrib & ATTRIB_ISADIR)
                        newDefDirID = cpb.dirInfo.ioDrDirID;
                    else
                        newDefDirID = cpb.hFileInfo.ioFlParID;
                }
            } while(err1 && cpb.hFileInfo.ioDirID == CLC(0) && (cpb.hFileInfo.ioDirID = CLC(2)));
        }
        if(newDefDirID)
            DefDirID = newDefDirID;
        else if(DefVCBPtr != newDefVCBPtr || DefVRefNum != newDefVRefNum)
            DefDirID = CLC(2);
        DefVCBPtr = newDefVCBPtr;
        DefVRefNum = newDefVRefNum;
    }
    PBRETURN(pb, err);
}

OSErr Executor::hfsPBSetVol(ParmBlkPtr pb, BOOLEAN async)
{
    return setvolhelper((VolumeParam *)pb, async, 0, true);
}

OSErr Executor::hfsPBHSetVol(WDPBPtr pb, BOOLEAN async)
{
    return setvolhelper((VolumeParam *)pb, async, Cx(pb->ioWDDirID), false);
}

OSErr Executor::hfsPBFlushVol(ParmBlkPtr pb, BOOLEAN async)
{
    VCB *vcbp;
    OSErr err;

    vcbp = ROMlib_findvcb(Cx(pb->volumeParam.ioVRefNum),
                          MR(pb->volumeParam.ioNamePtr), (LONGINT *)0, true);
    if(vcbp)
        err = ROMlib_flushvcbp(vcbp);
    else
        err = nsvErr;
    PBRETURN((VolumeParam *)pb, err);
}

static void closeallvcbfiles(HVCB *vcbp)
{
    filecontrolblock *fcbp, *efcbp;
    IOParam iopb;
    short length;

    length = CW(*(GUEST<INTEGER> *)MR(FCBSPtr));
    fcbp = (filecontrolblock *)((short *)MR(FCBSPtr) + 1);
    efcbp = (filecontrolblock *)((char *)MR(FCBSPtr) + length);
    for(; fcbp < efcbp; fcbp = (filecontrolblock *)((char *)fcbp + Cx(FSFCBLen)))
        if(fcbp->fcbFlNum && MR(fcbp->fcbVPtr) == vcbp)
        {
            iopb.ioRefNum = CW((char *)fcbp - (char *)MR(FCBSPtr));
            /* my */ PBFlushFile((ParmBlkPtr)&iopb, false);
        }
}

OSErr Executor::hfsPBUnmountVol(ParmBlkPtr pb)
{
    OSErr err;
    HVCB *vcbp;

    vcbp = ROMlib_findvcb(Cx(pb->volumeParam.ioVRefNum),
                          MR(pb->volumeParam.ioNamePtr), (LONGINT *)0, false);
    if(vcbp)
    {
        closeallvcbfiles(vcbp);
        err = ROMlib_flushvcbp(vcbp);
        Dequeue((QElemPtr)vcbp, &VCBQHdr);
        DisposPtr(MR(vcbp->vcbMAdr));
        DisposPtr(MR(vcbp->vcbBufAdr));
        DisposPtr(MR(vcbp->vcbCtlBuf));
        DisposPtr((Ptr)vcbp);
    }
    else
        err = nsvErr;
    PBRETURN((VolumeParam *)pb, err);
}

static OSErr offlinehelper(VolumeParam *pb, HVCB *vcbp)
{
    OSErr err, err1, err2;
    IOParam iop;

    err = /* my */ PBFlushVol((ParmBlkPtr)pb, false);
    err1 = 0;
    err2 = 0;
    if(err == noErr)
    {
        if(vcbp)
        {
            iop.ioRefNum = vcbp->vcbXTRef;
            err1 = PBClose((ParmBlkPtr)&iop, false);
            iop.ioRefNum = vcbp->vcbCTRef;
            err2 = PBClose((ParmBlkPtr)&iop, false);
#if 1
            DisposPtr(MR(vcbp->vcbMAdr));
            vcbp->vcbMAdr = 0;
            DisposPtr(MR(vcbp->vcbBufAdr));
            vcbp->vcbBufAdr = 0;
            DisposPtr(MR(vcbp->vcbCtlBuf));
            vcbp->vcbCtlBuf = 0;
#endif
            vcbp->vcbDrvNum = 0;
            /* TODO:  look for offline flags in mpw equate files and set them */
        }
        else
            err = nsvErr;
    }
#if !defined(MAC)
#if 0
    if (err == noErr)
	err = updatefloppy();
#endif
#endif
    if(err == noErr)
        err = err1;
    if(err == noErr)
        err = err2;
    return err;
}

OSErr Executor::hfsPBOffLine(ParmBlkPtr pb)
{
    OSErr err;
    HVCB *vcbp;

    vcbp = ROMlib_findvcb(Cx(pb->volumeParam.ioVRefNum),
                          MR(pb->volumeParam.ioNamePtr), (LONGINT *)0, false);
    if(vcbp)
    {
        if(vcbp->vcbDrvNum)
        {
            vcbp->vcbDRefNum = CW(-Cx(vcbp->vcbDrvNum));
            err = offlinehelper((VolumeParam *)pb, vcbp);
        }
        else
            err = noErr;
    }
    else
        err = nsvErr;
    PBRETURN((VolumeParam *)pb, err);
}

OSErr Executor::hfsPBEject(ParmBlkPtr pb)
{
    OSErr err;
    HVCB *vcbp;
    INTEGER vref;

    vref = Cx(pb->volumeParam.ioVRefNum);
    vcbp = ROMlib_findvcb(vref,
                          MR(pb->volumeParam.ioNamePtr), (LONGINT *)0, false);
    if(vcbp)
    {
        if(Cx(vcbp->vcbDrvNum))
        {
            vcbp->vcbDRefNum = vcbp->vcbDrvNum;
            err = offlinehelper((VolumeParam *)pb, vcbp);
        }
        else
        {
            if(Cx(vcbp->vcbDRefNum) < 0) /* offline */
                vcbp->vcbDRefNum = CW(Cx(vcbp->vcbDRefNum) * -1);
            err = noErr;
        }
    }
    else
    {
        if(vref == 1 || vref == 2)
            err = noErr; /* They're explicitly ejecting a particular drive */
        else
            err = nsvErr;
    }
#if !defined(MAC)
    if(err == noErr)
        err = ROMlib_ejectfloppy(vcbp ? ((VCBExtra *)vcbp)->u.hfs.fd : -1);
#endif
    PBRETURN((VolumeParam *)pb, err);
}

OSErr Executor::ROMlib_pbvolrename(IOParam *pb, StringPtr newnamep)
{
    OSErr err;
    HParamBlockRec hpb;
    Str255 name_copy;

    str255assign(name_copy, MR(pb->ioNamePtr));
    hpb.volumeParam.ioNamePtr = RM(name_copy);
    hpb.volumeParam.ioVRefNum = pb->ioVRefNum;
    hpb.volumeParam.ioVolIndex = CWC(-1);
    err = /* my */ PBHGetVInfo((HParmBlkPtr)&hpb, false);
    if(err == noErr)
    {
        hpb.volumeParam.ioNamePtr = RM(newnamep);
        err = /* my */ PBSetVInfo((HParmBlkPtr)&hpb, false);
    }
    return err;
}

/* Copyright 1992 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "SegmentLdr.h"
#include "ToolboxEvent.h"
#include "MemoryMgr.h"
#include "SysErr.h"
#include "DialogMgr.h"
#include "ResourceMgr.h"

#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/partition.h"

#if defined(MSDOS) || defined(CYGWIN32)
#include "dosdisk.h"
#include "aspi.h"
#elif defined(WIN32)
// ### TODO: new win32 OS code does not yet
// include the direct disk access stuff
#else
#include <unistd.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#if defined(MACOSX_)
#include <sys/disk.h>
#endif
#endif

using namespace Executor;

#if !defined(MAC)

#define FLOPPYWORKSFS "/usr/filesystems/macintosh.fs"
#define NEWFLOPPYWORKSFS "/usr/filesystems/macintosh.fs.SAVE"

#define MAC30FS "/usr/filesystems/mac.fs"
#define NEWMAC30FS "/usr/filesystems/mac.fs.SAVE"

#define OURSOCK "/dev/HFS_XFer"

#define HFSXFERDOTFS "/usr/filesystems/HFS_XFer.fs"
#define HFSXFERUTIL HFSXFERDOTFS "/HFS_XFer.util"

void Executor::ROMlib_hfsinit(void)
{
}

/*
 * NOTE: The weird name constructed below is inherited from HFS_XFer.util.c,
 *	 which is an ARDI written NEXTSTEP atrocity.
 */

#if !defined(LINUX) && !defined(MACOSX)
#define EJECTABLE(buf) false
#else
/* #warning this is not the proper way to tell if something is ejectable */
#define EJECTABLE(buf) (buf[strlen(buf) - 3] == 'f' && buf[strlen(buf) - 2] == 'd')
#endif

#define ASSIGN_NAME_MODE_STRING(buf, lenp, filename, sbuf)                       \
    do                                                                           \
    {                                                                            \
        char ejectable;                                                          \
                                                                                 \
        ejectable = EJECTABLE(filename);                                         \
        *(lenp) = strlen((filename)) + 1 + 6 + 1 + 1 + 1;                        \
        (buf) = (char *)alloca(*(lenp));                                         \
        sprintf((buf), "%s%c%06o%c%c", (filename), 0, (sbuf).st_mode & 07777, 0, \
                ejectable);                                                      \
    } while(0)

#define NRETRIES 5

long
Executor::ROMlib_priv_open(const char *filename, long mode)
{
    long retval;

    retval = Uopen(filename, mode, 0);
    if(retval < 0)
        retval = ROMlib_maperrno();

    return retval;
}

static void eject_floppy_notify(void)
{

    /*
 * We make the test for 'ALRT' because when Executor shuts down, an ejectable
 * volume may be unmounted after the volume with the System file has already
 * been unmounted.  In the past, this caused a crash.  It may make sense to
 * deliberately unmount last the volume containing System, but that's a fix
 * for another day.
 */

    if(LM(WWExist) == EXIST_YES && GetResource(TICK("ALRT"), EJECTALERTID))
        Alert(EJECTALERTID, (ProcPtr)0);
}

OSErr Executor::ROMlib_ejectfloppy(LONGINT floppyfd)
{
    OSErr err;

    err = noErr;
#if defined(MSDOS) || defined(CYGWIN32)
    if(floppyfd == -1 || (floppyfd & (DOSFDBIT | ASPIFDBIT)))
    {
        if(floppyfd != -1)
        {
            if(floppyfd & DOSFDBIT)
            {
                floppyfd &= ~DOSFDBIT;
                dosdisk_close(floppyfd, true);
            }
        }
        eject_floppy_notify();
    }
    else
    {
#endif
#if defined(MACOSX_)
        if(floppyfd != -1 && ioctl(floppyfd, DKIOCEJECT, (char *)0) < 0)
        {
            fprintf(stderr, "couldn't eject disk\n");
            err = ioErr;
        }
#endif
        if(floppyfd != -1)
            close(floppyfd);
#if defined(LINUX) || defined(MACOSX_)
        eject_floppy_notify();
#endif
#if defined(MSDOS) || defined(CYGWIN32)
    }
#endif
    return err;
}

void Executor::ROMlib_OurClose(void)
{
    HVCB *vcbp, *next;
    ParamBlockRec pbr;

    for(vcbp = (HVCB *)MR(LM(VCBQHdr).qHead); vcbp; vcbp = next)
    {
        next = (HVCB *)MR(vcbp->qLink);
        pbr.ioParam.ioNamePtr = 0;
        if(Cx(vcbp->vcbCTRef))
        {
            pbr.ioParam.ioVRefNum = vcbp->vcbVRefNum;
            PBUnmountVol(&pbr);
#if defined(MACOSX_) || defined(MACOSX_)
            if(!(Cx(vcbp->vcbAtrb) & VNONEJECTABLEBIT) && Cx(vcbp->vcbDrvNum))
                ROMlib_ejectfloppy(((VCBExtra *)vcbp)->u.hfs.fd);
#endif
        }
        else
            ROMlib_dbm_close((VCBExtra *)vcbp);
    }
}

#if 0
static BOOLEAN isejectable(const charCx( *dname), LONGINT fd)
{
    BOOLEAN retval;
#if defined(MACOSX_) || defined(MACOSX_)
    struct scsi_req sr;
    char inqbuf[sizeof(struct inquiry_reply) + 3];
    struct inquiry_replyCx( *inqp);
    const charCx( *p);
#endif

    /* look for rfd[0-9] */
    retval = false;
#if defined(MACOSX_) || defined(MACOSX)
    for (p = dname; p = index(p, 'r'); ++p) {
	if (p[1] == 'f' && p[2] == 'd' && isdigit(p[3])) {
	    retval = true;
/*-->*/	    break;
	}
    }
    if (!retval) {
	inqp = (struct inquiry_reply *) (((LONGINT) inqbuf + 3) / 4Cx( * 4));
	bzero (&sr, sizeof(sr));
	sr.sr_cdb.cdb_c6.c6_opcode = C6OP_INQUIRY;
	sr.sr_cdb.cdb_c6.c6_len	   = sizeofCx((*inqp));
	sr.sr_dma_dir	           = SR_DMA_RD;
	sr.sr_addr	           = (caddr_t) inqp;
	sr.sr_dma_max	           = sr.sr_cdb.cdb_c6.c6_len;
	sr.sr_ioto	           = 1;
	if (ioctl(fd, SGIOCREQ, &sr) == 0 && sr.sr_io_status == 0 &&
							    Cx(inqp->ir_removable))
	    retval = true;
    }
#endif
    return retval;
}
#endif

/*
 * NOTE: messp below points to the longint that will be filled in with
 *	 the status of the first disk insert event that we create, but
 *	 because we can have multiple drives mounted on one physical
 *	 drive, we wind put using PostEvent for all but the first.
 *
 *	 The above method sounds pretty hacky to me, and should probably
 *	 be replaced once we find a cheaper way to be notified that
 *	 a disk can be read (probably use a signal on the pipe).
 */

static LONGINT try_to_open_disk(const char *dname, LONGINT *bsizep,
                                LONGINT *maxbytesp, drive_flags_t *flagsp,
                                uint32_t *offsetp)
{
    LONGINT floppyfd;
    int len;

    *flagsp = 0;
    len = strlen(dname) + 1; /* first component: name */
    len += strlen(dname + len) + 1; /* second component: permission */
    if(!dname[len])
        *flagsp |= DRIVE_FLAGS_FIXED; /* third component: ejectable */

#if !defined(CYGWIN32)
#define EXTRA_BITS O_EXCL
#else
#define EXTRA_BITS 0
#endif

    if((floppyfd = Uopen(dname, O_BINARY | O_RDWR | EXTRA_BITS, 0000)) < 0 && (*flagsp |= DRIVE_FLAGS_LOCKED,
                                                                               (floppyfd = Uopen(dname, O_BINARY | O_RDONLY | EXTRA_BITS,
                                                                                                 0000))
                                                                                   < 0))
        /* fprintf(stderr, "can't open %s\n", dname) */;
    else
    {
        *bsizep = PHYSBSIZE;
        *maxbytesp = 1024L * 1024;
    }

    if(floppyfd >= 0)
    {
        struct stat sbuf;

        if(fstat(floppyfd, &sbuf) >= 0 && (S_IFREG & sbuf.st_mode))
            *offsetp = sbuf.st_size % PHYSBSIZE;
        else
            *offsetp = 0;
    }

    return floppyfd;
}

static LONGINT
read_driver_block_size(LONGINT fd, LONGINT bsize, LONGINT maxbytes,
                       char aligned_buf[])
{
    LONGINT retval;

    retval = PHYSBSIZE;
    if(ROMlib_readwrite(fd, aligned_buf, PHYSBSIZE, 0, reading, bsize,
                        maxbytes)
       == noErr)
    {
        if(aligned_buf[0] == 0x45 && aligned_buf[1] == 0x52)
        {
            retval = (unsigned short)CW(*(GUEST<uint16_t> *)&aligned_buf[2]);
            warning_fs_log("fd = 0x%x, block size = %d", fd, retval);
        }
    }
    return retval;
}

void
Executor::try_to_mount_disk(const char *dname, LONGINT floppyfd, GUEST<LONGINT> *messp,
                            LONGINT bsize, LONGINT maxbytes, drive_flags_t flags,
                            uint32_t offset_in)
{
    INTEGER partition;
    ParamBlockRec pb;
    char *tbuf, *buf;
    INTEGER drivenum;
    LONGINT offset;
    BOOLEAN foundmap, first;
    oldblock1_t *oldmapp;
    partmapentry_t *partp;
    int i;
    LONGINT mess;
    OSErr err;
    DrvQExtra *dqp;
    hfs_access_t hfs;
    LONGINT driver_block_size;

    *messp = 0;
    tbuf = (char *)alloca(bsize + 3);
    buf = (char *)(((uintptr_t)tbuf + 3) & ~3);
    partition = 0;

    if(floppyfd >= 0)
        driver_block_size = read_driver_block_size(floppyfd, bsize, maxbytes,
                                                   buf);
    else
        driver_block_size = PHYSBSIZE;

    hfs.fd = (flags & DRIVE_FLAGS_FLOPPY) ? floppyfd : -1;
    hfs.offset = offset_in;
    hfs.bsize = bsize;
    hfs.maxbytes = maxbytes;

    dqp = ROMlib_addtodq(2048L * 2, dname, partition, OURHFSDREF, flags,
                         &hfs);

    if(floppyfd < 0)
        /*-->*/ return;

    drivenum = CW(dqp->dq.dQDrive);

    pb.ioParam.ioVRefNum = CW(drivenum);

    foundmap = false;
    first = true;
    offset = hfs.offset + PARTOFFSET * driver_block_size;
    if(ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset, reading,
                        bsize, maxbytes)
       == noErr)
    {
        if(buf[0] == PARMAPSIG0 && buf[1] == PARMAPSIG1)
        {
            warning_fs_log("found partition sig at %d, floppyfd = 0x%x",
                           offset, floppyfd);
            partp = (partmapentry_t *)buf;
            do
            {
                if(strncmp((char *)partp->pmPartType, HFSPARTTYPE, 32)
                   == 0)
                {
                    foundmap = true;
                    if(!first)
                    {
                        ++partition;
                        dqp = ROMlib_addtodq(2048L * 2, dname,
                                             partition, OURHFSDREF, flags,
                                             &hfs);
                        drivenum = CW(dqp->dq.dQDrive);
                        pb.ioParam.ioVRefNum = CW(drivenum);
                    }
                    dqp->hfs.offset = hfs.offset + (CL(partp->pmPyPartStart)
                                                    * driver_block_size);
                    err = hfsPBMountVol(&pb, floppyfd, dqp->hfs.offset, bsize,
                                        maxbytes, flags, dqp);
                    mess = ((LONGINT)err << 16) | drivenum;
                    if(first)
                    {
                        *messp = CL(mess);
                        first = false;
                    }
                    else
                        PPostEvent(diskEvt, mess, (GUEST<EvQElPtr> *)0);
                }
                offset += driver_block_size;
            } while(ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset,
                                     reading, bsize, maxbytes)
                        == noErr
                    && buf[0] == PARMAPSIG0 && buf[1] == PARMAPSIG1);
        }
        else if(buf[0] == OLDMAPSIG0 && buf[1] == OLDMAPSIG1)
        {
            oldmapp = (oldblock1_t *)buf;
            for(i = 0; i < NOLDENTRIES && (Cx(oldmapp->oldmapentry[i].pdStart) != 0 || Cx(oldmapp->oldmapentry[i].pdSize) != 0 || Cx(oldmapp->oldmapentry[i].pdFSID) != 0); ++i)
            {
                /*
* NOTE: We initially tried looking for 'TFS1' in pdFSID, but our Cirrus 80
*	 didn't use that id.
*/
                if(!first)
                {
                    ++partition;
                    dqp = ROMlib_addtodq(2048L * 2, dname, partition,
                                         OURHFSDREF, flags, &hfs);
                    drivenum = CW(dqp->dq.dQDrive);
                    pb.ioParam.ioVRefNum = CW(drivenum);
                }
                dqp->hfs.offset = hfs.offset + (CL(oldmapp->oldmapentry[i].pdStart)
                                                * driver_block_size);
                err = hfsPBMountVol(&pb, floppyfd, dqp->hfs.offset,
                                    bsize, maxbytes, flags, dqp);
                mess = ((LONGINT)err << 16) | drivenum;
                if(first)
                {
                    *messp = CL(mess);
                    first = false;
                }
                else
                    PPostEvent(diskEvt, mess, (GUEST<EvQElPtr> *)0);
            }
            foundmap = true;
        }
    }

    if(!foundmap)
    {
        for(offset = hfs.offset + VOLUMEINFOBLOCKNO * PHYSBSIZE, i = 4;
            --i >= 0; offset += PHYSBSIZE)
        {
            if(i == 0)
            {
                if(ROMlib_magic_offset == -1)
                    /*-->*/ continue;
                else
                    offset = ROMlib_magic_offset;
            }
            err = ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset, reading,
                                   bsize, maxbytes);
            if(err != noErr)
            {
                warning_unexpected("fd = 0x%x err = %d, offset = %d, "
                                   "bsize = %d, maxbytes = %d",
                                   floppyfd,
                                   err, offset, bsize, maxbytes);
                /*-->*/ break;
            }
            if(buf[0] == 'B' && buf[1] == 'D')
            {
                warning_fs_log("Found HFS volume on 0x%x %d", floppyfd,
                               offset);
                offset -= VOLUMEINFOBLOCKNO * PHYSBSIZE;
                foundmap = true;
                /*-->*/ break;
            }
            else if(buf[0] == 'H' && buf[1] == '+')
            {
                warning_fs_log("Found HFS+ volume on 0x%x %d", floppyfd,
                               offset);
            }
            else
            {
                warning_fs_log("fd = 0x%x, offset = %d, sig = 0x%02x%02x",
                               floppyfd, offset, buf[0], buf[1]);
            }
        }
        if(foundmap)
        {
            dqp->hfs.offset = offset;
            err = hfsPBMountVol(&pb, floppyfd, offset, bsize, maxbytes,
                                flags, dqp);
            *messp = CL(((LONGINT)err << 16) | drivenum);
        }
    }
}

void Executor::ROMlib_openfloppy(const char *dname, GUEST<LONGINT> *messp)
{
    LONGINT floppyfd;
    LONGINT bsize, maxbytes;
    drive_flags_t flags;
    uint32_t offset;

    *messp = 0;
    floppyfd = try_to_open_disk(dname, &bsize, &maxbytes, &flags, &offset);
    if(floppyfd >= 0)
        try_to_mount_disk(dname, floppyfd, messp, bsize, maxbytes, flags,
                          offset);
}

void Executor::ROMlib_openharddisk(const char *dname, GUEST<LONGINT> *messp)
{
    char *newbuf;
    long len;
    struct stat sbuf;

    *messp = 0;
    if(Ustat(dname, &sbuf) == 0)
    {
        ASSIGN_NAME_MODE_STRING(newbuf, &len, dname, sbuf);
        ROMlib_openfloppy(newbuf, messp);
    }
}

#endif

#define JUMPTODONEIF(x) \
    if((x))             \
    {                   \
        err = ioErr;    \
        goto DONE;      \
    }

OSErr Executor::ROMlib_readwrite(LONGINT fd, char *buffer, LONGINT count,
                                 LONGINT offset, accesstype rw,
                                 LONGINT blocksize, LONGINT maxtransfer)
{
    char *newbuffer;
    LONGINT remainder, totransfer;
    BOOLEAN needlseek;
    OSErr err;
    int (*readfp)(int fd, void *buf, int nbytes);
    int (*writefp)(int fd, const void *buf, int nbytes);
    off_t (*seekfp)(int fd, off_t where, int how);

    if(blocksize > 18 * 1024)
    {
        warning_unexpected("fd = 0x%x, block size = %d", fd, blocksize);
        return fsDSIntErr;
    }

    if(blocksize == 0)
    {
        blocksize = 2048;
        warning_unexpected("fd = 0x%x, zero block size", fd);
    }
#if defined(MSDOS) || defined(CYGWIN32)
    if(fd & DOSFDBIT)
    {
        fd &= ~DOSFDBIT;
        seekfp = dosdisk_seek;
        readfp = dosdisk_read;
        writefp = dosdisk_write;
    }
    else
    {
#endif
        seekfp = (off_t(*)(int, off_t, int))lseek;
        readfp = (int (*)(int, void *, int))read;
        writefp = (int (*)(int, const void *, int))write;
#if defined(MSDOS) || defined(CYGWIN32)
    }
#endif
    err = noErr;
    newbuffer = 0;
    needlseek = true;
    if((remainder = offset % blocksize))
    { /* |xxxDATA| */
        remainder = blocksize - remainder;
        totransfer = MIN(count, remainder);
        newbuffer = (char *)(((uintptr_t)alloca(blocksize + 3) + 3) & ~3);
        offset = offset / blocksize * blocksize;
        JUMPTODONEIF(seekfp(fd, offset, SEEK_SET) < 0)
        needlseek = false;
        JUMPTODONEIF(readfp(fd, newbuffer, blocksize) != blocksize)
        if(rw == reading)
        {
            memmove(buffer, newbuffer + blocksize - remainder, totransfer);
        }
        else
        {
            memmove(newbuffer + blocksize - remainder, buffer, totransfer);
            JUMPTODONEIF(seekfp(fd, offset, SEEK_SET) < 0)
            JUMPTODONEIF(writefp(fd, newbuffer, blocksize) != blocksize)
        }
        buffer += totransfer;
        count -= totransfer;
        offset += blocksize;
    }
    if(count >= blocksize)
    { /* |DATADATADATA...| */
        remainder = count % blocksize;
        count -= remainder;
        if(needlseek)
        {
            JUMPTODONEIF(seekfp(fd, offset, SEEK_SET) < 0)
            needlseek = false;
        }
        while(count)
        {
            totransfer = MIN(maxtransfer, count);
            if(rw == reading)
            {
                JUMPTODONEIF(readfp(fd, buffer, totransfer) != totransfer)
            }
            else
            {
                JUMPTODONEIF(writefp(fd, buffer, totransfer) != totransfer)
            }
            buffer += totransfer;
            count -= totransfer;
            offset += totransfer;
        }
        count = remainder;
    }
    if(count)
    { /* |DATAxxx| */
        if(!newbuffer)
            newbuffer = (char *)(((uintptr_t)alloca(blocksize + 3) + 3) & ~3);
        if(needlseek)
            JUMPTODONEIF(seekfp(fd, offset, SEEK_SET) < 0)
        JUMPTODONEIF(readfp(fd, newbuffer, blocksize) != blocksize)
        if(rw == reading)
        {
            memmove(buffer, newbuffer, count);
        }
        else
        {
            memmove(newbuffer, buffer, count);
            JUMPTODONEIF(seekfp(fd, offset, SEEK_SET) < 0)
            JUMPTODONEIF(writefp(fd, newbuffer, blocksize) != blocksize)
        }
    }
DONE:
    return err;
}

OSErr
Executor::ROMlib_transphysblk(hfs_access_t *hfsp, LONGINT physblock, short nphysblocks,
                              Ptr bufp, accesstype rw, GUEST<LONGINT> *actp)
{
    LONGINT fd;
    OSErr err;
    Ptr newbufp;

#if defined(MAC)
    ioParam pb;

    pb.ioVRefNum = vcbp->vcbDrvNum;
    pb.ioRefNum = vcbp->vcbDRefNum;
    pb.ioBuffer = CL(bufp);
    pb.ioReqCount = CL(PHYSBSIZE * (LONGINT nphysblocks));
    pb.ioPosMode = CW(fsFromStart);
    pb.ioPosOffset = CL(physblock);
    err = rw == reading ? PBRead((ParmBlkPtr)&pb, false) : PBWrite((ParmBlkPtr)&pb, false);
    if(actp)
        *actp = pb.ioActCount;
#else
#if 0 && (defined(NEXTSTEP) || defined(MACOSX))
    if ((LONGINT) bufp & 3) {
        newbufp = alloca( (LONGINT) nphysblocks * PHYSBSIZE + 4);
        newbufp = (Ptr) (((LONGINT) newbufp + 3) & ~3);
        if (rw == writing)
            memmove(newbufp, bufp, (LONGINT) nphysblocks * PHYSBSIZE);
    } else
#endif
    newbufp = bufp;
    fd = hfsp->fd;

    err = ROMlib_readwrite(fd, (char *)newbufp,
                           (LONGINT)nphysblocks * PHYSBSIZE,
                           physblock + hfsp->offset, rw, hfsp->bsize,
                           hfsp->maxbytes);
#if 0 && (defined(NEXTSTEP) || defined(MACOSX))
    if (rw == reading && bufp != newbufp && err == noErr)
        memmove(bufp, newbufp, (LONGINT) nphysblocks * PHYSBSIZE);
#endif
    if(actp)
        *actp = err != noErr ? CLC(0) : CL((LONGINT)nphysblocks * PHYSBSIZE);

#endif
    if(err != noErr)
        warning_unexpected("fd = 0x%x, err in transphysblock (err = %d)",
                           fd, err);
    return err;
}

char *Executor::ROMlib_indexn(char *str, char tofind, INTEGER length)
{
    while(--length >= 0)
        if(*str++ == tofind)
            return str - 1;
    return 0;
}

#if !defined(str255assign)
void Executor::str255assign(StringPtr dstp, StringPtr srcp)
{
    memmove(dstp, srcp, (size_t)srcp[0] + 1);
}
#endif /* !defined(str255assign) */

/*
 * ROMlib_indexqueue returns a pointer to the n'th entry on a queue.
 * ROMlib_indexqueue is one based; not zero based.
 */

void *Executor::ROMlib_indexqueue(QHdr *qp, short index)
{
    QElemPtr p;

    for(p = MR(qp->qHead); (--index > 0) && p; p = MR(p->vcbQElem.qLink))
        ;
    return p;
}

OSErr Executor::ROMlib_writefcbp(filecontrolblock *fcbp)
{
    Byte flags;
    OSErr retval;

    flags = fcbp->fcbMdRByt;
    if(!(flags & WRITEBIT))
        retval = wrPermErr;
    else if(flags & FLOCKEDBIT)
        retval = fLckdErr;
    else
        retval = noErr;
    return retval;
}

OSErr Executor::ROMlib_writevcbp(HVCB *vcbp)
{
    INTEGER vflags;
    OSErr retval;

    vflags = Cx(vcbp->vcbAtrb);
    if(vflags & VSOFTLOCKBIT)
        retval = vLckdErr;
    else if(vflags & VHARDLOCKBIT)
        retval = wPrErr;
    else
        retval = noErr;
    return retval;
}

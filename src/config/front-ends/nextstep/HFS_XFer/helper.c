#if defined(OUTDATEDCODE)
/*
 * TODO: better support for read-only floppies
 */

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "myhfs.h"
#include <string.h>
#include "SegmentLdr.h"
#include "xbar.h"
#include "ToolboxEvent.h"

#if defined(UNIX)

#include <sys/file.h>
#include <nextdev/disk.h>
#include <nextdev/fd_extern.h>
#include "fs.h"
#include <assert.h>

#define BIGBLOCK	(20L * 1024)
#define NBIGBLOCKS	144		/* enough for an ED disk */

static char buf[NBIGBLOCKS * BIGBLOCK];
static char readmap[NBIGBLOCKS];
static char writemap[NBIGBLOCKS];
LONGINT floppyfd = -1;	/* should be static */

PUBLIC OSErr updatefloppy( void )
{
    OSErr err;
    INTEGER i;
    LONGINT offset;

    err = noErr;
    for (i = 0; i < NBIGBLOCKS; ++i) {
	if (writemap[i]) {
	    offset = i * BIGBLOCK;
	    if (lseek(floppyfd, offset, L_SET) < 0 ||
			     write(floppyfd, buf+offset, BIGBLOCK) != BIGBLOCK)
		err = ioErr;
	}
    }
    return err;
}

PUBLIC OSErr ejectfloppy( void )
{
    OSErr err;

    err = noErr;
    if (floppyfd != -1 && ioctl(floppyfd, DKIOCEJECT, (char *) 0) < 0)  {
	fprintf(stderr, "couldn't eject disk\n");
	err = ioErr;
    } else {
	close(floppyfd);
	floppyfd = -1;
    }
    return err;
}

PRIVATE void OurClose( void )
{
    ParamBlockRec pb;

    pb.ioParam.ioVRefNum = 5;
    pb.ioParam.ioNamePtr = 0;
    myPBUnmountVol((volumeParam *) &pb);
    updatefloppy();
    ejectfloppy();
}

PUBLIC void openfloppy( const char *dname )
{
    ParamBlockRec pb;
    extern long pipefd[2];
    struct fd_ioreq req;
    LONGINT dir;
    HVCB *vcbp;
    BOOLEAN writelocked;
    char savec;
    OSErr myPBMountVol(volumeParam *pb);
    extern void (*ROMlib_slimyclosehack)(void);

    bzero(readmap,  (size_t) sizeof(readmap));
    bzero(writemap, (size_t) sizeof(writemap));
    if ((floppyfd = open(dname, O_RDWR|O_EXCL)) < 0) {
	if (pipefd[1] != -1)
	    write(pipefd[1], dname, strlen(dname)+1);
	fprintf(stderr, "can't open %s\n", dname);
    } else {
	if (pipefd[1] != -1)
	    write(pipefd[1], dname, strlen(dname)+1);
	pb.ioParam.ioVRefNum = 5;
	myPBMountVol((volumeParam *) &pb);
	writelocked = false;
	if (dname[6] == 'f') {	/* counting on /dev/rfd?b */
	    bzero(&req, sizeof(req));
	    req.command = FDCMD_GET_STATUS;
	    req.drive_stat.write_prot = 0;
	    if (ioctl(floppyfd, FDIOCREQ, &req) < 0)
		printf("IOCREQ failed %d\n", errno);
	    if (req.drive_stat.write_prot)
		writelocked = true;
	} else {
/*
 * NOTE: the bullshit below is because our DIT floppy pretends it can
 *	 write
 */
	    if (lseek(floppyfd, 0, L_SET) < 0)
		fprintf(stderr, "lseek errno %d\n", errno);
	    if (read(floppyfd, buf, 1024) != 1024)
		fprintf(stderr, "read errno %d\n", errno);
	    if (lseek(floppyfd, 0, L_SET) < 0)
		fprintf(stderr, "lseek errno %d\n", errno);
	    savec = buf[1023];
	    buf[1023] ^= 0xFF;
	    if (write(floppyfd, buf, 1024) != 1024)
		writelocked = true;
	    if (lseek(floppyfd, 0, L_SET) < 0)
		fprintf(stderr, "lseek errno %d\n", errno);
	    if (read(floppyfd, buf, 1024) != 1024)
		fprintf(stderr, "read errno %d\n", errno);
	    if (buf[1023] == savec)
		writelocked = true;
	    else {
		if (lseek(floppyfd, 0, L_SET) < 0)
		    fprintf(stderr, "lseek errno %d\n", errno);
		buf[1023] = savec;
		if (write(floppyfd, buf, 1024) != 1024)
		    writelocked = true;
	    }
	}
	if (writelocked) {
	    vcbp = findvcb(pb.ioParam.ioVRefNum, (StringPtr) 0, &dir);
	    if (vcbp)
		vcbp->vcbAtrb |= VHARDLOCKBIT;
	}
    }
    ROMlib_slimyclosehack = OurClose;
}

BOOLEAN xGetNextEvent( INTEGER em, EventRecord *evtp )
{
    BOOLEAN retval;
    struct sockaddr sockname;
    long addrlen;
    char device[DEVNAMELEN];
    long ns, nread;
    extern long sock;

    retval = false;
    if ((em & diskMask) && floppyfd == -1) {
	addrlen = sizeof(sockname);
	if ((ns = accept(sock, &sockname, (int *) &addrlen)) >= 0) {
	    fcntl(ns, F_SETFL, 0);	/* turn off FNDELAY */
	    nread = read(ns, device, sizeof(device));
	    openfloppy(device);
	    evtp->what = diskEvt;
	    retval = true;
	} else
	    assert(errno == EWOULDBLOCK);
    }
    return retval ? true : GetNextEvent(em, evtp);
}

BOOLEAN xWaitNextEvent( INTEGER em, EventRecord *evtp, LONGINT sleep,
							   RgnHandle mousergn )
{
    BOOLEAN retval;
    struct sockaddr sockname;
    long addrlen;
    char device[DEVNAMELEN];
    long ns, nread;
    extern long sock;

    retval = false;
    if ((em & diskMask) && floppyfd == -1) {
	addrlen = sizeof(sockname);
	if ((ns = accept(sock, &sockname, (int *) &addrlen)) >= 0) {
	    fcntl(ns, F_SETFL, 0);	/* turn off FNDELAY */
	    nread = read(ns, device, sizeof(device));
	    openfloppy(device);
	    evtp->what = diskEvt;
	    retval = true;
	} else
	    assert(errno == EWOULDBLOCK);
    }
    return retval ? true : WaitNextEvent(em, evtp, sleep, mousergn);
}

#endif

PUBLIC void OurExit( void )
{
#if defined(UNIX)
    OurClose();
#endif
    ExitToShell();
}

PUBLIC OSErr TransPhysBlk(HVCB *vcbp, long physblock, short nphysblocks,
					    Ptr bufp, accesstype rw, long *actp)
{
#if !defined(UNIX)
    ioParam pb;
#else
    LONGINT firstbigblock, lastbigblock, offset, ntocopy;
    INTEGER i;
#endif
    OSErr err;
    
#if 1 && !defined(UNIX)
    if (rw != reading && strncmp((char *) vcbp->vcbVN, "\pMyVol",
						      vcbp->vcbVN[0]+1) != 0) {
	errormessage((StringPtr) "\pAck! not Myvol.", CAUTION);
	printf("Dangerous chemicals, Timmy! (we should stick to MyVol)\n");
	exit(1);
    }
#endif
#if !defined(UNIX)
    pb.ioVRefNum = vcbp->vcbDrvNum;
    pb.ioRefNum = vcbp->vcbDRefNum;
    pb.ioBuffer = bufp;
    pb.ioReqCount = PHYSBSIZE * nphysblocks;
    pb.ioPosMode = fsFromStart;
    pb.ioPosOffset = physblock;
    err = rw == reading ? PBRead ((ParmBlkPtr) &pb, false) :
			  PBWrite((ParmBlkPtr) &pb, false);
    if (actp)
	*actp = pb.ioActCount;
#else
    firstbigblock = physblock / BIGBLOCK;
    lastbigblock = (physblock + PHYSBSIZE * nphysblocks - 1) / BIGBLOCK;
    err = noErr;
    if (firstbigblock >= 0 || lastbigblock < NBIGBLOCKS) {
	for (i = firstbigblock; i <= lastbigblock; ++i) {
	    if (!readmap[i]) {
		offset = i * BIGBLOCK;
		if (lseek(floppyfd, offset, L_SET) < 0 ||
			    read(floppyfd, buf+offset, BIGBLOCK) != BIGBLOCK) {
		    err = ioErr;
		    break;
		}
		readmap[i] = 1;
	    }
	    if (rw == writing)
		writemap[i] = 1;
	}
    } else
	err = paramErr;
    ntocopy = nphysblocks * PHYSBSIZE;
    if (rw == reading)
	memcpy(bufp, buf+physblock, ntocopy);
    else
	memcpy(buf+physblock, bufp, ntocopy);
    if (actp)
	*actp = ntocopy;
#endif
#if 1
    if (err != noErr)
        DebugStr((StringPtr)"\perr in transphysblock");
#endif
    return err;
}

PUBLIC char *indexn(char *str, char tofind, INTEGER length)
{
    while (--length >= 0)
	if (*str++ == tofind)
	    return str - 1;
    return 0;
}

#if !defined(str255assign)
PUBLIC void str255assign(StringPtr dstp, StringPtr srcp)
{
    memcpy(dstp, srcp, (size_t) srcp[0]+1);
}
#endif /* !defined(str255assign) */

/*
 * indexqueue returns a pointer to the n'th entry on a queue.
 * indexqueue is one based; not zero based.
 */
 
PUBLIC void *indexqueue(QHdr *qp, short index)
{
    QElemPtr p;
    
#if !defined (UNIX) 
    for (p = qp->qHead; --index > 0; p = p->qLink)
	;
#else
    for (p = qp->qHead; --index > 0; p = p->vcbQElem.qLink)
	;
#endif
    return p;
}

PUBLIC OSErr writefcbp(filecontrolblock *fcbp)
{
    Byte flags;
    OSErr retval;
    
    flags = fcbp->fcbMdRByt;
    if (!(flags & WRITEBIT))
	retval = wrPermErr;
    else if (flags & FLOCKEDBIT)
	retval = fLckdErr;
    else
	retval = noErr;
    return retval;
}
		
PUBLIC OSErr writevcbp(HVCB *vcbp)
{
    INTEGER vflags;
    OSErr retval;
    
    vflags = vcbp->vcbAtrb;
    if (vflags & VSOFTLOCKBIT)
	retval = vLckdErr;
    else if (vflags & VHARDLOCKBIT)
	retval = wPrErr;
    else
	retval = noErr;
    return retval;
}
#endif

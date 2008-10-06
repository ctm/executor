/* Copyright 1992 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_hfsHelper[] =
	    "$Id: hfsHelper.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "OSUtil.h"
#include "FileMgr.h"
#include "SegmentLdr.h"
#include "ToolboxEvent.h"
#include "MemoryMgr.h"
#include "ThinkC.h"
#include "SysErr.h"
#include "DialogMgr.h"
#include "ResourceMgr.h"

#include "rsys/hfs.h"
#include "rsys/file.h"
#include "rsys/setuid.h"
#include "rsys/partition.h"

#if defined (MSDOS) || defined (CYGWIN32)
#include "dosdisk.h"
#include "aspi.h"
#endif

#if !defined(MAC)

#define FLOPPYWORKSFS	"/usr/filesystems/macintosh.fs"
#define NEWFLOPPYWORKSFS	"/usr/filesystems/macintosh.fs.SAVE"

#define MAC30FS		"/usr/filesystems/mac.fs"
#define NEWMAC30FS		"/usr/filesystems/mac.fs.SAVE"

#define OURSOCK	"/dev/HFS_XFer"

#define HFSXFERDOTFS		"/usr/filesystems/HFS_XFer.fs"
#define HFSXFERUTIL		HFSXFERDOTFS "/HFS_XFer.util"

#if defined (NEXT)
PRIVATE LONGINT pipefd[2];
#endif /* NEXT */

PUBLIC LONGINT ROMlib_sock;

PUBLIC void ROMlib_hfsinit( void )
{
#if defined (NEXT)
    struct stat sbuf;
    struct sockaddr sockname;
    LONGINT i;
    char device[DEVNAMELEN];
    LONGINT nread, nwritten;
    LONGINT err;
    LONGINT savemask;
    LONGINT myfd;
    LONGINT mode;

    if (geteuid())
	mustbesetuid();
    if (Ustat(HFSXFERDOTFS, &sbuf) < 0) {
	savemask = umask(0);
	Umkdir(HFSXFERDOTFS, 0755);
	umask(savemask);
    }
    if (((Ustat(HFSXFERUTIL, &sbuf) < 0) || (sbuf.st_mtime < ROMlib_ourmtime))
							&& ROMlib_xfervmsize) {
	myfd = Uopen(HFSXFERUTIL, O_BINARY|O_CREAT|O_WRONLY, 000);
	if (myfd >= 0) {
	    nwritten = write(myfd, ROMlib_xfervmaddr, ROMlib_xfervmsize);
	    err = close(myfd);
	    if (nwritten == ROMlib_xfervmsize && err == 0)
		Uchmod(HFSXFERUTIL, 04711);
	    else
		Uunlink(HFSXFERUTIL);
	}
    }
    if (Ustat(FLOPPYWORKSFS, &sbuf) >= 0)
	Urename(FLOPPYWORKSFS, NEWFLOPPYWORKSFS);
    if (Ustat(MAC30FS, &sbuf) >= 0)
	Urename(MAC30FS, NEWMAC30FS);
    pipefd[1] = -1;
    pipe((void *) pipefd);
    if (fork() == 0) {
	setpgrp(0, 0);
	for (i = 1; i < NSIG; ++i)
	    signal(i, SIG_IGN);
	close(pipefd[1]);
	do {
	    nread = read(pipefd[0], device, sizeof(device));
	    if (nread > 1) {
		sscanf(device+strlen(device)+1, "%o", &mode);
		Uchmod(device, mode);
	    }
	} while (nread > 0);
	Urename(NEWFLOPPYWORKSFS, FLOPPYWORKSFS);
	Urename(NEWMAC30FS,       MAC30FS);
	Uunlink(OURSOCK);
	exit(0);
    } else
	close(pipefd[0]);

    ROMlib_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    gui_assert(ROMlib_sock >= 0);
    sockname.sa_family = AF_UNIX;
    memmove(sockname.sa_data, OURSOCK, sizeof(OURSOCK));
    Uunlink(OURSOCK);
    err = bind(ROMlib_sock, &sockname, sizeof(sockname));
    if (err < 0)
	someoneelseonfloppy();
/*  gui_assert(err >= 0); */
    ROMlib_setuid(getuid());
    err = fcntl(ROMlib_sock, F_SETFL, FNDELAY);
/*  gui_assert(err >= 0); */
    err = listen(ROMlib_sock, 1);
/*  gui_assert(err >= 0); */
#endif /* NEXT */
}

/*
 * NOTE: The weird name constructed below is inherited from HFS_XFer.util.c,
 *	 which is an ARDI written NEXTSTEP atrocity.
 */

#if !defined(LINUX)
#define EJECTABLE(buf) FALSE
#else
/* #warning this is not the proper way to tell if something is ejectable */
#define EJECTABLE(buf) (buf[strlen(buf)-3] == 'f' && buf[strlen(buf)-2] == 'd')
#endif

#define ASSIGN_NAME_MODE_STRING(buf, lenp, filename, sbuf)		      \
do {									      \
    char ejectable;							      \
									      \
    ejectable = EJECTABLE(filename);					      \
    *(lenp) = strlen((filename)) + 1 + 6 + 1 + 1 + 1;			      \
    (buf) = alloca(*(lenp));						      \
    sprintf((buf), "%s%c%06o%c%c", (filename), 0, (sbuf).st_mode & 07777, 0,  \
								   ejectable);\
} while (0)

#define NRETRIES	5

#if defined (NEXT)
PUBLIC long ROMlib_priv_open(const char *filename, long mode)
{
    long retval;
    struct stat sbuf;
    long savemode;
    char *dname;
    long len;
    int count;

    if (Ustat(filename, &sbuf) < 0)
	retval = ROMlib_maperrno();
    else {
	savemode = sbuf.st_mode;
	sbuf.st_mode = 0666;
	ASSIGN_NAME_MODE_STRING(dname, &len, filename, sbuf);
	write(pipefd[1], dname, len);
	count = NRETRIES;
	do {
	    if (count != NRETRIES)
		Delay(60, (LONGINT *) 0);
	    retval = Uopen(filename, mode);
	} while ((retval < 0) && (errno == EACCES) && --count);
	if (retval < 0)
	    retval = ROMlib_maperrno();
	sbuf.st_mode = savemode;
	ASSIGN_NAME_MODE_STRING(dname, &len, filename, sbuf);
	write(pipefd[1], dname, len);
    }
    return retval;
}
#else
PUBLIC long
ROMlib_priv_open (const char *filename, long mode)
{
  long retval;
    
  retval = Uopen(filename, mode, 0);
  if (retval < 0)
    retval = ROMlib_maperrno();

  return retval;
}
#endif

PRIVATE void eject_floppy_notify( void )
{

/*
 * We make the test for 'ALRT' because when Executor shuts down, an ejectable
 * volume may be unmounted after the volume with the System file has already
 * been unmounted.  In the past, this caused a crash.  It may make sense to
 * deliberately unmount last the volume containing System, but that's a fix
 * for another day.
 */

  if (WWExist == EXIST_YES && GetResource(TICK("ALRT"), EJECTALERTID))
    Alert(EJECTALERTID, (ProcPtr) 0);
}

PUBLIC OSErr ROMlib_ejectfloppy( LONGINT floppyfd )
{
    OSErr err;

    err = noErr;
#if defined(MSDOS) || defined (CYGWIN32)
    if (floppyfd == -1 || (floppyfd & (DOSFDBIT|ASPIFDBIT))) {
      if (floppyfd != -1)
	{
	  if (floppyfd & DOSFDBIT)
	    {
	      floppyfd &= ~DOSFDBIT;
	      dosdisk_close(floppyfd, TRUE);
	    }
#if defined (MSDOS)
	  else
	    {
	      floppyfd &= ~ASPIFDBIT;
	      aspi_disk_close(floppyfd, TRUE);
	    }
#endif
	}
      eject_floppy_notify();
    } else {
#endif
#if defined(NEXTSTEP)
	if (floppyfd != -1 && ioctl(floppyfd, DKIOCEJECT, (char *) 0) < 0)  {
	    fprintf(stderr, "couldn't eject disk\n");
	    err = ioErr;
	}
#endif
	if (floppyfd != -1)
	  close(floppyfd);
#if defined(LINUX)
	eject_floppy_notify();
#endif
#if defined(MSDOS) || defined (CYGWIN32)
    }
#endif
    return err;
}

PUBLIC void ROMlib_OurClose( void )
{
    HVCB *vcbp, *next;
    ParamBlockRec pbr;

    for (vcbp = (HVCB *) MR(VCBQHdr.qHead); vcbp; vcbp = next) {
	next = (HVCB *) MR(vcbp->qLink);
	pbr.ioParam.ioNamePtr = 0;
	if (Cx(vcbp->vcbCTRef)) {
	    pbr.ioParam.ioVRefNum = vcbp->vcbVRefNum;
	    PBUnmountVol(&pbr);
#if defined (NEXTSTEP)
	    if (!(Cx(vcbp->vcbAtrb) & VNONEJECTABLEBIT) && Cx(vcbp->vcbDrvNum))
		ROMlib_ejectfloppy(((VCBExtra *) vcbp)->u.hfs.fd);
#endif
	} else
	    ROMlib_dbm_close((VCBExtra *) vcbp);
    }
}

#if 0
PRIVATE BOOLEAN isejectable( const charCx( *dname), LONGINT fd )
{
    BOOLEAN retval;
#if defined(NEXTSTEP)
    struct scsi_req sr;
    char inqbuf[sizeof(struct inquiry_reply) + 3];
    struct inquiry_replyCx( *inqp);
    const charCx( *p);
#endif

    /* look for rfd[0-9] */
    retval = FALSE;
#if defined(NEXTSTEP)
    for (p = dname; p = index(p, 'r'); ++p) {
	if (p[1] == 'f' && p[2] == 'd' && isdigit(p[3])) {
	    retval = TRUE;
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
	    retval = TRUE;
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

PRIVATE LONGINT try_to_open_disk( const char *dname,
		   LONGINT *bsizep, LONGINT *maxbytesp, drive_flags_t *flagsp,
				 uint32 *offsetp)
{
    LONGINT floppyfd;
    int len;
#if defined(NEXTSTEP)
    struct drive_info drvinfo;
#endif

    *flagsp = 0;
    len  = strlen(dname)+1;	/* first component: name */
    len += strlen(dname+len)+1;	/* second component: permission */
    if (!dname[len])
      *flagsp |= DRIVE_FLAGS_FIXED;	/* third component: ejectable */
#if defined (NEXT)
    if (pipefd[1] != -1)
	write(pipefd[1], dname, len);
#endif /* NEXT */

#if !defined (CYGWIN32)
#define EXTRA_BITS O_EXCL
#else
#define EXTRA_BITS 0
#endif

    if ((floppyfd = Uopen(dname, O_BINARY|O_RDWR|EXTRA_BITS, 0000)) < 0 &&
			 (*flagsp |= DRIVE_FLAGS_LOCKED,
		      (floppyfd = Uopen(dname, O_BINARY|O_RDONLY|EXTRA_BITS,
					0000)) < 0))
	/* fprintf(stderr, "can't open %s\n", dname) */;
    else {
#if defined(NEXTSTEP)
	if (ioctl(floppyfd, DKIOCINFO, &drvinfo) == 0) {
	    *bsizep = drvinfo.di_devblklen;
	    *maxbytesp = drvinfo.di_maxbcount / *bsizep * *bsizep;
	} else {
	    *bsizep = PHYSBSIZE;
	    *maxbytesp = 1024L * 1024;
	}
#else /* defined(NEXTSTEP) */
	*bsizep = PHYSBSIZE;
	*maxbytesp = 1024L * 1024;
#endif /* defined(NEXTSTEP) */
    }

    if (floppyfd >= 0)
      {
	struct stat sbuf;

	if (fstat (floppyfd, &sbuf) >= 0 && (S_IFREG & sbuf.st_mode))
	  *offsetp =  sbuf.st_size % PHYSBSIZE;
	else
	  *offsetp = 0;
      }

    return floppyfd;
}

PRIVATE LONGINT
read_driver_block_size (LONGINT fd, LONGINT bsize, LONGINT maxbytes,
			char aligned_buf[])
{
  LONGINT retval;

  retval = PHYSBSIZE;
  if (ROMlib_readwrite (fd, aligned_buf, PHYSBSIZE, 0, reading, bsize,
			maxbytes) == noErr)
    {
      if (aligned_buf[0] == 0x45 && aligned_buf[1] == 0x52)
	{
	  retval = (unsigned short) CW (*(unsigned short *) &aligned_buf[2]);
	  warning_fs_log ("fd = 0x%x, block size = %d", fd, retval);
	}
    }
  return retval;
}

PUBLIC void
try_to_mount_disk (const char *dname, LONGINT floppyfd, LONGINT *messp,
		   LONGINT bsize, LONGINT maxbytes, drive_flags_t flags,
		   uint32 offset_in)
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
    tbuf = alloca (bsize + 3);
    buf = (char *) (((long)tbuf +3)&~3);
    partition = 0;

    if (floppyfd >= 0)
      driver_block_size = read_driver_block_size (floppyfd, bsize, maxbytes,
						  buf);
    else
      driver_block_size = PHYSBSIZE;

    hfs.fd = (flags & DRIVE_FLAGS_FLOPPY) ? floppyfd : -1;
    hfs.offset = offset_in;
    hfs.bsize = bsize;
    hfs.maxbytes = maxbytes;

    dqp = ROMlib_addtodq (2048L * 2, dname, partition, OURHFSDREF, flags,
			  &hfs);

    if (floppyfd < 0)
/*-->*/return;

    drivenum = CW(dqp->dq.dQDrive);

    pb.ioParam.ioVRefNum = CW(drivenum);

    foundmap = FALSE;
    first    = TRUE;
    offset = hfs.offset + PARTOFFSET * driver_block_size;
    if (ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset, reading,
					       bsize, maxbytes) == noErr) {
	if (buf[0] == PARMAPSIG0 && buf[1] == PARMAPSIG1) {
	    warning_fs_log ("found partition sig at %d, floppyfd = 0x%x",
			    offset, floppyfd);
	    partp = (partmapentry_t *) buf;
	    do {
		if (strncmp((char *) partp->pmPartType, HFSPARTTYPE, 32)
								    == 0) {
		    foundmap = TRUE;
		    if (!first) {
			++partition;
			dqp = ROMlib_addtodq (2048L * 2, dname,
					      partition, OURHFSDREF, flags,
					      &hfs);
			drivenum = CW(dqp->dq.dQDrive);
			pb.ioParam.ioVRefNum = CW(drivenum);
		    }
		    dqp->hfs.offset = hfs.offset + (CL (partp->pmPyPartStart)
				       * driver_block_size);
		    err = hfsPBMountVol(&pb, floppyfd, dqp->hfs.offset, bsize,
					maxbytes, flags, dqp);
		    mess = ((LONGINT) err << 16) | drivenum;
		    if (first) {
			*messp = CL(mess);
			first = FALSE;
		    } else
			PPostEvent(diskEvt, mess, (HIDDEN_EvQElPtr *) 0);
		}
		offset += driver_block_size;
	    } while (ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset,
				      reading, bsize, maxbytes) == noErr &&
			     buf[0] == PARMAPSIG0 && buf[1] == PARMAPSIG1);
	} else if (buf[0] == OLDMAPSIG0 && buf[1] == OLDMAPSIG1) {
	    oldmapp = (oldblock1_t *) buf;
	    for (i = 0; i < NOLDENTRIES &&
			    (Cx(oldmapp->oldmapentry[i].pdStart) != 0 ||
			     Cx(oldmapp->oldmapentry[i].pdSize)  != 0 ||
			     Cx(oldmapp->oldmapentry[i].pdFSID)  != 0) ; ++i) {
/*
* NOTE: We initially tried looking for 'TFS1' in pdFSID, but our Cirrus 80
*	 didn't use that id.
*/
		if (!first) {
		    ++partition;
		    dqp = ROMlib_addtodq (2048L * 2, dname, partition,
					  OURHFSDREF, flags, &hfs);
		    drivenum = CW(dqp->dq.dQDrive);
		    pb.ioParam.ioVRefNum = CW(drivenum);
		}
		dqp->hfs.offset = hfs.offset +
		                  (CL (oldmapp->oldmapentry[i].pdStart)
				   * driver_block_size);
		err = hfsPBMountVol(&pb, floppyfd, dqp->hfs.offset,
				    bsize, maxbytes, flags, dqp);
		mess = ((LONGINT) err << 16) | drivenum;
		if (first) {
		    *messp = CL(mess);
		    first = FALSE;
		} else 
		    PPostEvent(diskEvt, mess, (HIDDEN_EvQElPtr *) 0);
	    }
	    foundmap = TRUE;
	}
    }

    if (!foundmap) {
	for (offset = hfs.offset + VOLUMEINFOBLOCKNO * PHYSBSIZE, i = 4;
	     --i >= 0 ; offset += PHYSBSIZE) {
	    if (i == 0)
	      {
	        if (ROMlib_magic_offset == -1)
/*-->*/		  continue;
	        else
		  offset = ROMlib_magic_offset;
	      }
	    err = ROMlib_readwrite(floppyfd, buf, PHYSBSIZE, offset, reading,
				   bsize, maxbytes);
	    if (err != noErr)
	      {
		warning_unexpected ("fd = 0x%x err = %d, offset = %d, "
				    "bsize = %d, maxbytes = %d", floppyfd,
				    err, offset, bsize, maxbytes);
/*-->*/		    break;
	      }
	    if (buf[0] == 'B' && buf[1] == 'D') {
	      warning_fs_log ("Found HFS volume on 0x%x %d", floppyfd,
			      offset);
	      offset -= VOLUMEINFOBLOCKNO * PHYSBSIZE;
	      foundmap = TRUE;
/*-->*/	      break;
	    } else if (buf[0] == 'H' && buf[1] == '+') {
	      warning_fs_log ("Found HFS+ volume on 0x%x %d", floppyfd,
			      offset);
	    } else {
	      warning_fs_log ("fd = 0x%x, offset = %d, sig = 0x%02x%02x",
			      floppyfd, offset, buf[0], buf[1]);
	    }
	    
	}
	if (foundmap) {
	    dqp->hfs.offset = offset;
	    err = hfsPBMountVol(&pb, floppyfd, offset, bsize, maxbytes,
					      flags, dqp);
	    *messp = CL(((LONGINT) err << 16) | drivenum);
	}
    }
}

PUBLIC void ROMlib_openfloppy( const char *dname, LONGINT *messp)
{
    LONGINT floppyfd;
    LONGINT bsize, maxbytes;
    drive_flags_t flags;
    uint32 offset;

    *messp = 0;
    floppyfd = try_to_open_disk (dname, &bsize, &maxbytes, &flags, &offset);
    if (floppyfd >= 0)
	try_to_mount_disk (dname, floppyfd, messp, bsize, maxbytes, flags,
			   offset);
}

PUBLIC void ROMlib_openharddisk( const char *dname, LONGINT *messp )
{
    char *newbuf;
    long len;
    struct stat sbuf;

    *messp = 0;
    if (Ustat(dname, &sbuf) == 0) {
	ASSIGN_NAME_MODE_STRING(newbuf, &len, dname, sbuf);
	ROMlib_openfloppy(newbuf, messp);
    }
}

#endif

#define JUMPTODONEIF(x)	if ((x)) { err = ioErr; goto DONE; }

PUBLIC OSErr ROMlib_readwrite(LONGINT fd, char *buffer, LONGINT count, LONGINT offset,
			       accesstype rw, LONGINT blocksize, LONGINT maxtransfer)
{
    char *newbuffer;
    LONGINT remainder, totransfer;
    BOOLEAN needlseek;
    OSErr err;
    int  (* readfp)(int fd,       void *buf, int nbytes);
    int  (*writefp)(int fd, const void *buf, int nbytes);
    off_t (*seekfp)(int fd, off_t where, int how);

    if (blocksize > 18 * 1024)
      {
	warning_unexpected ("fd = 0x%x, block size = %d", fd, blocksize);
	return fsDSIntErr;
      }

    if (blocksize == 0)
      {
	blocksize = 2048;
	warning_unexpected ("fd = 0x%x, zero block size", fd);
      }
#if defined(MSDOS) || defined (CYGWIN32)
    if (fd & DOSFDBIT)
      {
	fd &= ~DOSFDBIT;
	seekfp  = dosdisk_seek;
	readfp  = dosdisk_read;
	writefp = dosdisk_write;
      }
#if defined (MSDOS)
    else if (fd & ASPIFDBIT)
      {
	fd &= ~ASPIFDBIT;
	seekfp  = aspi_disk_seek;
	readfp  = aspi_disk_read;
	writefp = aspi_disk_write;
      }
#endif
    else
      {
#endif
	seekfp  = (off_t (*)(int,       off_t,  int)) lseek;
	readfp  = (int   (*)(int,       void *, int)) read;
	writefp = (int   (*)(int, const void *, int)) write;
#if defined(MSDOS) || defined (CYGWIN32)
      }
#endif
    err = noErr;
    newbuffer = 0;
    needlseek = TRUE;
    if ((remainder = offset % blocksize)) {	/* |xxxDATA| */
	remainder = blocksize - remainder;
	totransfer = MIN(count, remainder);
	newbuffer = (char *) (((long) alloca(blocksize + 3)+3) & ~3);
	offset = offset / blocksize * blocksize;
	JUMPTODONEIF(seekfp(fd, offset, L_SET) < 0)
	needlseek = FALSE;
	JUMPTODONEIF(readfp(fd, newbuffer, blocksize) != blocksize)
	if (rw == reading) {
	    memmove(buffer, newbuffer+blocksize-remainder, totransfer);
	} else {
	    memmove(newbuffer+blocksize-remainder, buffer, totransfer);
	    JUMPTODONEIF(seekfp(fd, offset, L_SET) < 0)
	    JUMPTODONEIF(writefp(fd, newbuffer, blocksize) != blocksize)
	}
	buffer += totransfer;
	count  -= totransfer;
	offset += blocksize;
    }
    if (count >= blocksize) {	/* |DATADATADATA...| */
	remainder = count % blocksize;
	count -= remainder;
	if (needlseek) {
	    JUMPTODONEIF(seekfp(fd, offset, L_SET) < 0)
	    needlseek = FALSE;
	}
	while (count) {
	    totransfer = MIN(maxtransfer, count);
	    if (rw == reading) {
		JUMPTODONEIF(readfp(fd, buffer, totransfer) != totransfer)
	    } else {
		JUMPTODONEIF(writefp(fd, buffer, totransfer) != totransfer)
	    }
	    buffer += totransfer;
	    count  -= totransfer;
	    offset += totransfer;
	}
	count = remainder;
    }
    if (count) {	/* |DATAxxx| */
	if (!newbuffer)
	    newbuffer = (char *) (((long) alloca(blocksize+3)+3) & ~3);
	if (needlseek)
	    JUMPTODONEIF(seekfp(fd, offset, L_SET) < 0)
	JUMPTODONEIF(readfp(fd, newbuffer, blocksize) != blocksize)
	if (rw == reading) {
	    memmove(buffer, newbuffer, count);
	} else {
	    memmove(newbuffer, buffer, count);
	    JUMPTODONEIF(seekfp(fd, offset, L_SET) < 0)
	    JUMPTODONEIF(writefp(fd, newbuffer, blocksize) != blocksize)
	}
    }
DONE:
    return err;
}

PUBLIC OSErr
ROMlib_transphysblk (hfs_access_t *hfsp, LONGINT physblock, short nphysblocks,
		     Ptr bufp, accesstype rw, LONGINT *actp)
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
    pb.ioPosMode  = CW(fsFromStart);
    pb.ioPosOffset = CL(physblock);
    err = rw == reading ? PBRead ((ParmBlkPtr) &pb, FALSE) :
			  PBWrite((ParmBlkPtr) &pb, FALSE);
    if (actp)
	*actp = pb.ioActCount;
#else
#if defined(NEXTSTEP)
    if ((LONGINT) bufp & 3) {
	newbufp = alloca( (LONGINT) nphysblocks * PHYSBSIZE + 4);
	newbufp = (Ptr) (((LONGINT) newbufp + 3) & ~3);
	if (rw == writing)
	    memmove(newbufp, bufp, (LONGINT) nphysblocks * PHYSBSIZE);
    } else
#endif 
	newbufp = bufp;
    fd = hfsp->fd;

    err = ROMlib_readwrite(fd, (char *) newbufp,
			   (LONGINT) nphysblocks * PHYSBSIZE,
			   physblock + hfsp->offset, rw, hfsp->bsize,
			   hfsp->maxbytes);
#if defined(NEXTSTEP)
	if (rw == reading && bufp != newbufp && err == noErr)
	    memmove(bufp, newbufp, (LONGINT) nphysblocks * PHYSBSIZE);
#endif
    if (actp)
	*actp = err != noErr ? 0 : CL((LONGINT) nphysblocks * PHYSBSIZE);


#endif
    if (err != noErr)
        warning_unexpected ("fd = 0x%x, err in transphysblock (err = %d)",
			    fd, err);
    return err;
}

PUBLIC char *ROMlib_indexn(char *str, char tofind, INTEGER length)
{
    while (--length >= 0)
	if (*str++ == tofind)
	    return str - 1;
    return 0;
}

#if !defined(str255assign)
PUBLIC void str255assign(StringPtr dstp, StringPtr srcp)
{
    memmove(dstp, srcp, (size_t) srcp[0]+1);
}
#endif /* !defined(str255assign) */

/*
 * ROMlib_indexqueue returns a pointer to the n'th entry on a queue.
 * ROMlib_indexqueue is one based; not zero based.
 */
 
PUBLIC void *ROMlib_indexqueue(QHdr *qp, short index)
{
    QElemPtr p;
    
#if defined(MAC)
    for (p = CL(qp->qHead); (--index > 0) && p; p = CL(p->qLink))
	;
#else
    for (p = MR(qp->qHead); (--index > 0) && p; p = MR(p->vcbQElem.qLink))
	;
#endif
    return p;
}

PUBLIC OSErr ROMlib_writefcbp(filecontrolblock *fcbp)
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
		
PUBLIC OSErr ROMlib_writevcbp(HVCB *vcbp)
{
    INTEGER vflags;
    OSErr retval;
    
    vflags = Cx(vcbp->vcbAtrb);
    if (vflags & VSOFTLOCKBIT)
	retval = vLckdErr;
    else if (vflags & VHARDLOCKBIT)
	retval = wPrErr;
    else
	retval = noErr;
    return retval;
}

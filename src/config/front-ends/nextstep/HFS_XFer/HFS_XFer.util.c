/* #include "rsys/common.h" */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <kernserv/loadable_fs.h>
#include <libc.h>
#include <bsd/dev/scsireg.h>
#include <errno.h>
#include <ctype.h>
#include "fs.h"
#include "rsys/partition.h"

/*
 * This is a hack to allow us to get "disk inserted" events
 */

unsigned char buf[8][512];	/* don't change this size */

#if !defined(TRUE)
#define TRUE	1
#define FALSE	0
#endif /* !defined(TRUE) */

static char isejectable( const char *dname, long fd )
{
    char retval;
#if defined(NEXTSTEP)
    struct scsi_req sr;
    char inqbuf[sizeof(struct inquiry_reply) + 3];
    struct inquiry_reply *inqp;
    const char *p;
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
	inqp = (struct inquiry_reply *) (((long) inqbuf + 3) / 4 * 4);
	bzero (&sr, sizeof(sr));
	sr.sr_cdb.cdb_c6.c6_opcode = C6OP_INQUIRY;
	sr.sr_cdb.cdb_c6.c6_len	   = sizeof(*inqp);
	sr.sr_dma_dir	           = SR_DMA_RD;
	sr.sr_addr	           = (caddr_t) inqp;
	sr.sr_dma_max	           = sr.sr_cdb.cdb_c6.c6_len;
	sr.sr_ioto	           = 1;
	if (ioctl(fd, SGIOCREQ, &sr) == 0 && sr.sr_io_status == 0 &&
							    inqp->ir_removable)
	    retval = TRUE;
    }
#endif
    return retval;
}


/*
 * BE CAREFUL when using modesenseselect.  You don't want to confuse some
 *	      poor SCSI device.
 */

int modesenseselect( long fd, unsigned char opcode,
					           struct mode_sel_data *msdp )
{
    struct scsi_req sr;

    bzero (&sr, sizeof(sr));
    sr.sr_cdb.cdb_c6.c6_opcode = opcode;
    sr.sr_cdb.cdb_c6.c6_len    = 4; /* NOT sizeof(*msdp) */;
    sr.sr_dma_dir	       = opcode == C6OP_MODESELECT ?
							 SR_DMA_WR : SR_DMA_RD;
    sr.sr_addr	               = (caddr_t) msdp;
    sr.sr_dma_max	       = sr.sr_cdb.cdb_c6.c6_len;    
    sr.sr_ioto	               = 1;
    return ioctl(fd, SGIOCREQ, &sr) == 0 && sr.sr_io_status == 0;
}

typedef enum { MODEUNCHANGED = 0, MODE800K = 0x22, MODE1400K = 0x28 } mode_t;

static void setheadermode(struct mode_sel_hdr *hdrp, mode_t mode) {
    hdrp->msh_sd_length_0 = 0;
    hdrp->msh_med_type    = mode;
    hdrp->msh_wp          = 0;
    hdrp->msh_bufmode     = 0;
    hdrp->msh_speed       = 0;
    hdrp->msh_bd_length   = 0;
}

static long moderead(mode_t mode, long fd, char *buf, long ntoread)
{
    long retval;
    struct mode_sel_data msd;
    mode_t savemode;

    retval = -1;
    if (mode != MODEUNCHANGED) {
	if (modesenseselect(fd, C6OP_MODESENSE, &msd)) {
	    savemode = msd.msd_header.msh_med_type;
	    setheadermode(&msd.msd_header, mode);
	    if (modesenseselect(fd, C6OP_MODESELECT, &msd)) {
		retval = read(fd, (char *) buf, ntoread);
		if (retval < 0 && errno == EIO) {
		    setheadermode(&msd.msd_header, savemode);
		    modesenseselect(fd, C6OP_MODESELECT, &msd);
		}
	    }
	}
    }
    if (retval < 0)
	retval = read(fd, (char *) buf, ntoread);
    return retval;
}

static void trywithmode(char *argv0, char *rawname, mode_t mode)
{
    long sock, fd;
    char name[DEVNAMELEN];
    struct sockaddr sockname;
    struct stat sbuf;
    char newbuf[100];
    int len;
    char openit;
    char *p;
    unsigned short nmalblks;
    unsigned long alblksiz;
    long newmode;
    char ejectable;

    if (rawname[0] == 'f')
	sprintf(name, "/dev/r%sb", rawname);
    else
	sprintf(name, "/dev/r%sh", rawname);
    openit = FALSE;
    if ((fd = open(name, O_RDONLY)) >= 0 &&
	        moderead(mode, fd, (char *) buf, sizeof(buf)) == sizeof(buf)) {
	p = argv0 + strlen(argv0) - 3 - 2;	/* ".fs" -3 " "RO" -2 */
	if ((p[0] == 'r' || p[0] == 'R') && (p[1] == 'o' || p[1] == 'O'))
	    newmode = 0444;
	else
	    newmode = 0666;
	if (buf[2][0] == 'B' && buf[2][1] == 'D') {
	    openit = TRUE;
	    nmalblks = (buf[2][18] <<  8) |  buf[2][19];

	    alblksiz = (buf[2][20] << 24) | (buf[2][21] << 16) |
		       (buf[2][22] <<  8) |  buf[2][23];

	    if ( nmalblks * alblksiz <=  1024L * 1024L * 3)
		newmode = 0666;
	} else if ((buf[1][0] == PARMAPSIG0 && buf[1][1] == PARMAPSIG1) ||
		   (buf[1][0] == OLDMAPSIG0 && buf[1][1] == OLDMAPSIG1))
	    openit = TRUE;

	ejectable = isejectable(name, fd);
	close(fd);
	if (openit) {
	    sock = socket(AF_UNIX, SOCK_STREAM, 0);
	    sockname.sa_family = AF_UNIX;
	    memcpy(sockname.sa_data, OURSOCK, sizeof(OURSOCK));
	    if (connect(sock, &sockname, sizeof(sockname)) == 0) {
		stat(name, &sbuf);
		chmod(name, newmode);
		sprintf(newbuf, "%s%c%06o%c%c", name, 0, sbuf.st_mode, 0,
							            ejectable);
		len  = strlen(newbuf)+1;		/* name */
		len += strlen(newbuf+len)+1;	/* perm */
		len += 1;			/* ejectable */
		write(sock, newbuf, len);
		exit(FSUR_RECOGNIZED);
	    }
	}
    }
}

void main(int argc, char *argv[])
{
    if (argc < 3) {
	fprintf(stderr, "too few args\n");
	exit(FSUR_INVAL);
    }
    if (argv[1][0] != '-') {
	fprintf(stderr, "first arg character not '-'\n");
	exit(FSUR_INVAL);
    }
    switch (argv[1][1]) {
    case '8':
	if (strcmp(argv[1], "-800k") == 0)
	    trywithmode(argv[0], argv[2], MODE800K);
	exit(FSUR_UNRECOGNIZED);
	break;
    case '1':
	if (strcmp(argv[1], "-1400k") == 0)
	    trywithmode(argv[0], argv[2], MODE1400K);
	exit(FSUR_UNRECOGNIZED);
	break;
    case 'p':
	trywithmode(argv[0], argv[2], MODEUNCHANGED);
	exit(FSUR_UNRECOGNIZED);
	break;
    case 'm':
	exit(FSUR_IO_SUCCESS);
	break;
    case 'r':
	fprintf(stderr, "repair %s\n", argv[2]);
	exit(FSUR_IO_FAIL);
	break;
    case 'u':
	fprintf(stderr, "unmount %s\n", argv[2]);
	exit(FSUR_IO_FAIL);
	break;
    case 'M':
	fprintf(stderr, "Mount %s on %s\n", argv[2], argv[3]);
	exit(FSUR_IO_FAIL);
	break;
    default:
	fprintf(stderr, "unknown argument\n");
	exit(FSUR_INVAL);
    }
}

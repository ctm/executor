/*
 * This program attempts to dump a SCSI device to stdout, reading
 * 4K chunks at a time.
 *
 * Usage: scsi_dump [-t] unit lun
 *
 * The -t option can be used to tell how big the device is.
 */

#include "rsys/common.h"
#include <stdio.h>
#include <dev/scsireg.h>
#include <sys/file.h>

enum { FALSE, TRUE };

static capacity_reply_t cap;
char buf[1024];

void main(int argc, const char *argv[])
{
    boolean_t goodargs, tflag;
    const char *unitstr, *lunstr;
    int fd;
    scsi_adr_t saddr;
    scsi_req_t cmd;
    
    goodargs = TRUE;
    tflag = FALSE;
    if (argc == 3) {
	unitstr = argv[1];
	lunstr  = argv[2];
    } else if (argc == 4 && argv[1][0] == '-' &&
			    argv[1][1] == 't' &&
			    argv[1][2] == 0) {
	tflag = TRUE;
	unitstr = argv[2];
	lunstr  = argv[3];
    } else
	goodargs = FALSE;
    if (goodargs) {
	saddr.sa_target = atoi(unitstr);
	saddr.sa_lun    = atoi(lunstr);
	if (saddr.sa_target > 7 || saddr.sa_lun > 7)
	    goodargs = FALSE;
    }
    if (!goodargs)
	exit(1);
    fd = open("/dev/sg0", O_RDWR);
    if (fd < 0)
	exit(2);
    if (ioctl(fd, SGIOCSTL, &saddr) < 0)
	exit(3);
    if (ioctl(fd, SGIOCENAS, 0) < 0)
	exit(7);
/*
 * NOTE: since we're using the generic SCSI driver, we can't do a
 *	 SDIOCGETCAP; I know, I tried it.
 */
    bzero(&cmd, sizeof(cmd));
    cmd.sr_cdb.cdb_c10.c10_opcode = C10OP_READCAPACITY;
    cmd.sr_cdb.cdb_c10.c10_lun    = saddr.sa_lun;
    cmd.sr_dma_dir               = SR_DMA_RD;
    cmd.sr_addr                  = (caddr_t) &cap;
    cmd.sr_dma_max               = sizeof(cap);
    cmd.sr_ioto                  = 5;
    if (ioctl(fd, SGIOCREQ, &cmd) < 0)
	exit(6);
    if (tflag) {
        printf("lastblock = %d, blklen = %d\n", cap.cr_lastlba, cap.cr_blklen);
	exit(0);
    } else {
	bzero(&cmd, sizeof(cmd));
	cmd.sr_cdb.cdb_c6.c6_opcode = C6OP_READ;
	cmd.sr_cdb.cdb_c6.c6_lun    = saddr.sa_lun;
	cmd.sr_cdb.cdb_c6.c6_lba    = 0;
	cmd.sr_cdb.cdb_c6.c6_len    = 1;
	cmd.sr_dma_dir              = SR_DMA_RD;
	cmd.sr_addr                 = buf;
	cmd.sr_dma_max              = sizeof(buf);
	cmd.sr_ioto                 = 5;
	if (ioctl(fd, SGIOCREQ, &cmd) < 0)
	    exit(8);
    }
    close(fd);
    exit(0);
}

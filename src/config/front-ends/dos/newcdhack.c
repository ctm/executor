/*
---command codes 04h,08h,09h (except Compaq DOS 3.31, DR DOS 6)---
 0Dh	BYTE	media descriptor (block devices only)
 0Eh	DWORD	transfer address
 12h	WORD	byte count (character devices) or sector count (block devices)
 14h	WORD	starting sector number (block devices only)
 16h	DWORD	(DOS 3.0+) pointer to volume ID if error 0Fh returned
 1Ah	DWORD	(DOS 4.0+) 32-bit starting sector number (block devices with
		  device attribute word bit 1 set only) if starting sector
		  number above is FFFFh (see INT 21/AH=52h)

int ReadLong(DWORD loc, WORD secnum,WORD bufseg)
{
	struct ReadL *pcmd=(struct ReadL *)pcmdbuf;
	pcmd->req.len=sizeof(struct ReadL);
	pcmd->req.unit=0;
	pcmd->req.command=128;
	pcmd->mode=0;
	pcmd->address=MK_FP(bufseg,0);
	pcmd->secnum=secnum;
	pcmd->loc=loc;
	pcmd->readmode=RAW_MODE;
	pcmd->skip[0]=pcmd->skip[1]=0;
	CallDriver();
	return pcmd->req.status;
}

struct ReqHdr {
  BYTE len;
  BYTE unit;
  BYTE command;
  WORD status;
  BYTE reserved[8];
};

struct IOCTLI {
  struct ReqHdr req;
  BYTE descriptor;
  void far *address;
  WORD len;
  WORD secnum;
  void far *ptr;
};

struct DiskInfo {
  BYTE control;
  BYTE lowest;
  BYTE highest;
  DWORD total;
};

struct ReadL {
	struct ReqHdr req;
	BYTE mode;
	void far *address;
	WORD secnum;
	DWORD loc;
	BYTE readmode;
	BYTE skip[2];
	};

*/

#include <stdio.h>
#include <string.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

#define PACKED __attribute__((packed))

typedef struct
{
    uint8 len PACKED; /* 0x00 */
    uint8 unit PACKED; /* 0x01 */
    uint8 command PACKED; /* 0x02 */
    uint16 status PACKED; /* 0x03 */
    uint8 reserved[8] PACKED; /* 0x05 */
    uint8 media_descriptor PACKED; /* 0x0d */
    uint32 transfer_address PACKED; /* 0x0e */
    uint16 sector_count PACKED; /* 0x12 */
    uint16 starting_sector PACKED; /* 0x14 should be 0xffff */
    uint32 volid_pointer PACKED; /* 0x16 */
    uint32 long_starting_sector PACKED; /* 0x1A */
} ioctl_read_t;

enum
{
    IOCTL_INPUT = 4
};

#define xxx(yyy, zzz) ((yyy)*16 + (zzz))

void fill_read_t(ioctl_read_t *readp, uint8 unit, uint16 sector_count,
                 uint32 starting_sector)
{
    memset(readp, 0, sizeof *readp);
    readp->len = sizeof *readp;
    readp->unit = unit;
    readp->command = IOCTL_INPUT;
    readp->transfer_address = xxx(dos_buf_selector, dos_buf_segment);
    readp->sector_count = sector_count;
    readp->starting_sector = 0xFFFF;
    readp->long_starting_sector = starting_sector;
}

int main(void)
{
    int retval;

    printf("%d\n", sizeof(ioctl_read_t));
    retval = 0;
    return retval;
}

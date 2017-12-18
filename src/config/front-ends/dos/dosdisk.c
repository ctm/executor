/* Copyright 1994 - 1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include <pc.h>
#include "dosdisk.h"
#include "vga.h"
#include "rsys/hfs.h"
#include <dos.h>
#include "dpmicall.h"
#include "rsys/dcache.h"
#include "rsys/syncint.h"
#include <sys/farptr.h>

#define MAX_OPEN_DISKS 12 /* Arbitrary.               */
#define BYTES_PER_BLOCK 512 /* Bytes per logical block. */
#define LOWMEM_BUFFER_BLOCKS (TRANSFER_BUFFER_SIZE / BYTES_PER_BLOCK)

#define CD_BLOCK_SIZE 2048
#define LOWMEM_CD_BUFFER_BLOCKS (TRANSFER_BUFFER_SIZE / CD_BLOCK_SIZE)

#define IS_FLOPPY_DRIVE(n) ((n) < 0x80)

typedef struct
{
    unsigned long fpos; /* Byte offset into disk; % 512 == 0. */
    unsigned long num_tracks; /* # of tracks on the disk.           */
    unsigned short num_heads; /* # of disk heads; we assume 1 or 2. */
    unsigned short sectors_per_track; /* Assumed to be same for each track. */
    unsigned char disk_number; /* Actual disk # for this struct.     */
    unsigned char is_open; /* Boolean value; 1 iff open, else 0. */
    unsigned char is_cd_rom; /* 1 iff cd-rom */
} dosdisk_info_t;

/* State for each disk. */
static dosdisk_info_t disks[MAX_OPEN_DISKS];

/* Maps a disk number to the doskdisk_info_t for that disk, iff there exists
 * one.  Returns NULL if there isn't one.
 */
static dosdisk_info_t *
disk_number_to_disk_info(int number)
{
    int i;

    /* Search for it. */
    for(i = MAX_OPEN_DISKS - 1; i >= 0; i--)
        if(disks[i].disk_number == number && disks[i].is_open)
            return &disks[i];

    return NULL;
}

/* Returns an empty dosdisk_info_t struct, or NULL if they are all taken. */
static dosdisk_info_t *
find_empty_slot(void)
{
    int i;

    for(i = MAX_OPEN_DISKS - 1; i >= 0; i--)
        if(!disks[i].is_open)
            return &disks[i];
    return NULL;
}

typedef enum {
    lock = 0, /* don't change these values */
    unlock = 1, /* they correspond to INT 13 ah = 45 args */
} lock_op_t;

PRIVATE void
lock_media_helper(int disk, lock_op_t lock_op)
{
    __dpmi_regs regs;

    dpmi_zero_regs(&regs);
    regs.h.ah = 0x45;
    regs.h.al = lock_op;
    regs.h.dl = disk;
    logging_dpmi_int_check_carry(0x13, &regs, "lock/unlock drive");
}

PRIVATE void
    lock_media(disk)
{
    lock_media_helper(disk, lock);
}

PRIVATE void
    unlock_media(disk)
{
    lock_media_helper(disk, unlock);
}

PRIVATE void
    eject_media(disk)
{
    __dpmi_regs regs;

    dpmi_zero_regs(&regs);
    regs.h.ah = 0x46;
    regs.h.al = 0;
    regs.h.dl = disk;
    logging_dpmi_int_check_carry(0x13, &regs, "eject media");
}

#if 0
PRIVATE void
detect_media_extensions (void)
{
  int i;
    
  warning_unexpected ("about to check");
  for (i = 0; i < 256; ++i)
    {
      int check_carry;

      dpmi_zero_regs (&regs);
      regs.h.ah = 0x41; /* int 13 extension installation check */
      regs.x.bx = 0x55aa;
      regs.h.dl = i;
      check_carry = logging_dpmi_int_check_carry (0x13, &regs,
				  "extended media installation check");
      if (regs.x.bx == 0xaa55)
	{
	  warning_unexpected ("found extensions check_carry = 0x%x, i = 0x%x, "
			      "bx = 0x%x, ah = 0x%x, cx = 0x%x", check_carry,
			      i, regs.x.bx, regs.h.ah, regs.x.cx);
	  break;
	}
    }
}
#endif

PRIVATE bool
check_and_clear_media_change(int disk)
{
    __dpmi_regs regs;
    bool error;
    bool retval;

    dpmi_zero_regs(&regs);
    regs.h.ah = 0x49; /* Extended media change */
    regs.h.dl = disk;
    error = (logging_dpmi_int_check_carry(0x13, &regs, "extended media change")
             == -1);
    retval = error && regs.h.ah == 6;
    return retval;
}

PRIVATE void
complain_about_changed_media(void)
{
    /* detected media change -- what do we do? */
    SysBeep(20);
    Delay(5, 0);
    SysBeep(20);
    Delay(5, 0);
    SysBeep(20);
}

/* Closes a particular disk.  Returns -1 on failure, 0 on success. */
int dosdisk_close(int disk, bool eject_p)
{
    dosdisk_info_t *d;
    int retval;

    d = disk_number_to_disk_info(disk);
    if(d == NULL)
        retval = -1;
    else
    {
        dcache_invalidate(disk | DOSFDBIT, true);
        d->is_open = 0;
        retval = 0;
    }

    unlock_media(disk);
    if(eject_p)
        eject_media(disk);

    return retval;
}

typedef enum {
    FLOPPY_ERROR,
    FLOPPY_360K,
    FLOPPY_1200K,
    FLOPPY_720K,
    FLOPPY_1440K
} floppy_type_t;

static floppy_type_t
floppy_type(int drive_number)
{
    __dpmi_regs regs;
    int dpmi_return;

    dpmi_zero_regs(&regs);
    regs.x.ax = 0x0800;
    regs.x.dx = drive_number & 0x7F;

    dpmi_return = logging_dpmi_int_check_carry(0x13, &regs, "floppy type");

    warning_fs_log("drive_number = %d", drive_number);

    if(dpmi_return == -1
       || (regs.x.ax & 0xFF00) /* %ah != 0 -> error.	*/
       || drive_number > (regs.x.dx & 0xFF)) /* %dl has # of drives.	*/
    {
        warning_fs_log("returning FLOPPY_ERROR");
        return FLOPPY_ERROR;
    }
    /* Return the drive type. */
    return regs.x.bx & 0xFF;
}

/* Identifies the drive number of a CD-ROM drive and returns it if
 * one is found (where 0 == A:, 1 == B:, etc.).  Returns -1 if
 * no CD-ROM is detected.
 */
int dosdisk_find_cdrom(void)
{
    __dpmi_regs regs;
    int retval, dpmi_return;

/* #define TEMPORARY_HACK */
#if defined(TEMPORARY_HACK)
    return -1;
#endif

    dpmi_zero_regs(&regs);
    regs.x.ax = 0x1500;
    regs.x.bx = 0;
    dpmi_return = logging_dpmi_int(0x2F, &regs, "cdrom identify");

    /* Carry bit is meaningless here, so don't check it. */
    if(dpmi_return == -1 || regs.x.bx == 0)
        retval = -1;
    else
    {
        retval = regs.x.cx;
        warning_fs_log("CDROM drive found at %c: (0x%X)",
                       'A' + retval, (unsigned)retval);
    }

    return retval;
}

/*
 * INT 13 uses 0 = floppy a:, 1 = floppy b:, 0x80 = hard disk c:, 0x81 = d:
 * but INT 2F uses 0 = a: (for a CD-ROM), 1 = b:, etc.
 */

PUBLIC int
int13_num_to_int2f_num(int num13)
{
    int retval;

    retval = (num13 & 0x80) ? (num13 & 0x7f) + 2 : num13;
    return retval;
}

/* This opens a particular disk drive.  You must open a disk before you can
 * mess with it.  The DISK parameter used here (and in all the other dosdisk
 * functions) refers to the floppy drive number; 0 for the first floppy
 * drive, 1 for the second, and so on.  You can specify hard drives by ORing
 * the drive number with 0x80.  0x80 is the first hard drive, 0x81 the
 * second, and so on.  Returns -1 on failure, disk on success.
 */
int dosdisk_open(int disk, LONGINT *bsizep, drive_flags_t *flagsp)
{
    unsigned short track_sec;
    unsigned char num_drives;
    unsigned char max_head;
    unsigned char error;
    unsigned short max_track, sectors_per_track;
    dosdisk_info_t *d;
    __dpmi_regs regs;
    static int cd_rom_drive = -2; /* NOTE: we only support 1 CD-ROM */

    warning_fs_log("disk = %d", disk);

    if(cd_rom_drive == -2)
    {
        cd_rom_drive = dosdisk_find_cdrom();
    }

    /* Find out if this disk is already open. */
    d = disk_number_to_disk_info(disk);
    if(d != NULL && !IS_FLOPPY_DRIVE(disk)) /* If so, return -1 */
        /*-->*/ return -1;
    else /* Otherwise, allocate a new slot. */
    {
        if(d == NULL)
            d = find_empty_slot();
        if(d == NULL) /* If we're out of slots, punt! */
            /*-->*/ return -1;
    }

    *flagsp = 0;
    if(int13_num_to_int2f_num(disk) == cd_rom_drive)
    {
        d->num_heads = 0;
        d->num_tracks = 0;
        d->sectors_per_track = 0;
        d->is_cd_rom = 1;
        *bsizep = CD_BLOCK_SIZE;
        *flagsp |= DRIVE_FLAGS_LOCKED;
    }
    else
    {

        if(IS_FLOPPY_DRIVE(disk))
        {
            floppy_type_t type;
            type = floppy_type(disk);

            /* Punt floppies that can't possibly be useful. */
            if(type == FLOPPY_ERROR
               || type == FLOPPY_360K
               || type == FLOPPY_1200K)
                /*-->*/ return -1;
            *flagsp |= DRIVE_FLAGS_FLOPPY;
        }

        /* First, reset the drive controller. */
        dpmi_zero_regs(&regs);
        regs.h.dl = disk;
        regs.h.ah = 0;
        logging_dpmi_int(0x13, &regs, "reset drive 1");

        /* Now query about the disk. */
        dpmi_zero_regs(&regs);
        regs.h.dl = disk;
        regs.h.ah = 8;
        error = (logging_dpmi_int_check_carry(0x13, &regs, "drive_query")
                 == -1);
        track_sec = regs.x.cx;
        max_head = regs.h.dh;
        num_drives = regs.h.dl;

        warning_fs_log("error = %d, track_sec = %d, max_head = %d, num_drives = %d",
                       error, track_sec, max_head, num_drives);

        /* Extract max_track and sectors_per_track fields. */
        max_track = (track_sec >> 8) | ((track_sec & 0xC0) << 2);
        sectors_per_track = track_sec & 0x3F;

        /* Check for error.  Be paranoid. */
        if(error || regs.h.ah != 0 || (disk & 0x7F) >= (num_drives & 0x7F)
           || max_track == 0 || sectors_per_track == 0)
        {
            warning_fs_log("returning error");
            /*-->*/ return -1;
        }

        d->num_heads = max_head + 1;
        d->num_tracks = max_track + 1;
        d->sectors_per_track = sectors_per_track;
        d->is_cd_rom = 0;
        if(!IS_FLOPPY_DRIVE(disk))
            *flagsp |= DRIVE_FLAGS_FIXED;
        *bsizep = PHYSBSIZE;
    }

    /* Stash the returned info in the struct for this disk. */
    d->fpos = 0;
    d->disk_number = disk;
    d->is_open = 1;

    dcache_invalidate(disk | DOSFDBIT, false);

    lock_media(disk);
    check_and_clear_media_change(disk);

    return disk;
}

/* Sets the current position into the disk.  No attempt is made to pin
 * pos to the end of the disk because if we have incorrect geometry info
 * we could be refusing legitimate block accesses.
 * Returns -1 on failure, 0 on success.
 *
 * FIXME - right now we have no way of getting the number of sectors
 * actually present on the disk in the drive; instead, the information
 * we get reflects the maximum capacity of the drive itself.
 */

off_t dosdisk_seek(int disk, off_t pos, int unused)
{
    dosdisk_info_t *d = disk_number_to_disk_info(disk);
    unsigned long bytes_on_disk;

    gui_assert(unused == L_SET);
    if(d == NULL)
    {
        gui_assert(0);
        return -1;
    }
    gui_assert((pos % BYTES_PER_BLOCK) == 0);

    /* Do NOT Pin the fpos at the end of the disk. */
    if(d->is_cd_rom)
        bytes_on_disk = 2047 * 1024 * 1024; /* FIXME */
    else
        bytes_on_disk = (d->num_heads * d->num_tracks
                         * d->sectors_per_track * BYTES_PER_BLOCK);
    if(pos > bytes_on_disk)
    {
        static char been_here = false;

        if(!been_here)
        {
            warning_fs_log("pos = %ld, bytes_on_disk = %lu",
                           (long)pos, bytes_on_disk);
            been_here = true;
        }
        /* pos = bytes_on_disk; NO! */
    }
    d->fpos = pos;

    return pos;
}

/* Returns the current position into the specified disk, or -1UL on failure. */
ULONGINT
dosdisk_tell(int disk)
{
    dosdisk_info_t *d = disk_number_to_disk_info(disk);

    if(d == NULL)
    {
        gui_assert(0);
        return -1;
    }

    return d->fpos;
}

/* Helper function for dosdisk_read and dosdisk_write.  Computes information
 * about the map from a specified logical block number to the head/track/sector
 * containing that block, as well as the number of contiguous blocks,
 * including the specified block, remaining on the desired track.
 */
static void
compute_sector_info(const dosdisk_info_t *d, unsigned long block,
                    unsigned short *head, unsigned short *track,
                    unsigned short *sector,
                    unsigned long *blocks_left_on_track)
{
    unsigned long secnum;

    /* Compute the number of the containing physical sector. */
    secnum = block;

    /* Compute head/track/sector.  We assume that, for disks with two heads,
   * the first track's worth of sectors occurs on one head, the next on the
   * other.  Therefore, with 18 sectors per track, the first 18 sectors
   * occur on track 0, head 0, the next 18 occur on track 0, head 1,
   * the next 18 on track 1, head 0, and so on.
   */
    *head = (secnum / d->sectors_per_track) % d->num_heads;
    *track = secnum / (d->sectors_per_track * d->num_heads);
    *sector = (secnum % d->sectors_per_track) + 1; /* Sector #'s start at 1. */

    /* Compute the # of blocks remaining on this track. */
    *blocks_left_on_track = d->sectors_per_track - (*sector - 1);
}

#if ERROR_SUPPORTED_P(ERROR_FILESYSTEM_LOG)

/* Fills the entire conventional memory DOS buffer with a repeating pattern
 * 0, 1, 2, 3, ... , 254, 255, 0, 1, ...
 */
static void
fill_dos_buf_with_signature(void)
{
    if(ERROR_ENABLED_P(ERROR_FILESYSTEM_LOG))
    {
        char signature_buf[256];
        int q, r;

        for(q = sizeof signature_buf - 1; q >= 0; q--)
            signature_buf[q] = q;

        /* Fill in the main buffer with a repeating pattern. */
        for(r = 0; r < DOS_BUF_SIZE; r += sizeof signature_buf)
        {
            movedata(dos_pm_ds, (unsigned)signature_buf,
                     dos_buf_selector, r,
                     MIN((int)sizeof signature_buf, DOS_BUF_SIZE - r));
        }
    }
}

/* Prints DOS stack used, assuming that `fill_dos_buf_with_signature'
 * was called before the call that used the stack space.
 */
static void
print_dos_stack_used(void)
{
    if(ERROR_ENABLED_P(ERROR_FILESYSTEM_LOG))
    {
        int i;

        for(i = TRANSFER_BUFFER_SIZE; i < DOS_STACK_TOP; i++)
        {
            unsigned char c = _farpeekb(dos_buf_selector, i);
            if(c != (i & 0xFF))
                break;
        }

        if(i == TRANSFER_BUFFER_SIZE)
        {
            warning_fs_log("DOS stack blown!");
            vdriver_shutdown();
            printf("DOS STACK WAS BLOWN!\n");
            exit(-1);
        }
        else
        {
            warning_fs_log("%d bytes of DOS stack space used (%d left)",
                           DOS_STACK_TOP - i, i - TRANSFER_BUFFER_SIZE);
        }
    }
}

/* Dumps out the specified buffer in hex (using od -x syntax) */
PRIVATE void
dump_buf(const void *buf, int size, int starting_address)
{
#if 0 /* Appears to cause too much I/O -- makes things noticeably worse \
     on beaut */
  if (ERROR_ENABLED_P (ERROR_FILESYSTEM_LOG))
    {
      char *p, *e;
      int i;

      e = p = alloca (100 + size * 4);
      p[0] = '\0';

      for (i = 0; i < size; i += 2)
	{
	  if ((i % 16) == 0)
	    e += sprintf (e, "\n%07o:", (unsigned) i+starting_address);
	  e += sprintf (e, " %04x", *(const uint16_t *) ((char *)buf+i));
	}
      
      warning_fs_log ("%s", p);
    }
#endif
}

#else /* !ERROR_SUPPORTED_P (ERROR_FILESYSTEM_LOG) */

#define fill_dos_buf_with_signature()
#define print_dos_stack_used()
#define dump_buf(b, s, a)

#endif /* !ERROR_SUPPORTED_P (ERROR_FILESYSTEM_LOG) */

/* Reads the specified data in from the specified disk at the current
 * file position within the disk.  Returns -1 on error, else number of
 * bytes read.
 *
 * Some data may be transferred even on an error, however.  This advances the
 * disk's file position by the amount read, although the file position is
 * undefined after an error.
 */

enum
{
    CD_DRIVE_NOT_READY = 21
};

#define CD_READ_SECTOR 0x1508

enum
{
    NO_MEDIA_IN_DRIVE = 0x31
};

int dosdisk_read(int disk, void *buf, int num_bytes)
{
    int orig_num_bytes;
    dosdisk_info_t *d = disk_number_to_disk_info(disk);
    void *orig_buf;
    unsigned long start_pos;
    bool old_slow_clock_p;
    bool changed;

    changed = false;
    orig_buf = buf;
    orig_num_bytes = num_bytes;
    /* Make sure they've opened this disk. */
    if(d == NULL)
    {
        gui_assert(0);
        /*-->*/ return -1;
    }

    /* Note that it's OK for the 1024 Hz clock to perform badly here. */
    old_slow_clock_p = set_expect_slow_clock(true);

    start_pos = d->fpos;

    /* Make sure they want an integral number of blocks. */
    if(d->is_cd_rom)
        gui_assert((num_bytes % CD_BLOCK_SIZE) == 0);
    else
        gui_assert((num_bytes % BYTES_PER_BLOCK) == 0);

    /* Loop until we've read in all the requested bytes.  We have to loop
   * because the BIOS traps appear to only let you read in bytes from one
   * track at a time.  So, we read in as much as we can (but no more than
   * we need to) from each track each time through the loop.
   */
    while(num_bytes != 0)
    {
        unsigned long blocks_left_on_track;
        unsigned long bytes_read;
        unsigned char sectors_to_read;
        unsigned short head, track, sector;
        unsigned char sectors_actually_read;
        unsigned char error;
        int error_tries_left;
        int block_size;
        __dpmi_regs regs;
        bool cache_hit_p;

        block_size = d->is_cd_rom ? CD_BLOCK_SIZE : BYTES_PER_BLOCK;
        if(dcache_read(disk | DOSFDBIT, buf, d->fpos, block_size, NULL))
        {
            bytes_read = block_size;
            cache_hit_p = true;
        }
        else if(d->is_cd_rom)
        {
            int error_count;
            uint32_t bad_data_magic_cookie;
            bool cookie_failed_p;

            cache_hit_p = false;

            bad_data_magic_cookie = 0xDE52AA03; /* Unlikely bytes */
            cookie_failed_p = false;
            error_count = 0;

            do
            {
                sectors_to_read = num_bytes / CD_BLOCK_SIZE;
                if(sectors_to_read > LOWMEM_CD_BUFFER_BLOCKS)
                    sectors_to_read = LOWMEM_CD_BUFFER_BLOCKS;

                dpmi_zero_regs(&regs);
                regs.x.ax = CD_READ_SECTOR; /* absolute disk read */
                regs.x.es = dos_buf_segment;
                regs.x.bx = 0;
                regs.x.cx = int13_num_to_int2f_num(d->disk_number);
                regs.x.si = (d->fpos / CD_BLOCK_SIZE) >> 16;
                regs.x.di = d->fpos / CD_BLOCK_SIZE;
                regs.x.dx = sectors_to_read;

                fill_dos_buf_with_signature();

                _farpokel(dos_buf_selector, 0, bad_data_magic_cookie);

                if(!changed)
                    changed = check_and_clear_media_change(d->disk_number);
                error = (logging_dpmi_int_check_carry(0x2F, &regs,
                                                      "cdrom sector read")
                         == -1);

                /* Under WinNT it appears that we get a false success,
	       * as though the CD read call were completely ignored
	       * and the carry happened to come back clear.  We detect
	       * that case here; if ax is untouched and a magic cookie
	       * we wrote to our DOS buffer isn't touched, we get
	       * suspicious and retry with a different cookie.  If
	       * that cookie is also untouched, we know we're getting
	       * bogus results and the read fails.
	       */
                if(!error
                   && regs.x.ax == CD_READ_SECTOR /* ax untouched? */
                   && _farpeekl(dos_buf_selector, 0) == bad_data_magic_cookie)
                {
                    if(cookie_failed_p)
                    {
                        warning_unexpected("CD-ROM read request claimed to be "
                                           "successful but appears to have "
                                           "been ignored.");
                        error = -1;
                    }
                    else
                    {
                        bad_data_magic_cookie ^= 0xF723567F; /* arbitrary */
                        cookie_failed_p = true;
                    }
                }
            } while(error && regs.h.al == CD_DRIVE_NOT_READY
                    && ++error_count < 5);

            print_dos_stack_used();

            if(error)
            {
                goto done;
            }
            bytes_read = sectors_to_read * CD_BLOCK_SIZE;
        }
        else
        {
            cache_hit_p = false;

            compute_sector_info(d, d->fpos / BYTES_PER_BLOCK, &head, &track,
                                &sector, &blocks_left_on_track);

            /* Compute how many sectors to transfer. */
            sectors_to_read = MIN(blocks_left_on_track,
                                  num_bytes / BYTES_PER_BLOCK);
            if(sectors_to_read > LOWMEM_BUFFER_BLOCKS)
                sectors_to_read = LOWMEM_BUFFER_BLOCKS;

            for(error_tries_left = 3; error_tries_left > 0; error_tries_left--)
            {
                dpmi_zero_regs(&regs);
                regs.h.ah = 2;
                regs.h.al = sectors_to_read;
                regs.h.ch = track & 0xFF;
                regs.h.cl = (sector & 0x3F) | ((track & 0x300) >> 2);
                regs.h.dh = head;
                regs.h.dl = disk;
                regs.x.es = dos_buf_segment;
                regs.x.bx = 0;

                fill_dos_buf_with_signature();

                if(!changed)
                {
                    changed = check_and_clear_media_change(disk);
                    if(changed && check_and_clear_media_change(disk))
                    {
                        /* no media in drive ... save time by giving up now */
                        error_tries_left = 0;
                        break;
                    }
                }
                error = (logging_dpmi_int_check_carry(0x13, &regs,
                                                      "sector read")
                         == -1);

                print_dos_stack_used();

                warning_fs_log("sectors_to_read = %d, track = %d, sector = %d, head = %d\n"
                               "disk = %d, error = %d, regs.h.al = %d, regs.h.ah = %d "
                               "regs.x.flags = 0x%x",
                               sectors_to_read, track, sector, head, disk, error, regs.h.al,
                               regs.h.ah, regs.x.flags);

#if 0
	      /* Compute how many bytes we really read in.  For some reason,
	       * we always get 0 for hard drives!  So for hard drives, we
	       * pretend like we read in all the sectors we wanted.  If there
	       * was a problem, the error flag will be set anyway.
	       */
	      if (IS_FLOPPY_DRIVE (disk))
		sectors_actually_read = regs.h.al;
	      else
		sectors_actually_read = sectors_to_read;
#else
                /*
	       * Under DJGPP 1.11 this appears to be needed
	       */
                sectors_actually_read = sectors_to_read;
#endif

                if(!error && regs.h.ah == 0)
                    bytes_read = sectors_actually_read * BYTES_PER_BLOCK;
                else
                {
                    bytes_read = 0;
                    if(regs.h.ah == NO_MEDIA_IN_DRIVE)
                    {
                        error_tries_left = 0;
                        break; /* no media?  don't keep trying */
                    }
                }

                /* See if we were successful. */
                if(!error && sectors_actually_read != 0 && regs.h.ah == 0)
                    /*-->*/ break;

#define DMA_OVERRUN 8

                /* Some systems give us frequent DMA overrun errors
	       * when trying to read a large number of blocks while
	       * a SoundBlaster is going.  Occasionally we were failing
	       * enough times in a row to generate an I/O error.
	       * Now when we've had a few failures, or a DMA overrun error,
	       * we back off and only try to read a single sector.
	       */
                if(sectors_to_read > 1
                   && (error_tries_left - 1 == 0 || regs.h.ah == DMA_OVERRUN))
                {
                    sectors_to_read = 1;
                    error_tries_left += 3;
                }

                /* Reset the disk and try again (docs say to do this). */
                dpmi_zero_regs(&regs);
                regs.h.dl = disk;
                regs.h.ah = 0;
                logging_dpmi_int(0x13, &regs, "reset drive 2");
            }

            /* If we ran out of error tries, punt. */
            if(error_tries_left == 0)
            {
                /*-->*/ goto done;
            }
        }

        /* Copy the data we just read into their buffer. */
        if(!cache_hit_p)
        {
            movedata(dos_buf_selector, 0, dos_pm_ds, (unsigned)buf,
                     bytes_read);
            dcache_write(disk | DOSFDBIT, buf, d->fpos, bytes_read, NULL);
        }

        /* Move on and continue reading. */
        d->fpos += bytes_read;
        buf += bytes_read;
        num_bytes -= bytes_read;
    }

done:
    dump_buf(orig_buf, orig_num_bytes - num_bytes, start_pos);
    set_expect_slow_clock(old_slow_clock_p);
    if(changed && num_bytes == 0)
        complain_about_changed_media();
    return orig_num_bytes - num_bytes;
}

/* Reads the specified data in from the specified disk at the current
 * file position within the disk.  Returns -1 on error, else number of
 * bytes read.
 *
 * Some data may still be written even on failure, however.  This advances
 * the disk's file position by the amount written, although the file position
 * is undefined after an error.
 */
int dosdisk_write(int disk, const void *buf, int num_bytes)
{
    dosdisk_info_t *d = disk_number_to_disk_info(disk);
    int orig_num_bytes;
    bool old_slow_clock_p;
    bool changed;

    changed = false;
    orig_num_bytes = num_bytes;
    /* Make sure they've opened this disk. */
    if(d == NULL)
    {
        gui_assert(0);
        /*-->*/ return -1;
    }

    if(d->is_cd_rom)
        /*-->*/ return -1;

    /* Make sure they want an integral number of blocks. */
    gui_assert((num_bytes % BYTES_PER_BLOCK) == 0);

    /* Note that it's OK for the 1024 Hz clock to perform badly here. */
    old_slow_clock_p = set_expect_slow_clock(true);

    /* Invalidate the cache on any write. */
    dcache_invalidate(disk | DOSFDBIT, true);

    /* Loop until all bytes are written. */
    while(num_bytes != 0)
    {
        unsigned long blocks_left_on_track;
        unsigned long bytes_written;
        unsigned char sectors_to_write;
        unsigned short head, track, sector;
        unsigned char sectors_actually_written;
        unsigned char error;
        int error_tries_left;

        bytes_written = 0;
        compute_sector_info(d, d->fpos / BYTES_PER_BLOCK, &head, &track,
                            &sector, &blocks_left_on_track);

        /* Compute how many sectors to transfer. */
        sectors_to_write = MIN(blocks_left_on_track,
                               num_bytes / BYTES_PER_BLOCK);
        if(sectors_to_write > LOWMEM_BUFFER_BLOCKS)
            sectors_to_write = LOWMEM_BUFFER_BLOCKS;

        for(error_tries_left = 3; error_tries_left > 0; error_tries_left--)
        {
            __dpmi_regs regs;

            /* Copy their data into low memory. */
            movedata(dos_pm_ds, (unsigned)buf, dos_buf_selector, 0,
                     sectors_to_write * BYTES_PER_BLOCK);

            /* Write the data out. */
            dpmi_zero_regs(&regs);
            regs.h.ah = 3; /* write */
            regs.h.al = sectors_to_write;
            regs.h.ch = track & 0xFF;
            regs.h.cl = (sector & 0x3F) | ((track & 0x300) >> 2);
            regs.h.dh = head;
            regs.h.dl = disk;
            regs.x.es = dos_buf_segment;
            regs.x.bx = 0;

            if(!changed)
            {
                changed = check_and_clear_media_change(disk);
                if(changed && check_and_clear_media_change(disk))
                {
                    /* no media in drive ... save time by giving up now */
                    error_tries_left = 0;
                    break;
                }
            }
            error = (logging_dpmi_int_check_carry(0x13, &regs, "sector write")
                     == -1);

            warning_fs_log("sectors_to_write = %d, track = %d, sector = %d, head = %d\n"
                           "disk = %d, error = %d, regs.h.al = %d, regs.h.ah = %d, "
                           "regs.x.flags = 0x%x",
                           sectors_to_write, track, sector, head, disk, error, regs.h.al,
                           regs.h.ah, regs.x.flags);

#if 0
	  /* Compute how many bytes we really wrote out.  For some reason,
	   * we always get 0 for hard drives!  So for hard drives, we
	   * pretend like we wrote all the sectors we wanted.  If there
	   * was a problem, the error flag will be set anyway.  */
	  if (IS_FLOPPY_DRIVE (disk))
	    sectors_actually_written = regs.h.al;
	  else
	    sectors_actually_written = sectors_to_write;
#else
            /*
	   * Under DJGPP 1.11 this appears to be needed
	   */

            sectors_actually_written = sectors_to_write;
#endif

            if(!error && regs.h.ah == 0)
                bytes_written = sectors_actually_written * BYTES_PER_BLOCK;
            else
                bytes_written = 0;

            /* See if we were successful. */
            if(!error && sectors_actually_written != 0 && regs.h.ah == 0)
                /*-->*/ break;

            /* Some systems give us frequent DMA overrun errors when
	   * trying to read a large number of blocks while a
	   * SoundBlaster is going.  Occasionally we were failing
	   * enough times in a row to generate an I/O error.  Because
	   * this might happen with writes also, we defensively
	   * protect against that.  Now when we've had a few failures,
	   * or a DMA overrun error, we back off and only try to write
	   * a single sector.
	   */
            if(sectors_to_write > 1
               && (error_tries_left - 1 == 0 || regs.h.ah == DMA_OVERRUN))
            {
                sectors_to_write = 1;
                error_tries_left += 3;
            }

            /* We failed, reset the disk and try again (docs say to do this). */
            dpmi_zero_regs(&regs);
            regs.h.dl = disk;
            regs.h.ah = 0;
            logging_dpmi_int(0x13, &regs, "reset drive 3");
        }

        /* If we ran out of error tries, punt. */
        if(error_tries_left == 0)
        {
            set_expect_slow_clock(old_slow_clock_p);
            /*-->*/ return orig_num_bytes - num_bytes + bytes_written;
        }

        /* Move on and continue writing. */
        d->fpos += bytes_written;
        buf += bytes_written;
        num_bytes -= bytes_written;
    }

    set_expect_slow_clock(old_slow_clock_p);
    if(changed && num_bytes == 0)
        complain_about_changed_media();
    return orig_num_bytes - num_bytes;
}

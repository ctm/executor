/* Copyright 1994-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_aspi[] =
		"$Id: aspi.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include <pc.h>
#include <dos.h>
#include <dpmi.h>
#include <sys/farptr.h>

#include "rsys/stdfile.h" /* for ROMlib_macdrives */

#include "aspi.h"
#include "rsys/hfs.h"
#include "dosdisk.h"
#include "rsys/flags.h"
#include "libc/dosio.h"
#include "rsys/blockinterrupts.h"
#include "dpmicall.h"
#include "rsys/dcache.h"
#include "rsys/syncint.h"
#include "rsys/checkpoint.h"

#if defined (ASPI_STANDALONE)
# include <assert.h>
# undef gui_assert
# define gui_assert assert
# undef warning_unexpected
# define warning_unexpected printf
# define NL "\n"
#else
# define NL
#endif

/*
 * NOTE: This implementation is NOT reentrant.  To make it so would require
 *	 different management of conventional memory, which would either
 *	 require sucking up more memory than we use, or the possibility that
 *	 a reentrant call would fail due to lack of conventional memory
 *	 anyway.  DOS sucks.
 */


PUBLIC int ROMlib_skipaspi = 0;

/* TRUE iff we have a legitimate ASPI driver. */
PRIVATE boolean_t has_aspi_p;

/* Number of host adaptors.*/
PRIVATE int num_host_adaptors;

/* The entry point for the ASPI driver. */
PRIVATE uint16 aspi_entry_segment;
PRIVATE uint16 aspi_entry_offset;


PUBLIC void 
aspi_iterator_init (aspi_iterator_t *aip)
{
  aip->last_adaptor = -1;
  aip->last_target  = 7;
  aip->last_lun     = 7;
}

PRIVATE inline unsigned char
srb_status (void)
{
  unsigned char retval;

#if 0
  retval = _farpeekb (dos_buf_selector,
		      (ASPI_COMMAND_OFFSET
		       + offsetof (aspi_command_t, srb.status)));
#else
  retval = _farpeekb (dos_buf_selector,
		      (ASPI_COMMAND_OFFSET
		       + 1));
#endif

  return retval;
}

/* Waits for completion or until the timeout expires.  Returns TRUE
 * if success was achieved before timeout.
 */
PRIVATE boolean_t
aspi_wait (unsigned long msecs_timeout, unsigned char *statusp)
{
  unsigned char status;
  boolean_t retval;
  unsigned long start_ticks, new_ticks;
  unsigned long ticks_timeout;

  /* Figure out how many 1/18.2's of a second to wait. */
  ticks_timeout = msecs_timeout / (10000 / 182);

  /* Use rawclock for timeouts.  We must do this since Executor's
   * clock isn't running when we first get here.
   */
  start_ticks = rawclock ();
  do
    {
      status = srb_status ();
      new_ticks = rawclock ();
      if (new_ticks < start_ticks)	/* handle midnight wraparound */
	start_ticks = new_ticks;
    }
  while (status == BUSY && new_ticks - start_ticks < ticks_timeout);

  if (statusp)
    *statusp = status;
  retval = (status == NO_ERROR);

  if (!retval)
    warning_unexpected ("status = %d", status);

  return retval;
}

/* Calls the ASPI driver. */
PRIVATE boolean_t
aspi_call (aspi_command_t *cmd)
{
  __dpmi_regs regs;
  __dpmi_raddr cmd_addr;
  boolean_t success_p;
  
  if (!aspi_entry_segment && !aspi_entry_offset)
    {
      warning_unexpected ("segment = 0x%x, offset = 0x%x", aspi_entry_segment,
			aspi_entry_offset);
      return FALSE;
    }

  cmd->srb.status = BUSY;

  /* Copy CMD into conventional memory. */
  movedata (dos_pm_ds, (unsigned) cmd, dos_buf_selector, ASPI_COMMAND_OFFSET,
	    sizeof *cmd);
  
  dpmi_zero_regs (&regs);
  regs.x.ss = dos_buf_segment;
  regs.x.sp = DOS_STACK_TOP;
  regs.x.cs = aspi_entry_segment;
  regs.x.ip = aspi_entry_offset;

  cmd_addr.segment = dos_buf_segment;
  cmd_addr.offset16 = ASPI_COMMAND_OFFSET;

  success_p =
    (__dpmi_simulate_real_mode_procedure_retf_stack (&regs, sizeof cmd_addr,
						     &cmd_addr) != -1);

  if (!success_p)
    warning_unexpected ("aspi_call failed");

  return success_p;
}

PRIVATE void aspi_reset (aspi_command_t *cmd);

/* Makes an ASPI call and waits a set amount of time for a 
 * returned success value.
 */
PRIVATE boolean_t
aspi_call_wait (aspi_command_t *cmd, unsigned long msecs_timeout)
{
  boolean_t retval;
  unsigned char status;
  int count, max;

  count = 0;
  max = 2; /* NOTE: Currently it is safe to retry any command because
	      we only support directly accessible writes.  When we support
	      tape writes, this will not be legit, because a partial write
	      can't be overwritten.  BTW, we have to do the retry because
	      we get a media changed error the first time we try to access
	      a CD-ROM drive after the CD-ROM has been changed. */
  do
    {
      if (!aspi_call (cmd))
	return FALSE;
      retval = aspi_wait (msecs_timeout, &status);
      if (!retval && status == COMPLETED_WITH_ERROR && (count != max -1))
	aspi_reset (cmd);
    }
  while (!retval && status == COMPLETED_WITH_ERROR && ++count < max);

  /* Copy the aspi_command_t struct back to protected mode memory. */
  movedata (dos_buf_selector, ASPI_COMMAND_OFFSET,
	    dos_pm_ds, (unsigned) cmd,
	    sizeof *cmd);

  return retval;
}


/*
 * if the command was an execute command reset the device using
 * most of the fields from that command
 *
 */

PRIVATE void
aspi_reset (aspi_command_t *cmd)
{
#if 0

  /* this code is a lose ... it will prevent zip drives from being
     ejectable because we'll always get media changed errors.  It is
     better to just let a command fail and retry it without a reset. */

  if (cmd->srb.command == EXECUTE_SCSI_COMMAND)
    {
      aspi_command_t reset_cmd;

      memset (&reset_cmd, 0, sizeof reset_cmd);
      reset_cmd.srb = cmd->srb;
      reset_cmd.srb.command = RESET_SCSI_DEVICE;
      reset_cmd.u.rc.target_id = cmd->u.ec.target_id;
      reset_cmd.u.rc.lun = cmd->u.ec.lun;
      aspi_call_wait (&reset_cmd, ASPI_DEFAULT_TIMEOUT);
      warning_fs_log ("reset");
    }
#endif
}

/* Helper function:  fills in the fields of a given srb_t. */
PRIVATE void
set_srb (srb_t *srb, command_t command, uint8 adaptor, uint8 flags)
{
  srb->command  = command;
  srb->status   = 0;	/* To be safe. */
  srb->adaptor  = adaptor;
  srb->flags    = flags;
  srb->reserved = 0;
}


/* Helper function:  fills in the fields of a given execute_command_t. */
PRIVATE void
set_ec_common (execute_command_t *ecb, uint8 target_id, uint8 lun,
	       uint32 data_allocation_length, int cdb_length)
{
  memset (ecb, 0, sizeof *ecb);
  ecb->target_id                   = target_id;
  ecb->lun                         = lun;
  ecb->data_allocation_length      = data_allocation_length;
  ecb->sense_allocation_length     = DATA_SENSE_LENGTH;
  ecb->data_buffer_pointer.offset  = 0;
  ecb->data_buffer_pointer.segment = dos_buf_segment;
  ecb->cdb_length                  = cdb_length;
}


/* Helper function:  fills in the fields of a given read_write_10_t. */
PRIVATE void
set_rw_10 (read_write_10_t *rw, operation_code_t op, uint8 lun,
	uint16 blocks_to_xfer, uint32 logical_block_address)
{
  memset (rw, 0, sizeof *rw);
  rw->operation_code        = op;
  rw->lun_shifted_5         = lun << 5;
  rw->logical_block_address = CL (logical_block_address);
  rw->transfer_length       = CW (blocks_to_xfer);
  rw->must_be_zero          = 0;
}

#if 0
/* NOTE:  For the vast majority of its pre-beta incarnation, Executor only
          used 10 byte CDBs for reads and writes.  I started fiddling around
	  with 6 byte CDBs in a failed attempt to get our zip drive working.
	  Mat and I agree that it would be a bad idea to start using 6 byte
	  CDBs this late into the game.  */

PRIVATE void
set_rw_6 (read_write_6_t *rw, operation_code_t op, uint8 lun,
	      uint16 blocks_to_xfer, uint32 logical_block_address)
{
  memset (rw, 0, sizeof *rw);
  rw->operation_code                = op;
  rw->lun_shifted_5_and_logical_msb = lun << 5;
  rw->lun_shifted_5_and_logical_msb |= (logical_block_address >> 16) & 0x1f;
  rw->logical                       = logical_block_address >> 8;
  rw->logical_lsb                   = logical_block_address;
  rw->transfer_length               = blocks_to_xfer;
  rw->must_be_zero                  = 0;
}
#endif

PRIVATE void
set_mode_sense (mode_sense_t *m, operation_code_t op, uint8 lun,
		uint8 allocation_length)
{
  memset (m, 0, sizeof *m);
  m->operation_code    = op;
  m->lun_shifted_5     = lun << 5;
#if 0
  m->reserved          = 0;
#else
  m->reserved          = 1; /* page 1 */
#endif
  m->allocation_length = allocation_length;
}

PRIVATE void
set_inquiry (inquiry_t *m, operation_code_t op, uint8 lun,
		  uint8 allocation_length)
{
  memset (m, 0, sizeof *m);
  m->operation_code    = op;
  m->lun_shifted_5     = lun << 5;
  m->allocation_length = allocation_length;
}

PRIVATE void
set_start_stop (start_stop_t *m, operation_code_t op, uint8 lun, uint8 immed,
		uint8 start_stop_val)
{
  memset (m, 0, sizeof *m);
  m->operation_code = op;
  m->lun_shifted_5_plus_immed = (lun << 5) | immed;
  m->start_stop_val = start_stop_val;
}

/* Reads a three byte array as a big endian value. */
PRIVATE uint32
read3 (const uint8 np[3])
{
  return ((uint32) np[0] << 16) | (np[1] << 8) | np[2];
}

char * type_name (unsigned char type)
{
  char *retval;

  switch (type)
    {
    case DIRECT_ACCESS_DEVICE:
      retval = "direct access";
      break;
    case SEQUENTIAL_ACCESS_DEVICE:
      retval = "sequential access";
      break;
    case PRINTER_DEVICE:
      retval = "printer";
      break;
    case PROCESSOR_DEVICE:
      retval = "processor";
      break;
    case WRITE_ONCE_READ_MULTIPLE_DEVICE:
      retval = "worm";
      break;
    case READ_ONLY_DIRECT_ACCESS_DEVICE:
      retval = "r/o direct access";
      break;
    case LOGICAL_UNIT_NOT_PRESENT:
      retval = "not present";
      break;
    default:
      {
	static char unknown[5];
	
	sprintf (unknown, "0x%02x", type);
	retval = unknown;
      }
    }
  return retval;
}

PRIVATE peripheral_type_t
get_device_type (const aspi_iterator_t *aip)
{
  peripheral_type_t retval;
  aspi_command_t cmd;

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, GET_DEVICE_TYPE, aip->last_adaptor, 0);
  cmd.u.gdt.target_id = aip->last_target;
  cmd.u.gdt.lun       = aip->last_lun;

  if (aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
    {
      retval = cmd.u.gdt.device_type;
      warning_fs_log ("(%d,%d,%d): device type = %s", aip->last_adaptor,
		      aip->last_target, aip->last_lun, type_name (retval));
    }
  else
    {
      warning_unexpected ("aspi_wait_failed (%d,%d,%d)", aip->last_adaptor,
			  aip->last_target, aip->last_lun);
      retval = LOGICAL_UNIT_NOT_PRESENT;
    }
  return retval;
}

PRIVATE boolean_t
is_int13_accessible (const aspi_iterator_t *aip, uint8 *drivep)
{
  boolean_t retval;
  aspi_command_t cmd;

/* #define TEMPORARY_HACK */
#if defined(TEMPORARY_HACK)
  return FALSE;
#endif

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, GET_DISK_DRIVE_INFORMATION, aip->last_adaptor, 0);
  cmd.u.gddi.target_id = aip->last_target;
  cmd.u.gddi.lun = aip->last_lun;

  if (!aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
    retval = FALSE; /* assume it's not accessible */
  else
    {
      retval = !((cmd.u.gddi.drive_flags & INT13_MASK)
		 == NOT_ACCESSIBLE_VIA_INT13);
      if (retval)
	*drivep = int13_num_to_int2f_num (cmd.u.gddi.int_13h_drive);
      warning_fs_log ("(%d,%d,%d) flags = 0x%02x, drive = 0x%02x",
		      aip->last_adaptor, aip->last_target, aip->last_lun,
		      cmd.u.gddi.drive_flags, cmd.u.gddi.int_13h_drive);
    }
  return retval;
}

PRIVATE void
dump_sense (unsigned char *sensep)
{
  char buf[80];
  int i;

  for (i = 0; i < 4; ++i)
    {
      sprintf (buf,
	       "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
	       sensep[0], sensep[1], sensep[2], sensep[3],
	       sensep[4], sensep[5], sensep[6], sensep[7]);
      warning_fs_log (buf);
      sensep += 8;
    }
}

PRIVATE boolean_t
mode_sense (const aspi_iterator_t *aip, unsigned long *block_lengthp,
	    unsigned long *num_blocksp, boolean_t *write_protectp)
{
  boolean_t retval;
  aspi_command_t cmd;

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, EXECUTE_SCSI_COMMAND, aip->last_adaptor, DIR_UNSPECIFIED);
  set_ec_common (&cmd.u.ec, aip->last_target, aip->last_lun,
		 sizeof (mode_sense_data_t), 6);
#if 0
  set_mode_sense (&cmd.u.ec.u.mode_sense, MODE_SENSE, aip->last_lun,
		  sizeof (mode_sense_data_t));
#else
  set_mode_sense (&cmd.u.ec.u.mode_sense, MODE_SENSE, aip->last_lun,
		  12);
#endif
  
  if (!aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
    {
      retval = FALSE;
      warning_fs_log ("(%d) mode_sense failed, host adapter status = %d, "
		      "target status = %d", aip->last_target,
		      cmd.u.ec.host_adaptor_status, cmd.u.ec.target_status);
      dump_sense ((char *) &cmd.u.ec.u.mode_sense
		  + sizeof (cmd.u.ec.u.mode_sense));
      /* TODO: dump sense data? */
    }
  else
    {
      mode_sense_data_t mode_sense_data;

      movedata (dos_buf_selector, 0, dos_pm_ds, (unsigned) &mode_sense_data,
		sizeof (mode_sense_data));
      *block_lengthp
	= read3 (mode_sense_data.block_descriptors[0].block_length);
      if (*block_lengthp == 2340)
	*block_lengthp = 2048;
      *num_blocksp
	= read3 (mode_sense_data.block_descriptors[0].number_of_blocks);
      *write_protectp = mode_sense_data.wp_shifted_7 >> 7;
      warning_fs_log ("(%d) mode_sense succeeded, bs = %ld, num = %ld",
		      aip->last_target, *block_lengthp, *num_blocksp);
      retval = TRUE;
    }
  return retval;
}

/*
 * Try to read block 0 to determine if media is present
 */

PRIVATE boolean_t
media_present (const aspi_iterator_t *aip, int block_length)
{
  boolean_t retval;
  aspi_command_t cmd;

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, EXECUTE_SCSI_COMMAND, aip->last_adaptor,
	   DIR_TARGET_TO_HOST);
  set_ec_common (&cmd.u.ec, aip->last_target, aip->last_lun, block_length, 10);
  set_rw_10 (&cmd.u.ec.u.read_write_10, READ_10, aip->last_lun, 1, 0);
	  
  retval = aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT);

  return retval;
}

PRIVATE void
inquiry (const aspi_iterator_t *aip, boolean_t *removablep)
{
  aspi_command_t cmd;

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, EXECUTE_SCSI_COMMAND, aip->last_adaptor,
	   DIR_TARGET_TO_HOST);
  set_ec_common (&cmd.u.ec, aip->last_target, aip->last_lun, 64, 6);
  set_inquiry (&cmd.u.ec.u.inquiry, INQUIRY, aip->last_lun, 64);
  
  if (!aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
    {
      warning_fs_log ("inquiry failed");
      *removablep = FALSE;
    }
  else
    {
      inquiry_data_t inquiry_data;

      movedata (dos_buf_selector, 0, dos_pm_ds, (unsigned) &inquiry_data,
		sizeof (inquiry_data));
      *removablep = inquiry_data.removable_bit_and_qualifier >> 7;
    }
}

PRIVATE boolean_t
get_aspi_info (aspi_iterator_t *aip, aspi_info_t *aspi_info_p)
{
  boolean_t done_p;
  boolean_t last_success_p;
  boolean_t retval;

  if (!has_aspi_p)
    return FALSE;

  retval = FALSE;
  last_success_p = TRUE;
  done_p = FALSE;
  do
    {
      ++aip->last_lun;
      if (1 || aip->last_lun > 7 || !last_success_p)
	{
	  aip->last_lun = 0;
	  ++aip->last_target;
	  if (aip->last_target > 6)
	    {
	      aip->last_target = 0;
	      ++aip->last_adaptor;
	      if (aip->last_adaptor >= num_host_adaptors)
		done_p = TRUE;
	    }
	}
      last_success_p = FALSE;

      if (!done_p)
	{
	  peripheral_type_t type;
	  boolean_t force_read_only;

	  force_read_only = FALSE;
	  type = get_device_type (aip);
	  switch (type)
	    {
	    case WRITE_ONCE_READ_MULTIPLE_DEVICE:
	    case READ_ONLY_DIRECT_ACCESS_DEVICE:
	      force_read_only = TRUE;
	      /* FALL THROUGH */
	    case DIRECT_ACCESS_DEVICE:
	      {
		boolean_t int13_accessible;
		uint8 drive;

		int13_accessible = is_int13_accessible (aip, &drive);
		if (int13_accessible)
		  ROMlib_macdrives |= (1 << drive);
		else
		  {
		    unsigned long block_length, num_blocks;
		    boolean_t write_protect, removable, media_loaded;
		    
		    inquiry (aip, &removable);

		    media_loaded = mode_sense (aip, &block_length, &num_blocks,
					       &write_protect);

		    if (removable || media_loaded)
		      {
			aspi_info_p->adaptor = aip->last_adaptor;
			aspi_info_p->target  = aip->last_target;
			aspi_info_p->lun     = aip->last_lun;
			aspi_info_p->force_write_protect = force_read_only;
			aspi_info_p->removable = removable;
			aspi_info_p->media_present = media_loaded;

			if (media_loaded)
			  {
			    aspi_info_p->block_length = block_length;

			    aspi_info_p->num_blocks = num_blocks;
			    aspi_info_p->write_protect = (force_read_only
							  ? TRUE
							  : write_protect);
			    if (!media_present (aip, block_length))
			      {
				warning_fs_log ("not ready");
				media_loaded = FALSE;
			      }
			  }
			retval = TRUE;
			done_p = TRUE;
			last_success_p = TRUE;
		      }
		  }
	      }
	      break;
	    default:
	      break;
	    }
	}
    }
  while (!done_p);
  
  return retval;
}

#define MAX_ASPI_DISKS	12	/* arbitrary: two full host adaptors */

PRIVATE aspi_info_t aspi_info[MAX_ASPI_DISKS];


PRIVATE aspi_info_t *
disk_number_to_aspi_info (int disk)
{
  aspi_info_t *retval;

  if (disk >= 0 && disk < MAX_ASPI_DISKS && aspi_info[disk].block_length)
    retval = &aspi_info[disk];
  else
    retval = NULL;

  return retval;
}

PRIVATE void info_to_iterator (aspi_info_t *infop, aspi_iterator_t *iterp)
{
  iterp->last_adaptor = infop->adaptor;
  iterp->last_target = infop->target;
  iterp->last_lun = infop->lun;
}

PRIVATE void
update_aitp (aspi_info_t *aitp)
{
  aspi_iterator_t aip;
  unsigned long block_length;
  unsigned long num_blocks;
  boolean_t write_protect;

  info_to_iterator (aitp, &aip);
  if (mode_sense (&aip, &block_length, &num_blocks, &write_protect))
    {
      aitp->block_length = block_length;
      aitp->num_blocks = num_blocks;
      aitp->write_protect = aitp->force_write_protect ? TRUE : write_protect;
      aitp->media_present = media_present (&aip, block_length);
    }
  else
    aitp->media_present = FALSE;
}

/*
 * aspi_disk_open is private because it does no checking to see whether
 * or not the drive is already open.
 */
PRIVATE boolean_t
aspi_disk_open (int disk, drive_flags_t *flagsp, LONGINT *bsizep)
{
  boolean_t retval;
  aspi_info_t *aitp;

  *flagsp = 0;
  aitp = disk_number_to_aspi_info (disk);
  if (aitp)
    {
      if (aitp->is_open)
	retval = FALSE;
      else
	{
	  if (aitp->removable)
	    update_aitp (aitp);

	  if (!aitp->media_present)
	    retval = FALSE;
	  else
	    {
	      if (aitp->write_protect)
		*flagsp |= DRIVE_FLAGS_LOCKED;
	      if (!aitp->removable)
		*flagsp |= DRIVE_FLAGS_FIXED;
	      *bsizep = aitp->block_length;
	      warning_fs_log ("*bsizep = %d", *bsizep);
	      if (!*bsizep)
		{
		  warning_unexpected ("aitp->block_length == 0");
		  *bsizep = 2048;
		}
	      aitp->is_open = TRUE;
	      retval = TRUE;
	    }
	}
    }
  else
    retval = FALSE;

  dcache_invalidate (disk | ASPIFDBIT, FALSE);	/* just being paranoid...can't hurt */

  return retval;
}


PUBLIC int 
aspi_disk_close (int disk, boolean_t eject)
{
  int retval;
  aspi_info_t *aitp;

  aitp = disk_number_to_aspi_info (disk);
  if (!aitp)
    retval = -1;
  else
    {
      dcache_invalidate (disk | ASPIFDBIT, TRUE);
      aitp->is_open = FALSE;
      retval = 0;
    }


  if (eject && aitp->removable)
    {
      aspi_command_t cmd;

      memset (&cmd, 0, sizeof cmd);
      set_srb (&cmd.srb, EXECUTE_SCSI_COMMAND, aitp->adaptor, DIR_NO_TRANSFER);
      set_ec_common (&cmd.u.ec, aitp->target, aitp->lun, 0, 6);
      set_start_stop (&cmd.u.ec.u.start_stop, START_STOP, aitp->lun, 1, 2);
	  
      aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT);
    }

  return retval;
}


PUBLIC off_t 
aspi_disk_seek (int fd, off_t pos, int unused)
{
  aspi_info_t *aitp;
  unsigned long bytes_on_disk;

  gui_assert (unused == L_SET);
  aitp = disk_number_to_aspi_info (fd);
  if (!aitp)
    {
      warning_unexpected ("Trying to seek on invalid ASPI device!" NL);
      return -1;
    }
  gui_assert ((pos % aitp->block_length) == 0);

  /* Do not pin the fpos at the end of the disk.  Issue a warning the first
     time someone trys to seek past the end.  When we used to pin, we weren't
     able to read CD-ROMs on the HP-4020i */
  bytes_on_disk = aitp->block_length * aitp->num_blocks;
  if (pos > bytes_on_disk)
    {
      static char been_here;

      if (!been_here)
	{
	  warning_unexpected ("(%d,%d,%d) pos = %d, bsize = %d, "
			      "num_blocks = %d",
			      aitp->adaptor,
			      aitp->target,
			      aitp->lun,
			      pos,
			      aitp->block_length,
			      aitp->num_blocks);
	  been_here = TRUE;
	}
      /* pos = bytes_on_disk; NO! */
    }
  aitp->fpos = pos;

  return pos;
}


/* This helper routine does the grunge work of either a read or a write.
 * It returns the number of bytes actually transferred.
 */

PRIVATE int 
aspi_disk_xfer (operation_code_t op, int disk, void *buf, int num_bytes)
{
  int orig_num_bytes;
  aspi_info_t *aspi_info_ptr;
  unsigned long blocks_to_xfer, bytes_to_xfer, bytes_xfered;
  boolean_t old_slow_clock_p;

  /* Sanity check our arguments and grab info about this device. */
  gui_assert (op == READ_10 || op == WRITE_10);

  warning_fs_log ("op = %s, disk = %d, buf = %p, num_bytes = %d",
		  op == READ_10 ? "read" : "write",
		  disk, (char *) buf, num_bytes);

  aspi_info_ptr = disk_number_to_aspi_info (disk);
  if (!aspi_info_ptr)
    {
      warning_unexpected ("Tried to access illegal ASPI device." NL);
      return -1;
    }
  gui_assert ((num_bytes % aspi_info_ptr->block_length) == 0);

  if (op == WRITE_10)
    dcache_invalidate (disk | ASPIFDBIT, TRUE);

  old_slow_clock_p = set_expect_slow_clock (TRUE);

  warning_fs_log ("(%d,%d,%d) pos = %d", aspi_info_ptr->adaptor,
		  aspi_info_ptr->target, aspi_info_ptr->lun,
		  aspi_info_ptr->fpos);

  orig_num_bytes = num_bytes;
  while (num_bytes != 0)
    {
      aspi_command_t cmd;

      memset (&cmd, 0, sizeof cmd);
      /* Compute how much to transfer this time through the loop. */
      blocks_to_xfer = (MIN (num_bytes, TRANSFER_BUFFER_SIZE)
			/ aspi_info_ptr->block_length);
      bytes_to_xfer = blocks_to_xfer * aspi_info_ptr->block_length;

      if (op == READ_10
	  && dcache_read (disk | ASPIFDBIT, buf, aspi_info_ptr->fpos,
			  bytes_to_xfer, NULL))
	{
	  bytes_xfered = bytes_to_xfer;
	}
      else
	{
	  /* Set up the SRB. */
	  set_srb (&cmd.srb, EXECUTE_SCSI_COMMAND, aspi_info_ptr->adaptor,
		   ((op == READ_10) ? DIR_TARGET_TO_HOST : DIR_HOST_TO_TARGET));
	  
	  /* Set up the shared fields of the execute command. */
	  set_ec_common (&cmd.u.ec, aspi_info_ptr->target, aspi_info_ptr->lun,
			 bytes_to_xfer, 10);
	  
	  /* Set up the read_write_10 info. */
	  set_rw_10 (&cmd.u.ec.u.read_write_10, op, aspi_info_ptr->lun,
		  blocks_to_xfer,
		  aspi_info_ptr->fpos / aspi_info_ptr->block_length);
	  
	  /* Copy the bytes to be written to our DOS transfer buffer. */
	  if (op == WRITE_10)
	    movedata (dos_pm_ds, (unsigned) buf, dos_buf_selector, 0,
		      bytes_to_xfer);

	  /* Actually call the ASPI driver. */
	  if (!aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
	    {
	      warning_unexpected ("DANGEROUS SHORT XFER"); /* FIXME */
	      goto done;
	    }
	  
	  /* Transfer all bytes read from DOS memory to our memory. */
	  bytes_xfered = bytes_to_xfer;
	  if (op == READ_10)
	    {
	      movedata (dos_buf_selector, 0, dos_pm_ds, (unsigned) buf,
			bytes_xfered);

	      /* Save away this data in the cache. */
	      dcache_write (disk | ASPIFDBIT, buf, aspi_info_ptr->fpos,
			    bytes_xfered, NULL);
	    }
	}

      /* Note the number of bytes transferred. */
      aspi_info_ptr->fpos += bytes_xfered;
      buf += bytes_xfered;
      num_bytes -= bytes_xfered;
    }

 done:
  set_expect_slow_clock (old_slow_clock_p);
  return orig_num_bytes - num_bytes;
}


PUBLIC int 
aspi_disk_read (int disk, void *buf, int num_bytes)
{
  return aspi_disk_xfer (READ_10, disk, buf, num_bytes);
}


PUBLIC int 
aspi_disk_write (int disk, const void *buf, int num_bytes)
{
#if defined (DISABLE_SCSI_WRITES)
  return -1;
#else
  return aspi_disk_xfer (WRITE_10, disk, (void *) buf, num_bytes);
#endif
}


#define SCSI_DRIVER_NAME "SCSIMGR$"


/* Attempts to open up the ASPI driver, filling in *file_handle with
 * the file handle for the ASPI driver.  Returns TRUE if successful,
 * else FALSE.
 */
PRIVATE boolean_t
open_aspi_driver (int *file_handle)
{
  __dpmi_regs regs;

  *file_handle = 0;  /* Default */

  /* First open the ASPI driver with DOS. */
  dpmi_zero_regs (&regs);
  regs.x.ax = 0x3D00;
  regs.x.ds = dos_buf_segment;
  regs.x.dx = 0;
  movedata (dos_pm_ds, (unsigned) SCSI_DRIVER_NAME, dos_buf_selector, 0,
	    sizeof (SCSI_DRIVER_NAME));
  
  if (logging_dpmi_int_check_carry (0x21, &regs, "ASPI Open") == -1)
    {
      warning_unexpected ("Unable to open ASPI driver: %s" NL,
			  strerror (__doserr_to_errno (regs.x.ax)));
      return FALSE;
    }

  /* Note the DOS file handle for the SCSI driver. */
  *file_handle = regs.x.ax;

  return TRUE;
}


/* Closes the ASPI driver.  Returns TRUE iff successful. */
PRIVATE boolean_t
close_aspi_driver (int file_handle)
{
  __dpmi_regs regs;
  boolean_t retval;

  /* Make the DOS call to close the ASPI driver. */
  dpmi_zero_regs (&regs);
  regs.h.ah = 0x3E;
  regs.x.bx = file_handle;
  retval = (logging_dpmi_int_check_carry (0x21, &regs, "ASPI Close") != -1);
  return retval;
}


/* Computes the entry point for the ASPI driver, storing it
 * in aspi_entry_segment and aspi_entry_offset.  Returns TRUE
 * if successful, else FALSE.
 */
PRIVATE boolean_t
setup_aspi_entry_point (int file_handle)
{
  __dpmi_regs regs;

  /* Zero the 4-byte buffer used for the ASPI return value, to be safe. */
  _farpokel (dos_buf_selector, 0, 0);

  /* Fetch the entry point for the ASPI driver. */
  dpmi_zero_regs (&regs);
  regs.x.ax = 0x4402;
  regs.x.ds = dos_buf_segment;
  regs.x.dx = 0;
  regs.x.cx = 4;
  regs.x.bx = file_handle;
  if (logging_dpmi_int (0x21, &regs, "ASPI Entry") == -1)
    return FALSE;

  /* Load the segment/offset returned by the ASPI driver. */
  aspi_entry_offset  = _farpeekw (dos_buf_selector, 0);
  aspi_entry_segment = _farpeekw (dos_buf_selector, 2);

  /* Only return TRUE if either value was set to nonzero, to be safe. */
  return (aspi_entry_segment || aspi_entry_offset);
}

PRIVATE void
aspi_try_to_open_and_mount_disk (int disk)
{
  char aspi_name[sizeof ("aspi(nn,nn,nn)")];
  LONGINT mess;
  LONGINT blocksize;
  drive_flags_t flags;

  if (aspi_disk_open (disk, &flags, &blocksize))
    {
      sprintf (aspi_name, "aspi(%d,%d,%d)",
	       aspi_info[disk].adaptor, aspi_info[disk].target,
	       aspi_info[disk].lun);
#if defined (DISABLE_SCSI_WRITES)
      flags |= DRIVE_FLAGS_LOCKED;
#endif
      try_to_mount_disk (aspi_name, disk | ASPIFDBIT, &mess,
			 blocksize, 16 * PHYSBSIZE, flags, 0);
      mess = CL (mess);
      if (mess && mess >> 16 == 0)
	{
	  PPostEvent (diskEvt, mess, (HIDDEN_EvQElPtr *) 0);
	  /* TODO: we probably should post if mess returns an
	     error, but I think we get confused if we do */
	}
      else
	{
	  aspi_disk_close (disk, FALSE);
	}
    }
}

#if !defined (ASPI_STANDALONE)
/* Code largely taken from futzwithdosdisks in stdfile.c -- If you find
 * bugs here, you should check there for the same bugs.  These should be
 * merged sometime.
 */
PRIVATE boolean_t
mount_all_aspi_devices (void)
{
  int i;
  aspi_iterator_t ait;

  /* Loop over all potential devices, trying to mount them, until we fail. */
  aspi_iterator_init (&ait);
  for (i = 0; i < NELEM (aspi_info); ++i)
    {
      if (!get_aspi_info (&ait, &aspi_info[i]))
	break;
      
      aspi_try_to_open_and_mount_disk (i);
    }

  return TRUE;
}
#endif /* !ASPI_STANDALONE */


/* Computes and fills in num_host_adaptors, and returns TRUE if successful. */
PRIVATE
boolean_t setup_num_host_adaptors (void)
{
  boolean_t success_p;
  aspi_command_t cmd;

  memset (&cmd, 0, sizeof cmd);
  set_srb (&cmd.srb, HOST_ADAPTOR_INQUIRY, 0, 0);

  if (aspi_call_wait (&cmd, ASPI_DEFAULT_TIMEOUT))
    {
      num_host_adaptors = cmd.u.haq.number_host_adaptors;
      success_p = TRUE;
    }
  else
    success_p = FALSE;

  return success_p;
}


/* Initializes the ASPI driver and mounts all ASPI devices.  Returns
 * TRUE on success, else FALSE.
 */  
PUBLIC boolean_t
aspi_init (void)
{
  int file_handle;
  boolean_t retval;

  if (!ROMlib_skipaspi)
    checkpoint_aspi (checkpointp, begin);
  file_handle = 0;

  gui_assert (ASPI_COMMAND_SPACE >= sizeof (aspi_command_t));

  /* Default to not having ASPI. */
  has_aspi_p = FALSE;

  if (ROMlib_skipaspi || !open_aspi_driver (&file_handle) ||
      !setup_aspi_entry_point (file_handle))
    retval = FALSE;
  else
    {
      /* We close the driver once we have the entry point, because this
       * is what Cliff's old code used to do.  It's presumably safe
       * to refer to the driver entry point after the file handle has
       * been closed. (?)
       */
      close_aspi_driver (file_handle); /* can close once we have
					  entry point. */

      /* Compute the number of host adaptors. */
      if (!setup_num_host_adaptors ())
	retval = FALSE;
      else
	{
	  has_aspi_p = TRUE;

#if !defined (ASPI_STANDALONE)
	  retval = mount_all_aspi_devices ();
#else
	  retval = TRUE;
#endif
	}
    }
  if (!ROMlib_skipaspi)
    checkpoint_aspi (checkpointp, end);
  return retval;
}

PUBLIC void
aspi_rescan (void)
{
  if (!ROMlib_skipaspi)
    {
      int i;

      for (i = 0; i < NELEM (aspi_info) ; ++i)
	{
	  if (aspi_info[i].removable && !aspi_info[i].is_open)
	    aspi_try_to_open_and_mount_disk (i);
	}
    }
}

#if defined (ASPI_STANDALONE)

PUBLIC int 
main (void)
{
  aspi_iterator_t aspi_iterator;
  aspi_info_t aspi_info;

  if (!init_dos_memory ())
    {
      fprintf (stderr, "Unable to initialize DOS memory.\n");
      exit (-1);
    }

  if (aspi_init ())
    {
      aspi_iterator_init (&aspi_iterator);
      while (get_aspi_info (&aspi_iterator, &aspi_info))
	{
	  printf (" adaptor = %d, target = %d, lun = %d\n", aspi_info.adaptor,
		  aspi_info.target, aspi_info.lun);
	  printf (" block length = %lu, num blocks = %lu, "
		  "write protect = %d\n",
		  (unsigned long) aspi_info.block_length,
		  (unsigned long) aspi_info.num_blocks,
		  (int) aspi_info.write_protect);
	}
    }
  else
    puts ("aspi_init failed!");

  return EXIT_SUCCESS;
}
#endif /* ASPI_STANDALONE */

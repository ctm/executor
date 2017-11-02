/* Copyright 1997 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_win_disk[] = "$Id: win_disk.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#define USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES

#include "rsys/common.h"
#include "rsys/drive_flags.h"
#include "rsys/error.h"
#include "rsys/dcache.h"
#include "rsys/hfs.h"

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include "vwin32.h"
#include "dosdisk.h"
#include "win_ntcd.h"

#include "cdenable.h"
#include "vxdiface.h"

#if 0

HANDLE log_handle;

PUBLIC void
win32_log_begin (void)
{
  log_handle = CreateFile ("\\tmp\\log.txt", GENERIC_WRITE, FILE_SHARE_READ, 0,
			   CREATE_ALWAYS,
			   /*FILE_ATTRIBUTE_ATOMIC_WRITE|*/
			   FILE_FLAG_WRITE_THROUGH,
			   0);
}

PUBLIC void
win32_log_data (const char *data, int len)
{
  DWORD n;

  if (!log_handle)
    win32_log_begin ();

  WriteFile (log_handle, data, len, &n, 0);
}

PUBLIC void
win32_log_end (void)
{
  CloseHandle (log_handle);
}
#endif

PRIVATE HMODULE hB2Win32 = 0;
PRIVATE bool cdrom_realmode_p = false;

PUBLIC int
ROMlib_set_realmodecd (int value)
{
  int retval;

  retval = cdrom_realmode_p;
  cdrom_realmode_p = value;
  return retval;
}

enum { MAX_OPEN_DISKS = 30 }; /* arbitrary */

typedef struct
{
  uint32 fpos;
  uint32 sector_size;
  uint32 num_sectors;
  bool open_p;
  bool floppy_p;
  bool cdrom_p;
  bool volume_locked_p;
  HANDLE win_nt_handle;
}
dosdisk_info_t;

PRIVATE dosdisk_info_t disks[MAX_OPEN_DISKS];

PRIVATE which_win32_t which = WIN32_UNKNOWN;
PRIVATE HANDLE vwin32_handle;

/*
 * Maps a disk number to the dosdisk_info_t for that disk, iff there exists
 * one.  Returns NULL if there isn't one.
 */

PRIVATE dosdisk_info_t *
disk_number_to_disk_info (int number)
{
  dosdisk_info_t *retval;

  number &= ~DOSFDBIT;
  retval = (number < (int) NELEM (disks) && disks[number].open_p)
    ? &disks[number] : NULL;

  return retval;
}

PRIVATE void
shutdown (void)
{
  VxdFinal ();
  if (hB2Win32)
    {
      FreeLibrary (hB2Win32);
      hB2Win32 = 0;
    }
}

PRIVATE void
init_vwin32 (void)
{
  if (which == WIN32_UNKNOWN)
    { 
      OSVERSIONINFO info;

      info.dwOSVersionInfoSize = sizeof info;
      if (GetVersionEx (&info) && info.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
	  which = WIN32_NT;
	  CdenableSysInstallStart ();
	}
      else
	{
	  which = WIN32_95;
	  if (!VxdInit ())
	    {
	      warning_unexpected ("VxdInit failed");
	      cdrom_realmode_p = false;
	    }
	  else
	    {
	      VxdPatch (1);
	      atexit (shutdown);
	    }
	  vwin32_handle = CreateFile (VWIN32_VXD_NAME,
				      GENERIC_READ|GENERIC_WRITE, 0, NULL,
				      CREATE_NEW,
				      FILE_FLAG_DELETE_ON_CLOSE, NULL);
  

	}
    }
}

#define DRIVE_TEMPLATE "a:\\"

PRIVATE char *
drive_num_to_string (disk)
{
  char *retval;

  retval = malloc (sizeof DRIVE_TEMPLATE);
  if (disk < 0 || disk > 26)
    *retval = '\0';
  else
    {
      memcpy (retval, DRIVE_TEMPLATE, sizeof DRIVE_TEMPLATE);
      *retval += disk;
    }
  return retval;
}

PRIVATE UINT
drive_type (int disk)
{
  char *drive_string;
  UINT retval;
  UINT old_err_mode;

  old_err_mode = SetErrorMode (SEM_FAILCRITICALERRORS);

  drive_string = drive_num_to_string (disk);
  retval = GetDriveType (drive_string);
  free (drive_string);

  SetErrorMode (old_err_mode);
  return retval;
}

#define WIN_NT_PARTITION_TEMPLATE "\\\\.\\A:" /* 'A' must be 2nd to last
						 non-NUL character.
						 See Below. */

#define WIN_NT_DRIVE_TEMPLATE "\\\\.\\PHYSICALDRIVE0" /* '0' must be last
							 non-NUL character.
							 See Below. */

PRIVATE bool
win_nt_open_common (int disk, HANDLE *handlep, const char *str,
		    int str_len, int offset, char expect_char,
		    drive_flags_t *flagsp)
{
  bool retval;
  char *file_name;

  file_name = alloca (str_len);
  memcpy (file_name, str, str_len);
  assert (file_name[str_len-offset] == expect_char); /* See above. */
  file_name[str_len-offset] += disk;

  *handlep = CreateFile (file_name, GENERIC_READ|GENERIC_WRITE,
			 0, NULL, OPEN_EXISTING,
			 FILE_FLAG_WRITE_THROUGH|FILE_FLAG_RANDOM_ACCESS,
			 NULL);

  if (*handlep == INVALID_HANDLE_VALUE)
    {
      *handlep = CreateFile (file_name, GENERIC_READ,
			     0, NULL,
			     OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
      if (*handlep != INVALID_HANDLE_VALUE)
	*flagsp |= DRIVE_FLAGS_LOCKED;
    }

  retval = *handlep != INVALID_HANDLE_VALUE;

  return retval;
}

PRIVATE bool
win_nt_open (int disk, HANDLE *handlep, drive_flags_t *flagsp)
{
  bool retval;

  retval = win_nt_open_common (disk, handlep, WIN_NT_PARTITION_TEMPLATE,
			       sizeof WIN_NT_PARTITION_TEMPLATE, 3, 'A',
			       flagsp);

  if (!retval && disk >= 2) /* hard disk C starts at 0 */
    retval = win_nt_open_common (disk-2, handlep, WIN_NT_DRIVE_TEMPLATE,
				 sizeof WIN_NT_DRIVE_TEMPLATE, 2, '0', flagsp);
  return retval;
}

PUBLIC int
dosdisk_open (int disk, LONGINT *bsizep, drive_flags_t *flagsp)
{
  int retval;
  dosdisk_info_t *d;
  UINT old_err_mode;

  *flagsp = 0;
  *bsizep = 0;
  old_err_mode = SetErrorMode (SEM_FAILCRITICALERRORS);

  init_vwin32 ();

  d = disk_number_to_disk_info (disk);
  if (d != NULL && !d->floppy_p)
    {
      *bsizep = d->sector_size;
      retval = -1;
    }
  else if (disk >= (int) NELEM (disks))
    {
      *bsizep = 0;
      retval = -1;
    }
  else
    {
      if (d)
	dosdisk_close (disk, false);

      d = &disks[disk];
      {
	bool saved_lock;

	saved_lock = d->volume_locked_p;
	memset (d, 0, sizeof *d);
	d->volume_locked_p = saved_lock;
      }
      d->win_nt_handle = INVALID_HANDLE_VALUE;
      *flagsp = 0;
      switch (which)
	{
	case WIN32_95:
	  d->open_p = true;
	  break;
	case WIN32_NT:
	  d->open_p = win_nt_open (disk, &d->win_nt_handle, flagsp);
	  break;
	default:
	  warning_unexpected (NULL_STRING);
	  d->open_p = false;
	  break;
	}
      {
	char buf[MAX_BYTES_PER_SECTOR];
	int nread;
	UINT dt;

	dt = drive_type (disk);
	switch (dt)
	  {
	  case DRIVE_REMOVABLE:
	    d->sector_size = *bsizep = BYTES_PER_SECTOR;
	    if (disk < 2)
	      {
		*flagsp |= DRIVE_FLAGS_FLOPPY;
		d->floppy_p = true;
	      }
	    break;
	  case DRIVE_FIXED:
	    d->sector_size = *bsizep = BYTES_PER_SECTOR;
	    *flagsp |= DRIVE_FLAGS_FIXED;
	    break;
	  case DRIVE_CDROM:
	    d->sector_size = *bsizep = CDROM_BYTES_PER_SECTOR;
	    *flagsp |= DRIVE_FLAGS_LOCKED;
	    d->cdrom_p = true;
	    break;
	  case DRIVE_REMOTE:
	  case DRIVE_RAMDISK:
	  default:
	    /* probably won't work */
	    warning_unexpected ("dt = %d", dt);
	    d->sector_size = *bsizep = BYTES_PER_SECTOR;
	    break;
	  }
	dosdisk_seek (disk, 0, 0);
	nread = dosdisk_read (disk, buf, *bsizep);
	if (nread == *bsizep || d->floppy_p)
	  retval = disk;
	else
	  {
	    dosdisk_close (disk, false);
	    retval = -1;
	  }
      }
    }

  dcache_invalidate (disk | DOSFDBIT, false);

  SetErrorMode (old_err_mode);

  return retval;
}

#define NEW_LOCK

#if !defined (NEW_LOCK)

PRIVATE int
disk_to_volume (int disk)
{
  int retval;

  retval = disk < 2 ? disk : (0x80 | (disk-2));
  return retval;
}

PRIVATE bool
win_95_lock (int disk)
{
  bool retval;
  vwin32_regs regs;
  int volume;
  DWORD byte_count;
  BOOL result;

  volume = disk_to_volume (disk);
  memset (&regs, 0, sizeof regs);
  regs.ebx = (1 << 8) | volume;
  regs.edx = 1;
  regs.ecx = 0x084b;
  regs.eax = 0x440d;
  regs.flags = 1;

  result = DeviceIoControl (vwin32_handle, VWIN32_IOCTL, &regs, sizeof regs,
			    &regs, sizeof regs, &byte_count, 0);

  retval = result && !(regs.flags & 1);
  return retval;
}

PRIVATE bool
win_95_unlock (int disk)
{
  bool retval;
  vwin32_regs regs;
  int volume;
  DWORD byte_count;
  BOOL result;

  volume = disk_to_volume (disk);
  memset (&regs, 0, sizeof regs);
  regs.ebx = volume;
  regs.ecx = 0x086b;
  regs.eax = 0x440d;
  regs.flags = 1;

  result = DeviceIoControl (vwin32_handle, VWIN32_IOCTL, &regs, sizeof regs,
			    &regs, sizeof regs, &byte_count, 0);

  retval = result && !(regs.flags & 1);
  return retval;
}
#else
PRIVATE  int cat_list[] = { 0x48, 0x08 };

PRIVATE bool
win_95_lock (int disk)
{
  bool retval;
  vwin32_regs regs;
  int volume;
  DWORD byte_count;
  BOOL result;
  int i;
  
  volume = disk + 1; /* woo hoo */

  retval = false;
  for (i = 0; !retval && i < (int) NELEM (cat_list); ++i)
    {
      memset (&regs, 0, sizeof regs);
      regs.ebx = (0 << 8) | volume; /* level 0 lock */
      regs.edx = 0; /* permission */
      regs.ecx = (cat_list[i] << 8)| 0x4a;
      regs.eax = 0x440d;
      regs.flags = 1;

      result = DeviceIoControl (vwin32_handle, VWIN32_IOCTL, &regs, sizeof regs,
				&regs, sizeof regs, &byte_count, 0);

      retval = result && !(regs.flags & 1);
    }

  warning_fs_log ("disk = %d, retval = %d", disk, retval);

  return retval;
}

PRIVATE bool
win_95_unlock (int disk)
{
  bool retval;
  vwin32_regs regs;
  int volume;
  DWORD byte_count;
  BOOL result;
  int i;
  
  volume = disk + 1;
  retval = false;
  for (i = 0; !retval && i < (int) NELEM (cat_list); ++i)
    {
      memset (&regs, 0, sizeof regs);
      regs.ebx = volume;
      regs.ecx = (cat_list[i] << 8)|0x6a;
      regs.eax = 0x440d;
      regs.flags = 1;

      result = DeviceIoControl (vwin32_handle, VWIN32_IOCTL, &regs,
				sizeof regs, &regs, sizeof regs,
				&byte_count, 0);

      retval = result && !(regs.flags & 1);
    }

  warning_fs_log ("disk = %d, retval = %d", disk, retval);

  return retval;
}
#endif

int
dosdisk_close (int disk, bool eject_p)
{
  int retval;
  dosdisk_info_t *d;

  d = disk_number_to_disk_info (disk);
  if (d == NULL)
    retval = -1;
  else
    {
      dcache_invalidate (disk | DOSFDBIT, true);

      if (d->volume_locked_p)
	{
	  win_95_unlock (disk);
	  d->volume_locked_p = false;
	}
      d->open_p = false;
      if (d->win_nt_handle != INVALID_HANDLE_VALUE)
	{
	  CloseHandle (d->win_nt_handle);
	  d->win_nt_handle = INVALID_HANDLE_VALUE;
	}
      retval = 0;
    }

  return retval;
}

PUBLIC off_t
dosdisk_seek (int disk, off_t pos, int unused)
{
  off_t retval;
  dosdisk_info_t *d;

  d = disk_number_to_disk_info (disk);
#warning need to detect seeks past end of device
  if (d == NULL || pos % d->sector_size)
    retval = -1;
  else
    {
      d->fpos = pos;
      retval = 0;
    }

  return retval;
}

PRIVATE int
win_nt_dosdisk_xfer (int disk, dosdisk_info_t *d, void *buf, uint32 offset,
		     int num_bytes, DeviceIoControl_function_t func)
{
  int retval;

  if (d->cdrom_p && func == VWIN32_SECTOR_READ)
    {
      retval = CdenableSysReadCdBytes (d->win_nt_handle, offset, num_bytes,
				       buf);
    }
  else
    {
      if (SetFilePointer (d->win_nt_handle, offset, NULL, FILE_BEGIN)
	  == 0xFFFFFFFF)
	{
	  retval = 0;
	}
      else
	{
	  DWORD count;

	  if (func == VWIN32_SECTOR_WRITE)
	    {
	      WriteFile (d->win_nt_handle, buf, num_bytes, &count, NULL);
	    }
	  else
	    {
	      ReadFile (d->win_nt_handle, buf, num_bytes, &count, NULL);
	    }
	  retval = count;
	}
    }
  return retval;
}

PRIVATE int
win_95_dosdisk_disk_xfer (int disk, dosdisk_info_t *d, void *buf,
			  uint32 offset, int num_bytes,
			  DeviceIoControl_function_t func)
{
  int retval;
  disk_io_t disk_io;
  xfer_sector_t xfer_sector;
  DWORD byte_count;
  BOOL result;

  if (!d->volume_locked_p)
    d->volume_locked_p = win_95_lock (disk);

  if (!d->volume_locked_p)
    retval = 0;
  else
    {
      disk_io.diStartSector = offset / d->sector_size;
      disk_io.diSectors = num_bytes / d->sector_size;
      disk_io.diBuffer = buf;
      memset (&xfer_sector, 0, sizeof xfer_sector);
      xfer_sector.disk_iop = &disk_io;
      xfer_sector.magic = SECTOR_XFER_MAGIC;
      xfer_sector.drive_number_0based = disk;
      xfer_sector.success_flag = 1;

      result = DeviceIoControl (vwin32_handle, func, &xfer_sector,
				sizeof xfer_sector, &xfer_sector,
				sizeof xfer_sector, &byte_count, 0);
      retval = !(xfer_sector.success_flag & 1)
	? num_bytes / d->sector_size * d->sector_size : 0;

      warning_fs_log ("result = %d, xfer_sector.success_flag = %d, retval = %d",
		      result, xfer_sector.success_flag, retval);

    }
  return retval;
}

PRIVATE int
win_95_dosdisk_cdrom_xfer (int disk, dosdisk_info_t *d, void *buf,
			   uint32 offset, int num_bytes,
			   DeviceIoControl_function_t func)
{
  int retval;
  static BOOL (WINAPI *GetSectors) (BYTE drive, DWORD start_sector,
				    WORD nsectors, LPBYTE buf);

  if (cdrom_realmode_p && !GetSectors)
    {
      hB2Win32 = LoadLibrary( "B2Win32.dll" );
      if(hB2Win32)
	GetSectors = (void *) GetProcAddress (hB2Win32, "GETCDSECTORS" );
      if (!GetSectors)
	cdrom_realmode_p = false;
    }

  if (!cdrom_realmode_p)
    retval = VxdReadCdSectors (disk, offset, num_bytes, buf);
  else
    {
      char disk_char;

      disk_char = 'a' + disk;
      if (func == VWIN32_SECTOR_READ
	  && GetSectors (disk_char, offset / d->sector_size,
			 num_bytes / d->sector_size, buf))
	retval = num_bytes;
      else
	retval = 0;
    }
  return retval;
}

PRIVATE int
win_95_dosdisk_xfer (int disk, dosdisk_info_t *d, void *buf, uint32 offset,
			   int num_bytes, DeviceIoControl_function_t func)
{
  int retval;

  retval = d->cdrom_p ? 
    win_95_dosdisk_cdrom_xfer (disk, d, buf, offset, num_bytes, func)
    :
    win_95_dosdisk_disk_xfer (disk, d, buf, offset, num_bytes, func);
    
  return retval;
}

PRIVATE int
dosdisk_xfer (int disk, void *buf, uint32 offset, int num_bytes,
	      DeviceIoControl_function_t func)
{
  int retval;
  dosdisk_info_t *d;
  UINT old_err_mode;

  old_err_mode = SetErrorMode (SEM_FAILCRITICALERRORS);
  d = disk_number_to_disk_info (disk);
  if (!d)
    retval = -1;
  else
    {
      switch (which)
	{
	case WIN32_95:
	  retval = win_95_dosdisk_xfer (disk, d, buf, offset, num_bytes, func);
	  break;
	case WIN32_NT:
	  retval = win_nt_dosdisk_xfer (disk, d, buf, offset, num_bytes, func);
	  break;
	default:
	  warning_unexpected (NULL_STRING);
	  retval = 0;
	  break;
	}
    }

  SetErrorMode (old_err_mode);
  return retval;
}

PRIVATE uint32
read_in (uint32 fd, void *buf, uint32 offset, uint32 count)
{
  uint32 retval;

  fd &= ~DOSFDBIT;
  retval = dosdisk_xfer (fd, buf, offset, count, VWIN32_SECTOR_READ);
  return retval;
}

PUBLIC int
dosdisk_read (int disk, void *buf, int num_bytes)
{
  int retval;
  dosdisk_info_t *d;

  d = disk_number_to_disk_info (disk);
  if (!d)
    retval = -1;
  else
    {
      retval = dcache_read (disk|DOSFDBIT, buf, d->fpos, num_bytes, read_in);
      if (retval > 0)
	d->fpos += retval;
    }
  return retval;
}

PRIVATE uint32
write_back (uint32 fd, const void *buf, uint32 offset, uint32 count)
{
  uint32 retval;

  fd &= ~DOSFDBIT;
  retval = dosdisk_xfer (fd, (void *) buf, offset, count, VWIN32_SECTOR_WRITE);
  return retval;
}

PUBLIC int
dosdisk_write (int disk, const void *buf, int num_bytes)
{
  int retval;
  dosdisk_info_t *d;

  d = disk_number_to_disk_info (disk);
  if (!d)
    retval = -1;
  else
    {
      retval = dcache_write (disk|DOSFDBIT, buf, d->fpos, num_bytes,
			     write_back);
      if (retval > 0)
	d->fpos += retval;
    }
  return retval;
}

PUBLIC bool
is_win_nt (void)
{
  bool retval;

  init_vwin32 ();
  retval = which == WIN32_NT;
  return retval;
}

PUBLIC uint32
win_GetLogicalDriveStrings (size_t size, char *buf)
{
  uint32 retval;

  retval = GetLogicalDriveStrings (size, buf);
  return retval;
}

PUBLIC bool
win_direct_accessible_disk (const char *p)
{
  bool retval;
  UINT dt;
  UINT old_err_mode;

  old_err_mode = SetErrorMode (SEM_FAILCRITICALERRORS);

  dt = GetDriveType (p);
  switch (dt)
    {
    case 0:
    case 1:
    case DRIVE_REMOVABLE:
    case DRIVE_FIXED:
    case DRIVE_CDROM:
      retval = true;
      break;
    case DRIVE_REMOTE:
    case DRIVE_RAMDISK:
    default:
      retval = false;
      break;
    }

  SetErrorMode (old_err_mode);
  return retval;
}

PUBLIC bool
win_access (const char *drive_to_mount)
{
  bool retval;

  UINT old_err_mode;

  old_err_mode = SetErrorMode (SEM_FAILCRITICALERRORS);
  retval = access (drive_to_mount, 0) == 0;
  SetErrorMode (old_err_mode);
  return retval;
}

#if 0
int PASCAL
WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
  int retval;
  HANDLE h;

  freopen ("stdout.txt", "w", stdout);
  setbuf (stdout, 0);
  freopen ("stderr.txt", "w", stderr);
  setbuf (stderr, 0);

  h = CreateFile (VWIN32_VXD_NAME, 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE,
		  NULL);

  if (h == INVALID_HANDLE_VALUE)
    printf ("vxd failure = %d\n", GetLastError ());
  else
    {
      extended_sector_op_t extended;
      read_sector_t read_sector;
      disk_io_t disk_io;
      char buf[2048]; /* how can we tell the sector size if there's no FAT? */
      uint32 byte_count;
      BOOL result;

      printf ("vxd success\n");

      memset (buf, 0, sizeof buf);
      disk_io.diStartSector = 0;
      disk_io.diSectors = 1;
      disk_io.diBuffer = buf;
      memset (&extended, 0, sizeof extended);
      extended.disk_iop = &disk_io;
      extended.drive_number_1based = 1; /* A */
      extended.magic0 = SECTOR_READ_MAGIC;
      extended.magic1 = EXTENDED_SECTOR_OP;
      extended.op = READ_OP;
      extended.success_flag = 0;

      result = DeviceIoControl (h, VWIN32_EXTENDED_OP, &extended,
				sizeof extended, &extended, sizeof extended,
				&byte_count, 0);

      printf ("result = %d, extended.success_flag & 1 == %d, buf[0] = 0x%x\n",
	      result, extended.success_flag & 1, buf[0]);

      memset (buf, 0, sizeof buf);
      disk_io.diStartSector = 0;
      disk_io.diSectors = 1;
      disk_io.diBuffer = buf;
      memset (&read_sector, 0, sizeof read_sector);
      read_sector.disk_iop = &disk_io;
      read_sector.magic = SECTOR_READ_MAGIC;
      read_sector.drive_number_0based = 0; /* A */
      read_sector.success_flag = 1;

      result = DeviceIoControl (h, VWIN32_SECTOR_READ, &read_sector,
				sizeof read_sector, &read_sector,
				sizeof read_sector, &byte_count, 0);

      printf ("result = %d, success_flag & 1 == %d, buf[0] = 0x%x\n", result,
	      read_sector.success_flag & 1, buf[0]);
      

      /* unlock volume? */

      CloseHandle (h);
    }



#if 1
#define FILE_NAME "\\\\.\\A:"
#else
#define FILE_NAME "testfile.txt"
#endif


  h = CreateFile (FILE_NAME, GENERIC_READ|GENERIC_WRITE,
		  FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		  FILE_FLAG_WRITE_THROUGH|FILE_FLAG_RANDOM_ACCESS,
		  NULL);

  if (h == INVALID_HANDLE_VALUE)
    printf ("failure = %d\n", GetLastError ());
  else
    {
      printf ("success\n");
      CloseHandle (h);
    }

  retval = 0;
  return retval;
}
#endif

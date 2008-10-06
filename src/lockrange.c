/* Copyright 2000 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_lockrange[] =
		"$Id: lockrange.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/*
 * The semantics of MacOS's LockRange routine requires that we make a
 * distinction between lock attempts that fail due to ourselves having
 * already locked a byte versus someone else having locked a byte.  On
 * Windows (where we're first implementing this functionality), no such
 * distinction is made.  As such, we need to keep track of which bytes
 * have been locked by ourselves.  Additionally, the Win32 API cautions
 * the programmer to not close a file that has pending locks.  Most likely
 * all recent implementations of Win32 don't really care, since it would
 * be kind of insane for them to do something freakish if someone were
 * to close a file with some bytes locked, but since we're keeping track
 * of the locks we've made, it makes sense to unlock, just in case.
 *
 * Locking is considered sufficiently rare that we use a slow data structure
 * to store our locks.
 */

#include "rsys/common.h"

#include "MemoryMgr.h"

#include "rsys/lockunlock.h"

typedef struct
{
  int fd;
  uint32 start_byte;
  uint32 count;
}
lock_entry_t;

PRIVATE lock_entry_t *entries;
PRIVATE int n_entries;

PRIVATE void
delete_entry (lock_entry_t *entry)
{
  size_t n_bytes;

  --n_entries;
  n_bytes = (entries + n_entries - entry) * sizeof *entries;
  if (n_bytes)
    memmove (entry, &entry[1], n_bytes);
}


PUBLIC OSErr
ROMlib_fd_clear_locks_after_open (int fd, boolean_t be_surprised_p)
{
  int i;
  int n_removed;
  OSErr retval;

  retval = noErr;
  n_removed = 0;
  for (i = 0; i < n_entries; ++i)
    {
      if (entries[i].fd == fd)
	{
	  if (be_surprised_p)
	    warning_unexpected ("fd = %d, start_byte = %d, count = %d",
				fd, entries[i].start_byte, entries[i].count);
	  delete_entry (&entries[i]);
	  ++n_removed;
	  --i;
	}
    }
  if (n_removed)
    {
      typeof (entries) new_entries;

      new_entries = realloc (entries, n_entries * sizeof *entries);
      if (new_entries)
	entries = new_entries;
      else
	retval = memFullErr;
    }
  return retval;
}

PUBLIC OSErr
ROMlib_fd_release_locks_for_close (int fd)
{
  OSErr err;
  OSErr retval;
  int i;

  retval = noErr;
  for (i = 0; i < n_entries; ++i)
    {
      if (entries[i].fd == fd)
	{
	  err = ROMlib_lockunlockrange (fd, entries[i].start_byte,
					entries[i].count, unlock);
	  if (err && retval == noErr)
	    retval = err;
	}
    }

  err = ROMlib_fd_clear_locks_after_open (fd, FALSE);
  if (err && retval == noErr)
    retval = err;

  return retval;
}

PUBLIC OSErr
ROMlib_fd_add_range (int fd, uint32 start_byte, uint32 count)
{
  OSErr retval;

  if (!count)
    retval = noErr;
  else
    {
      typeof (entries) new_entries;

      new_entries = realloc (entries, (n_entries + 1) * sizeof *entries);
      if (!new_entries)
	{
	  retval = afpNoMoreLocks;
	  warning_unexpected ("fd = %d, start_byte = %d, count = %d",
			      fd, start_byte, count);
	}
      else
	{
	  entries = new_entries;
	  entries[n_entries].fd = fd;
	  entries[n_entries].start_byte = start_byte;
	  entries[n_entries].count = count;
	  ++n_entries;
	  retval = noErr;
	}
    }
  
  return retval;
}

PUBLIC OSErr
ROMlib_fd_range_overlap (int fd, uint32 start_byte, uint32 count)
{
  OSErr retval;

  retval = noErr;
  if (count)
    {
      uint32 stop_byte;
      int i;

      stop_byte = start_byte + count;
      if (stop_byte < start_byte)
	stop_byte = ~0;
      for (i = 0; i < n_entries; ++i)
	{
	  if (entries[i].fd == fd)
	    {
	      uint32 entries_stop_byte;

	      entries_stop_byte = entries[i].start_byte + entries[i].count;
	      if (entries_stop_byte < entries[i].start_byte)
		entries_stop_byte = ~0;

	      if (entries[i].start_byte < start_byte)
		{
		  if (entries_stop_byte > start_byte)
		    retval = afpRangeOverlap;
		}
	      else if (entries[i].start_byte > start_byte)
		{
		  if (stop_byte > entries[i].start_byte)
		    retval = afpRangeOverlap;
		}
	      else
		retval = afpRangeOverlap;
	    }
	}
    }

  return retval;
}

PRIVATE lock_entry_t *
find_fd_start_count_helper (int fd, uint32 start_byte, uint32 count)
{
  int i;
  lock_entry_t *retval;

  retval = NULL;
  for (i = 0; !retval && i < n_entries; ++i)
    {
      if (entries[i].fd == fd && entries[i].start_byte == start_byte)
	{
	  if (entries[i].count == count) /* perhaps we should check to
					    see if both of them run past
					    maxuint */
	    retval = &entries[i];
	}
    }
  return retval;
}

PUBLIC OSErr
ROMlib_find_fd_start_count (int fd, uint32 start_byte, uint32 count)
{
  OSErr retval;

  retval = find_fd_start_count_helper (fd, start_byte, count)
    ? noErr : afpRangeNotLocked;

  return retval;
}

PUBLIC OSErr
ROMlib_fd_remove_range (int fd, uint32 start_byte, uint32 count)
{
  OSErr retval;
  lock_entry_t *entry;

  entry = find_fd_start_count_helper (fd, start_byte, count);
  if (!entry)
    retval = afpRangeNotLocked;
  else
    {
      delete_entry (entry);
      retval = noErr;
    }
  
  return retval;
}

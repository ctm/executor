#if !defined(__RSYS_LOCKUNLOCK_H___)
#define __RSYS_LOCKUNLOCK_H___

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: lockunlock.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef enum
{
  lock,
  unlock
}
lockunlock_t;

extern int ROMlib_lockunlockrange (int fd, uint32 begin, uint32 count,
				   lockunlock_t op);


PUBLIC OSErr ROMlib_fd_clear_locks_after_open (int fd,
					       boolean_t be_surprised_p);
PUBLIC OSErr ROMlib_fd_release_locks_for_close (int fd);
PUBLIC OSErr ROMlib_fd_add_range (int fd, uint32 start_byte, uint32 count);
PUBLIC OSErr ROMlib_fd_remove_range (int fd, uint32 start_byte, uint32 count);
PUBLIC OSErr ROMlib_fd_range_overlap (int fd, uint32 start_byte, uint32 count);
PUBLIC OSErr ROMlib_find_fd_start_count (int fd, uint32 start_byte,
					 uint32 count);

#define paramErr (-50)
#define fLckdErr (-45)
#define afpRangeNotLocked (-5020)
#define afpRangeOverlap (-5021)
#define afpNoMoreLocks (-5015)

#endif

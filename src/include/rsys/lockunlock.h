#if !defined(__RSYS_LOCKUNLOCK_H___)
#define __RSYS_LOCKUNLOCK_H___

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
typedef enum {
    lock,
    unlock
} lockunlock_t;

extern int ROMlib_lockunlockrange(int fd, uint32_t begin, uint32_t count,
                                  lockunlock_t op);

PUBLIC OSErr ROMlib_fd_clear_locks_after_open(int fd,
                                              bool be_surprised_p);
PUBLIC OSErr ROMlib_fd_release_locks_for_close(int fd);
PUBLIC OSErr ROMlib_fd_add_range(int fd, uint32_t start_byte, uint32_t count);
PUBLIC OSErr ROMlib_fd_remove_range(int fd, uint32_t start_byte, uint32_t count);
PUBLIC OSErr ROMlib_fd_range_overlap(int fd, uint32_t start_byte, uint32_t count);
PUBLIC OSErr ROMlib_find_fd_start_count(int fd, uint32_t start_byte,
                                        uint32_t count);

#define paramErr (-50)
#define fLckdErr (-45)
#define afpRangeNotLocked (-5020)
#define afpRangeOverlap (-5021)
#define afpNoMoreLocks (-5015)
}
#endif

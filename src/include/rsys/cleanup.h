#if !defined(__RSYS_CLEANUP__)
#define __RSYS_CLEANUP__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: cleanup.h 63 2004-12-24 18:19:43Z ctm $
 */

#if defined(MSDOS) || defined(CYGWIN32)

#define CLEANUP_BATCH_FILE_NAME "+/cleanup.bat"

extern void add_to_cleanup(const char *s, ...);
extern void call_cleanup_bat(void);

#endif

#endif /* !defined(__RSYS_CLEANUP__) */

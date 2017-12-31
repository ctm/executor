#if !defined(__RSYS_CLEANUP__)
#define __RSYS_CLEANUP__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

extern void add_to_cleanup(const char *s, ...);
extern void call_cleanup_bat(void);

#endif /* !defined(__RSYS_CLEANUP__) */

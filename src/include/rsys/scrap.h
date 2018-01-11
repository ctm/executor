#if !defined(__RSYS_SCRAP_H__)
#define __RSYS_SCRAP_H__

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
extern void sendsuspendevent(void);
extern void sendresumeevent(bool cvtclip);

#if defined(CYGWIN32)
extern int get_scrap_helper(void *vh, void *lp, int len, bool cvt_rets);
extern int count_char(const char *p, int len, char c);
extern bool we_lost_clipboard(void);
#if defined(SDL)
extern int get_scrap_helper_dib(void *vh, void *lp);
extern void put_scrap_helper_dib(void *lp);
#endif
#endif
}

#endif

#if !defined(_RSYS_COOKIE_H_)
#define _RSYS_COOKIE_H_

/*
 * Copyright 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: cookie.h 63 2004-12-24 18:19:43Z ctm $
 */

typedef struct
{
  long version;
  long day;
}
cookie_t;

#if defined (CYGWIN32)

extern int win_retrieve_cookie (cookie_t *cookiep);
extern int win_leave_cookie (const cookie_t *cookiep);

#endif

#endif

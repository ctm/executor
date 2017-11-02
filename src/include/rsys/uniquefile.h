#if !defined (_RSYS_UNIQUEFILE_)
#define _RSYS_UNIQUEFILE_

/*
 * Copyright 1995 - 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: uniquefile.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
extern bool unique_file_name (const char *template1,
				   const char *default_template,
				   Str255 result);
}

#endif /* !_RSYS_UNIQUEFILE_ */

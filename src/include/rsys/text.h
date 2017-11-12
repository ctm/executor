#if !defined(_TEXT_H_)
#define _TEXT_H_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: text.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
extern bool disable_text_printing(void);
extern void set_text_printing(bool state);
extern INTEGER ROMlib_wordb(char *p);
extern int ROMlib_forward_del_p;
}

#endif

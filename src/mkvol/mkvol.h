#if !defined(_MKVOL_H_)
#define _MKVOL_H_

#include "rsys/mactype.h"
/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

typedef size_t (*write_funcp_t)(int user_arg, const void *bufp, size_t buf_len);

extern int
format_disk(Executor::GUEST<uint32_t> timevar, const char *volumename, int nsecs,
            write_funcp_t writefuncp, int user_arg);
#endif

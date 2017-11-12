#if !defined(_romlib_float_fcw_h_)
#define _romlib_float_fcw_h_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: float_fcw.h 63 2004-12-24 18:19:43Z ctm $
 */
namespace Executor
{
extern uint32 ROMlib_get_fcw_fsw(void);
extern void ROMlib_set_fcw_fsw(uint32 fcwfsw);
extern void ROMlib_compare_fcw_fsw(uint32 fcwfsw, const char *func, int line);
}
#endif

#if !defined(_romlib_float_fcw_h_)
#define _romlib_float_fcw_h_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
extern uint32_t ROMlib_get_fcw_fsw(void);
extern void ROMlib_set_fcw_fsw(uint32_t fcwfsw);
extern void ROMlib_compare_fcw_fsw(uint32_t fcwfsw, const char *func, int line);
}
#endif

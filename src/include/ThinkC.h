#if !defined(__THINKCDOTH__)
#define __THINKCDOTH__
/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
extern void CDebugStr(StringPtr p);

extern StringPtr CtoPstr(char *str);
extern char *PtoCstr(StringPtr str);
extern pascal trap void C_DebugStr(StringPtr p);
PASCAL_TRAP(DebugStr, 0xABFF);

}
#endif /* __THINKCDOTH__ */

#if !defined(__THINKCDOTH__)
#define __THINKCDOTH__
/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ThinkC.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor
{
extern void CDebugStr(StringPtr p);

extern StringPtr CtoPstr(char *str);
extern char *PtoCstr(StringPtr str);
extern pascal trap void C_DebugStr(StringPtr p);
extern pascal trap void P_DebugStr(StringPtr p);
}
#endif /* __THINKCDOTH__ */

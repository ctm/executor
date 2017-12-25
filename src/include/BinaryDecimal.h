#if !defined(__BINDEC__)
#define __BINDEC__
/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
extern trap void NumToString(LONGINT l, StringPtr s);
extern trap void StringToNum(StringPtr s, LONGINT *lp);
}
#endif /* __BINDEC__ */

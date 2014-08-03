#if !defined (__BINDEC__)
#define __BINDEC__
/*
 * Copyright 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: BinaryDecimal.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
extern trap void NumToString( LONGINT l, StringPtr s );
extern trap void StringToNum( StringPtr s, LONGINT *lp );
}
#endif /* __BINDEC__ */

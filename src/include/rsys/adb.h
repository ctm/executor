#if !defined(_RSYS_ADB_H_)
#define _RSYS_ADB_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

namespace Executor
{
extern void C_adb_service_stub(void);
extern void adb_apeiron_hack(int /*bool*/ deltas_p, ...);
extern void reset_adb_vector(void);
}
#endif

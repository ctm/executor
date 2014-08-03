#if !defined(_RSYS_ADB_H_)
#define _RSYS_ADB_H_

/*
 * Copyright 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: adb.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
extern void C_adb_service_stub (void);
extern void adb_apeiron_hack (boolean_t deltas_p, ...);
extern void reset_adb_vector (void);
}
#endif

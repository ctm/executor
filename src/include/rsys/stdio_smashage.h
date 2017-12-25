#if !defined(_STDIO_SMASHAGE_H_)
#define _STDIO_SMASHAGE_H_

/*
 * Copyright 2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#if defined(STDIO_SMASHAGE_DEBUGGING)
#define fprintf ROMlib_checking_fprintf

namespace Executor
{
extern void stdio_smashage_init(void);
extern void stdio_smashage_compare(void);
extern int ROMlib_checking_fprintf(FILE *stream, const char *format, ...);
}
#endif

#endif /* !_STDIO_SMASHAGE_H_ */

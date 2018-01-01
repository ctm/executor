#if !defined(_OS_LINUX_H_)
#define _OS_LINUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dirent.h>
#include <sys/vfs.h>
#include <sys/param.h>
#include <sys/errno.h>

#undef trap
#include <signal.h>
#define trap

#if !defined(__GLIBC__) || __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 1)

#undef GLIBC_DB1_INCLUDES_NDBM

#else

#define GLIBC_DB1_INCLUDES_NDBM
#include <stdint.h>

#endif

#if !defined(LINUX)
#define LINUX
#endif

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#if !defined(PRIVATE)
#define PRIVATE static
#endif

#if !defined(REINSTALL_SIGNAL_HANDLER)
/* define `REINSTALL_SIGNAL_HANDLER' if signal handlers are
   de-installed after the signals occur, and require reinstallation */
#define REINSTALL_SIGNAL_HANDLER
#endif /* !REINSTALL_SIGNAL_HANDLER */

/* These functions don't exist in the math library, so use some
 * approximately correct versions of our own.
 */
#define NEED_SCALB
#define NEED_LOGB

#define HAVE_MMAP

/*
 * In the bad old days we could allocate page 0 and use it and not have
 * to offset memory.  These days that's rarely allowed, and machines are
 * fast enough that it doesn't matter.
 */

#define CONFIG_OFFSET_P 1 /* don't normally use offset memory */

extern int ROMlib_launch_native_app(int n_filenames, char **filenames);

/* #if !defined (GLIBC_DB1_INCLUDES_NDBM) */
/* #include <ndbm.h> */
/* #else */
/* #include <db1/ndbm.h> */
/* #endif */
/* to be fixed, 12/08/03 */

#define DB_DBM_HSEARCH 1
#include <db.h>

#endif /* !_OS_LINUX_H_ */

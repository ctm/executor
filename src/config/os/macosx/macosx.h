#if !defined(__OS_MACOSX_H_)
#define __OS_MACOSX_H_

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

#if !defined(MACOSX)
#define MACOSX
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

#define CONFIG_OFFSET_P 1 /* Use offset memory, at least for the first port */

extern int ROMlib_launch_native_app(int n_filenames, char **filenames);

/* #if !defined (GLIBC_DB1_INCLUDES_NDBM) */
/* #include <ndbm.h> */
/* #else */
/* #include <db1/ndbm.h> */
/* #endif */
/* to be fixed, 12/08/03 */

// DO NOT COMMIT THESE UNCONDITIONAL HACKS
// #define	DB_DBM_HSEARCH	1
// #include <db.h>

#include <ndbm.h>

#if !defined(COMPILE_FOR_BUILD)

extern void _dbm_fetch(datum *datump, DBM *db, datum datum);
extern void _dbm_firstkey(datum *datump, DBM *db);
extern void _dbm_nextkey(datum *datump, DBM *db);

#define DBM_FETCH(datump, db, datum) _dbm_fetch((datump), (db), (datum))
#define DBM_FIRSTKEY(datump, db) _dbm_firstkey((datump), (db))
#define DBM_NEXTKEY(datump, db) _dbm_nextkey((datump), (db))

#endif

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/disk.h>

#endif /* !defined(__OS_MACOSX_H_) */

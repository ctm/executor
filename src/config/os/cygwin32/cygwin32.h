#if !defined(_OS_CYGWIN32_H_)
#define _OS_CYGWIN32_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include "win_screen.h"

/* #include <ndbm.h> */
/* #include <sys/vfs.h> */
/* #include <sys/param.h> */

/* win32 headers define NULL as 0, which we don't want.  That definition
 * causes CLC(NULL) to generate warnings in some places.
 */
#undef NULL
#define NULL ((void *)0)

#if !defined(CYGWIN32)
#define CYGWIN32
#endif

#if !defined(static)

#endif

typedef struct
{
    char *dptr;
    unsigned dsize;
} datum;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.4426950408889634074
#endif

typedef unsigned short uid_t;
typedef unsigned short gid_t;

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES) || !defined(_WINSOCK_H)
/*
  time structures
  */
struct timeval
{
    long tv_sec; /* seconds */
    long tv_usec; /* microseconds */
};

#define _WINSOCK_H /* so we don't get another timeval declaration */

#endif

struct timezone
{
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime; /* type of dst correction */
};

#define pclose(p) (-1)
#define MAXPATHLEN (260 - 1)

#define CONFIG_OFFSET_P 1 /* Mac memory is always offset */

/* Specify which extra functions we need in float.h. */
#define NEED_RINT
#define NEED_LOGB
#define NEED_SCALB
#define NEED_LOG1P

extern int geteuid(void);
extern int Timer32_Init(void);

#define ST_INO(buf)                                             \
    ({                                                          \
        struct stat tmp;                                        \
                                                                \
        tmp = buf;                                              \
        ((uint16_t)tmp.st_ino << 16) | ((uint16_t)tmp.st_rdev); \
    })

extern int ROMlib_set_realmodecd(int value);
extern int ROMlib_launch_native_app(int n_filenames, char **filenames);

#endif /* !_OS_CYGWIN32_H_ */

#if !defined(_OS_MSDOS_H_)
#define _OS_MSDOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <sys/param.h>

/* djgpp headers define NULL as 0, which we don't want.  That definition
 * causes CLC(NULL) to generate warnings in some places.
 */
#undef NULL
#define NULL ((void *)0)

#if !defined(MSDOS)
#define MSDOS
#endif

#if !defined(GO32)
#define GO32
#endif

#include "rsys/types.h"

typedef struct
{
    char *dptr;
    uint32_t dsize;
} datum;

/* Specify which extra functions we need in float.h. */
#define NEED_RINT
#define HAVE_LOG2
#define NEED_LOGB
#define NEED_SCALB
#define NEED_LOG1P

extern void msdos_print_info(void);
extern bool msdos_check_memory_remaining(unsigned long desired_bytes);

/* Indicate that we should sbrk memory that will never be freed, rather
 * than mallocing it.  This is solely an efficiency issue.  This is a
 * win since djgpp's malloc rounds everything up to the next power of
 * 2 in size.
 */
#define SBRK_PERMANENT_MEMORY

extern void switch_to_non_moving_sbrk(void);

#define CONFIG_OFFSET_P 0 /* don't normally offset Mac memory */

#endif /* !_OS_MSDOS_H_ */

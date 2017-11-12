#if !defined(_NEXT_H_)
#define _NEXT_H_

/* We need this hack to avoid appkit.h including db/db.h, which conflicts
 * with ndbm.h.  NeXT says not to use db.h after NEXTSTEP 2.0, but
 * ends up #including it anyway from appkit/appkit.h!
 */
#define _DB_INCLUDED_

#include <libc.h>
#include <errno.h>
#include <unistd.h>
#include <ndbm.h>
#include <sys/dir.h>
#include <dev/disk.h>

/* Use struct direct, not struct dirent. */
#define USE_STRUCT_DIRECT

/* Use BSD signal calls, not newer POSIX stuff. */
#define USE_BSD_SIGNALS

#if !defined(S_ISDIR)
#define S_ISDIR(mode) (((mode) & (_S_IFMT)) == (_S_IFDIR))
#endif

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ndbm.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <dev/disk.h>
#endif

#if !defined(NEXT)
#define NEXT
#endif

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#if !defined(PRIVATE)
#define PRIVATE static
#endif

/* NEXTSTEP only works on i486's and better. */
#if defined(i386) && !defined(ALWAYS_ON_I486)
#define ALWAYS_ON_I486
#endif

/* Don't ask.  This is historical brain-damage. */
#if !defined(SUN)
#define SUN
#endif

#define BOOLEAN_T_TYPEDEFED /* NeXT does this for us. */

#define TRY_TO_MMAP_ZONES
extern void *mmap_permanent_memory(unsigned long amount_wanted);

#if defined(OPENSTEP) && defined(STRICT_OPENSTEP)
#if defined(m68k)
/* Currently on m68k systems we use the native CPU,
   so we can't offset memory.  I doubt this will ever change. */
#error STRICT_OPENSTEP conflicts with m68k
#endif
#define CONFIG_OFFSET_P 1
#else
#define CONFIG_OFFSET_P 0
#endif

#endif /* !_NEXT_H_ */

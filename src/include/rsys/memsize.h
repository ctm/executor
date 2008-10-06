#if !defined (_RSYS_MEMSIZE_H_)
#define _RSYS_MEMSIZE_H_

#define MIN_APPLZONE_SIZE	(512 * 1024)
#define MAX_APPLZONE_SIZE	(2047 * 1024 * 1024)
#define DEFAULT_APPLZONE_SIZE	(2 * 1024 * 1024)

#define MIN_SYSZONE_SIZE	(128 * 1024)
#define MAX_SYSZONE_SIZE	(2047 * 1024 * 1024)

#if !defined (powerpc)
#define DEFAULT_SYSZONE_SIZE	(512 * 1024)
#else

/* 681k is minimum for Photoshop 5.5 to not complain */

#define DEFAULT_SYSZONE_SIZE	(768 * 1024)
#endif

#define MIN_STACK_SIZE		(64 * 1024)
#define MAX_STACK_SIZE		(2047 * 1024 * 1024)
#define DEFAULT_STACK_SIZE	(256 * 1024)

#endif /* !_RSYS_MEMSIZE_H_ */

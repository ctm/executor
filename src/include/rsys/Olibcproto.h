#if	!defined(__OLIBCPROTO__)
#define	__OLIBCPROTO__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Olibcproto.h 63 2004-12-24 18:19:43Z ctm $
 */

#include <MacTypes.h>

#if	defined(BINCOMPAT)	&& 0
#include	<rsys/pragmal.h>
#endif	/* BINCOMPAT */

#ifdef	SPARC
#include	<alloca.h>
#endif	/* SPARC */

#include	<stdio.h>

#ifndef		ALREADYSEENSYSTYPES
#include	<sys/types.h>
#endif

#include	<sys/stat.h>

#if	!defined(__alpha)
#include	<sys/vfs.h>
#else
#include	<sys/mount.h>
#include	<termio.h>
#endif

#ifndef		SYSV
#include	<sys/time.h>
#else		/* SYSV */
#include	<time.h>
#endif		/* SYSV */

#include	<sys/param.h>

#if 0
#include	<sys/dir.h>
#endif

#if	!defined(MSDOS)
#include	<ndbm.h>
#else	/* defined(MSDOS) */
typedef struct {
    char *dptr;
    int dsize;
} datum;
#endif	/* defined(MSDOS) */

#include	<errno.h>
#include	<sys/file.h>

#if		defined(SUN) || defined(LINUX)
#include	<sys/resource.h>
#include	<sys/wait.h>
#endif

#if		defined(GO32)
#include	<../include/dos.h>
#endif

#if !defined (SYSV) && !defined (__alpha) && !defined(MSDOS) && !defined (LINUX)
#include	<sys/ttydev.h>
#endif /* !SYSV && !__alpha && !MSDOS && !LINUX */

#if	defined(__alpha) || defined(GO32)
extern void sync(void);
#endif

#if	defined(__alpha)
#include	<unistd.h>
#include	<string.h>
extern void bzero(void *str, int len);
extern void bcopy(void *src, void *dst, int len);
extern int gethostname(char *addr, int len);
extern int statfs(const char *path, struct statfs *buf, int length);
extern void *alloca(size_t size);
extern int setuid(uid_t uid);
extern int seteuid(uid_t uid);

#if !defined (XTC)
extern pid_t wait3(union wait *statusp, int options, struct rusage *resp);
extern pid_t wait4(pid_t process_id, union wait *statusp, int options,
							  struct rusage *resp);
#endif /* !XTC */

extern char *getwd(char *pathname);
extern int ioctl(int d, unsigned long request, void *argp);
#endif

#include	<dirent.h>
#ifdef		SYSV
#include	<fcntl.h>
#include	<sys/signal.h>
#include	<unistd.h>
#define		MAXPATHLEN	1024

struct	timeval {
    LONGINT tv_sec;
    LONGINT tv_usec;
};

struct itimerval {
    struct timeval it_interval;
    struct timeval it_value;
};

#define	ITIMER_REAL	0

struct sigcontext {
    int foo;
};

#define sigmask(n) (1L << ((n)-1))

#endif		/* SYSV */

#if	defined(NEXT)

#if	0 && defined(mc68000) && (!defined(NX_CURRENT_COMPILER_RELEASE) || \
		(NX_CURRENT_COMPILER_RELEASE < NX_COMPILER_RELEASE_3_0))

#include	<mach.h>
#include	<sys/loader.h>
#include	<nextdev/disk.h>
#include	<nextdev/scsireg.h>

#else

#if !defined (STRICT_OPENSTEP)
#include	<mach/mach.h>
/* define _ARCHITECTURE_BYTE_ORDER_H_ so that <mach-o/loader.h>
   doesn't include <architecture/byte_order.h>, which results in
   _many_ NSxxx swap functions ending up in ROMlib non-optimized
   objects.  this depends on the fact that we don't use MH_CIGAM,
   which is defined in terms of NXSwapInt */
#define _ARCHITECTURE_BYTE_ORDER_H_
#include	<mach-o/loader.h>
#endif

#include	<bsd/dev/disk.h>
#include	<bsd/dev/scsireg.h>

#endif

/* #include	<nextdev/fd_extern.h> */
#include        <appkit/publicWraps.h>
#include        <dpsclient/event.h>

#endif

#if	!defined(MSDOS)
#include	<sys/ioctl.h>
#endif

#include	<ctype.h>
#include	<math.h>
#include	<pwd.h>

#if !defined (LINUX)
  
#ifndef	__STDC__

extern int _flsbuf();
extern void abort();
extern int access();
extern char *alloca();
extern double atan2();
extern int atoi();
extern void bcopy();
extern char *brk();
extern void bzero();
extern int chdir();

#ifndef	AVIION
extern int chmod();
#endif	/* AVIION */

extern int close();

#ifndef	SYSV
extern void closedir();
#else	/* SYSV */
extern int closedir();
#endif	/* SYSV */

extern double cos();
extern int creat();
extern void dbm_close();
extern int dbm_delete();
extern datum dbm_fetch();
extern datum dbm_firstkey();
extern datum dbm_nextkey();
extern int dbm_store();
extern DBM *dbm_open();
extern int dup();
extern int execlp();
extern int execl();
extern void exit();
extern int flock();
extern int fork();
extern int fprintf();
extern LONGINT free();
extern int fstat();
extern int ftruncate();
extern char *getenv();
extern int gethostname();
extern int getpid();
extern struct passwd *getpwuid();
extern int gettimeofday();
extern int geteuid();
extern int getuid();
extern char *getwd();
extern struct tm *gmtime();
extern char *index();
extern int ioctl();
extern int kill();
extern struct tm *localtime();
extern int lockf();
extern LONGINT lseek();
extern int lstat();
extern char *malloc();
extern char *memcpy();

#ifndef	AVIION
extern int mkdir();
#endif	/* AVIION */

extern int open();
extern DIR *opendir();
extern int pipe();
extern int printf();
extern int qsort();
extern int rand();
extern int read();

#ifndef	SYSV
extern struct direct *readdir();
#else	/* SYSV */
extern struct dirent *readdir();
#endif	/* SYSV */

extern char *realloc();
extern int rename();
extern char *rindex();
extern int rmdir();
extern char *sbrk();
extern LONGINT setitimer();
extern int settimeofday();
extern int setuid();
extern LONGINT sigblock();
extern int (*signal())();
extern int sigpause();
extern LONGINT sigsetmask();
extern double sin();

#if	!defined(AVIION) && !defined(VAX)
#ifndef	MACBLITZ
extern char *sprintf();
#endif	/* MACBLITZ */
#endif	/* AVIION */

extern double sqrt();

#if	!defined(VAX)
extern int stat();
#endif	/* !defined(VAX) */

extern int statfs();

extern int strcmp();
extern char *strchr();
extern char *strcpy();
extern int strlen();
extern int strncmp();
extern char *strncpy();
extern void sync();
extern LONGINT syscall();
extern LONGINT time();

#ifndef	AVIION
extern int umask();
#endif	/* AVIION */

extern int unlink();
extern int utimes();
extern char *valloc();
extern int wait3();
extern int write();

#else	/* __STDC__ */

#if	!defined(__alpha) && !defined(NEXT)

extern LONGINT getuid( void );

#ifndef	SYSV
extern LONGINT settimeofday( const struct timeval *tp,
						  const struct timezone *tzp );
#endif	/* SYSV */

#endif

/*
 * NOTE: the next two routines aren't available under Mach (on the NeXT)
 *	 (on the NeXT we define them in mman.c).
 */

#if	!defined(MSDOS) && !defined(__alpha)

extern char *brk(char *addr);

extern char *sbrk( LONGINT incr );

#ifndef	NEXT
extern LONGINT _flsbuf( ULONGINT c, FILE *p );

extern void abort( void );

extern LONGINT access( const char *path, LONGINT mode );

#if	!defined(SUN)
extern void *alloca( LONGINT size );
#endif	!defined(SUN)

extern double atan2( double x, double y );

extern int atoi( const char *numstring );

extern void bcopy( const void *src, void *dst, LONGINT length );

extern void bzero( void *p, LONGINT length );

extern LONGINT chdir( const char *path );

#ifndef	AVIION
extern LONGINT chmod( const char *path, LONGINT mode );
#endif	/* AVIION */

extern LONGINT close( LONGINT fd );

#if defined (SYSV)
extern int closedir( DIR *dirp );
#else /* !SYSV */
extern void closedir( DIR *dirp );
#endif /* !SYSV */

extern double cos( double angle );

extern LONGINT creat( const char *name, LONGINT mode );

extern void dbm_close( DBM *dbp );

extern int dbm_delete( DBM *db, datum key );

extern datum dbm_fetch( DBM *db, datum key );

extern datum dbm_firstkey( DBM *db );

extern datum dbm_nextkey( DBM *db );

extern int dbm_store( DBM *db, datum key, datum contents, LONGINT flags );

extern DBM *dbm_open( const char *name, LONGINT flags, LONGINT mode );

extern LONGINT dup( LONGINT oldfd );

extern LONGINT execl( char *name, char *arg0, ... );

extern LONGINT execlp( const char *name, const char *arg0, ...);

extern void exit( LONGINT status );

extern LONGINT flock( LONGINT fd, LONGINT operation );

extern LONGINT fork( void );

#ifndef	SYSV
extern int fprintf(FILE *stream, const char *format, ...);
#endif	/* SYSV */

extern LONGINT free( void *ptr );

extern LONGINT fstat( LONGINT fd, struct stat *buf );

extern LONGINT ftruncate(LONGINT fd, ULONGINT length);

extern char *getenv( const char *name );

extern LONGINT gethostname( char *name, LONGINT len );

extern LONGINT getpid( void );

extern struct passwd *getpwuid( LONGINT uid );

#ifndef	SYSV
extern LONGINT gettimeofday( struct timeval *dp, struct timezone *tzp );
#endif	/* SYSV */

extern LONGINT geteuid( void );

extern char *getwd( char pathname[MAXPATHLEN] );

extern struct tm *gmtime( LONGINT *clock );

extern LONGINT ioctl( LONGINT d, LONGINT request, void *argp );

extern LONGINT kill( LONGINT pid, LONGINT sig );

extern char *index( const char *str, LONGINT c );

extern struct tm *localtime( LONGINT *clock );

extern LONGINT lockf( LONGINT fd, LONGINT cmd, LONGINT size );

extern LONGINT lseek( LONGINT d, LONGINT offset, LONGINT whence );

extern LONGINT lstat( const char *path, struct stat *sbufp );

extern void *malloc( ULONGINT size );

#if	!defined(SUN) && !defined(X)
extern char *memcpy( void *dst, const void *src, LONGINT len );
#endif

#ifndef	AVIION
extern LONGINT mkdir( const char *path, LONGINT mode);
#endif	/* AVIION */

extern LONGINT open( const char *path, LONGINT flags, ... );

extern DIR *opendir( const char *filename );

extern LONGINT pipe( LONGINT fildes[2] );

#ifndef	SYSV
extern int printf(const char *format, ...);
#endif	/* SYSV */

extern LONGINT qsort(void *base, LONGINT nel, LONGINT width, LONGINT (*compar)());

extern LONGINT rand( void );

extern LONGINT read( LONGINT fd, const void *buf, LONGINT nbytes );

#ifndef	SYSV
extern struct direct *readdir( DIR *dirp );
#else	/* SYSV */
extern struct dirent *readdir( DIR *dirp );
#endif	/* SYSV */

extern void *realloc( void *ptr, ULONGINT size );

extern LONGINT rename( const char *from, const char *to );

extern char *rindex( const char *str, LONGINT c );

extern LONGINT rmdir( const char *path );

extern LONGINT setitimer( LONGINT which, const struct itimerval *value,
						    struct itimerval *ovalue );


extern LONGINT setuid( LONGINT uid );

extern LONGINT sigblock( LONGINT mask );

#ifndef	SYSV
extern int
    (*signal( LONGINT sig,  LONGINT (*func)(LONGINT signo, LONGINT code,
						  struct sigcontext *scp) ))();
#else	/* SYSV */
extern int (*signal( LONGINT sig,  LONGINT (*func)(LONGINT signo, LONGINT code) ))();
#endif	/* SYSV */

extern LONGINT sigpause( LONGINT mask );

extern LONGINT sigsetmask( LONGINT mask );

extern double sin( double angle );

#ifndef	SYSV
extern char *sprintf(char *str, const char *format, ...);
#endif	/* SYSV */

extern double sqrt( double d );

#if	!defined(VAX)
extern LONGINT stat( const char *path, struct stat *sbufp );
#endif	/* !defined(VAX) */

extern LONGINT statfs( const char *path, struct statfs *sbufp );

extern char *strchr( const char *str, LONGINT c );

extern LONGINT strcmp( const char *str1, const char *str2 );

extern char *strcpy( char *dst, const char *src );

extern LONGINT strlen( const char *str );

extern LONGINT strncmp( const char *str1, const char *str2, LONGINT len );

extern char *strncpy( char *dst, const char *src, LONGINT n );

extern void sync( void );

extern LONGINT syscall( LONGINT callno, ... );

extern LONGINT time( LONGINT *tloc );

#ifndef	AVIION
extern LONGINT umask( LONGINT mask );
#endif	/* AVIION */

extern LONGINT unlink( const char *path );

extern LONGINT utimes( const char *file, struct timeval tvp[2] );

extern void *valloc( unsigned size );

extern LONGINT wait3( union wait *status, LONGINT options, struct rusage *rusage );

extern LONGINT write( LONGINT fd, void *buf, LONGINT nbytes );

#else	/* NEXT */
#include	<libc.h>
/* #include	<pwd.h> */

#endif	/* NEXT */

#endif	/* !defined(MSDOS) */

#endif	/* __STDC__ */

#else /* LINUX */

#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#if defined (DEBUG_MALLOC)
#include <dmalloc.h>
#endif

#endif /* LINUX */

#if	defined(BINCOMPAT) && 0
#include	<rsys/pragmas.h>
#endif	/* BINCOMPAT */

#endif /* !__OLIBCPROTO__ */

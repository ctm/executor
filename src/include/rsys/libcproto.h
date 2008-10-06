#if	!defined(__LIBCPROTO__)
#define	__LIBCPROTO__

/*
 * Copyright 1990, 1991 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: libcproto.h 63 2004-12-24 18:19:43Z ctm $
 */


#if	defined(NEXTSTEP) || defined(MSDOS) || defined(SUN) || defined(LINUX)

#if	defined(GO32)

typedef	unsigned short ushort;

#include <sys/time.h>

struct itimerval {
  struct timeval it_interval;
  struct timeval it_value;
};

extern int gettimeofday (struct timeval *tp, struct timezone *tzp);
extern void protected_gettimeofday (struct timeval *, struct timezone *);

#endif /* GO32 */

#include <rsys/Olibcproto.h>

#if	!defined(O_BINARY)
#define	O_BINARY 0
#endif	/* !defined(O_BINARY) */

#define Udirect_t	struct direct
#define Udirent_t	struct dirent
#define Utimeval_t	struct timeval
#define Uitimerval_t	struct itimerval
#define Upasswd_t	struct passwd
#define Urusage_t	struct rusage
#define Usigcontext_t	struct sigcontext
#define Ustat_t		struct stat
#define Ustatfs_t	struct statfs
#define Utimezone_t	struct timezone
#define Utm_t		struct tm
#define Uwait_t		union wait
#define UDBM_t		DBM
#define Udatum_t	datum
#define UFILE_t		FILE
#define UDIR_t		DIR
#define Usockaddr_t	struct sockaddr
#define Udrive_info_t	struct drive_info
#define Umach_header_t	struct mach_header
#define Uload_command_t	struct load_command
#define Usegment_command_t	struct segment_command
#define Usgttyb_t	struct sgttyb
#define Utchars_t	struct tchars
#define Usigstack_t	struct sigstack
#define Usigvec_t	struct sigvec

#if	!defined(MSDOS)

#define	Uaccess access
#define	Uchdir chdir
#define	Uchmod chmod
#define	Uchown chown
#define	Ucreat creat
#define	Ufopen fopen
#define	Ulink link
#define	Ulstat lstat
#define	Umkdir mkdir
#define	Uopen open
#define	Uopendir opendir
#define	Urename rename
#define	Urmdir rmdir
#define	Ustat stat
#define	Ustatfs statfs
#define	Uunlink unlink
#define	Uutimes utimes

#else	/* !defined(MSDOS) */

extern int Uaccess(const char *path, int mode);
extern int Uchdir(const char *path);
extern int Uchmod(const char *path, int mode);
extern int Uchown(const char *path, uid_t owner, gid_t group);
extern int Ucreat(const char *path, int mode);
extern FILE *Ufopen(const char *path, const char *type);
extern int Ulink(const char *path, const char *newpath);
extern int Umkdir(const char *path, int mode);
extern int Uopen(const char *path, int flags, int mode);
extern DIR *Uopendir(const char *path);
extern int Urename(const char *path, const char *new);
extern int Urmdir(const char *path);
extern int Ustat(const char *path, struct stat *buf);
extern int Ustatfs(const char *path, struct statfs *buf);
extern int Uunlink(const char *path);
extern int Uutimes(const char *path, struct timeval tvp[2]);

#endif	/* defined(MSDOS) */

#define	Uabort abort
#define	Uaccept accept
#define	Uatan2 atan2
#define	Uatoi atoi
#define	Ubcopy bcopy
#define	Ubind bind
#define	Ubrk brk
#define	Ubzero bzero
#define	Uclose close
#define	Ucos cos
#define	Udbm_close dbm_close
#define	Udbm_delete dbm_delete
#define	Udbm_fetch dbm_fetch
#define	Udbm_firstkey dbm_firstkey
#define	Udbm_nextkey dbm_nextkey
#define	Udbm_store dbm_store
#define	Udbm_open dbm_open
#define	Udup dup
#define	Uexeclp execlp
#define	Uexecl execl
#define	Uexit exit
#define	Ufclose fclose
#define	Ufcntl fcntl
#define	Ufflush fflush
#define	Ufileno fileno
#define	Uflock flock
#define	Ufork fork
#define	Ufprintf fprintf
#define	Ufread fread
#define	Ufree free
#define	Ufseek fseek
#define	Ufstat fstat
#define	Ufsync fsync
#define	Uftruncate ftruncate
#define	Ufwrite fwrite
#define	Ugetenv getenv
#define	Ugethostname gethostname
#define	Ugetpid getpid
#define	Ugetpwuid getpwuid
#define	Ugettimeofday gettimeofday
#define	Ugeteuid geteuid
#define	Ugetuid getuid
#define	Ugetwd getwd
#define	Ugmtime gmtime
#define	Uindex index
#define	Uioctl ioctl
#define	Ukill kill
#define	Ulisten listen
#define	Ulocaltime localtime
#define	Ulockf lockf
#define	Ulseek lseek
#define	Umalloc malloc
#define	Umemcpy memcpy
#define	Upipe pipe
#define	Uprintf printf
#define	Uqsort qsort
#define	Urand rand
#define	Uread read
#define	Urealloc realloc
#define	Urindex rindex
#define	Usbrk sbrk
#define	Usscanf sscanf
#define	Usetitimer setitimer
#define	Usetpgrp setpgrp
#define	Usettimeofday settimeofday
#define	Usetuid setuid
#define	Usigblock sigblock
#define	Usigmask sigmask
#define	Usigstack sigstack
#define	Usignal signal
#define	Usigpause sigpause
#define	Usigsetmask sigsetmask
#define	Usigvec sigvec
#define	Usin sin
#define	Usocket socket
#define	Usprintf sprintf
#define	Usqrt sqrt
#define	Ustrcmp strcmp
#define	Ustrchr strchr
#define	Ustrcpy strcpy
#define	Ustrlen strlen
#define Ustrncmp strncmp
#define	Ustrncpy strncpy
#define	Usync sync
#define	Usyscall syscall
#define	Utime time
#define	Uumask umask
#define	Uvalloc valloc
#define	Uwait3 wait3
#define	Uwait4 wait4
#define Uwrite write
#define	Uclosedir closedir
#define	Ureaddir readdir

#else /* NEXTSTEP || MSDOS || SUN || LINUX */

#include	<rsys/assert.h>	/* GNU assert produces warning messages */
#include	"libcconsts.h"

extern	LONGINT	errno;

typedef	unsigned short ushort;

/*
 * NOTE: In the typedefs below, we only need to include the fields that
 *	 we use.  NOTE also, in most cases the fields should be longs so
 *	 we won't have to worry about padding.
 *
 *	 Of course this means that our wrapper routines have to hand assign
 *	 each field...
 */

typedef struct {
    ULONGINT d_namlen	PACKED;
    char d_name[MAXNAMLEN + 1]	PACKED;
} Udirect_t;

typedef struct {
    ULONGINT d_namelen	PACKED;
    char d_name[MAXNAMLEN + 1]	PACKED;
} Udirent_t;

typedef struct {
    LONGINT tv_sec	PACKED;
    LONGINT tv_usec	PACKED;
} Utimeval_t;

typedef struct {
    Utimeval_t it_interval	PACKED;
    Utimeval_t it_value	PACKED;
} Uitimerval_t;

typedef struct {
    char *pw_dir	PACKED;
} Upasswd_t;

typedef struct {	/* NOTE: we really don't use any fields of this */
} Urusage_t;

typedef struct {	/* NOTE: the only place we use this is in a .m file */
} Usigcontext_t;

typedef struct {
    ULONGINT st_ino	PACKED;
    ULONGINT st_mode	PACKED;
    ULONGINT st_uid	PACKED;
    ULONGINT st_size	PACKED;
    ULONGINT st_atime	PACKED;
    ULONGINT st_mtime	PACKED;
    ULONGINT st_ctime	PACKED;
} Ustat_t;

typedef struct {
    LONGINT f_bsize	PACKED;
    LONGINT f_blocks	PACKED;
    LONGINT f_bfree	PACKED;
    LONGINT f_bavail	PACKED;
    LONGINT f_files	PACKED;
} Ustatfs_t;

typedef struct {
    LONGINT tz_minuteswest	PACKED;
    LONGINT tz_dsttime	PACKED;
} Utimezone_t;

typedef struct {
    LONGINT tm_sec	PACKED;
    LONGINT tm_min	PACKED;
    LONGINT tm_hour	PACKED;
    LONGINT tm_mday	PACKED;
    LONGINT tm_mon	PACKED;
    LONGINT tm_year	PACKED;
    LONGINT tm_wday	PACKED;
    LONGINT tm_yday	PACKED;
} Utm_t;

typedef union {	/* NOTE: just like rusage */
} Uwait_t;

typedef struct {	/* NOTE: we don't use any fields of this puppy */
} *UDBM_t;

typedef struct {
    char *dptr	PACKED;
    LONGINT dsize	PACKED;
} Udatum_t;

typedef struct {	/* NOTE: we don't use any fields of this puppy */
} *UFILE_t;

typedef struct {	/* NOTE: another one we don't peek inside */
} UDIR_t;

typedef struct {
    ULONGINT sa_family	PACKED;
    char sa_data[14]	PACKED;
} Usockaddr_t;

typedef struct {
    char di_name[MAXDNMLEN]	PACKED;
    LONGINT di_label_blkno[NLABELS]	PACKED;
    LONGINT di_devblklen	PACKED;
    LONGINT di_maxbcount	PACKED;
} Udrive_info_t;

typedef struct {
    ULONGINT magic	PACKED;
    ULONGINT ncmds	PACKED;
} Umach_header_t;

typedef struct {
    ULONGINT cmd	PACKED;
    ULONGINT cmdsize	PACKED;
} Uload_command_t;

typedef struct {
    ULONGINT cmd	PACKED;
    ULONGINT cmdsize	PACKED;
    char segname[16]	PACKED;
    ULONGINT vmaddr	PACKED;
    ULONGINT vmsize	PACKED;
    ULONGINT fileoff	PACKED;
} Usegment_command_t;

typedef struct {
    char sg_ispeed	PACKED;
    char sg_ospeed	PACKED;
    char sg_erase	PACKED;
    char sg_kill	PACKED;
    char sg_flags	PACKED;
} Usgttyb_t;

typedef struct {
    char t_intrc	PACKED;
    char t_quitc	PACKED;
    char t_startc	PACKED;
    char t_stopc	PACKED;
    char t_eofc	PACKED;
    char t_brkc	PACKED;
} Utchars_t;

typedef struct {
    char *ss_sp	PACKED;
    LONGINT ss_onstack	PACKED;
} Usigstack_t;

typedef struct {
    void (*sv_handler)()	PACKED;
    LONGINT sv_mask	PACKED;
    LONGINT sv_flags	PACKED;
} Usigvec_t;

#define	MIN(a, b)	((a) < (b) ? (a) : (b))

#ifndef	__STDC__

extern void *alloca();

extern void Uabort();
extern LONGINT Uaccept();
extern int Uaccess();
extern double Uatan2();
extern int Uatoi();
extern void Ubcopy();
extern LONGINT Ubind();
extern char *Ubrk();
extern void Ubzero();
extern int Uchdir();
extern int Uchmod();
extern LONGINT Uchown();
extern int Uclose();
extern double Ucos();
extern int Ucreat();
extern void Udbm_close();
extern int Udbm_delete();
extern Udatum_t Udbm_fetch();
extern Udatum_t Udbm_firstkey();
extern Udatum_t Udbm_nextkey();
extern int Udbm_store();
extern UDBM_t *Udbm_open();
extern int Udup();
extern int Uexeclp();
extern int Uexecl();
extern void Uexit();
extern LONGINT Ufclose();
extern LONGINT Ufcntl();
extern LONGINT Ufflush();
extern LONGINT Ufileno();
extern int Uflock();
extern UFILE_t *Ufopen();
extern int Ufork();
extern int Ufprintf();
extern LONGINT Ufread();
extern LONGINT Ufree();
extern LONGINT Ufseek();
extern int Ufstat();
extern LONGINT Ufsync();
extern int Uftruncate();
extern LONGINT Ufwrite();
extern char *Ugetenv();
extern int Ugethostname();
extern int Ugetpid();
extern Upasswd_t *Ugetpwuid();
extern int Ugettimeofday();
extern int Ugeteuid();
extern int Ugetuid();
extern char *Ugetwd();
extern Utm_t *Ugmtime();
extern char *Uindex();
extern int Uioctl();
extern int Ukill();
extern LONGINT Ulink();
extern LONGINT Ulisten();
extern Utm_t *Ulocaltime();
extern int Ulockf();
extern LONGINT Ulseek();
extern int Ulstat();
extern char *Umalloc();
extern char *Umemcpy();
extern int Umkdir();
extern int Uopen();
extern UDIR_t *Uopendir();
extern int Upipe();
extern int Uprintf();
extern int Uqsort();
extern int Urand();
extern int Uread();
extern char *Urealloc();
extern int Urename();
extern char *Urindex();
extern int Urmdir();
extern char *Usbrk();
extern LONGINT Usscanf();
extern LONGINT Usetitimer();
extern LONGINT Usetpgrp();
extern int Usettimeofday();
extern int Usetuid();
extern LONGINT Usigblock();
extern LONGINT Usigmask();
extern LONGINT Usigstack();
extern int (*Usignal())();
extern int Usigpause();
extern LONGINT Usigsetmask();
extern LONGINT Usigvec();
extern double Usin();
extern LONGINT Usocket();
extern char *Usprintf();
extern double Usqrt();
extern int Ustat();
extern int Ustatfs();
extern int Ustrcmp();
extern char *Ustrchr();
extern char *Ustrcpy();
extern int Ustrlen();
extern int Ustrncmp();
extern char *Ustrncpy();
extern void Usync();
extern LONGINT Usyscall();
extern LONGINT Utime();
extern int Uumask();
extern int Uunlink();
extern int Uutimes();
extern char *Uvalloc();
extern int Uwait3();
extern LONGINT Uwait4();
extern int Uwrite();

#ifndef	SYSV

extern void Uclosedir();
extern Udirect_t *Ureaddir();

#else	/* SYSV */

extern int Uclosedir();
extern Udirent_t *Ureaddir();

#endif	/* SYSV */

#else	/* __STDC__ */

extern void *alloca( LONGINT );

extern char *Ubrk(char *addr);

extern char *Usbrk( LONGINT incr );

extern void Uabort( void );

extern LONGINT Uaccept( LONGINT fd, Usockaddr_t *addrp, LONGINT *addrlenp );

extern LONGINT Uaccess( const char *path, LONGINT mode );

extern double Uatan2( double x, double y );

extern int Uatoi( const char *numstring );

extern void Ubcopy( const void *src, void *dst, LONGINT length );

extern LONGINT Ubind( LONGINT fd, Usockaddr_t *namep, LONGINT namelen );

extern void Ubzero( void *p, LONGINT length );

extern LONGINT Uchdir( const char *path );

extern LONGINT Uchmod( const char *path, LONGINT mode );

extern LONGINT Uchown( const char *pathp, LONGINT owner, LONGINT group );

extern LONGINT Uclose( LONGINT fd );

extern double Ucos( double angle );

extern LONGINT Ucreat( const char *name, LONGINT mode );

extern void Udbm_close( UDBM_t *dbp );

extern int Udbm_delete( UDBM_t *db, Udatum_t key );

extern Udatum_t Udbm_fetch( UDBM_t *db, Udatum_t key );

extern Udatum_t Udbm_firstkey( UDBM_t *db );

extern Udatum_t Udbm_nextkey( UDBM_t *db );

extern int Udbm_store( UDBM_t *db, Udatum_t key, Udatum_t contents,
								  LONGINT flags );

extern UDBM_t *Udbm_open( const char *name, LONGINT flags, LONGINT mode );

extern LONGINT Udup( LONGINT oldfd );

extern LONGINT Uexecl( char *name, char *arg0, ... );

extern LONGINT Uexeclp( const char *name, const char *arg0, ...);

extern void Uexit( LONGINT status );

extern LONGINT Ufclose( UFILE_t *fp );

extern LONGINT Ufcntl( LONGINT fd, LONGINT cmd, LONGINT arg );

extern LONGINT Ufflush( UFILE_t *fp );

extern LONGINT Ufileno( UFILE_t *fp );

extern LONGINT Uflock( LONGINT fd, LONGINT operation );

extern UFILE_t *Ufopen( const char *filenamep, const char *typep );

extern LONGINT Ufork( void );

extern int Ufprintf(UFILE_t *stream, const char *format, ...);

extern LONGINT Ufread( void *ptr, LONGINT size, LONGINT nittems, UFILE_t *fp );

extern LONGINT Ufree( void *ptr );

extern LONGINT Ufseek( UFILE_t *fp, LONGINT offset, LONGINT whence );

extern LONGINT Ufstat( LONGINT fd, Ustat_t *buf );

extern LONGINT Ufsync( LONGINT fd );

extern LONGINT Uftruncate(LONGINT fd, ULONGINT length);

extern LONGINT Ufwrite( void *ptr, LONGINT size, LONGINT nittems, UFILE_t *fp );

extern char *Ugetenv( const char *name );

extern LONGINT Ugethostname( char *name, LONGINT len );

extern LONGINT Ugetpid( void );

extern Upasswd_t *Ugetpwuid( LONGINT uid );

extern LONGINT Ugettimeofday( Utimeval_t *dp, Utimezone_t *tzp );

extern LONGINT Ugeteuid( void );

extern LONGINT Ugetuid( void );

extern char *Ugetwd( char pathname[] );

extern Utm_t *Ugmtime( LONGINT *clock );

extern LONGINT Uioctl( LONGINT d, LONGINT request, void *argp );

extern LONGINT Ukill( LONGINT pid, LONGINT sig );

extern char *Uindex( const char *str, LONGINT c );

extern LONGINT Ulink( const char *srcp, const char *dstp );

extern LONGINT Ulisten( LONGINT fd, LONGINT backlog );

extern Utm_t *Ulocaltime( LONGINT *clock );

extern LONGINT Ulockf( LONGINT fd, LONGINT cmd, LONGINT size );

extern LONGINT Ulseek( LONGINT d, LONGINT offset, LONGINT whence );

extern LONGINT Ulstat( const char *path, Ustat_t *sbufp );

extern void *Umalloc( ULONGINT size );

extern char *Umemcpy( void *dst, const void *src, LONGINT len );

extern LONGINT Umkdir( const char *path, LONGINT mode);

extern LONGINT Uopen( const char *path, LONGINT flags, ... );

extern UDIR_t *Uopendir( const char *filename );

extern LONGINT Upipe( LONGINT fildes[2] );

extern int Uprintf(const char *format, ...);

extern LONGINT Uqsort(void *base, LONGINT nel, LONGINT width, LONGINT (*compar)());

extern LONGINT Urand( void );

extern LONGINT Uread( LONGINT fd, const void *buf, LONGINT nbytes );

extern void *Urealloc( void *ptr, ULONGINT size );

extern LONGINT Urename( const char *from, const char *to );

extern char *Urindex( const char *str, LONGINT c );

extern LONGINT Urmdir( const char *path );

extern LONGINT Usetitimer( LONGINT which, const Uitimerval_t *value,
						 Uitimerval_t *ovalue );

extern LONGINT Usetpgrp( LONGINT pid, LONGINT pgrp );

extern LONGINT Usettimeofday( const Utimeval_t *tp, const Utimezone_t *tzp );

extern LONGINT Usetuid( LONGINT uid );

extern LONGINT Usigblock( LONGINT mask );

extern LONGINT Usigmask( LONGINT sig );

extern LONGINT Usigstack( Usigstack_t *ssp, Usigstack_t *ossp );

extern int
    (*Usignal( LONGINT sig,  LONGINT (*func)(LONGINT signo, LONGINT code,
					       Usigcontext_t *scp) ))();

extern LONGINT Usigpause( LONGINT mask );

extern LONGINT Usigsetmask( LONGINT mask );

extern LONGINT Usigvec( LONGINT sig, Usigvec_t *vecp, Usigvec_t *ovecp );

extern double Usin( double angle );

extern LONGINT Usocket( LONGINT domain, LONGINT type, LONGINT protocol );

extern char *Usprintf(char *str, const char *format, ...);

extern double Usqrt( double d );

extern LONGINT Usscanf( char *s, const char *format, ... );

extern LONGINT Ustat( const char *path, Ustat_t *sbufp );

extern LONGINT Ustatfs( const char *path, Ustatfs_t *sbufp );

extern char *Ustrchr( const char *str, LONGINT c );

extern LONGINT Ustrcmp( const char *str1, const char *str2 );

extern char *Ustrcpy( char *dst, const char *src );

extern LONGINT Ustrlen( const char *str );

extern LONGINT Ustrncmp( const char *str1, const char *str2, LONGINT len );

extern char *Ustrncpy( char *dst, const char *src, LONGINT n );

extern void Usync( void );

extern LONGINT Usyscall( LONGINT callno, ... );

extern LONGINT Utime( LONGINT *tloc );

extern LONGINT Uumask( LONGINT mask );

extern LONGINT Uunlink( const char *path );

extern LONGINT Uutimes( const char *file, Utimeval_t tvp[2] );

extern void *Uvalloc( unsigned size );

extern LONGINT Uwait3( Uwait_t *status, LONGINT options, Urusage_t *rusage );

extern LONGINT Uwait4( LONGINT pid, Uwait_t *statusp, LONGINT options, Urusage_t *rup );

extern LONGINT Uwrite( LONGINT fd, const void *buf, LONGINT nbytes );


#ifndef	SYSV

extern void Uclosedir( UDIR_t *dirp );
extern Udirect_t *Ureaddir( UDIR_t *dirp );

#else	/* SYSV */

extern int Uclosedir( UDIR_t *dirp );
extern Udirent_t *Ureaddir( UDIR_t *dirp );

#endif	/* SYSV */

#endif	/* __STDC__ */

#endif

#endif	/* !defined(__LIBCPROTO__) */

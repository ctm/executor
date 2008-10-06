#if !defined (_SLASH_H_)
#define _SLASH_H_

#if defined (MSDOS) || defined (CYGWIN32)

extern int Uaccess(const char *path, int mode);
extern int Uchdir(const char *path);
extern int Uchmod(const char *path, int mode);
extern int Uchown(const char *path, uid_t owner, gid_t group);
extern int Ucreat(const char *path, int mode);
extern FILE *Ufopen(const char *path, const char *type);
extern int Ulink(const char *path, const char *newpath);
extern int Umkdir(const char *path, int mode);
extern int Uopen(const char *path, int flags, int mode);
extern int Uclose(int fd);
extern DIR *Uopendir(const char *path);

#if !defined (CYGWIN32) /* statfs is in winfs.h for now */
extern int Ustatfs(const char *path, struct statfs *buf);
#endif

extern int Urename(const char *path, const char *new);
extern int Urmdir(const char *path);
extern int Ustat(const char *path, struct stat *buf);
extern int Uunlink(const char *path);
extern int Uutimes(const char *path, struct timeval tvp[2]);

#else /* !MSDOS && !defined (CYGWIN32) */

#define Uaccess access
#define Uchdir chdir
#define Uchmod chmod
#define Uchown chown
#define Ucreat creat
#define Ufopen fopen
#define Ulink link
#define Ulstat lstat
#define Umkdir mkdir
#define Uopendir opendir
#define Urename rename
#define Urmdir rmdir
#define Ustat stat
#define Ustatfs statfs
#define Uunlink unlink
#define Uutimes utimes

extern int Uopen(const char *path, int flags, int mode);
extern int Uclose (int fd);

#endif /* !MSDOS && !defined (CYGWIN32) */

#endif /* !_SLASH_H_ */

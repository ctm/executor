#if !defined(_winfs_h_)
#define _winfs_h_

/*
 * Copyright 1997 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

#if 0
struct dirent
{
  char d_name[FILENAME_MAX];
};

typedef struct
{
  int valid_dir_magic; /* put magic number at top where it's most likely
			  to get smashed */
  struct dirent *showme;
  struct dirent *stored;
  HANDLE private_data;
}
DIR;

enum { WINFS_SIG = 0x3301; }; /* arbitrary number */
#endif

struct statfs
{
    uint32 f_blocks;
    uint32 f_bsize;
    uint32 f_bavail;
    uint32 f_bfree;
    uint32 f_files;
};

#if !defined _DEV_T_
typedef short dev_t;
#endif

#if 0
extern int closedir (DIR *dirp);
extern DIR *Uopendir (const char *path);
extern DIR *opendir (const char *path);
extern struct dirent *readdir (DIR *dirp);
#endif

extern int Ustatfs(const char *name, struct statfs *fsp);
extern int statfs(const char *name, struct statfs *fsp);
extern int sync(void);
extern int link(const char *oldpath, const char *newpath);
extern char *getwd(char *buf);
extern int fsync(int fd);

#endif /* !defined (_winfs_h_) */

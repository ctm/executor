/*
 * Copyright 1997-2000 by Abacus Research and Development, Inc.
 * All rights reserved.
 */

/* NOTE: we have to include common.h since it sets up some macros that
   we need, including PUBLIC and PRIVATE.  However, since this is file's
   purpose is to export routines that are definitely not Mac-routines, we
   included common.h first and then include windows.h later so that we have
   access to all the Windows data types and structures. */

#define _STUBIFY_H_ /* prevent conflicts between stubify.h and windows.h */
#define BOOLEAN MAC_BOOLEAN /* we want the Windows version of these two */
#define CHAR MAC_CHAR
#include "rsys/common.h"
#undef BOOLEAN
#undef CHAR
#undef Time

#include <windows.h>
#include <stdio.h>
#include <errno.h>

#include "winfs.h"
#include "rsys/lockunlock.h"

#if 0 /* mingw32 has opendir, readdir, closedir and friends */

/* I was in the middle of cleaning Sam's code for pedantic purposes
   when I realized that mingw32 1.4 has these routines, so (since I'm
   pressed for time) I abandoned the cleaning.  The code is left here
   for Sam to look at, then I'll remove it at some future date */

#define ALL_FILES_STR "*.*"

/*
 * In general using gotos is a bad idea, although much less so in certain
 * circumstances, specifically when they're forward jumps that make up
 * for C's lack of exception handling.  Of course you have to pay very
 * close attention to how you construct and use the cleanup code at the
 * end.  In our case we make sure that the cleanup code can easily detect
 * any outstanding code that it needs to free up.
 */

PUBLIC DIR *
opendir (const char *path)
{
  HANDLE found;
  WIN32_FIND_DATA file_info;
  DIR *retval;

  retval = NULL;
  {
    char *direxp;

    direxp = malloc(strlen(path)+1+strlen(ALL_FILES_STR)+1);
    if (!direxp)
      goto OUT_OF_MEMORY;

    if (path[strlen(path)-1] == '/')
      sprintf(direxp, "%s%s", path, ALL_FILES_STR);
    else
      sprintf(direxp, "%s/%s", path, ALL_FILES_STR);

    found = FindFirstFile(direxp, &file_info);
    free(direxp);
  }

  if ( found == INVALID_HANDLE_VALUE )
    retval = NULL; /* FIXME: determine reason for failure and set errno.. */
  else
    {
      retval = malloc(sizeof *retval);
      if (!retval)
	goto OUT_OF_MEMORY;

      retval->showme = malloc(sizeof *retval->showme);
      retval->stored = malloc(sizeof *retval->stored);
      if (!retval->showme || !retval->stored)
	goto OUT_OF_MEMORY;

      {
	int dname_len;

	dname_len = sizeof retval->showme->d_name;
	strncpy(retval->showme->d_name, file_info.cFileName, dname_len - 1);
	retval->showme->d_name[dname_len - 1] = 0;
      }
      retval->private_data = found;
      retval->valid_dir_magic = WINFS_SIG;
    }
  return retval;
OUT_OF_MEMORY:
  if (retval)
    {
      if (retval->showme)
	free (retval->showme);
      if (retval->stored)
	free (retval->stored);
      free retval;
      retval = NULL;
    }
  errno = ENOMEM;
  return retval;
}

/*
 * The swapping of available dirent structs is a bit fancy and possibly
 * wasteful of memory, but it guarantees that a valid return value for
 * readdir() is contained in dirp->showme
 */

PUBLIC struct dirent *
readdir (DIR *dirp)
{
  WIN32_FIND_DATA file_info;
  struct dirent *retval;

  /* Sanity.. */
  if ( !dirp || !dirp->showme || dirp->valid_dir_magic != WINFS_SIG )
    retval = NULL;
  else
    {
      {
	struct dirent *temp;
	
	temp = dirp->showme;
	dirp->showme = dirp->stored;
	dirp->stored = temp;
      }
      retval = dirp->stored;

      /* Get the next directory entry */
      if (FindNextFile(dirp->private_data, &file_info))
	{
	  strncpy((dirp->showme)->d_name, file_info.cFileName, FILENAME_MAX);
	  (dirp->showme)->d_name[FILENAME_MAX-1] = '\0';
	}
      else
	{
	  free(dirp->showme);
	  dirp->showme = NULL;
	}
      return(retval);
    }
}

PUBLIC int
closedir (DIR *dirp)
{
  HANDLE dir_handle;

  /* Don't close a directory multiple times */
  if ( ! dirp || dirp->valid_dir_magic != WINFS_SIG )
    return(-1);

  /* Free everything... */
  dir_handle = (HANDLE)(dirp->private_data);
  FindClose(dir_handle);
  if ( dirp->showme ) {
    free(dirp->showme);
    dirp->showme = NULL;
  }
  if ( dirp->stored ) {
    free(dirp->stored);
    dirp->stored = NULL;
  }
  dirp->valid_dir_magic = 0;
  free(dirp);
  return(0);
}
#endif

#warning impotent fsync

PUBLIC int
fsync(int fd)
{
    int retval;

    retval = 0;
    return retval;
}

#warning impotent sync

PUBLIC int
sync(void)
{
    int retval;

    retval = 0;
    return retval;
}

PUBLIC char *
getwd(char *buf)
{
    char *retval;

    retval = getcwd(buf, MAXPATHLEN);
    return retval;
}

#warning link is just a spoof -- I think it will work for our purposes though
/* verify that we can spoof in all contexts that it's used */
/* serial.c and main.c */

PUBLIC int
link(const char *oldpath, const char *newpath)
{
    int retval;
    int hand;

    hand = creat(newpath, O_RDWR);
    if(hand == -1)
        retval = -1;
    else
    {
        close(hand);
        retval = 0;
    }
    return retval;
}

#warning statfs is spoofed

PUBLIC int
statfs(const char *path, struct statfs *bufp)
{
    int retval;
    DWORD sectors_per_cluster;
    DWORD bytes_per_sector;
    DWORD free_clusters;
    DWORD clusters;
    BOOL success;

    success = GetDiskFreeSpace(path, &sectors_per_cluster, &bytes_per_sector,
                               &free_clusters, &clusters);
    if(success)
    {
        bufp->f_bsize = bytes_per_sector;
        bufp->f_blocks = clusters * sectors_per_cluster;
        bufp->f_bfree = free_clusters * sectors_per_cluster;
        bufp->f_bavail = bufp->f_bfree;
        bufp->f_files = 10;
#warning f_files made up
        retval = 0;
    }
    else
    {
        warning_trace_info("path = '%s'", path);
        bufp->f_bsize = 512;
        bufp->f_blocks = 512 * 1024 * 1024 / bufp->f_bsize;
        bufp->f_bavail = 256 * 1024 * 1024 / bufp->f_bsize;
        bufp->f_bfree = 256 * 1024 * 1024 / bufp->f_bsize;
        bufp->f_files = 10;
        retval = -1;
    }

    return retval;
}

PUBLIC int
ROMlib_lockunlockrange(int fd, uint32 begin, uint32 count, lockunlock_t op)
{
    int retval;
    BOOL WINAPI (*routine)(HANDLE, DWORD, DWORD, DWORD, DWORD);

    warning_trace_info("fd = %d, begin = %d, count = %d, op = %d",
                       fd, begin, count, op);
    switch(op)
    {
        case lock:
            routine = LockFile;
            break;
        case unlock:
            routine = UnlockFile;
            break;
        default:
            warning_unexpected("op = %d", op);
            routine = 0;
            break;
    }

    if(!routine)
        retval = paramErr;
    else
    {
        BOOL success;
        HANDLE h;

        h = (HANDLE)_get_osfhandle(fd);
        success = routine(h, begin, 0, count, 0);
        if(success)
            retval = noErr;
        else
        {
            DWORD err;

            err = GetLastError();
            switch(err)
            {
                case ERROR_LOCK_VIOLATION:
                    retval = fLckdErr;
                    break;
                case ERROR_NOT_LOCKED:
                    retval = afpRangeNotLocked;
                    break;
                case ERROR_LOCK_FAILED:
                    retval = afpRangeOverlap;
                    break;
                default:
                    warning_unexpected("err = %ld, h = %p", err, h);
                    retval = noErr;
                    break;
            }
        }
    }
    return retval;
}

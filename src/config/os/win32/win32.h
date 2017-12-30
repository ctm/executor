#pragma once

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
//#include <sys/vfs.h>
#include <sys/param.h>
//#include <sys/errno.h>


#if !defined(PRIVATE)
#define PRIVATE static
#endif

#define CONFIG_OFFSET_P 1 /* Use offset memory, at least for the first port */

#define NEED_SCALB
#define NEED_LOGB

extern int ROMlib_launch_native_app(int n_filenames, char **filenames);

//#define WIN32

typedef struct
{
    char *dptr;
    unsigned dsize;
} datum;

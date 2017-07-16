#if !defined(_FILEDOUBLE_H_)
#define _FILEDOUBLE_H_

#include "FileMgr.h"

/*
 * Copyright 1991, 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: filedouble.h 63 2004-12-24 18:19:43Z ctm $
 */
namespace Executor {
typedef enum {
    Data_Fork_ID = 1,
    Resource_Fork_ID,
    Real_Name_ID,
    Comment_ID,
    BandW_ICON_ID,
    Color_ICON_ID,
    File_Dates_Info_ID = 8,
    Finder_Info_ID,
    Macintosh_File_Info_ID,
    ProDOS_File_Info_ID,
    MSDOS_File_Info_ID,
    Short_Name_ID,
    AFT_File_Info_ID,
    Directory_ID
} Single_ID;

typedef struct PACKED {
  ULONGINT id;
  ULONGINT offset;
  ULONGINT length;
} Single_descriptor;

typedef struct PACKED {
  LONGINT magic;
  LONGINT version;
  LONGINT filler[4];
  INTEGER nentries;
} Single_header;

typedef struct PACKED {
  LONGINT crdat;
  LONGINT moddat;
  LONGINT backupdat;
  LONGINT accessdat;
} Single_dates;

typedef struct PACKED {
  FInfo finfo;
  FXInfo fxinfo;
} Single_finfo;

typedef ULONGINT Single_attribs;

typedef struct PACKED defaulthead {
    Single_header head;
    Single_descriptor desc[10];	/* we use 4, 6 for spare */
} defaulthead_t;

typedef struct PACKED defaultentries {
  Single_attribs attribs;
  Single_dates   dates;
  Single_finfo   finfo;
} defaultentries_t;

#define SINGLEMAGIC	0x0051600
#define DOUBLEMAGIC	0x0051607

#define SINGLEVERSION	0x00020000

extern OSErr ROMlib_newresfork (char *name, LONGINT *fdp, boolean_t unix_p);

typedef enum { mkdir_op, rmdir_op } double_dir_op_t;

extern int afpd_conventions_p;
extern int netatalk_conventions_p;
extern char apple_double_quote_char;
extern const char *apple_double_fork_prefix;
extern int apple_double_fork_prefix_length;

extern void double_dir_op (char *name, double_dir_op_t op);
}
#endif

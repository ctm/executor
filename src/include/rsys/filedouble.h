#if !defined(_FILEDOUBLE_H_)
#define _FILEDOUBLE_H_

#include "FileMgr.h"

/*
 * Copyright 1991, 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: filedouble.h 63 2004-12-24 18:19:43Z ctm $
 */

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

typedef struct {
    ULONGINT id		PACKED;
    ULONGINT offset	PACKED;
    ULONGINT length	PACKED;
} Single_descriptor;

typedef struct {
    LONGINT magic	PACKED;
    LONGINT version	PACKED;
    LONGINT filler[4]	PACKED;
    INTEGER nentries	PACKED;
} Single_header;

typedef struct {
    LONGINT crdat	PACKED;
    LONGINT moddat	PACKED;
    LONGINT backupdat	PACKED;
    LONGINT accessdat	PACKED;
} Single_dates;

typedef struct {
    FInfo finfo	LPACKED;
    FXInfo fxinfo	LPACKED;
} Single_finfo;

typedef ULONGINT Single_attribs;


typedef struct defaulthead {
    Single_header head	LPACKED;
    Single_descriptor desc[10]	LPACKED;	/* we use 4, 6 for spare */
} defaulthead_t;

typedef struct defaultentries {
    Single_attribs attribs	PACKED;
    Single_dates   dates	LPACKED;
    Single_finfo   finfo	LPACKED;
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

#endif

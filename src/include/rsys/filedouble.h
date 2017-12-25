#if !defined(_FILEDOUBLE_H_)
#define _FILEDOUBLE_H_

#include "FileMgr.h"

/*
 * Copyright 1991, 1998 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */
namespace Executor
{
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

struct Single_descriptor
{
    GUEST_STRUCT;
    GUEST<ULONGINT> id;
    GUEST<ULONGINT> offset;
    GUEST<ULONGINT> length;
};

struct Single_header
{
    GUEST_STRUCT;
    GUEST<LONGINT> magic;
    GUEST<LONGINT> version;
    GUEST<LONGINT[4]> filler;
    GUEST<INTEGER> nentries;
};

struct Single_dates
{
    GUEST_STRUCT;
    GUEST<LONGINT> crdat;
    GUEST<LONGINT> moddat;
    GUEST<LONGINT> backupdat;
    GUEST<LONGINT> accessdat;
};

struct Single_finfo
{
    GUEST_STRUCT;
    GUEST<FInfo> finfo;
    GUEST<FXInfo> fxinfo;
};

typedef ULONGINT Single_attribs;

typedef struct defaulthead
{
    GUEST_STRUCT;
    GUEST<Single_header> head;
    GUEST<Single_descriptor[10]> desc; /* we use 4, 6 for spare */
} defaulthead_t;

typedef struct defaultentries
{
    GUEST_STRUCT;
    GUEST<Single_attribs> attribs;
    GUEST<Single_dates> dates;
    GUEST<Single_finfo> finfo;
} defaultentries_t;

#define SINGLEMAGIC 0x0051600
#define DOUBLEMAGIC 0x0051607

#define SINGLEVERSION 0x00020000

extern OSErr ROMlib_newresfork(char *name, LONGINT *fdp, bool unix_p);
extern char *ROMlib_resname(char *pathname, char *filename, char *endname);

extern int afpd_conventions_p;
extern int netatalk_conventions_p;
extern char apple_double_quote_char;

typedef enum { mkdir_op,
               rmdir_op } double_dir_op_t;

extern void double_dir_op(char *name, double_dir_op_t op);
extern char *ROMlib_newunixfrommac(char *ip, INTEGER n);
extern BOOLEAN ROMlib_isresourcefork(const char *fullname);

enum class ResForkFormat
{
	standard,
	afpd,
	netatalk,
	native
};

extern void setup_resfork_format(ResForkFormat rf);
extern void report_resfork_problem();
}
#endif

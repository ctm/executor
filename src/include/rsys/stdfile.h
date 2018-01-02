#if !defined(__RSYS_STDFILE__)
#define __RSYS_STDFILE__

#include "rsys/pstuff.h"
#include "FileMgr.h"
#include "EventMgr.h"
#include "ControlMgr.h"
#include "DialogMgr.h"
#include "rsys/file.h"

namespace Executor
{
/*
 * Copyright 1989 - 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#define putPrompt 3
#define putDiskName 4
#define putNmList 8

#define getDiskName 4
#define getDotted 9

#define MICONLETTER 0
#define MICONCFOLDER 1
#define MICONFLOPPY 2
#define MICONOFOLDER 3
#define MICONAPP 4
#define MICONDISK 5

#define FAKEREDRAW 101
#define FAKECURDIR 102
#define FAKEOPENDIR 103

extern int ROMlib_strcmp(const Byte *s1, const Byte *s2);
extern void futzwithdosdisks(void);
extern void C_ROMlib_stdftrack(ControlHandle, INTEGER);
extern INTEGER C_ROMlib_stdffilt(DialogPeek, EventRecord *, GUEST<INTEGER> *);
extern void ROMlib_init_stdfile(void);

#if defined(LINUX)
extern int linuxfloppy_open(int disk, LONGINT *bsizep,
                            drive_flags_t *flagsp, const char *dname);
extern int linuxfloppy_close(int disk);
#endif

enum
{
    STANDARD_HEIGHT = 200,
    STANDARD_WIDTH = 348
};

extern int nodrivesearch_p;

OSErr C_unixmount(CInfoPBRec *cbp);
}
#endif /* !defined(__RSYS_STDFILE__) */

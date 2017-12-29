#if !defined(__SCRAP__)
#define __SCRAP__

#include "ResourceMgr.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

enum
{
    noScrapErr = (-100),
    noTypeErr = (-102),
};

namespace Executor
{
struct ScrapStuff
{
    GUEST_STRUCT;
    GUEST<LONGINT> scrapSize;
    GUEST<Handle> scrapHandle;
    GUEST<INTEGER> scrapCount;
    GUEST<INTEGER> scrapState;
    GUEST<StringPtr> scrapName;
};
typedef ScrapStuff *PScrapStuff;

#if 0
#if !defined(ScrapHandle_H)
extern GUEST<Handle> 	ScrapHandle_H;
extern GUEST<StringPtr> 	ScrapName_H;
extern LONGINT 	ScrapSize;
extern INTEGER 	ScrapCount;
extern INTEGER 	ScrapState;
#endif

enum
{
    ScrapHandle = (ScrapHandle_H.p),
    ScrapName = (ScrapName_H.p),
};
#endif

extern pascal trap PScrapStuff C_InfoScrap(void);
PASCAL_TRAP(InfoScrap, 0xA9F9);
extern pascal trap LONGINT C_UnloadScrap(void);
PASCAL_TRAP(UnloadScrap, 0xA9FA);
extern pascal trap LONGINT C_LoadScrap(void);
PASCAL_TRAP(LoadScrap, 0xA9FB);
extern LONGINT ROMlib_ZeroScrap(void);
extern pascal trap LONGINT C_ZeroScrap(void);
PASCAL_TRAP(ZeroScrap, 0xA9FC);
extern pascal trap LONGINT C_PutScrap(LONGINT len, ResType rest, Ptr p);
PASCAL_TRAP(PutScrap, 0xA9FE);
extern pascal trap LONGINT C_GetScrap(Handle h, ResType rest,
                                      GUEST<LONGINT> *off);
PASCAL_TRAP(GetScrap, 0xA9FD);
}
#endif /* __SCRAP__ */

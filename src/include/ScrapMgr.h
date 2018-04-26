#if !defined(__SCRAP__)
#define __SCRAP__

#include "ResourceMgr.h"

#define MODULE_NAME ScrapMgr
#include <rsys/api-module.h>

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

const LowMemGlobal<LONGINT> ScrapSize { 0x960 }; // ScrapMgr IMI-457 (true);
const LowMemGlobal<Handle> ScrapHandle { 0x964 }; // ScrapMgr IMI-457 (true);
const LowMemGlobal<INTEGER> ScrapCount { 0x968 }; // ScrapMgr IMI-457 (true);
const LowMemGlobal<INTEGER> ScrapState { 0x96A }; // ScrapMgr IMI-457 (true);
const LowMemGlobal<StringPtr> ScrapName { 0x96C }; // ScrapMgr IMI-457 (true);

extern PScrapStuff C_InfoScrap(void);
PASCAL_TRAP(InfoScrap, 0xA9F9);
extern LONGINT C_UnloadScrap(void);
PASCAL_TRAP(UnloadScrap, 0xA9FA);
extern LONGINT C_LoadScrap(void);
PASCAL_TRAP(LoadScrap, 0xA9FB);
extern LONGINT ROMlib_ZeroScrap(void);
extern LONGINT C_ZeroScrap(void);
PASCAL_TRAP(ZeroScrap, 0xA9FC);
extern LONGINT C_PutScrap(LONGINT len, ResType rest, Ptr p);
PASCAL_TRAP(PutScrap, 0xA9FE);
extern LONGINT C_GetScrap(Handle h, ResType rest,
                                      GUEST<LONGINT> *off);
PASCAL_TRAP(GetScrap, 0xA9FD);
}
#endif /* __SCRAP__ */

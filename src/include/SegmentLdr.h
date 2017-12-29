#if !defined(__SEGMENT__)
#define __SEGMENT__

/*
 * Copyright 1989 - 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "rsys/noreturn.h"

namespace Executor
{
extern _NORET_1_ pascal trap void C_ExitToShell(void) _NORET_2_;

#if !defined(USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#define appOpen 0
#define appPrint 1

struct AppFile
{
    GUEST_STRUCT;
    GUEST<INTEGER> vRefNum;
    GUEST<OSType> fType;
    GUEST<INTEGER> versNum;
    GUEST<Str255> fName;
};

#define hwParamErr (-502)

#if 0
#if !defined(AppParmHandle_H)
extern GUEST<Handle> 	AppParmHandle_H;
extern Byte 	loadtrap;
extern Byte 	FinderName[16];
extern INTEGER 	CurApRefNum;
extern Byte 	CurApName[34];
extern INTEGER 	CurJTOffset;
extern INTEGER 	CurPageOption;
#endif

#define AppParmHandle (AppParmHandle_H.p)
#endif

extern void flushcache(void);

extern trap void HWPriv(LONGINT d0, LONGINT a0);
extern char *ROMlib_undotdot(char *origp);
extern void CountAppFiles(GUEST<INTEGER> *messagep,
                          GUEST<INTEGER> *countp);
extern void GetAppFiles(INTEGER index, AppFile *filep);
extern void ClrAppFiles(INTEGER index);
extern pascal trap void Launch(StringPtr appl, INTEGER vrefnum);
extern pascal trap void Chain(StringPtr appl, INTEGER vrefnum);

extern pascal trap void C_GetAppParms(StringPtr namep,
                                      GUEST<INTEGER> *rnp, GUEST<Handle> *aphandp);

extern pascal trap void C_UnloadSeg(Ptr addr);

extern pascal trap void C_LoadSeg(INTEGER volatile segno);

extern pascal trap void C_UnloadSeg(Ptr addr);


#endif
}
#endif /* __SEGMENT__ */

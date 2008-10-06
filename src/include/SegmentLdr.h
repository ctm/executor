#if !defined (__SEGMENT__)
#define __SEGMENT__

/*
 * Copyright 1989 - 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: SegmentLdr.h 63 2004-12-24 18:19:43Z ctm $
 */


#include "rsys/noreturn.h"

extern _NORET_1_ pascal trap void C_ExitToShell( void ) _NORET_2_;

#if !defined (USE_WINDOWS_NOT_MAC_TYPEDEFS_AND_DEFINES)

#define appOpen		0
#define appPrint	1

typedef struct {
    INTEGER vRefNum	PACKED;
    OSType fType	PACKED;
    INTEGER versNum	PACKED;
    Str255 fName	PACKED;
} AppFile;


#define hwParamErr (-502)


#if !defined (AppParmHandle_H)
extern HIDDEN_Handle 	AppParmHandle_H;
extern Byte 	loadtrap;
extern Byte 	FinderName[16];
extern INTEGER 	CurApRefNum;
extern Byte 	CurApName[34];
extern INTEGER 	CurJTOffset;
extern INTEGER 	CurPageOption;
#endif

#define AppParmHandle	(AppParmHandle_H.p)

extern void flushcache (void); 

extern trap void HWPriv( LONGINT d0, LONGINT a0 ); 
extern char *ROMlib_undotdot( char *origp ); 
extern void CountAppFiles( INTEGER *messagep, 
 INTEGER *countp ); 
extern void GetAppFiles( INTEGER index, AppFile *filep ); 
extern void ClrAppFiles( INTEGER index ); 
extern pascal trap void C_GetAppParms( StringPtr namep, 
 INTEGER *rnp, HIDDEN_Handle *aphandp ); extern pascal trap void P_GetAppParms( StringPtr namep, 
 INTEGER *rnp, HIDDEN_Handle *aphandp ); 
extern pascal trap void P_ExitToShell( void ); 
extern pascal trap void Launch( StringPtr appl, INTEGER vrefnum );
extern pascal trap void Chain( StringPtr appl, INTEGER vrefnum );
extern pascal trap void C_UnloadSeg( Ptr addr ); extern pascal trap void P_UnloadSeg( Ptr addr); 
extern pascal trap void C_LoadSeg( INTEGER volatile segno ); extern pascal trap void P_LoadSeg( INTEGER volatile segno); 
extern pascal trap void C_UnloadSeg( Ptr addr ); extern pascal trap void P_UnloadSeg( Ptr addr); 
#endif

#endif /* __SEGMENT__ */

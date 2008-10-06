#if !defined (__SCRAP__)
#define __SCRAP__

#include "ResourceMgr.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ScrapMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#define noScrapErr	(-100)
#define noTypeErr	(-102)

typedef struct {
    LONGINT scrapSize	PACKED;
    Handle scrapHandle	PACKED_P;
    INTEGER scrapCount	PACKED;
    INTEGER scrapState	PACKED;
    StringPtr scrapName	PACKED_P;
} ScrapStuff;
typedef ScrapStuff *PScrapStuff;


#if !defined (ScrapHandle_H)
extern HIDDEN_Handle 	ScrapHandle_H;
extern HIDDEN_StringPtr 	ScrapName_H;
extern LONGINT 	ScrapSize;
extern INTEGER 	ScrapCount;
extern INTEGER 	ScrapState;
#endif

#define ScrapHandle	(ScrapHandle_H.p)
#define ScrapName	(ScrapName_H.p)

#if !defined (__STDC__)
extern PScrapStuff InfoScrap(); 
extern LONGINT UnloadScrap(); 
extern LONGINT LoadScrap(); 
extern LONGINT ROMlib_ZeroScrap(); 
extern LONGINT ZeroScrap(); 
extern LONGINT PutScrap(); 
extern LONGINT GetScrap(); 
#else /* __STDC__ */
extern pascal trap PScrapStuff C_InfoScrap( void  ); extern pascal trap PScrapStuff P_InfoScrap( void ); 
extern pascal trap LONGINT C_UnloadScrap( void  ); extern pascal trap LONGINT P_UnloadScrap( void ); 
extern pascal trap LONGINT C_LoadScrap( void  ); extern pascal trap LONGINT P_LoadScrap( void ); 
extern LONGINT ROMlib_ZeroScrap( void  ); 
extern pascal trap LONGINT C_ZeroScrap( void  ); extern pascal trap LONGINT P_ZeroScrap( void ); 
extern pascal trap LONGINT C_PutScrap( LONGINT len, ResType rest, Ptr p ); extern pascal trap LONGINT P_PutScrap( LONGINT len, ResType rest, Ptr p); 
extern pascal trap LONGINT C_GetScrap( Handle h, ResType rest, 
 LONGINT *off ); extern pascal trap LONGINT P_GetScrap( Handle h, ResType rest, 
 LONGINT *off ); 
#endif /* __STDC__ */
#endif /* __SCRAP__ */

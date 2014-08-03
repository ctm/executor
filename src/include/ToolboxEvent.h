#if !defined (__TOOLEVENT__)
#define __TOOLEVENT__

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: ToolboxEvent.h 63 2004-12-24 18:19:43Z ctm $
 */


#include "EventMgr.h"
namespace Executor {
#if !defined (KeyThresh)
extern INTEGER 	KeyThresh;
extern INTEGER 	KeyRepThresh;
extern LONGINT 	DoubleTime;
extern LONGINT 	CaretTime;
extern Byte 	ScrDmpEnb;
#endif

#if !defined (__STDC__)
extern void ROMlib_alarmoffmbar(); 
extern LONGINT KeyTrans(); 
extern BOOLEAN GetNextEvent(); 
extern BOOLEAN WaitNextEvent(); 
extern BOOLEAN EventAvail(); 
extern void GetMouse(); 
extern BOOLEAN Button(); 
extern BOOLEAN StillDown(); 
extern BOOLEAN WaitMouseUp(); 
extern void GetKeys(); 
extern LONGINT TickCount(); 
extern LONGINT GetDblTime(); 
extern LONGINT GetCaretTime(); 
#else /* __STDC__ */
extern void ROMlib_alarmoffmbar( void  ); 
extern pascal trap LONGINT C_KeyTrans( Ptr mapp, unsigned short code, 
 LONGINT *state ); extern pascal trap LONGINT P_KeyTrans( Ptr mapp, unsigned short code, 
 LONGINT *state ); 
extern pascal trap BOOLEAN C_GetNextEvent( INTEGER em, 
 EventRecord *evt ); extern pascal trap BOOLEAN P_GetNextEvent( INTEGER em, 
 EventRecord *evt ); 
extern pascal trap BOOLEAN C_WaitNextEvent( INTEGER mask, 
 EventRecord *evp, LONGINT sleep, RgnHandle mousergn ); extern pascal trap BOOLEAN P_WaitNextEvent( INTEGER mask, 
 EventRecord *evp, LONGINT sleep, RgnHandle mousergn ); 
extern pascal trap BOOLEAN C_EventAvail( INTEGER em, EventRecord *evt ); extern pascal trap BOOLEAN P_EventAvail( INTEGER em, EventRecord *evt); 
extern pascal trap void C_GetMouse( Point *p ); extern pascal trap void P_GetMouse( Point *p); 
extern pascal trap BOOLEAN C_Button( void  ); extern pascal trap BOOLEAN P_Button( void ); 
extern pascal trap BOOLEAN C_StillDown( void  ); extern pascal trap BOOLEAN P_StillDown( void ); 
extern pascal trap BOOLEAN C_WaitMouseUp( void  ); extern pascal trap BOOLEAN P_WaitMouseUp( void ); 
extern pascal trap void C_GetKeys( unsigned char *keys ); extern pascal trap void P_GetKeys( unsigned char *keys); 
extern pascal trap LONGINT C_TickCount( void  ); extern pascal trap LONGINT P_TickCount( void ); 
extern LONGINT GetDblTime( void  ); 
extern LONGINT GetCaretTime( void  ); 
#endif /* __STDC__ */
}
#endif /* __TOOLEVENT__ */

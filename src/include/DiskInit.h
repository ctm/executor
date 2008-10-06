#if !defined (__DISKINIT__)
#define __DISKINIT__

#include "QuickDraw.h"

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: DiskInit.h 63 2004-12-24 18:19:43Z ctm $
 */

#define badMDBErr	(-60)
#define extFSErr	(-58)
#define firstDskErr	(-84)
#define ioErr		(-36)
#define lastDskErr	(-64)
#define memFullErr	(-108)
#define noMacDskErr	(-57)
#define nsDrvErr	(-56)
#define paramErr	(-50)
#define volOnLinErr	(-55)


/* DO NOT DELETE THIS LINE */
#if !defined (__STDC__)
extern void DILoad(); 
extern void DIUnload(); 
extern INTEGER DIBadMount(); 
extern INTEGER dibadmount(); 
extern OSErr DIFormat(); 
extern OSErr DIVerify(); 
extern OSErr DIZero(); 
#else /* __STDC__ */
extern void C_DILoad( void  ); 
extern void C_DIUnload( void  ); 
extern INTEGER C_DIBadMount( Point pt, LONGINT evtmess ); 
extern INTEGER C_dibadmount( Point *ptp, LONGINT evtmess ); 
extern OSErr C_DIFormat( INTEGER dn ); 
extern OSErr C_DIVerify( INTEGER dn ); 
extern OSErr C_DIZero( INTEGER dn, StringPtr vname ); 
#endif /* __STDC__ */
#endif /* __DISKINIT__ */

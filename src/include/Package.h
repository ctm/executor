#if !defined (__PACKAGE__)
#define __PACKAGE__

/*
 * Copyright 1986, 1989, 1990, 1996 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: Package.h 63 2004-12-24 18:19:43Z ctm $
 */

namespace Executor {
#define dskInit	2
#define stdFile 3
#define flPoint	4
#define trFunc	5
#define intUtil	6
#define bdConv	7

#if !defined (AppPacks_H)
extern HIDDEN_Handle AppPacks_H[8];
#endif

#if !defined (__STDC__)
extern void InitPack(); 
extern void InitAllPacks(); 
#else /* __STDC__ */
extern pascal trap void C_InitPack( INTEGER packid ); extern pascal trap void P_InitPack( INTEGER packid); 
extern pascal trap void C_InitAllPacks( void  ); extern pascal trap void P_InitAllPacks( void ); 
#endif /* __STDC__ */
}
#endif /* __PACKAGE__ */

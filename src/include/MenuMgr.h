#if !defined (__MENU__)
#define __MENU__

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *
 * $Id: MenuMgr.h 63 2004-12-24 18:19:43Z ctm $
 */

#include "QuickDraw.h"
#include "ResourceMgr.h"

namespace Executor {
#define noMark	0

#define mDrawMsg	0
#define mChooseMsg	1
#define mSizeMsg	2
#define mPopUpRect	3

#define textMenuProc	0

struct MenuInfo : GuestStruct {
    GUEST< INTEGER> menuID;
    GUEST< INTEGER> menuWidth;
    GUEST< INTEGER> menuHeight;
    GUEST< Handle> menuProc;
    GUEST< LONGINT> enableFlags;
    GUEST< Str255> menuData;
};
typedef MenuInfo *MenuPtr;
MAKE_HIDDEN(MenuPtr);
typedef HIDDEN_MenuPtr *MenuHandle;

typedef struct MCEntry : GuestStruct {
    GUEST< INTEGER> mctID;
    GUEST< INTEGER> mctItem;
    GUEST< RGBColor> mctRGB1;
    GUEST< RGBColor> mctRGB2;
    GUEST< RGBColor> mctRGB3;
    GUEST< RGBColor> mctRGB4;
    GUEST< INTEGER> mctReserved;
} *MCEntryPtr;
MAKE_HIDDEN(MCEntryPtr);

typedef MCEntry MCTable[1];

typedef MCEntry *MCTablePtr;
MAKE_HIDDEN(MCTablePtr);
typedef HIDDEN_MCTablePtr *MCTableHandle;
MAKE_HIDDEN(MCTableHandle);

#if !defined (MenuList_H)
extern HIDDEN_Handle 	MenuList_H;
extern HIDDEN_ProcPtr 	MBarHook_H;
extern HIDDEN_ProcPtr 	MenuHook_H;
extern HIDDEN_Handle 	MBDFHndl_H;
extern HIDDEN_Handle 	MBSaveLoc_H;
extern HIDDEN_MCTableHandle MenuCInfo_H;
extern INTEGER 	TopMenuItem;
extern INTEGER 	AtMenuBottom;
extern INTEGER 	MBarEnable;
extern INTEGER 	MenuFlash;
extern INTEGER 	TheMenu;
extern LONGINT 	MenuDisable;
extern INTEGER 	MBarHeight;
#endif

#define MenuList	(MenuList_H.p)
#define MBarHook	(MBarHook_H.p)
#define MenuHook	(MenuHook_H.p)
#define MBDFHndl	(MBDFHndl_H.p)
#define MBSaveLoc	(MBSaveLoc_H.p)
#define MenuCInfo	(MenuCInfo_H.p)

#if !defined (__STDC__)
extern void DrawMenuBar(); 
extern void ClearMenuBar(); 
extern void InitMenus(); 
extern MenuHandle NewMenu(); 
extern void CalcMenuSize(); 
extern MenuHandle GetMenu(); 
extern void DisposeMenu(); 
extern void AppendMenu(); 
extern void AddResMenu(); 
extern void DelMenuItem(); 
extern void InsertResMenu(); 
extern void InsMenuItem(); 
extern void InsertMenu(); 
extern void DeleteMenu(); 
extern Handle GetNewMBar(); 
extern Handle GetMenuBar(); 
extern void SetMenuBar(); 
extern INTEGER ROMlib_mentosix(); 
extern void HiliteMenu(); 
extern LONGINT ROMlib_menuhelper(); 
extern LONGINT MenuSelect(); 
extern void FlashMenuBar(); 
extern LONGINT MenuKey(); 
extern void SetItem(); 
extern void GetItem(); 
extern void DisableItem(); 
extern void EnableItem(); 
extern void CheckItem(); 
extern void SetItemMark(); 
extern void GetItemMark(); 
extern void SetItemIcon(); 
extern void GetItemIcon(); 
extern void SetItemStyle(); 
extern void GetItemStyle(); 
extern INTEGER CountMItems(); 
extern MenuHandle GetMHandle(); 
extern void SetMenuFlash(); 
extern BOOLEAN ROMlib_shouldalarm(); 
extern void InitProcMenu(); 
extern LONGINT MenuChoice(); 
extern void GetItemCmd(); 
extern void SetItemCmd(); 
extern LONGINT PopUpMenuSelect(); 
#else /* __STDC__ */
extern pascal trap void C_DrawMenuBar( void  ); extern pascal trap void P_DrawMenuBar( void ); 
extern pascal trap void C_ClearMenuBar( void  ); extern pascal trap void P_ClearMenuBar( void ); 
extern pascal trap void C_InitMenus( void  ); extern pascal trap void P_InitMenus( void ); 
extern pascal trap MenuHandle C_NewMenu( INTEGER mid, StringPtr str ); extern pascal trap MenuHandle P_NewMenu( INTEGER mid, StringPtr str); 
extern pascal trap void C_CalcMenuSize( MenuHandle mh ); extern pascal trap void P_CalcMenuSize( MenuHandle mh); 
extern pascal trap MenuHandle C_GetMenu( INTEGER rid ); extern pascal trap MenuHandle P_GetMenu( INTEGER rid); 
extern pascal trap void C_DisposeMenu( MenuHandle mh ); extern pascal trap void P_DisposeMenu( MenuHandle mh); 
extern pascal trap void C_AppendMenu( MenuHandle mh, StringPtr str ); extern pascal trap void P_AppendMenu( MenuHandle mh, StringPtr str); 
extern pascal trap void C_AddResMenu( MenuHandle mh, ResType restype ); extern pascal trap void P_AddResMenu( MenuHandle mh, ResType restype); 
extern pascal trap void C_DelMenuItem( MenuHandle mh, 
 INTEGER item ); extern pascal trap void P_DelMenuItem( MenuHandle mh, 
 INTEGER item ); 
extern pascal trap void C_InsertResMenu( MenuHandle mh, ResType restype, 
 INTEGER after ); extern pascal trap void P_InsertResMenu( MenuHandle mh, ResType restype, 
 INTEGER after ); 
extern pascal trap void C_InsMenuItem( MenuHandle mh, StringPtr str, 
 INTEGER after ); extern pascal trap void P_InsMenuItem( MenuHandle mh, StringPtr str, 
 INTEGER after ); 
extern pascal trap void C_InsertMenu( MenuHandle mh, INTEGER before ); extern pascal trap void P_InsertMenu( MenuHandle mh, INTEGER before); 
extern pascal trap void C_DeleteMenu( INTEGER mid ); extern pascal trap void P_DeleteMenu( INTEGER mid); 
extern pascal trap Handle C_GetNewMBar( INTEGER mbarid ); extern pascal trap Handle P_GetNewMBar( INTEGER mbarid); 
extern pascal trap Handle C_GetMenuBar( void  ); extern pascal trap Handle P_GetMenuBar( void ); 
extern pascal trap void C_SetMenuBar( Handle ml ); extern pascal trap void P_SetMenuBar( Handle ml); 
extern INTEGER ROMlib_mentosix( INTEGER menuid ); 
extern pascal trap void C_HiliteMenu( INTEGER mid ); extern pascal trap void P_HiliteMenu( INTEGER mid); 
extern LONGINT ROMlib_menuhelper( MenuHandle mh, Rect *saverp, 
 LONGINT oldwhere, BOOLEAN ispopup, INTEGER nmenusdisplayed ); 
extern pascal trap LONGINT C_MenuSelect( Point p ); extern pascal trap LONGINT P_MenuSelect( Point p); 
extern pascal trap void C_FlashMenuBar( INTEGER mid ); extern pascal trap void P_FlashMenuBar( INTEGER mid); 
extern pascal trap LONGINT C_MenuKey( CHAR thec ); extern pascal trap LONGINT P_MenuKey( CHAR thec); 
extern pascal trap void C_SetItem( MenuHandle mh, INTEGER item, 
 StringPtr str ); extern pascal trap void P_SetItem( MenuHandle mh, INTEGER item, 
 StringPtr str ); 
extern pascal trap void C_GetItem( MenuHandle mh, INTEGER item, 
 StringPtr str ); extern pascal trap void P_GetItem( MenuHandle mh, INTEGER item, 
 StringPtr str ); 
extern pascal trap void C_DisableItem( MenuHandle mh, INTEGER item ); extern pascal trap void P_DisableItem( MenuHandle mh, INTEGER item); 
extern pascal trap void C_EnableItem( MenuHandle mh, INTEGER item ); extern pascal trap void P_EnableItem( MenuHandle mh, INTEGER item); 
extern pascal trap void C_CheckItem( MenuHandle mh, INTEGER item, 
 BOOLEAN cflag ); extern pascal trap void P_CheckItem( MenuHandle mh, INTEGER item, 
 BOOLEAN cflag ); 
extern pascal trap void C_SetItemMark( MenuHandle mh, INTEGER item, 
 CHAR mark ); extern pascal trap void P_SetItemMark( MenuHandle mh, INTEGER item, 
 CHAR mark ); 
extern pascal trap void C_GetItemMark( MenuHandle mh, INTEGER item, 
 INTEGER *markp ); extern pascal trap void P_GetItemMark( MenuHandle mh, INTEGER item, 
 INTEGER *markp ); 
extern pascal trap void C_SetItemIcon( MenuHandle mh, INTEGER item, 
 Byte icon ); extern pascal trap void P_SetItemIcon( MenuHandle mh, INTEGER item, 
 Byte icon ); 
extern pascal trap void C_GetItemIcon( MenuHandle mh, INTEGER item, 
 INTEGER *iconp ); extern pascal trap void P_GetItemIcon( MenuHandle mh, INTEGER item, 
 INTEGER *iconp ); 
extern pascal trap void C_SetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER style ); extern pascal trap void P_SetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER style ); 
extern pascal trap void C_GetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER *stylep ); extern pascal trap void P_GetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER *stylep ); 
extern pascal trap INTEGER C_CountMItems( MenuHandle mh ); extern pascal trap INTEGER P_CountMItems( MenuHandle mh); 
extern pascal trap MenuHandle C_GetMHandle( INTEGER mid ); extern pascal trap MenuHandle P_GetMHandle( INTEGER mid); 
extern pascal trap void C_SetMenuFlash( INTEGER i ); extern pascal trap void P_SetMenuFlash( INTEGER i); 
extern BOOLEAN ROMlib_shouldalarm( void  ); 
extern pascal trap void C_InitProcMenu( INTEGER mbid ); extern pascal trap void P_InitProcMenu( INTEGER mbid); 
extern pascal trap LONGINT C_MenuChoice( void  ); extern pascal trap LONGINT P_MenuChoice( void ); 
extern pascal trap void C_GetItemCmd( MenuHandle mh, INTEGER item, 
 CHAR *cmdp ); extern pascal trap void P_GetItemCmd( MenuHandle mh, INTEGER item, 
 CHAR *cmdp ); 
extern pascal trap void C_SetItemCmd( MenuHandle mh, INTEGER item, 
 CHAR cmd ); extern pascal trap void P_SetItemCmd( MenuHandle mh, INTEGER item, 
 CHAR cmd ); 
extern pascal trap LONGINT C_PopUpMenuSelect( MenuHandle mh, INTEGER top, 
 INTEGER left, INTEGER item ); extern pascal trap LONGINT P_PopUpMenuSelect( MenuHandle mh, INTEGER top, 
 INTEGER left, INTEGER item );

extern pascal trap void C_DelMCEntries (INTEGER, INTEGER);
extern pascal trap MCTableHandle C_GetMCInfo ();
extern pascal trap void C_SetMCInfo (MCTableHandle);
extern pascal trap void C_DispMCInfo (MCTableHandle);
extern pascal trap MCEntryPtr C_GetMCEntry (INTEGER, INTEGER);
extern pascal trap void C_SetMCEntries (INTEGER, MCTablePtr);
extern pascal trap void C_InvalMenuBar (void);
}

#endif /* __STDC__ */
#endif /* __MENU__ */

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

struct MenuInfo { GUEST_STRUCT;
    GUEST< INTEGER> menuID;
    GUEST< INTEGER> menuWidth;
    GUEST< INTEGER> menuHeight;
    GUEST< Handle> menuProc;
    GUEST< LONGINT> enableFlags;
    GUEST< Str255> menuData;
};
typedef MenuInfo *MenuPtr;

typedef GUEST<MenuPtr> *MenuHandle;

typedef struct MCEntry { GUEST_STRUCT;
    GUEST< INTEGER> mctID;
    GUEST< INTEGER> mctItem;
    GUEST< RGBColor> mctRGB1;
    GUEST< RGBColor> mctRGB2;
    GUEST< RGBColor> mctRGB3;
    GUEST< RGBColor> mctRGB4;
    GUEST< INTEGER> mctReserved;

    MCEntry() = default;
    MCEntry(GUEST< INTEGER> mctID,
            GUEST< INTEGER> mctItem,
            GUEST< RGBColor> mctRGB1,
            GUEST< RGBColor> mctRGB2,
            GUEST< RGBColor> mctRGB3,
            GUEST< RGBColor> mctRGB4,
            GUEST< INTEGER> mctReserved)
        : mctID     (mctID),
          mctItem   (mctItem),
          mctRGB1   (mctRGB1),
          mctRGB2   (mctRGB2),
          mctRGB3   (mctRGB3),
          mctRGB4   (mctRGB4),
          mctReserved(mctReserved)
    {
    }
} *MCEntryPtr;


typedef MCEntry MCTable[1];

typedef MCEntry *MCTablePtr;

typedef GUEST<MCTablePtr> *MCTableHandle;


#if 0
#if !defined (MenuList_H)
extern GUEST<Handle> 	MenuList_H;
extern GUEST<ProcPtr> 	MBarHook_H;
extern GUEST<ProcPtr> 	MenuHook_H;
extern GUEST<Handle> 	MBDFHndl_H;
extern GUEST<Handle> 	MBSaveLoc_H;
extern GUEST<MCTableHandle> MenuCInfo_H;
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
#endif

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
 GUEST<INTEGER> *markp );
extern pascal trap void C_SetItemIcon( MenuHandle mh, INTEGER item, 
 Byte icon ); extern pascal trap void P_SetItemIcon( MenuHandle mh, INTEGER item, 
 Byte icon ); 
extern pascal trap void C_GetItemIcon( MenuHandle mh, INTEGER item, 
        GUEST<INTEGER> *iconp );
extern pascal trap void C_SetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER style ); extern pascal trap void P_SetItemStyle( MenuHandle mh, INTEGER item, 
 INTEGER style ); 
extern pascal trap void C_GetItemStyle( MenuHandle mh, INTEGER item, 
        GUEST<INTEGER> *stylep );
extern pascal trap INTEGER C_CountMItems( MenuHandle mh ); extern pascal trap INTEGER P_CountMItems( MenuHandle mh); 
extern pascal trap MenuHandle C_GetMHandle( INTEGER mid ); extern pascal trap MenuHandle P_GetMHandle( INTEGER mid); 
extern pascal trap void C_SetMenuFlash( INTEGER i ); extern pascal trap void P_SetMenuFlash( INTEGER i); 
extern BOOLEAN ROMlib_shouldalarm( void  ); 
extern pascal trap void C_InitProcMenu( INTEGER mbid ); extern pascal trap void P_InitProcMenu( INTEGER mbid); 
extern pascal trap LONGINT C_MenuChoice( void  ); extern pascal trap LONGINT P_MenuChoice( void ); 
extern pascal trap void C_GetItemCmd( MenuHandle mh, INTEGER item, 
 GUEST<CHAR> *cmdp );
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

#endif /* __MENU__ */

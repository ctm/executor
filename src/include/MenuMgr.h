#if !defined(__MENU__)
#define __MENU__

/*
 * Copyright 1986, 1989, 1990, 1995 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "QuickDraw.h"
#include "ResourceMgr.h"

namespace Executor
{
enum
{
    noMark = 0,
};

enum
{
    mDrawMsg = 0,
    mChooseMsg = 1,
    mSizeMsg = 2,
    mPopUpRect = 3,
};

enum
{
    textMenuProc = 0,
};

struct MenuInfo
{
    GUEST_STRUCT;
    GUEST<INTEGER> menuID;
    GUEST<INTEGER> menuWidth;
    GUEST<INTEGER> menuHeight;
    GUEST<Handle> menuProc;
    GUEST<LONGINT> enableFlags;
    GUEST<Str255> menuData;
};
typedef MenuInfo *MenuPtr;

typedef GUEST<MenuPtr> *MenuHandle;

typedef struct MCEntry
{
    GUEST_STRUCT;
    GUEST<INTEGER> mctID;
    GUEST<INTEGER> mctItem;
    GUEST<RGBColor> mctRGB1;
    GUEST<RGBColor> mctRGB2;
    GUEST<RGBColor> mctRGB3;
    GUEST<RGBColor> mctRGB4;
    GUEST<INTEGER> mctReserved;

    MCEntry() = default;
    MCEntry(GUEST<INTEGER> mctID,
            GUEST<INTEGER> mctItem,
            GUEST<RGBColor> mctRGB1,
            GUEST<RGBColor> mctRGB2,
            GUEST<RGBColor> mctRGB3,
            GUEST<RGBColor> mctRGB4,
            GUEST<INTEGER> mctReserved)
        : mctID(mctID)
        , mctItem(mctItem)
        , mctRGB1(mctRGB1)
        , mctRGB2(mctRGB2)
        , mctRGB3(mctRGB3)
        , mctRGB4(mctRGB4)
        , mctReserved(mctReserved)
    {
    }
} * MCEntryPtr;

typedef MCEntry MCTable[1];

typedef MCEntry *MCTablePtr;

typedef GUEST<MCTablePtr> *MCTableHandle;

#if 0
#if !defined(MenuList_H)
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

enum
{
    MenuList = (MenuList_H.p),
    MBarHook = (MBarHook_H.p),
    MenuHook = (MenuHook_H.p),
    MBDFHndl = (MBDFHndl_H.p),
    MBSaveLoc = (MBSaveLoc_H.p),
    MenuCInfo = (MenuCInfo_H.p),
};
#endif

extern pascal trap void C_DrawMenuBar(void);
PASCAL_TRAP(DrawMenuBar, 0xA937);

extern pascal trap void C_ClearMenuBar(void);
PASCAL_TRAP(ClearMenuBar, 0xA934);

extern pascal trap void C_InitMenus(void);
PASCAL_TRAP(InitMenus, 0xA930);

extern pascal trap MenuHandle C_NewMenu(INTEGER mid, StringPtr str);
PASCAL_TRAP(NewMenu, 0xA931);

extern pascal trap void C_CalcMenuSize(MenuHandle mh);
PASCAL_TRAP(CalcMenuSize, 0xA948);

extern pascal trap MenuHandle C_GetMenu(INTEGER rid);
PASCAL_TRAP(GetMenu, 0xA9BF);

extern pascal trap void C_DisposeMenu(MenuHandle mh);
PASCAL_TRAP(DisposeMenu, 0xA932);

extern pascal trap void C_AppendMenu(MenuHandle mh, StringPtr str);
PASCAL_TRAP(AppendMenu, 0xA933);

extern pascal trap void C_AddResMenu(MenuHandle mh, ResType restype);
PASCAL_TRAP(AddResMenu, 0xA94D);

extern pascal trap void C_DelMenuItem(MenuHandle mh,
                                      INTEGER item);
PASCAL_TRAP(DelMenuItem, 0xA952);
extern pascal trap void C_InsertResMenu(MenuHandle mh, ResType restype,
                                        INTEGER after);
PASCAL_TRAP(InsertResMenu, 0xA951);
extern pascal trap void C_InsMenuItem(MenuHandle mh, StringPtr str,
                                      INTEGER after);
PASCAL_TRAP(InsMenuItem, 0xA826);
extern pascal trap void C_InsertMenu(MenuHandle mh, INTEGER before);
PASCAL_TRAP(InsertMenu, 0xA935);

extern pascal trap void C_DeleteMenu(INTEGER mid);
PASCAL_TRAP(DeleteMenu, 0xA936);

extern pascal trap Handle C_GetNewMBar(INTEGER mbarid);
PASCAL_TRAP(GetNewMBar, 0xA9C0);

extern pascal trap Handle C_GetMenuBar(void);
PASCAL_TRAP(GetMenuBar, 0xA93B);

extern pascal trap void C_SetMenuBar(Handle ml);
PASCAL_TRAP(SetMenuBar, 0xA93C);

extern INTEGER ROMlib_mentosix(INTEGER menuid);
extern pascal trap void C_HiliteMenu(INTEGER mid);
PASCAL_TRAP(HiliteMenu, 0xA938);

extern LONGINT ROMlib_menuhelper(MenuHandle mh, Rect *saverp,
                                 LONGINT oldwhere, BOOLEAN ispopup, INTEGER nmenusdisplayed);
extern pascal trap LONGINT C_MenuSelect(Point p);
PASCAL_TRAP(MenuSelect, 0xA93D);

extern pascal trap void C_FlashMenuBar(INTEGER mid);
PASCAL_TRAP(FlashMenuBar, 0xA94C);

extern pascal trap LONGINT C_MenuKey(CHAR thec);
PASCAL_TRAP(MenuKey, 0xA93E);

extern pascal trap void C_SetItem(MenuHandle mh, INTEGER item,
                                  StringPtr str);
PASCAL_TRAP(SetItem, 0xA947);
extern pascal trap void C_GetItem(MenuHandle mh, INTEGER item,
                                  StringPtr str);
PASCAL_TRAP(GetItem, 0xA946);
extern pascal trap void C_DisableItem(MenuHandle mh, INTEGER item);
PASCAL_TRAP(DisableItem, 0xA93A);

extern pascal trap void C_EnableItem(MenuHandle mh, INTEGER item);
PASCAL_TRAP(EnableItem, 0xA939);

extern pascal trap void C_CheckItem(MenuHandle mh, INTEGER item,
                                    BOOLEAN cflag);
PASCAL_TRAP(CheckItem, 0xA945);
extern pascal trap void C_SetItemMark(MenuHandle mh, INTEGER item,
                                      CHAR mark);
PASCAL_TRAP(SetItemMark, 0xA944);
extern pascal trap void C_GetItemMark(MenuHandle mh, INTEGER item,
                                      GUEST<INTEGER> *markp);
PASCAL_TRAP(GetItemMark, 0xA943);
extern pascal trap void C_SetItemIcon(MenuHandle mh, INTEGER item,
                                      Byte icon);
PASCAL_TRAP(SetItemIcon, 0xA940);
extern pascal trap void C_GetItemIcon(MenuHandle mh, INTEGER item,
                                      GUEST<INTEGER> *iconp);
PASCAL_TRAP(GetItemIcon, 0xA93F);
extern pascal trap void C_SetItemStyle(MenuHandle mh, INTEGER item,
                                       INTEGER style);
PASCAL_TRAP(SetItemStyle, 0xA942);
extern pascal trap void C_GetItemStyle(MenuHandle mh, INTEGER item,
                                       GUEST<INTEGER> *stylep);
PASCAL_TRAP(GetItemStyle, 0xA941);
extern pascal trap INTEGER C_CountMItems(MenuHandle mh);
PASCAL_TRAP(CountMItems, 0xA950);

extern pascal trap MenuHandle C_GetMHandle(INTEGER mid);
PASCAL_TRAP(GetMHandle, 0xA949);

extern pascal trap void C_SetMenuFlash(INTEGER i);
PASCAL_TRAP(SetMenuFlash, 0xA94A);

extern BOOLEAN ROMlib_shouldalarm(void);
extern pascal trap void C_InitProcMenu(INTEGER mbid);
PASCAL_TRAP(InitProcMenu, 0xA808);

extern pascal trap LONGINT C_MenuChoice(void);
PASCAL_TRAP(MenuChoice, 0xAA66);

extern pascal trap void C_GetItemCmd(MenuHandle mh, INTEGER item,
                                     GUEST<CHAR> *cmdp);
PASCAL_TRAP(GetItemCmd, 0xA84E);
extern pascal trap void C_SetItemCmd(MenuHandle mh, INTEGER item,
                                     CHAR cmd);
PASCAL_TRAP(SetItemCmd, 0xA84F);
extern pascal trap LONGINT C_PopUpMenuSelect(MenuHandle mh, INTEGER top,
                                             INTEGER left, INTEGER item);
PASCAL_TRAP(PopUpMenuSelect, 0xA80B);

extern pascal trap void C_DelMCEntries(INTEGER, INTEGER);
PASCAL_TRAP(DelMCEntries, 0xAA60);
extern pascal trap MCTableHandle C_GetMCInfo();
PASCAL_TRAP(GetMCInfo, 0xAA61);
extern pascal trap void C_SetMCInfo(MCTableHandle);
PASCAL_TRAP(SetMCInfo, 0xAA62);
extern pascal trap void C_DispMCInfo(MCTableHandle);
PASCAL_TRAP(DispMCInfo, 0xAA63);
extern pascal trap MCEntryPtr C_GetMCEntry(INTEGER, INTEGER);
PASCAL_TRAP(GetMCEntry, 0xAA64);
extern pascal trap void C_SetMCEntries(INTEGER, MCTablePtr);
PASCAL_TRAP(SetMCEntries, 0xAA65);
extern pascal trap void C_InvalMenuBar(void);
PASCAL_TRAP(InvalMenuBar, 0xA81D);
}

#endif /* __MENU__ */

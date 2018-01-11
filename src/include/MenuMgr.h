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

const LowMemGlobal<INTEGER> TopMenuItem { 0xA0A }; // MenuMgr IMV-249 (true);
const LowMemGlobal<INTEGER> AtMenuBottom { 0xA0C }; // MenuMgr IMV-249 (true);
const LowMemGlobal<Handle> MenuList { 0xA1C }; // MenuMgr IMI-346 (true);
const LowMemGlobal<INTEGER> MBarEnable { 0xA20 }; // MenuMgr IMI-356 (true);
const LowMemGlobal<INTEGER> MenuFlash { 0xA24 }; // MenuMgr IMI-361 (true);
const LowMemGlobal<INTEGER> TheMenu { 0xA26 }; // MenuMgr IMI-357 (true);
const LowMemGlobal<ProcPtr> MBarHook { 0xA2C }; // MenuMgr IMI-356 (true);
const LowMemGlobal<ProcPtr> MenuHook { 0xA30 }; // MenuMgr IMI-356 (true);
const LowMemGlobal<LONGINT> MenuDisable { 0xB54 }; // MenuMgr IMV-249 (true);
const LowMemGlobal<Handle> MBDFHndl { 0xB58 }; // MenuMgr Private.a (true);
const LowMemGlobal<Handle> MBSaveLoc { 0xB5C }; // MenuMgr Private.a (true);
const LowMemGlobal<INTEGER> MBarHeight { 0xBAA }; // MenuMgr IMV-253 (true);
const LowMemGlobal<MCTableHandle> MenuCInfo { 0xD50 }; // QuickDraw IMV-242 (true);

extern void C_DrawMenuBar(void);
PASCAL_TRAP(DrawMenuBar, 0xA937);

extern void C_ClearMenuBar(void);
PASCAL_TRAP(ClearMenuBar, 0xA934);

extern void C_InitMenus(void);
PASCAL_TRAP(InitMenus, 0xA930);

extern MenuHandle C_NewMenu(INTEGER mid, StringPtr str);
PASCAL_TRAP(NewMenu, 0xA931);

extern void C_CalcMenuSize(MenuHandle mh);
PASCAL_TRAP(CalcMenuSize, 0xA948);

extern MenuHandle C_GetMenu(INTEGER rid);
PASCAL_TRAP(GetMenu, 0xA9BF);

extern void C_DisposeMenu(MenuHandle mh);
PASCAL_TRAP(DisposeMenu, 0xA932);

extern void C_AppendMenu(MenuHandle mh, StringPtr str);
PASCAL_TRAP(AppendMenu, 0xA933);

extern void C_AddResMenu(MenuHandle mh, ResType restype);
PASCAL_TRAP(AddResMenu, 0xA94D);

extern void C_DelMenuItem(MenuHandle mh,
                                      INTEGER item);
PASCAL_TRAP(DelMenuItem, 0xA952);
extern void C_InsertResMenu(MenuHandle mh, ResType restype,
                                        INTEGER after);
PASCAL_TRAP(InsertResMenu, 0xA951);
extern void C_InsMenuItem(MenuHandle mh, StringPtr str,
                                      INTEGER after);
PASCAL_TRAP(InsMenuItem, 0xA826);
extern void C_InsertMenu(MenuHandle mh, INTEGER before);
PASCAL_TRAP(InsertMenu, 0xA935);

extern void C_DeleteMenu(INTEGER mid);
PASCAL_TRAP(DeleteMenu, 0xA936);

extern Handle C_GetNewMBar(INTEGER mbarid);
PASCAL_TRAP(GetNewMBar, 0xA9C0);

extern Handle C_GetMenuBar(void);
PASCAL_TRAP(GetMenuBar, 0xA93B);

extern void C_SetMenuBar(Handle ml);
PASCAL_TRAP(SetMenuBar, 0xA93C);

extern INTEGER ROMlib_mentosix(INTEGER menuid);
extern void C_HiliteMenu(INTEGER mid);
PASCAL_TRAP(HiliteMenu, 0xA938);

extern LONGINT ROMlib_menuhelper(MenuHandle mh, Rect *saverp,
                                 LONGINT oldwhere, BOOLEAN ispopup, INTEGER nmenusdisplayed);
extern LONGINT C_MenuSelect(Point p);
PASCAL_TRAP(MenuSelect, 0xA93D);

extern void C_FlashMenuBar(INTEGER mid);
PASCAL_TRAP(FlashMenuBar, 0xA94C);

extern LONGINT C_MenuKey(CharParameter thec);
PASCAL_TRAP(MenuKey, 0xA93E);

extern void C_SetItem(MenuHandle mh, INTEGER item,
                                  StringPtr str);
PASCAL_TRAP(SetItem, 0xA947);
extern void C_GetItem(MenuHandle mh, INTEGER item,
                                  StringPtr str);
PASCAL_TRAP(GetItem, 0xA946);
extern void C_DisableItem(MenuHandle mh, INTEGER item);
PASCAL_TRAP(DisableItem, 0xA93A);

extern void C_EnableItem(MenuHandle mh, INTEGER item);
PASCAL_TRAP(EnableItem, 0xA939);

extern void C_CheckItem(MenuHandle mh, INTEGER item,
                                    BOOLEAN cflag);
PASCAL_TRAP(CheckItem, 0xA945);
extern void C_SetItemMark(MenuHandle mh, INTEGER item,
                                      CharParameter mark);
PASCAL_TRAP(SetItemMark, 0xA944);
extern void C_GetItemMark(MenuHandle mh, INTEGER item,
                                      GUEST<INTEGER> *markp);
PASCAL_TRAP(GetItemMark, 0xA943);
extern void C_SetItemIcon(MenuHandle mh, INTEGER item,
                                      Byte icon);
PASCAL_TRAP(SetItemIcon, 0xA940);
extern void C_GetItemIcon(MenuHandle mh, INTEGER item,
                                      GUEST<INTEGER> *iconp);
PASCAL_TRAP(GetItemIcon, 0xA93F);
extern void C_SetItemStyle(MenuHandle mh, INTEGER item,
                                       INTEGER style);
PASCAL_TRAP(SetItemStyle, 0xA942);
extern void C_GetItemStyle(MenuHandle mh, INTEGER item,
                                       GUEST<INTEGER> *stylep);
PASCAL_TRAP(GetItemStyle, 0xA941);
extern INTEGER C_CountMItems(MenuHandle mh);
PASCAL_TRAP(CountMItems, 0xA950);

extern MenuHandle C_GetMHandle(INTEGER mid);
PASCAL_TRAP(GetMHandle, 0xA949);

extern void C_SetMenuFlash(INTEGER i);
PASCAL_TRAP(SetMenuFlash, 0xA94A);

extern BOOLEAN ROMlib_shouldalarm(void);
extern void C_InitProcMenu(INTEGER mbid);
PASCAL_TRAP(InitProcMenu, 0xA808);

extern LONGINT C_MenuChoice(void);
PASCAL_TRAP(MenuChoice, 0xAA66);

extern void C_GetItemCmd(MenuHandle mh, INTEGER item,
                                     GUEST<CharParameter> *cmdp);
PASCAL_TRAP(GetItemCmd, 0xA84E);
extern void C_SetItemCmd(MenuHandle mh, INTEGER item,
                                     CharParameter cmd);
PASCAL_TRAP(SetItemCmd, 0xA84F);
extern LONGINT C_PopUpMenuSelect(MenuHandle mh, INTEGER top,
                                             INTEGER left, INTEGER item);
PASCAL_TRAP(PopUpMenuSelect, 0xA80B);

extern void C_DelMCEntries(INTEGER, INTEGER);
PASCAL_TRAP(DelMCEntries, 0xAA60);
extern MCTableHandle C_GetMCInfo();
PASCAL_TRAP(GetMCInfo, 0xAA61);
extern void C_SetMCInfo(MCTableHandle);
PASCAL_TRAP(SetMCInfo, 0xAA62);
extern void C_DispMCInfo(MCTableHandle);
PASCAL_TRAP(DispMCInfo, 0xAA63);
extern MCEntryPtr C_GetMCEntry(INTEGER, INTEGER);
PASCAL_TRAP(GetMCEntry, 0xAA64);
extern void C_SetMCEntries(INTEGER, MCTablePtr);
PASCAL_TRAP(SetMCEntries, 0xAA65);
extern void C_InvalMenuBar(void);
PASCAL_TRAP(InvalMenuBar, 0xA81D);
}

#endif /* __MENU__ */

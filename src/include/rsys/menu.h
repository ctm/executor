
#if !defined(__MENU_H_)
#define __MENU_H_

/*
 * Copyright 1986, 1989, 1990 by Abacus Research and Development, Inc.
 * All rights reserved.
 *

 */

#include "MenuMgr.h"
#include "ResourceMgr.h"
#include "rsys/mman.h"
#include "rsys/pstuff.h"

namespace Executor
{
#define MI_ID_X(mi) (HxX(mi, menuID))
#define MI_WIDTH_X(mi) (HxX(mi, menuWidth))
#define MI_HEIGHT_X(mi) (HxX(mi, menuHeight))
#define MI_PROC_X(mi) (HxX(mi, menuProc))
#define MI_ENABLE_FLAGS_X(mi) (HxX(mi, enableFlags))
#define MI_DATA(mi) (HxX(mi, menuData))

#define MI_ID(mi) (CW(MI_ID_X(mi)))
#define MI_WIDTH(mi) (CW(MI_WIDTH_X(mi)))
#define MI_HEIGHT(mi) (CW(MI_HEIGHT_X(mi)))
#define MI_PROC(mi) (MR(MI_PROC_X(mi)))
#define MI_ENABLE_FLAGS(mi) (CL(MI_ENABLE_FLAGS_X(mi)))

#define MI_TITLE(mi) (HxX(mi, menuTitle))

typedef struct mext
{
    GUEST_STRUCT;
    GUEST<Byte> micon;
    GUEST<Byte> mkeyeq;
    GUEST<Byte> mmarker;
    GUEST<Byte> mstyle;
    GUEST<Byte> mnextlen;
} * mextp;

extern void C_mdef0(INTEGER, MenuHandle, Rect *, Point, GUEST<INTEGER> *);
extern int32_t C_mbdf0(int16_t, int16_t, int16_t, int32_t);

#define SIZEOFMEXT 5
#define SIZEOFMINFO (sizeof(MenuInfo) - sizeof(Str255) + 1)

extern void ROMlib_alarmoffmbar(void);
extern mextp ROMlib_mitemtop(MenuHandle mh, INTEGER item, StringPtr *ssp);
extern INTEGER ROMlib_mentosix(INTEGER mid);
extern LONGINT ROMlib_menuhelper(MenuHandle mh, Rect *saver, LONGINT where,
                                 BOOLEAN ispopup, INTEGER ndisplayed);

#define mbDraw 0
#define mbHit 1
#define mbCalc 2
#define mbInit 3
#define mbDispose 4
#define mbHilite 5
#define mbHeight 6
#define mbSave 7
#define mbRestore 8
#define mbRect 9
#define mbSaveAlt 10
#define mbResetAlt 11
#define mbMenuRgn 12

#define MLMAX 16

struct muelem
{
    GUEST_STRUCT;
    GUEST<MenuHandle> muhandle;
    GUEST<INTEGER> muleft;
};

typedef struct menu_elt
{
    MenuHandle /* elt */ muhandle;
    /* for a hierarchical menu, the menu_left field is reserved */
    INTEGER /* menu_left */ muleft;
} menu_elt;

#define ML_MENU(ml, index) \
    (((menu_elt *)&HxX(ml, data))[index])
#define ML_HMENU(ml, index)                          \
    (((menu_elt *)((char *)STARH(ml)                 \
                   + ML_LAST_MENU_OFFSET(ml)         \
                   + sizeof(menu_list)               \
                   + sizeof(INTEGER) /* lastHMenu */ \
                   + sizeof(PixMapHandle) /* menuTitleSave */))[index])

#define ML_LAST_MENU_OFFSET_X(ml) (HxX(ml, last_menu_offset))
#define ML_LAST_RIGHT_X(ml) (HxX(ml, last_right))
#define ML_RES_ID_X(ml) (HxX(ml, mb_res_id))
#define ML_LAST_HMENU_OFFSET_X(ml)          \
    (*(INTEGER *)((char *)STARH(ml)         \
                  + ML_LAST_MENU_OFFSET(ml) \
                  + sizeof(menu_elt)))
#define ML_MENU_TITLE_SAVE_X(ml)                                   \
    ((*(GUEST<PixMapHandle> *)((char *)STARH(ml)                   \
                               + ML_LAST_MENU_OFFSET(ml)           \
                               + sizeof(menu_elt)                  \
                               + sizeof(INTEGER) /* lastHMenu */)) \
         .p)

#define ML_LAST_MENU_OFFSET(ml) (CW(ML_LAST_MENU_OFFSET_X(ml)))
#define ML_LAST_RIGHT(ml) (CW(ML_LAST_RIGHT_X(ml)))
#define ML_RES_ID(ml) (CW(ML_RES_ID_X(ml)))
#define ML_LAST_HMENU_OFFSET(ml) (CW(ML_LAST_HMENU_OFFSET_X(ml)))
#define ML_MENU_TITLE_SAVE(ml) (CW(ML_MENU_TITLE_SAVE_X(ml)))

struct menu_list
{
    GUEST_STRUCT;
    GUEST<INTEGER> last_menu_offset;
    GUEST<INTEGER> last_right;
    GUEST<INTEGER> mb_res_id;
    /* other stuff... */
    GUEST<char> data;
};

typedef menu_list *menu_list_ptr;

typedef GUEST<menu_list_ptr> *menu_list_handle;

struct menulist
{
    GUEST_STRUCT;
    GUEST<INTEGER> muoff;
    GUEST<INTEGER> muright;
    GUEST<INTEGER> mufu;
    GUEST<muelem[MLMAX]> mulist; /* WILL NEED Cx() */
};

typedef menulist *menulistp;

typedef GUEST<menulistp> *mlhandle;

/* Menu Color Entry accessors */
#define MCENTRY_RGB1(entry) ((entry)->mctRGB1)
#define MCENTRY_RGB2(entry) ((entry)->mctRGB2)
#define MCENTRY_RGB3(entry) ((entry)->mctRGB3)
#define MCENTRY_RGB4(entry) ((entry)->mctRGB4)

#define MCENTRY_ID_X(entry) ((entry)->mctID)
#define MCENTRY_ITEM_X(entry) ((entry)->mctItem)
#define MCENTRY_RESERVED_X(entry) ((entry)->mctReserved)

#define MCENTRY_ID(entry) (CW(MCENTRY_ID_X(entry)))
#define MCENTRY_ITEM(entry) (CW(MCENTRY_ITEM_X(entry)))

#define MENULIST ((mlhandle)MR(MenuList))

#define MENULEFT 10

#define DRAWMENUBAR 0
#define CLEARMENUBAR (-1)

struct mbdfheader
{
    GUEST_STRUCT;
    GUEST<INTEGER> lastMBSave; /* offset to top most menu saved */
    GUEST<Handle> mbCustomStorage; /* for custom jobs (i.e. we don't use) */
    GUEST<Rect> mbItemRect; /* currently chosen menu */
    GUEST<Byte> mbMenuDelay; /* MenuDelay from param ram */
    GUEST<Byte> mbMenuDrag; /* MenuDrag from param ram */
    GUEST<INTEGER> mbUglyScroll; /* HMenu flag having to do with scrolling?? */
    GUEST<INTEGER> mbIconState; /* ??? NMgr icon state */
};

typedef mbdfheader *mbdfheaderptr;

typedef GUEST<mbdfheaderptr> *mbdfheaderhand;

struct mbdfentry
{
    GUEST_STRUCT;
    GUEST<Rect> mbRectSave; /* where it is on screen */
    GUEST<Handle> mbBitsSave; /* where the bits are */
    GUEST<INTEGER> mbMenuDir; /* what direction the menu was placed */
    GUEST<INTEGER> mbMLOffset; /* 6 byte offset into MenuList */
    GUEST<MenuHandle> mbMLHandle; /* the handle from MenuList */
    GUEST<INTEGER> mbTopScroll; /* copy of TopMenuItem */
    GUEST<INTEGER> mbBotScroll; /* copy of AtMenuBottom */
    GUEST<LONGINT> mbReserved; /* i dunno */
};

#define NMBDFENTRIES 5
#define MBDFSTRUCTBYTES ((NMBDFENTRIES + 1) * sizeof(mbdfentry))

#define MBRIGHTDIR 0
#define MBLEFTDIR 1

#if 0
#if !defined(MBSaveLoc_H)
extern GUEST<Handle> MBSaveLoc_H;
extern GUEST<Handle> MBDFHndl_H;
#endif

#define MBSaveLoc (MBSaveLoc_H.p)
#define MBDFHndl (MBDFHndl_H.p)
#endif

#define MBSAVELOC ((mbdfheaderhand)MR(MBSaveLoc))

#define SLOP 13

typedef void (*menuprocp)(INTEGER mess, MenuHandle themenu,
                                 Rect *menrect, Point hit, INTEGER *which);
typedef LONGINT (*mbdfprocp)(INTEGER variant, INTEGER msg,
                                    INTEGER param1, intptr_t param2);

extern void ROMlib_menucall(INTEGER mess, MenuHandle themenu, Rect *menrect,
                            Point hit, GUEST<INTEGER> *which);
extern LONGINT ROMlib_mbdfcall(INTEGER msg, INTEGER param1, LONGINT param2);

#define MENUCALL ROMlib_menucall
#define MBDFCALL ROMlib_mbdfcall

#define MBDFDECL() INTEGER mbdfstate

#define MBDFBEGIN() (mbdfstate = HGetState(MR(MBDFHndl)), HSetState(MR(MBDFHndl), \
                                                                    mbdfstate | LOCKBIT))

#define MBDFEND() HSetState(MR(MBDFHndl), mbdfstate)

#define NOTHITINMBAR 0
#define NOTHIT (-1)

#define image_bits(bits)                               \
    (((((0x##bits##UL) >> (7 * 4)) << 7) & (1 << 7))   \
     | ((((0x##bits##UL) >> (6 * 4)) << 6) & (1 << 6)) \
     | ((((0x##bits##UL) >> (5 * 4)) << 5) & (1 << 5)) \
     | ((((0x##bits##UL) >> (4 * 4)) << 4) & (1 << 4)) \
     | ((((0x##bits##UL) >> (3 * 4)) << 3) & (1 << 3)) \
     | ((((0x##bits##UL) >> (2 * 4)) << 2) & (1 << 2)) \
     | ((((0x##bits##UL) >> (1 * 4)) << 1) & (1 << 1)) \
     | ((((0x##bits##UL) >> (0 * 4)) << 0) & (1 << 0)))

typedef enum { HILITE,
               RESTORE } highstate;

/* menu item icon code */

#define ICON_PAD 4 /* space on either side */
typedef struct con_info
{
    /* true if this is a color icon */
    int color_icon_p;

    /* pointer to the icon data */
    Handle icon;

    /* for convient access */
    int width, height;
} icon_info_t;

typedef struct mct_res
{
    GUEST_STRUCT;
    GUEST<int16_t> n_entries;
    GUEST<MCEntry[1]> entries;
} mct_res_t;

struct mbartype
{
    GUEST_STRUCT;
    GUEST<INTEGER> nmen;
    GUEST<INTEGER[1]> mrid;
};

typedef struct
{
    muelem *startp;
    muelem *endp;
} startendpairs[2];

typedef struct table
{
    int32_t lasttick;
    int16_t count;
    struct tableentry
    {
        int16_t top;
        StringPtr name;
        mextp options;
    } entry[1];
} * tablePtr;
typedef GUEST<tablePtr> *tableHandle;

void cleanup_icon_info(icon_info_t *info);
int get_icon_info(mextp item_info, icon_info_t *info, int need_icon_p);

extern int ROMlib_sticky_menus_p;

void menu_bar_color(RGBColor *bar_color);
void menu_title_color(int16_t id, RGBColor *title_color);
void menu_bk_color(int16_t id, RGBColor *bk_color);
void menu_item_colors(int16_t id, int16_t item,
                      RGBColor *bk_color, RGBColor *name_color,
                      RGBColor *mark_color, RGBColor *command_color);

void menu_delete_entries(int16_t menu_id);
}

extern "C" int ROMlib_AppleChar;

#endif /* !_MENU_H_ */

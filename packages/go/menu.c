#include "go.h"

#include "disk.proto.h"
#include "easymenu.proto.h"
#include "filemanip.proto.h"
#include "inithotband.proto.h"
#include "menu.proto.h"
#include "misc.proto.h"
#include "view.proto.h"
#include <Menus.h>

typedef void (*onemenu[]) (void);


/* Metroworks C wants the longest array first */

/* NOTE: changes to filemenu must be reflected in go.h */
onemenu filemenu =
{
  newfolder,
  openitem,
  printitem,
  closeitem,
  saveitem,
  nothing,
  deleteselection,
  duplicate,
  getinfo,
  renameselection,
  selectiontohotband,
  nothing,
  checkfordisk,
  format,
  goeject,
  nothing,
  quitgo,
}, gomenu =
{
  aboutgo
}, editmenu =
{
  undolast,
  nothing,
  gocut,
  gocopy,
  gopaste,
  goclear,
}, viewmenu =
{
  iconview,
  icsview,
  listingview
}, sortmenu =
{
  namesort,
  moddatesort,
  sizesort
};

onemenu *menubar[] =
{
  &gomenu,
  &filemenu,
  &editmenu,
  &viewmenu,
  &sortmenu
};

/* NOTE: onlywhenselected is based upon filemenu */
int onlywhenselected[] =
{
  open_menuid,
  print_menuid,
  delete_menuid,
  duplicate_menuid,
  get_info_menuid,
  rename_menuid,
  send_to_hotband_menuid,
  eject_menuid,
};

void
domenu (long choice)
{
  Str255 apname;
  MenuHandle mh;
  short menu_id, menu_item;

  menu_id = HiWord (choice);
  menu_item = LoWord (choice);
  if (HiWord (choice) == 0)
/*-->*/ return;
  if (menu_id != FIRSTMENU)
    {
      if (menu_id == EDITMENU || !SystemEdit (menu_id - 1))
	(*menubar[menu_id - FIRSTMENU])[menu_id - 1] ();
    }
  else
    {
      if (menu_id <= NELEM (gomenu))
	{
	  gomenu[menu_id - 1] ();
	}
      else
	{
	  mh = GetMHandle (FIRSTMENU);
	  GetItem (mh, menu_id, apname);
	  OpenDeskAcc (apname);
	}
    }
  HiliteMenu (0);
}

void
showviewmenu (short enable)
{
  MenuHandle mh;

  mh = GetMHandle (VIEWMENU);
  if (enable)
    EnableItem (mh, 0);
  else
    DisableItem (mh, 0);
}

void
menuchoices (short enable)
{
#if 0
  pascal void (*f) (MenuHandle, short);
#endif /* 0 */
  short i;
  MenuHandle mh;

  mh = GetMHandle (FILEMENU);
#if 0
  if (enable)
    f = EnableItem;
  else
    f = DisableItem;
  for (i = 0; i < NELEM (onlywhenselected); i++)
    f (mh, onlywhenselected[i]);
#else /* 0 */
  for (i = 0; i < NELEM (onlywhenselected); i++)
    if (enable)
      EnableItem (mh, onlywhenselected[i]);
    else
      DisableItem (mh, onlywhenselected[i]);
#endif /* 0 */
}

void
disable_menu_item (int item)
{
  MenuHandle mh;

  mh = GetMHandle (FILEMENU);
  DisableItem (mh, item);
}

void
enable_menu_item (int item)
{
  MenuHandle mh;

  mh = GetMHandle (FILEMENU);
  EnableItem (mh, item);
}

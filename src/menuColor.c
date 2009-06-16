/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_menuColor[] =
		"$Id: menuColor.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* color menu routines added in IMV and beyond */

#include "rsys/common.h"
#include "MenuMgr.h"
#include "MemoryMgr.h"

#include "rsys/menu.h"
#include "rsys/cquick.h"

void
menu_bar_color (RGBColor *bar_color)
{
  MCEntryPtr mc_entry;
  
  mc_entry = GetMCEntry (0, 0);
  if (mc_entry)
    *bar_color = mc_entry->mctRGB4;
  else
    *bar_color = ROMlib_white_rgb_color;
}

void
menu_title_color (int16 id, RGBColor *title_color)
{
  MCEntryPtr mc_entry;

  mc_entry = GetMCEntry (id, 0);
  if (!mc_entry)
    mc_entry = GetMCEntry (0, 0);
  if (mc_entry)
    *title_color = mc_entry->mctRGB1;
  else
    *title_color = ROMlib_black_rgb_color;
}

void
menu_bk_color (int16 id, RGBColor *bk_color)
{
  MCEntryPtr mc_entry;

  mc_entry = GetMCEntry (id, 0);
  if (mc_entry)
    *bk_color = mc_entry->mctRGB4;
  else
    {
      mc_entry = GetMCEntry (0, 0);
      if (mc_entry)
	*bk_color = mc_entry->mctRGB2;
      else
	*bk_color = ROMlib_white_rgb_color;
    }
}

void
menu_item_colors (int16 id, int16 item,
		  RGBColor *bk_color, RGBColor *name_color,
		  RGBColor *mark_color, RGBColor *command_color)
{
  MCEntryPtr mc_entry;
  
  mc_entry = GetMCEntry (id, item);
  if (mc_entry)
    {
      *mark_color = mc_entry->mctRGB1;
      *name_color = mc_entry->mctRGB2;
      *command_color = mc_entry->mctRGB3;
      menu_bk_color (id, bk_color);
    }
  else
    {
      mc_entry = GetMCEntry (id, 0);
      if (mc_entry)
	{
	  *bk_color = mc_entry->mctRGB4;
	  *mark_color = *name_color = *command_color = mc_entry->mctRGB3;
	}
      else
	{
	  mc_entry = GetMCEntry (0, 0);
	  if (mc_entry)
	    {
	      *bk_color = mc_entry->mctRGB2;
	      *mark_color = *name_color = *command_color = mc_entry->mctRGB3;
	    }
	  else
	    {
	      *bk_color = ROMlib_white_rgb_color;
	      *mark_color = *name_color = *command_color = ROMlib_black_rgb_color;
	    }
	}
    }
}

void
menu_delete_entries (int16 menu_id)
{
  MCTableHandle menu_c_info;
  MCEntryPtr entries;
  int menu_c_info_size;
  int i;
  
  menu_c_info = MR (MenuCInfo);
  menu_c_info_size = GetHandleSize ((Handle) menu_c_info);
  entries = STARH (menu_c_info);
  for (i = 0; MCENTRY_ID_X (&entries[i]) != CWC (-99);)
    {
      if (MCENTRY_ID (&entries[i]) == menu_id)
	{
	  /* delete this element and shrink the handle size */
	  BlockMove ((Ptr) &entries[i + 1], (Ptr) &entries[i],
		     menu_c_info_size - ((i + 1) * sizeof *entries));
	  SetHandleSize ((Handle) menu_c_info,
			 (menu_c_info_size -= sizeof *entries));
	  /* reassign `entries', because the handle could have moved */
	  entries = STARH (menu_c_info);
	}
      else
	i ++;
    }
}

P2 (PUBLIC pascal trap, void, DelMCEntries,
    INTEGER, menu_id, INTEGER, menu_item)
{
  MCTableHandle menu_c_info;
  MCEntryPtr entries;
  int menu_c_info_size;
  int i;
  
  menu_c_info = MR (MenuCInfo);
  menu_c_info_size = GetHandleSize ((Handle) menu_c_info);
  entries = STARH (menu_c_info);
  for (i = 0; MCENTRY_ID_X (&entries[i]) != CWC (-99); i ++)
    {
      if (MCENTRY_ID (&entries[i]) == menu_id
	  && MCENTRY_ITEM (&entries[i]) == menu_item)
	{
	  /* delete this element and shrink the handle size */
	  BlockMove ((Ptr) &entries[i + 1], (Ptr) &entries[i],
		     menu_c_info_size - ((i + 1) * sizeof *entries));
	  SetHandleSize ((Handle) menu_c_info,
			 (menu_c_info_size -= sizeof *entries));

	  return;
	}
    }
}

P0 (PUBLIC pascal trap, MCTableHandle, GetMCInfo)
{
  MCTableHandle retval, menu_c_info;
  int menu_c_info_size;
  
  menu_c_info = MR (MenuCInfo);
  menu_c_info_size = GetHandleSize ((Handle) menu_c_info);
  retval = (MCTableHandle) NewHandle (menu_c_info_size);
  if (retval)
    BlockMove ((Ptr) STARH (menu_c_info), (Ptr) STARH (retval),
	       menu_c_info_size);

  return retval;
}

P1 (PUBLIC pascal trap, void, SetMCInfo,
    MCTableHandle, menu_ctab)
{
  DispMCInfo (MR (MenuCInfo));

  ZONE_SAVE_EXCURSION
    (SysZone,
     {
       Handle t;
       int size;

       size = GetHandleSize ((Handle) menu_ctab);
       t = NewHandle (size);
       BlockMove ((Ptr) STARH (menu_ctab), (Ptr) STARH (t), size);
       MenuCInfo = (MCTableHandle) RM (t);
     });
}

P1 (PUBLIC pascal trap, void, DispMCInfo,
    MCTableHandle, menu_ctab)
{
  DisposHandle ((Handle) menu_ctab);
}

P2 (PUBLIC pascal trap, MCEntryPtr, GetMCEntry,
    INTEGER, menu_id, INTEGER, menu_item)
{
  MCTableHandle menu_c_info;
  MCEntryPtr t;
  
  menu_c_info = MR (MenuCInfo);
  for (t = STARH (menu_c_info); MCENTRY_ID_X (t) != CWC (-99); t ++)
    {
      if (MCENTRY_ID (t) == menu_id
	  && MCENTRY_ITEM (t) == menu_item)
	return t;
    }
  
  return 0;
}

P2 (PUBLIC pascal trap, void, SetMCEntries,
    INTEGER, n_entries, MCTablePtr, entries)
{
  MCTableHandle menu_c_info;
  int menu_c_info_size;
  int menu_c_info_n_entries;
  int i;


  menu_c_info = MR (MenuCInfo);
  menu_c_info_size = GetHandleSize ((Handle) menu_c_info);
  gui_assert (! (menu_c_info_size % sizeof (MCEntry)));
  menu_c_info_n_entries = (menu_c_info_size / sizeof (MCEntry));
  
  for (i = 0; i < n_entries; i ++)
    {
      MCEntry *dst_entry;
      MCEntry *src_entry;
      
      src_entry = &entries[i];
      dst_entry = GetMCEntry (MCENTRY_ID (src_entry),
			      MCENTRY_ITEM (src_entry));
      if (dst_entry)
	{
	  *dst_entry = *src_entry;
	}
      else
	{
	  MCEntry *dst_entries;

	  /* insert the entry into menu_c_info */
	  SetHandleSize ((Handle) menu_c_info,
			 (menu_c_info_size += sizeof (MCEntry)));
	  
	  dst_entries = STARH (menu_c_info);
	  dst_entries[menu_c_info_n_entries]
	    = dst_entries[menu_c_info_n_entries - 1];
	  dst_entries[menu_c_info_n_entries - 1]
	    = entries[i];
	  menu_c_info_n_entries ++;
	}
    }
}

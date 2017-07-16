/* Copyright 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_menuV[] =
		"$Id: menuV.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in MenuMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MenuMgr.h"
#include "WindowMgr.h"
#include "ResourceMgr.h"
#include "MemoryMgr.h"

#include "rsys/file.h"
#include "rsys/cquick.h"
#include "rsys/menu.h"
#include "rsys/wind.h"
#include "rsys/prefs.h"

using namespace Executor;
using namespace ByteSwap;

P1(PUBLIC pascal trap, void, InitProcMenu, INTEGER, mbid)
{
  if (!MenuList)
    InitMenus();
  
#if 0
    /* NOTE:  We don't dispose this guy because it is a phoney resource */
    if (MBDFHndl)
	DisposHandle(Cx(MBDFHndl));
#endif

    /* NOTE: even though the docs imply that the low three bits of the
     * mbid should be masked off, Microsoft PowerPoint showed that
     * this is not the case.  They try to get mbid 300, and there is a
     * 300 in the resource file but not a 296, so masking causes the
     * GetResource to fail.  A small test program then confirmed that
     * the bits are not masked off.
     */
    MBDFHndl = RM(GetResource(TICK("MBDF"), mbid));
    HxX(MENULIST, mufu) = BigEndianValue(mbid);
    MBDFCALL(mbInit, 0, 0L);
}

P0(PUBLIC pascal trap, LONGINT, MenuChoice)
{
    return Cx(MenuDisable);
}

P3(PUBLIC pascal trap, void, GetItemCmd, MenuHandle, mh, INTEGER, item,
								  CHAR *, cmdp)
{
    mextp mep;
    
    if ((mep = ROMlib_mitemtop(mh, item, (StringPtr *) 0)))
        *cmdp = BigEndianValue((unsigned short) (unsigned char) mep->mkeyeq);
}

P3(PUBLIC pascal trap, void, SetItemCmd, MenuHandle, mh, INTEGER, item,
								     CHAR, cmd)
{
    mextp mep;
    
    if ((mep = ROMlib_mitemtop(mh, item, (StringPtr *) 0))) {
        mep->mkeyeq = cmd;
        CalcMenuSize(mh);
    }
}

P4(PUBLIC pascal trap, LONGINT, PopUpMenuSelect, MenuHandle, mh, INTEGER, top,
						  INTEGER, left, INTEGER, item)
{
    Point p;
    Rect saver;
    INTEGER tempi;
    LONGINT where;
    RgnHandle saveclip;
    int count;
    
/*
 * The following call to ROMlib_destroy_blocks is only here because
 * CompuServe Information Manager creates code on the fly right before
 * calling PopUpMenuSelect.  How does this work on a Quadra?  I suspect
 * that the particular code in question isn't hit if color QuickDraw is
 * present (they call Gestalt for "qd  " before doing this goofy stuff).
 */
    if (ROMlib_flushoften)
	ROMlib_destroy_blocks(0, ~0, TRUE);	/* For CIM 2.1.4 */
    p.h = top; /* MacWriteII seems to use these in this fashion */
    p.v = left;

    /* ### what to do if `mh' is empty -- is this correct? */
    count = CountMItems (mh);
    
#if 0
    /* if we blow off empty menus, then ClarisImpact custom
       pulldown/popup menus don't come up */
    if (! count)
      return (int32) Hx (mh, menuID) << 16;
#endif
    
    /* if item is zero, it means no menu item was previously selected,
       and a `default' should be chosen */
    if (! item)
      item = 1;
    else if (item > count)
      {
	warning_unexpected ("menu item exceeds number of items in menu");
	item = count;
      }
    
    tempi = item;
    THEPORT_SAVE_EXCURSION
      (MR (wmgr_port),
       {
	 tempi = BigEndianValue (tempi);
	 MENUCALL (mPopUpRect, mh, &saver, p, &tempi);
	 TopMenuItem = tempi;
	 where = ROMlib_mentosix (Hx (mh, menuID));
	 
	 MBDFCALL (mbSave, where, (LONGINT) (long) &saver);
	 
	 saveclip = PORT_CLIP_REGION_X (thePort); /* ick */
	 PORT_CLIP_REGION_X (thePort) = RM (NewRgn ());
	 RectRgn (PORT_CLIP_REGION (thePort), &saver);
	 MENUCALL (mDrawMsg, mh, &saver, p, (INTEGER *) 0);
	 DisposeRgn (PORT_CLIP_REGION (thePort));
	 PORT_CLIP_REGION_X (thePort) = saveclip;
	 MBDFCALL (mbSaveAlt, 0, where);
       });
    return ROMlib_menuhelper (mh, &saver, where, TRUE, 1);
}

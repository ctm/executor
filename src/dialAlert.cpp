/* Copyright 1986, 1988, 1989, 1990, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialAlert[] =
	    "$Id: dialAlert.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "ResourceMgr.h"
#include "WindowMgr.h"
#include "DialogMgr.h"
#include "EventMgr.h"
#include "MemoryMgr.h"

#include "OSUtil.h"
#include "rsys/itm.h"
#include "rsys/resource.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/options.h"
#include "rsys/dial.h"

using namespace Executor;
using namespace ByteSwap;

int16 alert_extra_icon_id = -32768;

static icon_item_template_t icon_item_template =
{
  /* item count - 1 */
  CWC (0),
  CLC_NULL,
  { CWC (10), CWC (20), CWC (42), CWC (52), },
  CBC ((1 << 7) | (iconItem)),
  CBC (2),
  
  /* to be filled in later */ CWC ((short)-1),
};

P2 (PUBLIC pascal trap, INTEGER, Alert, INTEGER, id,		/* IMI-418 */
    ProcPtr, fp)
{
  alth ah;
  HIDDEN_Handle h;
  Handle ih;
  INTEGER n, hit, defbut;
  Handle alert_ctab_res_h;
  Handle item_ctab_res_h;
  
  if (id != Cx(ANumber))
    {
      ANumber = BigEndianValue(id);
      ACount = 0;
    }
  ah = (alth) GetResource(TICK("ALRT"), id);
  if (!ah)
    {
      BEEP (1);
      return -1;
    }
  
  LoadResource((Handle) ah);
  n = (Hx(ah, altstag) >> (4 * BigEndianValue(ACount))) & 0xF;
  ACount = BigEndianValue(BigEndianValue(ACount) + 1);
  if (BigEndianValue(ACount) > 3)
    ACount = CWC(3);
  BEEP(n & 3);
  if (! (n & 4))
    return -1;
  
  ih = GetResource (TICK ("DITL"), Hx (ah, altiid));
  if (!ih)
    return -1;
  LoadResource (ih);
  alert_ctab_res_h = ROMlib_getrestid (TICK ("actb"), Hx (ah, altiid));
  item_ctab_res_h = ROMlib_getrestid (TICK ("ictb"), Hx (ah, altiid));
  h.p = ih;
  HandToHand (&h);
  
  THEGDEVICE_SAVE_EXCURSION
    (MR (MainDevice),
     {
       Rect adjusted_rect;
       boolean_t color_p;
       DialogPeek dp;
       SignedByte flags;
       itmp ip;
       
       LOCK_HANDLE_EXCURSION_1
	 (ah,
	  {
	    dialog_compute_rect (&HxX (ah, altr),
				 &adjusted_rect,
				 (ALERT_RES_HAS_POSITION_P (ah)
				  ? ALERT_RES_POSITION (ah)
				  : noAutoCenter));
	  });
       
       color_p = ((alert_ctab_res_h
		   && CTAB_SIZE ((CTabHandle) alert_ctab_res_h) != -1)
		  || item_ctab_res_h);
       if (color_p)
	 dp = ((DialogPeek)
	       NewCDialog  (NULL, &adjusted_rect,
			    (StringPtr) "", FALSE, dBoxProc,
			    (WindowPtr) -1, FALSE, 0L, h.p));
       else
	 dp = ((DialogPeek)
	       NewDialog  (NULL, &adjusted_rect,
			   (StringPtr) "", FALSE, dBoxProc,
			   (WindowPtr) -1, FALSE, 0L, h.p));
       
       if (color_p)
	 {
	   THEPORT_SAVE_EXCURSION
	     (thePort,
	      {
		SetWinColor (DIALOG_WINDOW (dp),
			     (CTabHandle) alert_ctab_res_h);
	      });

	   if (item_ctab_res_h)
	     {
	       AuxWinHandle aux_win_h;
	       
	       aux_win_h = MR (*lookup_aux_win (DIALOG_WINDOW (dp)));
	       gui_assert (aux_win_h);
	       
	       HxX (aux_win_h, dialogCItem) = RM (item_ctab_res_h);
	     }
	 }
       
       if (alert_extra_icon_id != -32768)
	 {
	   Handle icon_item_h;
	   
	   icon_item_h = NewHandle (sizeof icon_item_template);
	   icon_item_template.res_id = BigEndianValue (alert_extra_icon_id);
	   memcpy (STARH (icon_item_h), &icon_item_template,
		   sizeof icon_item_template);
	   
	   AppendDITL ((DialogPtr) dp, icon_item_h, overlayDITL);
	   
	   alert_extra_icon_id = -32768;
	   
	   DisposHandle (icon_item_h);
	 }

       ShowWindow ((WindowPtr) dp);
       
       THEPORT_SAVE_EXCURSION
	 (FrontWindow (),
	  {
	    defbut = 1 + ((n & 8) >> 3);
	    ip = ROMlib_dpnotoip(dp, defbut, &flags);
	    if (ip)
	      {
		Rect r;
		
		r = ip->itmr;
		PenSize (3, 3);
		InsetRect (&r, -4, -4);
		if (!(ROMlib_options & ROMLIB_RECT_SCREEN_BIT))
		  FrameRoundRect(&r, 16, 16);
		else
		  FrameRect(&r);
	      }
	    dp->aDefItem = BigEndianValue(defbut);
	    ModalDialog (fp, &hit);
	  });
       HSetState (DIALOG_ITEMS (dp), flags);
       DisposDialog ((DialogPtr) dp);
     });
  return BigEndianValue (hit);
}

P2 (PUBLIC pascal trap, INTEGER, StopAlert, INTEGER, id,	/* IMI-419 */
    ProcPtr, fp)
{
  alert_extra_icon_id = stopIcon;
  return Alert (id, fp);
}

P2(PUBLIC pascal trap, INTEGER, NoteAlert, INTEGER, id,	/* IMI-420 */
								 ProcPtr, fp)
{
  alert_extra_icon_id = noteIcon;
  return Alert (id, fp);
}

P2(PUBLIC pascal trap, INTEGER, CautionAlert, INTEGER, id,	/* IMI-420 */
								 ProcPtr, fp)
{
  alert_extra_icon_id = cautionIcon;
  return Alert (id, fp);
}

namespace Executor {
  PRIVATE Handle lockres(ResType, INTEGER, BOOLEAN);
  PRIVATE void lockditl(INTEGER, BOOLEAN);
  PRIVATE void lockalert(INTEGER, BOOLEAN);
  PRIVATE void lockdialog(INTEGER, BOOLEAN);
}

A3(PRIVATE, Handle, lockres, ResType, rt, INTEGER, id, BOOLEAN, flag)
{
    Handle retval;

    SetResLoad(TRUE);  
    if ((retval = GetResource(rt, id))) {
	if (flag)
	    HNoPurge(retval);
	else
	    HPurge(retval);
    }
    return retval;
}

#define RESCTL  (ctrlItem|resCtrl)

A2(PRIVATE, void, lockditl, INTEGER, id, BOOLEAN, flag)
{
    Handle ih, h;
    INTEGER nitem, procid;
    itmp ip;
    
    if((ih = lockres(TICK("DITL"), id, flag))) {
	nitem = BigEndianValue(*MR(*(INTEGER **)ih));
	ip = (itmp)((INTEGER *) STARH(ih) + 1);
	while (nitem-- >= 0) {
	    if ((CB(ip->itmtype) & RESCTL) == RESCTL) {
		h = lockres(TICK("CNTL"), BigEndianValue(*(INTEGER *)(&(ip->itmlen)+1)),
									 flag);
		procid = BigEndianValue(*MR(*(INTEGER **)h)) + 8;
		lockres(TICK("CDEF"), procid >> 4, flag);
	    } else if (CB(ip->itmtype) & iconItem)
		lockres(TICK("ICON"), BigEndianValue(*(INTEGER *)(&(ip->itmlen)+1)), flag);
	    else if (CB(ip->itmtype) & picItem)
		lockres(TICK("PICT"), BigEndianValue(*(INTEGER *)(&(ip->itmlen)+1)), flag);
	    BUMPIP(ip);
	}
    }
}

A2(PRIVATE, void, lockalert, INTEGER, id, BOOLEAN, flag)
{
    alth ah;
    
    if ((ah = (alth) lockres(TICK("ALRT"), id, flag))) {
	lockditl(Hx(ah, altiid), flag);
	lockres(TICK("WDEF"), dBoxProc >> 4, flag);
	lockres(TICK("ICON"), stopIcon, flag);
	lockres(TICK("ICON"), noteIcon, flag);
	lockres(TICK("ICON"), cautionIcon, flag);
    }
}

P1(PUBLIC pascal trap, void, CouldAlert, INTEGER, id)	/* IMI-420 */
{
    lockalert(id, TRUE);
}

P1(PUBLIC pascal trap, void, FreeAlert, INTEGER, id)	/* IMI-420 */
{
    lockalert(id, FALSE);
}

A2(PRIVATE, void, lockdialog, INTEGER, id, BOOLEAN, flag)
{
    dlogh dh;
    
    if ((dh = (dlogh) lockres(TICK("DLOG"), id, flag))) {
	lockditl(Hx(dh, dlgditl), flag);
	lockres(TICK("WDEF"), Hx(dh, dlgprocid) >> 4, flag);
    }
}

P1(PUBLIC pascal trap, void, CouldDialog, INTEGER, id)	/* IMI-415 */
{
    lockdialog(id, TRUE);
}

P1(PUBLIC pascal trap, void, FreeDialog, INTEGER, id)	/* IMI-415 */
{
    lockdialog(id, FALSE);
}

/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialHandle[] =
	    "$Id: dialHandle.c 86 2005-05-25 00:47:12Z ctm $";
#endif

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

/* HLock checked by ctm on Mon May 13 17:54:22 MDT 1991 */

#include "rsys/common.h"
#include "EventMgr.h"
#include "DialogMgr.h"
#include "ControlMgr.h"
#include "MemoryMgr.h"
#include "rsys/osutil.h"
#include "OSUtil.h"
#include "ToolboxUtil.h"
#include "ToolboxEvent.h"
#include "Iconutil.h"
#include "PrintMgr.h"
#include "StdFilePkg.h"
#include "FontMgr.h"

#include "rsys/cquick.h"
#include "rsys/ctl.h"
#include "rsys/mman.h"
#include "rsys/itm.h"
#include "rsys/prefs.h"
#include "rsys/pstuff.h"
#include "rsys/stdfile.h"
#include "rsys/print.h"
#include "rsys/hook.h"
#include "rsys/executor.h"
#include "rsys/osevent.h"

using namespace Executor;

P3(PUBLIC pascal, BOOLEAN, ROMlib_myfilt, DialogPeek, dp, EventRecord *, evt,
					    INTEGER *, ith)	/* IMI-415 */
{
    itmp ip;
    ControlHandle c;
    WriteWhenType when;
    SignedByte flags;
    
    if (Cx(evt->what) == keyDown &&
	((Cx(evt->message) & 0xFF) == '\r' ||
	 (Cx(evt->message) & 0xFF) == NUMPAD_ENTER)) {
        ip = ROMlib_dpnotoip(dp, CW(*ith = dp->aDefItem), &flags);
	if (ip && (CB(ip->itmtype) & ctrlItem)) {
	    c = (ControlHandle) MR(ip->itmhand);
	    if (Hx(c, contrlVis) && U(Hx(c, contrlHilite)) != INACTIVE) {
		if ((when = ROMlib_when) != WriteNever)
		    ROMlib_WriteWhen(WriteInBltrgn);
		HiliteControl(c, inButton);
		Delay((LONGINT)5, (LONGINT *) 0);
		HiliteControl(c, 0);
		HSetState(MR(((DialogPeek) dp)->items), flags);
		ROMlib_WriteWhen(when);
    /*-->*/	    return -1;
	    }
	}
	HSetState(MR(((DialogPeek) dp)->items), flags);
    }
    return FALSE;
}

#define DIALOGEVTS  \
	       (mDownMask|mUpMask|keyDownMask|autoKeyMask|updateMask|activMask)

typedef pascal BOOLEAN (*modalprocp) (DialogPtr dial, EventRecord *evtp,
				      INTEGER *iht);

#define CALLMODALPROC(dp, evtp, ip, fp2)	\
  ROMlib_CALLMODALPROC((dp), (evtp), (ip), (modalprocp)(fp2))

static inline BOOLEAN
ROMlib_CALLMODALPROC (DialogPtr dp,
		      EventRecord *evtp, INTEGER *ip, modalprocp fp)
{
  BOOLEAN retval;

  if (fp == (modalprocp) P_ROMlib_myfilt)
    retval = C_ROMlib_myfilt((DialogPeek) dp, evtp, ip);
  else if (fp == (modalprocp) P_ROMlib_stdffilt)
    retval =  C_ROMlib_stdffilt((DialogPeek) dp, evtp, ip);
  else if (fp == (modalprocp) P_ROMlib_numsonlyfilterproc)
    retval =  C_ROMlib_numsonlyfilterproc((DialogPeek) dp, evtp, ip);
  else if (fp == (modalprocp) P_ROMlib_stlfilterproc)
    retval =  C_ROMlib_stlfilterproc((DialogPeek) dp, evtp, ip);
  else
    {
      ROMlib_hook(dial_modalnumber);
      HOOKSAVEREGS();
      retval = CToPascalCall((void*)fp, CTOP_ROMlib_myfilt, dp, evtp, ip);
      HOOKRESTOREREGS();
    }
  return retval;
}

typedef pascal void (*useritemp) (WindowPtr wp, INTEGER item);

#define CALLUSERITEM(dp, inum, temph)	\
  ROMlib_CALLUSERITEM (dp, inum, (useritemp) (temph))

static inline void
ROMlib_CALLUSERITEM (DialogPtr dp,
		     INTEGER inum, useritemp temph)
{
  if (temph == (useritemp) P_ROMlib_filebox)
    C_ROMlib_filebox((DialogPeek) dp, inum);
  else if (temph == (useritemp) P_ROMlib_circle_ok)
    C_ROMlib_circle_ok ((DialogPeek) dp, inum);
  else if (temph == (useritemp) P_ROMlib_orientation)
    C_ROMlib_orientation ((DialogPeek) dp, inum);
  else
    {
      ROMlib_hook(dial_usernumber);
      CToPascalCall((void*)temph, CTOP_ROMlib_filebox, dp, inum);
    }
}


#define _FindWindow(pt, wp)			\
  ({						\
    HIDDEN_WindowPtr __wp;			\
    int retval;					\
    						\
    retval = FindWindow (pt, &__wp);		\
    *(wp) = MR (__wp.p);			\
						\
    retval;					\
  })

#define ALLOW_MOVABLE_MODAL /* we've made so many other changes, we
			       may as well go whole hog */

#if !defined (ALLOW_MOVABLE_MODAL)

/* NOTE: the changes between #if #else and #else #endif should be very
   small, but THEGDEVICE_SAVE_EXCURSION prevents us from using #if, so
   we have a lot of replicated code.  This is scary and should be fixed. */

P2 (PUBLIC pascal trap, void, ModalDialog, ProcPtr, fp,		/* IMI-415 */
    INTEGER *, item)
{
  /*
   * The code used to save thePort and restore it at the end of the
   * function, but CALLMODALPROC expects thePort to be unchanged which
   * caused a bug in Macwrite II when size/fontsize... and clicking on
   * a size on the left.
   */
  
  THEGDEVICE_SAVE_EXCURSION
    (MR (MainDevice),
     {
       EventRecord evt;
       DialogPeek dp;
       HIDDEN_DialogPtr ndp;
       TEHandle idle;
       ProcPtr fp2;
       Point whereunswapped;
       boolean_t done;
       
       dp = (DialogPeek) FrontWindow ();
       if (dp->window.windowKind != CWC (dialogKind) &&
	   CW (dp->window.windowKind) >= 0)
	 *item = CWC (-1);
       else
	 {
	   idle = (Cx (dp->editField) == -1) ? 0 : MR (dp->textH);
	   if (fp)
	     fp2 = fp;
	   else
	     fp2 = (ProcPtr) P_ROMlib_myfilt;
	   for (done = FALSE; !done;)
	     {
	       WindowPtr temp_wp;
	       boolean_t mousedown_p;
	       int mousedown_where;
	       
	       if (idle)
		 TEIdle (idle);
	       GetNextEvent (DIALOGEVTS, &evt);
	       whereunswapped.h = CW (evt.where.h);
	       whereunswapped.v = CW (evt.where.v);
	       
	       mousedown_p = (CW (evt.what) == mouseDown);
	       
	       /* dummy initializations to keep gcc happy */
	       temp_wp = NULL;
	       mousedown_where = inContent;

	       if (mousedown_p)
		 mousedown_where = _FindWindow (whereunswapped, &temp_wp);
	       
	       if (mousedown_p
		   && mousedown_where != inMenuBar
		   && ! (mousedown_where == inContent
			 && temp_wp == (WindowPtr) dp))
		 BEEP (1);
	       else if (CALLMODALPROC ((DialogPtr) dp, &evt, item, fp2))
		 {
		   /* #### not sure what this means */
		   /* above callpascal might need to be `& 0xF0' */
		   done = TRUE;
		   break;
		 }
	       else
		 {
		   if (IsDialogEvent (&evt)
		       && DialogSelect (&evt, &ndp, item))
		     done = TRUE;
		 }
	     }
	 }
     });
}

#else /* defined (ALLOW_MOVABLE_MODAL) */

P2 (PUBLIC pascal trap, void, ModalDialog, ProcPtr, fp,		/* IMI-415 */
    INTEGER *, item)
{
  /*
   * The code used to save thePort and restore it at the end of the
   * function, but CALLMODALPROC expects thePort to be unchanged which
   * caused a bug in Macwrite II when size/fontsize... and clicking on
   * a size on the left.
   */
  
  THEGDEVICE_SAVE_EXCURSION
    (MR (MainDevice),
     {
       EventRecord evt;
       DialogPeek dp;
       HIDDEN_DialogPtr ndp;
       TEHandle idle;
       ProcPtr fp2;
       Point whereunswapped;
       boolean_t done;
       
       dp = (DialogPeek) FrontWindow ();
       if (dp->window.windowKind != CWC (dialogKind) &&
	   CW (dp->window.windowKind) >= 0)
	 *item = CWC (-1);
       else
	 {
	   idle = (Cx (dp->editField) == -1) ? 0 : MR (dp->textH);
	   if (fp)
	     fp2 = fp;
	   else
	     fp2 = (ProcPtr) P_ROMlib_myfilt;
	   for (done = FALSE; !done; )
	     {
	       WindowPtr temp_wp;
	       boolean_t mousedown_p;
	       int mousedown_where;
	       
	       if (idle)
		 TEIdle (idle);
	       GetNextEvent (DIALOGEVTS, &evt);
	       whereunswapped.h = CW (evt.where.h);
	       whereunswapped.v = CW (evt.where.v);
	       
	       mousedown_p = (CW (evt.what) == mouseDown);
	       
	       /* dummy initializations to keep gcc happy */
	       temp_wp = NULL;
	       mousedown_where = inContent;

	       if (mousedown_p)
		 mousedown_where = _FindWindow (whereunswapped, &temp_wp);
	       
	       if (CALLMODALPROC ((DialogPtr) dp, &evt, item, fp2))
		 {
		   /* #### not sure what this means */
		   /* above callpascal might need to be `& 0xF0' */
		   
		   done = TRUE;
		   break;
		 }
	       else if (mousedown_p
		   && mousedown_where != inMenuBar
		   && ! (mousedown_where == inContent
			 && temp_wp == (WindowPtr) dp))
		 BEEP (1);
	       else
		 {
		   if (IsDialogEvent (&evt)
		       && DialogSelect (&evt, &ndp, item))
		     done = TRUE;
		 }
	     }
	 }
     });
}
#endif

P1(PUBLIC pascal trap, BOOLEAN, IsDialogEvent,		/* IMI-416 */
						  EventRecord *, evt)
{
    HIDDEN_WindowPtr wp;
    DialogPeek dp;
    Point p;
    
    if (evt->what == CWC(activateEvt) || evt->what == CWC(updateEvt))
/*-->*/ return ((WindowPeek)(long)(MR(evt->message)))->windowKind == CWC(dialogKind);
    dp = (DialogPeek) FrontWindow();
    if (dp && dp->window.windowKind == CWC(dialogKind)) {
        if (dp->editField != -1)
            TEIdle(MR(dp->textH));
	p.h = CW(evt->where.h);
	p.v = CW(evt->where.v);
/*-->*/ return evt->what != CWC(mouseDown) || (FindWindow(p,
				&wp) == inContent && MR(wp.p) == (WindowPtr) dp);
    }
    return FALSE;
}

boolean_t
Executor::get_item_style_info (DialogPtr dp, int item_no,
		     uint16 *flags_return, item_style_info_t *style_info)
{
  AuxWinHandle aux_win_h;
  
  aux_win_h = MR (*lookup_aux_win (dp));
  if (aux_win_h && HxX (aux_win_h, dialogCItem))
    {
      Handle items_color_info_h;
      item_color_info_t *items_color_info, *item_color_info;
      
      items_color_info_h = HxP (aux_win_h, dialogCItem);
      items_color_info = (item_color_info_t *) STARH (items_color_info_h);
      
      item_color_info = &items_color_info[item_no - 1];
      if (item_color_info->data || item_color_info->offset)
	{
	  uint16 flags;
	  int style_info_offset;
	  
	  flags = CW (item_color_info->data);
	  style_info_offset = CW (item_color_info->offset);
	  
	  *style_info = *(item_style_info_t *) ((char *) items_color_info
						+ style_info_offset);
	  if (flags & doFontName)
	    {
	      char *font_name;
	      
	      font_name = (char *) items_color_info + CW (style_info->font);
	      GetFNum ((StringPtr) font_name, &style_info->font);
	    }
	  
	  *flags_return = flags;
	  return TRUE;
	}
    }
  return FALSE;
}

void
Executor::ROMlib_drawiptext (DialogPtr dp, itmp ip, int item_no)
{
  boolean_t restore_draw_state_p = FALSE;
  draw_state_t draw_state;
  uint16 flags;
  item_style_info_t style_info;
  Rect r;
  
  if (get_item_style_info (dp, item_no, &flags, &style_info))
    {
      draw_state_save (&draw_state);
      restore_draw_state_p = TRUE;
      
      if (flags & TEdoFont)
	TextFont (CW (style_info.font));
      if (flags & TEdoFace)
	TextFace (CB (style_info.face));
      if (flags & TEdoSize)
	TextSize (CW (style_info.size));
      if (flags & TEdoColor)
	RGBForeColor (&style_info.foreground);
#if 1
      /* NOTE: this code has been "#if 0"d out since it was first written,
	 but testing on the Mac leads me to believe that this works properly.
	 Perhaps we should only do this if we're pretending to be  running
	 System 7, but other than that possibility, I don't see a downside
	 to including this. */

      if (flags & doBColor)
	RGBBackColor (&style_info.background);
#endif
    }
  
  r = ip->itmr;
  if (CB (ip->itmtype) & statText)
    {
      Handle nh;
      LONGINT l;
      char subsrc[2], *sp;
      GUEST<Handle> *hp;
      
      *subsrc = '^';
      sp = subsrc + 1;
      nh = (Handle) MR (ip->itmhand);
      
      HandToHand (&nh);
      
      for (*sp = '0', hp = (GUEST<Handle> *) DAStrings;
	   *sp != '4'; ++*sp, hp++)
	{
	  if (hp)
	    {
	      for (l = 0; l >= 0;
		   l = Munger (nh, l,
			  (Ptr) subsrc, (LONGINT) 2, STARH (STARH (hp)) + 1,
			     (LONGINT) (unsigned char) *STARH (STARH (hp))))
		;
	    }
	}
      HLock (nh);
      TextBox (STARH (nh), GetHandleSize (nh), &r, teFlushDefault);
      HUnlock (nh);
      DisposHandle (nh);
    }
  else if (CB (ip->itmtype) & editText)
    {
      Handle text_h;
      
      text_h = MR (ip->itmhand);
      LOCK_HANDLE_EXCURSION_1
	(text_h, 
	 {
	   TextBox (STARH (text_h), GetHandleSize (text_h),
		    &r, teFlushDefault);
	 });
      
      PORT_PEN_SIZE (thePort).h = PORT_PEN_SIZE (thePort).v = CWC (1);
      InsetRect (&r, -3, -3);
      FrameRect (&r);
    }
  
  if (restore_draw_state_p)
    draw_state_restore (&draw_state);
}

void
Executor::dialog_draw_item (DialogPtr dp, itmp itemp, int itemno)
{
  if (itemp->itmtype & ctrlItem)
    {
      /* controls will already have been drawn */
    }
  else if (itemp->itmtype & (statText | editText))
    {
      Rect r;

#warning This fix helps Energy Scheming, but we really should find out the
#warning exact semantics for when we should try to draw items, different
#warning item types may have different behaviors, we also might want to
#warning look at visRgn and clipRgn.  BTW, we should also test to see
#warning whether SectRect will really write to location 0

      if (SectRect (&itemp->itmr, &dp->portRect, &r))
	ROMlib_drawiptext (dp, itemp, itemno);
    }
  else if (itemp->itmtype & iconItem)
    {
      Handle icon;
      
      icon = MR (itemp->itmhand);
      if (CICON_P (icon))
	PlotCIcon (&itemp->itmr, (CIconHandle) icon);
      else
	PlotIcon (&itemp->itmr, icon);
    }
  else if (itemp->itmtype & picItem)
    {
      DrawPicture ((PicHandle) MR (itemp->itmhand), &itemp->itmr);
    }
  else
    {
      Handle h;
      
      /* useritem */
      h = MR (itemp->itmhand);
      if (h)
	CALLUSERITEM (dp, itemno, h);
    }
}

/* #### look into having DrawDialog not draw stuff that can't be seen */

P1 (PUBLIC pascal trap, void, DrawDialog, DialogPtr, dp)	/* IMI-418 */
{
    INTEGER *intp, i, inum;
    itmp ip;
    GrafPtr gp;
    SignedByte state;
    
    if (dp)
      {
	gp = thePort;
	SetPort((GrafPtr) dp);
	if (Cx(((DialogPeek)dp)->editField) != -1)
	  TEDeactivate(MR(((DialogPeek)dp)->textH));
	DrawControls((WindowPtr) dp);
	state = HGetState(MR(((DialogPeek)dp)->items));
	HSetState(MR(((DialogPeek)dp)->items), state | LOCKBIT);
	intp = (INTEGER *) STARH(MR(((DialogPeek)dp)->items));
	ip = (itmp)(intp + 1);
	for (i = Cx (*intp), inum = 1; i-- >= 0; inum++, BUMPIP(ip))
	  {
	    dialog_draw_item (dp, ip, inum);
	  }
	if (Cx(((DialogPeek)dp)->editField) != -1)
	  TEActivate(MR(((DialogPeek)dp)->textH));
	HSetState(MR(((DialogPeek)dp)->items), state);
	SetPort(gp);
      }
}

P2(PUBLIC pascal trap, INTEGER, FindDItem, DialogPtr, dp,	/* IMIV-60 */
								   Point, pt)
{
    INTEGER *intp, i, inum;
    itmp ip;
    
    intp = (INTEGER *)STARH(MR(((DialogPeek)dp)->items));
    ip = (itmp)(intp + 1);
    for (i =Cx( *intp), inum = 0; i-- >= 0; inum++, BUMPIP(ip))
	if (PtInRect(pt, &ip->itmr))
/*-->*/	    return inum;
    return -1;
}

P2(PUBLIC pascal trap, void, UpdtDialog, DialogPtr, dp,		/* IMIV-60 */
							      RgnHandle, rgn)
{
    INTEGER *intp, i, inum;
    itmp ip;
    GrafPtr gp;
    SignedByte state;
    
    gp = thePort;
    SetPort((GrafPtr) dp);
    ShowWindow((WindowPtr) dp);
    DrawControls((WindowPtr) dp);
    state = HGetState(MR(((DialogPeek)dp)->items));
    HSetState(MR(((DialogPeek)dp)->items), state | LOCKBIT);
    intp = (INTEGER *) STARH(MR(((DialogPeek)dp)->items));
    ip = (itmp)(intp + 1);
    for (i = Cx (*intp), inum = 1; i-- >= 0; inum++, BUMPIP(ip))
      {
	if (RectInRgn(&ip->itmr, rgn))
	  {
	    dialog_draw_item (dp, ip, inum);
	  }
      }
    HSetState(MR(((DialogPeek)dp)->items), state);
    SetPort(gp);
}

P3 (PUBLIC pascal trap, BOOLEAN, DialogSelect,		/* IMI-417 */
    EventRecord *, evt, HIDDEN_DialogPtr *, dpp, INTEGER *, itemp)
{
  DialogPeek dp;
  Byte c;
  itmp ip;
  INTEGER *intp, i, iend;
  Point localp;
  GUEST<Point> glocalp;
  GrafPtr gp;
  BOOLEAN itemenabled, retval;
  SignedByte flags;
  
  dp = (DialogPeek) FrontWindow();
  retval = FALSE;
  *itemp = CWC (-1);
  switch (Cx(evt->what))
    {
    case mouseDown:
      glocalp = evt->where;
      gp = thePort;
      SetPort((GrafPtr) dp);
      GlobalToLocal(&glocalp); 
      localp = glocalp.get();
      SetPort(gp);
      intp = (INTEGER *) STARH(MR(dp->items));
      iend = Cx(*intp) + 2;
      ip = (itmp)(intp + 1);
      for (i = 0;
	   ++i != iend && !PtInRect(localp, &(ip->itmr));
	   BUMPIP(ip))
	;
      itemenabled = !(CB(ip->itmtype) & itemDisable);
      if (i == iend)
	break;
      if (CB(ip->itmtype) & editText)
	{
	  if (Cx(dp->editField) != i-1)
	    ROMlib_dpntoteh(dp, i);
	  TEClick(localp, (Cx(evt->modifiers)&shiftKey) ? TRUE : FALSE,
		  MR(dp->textH));
        }
      else if (CB(ip->itmtype) & ctrlItem)
	{
	  ControlHandle c;
	  
	  c = (ControlHandle) MR (ip->itmhand);
	  if (CTL_HILITE (c) == INACTIVE
	      || !TrackControl (c, localp,
				CTL_ACTION (c)))
	    break;
        }
      if (itemenabled)
	{
	  *itemp = CW(i);
	  retval = TRUE;
	  break;
        }
      break;
    case keyDown:
    case autoKey:
      if (Cx(dp->editField) == -1)
	break;
      c = Cx(evt->message) & 0xff;
      switch (c)
	{
	case '\t':
	  ROMlib_dpntoteh(dp, 0);
	  TESetSelect((LONGINT) 0, (LONGINT) 32767,
		      DIALOG_TEXTH (dp));
	  break;
        default:
	  TEKey (c, DIALOG_TEXTH (dp));
        }
      *itemp = CW(CW(dp->editField)+1);
      ip = ROMlib_dpnotoip(dp, CW(*itemp), &flags);
      if (ip)
	retval = !(CB(ip->itmtype) & itemDisable);
      else
	{
	  warning_unexpected ("couldn't resolve editField -- dp = %p, "
			      "CW (*itemp) = %d", dp, CW (*itemp));
	  retval = FALSE;
	}
      HSetState(MR(((DialogPeek) dp)->items), flags);
      break;
    case updateEvt:
      dp = (DialogPeek) (long) MR(evt->message);
      BeginUpdate((WindowPtr) dp);
      DrawDialog((DialogPtr) dp);
      if (dp->editField != -1)
	TEUpdate(&dp->window.port.portRect, MR(dp->textH));
      EndUpdate((WindowPtr) dp);
      break;
    case activateEvt:
      dp = (DialogPeek) (long) MR(evt->message);
      if (dp->editField != -1)
	{
	  if (Cx(evt->modifiers) & activeFlag)
	    TEActivate(MR(dp->textH));
	  else
	    TEDeactivate(MR(dp->textH));
	}
      break;
    }
  dpp->p = (DialogPtr) RM (dp);
  return retval;
}

A1(PUBLIC, void, DlgCut, DialogPtr, dp)	/* IMI-418 */
{
    if ((((DialogPeek) dp)->editField) != -1)
        TECut(MR(((DialogPeek)dp)->textH));
}

A1(PUBLIC, void, DlgCopy, DialogPtr, dp)	/* IMI-418 */
{
    if ((((DialogPeek) dp)->editField) != -1)
        TECopy(MR(((DialogPeek)dp)->textH));
}

A1(PUBLIC, void, DlgPaste, DialogPtr, dp)	/* IMI-418 */
{
    if ((((DialogPeek) dp)->editField) != -1)
        TEPaste(MR(((DialogPeek)dp)->textH));
}

A1(PUBLIC, void, DlgDelete, DialogPtr, dp)	/* IMI-418 */
{
    if ((((DialogPeek) dp)->editField) != -1)
        TEDelete(MR(((DialogPeek)dp)->textH));
}


void
Executor::BEEPER (INTEGER n)
{
  if (DABeeper) {
    if ((pascal void (*)(INTEGER))MR(DABeeper) == P_ROMlib_mysound)
      C_ROMlib_mysound((n));
    else {
      HOOKSAVEREGS();
      ROMlib_hook(dial_soundprocnumber);
      Executor::CToPascalCall((void*)(soundprocp)MR(DABeeper), CTOP_ROMlib_mysound, n);
      HOOKRESTOREREGS();
    }
  }
}

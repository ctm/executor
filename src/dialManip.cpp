/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialManip[] =
	    "$Id: dialManip.c 86 2005-05-25 00:47:12Z ctm $";
#endif

/* Forward declarations in DialogMgr.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "ControlMgr.h"
#include "DialogMgr.h"
#include "OSUtil.h"
#include "MemoryMgr.h"

#include "rsys/itm.h"
#include "rsys/wind.h"
#include "rsys/ctl.h"
#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;

namespace Executor {
  PRIVATE itmp htoip(Handle h,
	  WindowPeek *wp_return, int16 *nop_return,
	  SignedByte *flags_return);
}

P4(PUBLIC pascal trap, void, ParamText, StringPtr, p0,		/* IMI-421 */
				 StringPtr, p1, StringPtr, p2, StringPtr, p3)
{
	if (p0)
			PtrToXHand((Ptr) p0, MR(DAStrings[0]), (LONGINT)U(p0[0])+1);
	if (p1)
			PtrToXHand((Ptr) p1, MR(DAStrings[1]), (LONGINT)U(p1[0])+1);
	if (p2)
			PtrToXHand((Ptr) p2, MR(DAStrings[2]), (LONGINT)U(p2[0])+1);
	if (p3)
			PtrToXHand((Ptr) p3, MR(DAStrings[3]), (LONGINT)U(p3[0])+1);
}

A3 (PUBLIC, itmp, ROMlib_dpnotoip, DialogPeek, dp,		/* INTERNAL */
    INTEGER, itemno, SignedByte *, flags)
{
  Handle items;
  GUEST<INTEGER> *intp;
  itmp retval;
  
  items = DIALOG_ITEMS (dp);
  *flags = hlock_return_orig_state (items);
  intp = (GUEST<INTEGER> *) STARH (items);
  if (itemno <= 0 || itemno > CW (*intp) + 1)
    retval = 0;
  else
    {
      retval = (itmp) (intp + 1);
      while (--itemno)
	BUMPIP (retval);
    }
  return retval;
}

A4 (PRIVATE, itmp, htoip, Handle, h,
    WindowPeek *, wp_return, int16 *, nop_return,
    SignedByte *, flags_return)
{
  WindowPeek wp;
  GUEST<INTEGER> *ip;
  INTEGER i, nop;
  itmp retval;
  
  for (wp = MR (WindowList); wp; wp = WINDOW_NEXT_WINDOW (wp))
    {
      if (WINDOW_KIND_X (wp) == CWC (dialogKind)
	  || WINDOW_KIND (wp) < 0)
	{
	  Handle items;
	  SignedByte flags;
	  
	  items = DIALOG_ITEMS (wp);
	  flags = hlock_return_orig_state (items);
	  
	  ip = (GUEST<INTEGER> *) STARH (items);
	  retval = (itmp) (ip + 1);
	  for (i = CW (*ip) + 1, nop = 1; i --; BUMPIP (retval))
	    {
	      if (MR (retval->itmhand) == h)
		{
		  *wp_return  = wp;
		  *nop_return = nop;
		  *flags_return = flags;
		  return retval;
		}
	      nop ++;
	    }
	  HSetState (items, flags);
	}
    }
  return NULL;
}

P5(PUBLIC pascal trap, void, GetDItem, DialogPtr, dp,	/* IMI-421 */
	   INTEGER, itemno, GUEST<INTEGER> *, itype, GUEST<Handle> *, item, Rect *, r)
{
    SignedByte flags;
    itmp ip = ROMlib_dpnotoip((DialogPeek) dp, itemno, &flags);
    
    if (ip)
      {
	if (itype)
	  *itype = CW((INTEGER) ip->itmtype);
#if 0
	else
	  {
	    /* test on Mac shows unconditional write of *itype */
	    /* of course this is very rude, but that's what the Mac did
	       when we tested it.  Perhaps they've fixed that now and we
	       too should fix it.  ARGH! */
	   *(INTEGER *) (US_TO_SYN68K(itype)) = CW((INTEGER) ip->itmtype);
          }
          // but why should we duplicate this kind of bug?
#endif
        if (item) /* We didn't test what happens when item is 0 on Mac */
	  *item = ip->itmhand;
	if (r) /* test on Mac shows r will not be written if 0 */
	  *r = ip->itmr;
      }
    HSetState(MR(((DialogPeek) dp)->items), flags);
}

static void
settexth (DialogPeek dp, itmp ip, int item_no)
{
  GrafPtr current_port;
  Handle item_text_h;
  TEHandle te;
  TEPtr tep;
  int16 length;

  current_port = thePort;
  
  te = DIALOG_TEXTH (dp);
  tep = STARH (te);
  
  TEP_DEST_RECT (tep) = TEP_VIEW_RECT (tep) = ITEM_RECT (ip);
  
#if 0
  /* #### this was commented out, otherwise it's origin is unknown */
  InsetRect (TEP_VIEW_RECT (tep), -3, -3);
#endif
  
  item_text_h = ITEM_H (ip);
  length = GetHandleSize (item_text_h);
  TEP_LENGTH_X (tep) = CW (length);
  /* this is not a leak, always a duplicate */
  TEP_HTEXT_X (tep) = RM (item_text_h);
  
  /* set up the text styles */
  {
    int16 te_style_size, te_style_font;
    Style te_style_face;
    RGBColor te_style_color;
    uint16 flags;
    item_style_info_t style_info;
    
    te_style_font = PORT_TX_FONT (dp);
    te_style_face = PORT_TX_FACE (dp);
    te_style_size = PORT_TX_SIZE (dp);
    
    THEPORT_SAVE_EXCURSION
      ((GrafPtr) dp, { GetForeColor (&te_style_color); });
    
    if (get_item_style_info ((DialogPtr) dp, item_no, &flags, &style_info))
      {
	if (flags & TEdoFont)
	  te_style_font = CW (style_info.font);
	if (flags & TEdoFace)
	  te_style_face = CB (style_info.face);
	if (flags & TEdoSize)
	  te_style_size = CW (style_info.size);
	
	if (flags & TEdoColor)
	  te_style_color = style_info.foreground;
#if 0
	/* ##### */
	if (flags & doBColor)
	  ... = style_info.background;
#endif
      }
    
    /* #### this code should be somehow unified with `TESetText ()',
       although that cannot be called directly */
    
    if (TEP_STYLIZED_P (tep))
      {
	TEStyleHandle te_style;
	STHandle style_table;
	FontInfo finfo;
	GUEST<int16> tx_font_save_x, tx_size_save_x;
	GUEST<Style> tx_face_save;
	
	te_style = TE_GET_STYLE (te);
	HASSIGN_2
	  (te_style,
	   nRuns, CWC (1),
	   nStyles, CWC (1));
	
	SetHandleSize ((Handle) te_style,
		       TE_STYLE_SIZE_FOR_N_RUNS (1));
	HxX (te_style, runs[0].startChar)  = CWC (0);
	HxX (te_style, runs[0].styleIndex) = CWC (0);
	HxX (te_style, runs[1].startChar)  = CW (length + 1);
	HxX (te_style, runs[1].styleIndex) = CWC (-1);
	style_table = TE_STYLE_STYLE_TABLE (te_style);
	SetHandleSize ((Handle) style_table,
		       STYLE_TABLE_SIZE_FOR_N_STYLES (1));
	
	tx_font_save_x = PORT_TX_FONT_X (current_port);
	tx_size_save_x = PORT_TX_SIZE_X (current_port);
	tx_face_save = PORT_TX_FACE (current_port);
	
	PORT_TX_FONT_X (current_port) = CW (te_style_font);
	PORT_TX_SIZE_X (current_port) = CW (te_style_size);
	PORT_TX_FACE (current_port)   = CB (te_style_face);
	
	GetFontInfo (&finfo);
	
	PORT_TX_FONT_X (current_port) = tx_font_save_x;
	PORT_TX_SIZE_X (current_port) = tx_size_save_x;
	PORT_TX_FACE (current_port) = tx_face_save;
	
	HASSIGN_7
	  (style_table,
	   stCount, CWC (1),
	   stFont, CW (te_style_font),
	   stFace, te_style_face,
	   stSize, CW (te_style_size),
	   stColor, te_style_color,
	   stHeight, CW (CW (finfo.ascent)
			 + CW (finfo.descent)
			 + CW (finfo.leading)),
	   stAscent, finfo.ascent);
      }
    else
      {
	TEP_TX_FONT_X (tep) = CW (te_style_font);
	TEP_TX_SIZE_X (tep) = CW (te_style_size);
	TEP_TX_FACE (tep)   = CB (te_style_face);
      }
  }
  
  TEP_SEL_START_X (tep) = TEP_SEL_END_X (tep) = CWC (0);
  
  TECalText (te);
  if (WINDOW_VISIBLE_X (dp))
    TEActivate (te);
  
  DIALOG_EDIT_FIELD_X (dp) = CW (item_no - 1);
  DIALOG_EDIT_OPEN_X (dp)  = CW (! (ITEM_TYPE (ip) & itemDisable));
}

P5(PUBLIC pascal trap, void, SetDItem, DialogPtr, dp,		/* IMI-421 */
		    INTEGER, itemno, INTEGER, itype, Handle, item, Rect *, r)
{
    
    SignedByte flags;
    itmp ip = ROMlib_dpnotoip((DialogPeek) dp, itemno, &flags);

    if (ip)
      {
	ip->itmtype = CB(itype);
	ip->itmhand = RM(item);
	ip->itmr = *r;
	if (itemno - 1 == DIALOG_EDIT_FIELD (dp)
	    && (itype & editText)
            && !(itype & itemDisable))
	  {
	    TEDeactivate (DIALOG_TEXTH (dp));
	    if (item)
	      settexth ((DialogPeek) dp, ip, itemno);
	    else
	      warning_unexpected (NULL_STRING);
	  }
      }
    HSetState(MR(((DialogPeek) dp)->items), flags);
}

P2(PUBLIC pascal trap, void, GetIText, Handle, item,		/* IMI-422 */
							     StringPtr, text)
{
  Size hs;
  
  if (text)
    {
      if (!item)
	text[0] = 0;
      else
	{
	  hs = GetHandleSize(item);
	  if (hs > 255)	/* can't strassign with no leading count */
	    hs = 255;
	  text[0] = hs;
	  BlockMoveData(STARH(item), (Ptr) text+1, hs);
	}
    }
}

P2(PUBLIC pascal trap, void, SetIText, Handle, item,		/* IMI-422 */
   StringPtr, text)
{
  if (item) /* put this test in for Golf 6.0's sake */
    {
      WindowPeek wp;
      SignedByte flags;
      int16 no;
      itmp ip;
      Size hs;
      
      hs = text[0];
      /* test on Mac suggests not reallochandle */
      SetHandleSize (item, hs);
      
      /* test on Mac shows that if the size can't be set, the copy
	 isn't done, but the rest is */
      if (MemErr == CWC (noErr))
	BlockMoveData((Ptr) &text[1], STARH (item), hs);
      ip = htoip (item, &wp, &no, &flags);
      if (ip)
	{
	  THEPORT_SAVE_EXCURSION
	    ((WindowPtr) wp,
	     {
	       ROMlib_drawiptext ((DialogPtr) wp, ip, no);
	       if (no == DIALOG_EDIT_FIELD (wp) + 1)
		 {
		   TEHandle text_h;
		   
		   text_h = DIALOG_TEXTH (wp);
		   
		   TE_CARET_STATE_X (text_h) = CWC (255);
		   TESetText ((Ptr) &text[1], text[0],
			      text_h);
		 }
	       ValidRect (&ip->itmr);
	     });
	  HSetState (DIALOG_ITEMS (wp), flags);
	}
    }
}

/*
 * fields of TERec that need to be updated:
 * destRect, viewRect, selStart, selEnd, caretState, teLength, hText, nLines,
 * lineStarts
 */
 
A2(PUBLIC, void, ROMlib_dpntoteh, DialogPeek, dp, INTEGER, no)	/* INTERNAL */
{
    SignedByte flags;
    itmp ip;
    GUEST<INTEGER> *intp;
    INTEGER num;
    
    if (no == 0)
      {
	/* special case ... find next */
        intp = (GUEST<INTEGER> *) STARH (MR(dp->items));
        num =Cx (*intp) + 1;
        ip = ROMlib_dpnotoip (dp, no = Cx (dp->editField) + 1, &flags);
        do
	  {
	    if (ip)
	      BUMPIP(ip);
	    else
	      warning_unexpected ("no (inherited from editField) = %d", no);
            if (!ip || ++no > num)
	      {
                ip = (itmp)(intp + 1);
                no = 1;
	      }
	  } while (!(CB(ip->itmtype) & editText));
      }
    else
      ip = ROMlib_dpnotoip(dp, no, &flags);
    if (ip && (Cx (dp->editField) != no - 1))
      {
        if (Cx (dp->editField) != -1)
	  TEDeactivate (MR(dp->textH));
	settexth(dp, ip, no);
      }
    HSetState (MR(((DialogPeek) dp)->items), flags);
}

P4(PUBLIC pascal trap, void, SelIText, DialogPtr, dp,		/* IMI-422 */
			      INTEGER, itemno, INTEGER, start, INTEGER, stop)
{
  ROMlib_dpntoteh ((DialogPeek) dp, itemno);
  TESetSelect (start, stop, DIALOG_TEXTH (dp));
}

A0(PUBLIC, INTEGER, GetAlrtStage)	/* IMI-422 */
{
    return Cx(ACount);
}

A0(PUBLIC, void, ResetAlrtStage)	/* IMI-423 */
{
    ACount = 0;
}

P2 (PUBLIC pascal trap, void, HideDItem, DialogPtr, dp,		/* IMIV-59 */
    INTEGER, item)
{
  itmp ip;
  Rect r;
  SignedByte flags;
    
  ip = ROMlib_dpnotoip((DialogPeek) dp, item, &flags);
  if (ip && CW (ip->itmr.left) < 8192)
    {
      r = ip->itmr;
      ip->itmr.left   = CW (CW (ip->itmr.left)  + 16384);
      ip->itmr.right  = CW (CW (ip->itmr.right) + 16384);
      if (CB (ip->itmtype) & editText)
	{
	  InsetRect (&r, -3, -3);
	  if (item - 1 == DIALOG_EDIT_FIELD (dp))
	    {
	      TEDeactivate (DIALOG_TEXTH (dp));
	      DIALOG_EDIT_FIELD_X (dp) = CWC (-1);
	    }
	}
      else if (CB (ip->itmtype) & ctrlItem)
	{
	  ControlHandle ctl;
	    
	  ctl = (ControlHandle) MR (ip->itmhand);
	  CTL_VIS (ctl) = 0;
	  
	  if (item == DIALOG_ADEF_ITEM (dp))
	    InsetRect (&r, -4, -4);
	}
      THEPORT_SAVE_EXCURSION
	(dp,
	 {
	   EraseRect (&r);
	   InvalRect (&r);
	 });
    }
  HSetState (DIALOG_ITEMS (dp), flags);
}

P2 (PUBLIC pascal trap, void, ShowDItem, DialogPtr, dp,		/* IMIV-59 */
    INTEGER, item)
{
  itmp ip;
  Rect r;
  SignedByte flags;
  
  ip = ROMlib_dpnotoip ((DialogPeek) dp, item, &flags);
  if (ip && CW (ip->itmr.left) > 8192)
    {
      ip->itmr.left  = CW (CW (ip->itmr.left) - 16384);
      ip->itmr.right = CW (CW (ip->itmr.right) - 16384);
      r = ip->itmr;
      if (CB (ip->itmtype) & editText)
	{
	  InsetRect (&r, -3, -3);
	  if (item - 1 == DIALOG_EDIT_FIELD (dp))
	    TEActivate (DIALOG_TEXTH (dp));
	}
      else if (CB(ip->itmtype) & ctrlItem)
	{
	  ControlHandle ctl;
	  
	  ctl = (ControlHandle) MR (ip->itmhand);
	  CTL_VIS (ctl) = 255;
	  
	  if (item == DIALOG_ADEF_ITEM (dp))
	    InsetRect (&r, -4, -4);
	}
      THEPORT_SAVE_EXCURSION
	(dp,
	 {
	   InvalRect(&r);
	 });
    }
  if (CB (ip->itmtype) & ctrlItem)
    {
      ControlHandle ctl;
      
      ctl = (ControlHandle) MR (ip->itmhand);
      ShowControl (ctl);
    }
  HSetState (DIALOG_ITEMS (dp), flags);
}

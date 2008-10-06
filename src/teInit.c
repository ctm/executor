/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teInit[] =
	    "$Id: teInit.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "WindowMgr.h"
#include "ControlMgr.h"
#include "ToolboxUtil.h"
#include "FontMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"
#include "rsys/tesave.h"

P0 (PUBLIC pascal trap, void, TEInit)
{
  TEScrpHandle = RM (NewHandle (0));
  TEScrpLength = CWC (0);
  TEDoText = RM ((ProcPtr) P_ROMlib_dotext);
}

/* This code just does "moveql #1,d0 ; rts".  We use it because
 * DNA strider chains to the old clikLoop handler.
 */
static uint16 default_clik_loop[2] = { CWC (0x7001), CWC (0x4E75) };

P2 (PUBLIC pascal trap, TEHandle, TENew, Rect *, dst, Rect *, view)
{
  TEHandle teh;
  FontInfo finfo;
  Handle hText;
  tehiddenh temptehiddenh;
  int16 *tehlinestarts;
  int te_size;
  
  te_size = ((sizeof (TERec)
	      - sizeof TE_LINE_STARTS (teh))
	     + 4 * sizeof *TE_LINE_STARTS (teh));
  teh = (TEHandle) NewHandle (te_size);

  hText = RM (NewHandle (0));
  GetFontInfo (&finfo);
  /* zero the te record */
  memset (STARH (teh), 0, te_size);
  /* ### find out what to assign to the `selRect', `selPoint'
     and `clikStuff' fields */
  HASSIGN_15
    (teh,
     destRect, *dst,
     viewRect, *view,
     lineHeight, CW (CW (finfo.ascent)
		     + CW (finfo.descent)
		     + CW (finfo.leading)),
     fontAscent, finfo.ascent,
     active, FALSE,
     caretState, CWC (caret_invis),
     just, CWC (teFlushDefault),
     crOnly, CWC (1),
     clikLoop, (ProcPtr) RM (&default_clik_loop[0]),
     inPort, thePortX,
     txFont, PORT_TX_FONT_X (thePort),
     txFace, PORT_TX_FACE (thePort),
     txMode, PORT_TX_MODE_X (thePort),
     txSize, PORT_TX_SIZE_X (thePort),
     hText, hText);
  
  tehlinestarts = HxX (teh, lineStarts);
  tehlinestarts[0] = 0;
  tehlinestarts[1] = 0;  /* this one is only for mix & match w/mac */
  
  temptehiddenh = RM ((tehiddenh) NewHandle(sizeof(tehidden)));
  /* don't merge with line above */
  TEHIDDENHX (teh) = temptehiddenh;
  memset (STARH (TEHIDDENH (teh)), 0, sizeof (tehidden));
  
  TE_SLAM (teh);
  
  return teh;
}

P1 (PUBLIC pascal trap, void, TEDispose, TEHandle, teh)
{
  DisposHandle ((Handle) TEHIDDENH(teh));
  DisposHandle (TE_HTEXT (teh));
  DisposHandle ((Handle) teh);
}

/* Copyright 1986 - 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_teAccess[] =
	    "$Id: teAccess.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in TextEdit.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"

#include "WindowMgr.h"
#include "ControlMgr.h"
#include "TextEdit.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/cquick.h"
#include "rsys/mman.h"

using namespace Executor;

P3 (PUBLIC pascal trap, void, TESetText, Ptr, p, LONGINT, length, TEHandle, teh)
{
  TE_SLAM (teh);
  if (PtrToXHand (p, TE_HTEXT (teh), length) != noErr)
    length = 0;
#if 0
  HASSIGN_3
    (teh,
     selStart, CW (length),
     selEnd, CW (length),
     teLength, CW (length));
#else
  HxX (teh, teLength) = CW (length);
#endif
  /* ### adjust recal* fields? */
  if (TE_STYLIZED_P (teh))
    {
      TEStyleHandle te_style;
      STHandle style_table;
      FontInfo finfo;
      
      te_style = TE_GET_STYLE (teh);
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
      GetFontInfo (&finfo);
      HASSIGN_7
	(style_table,
	 stCount, CWC (1),
	 stFont, PORT_TX_FONT_X (thePort),
	 stFace, PORT_TX_FACE (thePort),
	 stSize, PORT_TX_SIZE_X (thePort),
	 stColor, ROMlib_black_rgb_color,
	 stHeight, CW (CW (finfo.ascent)
		       + CW (finfo.descent)
		       + CW (finfo.leading)),
	 stAscent, finfo.ascent);
    }
  TECalText (teh);
  TE_SLAM (teh);
}

P1 (PUBLIC pascal trap, CharsHandle, TEGetText, TEHandle, teh)
{
  return (CharsHandle) TE_HTEXT (teh);
}

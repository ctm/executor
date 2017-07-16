/* Copyright 1986, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qPen[] =
		"$Id: qPen.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;
using namespace ByteSwap;

P0(PUBLIC pascal trap, void, HidePen)
{
  if (thePortX)
    PORT_PEN_VIS_X (thePort) = BigEndianValue (PORT_PEN_VIS (thePort) - 1);
}

P0(PUBLIC pascal trap, void, ShowPen)
{
  if (thePortX)
    PORT_PEN_VIS_X (thePort) = BigEndianValue (PORT_PEN_VIS (thePort) + 1);
}

P1(PUBLIC pascal trap, void, GetPen, Point *, ptp)
{
  if (thePortX)
    *ptp = PORT_PEN_LOC (thePort);
}

P1(PUBLIC pascal trap, void, GetPenState, PenState *, ps)
{
  if (!thePortX)
    return;
  
  if (CGrafPort_p (thePort))
    {
      PixPatHandle pen_pixpat;

      ps->pnLoc = PORT_PEN_LOC (thePort);
      ps->pnSize = PORT_PEN_SIZE (thePort);
      ps->pnMode = PORT_PEN_MODE_X (thePort);
      
      pen_pixpat = CPORT_PEN_PIXPAT (theCPort);
/*
 * NOTE: it's not clear what the Mac does here.  Cotton has been
 *	 wrong about this stuff before.
 */
      if (PIXPAT_TYPE_X (pen_pixpat) == CWC (pixpat_type_orig))
/* #warning GetPenState not necessarily implemented correctly... */
	PATASSIGN (ps->pnPat, PIXPAT_1DATA (pen_pixpat));
      else
	{
	  /* high bit indicates there is a pixpat (not a pattern)
	     stored in the pnPat field */
	  ps->pnMode |= CWC (0x8000);
	  *(PixPatHandle *) &ps->pnPat[0] = pen_pixpat;
	}
    }
  else
    *ps = *(PenState *) &PORT_PEN_LOC (thePort);
}

P1(PUBLIC pascal trap, void, SetPenState, PenState *, ps)
{
  if (!thePortX)
    return;

  PORT_PEN_LOC (thePort) = ps->pnLoc;
  PORT_PEN_SIZE (thePort) = ps->pnSize;

  if (ps->pnMode & CWC (0x8000))
    {
      PORT_PEN_MODE_X (thePort) = ps->pnMode & CWC (~0x8000);
      PenPixPat (*(PixPatHandle *) &ps->pnPat[0]);
    }
  else
    {
      PORT_PEN_MODE_X (thePort) = ps->pnMode;
      PenPat (ps->pnPat);
    }
}

void
Executor::draw_state_save (draw_state_t *draw_state)
{
  GrafPtr current_port;
  
  current_port = thePort;
  
  GetPenState (&draw_state->pen_state);
  if (CGrafPort_p (current_port))
    {
      draw_state->fg_color = CPORT_RGB_FG_COLOR (current_port);
      draw_state->bk_color = CPORT_RGB_BK_COLOR (current_port);
    }
  draw_state->fg = PORT_FG_COLOR_X (current_port);
  draw_state->bk = PORT_BK_COLOR_X (current_port);
  
  draw_state->tx_font = PORT_TX_FONT_X (current_port);
  draw_state->tx_face = PORT_TX_FACE_X (current_port);
  draw_state->tx_size = PORT_TX_SIZE_X (current_port);
  draw_state->tx_mode = PORT_TX_MODE_X (current_port);
}

void
Executor::draw_state_restore (draw_state_t *draw_state)
{
  GrafPtr current_port;
  
  current_port = thePort;
  
  SetPenState (&draw_state->pen_state);
  if (CGrafPort_p (current_port))
    {
      CPORT_RGB_FG_COLOR (current_port) = draw_state->fg_color;
      CPORT_RGB_BK_COLOR (current_port) = draw_state->bk_color;
    }
  PORT_FG_COLOR_X (current_port) = draw_state->fg;
  PORT_BK_COLOR_X (current_port) = draw_state->bk;
  
  PORT_TX_FONT_X (current_port) = draw_state->tx_font;
  PORT_TX_FACE_X (current_port) = draw_state->tx_face;
  PORT_TX_SIZE_X (current_port) = draw_state->tx_size;
  PORT_TX_MODE_X (current_port) = draw_state->tx_mode;
}

P2(PUBLIC pascal trap, void, PenSize, INTEGER, w, INTEGER, h)
{
  if (thePortX)
    {
      PORT_PEN_SIZE (thePort).h = BigEndianValue (w);
      PORT_PEN_SIZE (thePort).v = BigEndianValue (h);
    }
}

P1(PUBLIC pascal trap, void, PenMode, INTEGER, m)
{
  if (thePortX)
    PORT_PEN_MODE_X (thePort) = BigEndianValue (m);
}

P1(PUBLIC pascal trap, void, PenPat, Pattern, pp)
{
  if (thePortX)
    {
      if (CGrafPort_p (thePort))
	{
	  PixPatHandle old_pen;

	  old_pen = CPORT_PEN_PIXPAT (theCPort);
	  if (PIXPAT_TYPE_X (old_pen) == CWC (pixpat_type_orig))
	    PATASSIGN (PIXPAT_1DATA (old_pen), pp);
	  else
	    {
	      PixPatHandle new_pen = NewPixPat ();
	      
	      PIXPAT_TYPE_X (new_pen) = CWC (0);
	      PATASSIGN (PIXPAT_1DATA (new_pen), pp);
	      
	      PenPixPat (new_pen);
	    }
/* #warning PenPat not currently implemented correctly... */
	}
      else
	PATASSIGN (PORT_PEN_PAT (thePort), pp);
    }
}

P0(PUBLIC pascal trap, void, PenNormal)
{
    if (thePortX) {
	PenSize(1, 1);
	PenMode(patCopy);
	PenPat(black);
    }
}

P2(PUBLIC pascal trap, void, MoveTo, INTEGER, h, INTEGER, v)
{
  if (thePortX)
    {
      PORT_PEN_LOC (thePort).h = BigEndianValue (h);
      PORT_PEN_LOC (thePort).v = BigEndianValue (v);
    }
}

P2(PUBLIC pascal trap, void, Move, INTEGER, dh, INTEGER, dv)
{
  if (thePortX)
    {
      PORT_PEN_LOC (thePort).h = BigEndianValue (BigEndianValue (PORT_PEN_LOC (thePort).h) + (dh));
      thePort->pnLoc.v = BigEndianValue (BigEndianValue (PORT_PEN_LOC (thePort).v) + (dv));
    }
}

P2(PUBLIC pascal trap, void, LineTo, INTEGER, h, INTEGER, v)
{
    Point p;
    
    if (thePortX) {
	p.h = h;
	p.v = v;
	CALLLINE(p);
	PORT_PEN_LOC (thePort).h = BigEndianValue(p.h);
	PORT_PEN_LOC (thePort).v = BigEndianValue(p.v);
    }
}

P2(PUBLIC pascal trap, void, Line, INTEGER, dh, INTEGER, dv)
{
    Point p;
    
    if (thePortX) {
	p.h = BigEndianValue (PORT_PEN_LOC (thePort).h) + dh;
	p.v = BigEndianValue (PORT_PEN_LOC (thePort).v) + dv;
	CALLLINE(p);
	PORT_PEN_LOC (thePort).h = BigEndianValue(p.h);
	PORT_PEN_LOC (thePort).v = BigEndianValue(p.v);
    }
}


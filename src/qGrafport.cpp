/* Copyright 1986-1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qGrafport[] =
	"$Id: qGrafport.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "ToolboxEvent.h"
#include "MemoryMgr.h"
#include "WindowMgr.h"
#include "SysErr.h"

#include "rsys/mman.h"
#include "rsys/cquick.h"
#include "rsys/picture.h"

using namespace Executor;
using namespace ByteSwap;

P1(PUBLIC pascal trap, void, InitGraf, Ptr, gp)
{
  PixMapHandle main_gd_pixmap;

#if defined (BINCOMPAT)
  (*(HIDDEN_Ptr *) (long) SYN68K_TO_US(a5)).p = RM (gp);
#endif /* BINCOMPAT */

  main_gd_pixmap = GD_PMAP (MR (MainDevice));

  /* screenBitsX flag bits must not be set */
  screenBitsX.baseAddr = PIXMAP_BASEADDR_X (main_gd_pixmap);
  screenBitsX.rowBytes = BigEndianValue (PIXMAP_ROWBYTES (main_gd_pixmap) /
			     PIXMAP_PIXEL_SIZE (main_gd_pixmap));
  screenBitsX.bounds = PIXMAP_BOUNDS (main_gd_pixmap);
  
#define patinit(d, s)	(*(LONGINT *)d = CLC(s), *((LONGINT *)d+1) = CLC(s))
  ZONE_SAVE_EXCURSION
    (SysZone,
     {
       patinit(white,  0x00000000);
       patinit(black,  0xffffffff);
       patinit(gray,   0xaa55aa55);
       patinit(ltGray, 0x88228822);
       patinit(dkGray, 0x77dd77dd);

       WMgrPort = RM ((WindowPtr) NewPtr (sizeof (GrafPort)));
       OpenPort (MR (WMgrPort));
    
       WMgrCPort = (CWindowPtr) RM (NewPtr (sizeof (CGrafPort)));
       OpenCPort (MR (WMgrCPort));
    
       thePortX = (GrafPtr) WMgrCPort;
       ScrnBase = screenBitsX.baseAddr;
    
       StuffHex((Ptr) arrowX.data,
		(StringPtr) ("\100000040006000700078007c007e007f"
			     "007f807c006c0046000600030003000000"));
       StuffHex((Ptr) arrowX.mask,
		(StringPtr) ("\100c000e000f000f800fc00fe00ff00ff"
			     "80ffc0ffe0fe00ef00cf00878007800380"));
       arrowX.hotSpot.h = arrowX.hotSpot.v = CWC(1);
       CrsrState = 0;
    
       RndSeed = BigEndianValue (TickCount ());
       ScrVRes = ScrHRes = CWC(72);
       ScreenRow = screenBitsX.rowBytes;
       randSeedX = CLC (1);
       QDExist = EXIST_YES;
     });
}

/*
 * ROMlib_initport does everything except play with cliprgn and visrgn.
 * It exists so that SysError can call it and maniuplate clip and
 * vis without using the memory manager.
 */

A1(PUBLIC, void, ROMlib_initport, GrafPtr, p)			/* INTERNAL */
{
  /* this is a grafport, not a cgrafport, so the flag bits must not be
     set.  when we get here, it is likely they they contain garbage,
     so initialize them before using any accesssor macros */
  p->portBits.rowBytes = CWC (0);
  
  PORT_DEVICE_X (p) = 0;
  PORT_BITS (p)     = screenBitsX;
  PORT_RECT (p)     = screenBitsX.bounds;
  PATASSIGN (PORT_BK_PAT (p),   white);
  PATASSIGN (PORT_FILL_PAT (p), black);
  PATASSIGN (PORT_PEN_PAT (p),  black);
  PORT_PEN_LOC (p).h  = PORT_PEN_LOC (p).v  = CWC (0);
  PORT_PEN_SIZE (p).h = PORT_PEN_SIZE (p).v = CWC (1);
  PORT_PEN_MODE_X (p) = CWC (patCopy);
  PORT_PEN_VIS_X (p)  = CWC (0);
  PORT_TX_FONT_X (p)  = CWC (0);
  /* txFace is a Style (signed char); don't swap */
  PORT_TX_FACE_X (p)  = 0;
  *((char *)&p->txFace + 1) = 0; /* Excel & tests show we need to do this. */
  PORT_TX_MODE_X (p)     = CWC (srcOr);
  PORT_TX_SIZE_X (p)     = CWC (0);
  PORT_SP_EXTRA_X (p)    = CWC (0);
  PORT_FG_COLOR_X (p)    = CLC (blackColor);
  PORT_BK_COLOR_X (p)    = CLC (whiteColor);
  PORT_COLR_BIT_X (p)    = CWC (0);
  PORT_PAT_STRETCH_X (p) = CWC (0);
  PORT_PIC_SAVE_X (p)    = (Handle)CLC(0);
  PORT_REGION_SAVE_X (p) = (Handle)CLC(0);
  PORT_POLY_SAVE_X (p)   = (Handle)CLC(0);
  PORT_GRAF_PROCS_X (p)  = (QDProcsPtr)CLC(0);
}

P1(PUBLIC pascal trap, void, SetPort, GrafPtr, p)
{
  if (p == NULL)
    warning_unexpected ("SetPort(NULL_STRING)");
  thePortX = RM(p);
}

P1(PUBLIC pascal trap, void, InitPort, GrafPtr, p)
{
  ROMlib_initport(p);
  SetEmptyRgn (PORT_VIS_REGION (p));
  SetEmptyRgn (PORT_CLIP_REGION (p));
  HxX (PORT_VIS_REGION (p), rgnBBox) = screenBitsX.bounds;

  SetRect (&HxX (PORT_CLIP_REGION (p), rgnBBox), -32767, -32767, 32767, 32767);
  SetPort (p);
}

P1(PUBLIC pascal trap, void, OpenPort, GrafPtr, p)
{
    PORT_VIS_REGION_X (p) = RM (NewRgn ());
    PORT_CLIP_REGION_X (p) = RM (NewRgn ());
    InitPort (p);
}

/*
 * "Five of a Kind" calls CloseWindow (x) followed by ClosePort (x).
 * That used to kill us, but checking MemErr gets around that problem.
 * I'm not sure how the Mac gets around it.
 */

P1(PUBLIC pascal trap, void, ClosePort, GrafPtr, p)
{
  DisposeRgn (PORT_VIS_REGION (p));
  if (MemErr == CWC (noErr))
    {
      DisposeRgn (PORT_CLIP_REGION (p));

      if (MemErr == CWC (noErr))
	{
	  if (CGrafPort_p (p))
	    {
	      DisposPixPat (CPORT_BK_PIXPAT (p));
	      DisposPixPat (CPORT_PEN_PIXPAT (p));
	      DisposPixPat (CPORT_FILL_PIXPAT (p));
	      DisposHandle ((Handle) CPORT_PIXMAP (p)); /* NOT DisposPixMap */
	      
	      DisposHandle ((Handle) CPORT_GRAFVARS (p));
	    }
	}
    }
}

P1(PUBLIC pascal trap, void, GetPort, HIDDEN_GrafPtr *, pp)
{
  (*pp).p = thePortX;
}

P1(PUBLIC pascal trap, void, GrafDevice, INTEGER, d)
{
  PORT_DEVICE_X (thePort) = Cx (d);
}

P1(PUBLIC pascal trap, void, SetPortBits, BitMap *, bm)
{
  if (! CGrafPort_p (thePort))
    PORT_BITS (thePort) = *bm;
}

P2(PUBLIC pascal trap, void, PortSize, INTEGER, w, INTEGER, h)
{
    PORT_RECT (thePort).bottom = BigEndianValue (BigEndianValue (PORT_RECT (thePort).top) + h);
    PORT_RECT (thePort).right  = BigEndianValue (BigEndianValue (PORT_RECT (thePort).left) + w);
}

P2 (PUBLIC pascal trap, void, MovePortTo, INTEGER, lg, INTEGER, tg)
{
  GrafPtr current_port;
  Rect *port_bounds, *port_rect;
  int w, h;

  current_port = thePort;
  port_bounds = &PORT_BOUNDS (current_port);
  port_rect = &PORT_RECT (current_port);
  lg = BigEndianValue (port_rect->left) - lg;
  tg = BigEndianValue (port_rect->top) - tg;
  w  = RECT_WIDTH (port_bounds);
  h  = RECT_HEIGHT (port_bounds);
  SetRect (port_bounds, lg, tg, lg + w, tg + h);
}

P2 (PUBLIC pascal trap, void, SetOrigin, INTEGER, h, INTEGER, v)
{
  int32 dh, dv;

  dh = h - Cx (PORT_RECT (thePort).left);
  dv = v - Cx (PORT_RECT (thePort).top);
  PIC_SAVE_EXCURSION
    ({
      int16 swappeddh;
      int16 swappeddv;
      
      PICOP (OP_Origin);
      
      swappeddh = BigEndianValue(dh);
      PICWRITE (&swappeddh, sizeof swappeddh);
      swappeddv = BigEndianValue(dv);
      PICWRITE (&swappeddv, sizeof swappeddv);
    });
  OffsetRect (&PORT_BOUNDS (thePort),   dh, dv);
  OffsetRect (&PORT_RECT (thePort),     dh, dv);
  OffsetRgn (PORT_VIS_REGION (thePort), dh, dv);
  if (SaveVisRgn)
    OffsetRgn (MR (SaveVisRgn), dh, dv);
}

P1(PUBLIC pascal trap, void, SetClip, RgnHandle, r)
{
    CopyRgn (r, PORT_CLIP_REGION (thePort));
}

P1(PUBLIC pascal trap, void, GetClip, RgnHandle, r)
{
    CopyRgn (PORT_CLIP_REGION (thePort), r);
}

P1(PUBLIC pascal trap, void, ClipRect, Rect *, r)
{
  Rect r_copy;

  /* We copy r to a local, in case it points to memory that might move
   * during the RectRgn.
   */
  r_copy = *r;
  RectRgn (PORT_CLIP_REGION (thePort), &r_copy);
}

P1(PUBLIC pascal trap, void, BackPat, Pattern, pp)
{
  if (CGrafPort_p (thePort))
    {
      PixPatHandle old_bk;

      old_bk = CPORT_BK_PIXPAT (theCPort);
      if (old_bk && PIXPAT_TYPE_X (old_bk) == pixpat_type_orig)
	PATASSIGN (PIXPAT_1DATA (old_bk), pp);
      else
	{
	  PixPatHandle new_bk = NewPixPat ();
	  
	  PIXPAT_TYPE_X (new_bk) = CWC (0);
	  PATASSIGN (PIXPAT_1DATA (new_bk), pp);
	  BackPixPat (new_bk);
	}
/*
 * On a Mac, PIXPAT_1DATA is *not* updated here, instead a new pixpat
 * is created and the other fields are set up to point to this guy.
 */

/* #warning BackPat not currently implemented properly ... */
    }
  else
    PATASSIGN (PORT_BK_PAT (thePort), pp);
}

A1 (PUBLIC, void, ROMlib_fill_pat, Pattern, pp) /* INTERNAL */
{
  if (CGrafPort_p (thePort))
    {
      PixPatHandle old_fill;

      old_fill = CPORT_FILL_PIXPAT (theCPort);
      if (PIXPAT_TYPE_X (old_fill) == CWC (pixpat_type_orig))
	PATASSIGN (PIXPAT_1DATA (old_fill), pp);
      else
	{
	  PixPatHandle new_fill = NewPixPat ();
	  
	  PIXPAT_TYPE_X (new_fill) = CWC (0);
	  PATASSIGN (PIXPAT_1DATA (new_fill), pp);
	  
	  ROMlib_fill_pixpat (new_fill);
	}
/*
 * It's not clear what we're supposed to be doing here.
 */
/* #warning ROMlib_fill_pat not currently implemented properly... */
    }
  else
    PATASSIGN (PORT_FILL_PAT (thePort), pp);
}

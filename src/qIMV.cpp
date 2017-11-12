/* Copyright 1989, 1990, 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qIMV[] =
		    "$Id: qIMV.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"

#include "rsys/pstuff.h"
#include "rsys/cquick.h"
#include "rsys/tempalloc.h"

using namespace Executor;

/* these stubs are here to make my Pic V2 code work */

P1(PUBLIC pascal trap, void, CharExtra, Fixed, Extra)	/* IMV-77 */
{
	/* TODO */
/* #warning CharExtra not implemented */
  warning_unimplemented (NULL_STRING);
}

P1(PUBLIC pascal trap, void, SetStdCProcs, CQDProcs *, cProcs) /* IMV-77 */
{
    cProcs->textProc		= RM((Ptr) P_StdText);
    cProcs->lineProc		= RM((Ptr) P_StdLine);
    cProcs->rectProc		= RM((Ptr) P_StdRect);
    cProcs->rRectProc		= RM((Ptr) P_StdRRect);
    cProcs->ovalProc		= RM((Ptr) P_StdOval);
    cProcs->arcProc		= RM((Ptr) P_StdArc);
    cProcs->polyProc		= RM((Ptr) P_StdPoly);
    cProcs->rgnProc		= RM((Ptr) P_StdRgn);
    cProcs->bitsProc		= RM((Ptr) P_StdBits);
    cProcs->commentProc		= RM((Ptr) P_StdComment);
    cProcs->txMeasProc		= RM((Ptr) P_StdTxMeas);
    cProcs->getPicProc		= RM((Ptr) P_StdGetPic);
    cProcs->putPicProc		= RM((Ptr) P_StdPutPic);
    cProcs->opcodeProc		= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc1Proc	= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc2Proc	= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc3Proc	= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc4Proc	= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc5Proc	= RM ((Ptr) 0)	/* ??? */	;
    cProcs->newProc6Proc	= RM ((Ptr) 0)	/* ??? */	;
}

P3 (PUBLIC pascal trap, void, GetCPixel, INTEGER, h, INTEGER, v,
    RGBColor *, pixelp)
{
  PixMap temp_pm;
  uint8 temp_fbuf[4];
  Rect src_rect, dst_rect;
  GrafPtr port = thePort;
  CTabHandle ctab;
  ColorSpec *cspec;
  int pixval;
  int bpp;
  
  temp_pm.baseAddr      = RM ((Ptr) temp_fbuf);
  temp_pm.bounds.top    = CWC (0);
  temp_pm.bounds.bottom = CWC (1);
  temp_pm.bounds.left   = CWC (0);
  temp_pm.bounds.right  = CWC (1);
  temp_pm.rowBytes      = CWC (4 | PIXMAP_DEFAULT_ROWBYTES);
  
  if (CGrafPort_p (port))
    {
      PixMapHandle port_pm = CPORT_PIXMAP (port);
      
      bpp = PIXMAP_PIXEL_SIZE (port_pm);
      ctab = PIXMAP_TABLE (port_pm);
    }
  else
    {
      bpp = 1;
      ctab = validate_relative_bw_ctab ();
    }
  temp_pm.pmTable = RM (ctab);
  pixmap_set_pixel_fields (&temp_pm, bpp);
  
  src_rect.top    = CW (v);
  src_rect.bottom = CW (v + 1);
  src_rect.left   = CW (h);
  src_rect.right  = CW (h + 1);

  dst_rect = temp_pm.bounds;
  
  CopyBits (PORT_BITS_FOR_COPY (thePort), (BitMap *) &temp_pm,
	    &src_rect, &dst_rect, srcCopy, NULL);
  
  if (bpp > 8)
    {
      gui_fatal ("> 8bpp unimplemented");
    }
  else
    {
      /* extract the pixel */
      pixval = (*temp_fbuf >> (8 - bpp));
      
      /* Convert that pixel to an RGB value. */
      cspec = CTAB_TABLE (ctab);
      if (CTAB_FLAGS_X (ctab) & CTAB_GDEVICE_BIT_X)
	*pixelp = cspec[pixval].rgb;
      else
	{
	  GUEST<int16> swapped_pixval;
	  int i;
      
	  /* non-device color tables aren't guaranteed to be sorted, so we
	     need to hunt for an entry with the specified value */
	  swapped_pixval = CW (pixval);
	  for (i = CTAB_SIZE (ctab); i >= 0; i--)
	    if (cspec[i].value == swapped_pixval)
	      break;
	  if (i >= 0)
	    *pixelp = cspec[i].rgb;
	  else
	    {
	      warning_unexpected ("Failed to find match in GetCPixel.");
	      pixelp->red = pixelp->green = pixelp->blue = CWC (0);
	    }
	}
    }
}					

P3 (PUBLIC pascal trap, void, SetCPixel, INTEGER, h, INTEGER, v,
    RGBColor *, pixelp)
{
  Rect temp_rect;

  GrafPtr port;
  bool cgrafport_p;
  
  RGBColor save_fg_rgb;
  GUEST<int32> save_fg;
  
  temp_rect.top    = CW (v);
  temp_rect.bottom = CW (v + 1);
  temp_rect.left   = CW (h);
  temp_rect.right  = CW (h + 1);
  
  port = thePort;
  cgrafport_p = CGrafPort_p (port);
  
  save_fg = PORT_FG_COLOR_X (port);
  if (cgrafport_p)
    save_fg_rgb = CPORT_RGB_FG_COLOR (port);
  
  RGBForeColor (pixelp);
  PenMode (patCopy);
  
  FillRect (&temp_rect, black);
  
  PORT_FG_COLOR_X (port) = save_fg;
  if (cgrafport_p)
    CPORT_RGB_FG_COLOR (port) = save_fg_rgb;
}

static int8
default_search_proc (RGBColor *rgb, GUEST<int32> *pixel)
{
  MatchRec *mr;

  mr = ptr_from_longint<MatchRec*>( GD_REF_CON (MR (TheGDevice)) );

  if (mr->red == rgb->red
      && mr->green == rgb->green
      && mr->blue == rgb->blue)
    *pixel = mr->matchData;
  else
    *pixel = CL (!CL(mr->matchData));
  return true;
}

syn68k_addr_t
default_search_proc_stub (syn68k_addr_t dummy_addr, void *dummy)
{
  void *arg1, *arg2;
  syn68k_addr_t retval;
  int8 result;

  retval = POPADDR ();

  arg2 = (void *) SYN68K_TO_US( POPADDR () );
  arg1 = (void *) SYN68K_TO_US( POPADDR () );
  
  result = default_search_proc ((RGBColor*)arg1, (GUEST<int32>*)arg2);
  WRITEUB (EM_A7, result);
  
  return retval;
}

P8 (PUBLIC pascal trap, void, SeedCFill, BitMap *, srcbp, BitMap *, dstbp,
    Rect *, srcrp, Rect *, dstrp, int16, seedh, int16, seedv,
    ProcPtr, matchprocp, int32, matchdata)
{
  MatchRec mr;
  GUEST<LONGINT> save_ref_con;
  GUEST<Handle> save_pic_handle;
  GUEST<QDProcsPtr> save_graf_procs;
  GDHandle gdev;
  RGBColor pixel;
  BitMap temp_bitmap1, temp_bitmap2;
  Rect temp_rect;
  int row_words;
  int width, height;
  void *t;

  TEMP_ALLOC_DECL (temp_bitmap1_bits);
  TEMP_ALLOC_DECL (temp_bitmap2_bits);
  
  gdev = MR (TheGDevice);

  if (!matchprocp)
    {
      matchprocp = (ProcPtr) SYN68K_TO_US (callback_install (default_search_proc_stub, NULL));
      mr.matchData = 0;
    }
  else
    {
      mr.matchData = CL (matchdata);
    }
  
  GetCPixel (seedh, seedv, &pixel);
  
  mr.red       = pixel.red;
  mr.green     = pixel.green;
  mr.blue      = pixel.blue;
  save_ref_con   = GD_REF_CON_X (gdev);
  GD_REF_CON_X (gdev) = guest_cast<int32> (RM (&mr));
  
  save_pic_handle = PORT_PIC_SAVE_X (thePort);
  save_graf_procs = PORT_GRAF_PROCS_X (thePort);
  
  PORT_PIC_SAVE_X (thePort)   = nullptr;
  PORT_GRAF_PROCS_X (thePort) = nullptr;
  GD_SEARCH_PROC_X (gdev)     = nullptr;
  AddSearch (matchprocp);
  
  width  = RECT_WIDTH (srcrp);
  height = RECT_HEIGHT (srcrp);
  
  temp_rect.top = temp_rect.left = CWC (0);
  temp_rect.right = CW (width);
  temp_rect.bottom = CW (height);

  row_words = (width + 15) / 16;
  temp_bitmap1.rowBytes = CW (row_words * 2);
  TEMP_ALLOC_ALLOCATE (t, temp_bitmap1_bits, row_words * 2 * height);
  temp_bitmap1.baseAddr = RM ((Ptr)t);
  memset (MR (temp_bitmap1.baseAddr), '\377', row_words * 2 * height);
  temp_bitmap1.bounds = temp_rect;
  
  CopyBits (srcbp, &temp_bitmap1, srcrp, &temp_rect, srcCopy, NULL);
  
  DelSearch (matchprocp);
  GD_REF_CON_X (gdev) = save_ref_con;

  temp_bitmap2 = temp_bitmap1;
  TEMP_ALLOC_ALLOCATE (t, temp_bitmap2_bits, row_words * 2 * height);
  temp_bitmap2.baseAddr = RM ((Ptr)t);
  
  SeedFill (MR (temp_bitmap1.baseAddr),
	    MR (temp_bitmap2.baseAddr),
	    row_words * 2, row_words * 2,
	    height, row_words, seedh, seedv);

  CopyBits (&temp_bitmap2, dstbp, &temp_rect, dstrp, srcCopy, NULL);
  
  PORT_PIC_SAVE_X (thePort)   = save_pic_handle;
  PORT_GRAF_PROCS_X (thePort) = save_graf_procs;

  TEMP_ALLOC_FREE (temp_bitmap1_bits);
  TEMP_ALLOC_FREE (temp_bitmap2_bits);
}

P7 (PUBLIC pascal trap, void, CalcCMask, BitMap *, srcbp, BitMap *, dstbp,
    Rect *, srcrp, Rect *, dstrp, RGBColor *, seedrgbp, ProcPtr, matchprocp,
    int32, matchdata)
{
  MatchRec mr;
  GUEST<LONGINT> save_ref_con;
  GUEST<Handle> save_pic_handle;
  GUEST<QDProcsPtr> save_graf_procs;
  GDHandle gdev;
  BitMap temp_bitmap1, temp_bitmap2;
  Rect temp_rect;
  int row_words;
  int width, height;
  void *t;
  
  TEMP_ALLOC_DECL (temp_bitmap1_bits);
  TEMP_ALLOC_DECL (temp_bitmap2_bits);
  
  gdev = MR (TheGDevice);

  if (!matchprocp)
    {
      matchprocp = (ProcPtr) SYN68K_TO_US(callback_install (default_search_proc_stub, NULL));
      mr.matchData = CL(1);
    }
  else
    {
      mr.matchData = CL (matchdata);
    }
  
  mr.red       = seedrgbp->red;
  mr.green     = seedrgbp->green;
  mr.blue      = seedrgbp->blue;
  save_ref_con   = GD_REF_CON_X (gdev);
  GD_REF_CON_X (gdev) = guest_cast<int32>( RM (&mr) );
  
  save_pic_handle = PORT_PIC_SAVE_X (thePort);
  save_graf_procs = PORT_GRAF_PROCS_X (thePort);
  
  PORT_PIC_SAVE_X (thePort)   = nullptr;
  PORT_GRAF_PROCS_X (thePort) = nullptr;
  GD_SEARCH_PROC_X (gdev)     = nullptr;
  AddSearch (matchprocp);
  
  width  = RECT_WIDTH (srcrp);
  height = RECT_HEIGHT (srcrp);
  
  temp_rect.top = temp_rect.left = CWC (0);
  temp_rect.right = CW (width);
  temp_rect.bottom = CW (height);

  row_words = (width + 15) / 16;
  temp_bitmap1.rowBytes = CW (row_words * 2);
  TEMP_ALLOC_ALLOCATE (t, temp_bitmap1_bits, row_words * 2 * height);
  temp_bitmap1.baseAddr = RM ((Ptr)t);
  memset (MR (temp_bitmap1.baseAddr), '\377', row_words * 2 * height);
  temp_bitmap1.bounds = temp_rect;
  
  CopyBits (srcbp, &temp_bitmap1, srcrp, &temp_rect, srcCopy, NULL);
  
  DelSearch (matchprocp);
  GD_REF_CON_X (gdev) = save_ref_con;

  temp_bitmap2 = temp_bitmap1;
  TEMP_ALLOC_ALLOCATE (t, temp_bitmap2_bits, row_words * 2 * height);
  temp_bitmap2.baseAddr = RM ((Ptr)t);
  
  CalcMask (MR (temp_bitmap1.baseAddr),
	    MR (temp_bitmap2.baseAddr),
	    row_words * 2, row_words * 2,
	    height, row_words);

  CopyBits (&temp_bitmap2, dstbp, &temp_rect, dstrp, srcCopy, NULL);
  
  PORT_PIC_SAVE_X (thePort)   = save_pic_handle;
  PORT_GRAF_PROCS_X (thePort) = save_graf_procs;

  TEMP_ALLOC_FREE (temp_bitmap1_bits);
  TEMP_ALLOC_FREE (temp_bitmap2_bits);
}

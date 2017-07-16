/* Copyright 1994, 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qIMVI[] =
		    "$Id: qIMVI.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"
#include "MemoryMgr.h"
#include "rsys/quick.h"
#include "rsys/pstuff.h"
#include "rsys/region.h"
#include "rsys/picture.h"

#include "sspairtable.ctable"

using namespace Executor;
using namespace ByteSwap;

/*
 * Basically a region is just a bunch of stop start pairs where the pairs
 * are the starting and stopping points of xors off the previous line.
 * As such, the meat of this loop is just to xor all the lines with their
 * immediately preceeding lines and output those values.  To start things
 * off and to finish things we need to do an xor with a line of zeros, and
 * we don't count on the bits past bounds.right necessarily containing zeros,
 * so it is necessary to temporarily hold those characters, mask them off
 * and then replace them after we've finished with the line.
 *
 * After we're done there's a chance that the region we constructed was
 * either a null region or a simple rectangle, so we detect that and adjust
 * by hand.
 *
 * NOTE: with our "special" regions, the maximum size a region can have
 *	 is 32767, unlike what IMVI-17-25 states.
 */

#define MAXRGNSIZE	32767

P2 (PUBLIC pascal trap, OSErr, BitMapToRegion, RgnHandle, rh,
    const BitMap *, bmp)
{
  INTEGER top, left, bottom, right, rowbytes, linelen, rgnsize;
  INTEGER x, y;
  unsigned char scruffmask, scruffhold0, scruffhold1;
  unsigned char *zeroline;
  unsigned char *line0p, *line1p, c, *p, *saveline0p, *saveline1p;
  INTEGER *outp, *endoutp, transition;
  BOOLEAN havewritteny;
  /* 0x00 or 0x100, depending on the state of the last xorred byte */
  unsigned int tableindex;

  if ((bmp->rowBytes & ~CWC(ROWMASK))
      && ((PixMap *)bmp)->pixelSize != CWC(1))
/*-->*/	return pixmapTooDeepErr;

  SetHandleSize((Handle) rh, MAXRGNSIZE);
  outp    = (INTEGER *) ((char *) STARH(rh) + SMALLRGN);
  endoutp = (INTEGER *) ((char *) STARH(rh) + MAXRGNSIZE);

#define OUTPUT(v)							\
  do									\
    {									\
      if (outp >= endoutp)						\
	{								\
	  /* ### set the size to something reasonable, although we	\
	     should see what the mac does */				\
	  SetHandleSize ((Handle) rh, SMALLRGN);			\
	  return rgnTooBigErr;						\
	}								\
      else								\
	*outp++ = BigEndianValue(v);						\
    } while (0)

  top      = BigEndianValue(bmp->bounds.top);
  left     = BigEndianValue(bmp->bounds.left);
  bottom   = BigEndianValue(bmp->bounds.bottom);
  right    = BigEndianValue(bmp->bounds.right);
  rowbytes = BigEndianValue(bmp->rowBytes) & ROWMASK;
  linelen  = (right - left + 7) / 8;
  if (linelen <= 0)
/*-->*/ goto it_is_empty;
    
  scruffmask = 0xFF << (8 - ((right - left) % 8));
  if (!scruffmask)
    scruffmask = 0xFF;
  zeroline = (unsigned char*)alloca (linelen);
  memset (zeroline, 0, linelen);

  line0p = zeroline;
  line1p = (unsigned char *) MR (bmp->baseAddr);
  for (y = top; y <= bottom; ++y)
    {
      if (y == bottom)
	line1p = zeroline;
      saveline0p = line0p;
      saveline1p = line1p;
      
      scruffhold0 = line0p[linelen-1];
      line0p[linelen-1] &= scruffmask;
      scruffhold1 = line1p[linelen-1];
      line1p[linelen-1] &= scruffmask;
      tableindex = 0x0;
      havewritteny = FALSE;
      for (x = left - 1; x < left + linelen * 8 - 1; x += 8)
	{
	  c = *line0p++ ^ *line1p++;
	  p = sspairtable[tableindex | c];
	  if (*p && !havewritteny)
	    {
	      OUTPUT(y);
	      havewritteny = TRUE;
	    }
	  while ((transition = *p++))
	    OUTPUT(x + transition);
	  tableindex = (c & 1) << 8;
	}
      if (tableindex)
	OUTPUT (x + 1);
      if (havewritteny)
	OUTPUT(RGNSTOP);
      saveline0p[linelen-1] = scruffhold0;
      saveline1p[linelen-1] = scruffhold1;
      line0p = saveline1p;
      line1p = saveline1p + rowbytes;
    }
  OUTPUT(RGNSTOP);
  rgnsize = (char *) outp - (char *) STARH(rh);
    switch (rgnsize)
      {
      case SMALLRGN + sizeof(INTEGER):
       it_is_empty:
	RECT_ZERO (&RGN_BBOX (rh));
	RGN_SET_SMALL (rh);
	break;
      case SMALLRGN + 9 * sizeof(INTEGER):
	outp = RGN_DATA (rh);
	HxX(rh, rgnBBox.top)    = outp[0];
	HxX(rh, rgnBBox.left)   = outp[1];
	HxX(rh, rgnBBox.bottom) = outp[4];
	HxX(rh, rgnBBox.right)  = outp[2];
	RGN_SET_SMALL (rh);
	break;
      default:
	HxX(rh, rgnBBox) = bmp->bounds;
/* #warning we are not setting the bounding box properly */
#if 1
	HxX(rh, rgnSize)       = BigEndianValue(rgnsize);
#else
	RGN_SET_SMALL (rh);
#endif
	break;
      }
  SetHandleSize((Handle) rh, RGN_SIZE (rh));
  return noErr;
}

P1 (PUBLIC pascal trap, PicHandle, OpenCPicture, OpenCPicParams *, newheaderp)
{
  PicHandle retval;

  retval = ROMlib_OpenPicture_helper (&newheaderp->srcRect, newheaderp);
  return retval;
}

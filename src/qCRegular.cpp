/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qCRegular[] =
		"$Id: qCRegular.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "CQuickDraw.h"

#include "rsys/quick.h"
#include "rsys/cquick.h"

using namespace Executor;

#define FillCxxx(CALLxxx)						\
  do									\
    {									\
      if (CGrafPort_p (thePort))					\
	{								\
	  GUEST<PixPatHandle> orig_fill_pixpat_x;			\
	  								\
	  PenMode (patCopy);						\
	  orig_fill_pixpat_x = CPORT_FILL_PIXPAT_X (thePort);		\
	  /* ROMlib_fill_pixpat (pixpat); */				\
	  CPORT_FILL_PIXPAT_X (thePort) = RM (pixpat);			\
	  CALLxxx;							\
	  CPORT_FILL_PIXPAT_X (thePort) = orig_fill_pixpat_x;		\
	}								\
      else								\
	{								\
	  PATASSIGN (PORT_FILL_PAT (thePort), PIXPAT_1DATA (pixpat));	\
	  CALLxxx;							\
	}								\
    }									\
  while (FALSE)

P2 (PUBLIC pascal trap, void, FillCRect,
    Rect *, r, PixPatHandle, pixpat)
{
/* #warning "restore settings after FillCxxx?" */
  FillCxxx (CALLRECT (fill, r));
}

P4 (PUBLIC pascal trap, void, FillCRoundRect,
    const Rect *, r,
    short, ovalWidth,
    short, ovalHeight,
    PixPatHandle, pixpat)
{
  FillCxxx (CALLRRECT (fill, (Rect *)r, ovalWidth, ovalHeight));
}

P2 (PUBLIC pascal trap, void, FillCOval,
    const Rect *, r,
    PixPatHandle, pixpat)
{
  FillCxxx (CALLOVAL (fill, (Rect *)r));
}

P4 (PUBLIC pascal trap, void, FillCArc,
    const Rect *, r,
    short, startAngle,
    short, arcAngle,
    PixPatHandle, pixpat)
{
  FillCxxx (CALLARC (fill, (Rect *)r, startAngle, arcAngle));
}

P2 (PUBLIC pascal trap, void, FillCPoly,
    PolyHandle, poly,
    PixPatHandle, pixpat)
{
  FillCxxx (CALLPOLY (fill, poly));
}

P2 (PUBLIC pascal trap, void, FillCRgn,
    RgnHandle, rgn,
    PixPatHandle, pixpat)
{
  FillCxxx (CALLRGN (fill, rgn));
}

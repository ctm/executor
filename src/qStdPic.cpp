/* Copyright 1986-1996 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStdPic[] =
	    "$Id: qStdPic.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "MemoryMgr.h"
#include "OSUtil.h"

#include "rsys/cquick.h"
#include "rsys/picture.h"
#include "rsys/print.h" 
#include "rsys/text.h"

using namespace Executor;

P3(PUBLIC pascal trap, void, StdComment, INTEGER, kind, INTEGER, size,
								  Handle, hand)
{
    SignedByte state;
    INTEGER swappedsize;

    switch (kind)
      {
      case textbegin:
	disable_stdtext ();
	break;
      case textend:
	enable_stdtext ();
	break;
      }

    kind = CW(kind);
    if (size) {
	PICSAVEBEGIN(OP_LongComment);
	PICWRITE(&kind, sizeof(kind));
	swappedsize = CW(size);
	PICWRITE(&swappedsize, sizeof(swappedsize));
	state = HGetState(hand);
	HLock(hand);
	PICWRITE(STARH(hand), size);
	if (size&1)
	    PICWRITE("", 1);
	HSetState(hand, state);
	PICSAVEEND;
    } else {
	PICSAVEBEGIN(OP_ShortComment);
	PICWRITE(&kind, sizeof(kind));
	PICSAVEEND;
    }
}

P2(PUBLIC pascal trap, void, StdGetPic, Ptr, dp, INTEGER, bc)	/* TODO */
{
  warning_unimplemented (NULL_STRING);
}

P2(PUBLIC pascal trap, void, StdPutPic, Ptr, sp, INTEGER, bc)
{
    piccachehand pch;
    PicHandle ph;
    LONGINT oldhowfar, newhowfar;
    Size newsize;

    pch = (piccachehand) PORT_PIC_SAVE (thePort);

    if (pch) {
	oldhowfar = Hx(pch, pichowfar);
	ph = HxP(pch, pichandle);
	newhowfar = Hx(pch, pichowfar) + bc;
	HxX(pch, pichowfar) = CL(newhowfar);
	if (newhowfar > 32766)
	  HxX(ph, picSize) = CWC (32766);
	else
	  HxX(ph, picSize) = CW (newhowfar);

	if (Hx(pch, pichowfar) > Hx(pch, picsize)) {
	    newsize = (Hx(pch, pichowfar) + 0xFF) & ~(LONGINT) 0xFF;
	    SetHandleSize((Handle) ph, newsize);
	    HxX(pch, picsize) = CL(newsize);
	}
	memmove((char *) STARH(ph) + oldhowfar, sp, bc);
    }
}

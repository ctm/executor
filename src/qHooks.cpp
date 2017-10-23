/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qHooks[] =
		"$Id: qHooks.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/quick.h"
#include "rsys/hook.h"
#include "rsys/print.h"
#include "rsys/options.h"

using namespace Executor;

#if defined (BINCOMPAT)

PRIVATE boolean_t text_is_enabled_p = TRUE;

PUBLIC void
Executor::disable_stdtext (void)
{
  if (ROMlib_options & ROMLIB_TEXT_DISABLE_BIT)
    text_is_enabled_p = FALSE;
}

PUBLIC void
Executor::enable_stdtext (void)
{
  text_is_enabled_p = TRUE;
}

void
Executor::ROMlib_CALLTEXT (INTEGER bc, Ptr bufp, Point num, Point den)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (INTEGER, Ptr, Point, Point);

  if (text_is_enabled_p)
    {
      if ((gp = MR (thePort->grafProcs))
	  && (pp = MR (gp->textProc)) != P_StdText)
	{
	  ROMlib_hook (q_textprocnumber);
	  HOOKSAVEREGS ();
	  CToPascalCall((void*)pp, CTOP_StdText, bc, bufp, num, den);
	  HOOKRESTOREREGS ();
	}
      else
	C_StdText (bc, bufp, num, den);
    }
}


void
Executor::ROMlib_CALLLINE (Point p)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (Point);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->lineProc)) != P_StdLine)
    {
      ROMlib_hook (q_lineprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdLine, p);
      HOOKRESTOREREGS ();
    }
  else
    C_StdLine (p);
}


void
Executor::ROMlib_CALLRECT (GrafVerb v, Rect * rp)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, Rect *);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->rectProc)) != P_StdRect)
    {
      ROMlib_hook (q_rectprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdRect, v, rp);
      HOOKRESTOREREGS ();
    }
  else
    C_StdRect (v, rp);
}


void
Executor::ROMlib_CALLOVAL (GrafVerb v, Rect * rp)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, Rect *);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->ovalProc)) != P_StdOval)
    {
      ROMlib_hook (q_ovalprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdOval, v, rp);
      HOOKRESTOREREGS ();
    }
  else
    C_StdOval (v, rp);
}


void
Executor::ROMlib_CALLRRECT (GrafVerb v, Rect * rp, INTEGER ow, INTEGER oh)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, Rect *, INTEGER, INTEGER);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->rRectProc)) != P_StdRRect)
    {
      ROMlib_hook (q_rrectprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdRRect, v, rp, ow, oh);
      HOOKRESTOREREGS ();
    }
  else
    C_StdRRect (v, rp, ow, oh);
}


void
Executor::ROMlib_CALLARC (GrafVerb v, Rect * rp, INTEGER starta, INTEGER arca)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, Rect *, INTEGER, INTEGER);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->arcProc)) != P_StdArc)
    {
      ROMlib_hook (q_arcprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdArc, v, rp, starta, arca);
      HOOKRESTOREREGS ();
    }
  else
    C_StdArc (v, rp, starta, arca);
}



void
Executor::ROMlib_CALLRGN (GrafVerb v, RgnHandle rh)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, RgnHandle);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->rgnProc)) != P_StdRgn)
    {
      ROMlib_hook (q_rgnprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdRgn, v, rh);
      HOOKRESTOREREGS ();
    }
  else
    C_StdRgn (v, rh);
}


void
Executor::ROMlib_CALLPOLY (GrafVerb v, PolyHandle rh)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (GrafVerb, PolyHandle);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->polyProc)) != P_StdPoly)
    {
      ROMlib_hook (q_polyprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdPoly, v, rh);
      HOOKRESTOREREGS ();
    }
  else
    C_StdPoly (v, rh);
}


void
Executor::ROMlib_CALLBITS (BitMap * bmp, const Rect *srcrp, const Rect *dstrp,
		 INTEGER mode, RgnHandle maskrh)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (BitMap *, Rect *, Rect *, INTEGER, RgnHandle);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->bitsProc)) != P_StdBits)
    {
      ROMlib_hook (q_bitsprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdBits, bmp, srcrp, dstrp, mode, maskrh);
      HOOKRESTOREREGS ();
    }
  else
    C_StdBits (bmp, srcrp, dstrp, mode, maskrh);
}


void
Executor::ROMlib_CALLCOMMENT (INTEGER kind, INTEGER size, Handle datah)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (INTEGER, INTEGER, Handle);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->commentProc)) != P_StdComment)
    {
      ROMlib_hook (q_commentprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdComment, kind, size, datah);
      HOOKRESTOREREGS ();
    }
  else
    C_StdComment (kind, size, datah);
}


INTEGER
Executor::ROMlib_CALLTXMEAS (INTEGER bc, Ptr bufp, GUEST<Point> * nump, GUEST<Point> * denp,
		   FontInfo * fip)
{
  QDProcsPtr gp;
  txMeasProc_t pp;
  INTEGER retval;

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->txMeasProc)) != P_StdTxMeas)
    {
      ROMlib_hook (q_txmeasprocnumber);
      HOOKSAVEREGS ();
      retval = CToPascalCall((void*)pp, CTOP_StdTxMeas, bc, bufp,
			      nump, denp, fip);
      HOOKRESTOREREGS ();
    }
  else
    retval = C_StdTxMeas (bc, bufp, nump, denp, fip);
  return retval;
}


void
Executor::ROMlib_PICWRITE (Ptr addr, INTEGER count)
{
  QDProcsPtr gp;
  pascal trap void (*pp) (Ptr, INTEGER);

  if ((gp = MR (thePort->grafProcs))
      && (pp = MR (gp->putPicProc)) != P_StdPutPic)
    {
      ROMlib_hook (q_putpicprocnumber);
      HOOKSAVEREGS ();
      CToPascalCall((void*)pp, CTOP_StdPutPic, addr, count);
      HOOKRESTOREREGS ();
    }
  else
    C_StdPutPic (addr, count);
}


#endif /* BINCOMPAT */

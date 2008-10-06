#include "ibutton.h"
#include <assert.h>

#include "ibutton.proto.h"

void
mydrawcontrol (short varcode, ControlHandle c)
{
  Rect sr, dr, *rp;
  Point loc;
  int dsize, isize, cheight, cwidth;
  Handle h;
  BitMap sbm, mbm;
  FontInfo fi;
  PenState ps;
  GrafPtr saveport;

  GetPort (&saveport);
  SetPort ((*c)->contrlOwner);
  rp = &(*c)->contrlRect;
  FrameRect (rp);
  InsetRect (rp, 1, 1);
  cheight = rp->bottom - rp->top;
  cwidth = rp->right - rp->left;

  EraseRect (rp);
  if (varcode & ICONBIT)
    {
      if (cheight < 32 || cwidth < 32)
	{
	  dsize = 16;
	  if (!(h = GetResource ('ics#', (*c)->contrlRfCon)))
	    {
	      h = GetResource ('ICN#', (*c)->contrlRfCon);
	      isize = 32;
	    }
	  else
	    isize = 16;
	}
      else
	{
	  dsize = 32;
	  isize = 32;
	  h = GetResource ('ICN#', (*c)->contrlRfCon);
	}
      HLock (h);
      sbm.baseAddr = *h;
      sbm.rowBytes = isize / 8;
      mbm.baseAddr = *h + isize * isize / 8;
      mbm.rowBytes = isize / 8;
      SetRect (&sr, 0, 0, isize, isize);
      SetRect (&sbm.bounds, 0, 0, isize, isize);
      SetRect (&mbm.bounds, 0, 0, isize, isize);
      if (varcode & TEXTBIT)
	loc.h = rp->left;
      else
	loc.h = rp->left + (cwidth - dsize) / 2;
      loc.v = rp->top + (cheight - dsize) / 2;
      SetRect (&dr, loc.h, loc.v, loc.h + dsize, loc.v + dsize);
      CopyMask (&sbm, &mbm, &(*c)->contrlOwner->portBits, &sr, &sr, &dr);
      HUnlock (h);
      loc.h = dr.right;
    }
  else
    loc.h = rp->left;

  if (varcode & TEXTBIT)
    {
      GetPenState (&ps);
      TextSize (12);
      if (StringWidth ((*c)->contrlTitle) > rp->right - loc.h)
	TextSize (9);
      GetFontInfo (&fi);
      loc.v = rp->top + (cheight + fi.ascent - fi.descent) / 2;
      MoveTo (loc.h, loc.v);
      DrawString ((*c)->contrlTitle);
      SetPenState (&ps);
    }
  if ((*c)->contrlHilite == inButton)
    InvertRect (rp);
  else if ((*c)->contrlHilite == 255)
    FrameRect (rp);
  InsetRect (rp, -1, -1);
  SetPort (saveport);
}

long
mytest (long param, ControlHandle c)
{
  Point p;

  p.v = HiWord (param);
  p.h = LoWord (param);
  if ((*c)->contrlHilite != 255)
    if (PtInRect (p, &(*c)->contrlRect))
      return (long) inButton;
  return 0L;
}

void
mycalc (RgnHandle rgn, ControlHandle c)
{
  RectRgn (rgn, &(*c)->contrlRect);
}

pascal long
main (short varcode, ControlHandle c, short message, long param)
{
  long retval;

  retval = 0;
  switch (message)
    {
    case drawCntl:
/* Ignore param, always draw the whole control */
      mydrawcontrol (varcode, c);
      break;
    case testCntl:
      retval = mytest (param, c);
      break;
    case calcCRgns:
#ifdef LIVEDANGEROUSLY
      if (param & 0x80000000)
	param &= 0x00FFFFFF
#endif /* LIVEDANGEROUSLY */
/* FALL THROUGH --> */
    case calcCntlRgn:
	mycalc ((RgnHandle) param, c);
      break;

    case initCntl:
    case dispCntl:
    case posCntl:
    case thumbCntl:
    case dragCntl:
    case autoTrack:
    case calcThumbRgn:
      break;
    default:
      assert (0);
      break;
    }
  return retval;
}

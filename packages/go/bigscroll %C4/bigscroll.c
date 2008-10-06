#include "bigscroll.h"

#include "bigscroll.proto.h"

void
getregions (ControlHandle c)
{
  Rect *r;
  bigsrgns **h;
  short height, width;

  HLock ((Handle) c);
  h = (bigsrgns **) (*c)->contrlData;
  r = &(*c)->contrlRect;
  height = r->bottom - r->top;
  width = r->right - r->left;
  if (height > width)
    {
      OpenRgn ();
      MoveTo (r->left, r->top + width / 2);
      LineTo (r->right, r->top + width / 2);
      LineTo (r->left + width / 2, r->top);
      LineTo (r->left, r->top + width / 2);
      CloseRgn ((*h)->upbutton);
      OpenRgn ();
      MoveTo (r->left, r->bottom - width / 2);
      LineTo (r->right, r->bottom - width / 2);
      LineTo (r->left + width / 2, r->bottom);
      LineTo (r->left, r->bottom - width / 2);
      CloseRgn ((*h)->downbutton);
    }
  else
    {
      OpenRgn ();
      MoveTo (r->left + height / 2, r->top);
      LineTo (r->left + height / 2, r->bottom);
      LineTo (r->left, r->top + height / 2);
      LineTo (r->left + height / 2, r->top);
      CloseRgn ((*h)->upbutton);
      OpenRgn ();
      MoveTo (r->right - height / 2, r->bottom);
      LineTo (r->right, r->top + height / 2);
      LineTo (r->right - height / 2, r->top);
      LineTo (r->right - height / 2, r->bottom);
      CloseRgn ((*h)->downbutton);
    }
  (*h)->rect = (*c)->contrlRect;
  HUnlock ((Handle) c);
}

void
draw1part (ControlHandle c, short part, RgnHandle rgn, short where, int val)
{
  RgnHandle tmprgn;
  short height, width;

  if (part == where || part == 0)
    {
      if ((*c)->contrlHilite == where)
	{
	  PaintRgn (rgn);
	}
      else
	EraseRgn (rgn);
      FrameRgn (rgn);
      if ((*c)->contrlHilite != 255 && (*c)->contrlValue != val)
	{
	  tmprgn = NewRgn ();
	  CopyRgn (rgn, tmprgn);
	  height = (*tmprgn)->rgnBBox.bottom - (*tmprgn)->rgnBBox.top;
	  width = (*tmprgn)->rgnBBox.right - (*tmprgn)->rgnBBox.left;
	  InsetRgn (tmprgn, width / 6, height / 6);
	  FrameRgn (tmprgn);
	  DisposeRgn (tmprgn);
	}
    }
}

void
drawcontrol (long part, ControlHandle c)
{
  GrafPtr saveport;

  if (!EqualRect (&(*c)->contrlRect, &(*(bigsrgns **) (*c)->contrlData)->rect))
    getregions (c);
  GetPort (&saveport);
  SetPort ((*c)->contrlOwner);
  PenNormal ();
  draw1part (c, part, (*(bigsrgns **) (*c)->contrlData)->upbutton, inUpButton,
	     (*c)->contrlMin);
  draw1part (c, part, (*(bigsrgns **) (*c)->contrlData)->downbutton, inDownButton,
	     (*c)->contrlMax);
  SetPort (saveport);
}

long
test (long param, ControlHandle c)
{
  Point p;

#if 1
  if (param)
    (*c)->contrlRfCon = param;
#endif /* 0 */
  p.v = HiWord (param);
  p.h = LoWord (param);
  if ((*c)->contrlHilite != 255)
    if (PtInRgn (p, (*(bigsrgns **) (*c)->contrlData)->downbutton))
      {
/*-->*/ return (long) inDownButton;
      }
    else if (PtInRgn (p, (*(bigsrgns **) (*c)->contrlData)->upbutton))
/*-->*/ return (long) inUpButton;
  return 0L;
}


void
calc (RgnHandle rgn, ControlHandle c)
{
  if (!EqualRect (&(*c)->contrlRect, &(*(bigsrgns **) (*c)->contrlData)->rect))
    getregions (c);
  UnionRgn ((*(bigsrgns **) (*c)->contrlData)->downbutton,
	    (*(bigsrgns **) (*c)->contrlData)->upbutton, rgn);
}

void
initbigs (ControlHandle c)
{
  bigsrgns **h;

  (*c)->contrlData = NewHandle (sizeof (bigsrgns));
  h = (bigsrgns **) (*c)->contrlData;
  (*h)->upbutton = NewRgn ();
  (*h)->downbutton = NewRgn ();
  getregions (c);
}

void
disposebigs (ControlHandle c)
{
  bigsrgns **h;

  h = (bigsrgns **) (*c)->contrlData;
  DisposeRgn ((*h)->upbutton);
  DisposeRgn ((*h)->downbutton);
  DisposHandle ((*c)->contrlData);
}

pascal long
main (short varcode, ControlHandle c, short message, long param)
{
  long retval;

  retval = 0;
  switch (message)
    {
    case drawCntl:
      drawcontrol (param, c);
      break;
    case testCntl:
      retval = test (param, c);
      break;
    case calcCRgns:
#ifdef LIVEDANGEROUSLY
      if (param & 0x80000000)
	param &= 0x00FFFFFF
#endif /* LIVEDANGEROUSLY */
/* FALL THROUGH --> */
    case calcCntlRgn:
	calc ((RgnHandle) param, c);
      break;
    case initCntl:
      initbigs (c);
      break;
    case dispCntl:
      disposebigs (c);
      break;

    case posCntl:
    case thumbCntl:
    case dragCntl:
    case autoTrack:
    case calcThumbRgn:
      break;
    default:
      break;
    }
  return retval;
}

/* TODO: check for null Ähandles */

#include "iconcontrol.h"
#include <assert.h>
#include <Packages.h>

#include "iconcontrol.proto.h"
#include "getrects.proto.h"

void
drawicxx (Ptr pat, BitMap * mbm, Rect * dest, CWindowPtr w, short size, short depth)
{
  PixMap spm;
  Rect sr;
  unsigned char state;

  SetRect (&sr, 0, 0, size, size);
  spm.baseAddr = pat;
  spm.rowBytes = (size * depth / 8);
  if (depth != 1)
    spm.rowBytes |= 0x8000;
  SetRect (&spm.bounds, 0, 0, size, size);
  spm.pmVersion = 0;
  spm.packType = 0;
  spm.packSize = 0;
  spm.hRes = spm.vRes = (Fixed) 72;
  spm.pixelType = 0;
  spm.pixelSize = depth;
  spm.cmpCount = 1;
  spm.cmpSize = depth;
  spm.planeBytes = 0;
#if 0
/* TODO: figure out the correct way to get the CTabHandle */
  spm.pmTable = (*w->portPixMap)->pmTable;
#else
  spm.pmTable = GetCTable (depth);
#endif

  SetRect (&sr, 0, 0, size, size);
  state = HGetState ((Handle) w->portPixMap);
  HLock ((Handle) w->portPixMap);
  CopyMask ((BitMap *) & spm, mbm, (BitMap *) * w->portPixMap, &sr, &sr, dest);
  HSetState ((Handle) w->portPixMap, state);
#if 1
  DisposCTable (spm.pmTable);
#endif /* 0 */
}


void
dodrawcontrol (short varcode, ControlHandle c)
{
  Rect r, dr, tr;
  Handle mask, data;
  GrafPtr saveport;
  short textsize, depth, size, view, textfont;
  BitMap mbm;
  item **ih;
  unsigned char mstate, dstate;
  Str255 s;
  RgnHandle save_clip_rgnh, new_clip_rgnh;
#if defined(YELLOW_HILITE)
  RGBColor blendcolor, hilite_color;

  blendcolor.red = blendcolor.green = 32767;
  blendcolor.blue = 65535;
  hilite_color.red = hilite_color.green = 65535;
  hilite_color.blue = 0;
#else
  RGBColor blendcolor, hilite_color;

  blendcolor.red = 45000;
  blendcolor.green = 32767;
  blendcolor.blue = 65535;
  hilite_color.red = 0;
  hilite_color.green = 65535;
  hilite_color.blue = 0;
#endif

  GetPort (&saveport);
  SetPort ((*c)->contrlOwner);
  save_clip_rgnh = NewRgn ();
  new_clip_rgnh = NewRgn ();
  GetClip (save_clip_rgnh);
  r = (*c)->contrlOwner->portRect;
  r.right -= 15;
  RectRgn (new_clip_rgnh, &r);
  SectRgn (new_clip_rgnh, save_clip_rgnh, new_clip_rgnh);
  SetClip (new_clip_rgnh);

  ih = (item **) (*c)->contrlData;
  getrects (c, &tr, &dr);
  EraseRect (&tr);
  InsetRect (&dr, -1, -1);
  EraseRect (&dr);
  InsetRect (&dr, 1, 1);
  view = (*ih)->view;
  if (view == ICSVIEW)
    {
      data = (*(*ih)->iconfam)->ics8;
      if (data == 0 || (*(*theGDevice)->gdPMap)->pixelSize < 8)
	{
	  data = (*(*ih)->iconfam)->ics4;
	  if (data == 0 || (*(*theGDevice)->gdPMap)->pixelSize < 4)
	    {
	      data = (*(*ih)->iconfam)->icssh;
	      depth = 1;
	      size = 16;
	    }
	  else
	    {
	      depth = 4;
	      size = 16;
	    }
	}
      else
	{
	  depth = 8;
	  size = 16;
	}
      mask = (*(*ih)->iconfam)->icssh;
      assert ((mask == 0) ^ (data != 0));
    }
  if (view == ICONVIEW || (view == ICSVIEW && data == 0))
    {
      data = (*(*ih)->iconfam)->icl8;
      if (data == 0 || (*(*theGDevice)->gdPMap)->pixelSize < 8)
	{
	  data = (*(*ih)->iconfam)->icl4;
	  if (data == 0 || (*(*theGDevice)->gdPMap)->pixelSize < 4)
	    {
	      data = (*(*ih)->iconfam)->icnsh;
	      assert (data != 0);
	      depth = 1;
	      size = 32;
	    }
	  else
	    {
	      depth = 4;
	      size = 32;
	    }
	}
      else
	{
	  depth = 8;
	  size = 32;
	}
      mask = (*(*ih)->iconfam)->icnsh;
      assert (mask != 0);
    }

  if (view != LISTVIEW)
    {
      dstate = HGetState (data);
      HLock (data);
      mstate = HGetState (mask);
      HLock (mask);
      mbm.baseAddr = *mask + size * size / 8;
      mbm.rowBytes = size / 8;
      SetRect (&mbm.bounds, 0, 0, size, size);
      drawicxx (*data, &mbm, &dr, (CWindowPtr) (*c)->contrlOwner, size, depth);
      if ((*c)->contrlHilite == inButton)
	{
#if 0
	  CopyBits (&mbm, &(*c)->contrlOwner->portBits, &mbm.bounds, &dr, srcXor, (RgnHandle) 0);
#else
	  PenNormal ();
	  RGBForeColor (&hilite_color);
	  BackColor (whiteColor);
	  PenMode (blend);
	  OpColor (&blendcolor);
	  PaintOval (&dr);
	  PenNormal ();
	  ForeColor (blackColor);
	  BackColor (whiteColor);
#endif
	}
      else if ((*c)->contrlHilite == INSELECTEDICON)
	{
#if 0
	  OpColor (halfwhite);
	  hiliteMode &= ~(1 << hiliteBit);
	  InvertRect (&dr);
#else
	  PenNormal ();
	  RGBForeColor (&hilite_color);
	  BackColor (whiteColor);
	  PenMode (blend);
	  OpColor (&blendcolor);
	  PaintOval (&dr);
	  PenNormal ();
	  ForeColor (blackColor);
	  BackColor (whiteColor);
#endif
	}
      HSetState (data, dstate);
      HSetState (mask, mstate);
    }

  textsize = (*c)->contrlOwner->txSize;
  textfont = (*c)->contrlOwner->txFont;
  TextSize (FONTSIZE);
  TextFont (applFont);
/* TODO: 8 should be figured out with getfontinfo */
  if (view == LISTVIEW)
    {
      MoveTo (tr.left + ICSSIZE, tr.top + 8);
      DrawString ((*c)->contrlTitle);
      MoveTo (tr.left + SIZETEXTLEFT, tr.top + 8);
      NumToString ((*ih)->size, s);
      DrawString (s);
      MoveTo (tr.left + DATETEXTLEFT, tr.top + 8);
      IUDateString ((*ih)->moddate, abbrevDate, s);
      DrawString (s);
    }
  else
    {
      MoveTo (tr.left, tr.top + 8);
      DrawString ((*c)->contrlTitle);
    }
  TextSize (textsize);
  TextFont (textfont);

  if ((*(item **) (*c)->contrlData)->selected)
    {
      FrameRect (&dr);
      FrameRect (&tr);
    }
  SetClip (save_clip_rgnh);
  DisposeRgn (save_clip_rgnh);
  DisposeRgn (new_clip_rgnh);
  SetPort (saveport);
}

long
dotest (long param, ControlHandle c)
{
  Point p;
  Rect ir, tr;

  p.v = HiWord (param);
  p.h = LoWord (param);
  getrects (c, &tr, &ir);
  if ((*c)->contrlHilite != 255)
    if (PtInRect (p, &ir))
      if ((*(item **) (*c)->contrlData)->selected)
	return (long) INSELECTEDICON;
      else
	return (long) inButton;
  if (PtInRect (p, &tr))
    return (long) INTEXT;
  return 0L;
}

void
docalc (RgnHandle rgn, ControlHandle c)
{
  RgnHandle rgn1, rgn2;
  Rect ir, tr;

  rgn1 = NewRgn ();
  rgn2 = NewRgn ();
  getrects (c, &tr, &ir);
  RectRgn (rgn1, &ir);
  RectRgn (rgn2, &tr);
  UnionRgn (rgn1, rgn2, rgn);
  DisposeRgn (rgn1);
  DisposeRgn (rgn2);
}

void
dodispose (ControlHandle c)
{
  DisposHandle ((Handle) (*c)->contrlData);
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
      dodrawcontrol (varcode, c);
      break;
    case testCntl:
      retval = dotest (param, c);
      break;
    case calcCRgns:
#ifdef LIVEDANGEROUSLY
      if (param & 0x80000000)
	param &= 0x00FFFFFF
#endif /* LIVEDANGEROUSLY */
/* FALL THROUGH --> */
    case calcCntlRgn:
	docalc ((RgnHandle) param, c);
      break;
    case dispCntl:
      dodispose (c);
      break;
    case autoTrack:
      break;

    case initCntl:
    case posCntl:
    case thumbCntl:
    case dragCntl:
    case calcThumbRgn:
      break;
    default:
      assert (0);
      break;
    }
  return retval;
}

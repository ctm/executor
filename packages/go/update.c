#include "go.h"
#include "display.h"

#include "update.proto.h"
#include "inithotband.proto.h"
#include "misc.proto.h"

void
drawpartialgrowicon (WindowPtr wp, int update_flag)
{
  RgnHandle rgn;

  DrawGrowIcon (wp);
  rgn = NewRgn ();
  SetRectRgn (rgn, wp->portRect.left, wp->portRect.bottom - SCROLLBARWIDTH + 1,
	      wp->portRect.right - SCROLLBARWIDTH + 1,
	      wp->portRect.bottom - SCROLLBARWIDTH + 3);
  EraseRgn (rgn);
  if (update_flag)
    UpdtControl (wp, rgn);
  DisposeRgn (rgn);
}

void
doactivate (EventRecord * ev)
{
  WindowPtr wp;

  wp = (WindowPtr) ev->message;
  if (ev->modifiers & activeFlag)
    {
      SetPort (wp);
    }
  if (wp == g_hotband)
    {
/* TODO: hilite the controls */
    }
  else
    {
      drawpartialgrowicon (wp, true);
      if (ev->modifiers & activeFlag)
	ShowControl ((*(opendirinfo **) ((WindowPeek) wp)->refCon)->sbar);
      else
	HideControl ((*(opendirinfo **) ((WindowPeek) wp)->refCon)->sbar);
    }
}

void
doupdate (EventRecord *ev)
{
  GrafPtr saveport;
  WindowPtr wp;

  wp = (WindowPtr) ev->message;

  GetPort (&saveport);
  SetPort (wp);
  BeginUpdate (wp);
  if (wp == g_hotband)
    {
      if (((WindowPeek) wp)->refCon & WIPEBIT)
	{
	  EraseRect (&wp->portRect);
	  ((WindowPeek) wp)->refCon ^= WIPEBIT;
	}
    }
  else
    {
      EraseRect (&wp->portRect);
      drawpartialgrowicon (wp, true);
    }
  UpdtControl (wp, wp->visRgn);
  EndUpdate (wp);
  SetPort (saveport);
}

void
changewindow (Str255 s, long dirid, short vrefnum,
	      void (*f) (WindowPeek, Str255, long, short))
{
  WindowPeek wp;

#ifdef THINK_C
  for (wp = WindowList; wp != 0; wp = wp->nextWindow)
    {
#else
  for (wp = LMGetWindowList (); wp != 0; wp = wp->nextWindow)
    {
#endif
      if ((WindowPtr) wp != g_hotband &&
	  (*(opendirinfo **) wp->refCon)->iodirid == dirid &&
	  (*(opendirinfo **) wp->refCon)->vrefnum == vrefnum)
	{
	  f (wp, s, dirid, vrefnum);
	  straightenwindow ((WindowPtr) wp);
	}
    }
}

/* TODO: put this in a better file */
short actiontoband (short action);
short
actiontoband (short action)
{
  switch (action)
    {
    case LAUNCH:
      return APPBAND;
      break;
    case OPENDIR:
      return FOLDERBAND;
      break;
    case LAUNCHCREATOR:
      return DOCBAND;
      break;
    case OPENDA:
      return DABAND;
      break;
    case NOACTION:
      return FONTBAND;
      break;
    default:
      return DOCBAND;
      break;
    }
}

void
changehot (ControlHandle c, long todir, short tovol)
{
  ControlHandle (**chh)[], c2;
  item **ih;
  short i, j, whichband, fromvol;
  long fromdir;
  bandinfo *p;
  Str255 s;

  ih = (item **) (*c)->contrlData;
  fromdir = (*ih)->ioparid;
  fromvol = (*ih)->vrefnum;
  mystr255copy (s, (*c)->contrlTitle);
  whichband = actiontoband ((*ih)->action);
  p = &bands[whichband];
  chh = p->items;
  for (i = 0; i < p->numitems; i++)
    if ((*(item **) (*(**chh)[i])->contrlData)->ioparid == fromdir &&
	(*(item **) (*(**chh)[i])->contrlData)->vrefnum == fromvol &&
	EqualString (s, (*(**chh)[i])->contrlTitle, false, false))
      {
	if (tovol != 0)
	  {
	    (*(item **) (*(**chh)[i])->contrlData)->ioparid = todir;
	    (*(item **) (*(**chh)[i])->contrlData)->vrefnum = tovol;
	  }
	else
	  {
	    c2 = (**chh)[i];
	    p->numitems--;
	    checkhotbandcontrol ();
	    for (j = i; j < p->numitems; j++)
	      (**p->items)[j] = (**p->items)[j + 1];
	    DisposHandle ((*(item **) (*c2)->contrlData)->path);
	    DisposeControl (c2);
	    if (whichband == g_currentband)
	      shiftband (i, -1);
	  }
      }
}

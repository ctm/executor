#include "go.h"
#include "display.h"

#include "mouse.proto.h"
#include "init.proto.h"
#include "window.proto.h"
#include "launch.proto.h"
#include "inithotband.proto.h"
#include "iconmanip.proto.h"
#include "misc.proto.h"
#include "menu.proto.h"
#include "view.proto.h"

ControlHandle (**g_selection)[];

int is_file (ControlHandle c)
{
  enum actions action;

  action = (*(item **) (*c)->contrlData)->action;
  return action != OPENDIR && action != NOACTION;
}

int
is_on_hot_band (ControlHandle c)
{
  return (*c)->contrlOwner == g_hotband;
}

int
is_volume (ControlHandle c)
{
  StringPtr sp;
  sp = (*c)->contrlTitle;
  return sp[sp[0]] == ':';
}

#define NON_EJECTABLE_BIT (1 << 5)

int
is_ejectable (ControlHandle c)
{
  HParamBlockRec hpb;
  int retval;
  OSErr e;

  if (is_volume (c))
    {
      e = get_HParamBlockRec_from_ControlHandle (&hpb, c);
      retval = e == noErr && !(hpb.volumeParam.ioVAtrb & NON_EJECTABLE_BIT);
    }
  else
    retval = false;
  return retval;
}

void
replaceselected (ControlHandle c)
{
  if ((**g_selection)[0] != 0)
    {
      (*(item **) ((*(**g_selection)[0])->contrlData))->selected = false;
      Draw1Control ((**g_selection)[0]);
    }
  (**g_selection)[0] = c;
  if (c != 0)
    {
      (*(item **) (*c)->contrlData)->selected = true;
      Draw1Control (c);
      menuchoices (true);
      if (!is_file (c))
	disable_menu_item (get_info_menuid);
      if (is_on_hot_band (c))
	disable_menu_item (send_to_hotband_menuid);
      if (!is_ejectable (c))
	disable_menu_item (eject_menuid);
      if (is_volume (c))
	disable_menu_item (duplicate_menuid);
    }
  else
    {
      menuchoices (false);
    }
}

short
dirwindowexists (Rect * r)
{
  WindowPeek wp;

#ifdef THINK_C
  for (wp = WindowList; wp != 0; wp = wp->nextWindow)
#else
  for (wp = LMGetWindowList (); wp != 0; wp = wp->nextWindow)
#endif
    if (((wp->port.portBits.rowBytes & 0xC000) == 0xC000) &&
	-r->top == (**(*(CGrafPtr) wp).portPixMap).bounds.top &&
	-r->left == (**(*(CGrafPtr) wp).portPixMap).bounds.left)
      return true;
  return false;
}

void
windfromicon (ControlHandle c)
{
  CWindowPtr wp;
  Handle h;
  Rect r;
  OSErr e;
  CInfoPBRec dir;

/* TODO: move to .h file */
#define DEFAULTWINDOWWIDTH	5
#define DEFAULTWINDOWHEIGHT	4
enum
  {
    HORIZONTAL_START = 20,
    VERTICAL_START = 40,
    HORIZONTAL_STAGGER = 15,
    VERTICAL_STAGGER = 15
  };

  wp = (CWindowPtr) (*c)->contrlOwner;
  h = (*(item **) (*c)->contrlData)->path;
  HandToHand (&h);
  if (wp == (CWindowPtr) g_hotband)
    {
      SetRect (&r, 100, HOTBANDBOTTOM + VERTICAL_START,
	       100 + DEFAULTWINDOWWIDTH * ICONWIDTHUSED + HORIZONTAL_START,
	       100 + DEFAULTWINDOWHEIGHT * ICONHEIGHTUSED);
    }
  else
    {
      r = (*c)->contrlOwner->portRect;
      OffsetRect (&r, -(*wp->portPixMap)->bounds.left,
		  -(*wp->portPixMap)->bounds.top + VERTICAL_STAGGER);
      appenddir (&h, (*c)->contrlTitle);
    }
  while (dirwindowexists (&r))
    {
      OffsetRect (&r, 0, VERTICAL_STAGGER);
      if (r.top > qd.screenBits.bounds.bottom)
	OffsetRect (&r, HORIZONTAL_STAGGER,
		    HOTBANDBOTTOM - r.top + VERTICAL_START);
    }
  if (r.left > qd.screenBits.bounds.right)
    OffsetRect (&r, -r.left + HORIZONTAL_STAGGER,
		HOTBANDBOTTOM - r.top + VERTICAL_START);

  dir.hFileInfo.ioVRefNum = (*(item **) (*c)->contrlData)->vrefnum;
  dir.hFileInfo.ioDirID = (*(item **) (*c)->contrlData)->ioparid;
  dir.hFileInfo.ioFDirIndex = 0;
  dir.hFileInfo.ioNamePtr = (*c)->contrlTitle;
  e = PBGetCatInfo (&dir, false);
  wp = createdirwindow (&dir, &r, h, (*(item **) (*c)->contrlData)->vrefnum);
  SelectWindow ((WindowPtr) wp);
  showviewmenu (true);
  ShowWindow ((WindowPtr) wp);
}

void
activateicon (ControlHandle c)
{
  switch ((*(item **) (*c)->contrlData)->action)
    {
    case LAUNCH:
      launchapp (c);
      break;
    case OPENDIR:
      windfromicon (c);
      break;
    case LAUNCHCREATOR:
      launchcreator (c, appOpen);
      /* TODO */
      break;
    case OPENDA:
      launchda (c);
      break;
    case NOACTION:
/* Do nothing */
      break;
    default:
      break;
    }
}

pascal void
scrollicons (ControlHandle c, short part)
{
  bandinfo *p;
  short shown;

  p = &bands[g_currentband];
  shown = p->bandpos + g_numdispinhotband;
  if (part == inDownButton)
    {
      if (shown < p->numitems)
	{
	  HideControl ((**p->items)[p->bandpos]);
	  p->bandpos++;
	  shiftband (p->bandpos, -1);
	}
/* shift everything over */
    }
  else if (part == inUpButton)
    {
      if (p->bandpos > 0)
	{
	  if (shown <= p->numitems)
	    HideControl ((**p->items)[shown - 1]);
	  p->bandpos--;
	  shiftband (p->bandpos + 1, 1);
	}
    }
  else
/*-->*/ return;
}

void
offsetwindow (ControlHandle c, short offset)
{
  short old, new;
  RgnHandle rgn;
  Rect r;

  rgn = NewRgn ();
  old = GetCtlValue (c);
  SetCtlValue (c, old + offset);
  new = GetCtlValue (c);
  SetOrigin (0, new);
  r = (*c)->contrlOwner->portRect;
  r.right -= SCROLLBARWIDTH;

#if 1
  ++r.right;
#endif

  ScrollRect (&r, 0, old - new, rgn);
#if 1
/* TODO: figure out why this isn't a movecontrol */
  OffsetRect (&(*c)->contrlRect, 0, new - old);
  UpdtControl ((*c)->contrlOwner, rgn);
#else /* 0 */
  MoveControl (c, (*c)->contrlRect.left, (*c)->contrlRect.top + new - old);
#endif /* 0 */
  DisposeRgn (rgn);
}

pascal void
windowscroller (ControlHandle c, short part)
{
  switch (part)
    {
    case inUpButton:
      offsetwindow (c, -SCROLLSPEED);
      break;
    case inDownButton:
      offsetwindow (c, SCROLLSPEED);
      break;
    case inPageUp:
      offsetwindow (c, (*c)->contrlRect.top - (*c)->contrlRect.bottom);
      break;
    case inPageDown:
      offsetwindow (c, (*c)->contrlRect.bottom - (*c)->contrlRect.top);
      break;
    default:
      break;
    }
}

void
actoncontrol (ControlHandle c, short part, EventRecord * ev)
{
  short before, after;

  switch (part)
    {
    case inButton:
    case INSELECTEDICON:
    case INTEXT:
      part = followcontrol (c, part);
      if (part == inButton || part == INTEXT)
	{
	  replaceselected (c);
	}
      else if (part == INSELECTEDICON)
	{
	  if (ev->when - g_lastclick < GetDblTime ())
	    activateicon (c);
	}			/* else if (part == INTEXT) {
				   } */
      break;
    case inThumb:
      before = GetCtlValue (c);
      if (TrackControl (c, ev->where, 0))
	{
	  after = GetCtlValue (c);
/* I don't like this, but it's the best way I could think of to avoid writing a
 * duplicate function
 */
	  SetCtlValue (c, before);
	  offsetwindow (c, after - before);
	}
      break;
    case inUpButton:
    case inDownButton:
    case inPageUp:
    case inPageDown:
      TrackControl (c, ev->where, windowscroller);
      break;
    default:
      break;
    }
}

void
checkcontrol (EventRecord * ev, WindowPtr wp)
{
  short part;
  ControlHandle c;
  GrafPtr saveport;

  GetPort (&saveport);
  SetPort (wp);
  GlobalToLocal (&ev->where);
  part = FindControl (ev->where, wp, &c);
  if (c != (ControlHandle) 0)
    {
      actoncontrol (c, part, ev);
    }
  else if ((**g_selection)[0] != (ControlHandle) 0)
    {
      replaceselected (c);
    }
  SetPort (saveport);
}

void
mouseinhotband (EventRecord * ev)
{
  DialogPtr dp;
  short item, part;
  ControlHandle c;
  Point p;
  GrafPtr saveport;

  GetPort (&saveport);
  SetPort (g_hotband);
  p = ev->where;
  GlobalToLocal (&p);
  if ((part = FindControl (p, g_hotband, &c)) != 0)
    {
      if ((*c)->contrlDefProc != g_iconcdefproc)
	{			/* see if it's an icon control */
	  if (DialogSelect (ev, &dp, &item) && (item <= NUMBANDS))
	    {
	      setband (item - 1);
	      ((WindowPeek) g_hotband)->refCon |= WIPEBIT;
	      InvalRect (&g_hotband->portRect);
	    }
	  else if (item == SORTBUTTON)
	    {
	      defaultsort ();
	    }
	  else if (item == HELPBUTTON)
	    {
	      help ();
	    }
	}
      else
	actoncontrol (c, part, ev);
    }
  SetPort (saveport);
}

void
domousedown (EventRecord * ev)
{
  WindowPtr wp;
  short where;

  where = FindWindow (ev->where, &wp);
  if (wp && ((WindowPeek) wp)->windowKind == userKind)
    {
      SelectWindow (wp);
      replaceselected ((ControlHandle) 0);
      showviewmenu (wp != g_hotband);
    }

  if (wp == g_hotband)
    {
      mouseinhotband (ev);
    }
  else
    {
      switch (where)
	{
	case inDesk:
/* DO NOTHING */
	  break;
	case inMenuBar:
	  domenu (MenuSelect (ev->where));
	  break;
	case inSysWindow:
	  SystemClick (ev, wp);
	  break;
	case inContent:
	  checkcontrol (ev, wp);
	  break;
	case inDrag:
	  dodragwin (ev->where, wp);
	  break;
	case inGrow:
	  dogrowwin (ev->where, wp);
	  break;
	case inGoAway:
	  dogoaway (ev->where, wp);
	  break;
	default:
	  break;
	}
    }
  g_lastclick = ev->when;
}

#include "go.h"
#include "xfer.h"
#include "display.h"

#include "getrects.proto.h"
#include "iconmanip.proto.h"
#include "initdirs.proto.h"
#include "inithotband.proto.h"
#include "misc.proto.h"
#include "sharedtransfer.proto.h"
#include "update.proto.h"

CCrsrHandle g_movecursor, g_copycursor;

enum
  {
    BIG_RECT_LEFT = -20000,
    BIG_RECT_TOP = -20000,
    BIG_RECT_RIGHT = 20000,
    BIG_RECT_BOTTOM = 20000
  }

void
xorrectframe (icondraginfo ** h)
{
  GrafPtr wport, saveport;
  PenState ps;
  RgnHandle rgn, savergn;
  unsigned char state;

  if (h == (icondraginfo **) 0 || (*h)->x == -1)
    return;
  rgn = NewRgn ();
  savergn = NewRgn ();
  GetPort (&saveport);
  GetWMgrPort (&wport);
  SetPort (wport);
  GetPenState (&ps);
  PenMode (patXor);
  SetRectRgn (rgn, BIG_RECT_LEFT, BIG_RECT_TOP, BIG_RECT_RIGHT,
	      BIG_RECT_BOTTOM);
  GetClip (savergn);
  SetClip (rgn);
  state = HGetState (h);
  HLock (h);
  OffsetRect (&(*h)->iconrect, (*h)->x, (*h)->y);
  FrameRect (&(*h)->iconrect);
  OffsetRect (&(*h)->iconrect, -(*h)->x, -(*h)->y);
  OffsetRect (&(*h)->textrect, (*h)->x, (*h)->y);
  FrameRect (&(*h)->textrect);
  OffsetRect (&(*h)->textrect, -(*h)->x, -(*h)->y);
  HSetState (h, state);
  SetClip (savergn);
  SetPenState (&ps);
  SetPort (saveport);
  DisposeRgn (rgn);
  DisposeRgn (savergn);
}

short
cleanupdrag (Point p, ControlHandle c)
{
  WindowPtr wp;
  short part;
  ControlHandle curc;

  xorrectframe ((icondraginfo **) (*c)->contrlRfCon);
  DisposeHandle ((Handle) (*c)->contrlRfCon);
  (*c)->contrlRfCon = 0L;

  FindWindow (p, &wp);
  if (wp == (WindowPtr) 0)
/*-->*/ return 0;

  SetPort (wp);
  GlobalToLocal (&p);
  part = FindControl (p, wp, &curc);
  return c == curc ? part : 0;
}

void
updateoutline (Point p, ControlHandle startc, ControlHandle * nextc)
{
  icondraginfo **h;
  short infoexists, part;
  Rect *r;
  WindowPtr wp;
  Point local_p;
  unsigned char state;

  infoexists = ((*startc)->contrlRfCon != 0L);
  FindWindow (p, &wp);
  SetPort (wp);
  GlobalToLocal (&p);
  local_p = p;
  part = FindControl (p, wp, nextc);
  LocalToGlobal (&p);
  if (startc != *nextc)
    {
      if (!infoexists)
	{
	  (*startc)->contrlRfCon = (long) NewHandle (sizeof (icondraginfo));
	  h = (icondraginfo **) (*startc)->contrlRfCon;
	  state = HGetState (h);
	  HLock (h);
	  getrects (startc, &(*h)->textrect, &(*h)->iconrect);
	  r = &(*h)->iconrect;
	  if (EmptyRect (r))
	    OffsetRect (&(*h)->textrect, -local_p.h, -(*h)->textrect.top);
	  else
	    OffsetRect (&(*h)->textrect, -r->left, -r->top);
	  OffsetRect (&(*h)->iconrect, -r->left, -r->top);
	  (*h)->x = -1;
	  (*h)->y = -1;
	  HSetState (h, state);
	}
      else
	h = (icondraginfo **) (*startc)->contrlRfCon;
      if ((*h)->x != p.h || (*h)->y != p.v)
	{
	  xorrectframe (h);
	  (*h)->x = p.h;
	  (*h)->y = p.v;
	  xorrectframe (h);
	}
    }
  else if (infoexists)
    {
      h = (icondraginfo **) (*startc)->contrlRfCon;
      xorrectframe (h);
      (*h)->x = -1;
      (*h)->y = -1;
    }
  if (wp == g_hotband && *nextc == (ControlHandle) 0)
    {
      SetCCursor (g_copycursor);
    }
  else if (*nextc != (ControlHandle) 0 && *nextc != startc &&
	   (*(item **) (**nextc)->contrlData)->action == OPENDIR)
    {
/* Todo: check for option key being held down */
      if ((*(item **) (**nextc)->contrlData)->vrefnum ==
	  (*(item **) (*startc)->contrlData)->vrefnum &&
	  (*startc)->contrlOwner != g_hotband)
	SetCCursor (g_movecursor);
      else
	SetCCursor (g_copycursor);
    }
  else if (*nextc == (ControlHandle) 0 && wp != 0 &&
	   wp != (*startc)->contrlOwner)
    {
      if ((*(opendirinfo **) ((WindowPeek) wp)->refCon)->vrefnum ==
	  (*(item **) (*startc)->contrlData)->vrefnum &&
	  (*startc)->contrlOwner != g_hotband)
	SetCCursor (g_movecursor);
      else
	SetCCursor (g_copycursor);
    }
  else
#ifdef THINK_C
    SetCursor (&arrow);
#else
    SetCursor (&qd.arrow);
#endif
}

short
followcontrol (ControlHandle c, short startpart)
{
  EventRecord ev;
  short part, selected;
  ControlHandle lastc, nextc;
  WindowPtr wp, saveport;
  item **ih1, **ih2;
  opendirinfo **dirinfoh;
  CInfoPBRec pb;
  Str255 s;
  unsigned char state;

  GetPort (&saveport);
  selected = (*(item **) (*c)->contrlData)->selected;
  lastc = (ControlHandle) 0;
  nextc = (ControlHandle) - 1;
  while (!GetNextEvent (mUpMask, &ev))
    {
      updateoutline (ev.where, c, &nextc);
      if (lastc != nextc)
	{
	  if (lastc != (ControlHandle) 0)
	    {
	      xorrectframe ((icondraginfo **) (*c)->contrlRfCon);
	      HiliteControl (lastc, 0);
	      xorrectframe ((icondraginfo **) (*c)->contrlRfCon);
	    }
	  if (nextc != 0)
	    {
	      if (nextc == c)
		HiliteControl (c, selected ? INSELECTEDICON : inButton);
	      else if ((*(item **) (*nextc)->contrlData)->action == OPENDIR)
		{
		  xorrectframe ((icondraginfo **) (*c)->contrlRfCon);
		  HiliteControl (nextc, inButton);
		  xorrectframe ((icondraginfo **) (*c)->contrlRfCon);
		}
	    }
	}
      lastc = nextc;
    }
  part = cleanupdrag (ev.where, c);
  if (lastc != 0)
    {
      HiliteControl (lastc, 0);
      wp = (*c)->contrlOwner;
      ih1 = (item **) (*c)->contrlData;
      ih2 = (item **) (*lastc)->contrlData;
      if ((*ih2)->action == OPENDIR && lastc != c)
	{
	  state = HGetState ((Handle) lastc);
	  HLock ((Handle) lastc);
	  pb.dirInfo.ioNamePtr = (*lastc)->contrlTitle;
	  pb.dirInfo.ioVRefNum = (*ih2)->vrefnum;
	  pb.dirInfo.ioFDirIndex = 0;
	  pb.dirInfo.ioDrDirID = (*ih2)->ioparid;
	  PBGetCatInfo (&pb, false);
	  mystr255copy (s, (*c)->contrlTitle);
	  if (wp == g_hotband || ((*ih2)->vrefnum != ((*ih1)->vrefnum)))
	    {
	      if (!commontrans ((*ih1)->ioparid, pb.dirInfo.ioDrDirID, (*ih1)->vrefnum,
			     (*ih2)->vrefnum, (*c)->contrlTitle, copy1file))
		changewindow (s, pb.dirInfo.ioDrDirID, (*ih2)->vrefnum, updatemove);
	    }
	  else
	    {
	      if (!commontrans ((*ih1)->ioparid, pb.dirInfo.ioDrDirID, (*ih1)->vrefnum,
			     (*ih2)->vrefnum, (*c)->contrlTitle, move1file))
		{
		  changehot (c, pb.dirInfo.ioDrDirID, (*ih2)->vrefnum);
		  changewindow (s, pb.dirInfo.ioDrDirID, (*ih2)->vrefnum, updatemove);
		  changewindow (s, (*ih1)->ioparid, (*ih1)->vrefnum, removefromlist);
		}
	    }
	  HSetState ((Handle) lastc, state);
	}
    }
  else if (nextc != (ControlHandle) - 1)
    {
      FindWindow (ev.where, &wp);
      if (wp == g_hotband)
	{
	  if ((*c)->contrlOwner != wp)
	    copycontroltohotband (c);
	}
      else if (wp != 0 && wp != (*c)->contrlOwner)
	{
	  dirinfoh = (opendirinfo **) ((WindowPeek) wp)->refCon;
	  ih1 = (item **) (*c)->contrlData;
	  mystr255copy (s, (*c)->contrlTitle);
	  if ((*dirinfoh)->vrefnum != ((*ih1)->vrefnum))
	    {
	      if (!commontrans ((*ih1)->ioparid, (*dirinfoh)->iodirid, (*ih1)->vrefnum,
			(*dirinfoh)->vrefnum, (*c)->contrlTitle, copy1file))
		changewindow (s, (*dirinfoh)->iodirid, (*dirinfoh)->vrefnum, updatemove);
	    }
	  else
	    {
	      if (!commontrans ((*ih1)->ioparid, (*dirinfoh)->iodirid, (*ih1)->vrefnum,
			(*dirinfoh)->vrefnum, (*c)->contrlTitle, move1file))
		{
		  changehot (c, (*dirinfoh)->iodirid, (*dirinfoh)->vrefnum);
		  changewindow (s, (*dirinfoh)->iodirid, (*dirinfoh)->vrefnum, updatemove);
		  changewindow (s, (*ih1)->ioparid, (*ih1)->vrefnum, removefromlist);
		}
	    }
	}
      else
	{
	  if ((*c)->contrlOwner == g_hotband && g_currentband != VOLBAND)
	    removeitemfromhotband (c);
	}
    }
#ifdef THINK_C
  SetCursor (&arrow);
#else
  SetCursor (&qd.arrow);
#endif
  return startpart == part ? part : 0;
  SetPort (saveport);
}

#include "go.h"
#include "display.h"

#include "window.proto.h"
#include "init.proto.h"
#include "initdirs.proto.h"
#include "initicons.proto.h"
#include "misc.proto.h"

void
setwindowicons (CWindowPeek wp)
{
  ControlHandle c;
  opendirinfo **infoh;

  infoh = (opendirinfo **) wp->refCon;
  for (c = wp->controlList; c != 0; c = (*c)->nextControl)
    {
      if (c != (*infoh)->sbar)
	{
	  setoneicon (c);
	  ShowControl (c);
	}
    }
}

int
executor_p (void)
{
  return *(long *) 0x5c != *(long *) 0x58;
}

CWindowPtr
createdirwindow (CInfoPBRec *dir, Rect *r, char **path, short volume)
{
  CWindowPeek wp;
  short i;
  OSErr e;
  Str255 s;
  opendirinfo **infoh;
  ControlHandle c;

  if (executor_p ())
    {
      CInfoPBRec cpb;
      OSErr e2;

      cpb = *dir;
      e2 = unixmount (&cpb);
      if (e2 == noErr)
	volume = cpb.hFileInfo.ioVRefNum;
    }
  tail (*path, s);
  wp = (CWindowPeek) NewPtr (sizeof (CWindowRecord));
  wp = (CWindowPeek) NewCWindow (wp, r, s, false, documentProc, (WindowPtr) 0, true, 0);
  wp->refCon = (long) NewHandle (sizeof (opendirinfo));

  infoh = (opendirinfo **) wp->refCon;
#if 1
/* This handle should be unlocked later, but it isn't for Executor 1.99m */
  HLock ((Handle) infoh);
#endif /* 0 */
  (*infoh)->sortorder = ALPHABETIC;
  (*infoh)->items = (ControlHandle (**)[])
    NewHandle (BANDARRAYSIZE * sizeof (ControlHandle));
  (*infoh)->path = path;
  (*infoh)->iodirid = dir->dirInfo.ioDrDirID;
  (*infoh)->vrefnum = volume;
  (*infoh)->numitems = 0;
  (*infoh)->view = ICONVIEW;

/* NOTE: r is the wrong rectangle, but it gets fixed before it's used. */
  (*infoh)->sbar = NewControl ((WindowPtr) wp, r, (StringPtr) "\p", false, 0, 0, 0,
			       scrollBarProc, 0);
  MoveControl ((*infoh)->sbar, wp->port.portRect.right - SCROLLBARWIDTH, 0);

  dir->hFileInfo.ioNamePtr = s;
  dir->hFileInfo.ioVRefNum = volume;
  e = noErr;
  for (i = 1; e == noErr; i++)
    {
      checkgrowitemarray (i, (*infoh)->items);
      dir->hFileInfo.ioDirID = (*infoh)->iodirid;
      dir->hFileInfo.ioFDirIndex = i;
      e = PBGetCatInfo (dir, false);
      if (e == noErr && !(dir->hFileInfo.ioFlFndrInfo.fdFlags & fInvisible))
	{
	  c = addtolist ((WindowPeek) wp, s, dir->hFileInfo.ioFlParID, volume);
	  if (dir->hFileInfo.ioFlFndrInfo.fdType == 'APPL' ||
	      dir->hFileInfo.ioFlFndrInfo.fdType == 'dfil')
	    hashicons (c);
	}
    }
  setwindowicons (wp);
  straightenwindow ((WindowPtr) wp);
  return (CWindowPtr) wp;
}

void
dodragwin (Point start, WindowPtr wp)
{
  Rect r;

  SetRect (&r, 4, HOTBANDBOTTOM + 4 + start.v -
	   (*((WindowPeek) wp)->strucRgn)->rgnBBox.top,
	   qd.screenBits.bounds.right - 4, qd.screenBits.bounds.bottom - 4);
  DragWindow (wp, start, &r);
}

void
dogrowwin (Point start, WindowPtr wp)
{
  Rect r;
  long newsize;

/* todo: find out what is normally done when there is no maximum size that needs to be enforced */
  SetRect (&r, ICONWIDTHUSED, ICONHEIGHTUSED, 32767, 32767);
  newsize = GrowWindow (wp, start, &r);
  SizeWindow (wp, LoWord (newsize), HiWord (newsize), true);
  InvalRect (&wp->portRect);
  straightenwindow (wp);
}

void
disposedirwindow (WindowPtr wp)
{
  SendBehind (g_hotband, (WindowPtr) 0);
  if ((**g_selection)[0] != 0 && (*(**g_selection)[0])->contrlOwner == wp)
    (**g_selection)[0] = 0;
  DisposHandle ((*(opendirinfo **) ((WindowPeek) wp)->refCon)->path);
  DisposeWindow (wp);
  showviewmenu (FrontWindow () != g_hotband);
}

void
dogoaway (Point start, WindowPtr wp)
{
  if (TrackGoAway (wp, start))
    disposedirwindow (wp);
}

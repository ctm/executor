#include "go.h"
#include "display.h"
#include "xfer.h"
#include <unix.h>
#include <assert.h>

#include "misc.proto.h"
#include "inithotband.proto.h"

/* todo: move to .h file */
#define HELPSCROLLBAR	3
#define HELPPICTITEM	2
#define HELPDIALOGID	204
#define HELPPICT		1000

pascal void
helpscroll (ControlHandle c, short part)
{
  short v, v2, pagesize;
  short type;
  Rect r;
  Handle h;
  RgnHandle rgn, savergn;

  v = GetCtlValue (c);
  pagesize = (*c)->contrlRect.bottom - (*c)->contrlRect.top - 3;
  switch (part)
    {
    case inUpButton:
      SetCtlValue (c, v - SCROLLSPEED);
      break;
    case inDownButton:
      SetCtlValue (c, v + SCROLLSPEED);
      break;
    case inPageDown:
      SetCtlValue (c, v + pagesize);
      break;
    case inPageUp:
      SetCtlValue (c, v - pagesize);
      break;
    default:
      break;
    }
  v2 = GetCtlValue (c);
  GetDItem ((*c)->contrlOwner, HELPPICTITEM, &type, &h, &r);
  rgn = NewRgn ();
  ScrollRect (&r, 0, v - v2, rgn);
#if 0
  InvalRgn (rgn);
#else
  savergn = (*c)->contrlOwner->clipRgn;
  (*c)->contrlOwner->clipRgn = rgn;
  drawhelp ((*c)->contrlOwner, HELPPICTITEM);
  (*c)->contrlOwner->clipRgn = savergn;
#endif
  DisposeRgn (rgn);
}

pascal void
drawhelp (DialogPtr dp, short item)
{
  PicHandle ph;
  Rect r, r2;
  ControlHandle c;
  short offset, type;
  Handle h;

  ph = (PicHandle) GetResource ('PICT', HELPPICT);
  GetDItem (dp, HELPSCROLLBAR, &type, (Handle *) & c, &r);
  offset = GetCtlValue (c);
  GetDItem (dp, item, &type, &h, &r);
  InsetRect (&r, -1, -1);
  FrameRect (&r);
  InsetRect (&r, 1, 1);
  ClipRect (&r);
  SetRect (&r2, r.left, r.top - offset,
	   r.left + (*ph)->picFrame.right - (*ph)->picFrame.left,
	   r.top - offset + (*ph)->picFrame.bottom - (*ph)->picFrame.top);
  CopyBits (&((GrafPtr) (*c)->contrlRfCon)->portBits, &(*c)->contrlOwner->portBits,
	    &(*ph)->picFrame, &r2, patCopy, 0);
  ClipRect (&dp->portRect);
}

pascal char
helpfilter (DialogPtr dp, EventRecord * ev, short *item)
{
  ControlHandle c;
  short part;
  Point pt;
  GrafPtr saveport;

  if (ev->what == keyDown && (((ev->message & charCodeMask) == '\n') ||
			      ((ev->message & charCodeMask) == 13)))
    {
      *item = 1;
      return true;
    }
  else if (ev->what == mouseDown)
    {
      GetPort (&saveport);
      SetPort (dp);
      pt = ev->where;
      GlobalToLocal (&pt);
      part = FindControl (pt, dp, &c);
      switch (part)
	{
	case inThumb:
	  TrackControl (c, pt, 0);
	  drawhelp ((*c)->contrlOwner, HELPPICTITEM);
	  break;
	case inUpButton:
	case inDownButton:
	case inPageUp:
	case inPageDown:
	  TrackControl (c, pt, helpscroll);
	  break;
	default:
	  break;
	}
      SetPort (saveport);
    }
  return false;
}

void
help (void)
{
  short item;
  PicHandle ph;
  Rect r;
  ControlHandle c;
  short type;
  Handle h;
  DialogPtr dp;
  GrafPtr gp;
  GrafPtr saveport;

  dp = GetNewDialog (HELPDIALOGID, 0, 0);
/* TODO: adjust the window size based upon the screen size, 
   outline the done button 
 */
  GetDItem (dp, HELPSCROLLBAR, &type, (Handle *) & c, &r);
  ph = (PicHandle) GetResource ('PICT', HELPPICT);
  SetCtlMax (c, (*ph)->picFrame.bottom - (*ph)->picFrame.top - r.bottom + r.top);

  GetPort (&saveport);
  gp = (GrafPtr) NewPtr (sizeof (GrafPort));
  OpenPort (gp);
  gp->portBits.rowBytes = ((*ph)->picFrame.right - (*ph)->picFrame.left + 15) / 16 * 2;
  gp->portBits.baseAddr = NewPtr (((*ph)->picFrame.bottom - (*ph)->picFrame.top) *
				  gp->portBits.rowBytes);
  gp->portBits.bounds = (*ph)->picFrame;
  (*c)->contrlRfCon = (long) gp;
  SetPort (gp);
  ClipRect (&gp->portBits.bounds);
  RectRgn (gp->visRgn, &gp->portBits.bounds);
  EraseRect (&gp->portBits.bounds);
  DrawPicture (ph, &gp->portBits.bounds);
  SetPort (saveport);

  GetDItem (dp, HELPPICTITEM, &type, &h, &r);
  SetDItem (dp, HELPPICTITEM, type, (Handle) drawhelp, &r);
  ShowWindow (dp);
  BringToFront (dp);
  ModalDialog ((ProcPtr) helpfilter, &item);
  DisposePtr (gp->portBits.baseAddr);
  ClosePort (gp);
  DisposPtr ((Ptr) gp);
  DisposDialog (dp);
}

void
screensaver (void)
{
}

void
setband (short whichband)
{
  short i, j, type;
  bandinfo *p;
  ControlHandle c;
  Rect r;

  p = &bands[g_currentband];
  for (i = p->bandpos; i < p->numitems; i++)
    HideControl ((**p->items)[i]);
  GetDItem (g_hotband, g_currentband + 1, &type, (Handle *) & c, &r);
  HiliteControl (c, 0);

  g_currentband = whichband;
  p = &bands[g_currentband];
  for (i = p->bandpos, j = 0; i < p->numitems && j < g_numdispinhotband; i++, j++)
    {
      MoveControl ((**p->items)[i], FIRSTICONX + ICONWIDTHUSED * j, 0);
      ShowControl ((**p->items)[i]);
    }
  GetDItem (g_hotband, g_currentband + 1, &type, (Handle *) & c, &r);
  HiliteControl (c, 255);
  checkhotbandcontrol ();
}

void
straightenwindow (WindowPtr wp)
{
  opendirinfo **infoh;
  short i, h, v, row, col, itemsperrow, height, numrows, ctlheight, top;
  short heightused, widthused;
  ControlHandle c;
  GrafPtr saveport;

  GetPort (&saveport);
  SetPort (wp);
  infoh = (opendirinfo **) ((WindowPeek) wp)->refCon;

  switch ((*infoh)->view)
    {
    case ICONVIEW:
      widthused = ICONWIDTHUSED;
      heightused = ICONHEIGHTUSED;
      break;
    case ICSVIEW:
      widthused = ICSWIDTHUSED;
      heightused = ICSHEIGHTUSED;
      break;
    case LISTVIEW:
      widthused = LISTWIDTHUSED;
      heightused = LISTHEIGHTUSED;
      break;
    }
  itemsperrow = (wp->portRect.right - wp->portRect.left) / widthused;
  if (itemsperrow < 1)
    itemsperrow = 1;
  numrows = ((*infoh)->numitems - 1) / itemsperrow + 1;

  c = (*infoh)->sbar;
  height = wp->portRect.bottom - wp->portRect.top;
  SizeControl (c, SCROLLBARWIDTH, height - SCROLLBARWIDTH + 2);
  top = GetCtlValue (c);
  ctlheight = numrows * heightused - height;
  if (ctlheight < 0)
    ctlheight = 0;
  SetCtlMax (c, ctlheight);
  if (top > ctlheight)
    top = ctlheight;
  SetCtlValue (c, top);
  SetOrigin (0, top);
  MoveControl (c, wp->portRect.right - SCROLLBARWIDTH + 1, wp->portRect.top);
  drawpartialgrowicon (wp, false);
  for (i = 0; i < (*infoh)->numitems; i++)
    {
      row = i / itemsperrow;
      col = i % itemsperrow;
      v = row * heightused;
      h = col * widthused;
      MoveControl ((**(*infoh)->items)[i], h, v);
    }
  if (GetCtlMax (c) <= 0)
    HiliteControl (c, 255);
  else
    HiliteControl (c, 0);
  if (FrontWindow () == wp)
    ShowControl (c);
  SetPort (saveport);
}

/* TODO: move to .h file */
#define GETINFOID	205
typedef enum
{
  GETINFO_OK_ITEM = 1,
  GETINFO_CANCEL_ITEM,
  GETINFO_FILELABEL_ITEM,
  GETINFO_CREATORLABEL_ITEM,
  GETINFO_TYPELABEL_ITEM,
  GETINFO_FILE_ITEM,
  GETINFO_CREATOR_ITEM,
  GETINFO_TYPE_ITEM
}
getinfo_items_t;

void
getinfo (void)
{
  item **ih;
  ControlHandle c;
  CInfoPBRec pb;
  Str255 s;
  DialogPtr dp;
  short type, itemno, valid;
  Rect r;
  Handle h;
/* 
 * TODO: add file size, modification date, etc. 
 *       outline ok button
 *       center window
 *       check to see if it's a folder info is gotten about.
 */

  c = (**g_selection)[0];
  ih = (item **) (*c)->contrlData;
  pb.hFileInfo.ioNamePtr = (*c)->contrlTitle;
  pb.hFileInfo.ioVRefNum = (*ih)->vrefnum;
  pb.hFileInfo.ioFDirIndex = 0;
  pb.hFileInfo.ioDirID = (*ih)->ioparid;
  PBGetCatInfo (&pb, false);

  dp = GetNewDialog (GETINFOID, 0, (WindowPtr) - 1);
  GetDItem (dp, GETINFO_FILE_ITEM, &type, &h, &r);
  SetIText (h, (*c)->contrlTitle);
  s[0] = 4;
  memcpy ((char *) s + 1, (char *) &pb.hFileInfo.ioFlFndrInfo.fdCreator, 4);
  GetDItem (dp, GETINFO_CREATOR_ITEM, &type, &h, &r);
  SetIText (h, s);
  s[0] = 4;
  memcpy ((char *) s + 1, (char *) &pb.hFileInfo.ioFlFndrInfo.fdType, 4);
  GetDItem (dp, GETINFO_TYPE_ITEM, &type, &h, &r);
  SetIText (h, s);
  SelIText (dp, GETINFO_CREATOR_ITEM, 0, 4);

  ShowWindow (dp);
  SetPort (dp);
  do
    {
      ModalDialog ((ProcPtr) 0, &itemno);
      valid = true;
      if (itemno == GETINFO_OK_ITEM)
	{
	  GetDItem (dp, GETINFO_CREATOR_ITEM, &type, &h, &r);
	  GetIText (h, s);
	  if (s[0] != 4)
	    valid = false;
	  else
	    memcpy ((char *) &pb.hFileInfo.ioFlFndrInfo.fdCreator,
		       (char *) s + 1, 4);
	  GetDItem (dp, GETINFO_TYPE_ITEM, &type, &h, &r);
	  GetIText (h, s);
	  if (s[0] != 4)
	    valid = false;
	  else
	    memcpy ((char *) &pb.hFileInfo.ioFlFndrInfo.fdType,
		       (char *) s + 1, 4);
	  if (valid)
	    {
	      pb.hFileInfo.ioDirID = (*ih)->ioparid;
	      PBSetCatInfo (&pb, false);
	      setoneicon (c);
	    }
	  else
	    {
	      ParamText ((StringPtr) "\pCreators and types are 4 characters long.",
			 0, 0, 0);
	      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
	    }
	}
    }
  while (!valid);
  DisposDialog (dp);
}

void
savestate (void)
{
  FILE *f;
  CWindowPeek wp;
  short i, j, res;

  unlink (GOBACKUPFILE);
  res = rename (GOSAVEFILE, GOBACKUPFILE);
  if (res)
    {
      ParamText ((StringPtr) "\pThe old state file coulnd't be save",
		 0, 0, 0);
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
      return;
    }
  f = fopen (GOSAVEFILE, "w");
  if (!f)
    {
      ParamText ((StringPtr) "\pThe browser's state could not be saved.",
		 0, 0, 0);
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
      return;
    }
  for (i = 0; i < NUMBANDS; i++)
    {
      if (i != VOLBAND)
	{
	  for (j = 0; j < bands[i].numitems; j++)
	    fprintf (f, "%s\n",
		  *(*(item **) (*(**bands[i].items)[j])->contrlData)->path);
	}
    }
  fprintf (f, "\n");
#ifdef THINK_C
  for (wp = (CWindowPeek) WindowList; wp != 0; wp = wp->nextWindow)
    {
#else
  for (wp = (CWindowPeek) LMGetWindowList (); wp != 0; wp = wp->nextWindow)
    {
#endif
      if (wp != (CWindowPeek) g_hotband && (wp->windowKind == userKind))
	{
	  fprintf (f, "%s\n", *(*(opendirinfo **) wp->refCon)->path);
	  OffsetRect (&wp->port.portRect, -(*wp->port.portPixMap)->bounds.left,
		      -(*wp->port.portPixMap)->bounds.top);
	  fprintf (f, "%d %d %d %d\n", wp->port.portRect.left,
		   wp->port.portRect.top, wp->port.portRect.right,
		   wp->port.portRect.bottom);
	}
    }
  res = fclose (f);
  if (res)
    {
      ParamText ((StringPtr) "\pThe browser's state could not be saved.",
		 0, 0, 0);
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
      return;
    }
/* TODO: make save file invisible */
  res = unlink (GOBACKUPFILE);
  if (res)
    {
      ParamText ((StringPtr) "\pThe browser's backup state file could not be removed.",
		 0, 0, 0);
      StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
      return;
    }
}

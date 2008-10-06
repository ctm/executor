#include "go.h"
#include <string.h>
#include "xfer.h"
#include "display.h"

#include "inithotband.proto.h"
#include "init.proto.h"
#include "initicons.proto.h"
#include "mouse.proto.h"
#include "misc.proto.h"
#include "view.proto.h"

DialogPtr g_hotband;
bandinfo bands[NUMBANDS];
short g_currentband;
Handle g_iconcdefproc;
short g_numdispinhotband;

void
additemtohotband (short whichband, char **path, long parid, short vrefnum,
		  short action)
{
  bandinfo *p;
  ControlHandle c;
  Str255 s;
  item **ih;

  p = &bands[whichband];
  checkgrowitemarray (p->numitems, p->items);
  tail (*path, s);
  c = (**p->items)[p->numitems] = getnewiconcontrol (g_hotband, path, parid,
						     vrefnum, s);
  ih = (item **) (*c)->contrlData;
  (*ih)->action = action;

/* setoneicon calls PBGetCatInfo which crashes when called with an offline volume */
  if (whichband != VOLBAND)
    setoneicon (c);
  else
    (*(item **) (*c)->contrlData)->iconfam = &g_diskiconsptr;
  if (g_currentband != whichband)
    setband (whichband);
  if (p->bandpos + g_numdispinhotband > p->numitems)
    {
      MoveControl (c, (p->numitems - p->bandpos) * ICONWIDTHUSED + FIRSTICONX, 0);
      ShowControl (c);
    }
  p->numitems++;
  checkhotbandcontrol ();
}

void
adjust_ctl_value (ControlHandle c, bandinfo *p)
{
  if (p->bandpos == 0)
    SetCtlValue (c, 0);
  else if (p->bandpos == p->numitems - g_numdispinhotband)
    SetCtlValue (c, 2);
  else
    SetCtlValue (c, 1);
}

void
checkhotbandcontrol (void)
{
  Rect r;
  ControlHandle c;
  short type;
  bandinfo *p;
  short new_hilite_val;
  int old_ctl_val;

  GetDItem (g_hotband, SCROLLITEM, &type, (Handle *) &c, &r);
  old_ctl_val = GetCtlValue (c);
  p = &bands[g_currentband];
  if (p->numitems > g_numdispinhotband)
    {
      new_hilite_val = 0;
      adjust_ctl_value (c, p);
    }
  else
    {
      if (p->bandpos > 0)
	{
	  /* FIXME - what the hell does this mean?  The offset is the same as the start? */
	  shiftband (p->bandpos, p->bandpos);
	  p->bandpos = 0;
	}
      new_hilite_val = 255;
    }
  if (GetCtlValue (c) != old_ctl_val || (*c)->contrlHilite != new_hilite_val)
    {
      if ((*c)->contrlHilite != new_hilite_val)
	HiliteControl (c, new_hilite_val);
      Draw1Control (c);
    }
}

void
shiftband (short start, short offset)
{
  ControlHandle c;
  bandinfo *p;
  short pixoffset, first, last, i, x, y;
  short type, old_ctl_val;
  Rect r;

  GetDItem (g_hotband, SCROLLITEM, &type, (Handle *) & c, &r);
  old_ctl_val = GetCtlValue (c);
  p = &bands[g_currentband];
  pixoffset = offset * ICONWIDTHUSED;
  last = start + g_numdispinhotband;
  if (last > p->numitems)
    last = p->numitems;
  if (offset < 0)
    {
      for (i = start; i < last; i++)
	MoveControl ((**p->items)[i],
		     (*(**p->items)[i])->contrlRect.left + pixoffset,
		     (*(**p->items)[i])->contrlRect.top);
    }
  else
    {
      for (i = last - 1; i >= start; i--)
	MoveControl ((**p->items)[i],
		     (*(**p->items)[i])->contrlRect.left + pixoffset,
		     (*(**p->items)[i])->contrlRect.top);
    }

  y = (*(**p->items)[start])->contrlRect.top;
  if (offset < 0)
    {
      first = p->bandpos + g_numdispinhotband + offset;
      last = first - offset;
      if (last > p->numitems)
	last = p->numitems;
      if (first <= p->numitems && first > 0)
	x = (*(**p->items)[first - 1])->contrlRect.left + ICONWIDTHUSED;
    }
  else
    {
      first = start - offset;
      last = start;
      x = (*(**p->items)[start])->contrlRect.left - pixoffset;
    }

  for (i = first; i < last; i++)
    {
      MoveControl ((**p->items)[i], x, y);
      ShowControl ((**p->items)[i]);
      x += ICONWIDTHUSED;
    }
  adjust_ctl_value (c, p);
  if (GetCtlValue (c) != old_ctl_val)
    Draw1Control (c);
}

void
removeitemfromhotband (ControlHandle c)
{
/* NOTE: this only works for items in the current band */
  short pos, i;
  bandinfo *p;

  p = &bands[g_currentband];
  pos = ((*c)->contrlRect.left - FIRSTICONX) / ICONWIDTHUSED + p->bandpos;
  p->numitems--;
  for (i = pos; i < p->numitems; i++)
    (**p->items)[i] = (**p->items)[i + 1];
  DisposHandle ((*(item **) (*c)->contrlData)->path);
  DisposeControl (c);
  shiftband (pos, -1);
  checkhotbandcontrol ();
}

void
copycontroltohotband (ControlHandle c)
{
  Handle newpath;
  typeinfo *tip;
  item **ih;
  opendirinfo **infoh;
  short i, band;
  GrafPtr saveport;

  ih = (item **) (*c)->contrlData;
  newpath = (*ih)->path;
  HandToHand (&newpath);
  appenddir (&newpath, (*c)->contrlTitle);
  if ((*ih)->action == OPENDIR)
    {
      band = FOLDERBAND;
    }
  else
    {
      tip = gettypeinfo ((*(*ih)->iconfam)->type);
      band = tip->band;
    }
  hashicons (c);
  additemtohotband (band, (char **) newpath, (*ih)->ioparid,
		    (*ih)->vrefnum, (*ih)->action);

  if (band == FONTBAND || band == DABAND)
    {
      ParamText ((StringPtr) "\pFonts and Desk Accessories are not yet implemented in the hot band.",
		 0, 0, 0);
      NoteAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
    }
  infoh = (opendirinfo **) ((WindowPeek) (*c)->contrlOwner)->refCon;
  for (i = 0; i < (*infoh)->numitems; i++)
    setoneicon ((**(*infoh)->items)[i]);
  GetPort (&saveport);
  SetPort ((*c)->contrlOwner);
  InvalRect (&(*c)->contrlOwner->portRect);
  SetPort (saveport);
}

void
selectiontohotband (void)
{
  copycontroltohotband ((**g_selection)[0]);
}

void
addvolumes (void)
{
  OSErr e;
  ParamBlockRec pb;
  short i, len;
  Str255 s;
  char **path;

  e = noErr;
  pb.volumeParam.ioNamePtr = s;
  for (i = 1; e == noErr; i++)
    {
      pb.volumeParam.ioVolIndex = i;
      e = PBGetVInfo (&pb, false);
/* todo: figure out which type of disk icon to use */
      if (e == noErr)
	{
	  len = pb.volumeParam.ioNamePtr[0];
	  path = (char **) NewHandle (len + 2);
	  PtoCstr (pb.volumeParam.ioNamePtr);
	  strcpy (*path, (char *) pb.volumeParam.ioNamePtr);
	  (*path)[len] = ':';
	  (*path)[len + 1] = 0;
	  additemtohotband (VOLBAND, path, 0, pb.volumeParam.ioVRefNum, OPENDIR);
	}
    }
  g_iconcdefproc = (*(**bands[VOLBAND].items)[0])->contrlDefProc;
}

void
resetvolumes (void)
{
  bandinfo *p;
  ControlHandle c;
  short i;
  GrafPtr saveport;

  p = &bands[VOLBAND];
  for (i = p->numitems - 1; i >= 0; i--)
    {
      c = (**p->items)[i];
      DisposHandle ((*(item **) (*c)->contrlData)->path);
      DisposeControl (c);
    }
  p->numitems = 0;

  addvolumes ();
  for (i = 0; i < p->numitems; i++)
    (*(item **) (*(**p->items)[i])->contrlData)->iconfam = &g_diskiconsptr;
  GetPort (&saveport);
  SetPort (g_hotband);
  InvalRect (&g_hotband->portRect);
  SetPort (saveport);
}

void
fillbands (FILE * f)
{
/* todo: check the value of e after the getinfo calls */
  CInfoPBRec cpb;
  short c, whichband, action;
  OSErr e;
  char **path;
  short volume;
  typeinfo *tip;

  cpb.hFileInfo.ioFDirIndex = 0;
  while ((c = getc (f)) && (c != EOF) && (c != '\n'))
    {
      ungetc (c, f);
      e = getonefileinfo (f, &cpb, &path, &volume);
      if (cpb.hFileInfo.ioFlAttrib & DIRBIT)
	{
	  whichband = FOLDERBAND;
	  action = OPENDIR;
	}
      else
	{
	  tip = gettypeinfo (cpb.hFileInfo.ioFlFndrInfo.fdType);
	  whichband = tip->band;
	  action = tip->action;
	}
      if (e == noErr)
	additemtohotband (whichband, path, cpb.hFileInfo.ioFlParID,
			  volume, action);
#if 0
      else
	printf ("couldn't find %s\n", *path);
#endif
    }
}

void
inithotband (FILE * f)
{
  short type;
  Handle h;
  Rect r;
  short i, j;

  g_hotband = GetNewDialog (HOTBANDRESID, (Ptr) 0, (WindowPtr) - 1);
  MoveWindow (g_hotband, 0, MENUBARHEIGHT, false);
  SizeWindow (g_hotband, qd.screenBits.bounds.right, HOTBANDHEIGHT, true);

  GetDItem (g_hotband, SCROLLITEM, &type, &h, &r);
  SetCtlMin ((ControlHandle) h, 0);
  SetCtlMax ((ControlHandle) h, 2);
  MoveControl ((ControlHandle) h, FIRSTICONX - ICONWIDTHUSED / 2, 0);
  SizeControl ((ControlHandle) h, qd.screenBits.bounds.right -
	       (*(ControlHandle) h)->contrlRect.left, ICONHEIGHTUSED);
  SetDItem (g_hotband, SCROLLITEM, type, h, &(*(ControlHandle) h)->contrlRect);
  SetCtlAction ((ControlHandle) h, scrollicons);
  g_numdispinhotband = (qd.screenBits.bounds.right - FIRSTICONX) / ICONWIDTHUSED;
  if (g_numdispinhotband < 1)
    g_numdispinhotband = 1;

  for (i = 0; i < NUMBANDS; i++)
    {
      bands[i].bandpos = 0;
      bands[i].numitems = 0;
      bands[i].sortorder = ALPHABETIC;
      bands[i].items =
	(ControlHandle (**)[])NewHandle (BANDARRAYSIZE * sizeof (ControlHandle));
    }
  inithash ();
/* This line is important.  g_currentband must be initialized before setband is called */
  g_currentband = APPBAND;
  addvolumes ();

  if (f != 0)
    fillbands (f);

  for (j = 0; j < NUMBANDS; j++)
    if (j != VOLBAND && j != FOLDERBAND)
      for (i = 0; i < bands[j].numitems; i++)
	hashicons ((**bands[j].items)[i]);

  seticons ();
  if (bands[APPBAND].numitems > 0)
    setband (APPBAND);
  else
    setband (VOLBAND);
  ShowWindow (g_hotband);
}

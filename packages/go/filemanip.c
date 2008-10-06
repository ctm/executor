#include "go.h"
#include "xfer.h"

#include "sharedtransfer.proto.h"
#include "filemanip.proto.h"
#include "mouse.proto.h"
#include "window.proto.h"
#include "dircreate.proto.h"
#include "delete.proto.h"
#include "initdirs.proto.h"
#include "update.proto.h"
#include "misc.proto.h"
#include "launch.proto.h"
#include <string.h>

/* todo: move to .h file */
#define STDFILEWIDTH	304
void
newfolder (void)
{
  Point pt;
  SFReply reply;
  long savecurdir;
  short savesfsavedisk;
  WindowPeek wp;

  wp = (WindowPeek) FrontWindow ();
  savecurdir = CurDirStore;
  savesfsavedisk = SFSaveDisk;
  if ((WindowPtr) wp != g_hotband && wp->windowKind == userKind)
    {
      CurDirStore = (*(opendirinfo **) wp->refCon)->iodirid;
      SFSaveDisk = -(*(opendirinfo **) wp->refCon)->vrefnum;
    }
#ifdef THINK_C
  SetPt (&pt, (screenBits.bounds.right - screenBits.bounds.left - STDFILEWIDTH) / 2,
	 100);
  SFPutFile (pt, (StringPtr) "\pNew Folder Name:", (StringPtr) "\pUntitled Folder",
	     (ProcPtr) 0, &reply);
#else
  SetPt (&pt, (qd.screenBits.bounds.right - qd.screenBits.bounds.left - STDFILEWIDTH) / 2,
	 100);
  SFPutFile (pt, (StringPtr) "\pNew Folder Name:", (StringPtr) "\pUntitled Folder",
	     (DlgHookUPP) 0, &reply);
#endif
  if (reply.good)
    {
#ifdef THINK_C
      createdir (-SFSaveDisk, CurDirStore, reply.fName);
#else
      createdir (-LMGetSFSaveDisk (), LMGetCurDirStore (), reply.fName);
#endif
#ifdef THINK_C
      changewindow (reply.fName, CurDirStore, -SFSaveDisk, updatemove);
#else
      changewindow (reply.fName, LMGetCurDirStore (), -LMGetSFSaveDisk (), updatemove);
#endif
    }
  CurDirStore = savecurdir;
  SFSaveDisk = savesfsavedisk;
}

void
openitem (void)
{
  if ((**g_selection)[0] != 0)
    activateicon ((**g_selection)[0]);
}

void
printitem (void)
{
  launchcreator ((**g_selection)[0], appPrint);
}

int
is_closable (WindowPtr wp)
{
  return wp != g_hotband
    && ((WindowPeek) wp)->windowKind == userKind;
}

void
closeitem (void)
{
  WindowPtr wp;

  wp = FrontWindow ();
  if (is_closable (wp))
    disposedirwindow (wp);
}

void
saveitem (void)
{
/* todo */
}

void
undolast (void)
{
/* todo */
}

void
duplicatename (Str255 name)
{
/* todo: put in .h file */
#define MAXNAMELEN	31
#define SUFFIX	" copy"
  short i, j;

  name[0] += sizeof (SUFFIX) -1;
  if (name[0] > MAXNAMELEN)
    name[0] = MAXNAMELEN;
  for (i = name[0] - sizeof (SUFFIX) , j = 0; i <= name[0]; i++, j++)
    name[i] = SUFFIX[j];
}

void
duplicate (void)
{
  item **ih;
  ControlHandle c;
  Str255 newname;
  unsigned char state;

  c = (**g_selection)[0];
  mystr255copy (newname, (*c)->contrlTitle);
  duplicatename (newname);
  ih = (item **) (*c)->contrlData;
  state = HGetState ((Handle) c);
  HLock ((Handle) c);
  if (duplicate1file ((*ih)->vrefnum, (*ih)->vrefnum, (*ih)->ioparid,
		      (*ih)->ioparid, (*c)->contrlTitle, newname, true))
    changewindow (newname, (*ih)->ioparid, (*ih)->vrefnum, updatemove);
  HSetState ((Handle) c, state);
}

/* todo: move to .h file */
#define RENAMEDIALOGID	202
#define NEWNAMEITEM		3

void
renameselection (void)
{
  Rect r;
  item **ih;
  short itemnum, type;
  Handle itemh;
  DialogPtr dp;
  HParamBlockRec pb;
  ControlHandle c;
  OSErr e;
  Str255 s;
  unsigned char state;

  dp = GetNewDialog (RENAMEDIALOGID, (Ptr) 0, (WindowPtr) - 1);
#ifdef THINK_C
  ModalDialog ((ProcPtr) 0, &itemnum);
#else
  ModalDialog ((ModalFilterUPP) 0, &itemnum);
#endif
  if (itemnum == OK)
    {
      GetDItem (dp, NEWNAMEITEM, &type, &itemh, &r);
      GetIText (itemh, s);
      c = (**g_selection)[0];
      ih = (item **) (*c)->contrlData;
      state = HGetState ((Handle) c);
      HLock ((Handle) c);
      pb.ioParam.ioNamePtr = (*c)->contrlTitle;
      pb.ioParam.ioMisc = (Ptr) s;
      pb.ioParam.ioVRefNum = (*ih)->vrefnum;
      pb.fileParam.ioDirID = (*ih)->ioparid;
      e = PBHRename (&pb, false);
      if (e == noErr)
	{
	  if (is_volume (c))
	    s[++s[0]] = ':';
	  SetCTitle (c, s);
	}
      else
	{
	  doerror (e, (StringPtr) "\pPBHRename");
	}
      HSetState ((Handle) c, state);
    }
  DisposDialog (dp);
}

void
deleteselection (void)
{
  ControlHandle c;
  item **ih;
  short vrefnum;
  long parid;
  unsigned char state;

/* Don't worry about c being null because the menu choice would not have been
 * hilit
 */
  c = (**g_selection)[0];
  if ((*c)->contrlOwner == g_hotband)
    {
      (**g_selection)[0] = 0;
      removeitemfromhotband (c);
    }
  else
    {
      ih = (item **) (*c)->contrlData;
      vrefnum = (*ih)->vrefnum;
      parid = (*ih)->ioparid;
      changehot (c, 0, 0);
      (**g_selection)[0] = 0;
      state = HGetState ((Handle) c);
      HLock ((Handle) c);
      if (delete1file (vrefnum, parid, (*c)->contrlTitle))
	changewindow ((*c)->contrlTitle, parid, vrefnum, removefromlist);
      HSetState ((Handle) c, state);
    }
}

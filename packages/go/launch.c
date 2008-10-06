#include "go.h"

#ifndef THINK_C
#include <Assembler.h>
#include <Traps.h>
#include <stdlib.h>
#endif /* THINK_C */
#include "launch.proto.h"
#include "misc.proto.h"

static void
adjust_sfpvars (const WDPBRec * wdpbrp, int vrefnum)
{
  SFSaveDisk = -vrefnum;
  CurDirStore = wdpbrp->ioWDDirID;
}

void
mylaunch (short vrefnum, long parid, Str255 s, int adjustflag)
{
  WDPBRec wdpb;
  OSErr e1, e2;

  wdpb.ioNamePtr = (StringPtr) 0;
  wdpb.ioVRefNum = vrefnum;
  wdpb.ioWDDirID = parid;
  wdpb.ioWDProcID = CREATOR;
  e1 = PBOpenWD (&wdpb, false);
  savestate ();
  e2 = SetVol ((StringPtr) 0, wdpb.ioVRefNum);
  if (e1 == noErr && e2 == noErr)
    {
      if (adjustflag)
	adjust_sfpvars (&wdpb, vrefnum);
      asm
      {
	move.l s, (a0)
	  clr.w 4 (a0)
	  _Launch
      }
    }
  ParamText (s, "\p could not be launched.", 0, 0);
  StopAlert (FOUR_PARAM_ALERT, 0);
}

void
launchapp (ControlHandle c)
{
  unsigned char state;

  state = HGetState ((Handle) c);
  HLock ((Handle) c);
  mylaunch ((*(item **) (*c)->contrlData)->vrefnum,
	    (*(item **) (*c)->contrlData)->ioparid, (*c)->contrlTitle, true);
  HSetState ((Handle) c, state);
}

void
launchcreator (ControlHandle c, short message)
{
  short hashval;
  OSType csig;
  applist **ah;
  item **ih;
  char *p;
  OSErr e;
  unsigned char state;
  WDPBRec wdpb;
  HParamBlockRec pb;
  int adjust_flag;

  csig = (*(*(item **) (*c)->contrlData)->iconfam)->sig;
  hashval = (unsigned long) csig % SIGARRAYSIZE;
  for (ah = sigowners[hashval]; ah != 0 && (*ah)->sig != csig; ah = (*ah)->next)
    ;
  pb.fileParam.ioVRefNum = (*(item **) (*c)->contrlData)->vrefnum;
  pb.fileParam.ioFDirIndex = 0;
  pb.fileParam.ioDirID = (*(item **) (*c)->contrlData)->ioparid;
  pb.fileParam.ioNamePtr = (*c)->contrlTitle;
  e = PBHGetFInfo (&pb, false);
  if (ah == 0)
    {
      if (e == noErr && pb.fileParam.ioFlFndrInfo.fdType == 'TEXT')
	ah = sigowners[TEXTEDITORPOS];
    }
  if (ah != 0)
    {
#if 0
      SetHandleSize (AppParmHandle, 14 + (*c)->contrlTitle[0]);
#else /* 0 */
#ifdef THINK_C
      TheZone = SysZone;
      AppParmHandle = NewHandle (14 + (*c)->contrlTitle[0]);
      TheZone = ApplZone;
#else
      LMSetTheZone (LMGetSysZone ());
      LMSetAppParmHandle (NewHandle (14 + (*c)->contrlTitle[0]));
      LMSetTheZone (LMGetApplZone ());
#endif
#endif /* 0 */
      if (MemError () == noErr)
	{
	  ih = (item **) (*c)->contrlData;
	  state = HGetState (AppParmHandle);
	  HLock (AppParmHandle);
#ifdef THINK_C
	  p = *AppParmHandle;
#else
	  p = *LMGetAppParmHandle ();
#endif
	  *(short *) p = message;
	  *(short *) (p + 2) = 1;
	  wdpb.ioNamePtr = (StringPtr) 0;
	  wdpb.ioVRefNum = (*(item **) (*c)->contrlData)->vrefnum;
	  wdpb.ioWDDirID = (*(item **) (*c)->contrlData)->ioparid;
	  wdpb.ioWDProcID = CREATOR;
	  e = PBOpenWD (&wdpb, false);
	  ((AppFile *) (p + 4))->vRefNum = wdpb.ioVRefNum;
	  ((AppFile *) (p + 4))->fType = pb.fileParam.ioFlFndrInfo.fdType;
	  ((AppFile *) (p + 4))->versNum = 0;
	  mystr255copy (((AppFile *) (p + 4))->fName, (*c)->contrlTitle);
	  adjust_sfpvars (&wdpb, (*(item **) (*c)->contrlData)->vrefnum);
	  adjust_flag = false;
	  HSetState (AppParmHandle, state);
	}
      else
	adjust_flag = true;
      mylaunch ((*ah)->vrefnum, (*ah)->parid, (*ah)->name, adjust_flag);
    }
}

void
launchda (ControlHandle c)
{
  c = c;
/* TODO */
}

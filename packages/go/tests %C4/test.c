#define THEICONID	128

#include "test.proto.h"

void InitToolbox (void);

void
InitToolbox (void)
{
  InitGraf ((Ptr) & qd.thePort);
  InitFonts ();
  InitWindows ();
  InitMenus ();
  FlushEvents (everyEvent, 0);
  TEInit ();
  InitDialogs (0L);
  InitCursor ();
}

extern GDHandle theGDevice:0xCC8;

void
drawicl8 (Ptr pat, Ptr mask, CWindowPtr w)
{
  PixMap spm;
  BitMap mbm;
  Rect sr, dr;

  SetRect (&sr, 0, 0, 32, 32);
  spm.baseAddr = pat;
  spm.rowBytes = 32 | 0x8000;
  SetRect (&spm.bounds, 0, 0, 32, 32);
  spm.pmVersion = 0;
  spm.packType = 0;
  spm.packSize = 0;
  spm.hRes = spm.vRes = (Fixed) 72;
  spm.pixelType = 0;
  spm.pixelSize = 8;
  spm.cmpCount = 1;
  spm.cmpSize = 8;
  spm.planeBytes = 0;
#if 1
  spm.pmTable = (*w->portPixMap)->pmTable;
#else /* 0 */
  spm.pmTable = (*(*theGDevice)->gdPMap)->pmTable;
#endif /* 0 */

  mbm.baseAddr = mask;
  mbm.rowBytes = 4;
  SetRect (&mbm.bounds, 0, 0, 32, 32);

  SetRect (&sr, 0, 0, 32, 32);
  SetRect (&dr, 10, 10, 42, 42);
  HLock ((Handle) w->portPixMap);
  CopyMask ((BitMap *) & spm, &mbm, (BitMap *) * w->portPixMap, &sr, &sr, &dr);
  HUnlock ((Handle) w->portPixMap);
}

main (void)
{
  CWindowPtr w;
  Handle h, h2;
  Rect r;

  InitToolbox ();

  SetRect (&r, 100, 100, 500, 300);
  w = (CWindowPtr) NewCWindow ((WindowPtr) 0, &r, "\ptest window", true, 0, (WindowPtr) - 1, true, 0L);
  h = GetResource ('icl8', THEICONID);
  h2 = GetResource ('ICN#', THEICONID);
  HLock (h);
  HLock (h2);
#ifndef CLIFFISNOTADORK
  drawicl8 (*h, *h2 + 128, w);
#else
  SetRect (&r, 120, 120, 152, 152);
  PlotCIcon (&r, (CIconHandle) h);
#endif
  HLock (h2);
  HLock (h);
  return 0;
}

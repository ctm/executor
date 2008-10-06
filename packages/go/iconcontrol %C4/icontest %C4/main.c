void drawicxx (Ptr pat, BitMap * mbm, Rect * dest, CWindowPtr w, short size, short depth);
void dodrawcontrol (CWindowPtr wp, short x, short y, short rdepth, short resid,
		    short rsize);

extern GDHandle theGDevice:0xCC8;

#define FIRSTICON		100
#define LASTICON		108

void InitToolbox (void);

void
InitToolbox ()
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
void
drawicxx (Ptr pat, BitMap * mbm, Rect * dest, CWindowPtr w, short size, short depth)
{
  PixMap spm;
  Rect sr;
  unsigned char state;
  short i;

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
/* TODO: figure out the correct way to get the CTabHandle */
#if 1
  spm.pmTable = (*w->portPixMap)->pmTable;
#else
  i = 127;
  spm.pmTable = GetCTable (i);
#endif

  SetRect (&sr, 0, 0, size, size);
  state = HGetState ((Handle) w->portPixMap);
  HLock ((Handle) w->portPixMap);
  CopyMask ((BitMap *) & spm, mbm, (BitMap *) * w->portPixMap, &sr, &sr, dest);
  HSetState ((Handle) w->portPixMap, state);
}

void
dodrawcontrol (CWindowPtr wp, short x, short y, short rdepth, short resid,
	       short rsize)
{
  Rect dr;
  Handle mask, data;
  short depth, size;
  BitMap mbm;
  unsigned char mstate, dstate;
  GrafPtr saveport;
  RGBColor blendcolor;

  GetPort (&saveport);
  SetPort ((GrafPtr) wp);
  SetRect (&dr, x, y, x + rsize, y + rsize);
  InsetRect (&dr, -1, -1);
  EraseRect (&dr);
  InsetRect (&dr, 1, 1);
  if (rdepth > (*(*theGDevice)->gdPMap)->pixelSize)
    rdepth = (*(*theGDevice)->gdPMap)->pixelSize;

  if (rsize == 16)
    {
      data = GetResource ('ics8', resid);
      if (data == 0 || rdepth < 8)
	{
	  data = GetResource ('ics4', resid);
	  if (data == 0 || rdepth < 4)
	    {
	      data = GetResource ('ics#', resid);
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
      mask = GetResource ('ics#', resid);
/*      assert ((mask == 0) ^ (data != 0)); */
    }
  if (rsize == 32 || (rsize == 16 && data == 0))
    {
      data = GetResource ('icl8', resid);
      if (data == 0 || rdepth < 8)
	{
	  data = GetResource ('icl4', resid);
	  if (data == 0 || rdepth < 4)
	    {
	      data = GetResource ('ICN#', resid);
/*              assert(data != 0); */
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
      mask = GetResource ('ICN#', resid);
/*      assert (mask != 0); */
    }

  dstate = HGetState (data);
  HLock (data);
  mstate = HGetState (mask);
  HLock (mask);
  mbm.baseAddr = *mask + size * size / 8;
  mbm.rowBytes = size / 8;
  SetRect (&mbm.bounds, 0, 0, size, size);
  drawicxx (*data, &mbm, &dr, wp, size, depth);
#if 0
  OpColor (halfwhite);
  hiliteMode &= ~(1 << hiliteBit);
  InvertRect (&dr);
#else
#if 0
  PenMode (blend);
  blendcolor.red = 32768;
  blendcolor.green = 32768;
  blendcolor.blue = 32768;
  OpColor (&blendcolor);
  PaintRect (&dr);
  PenNormal ();
#endif
#endif
  HSetState (data, dstate);
  HSetState (mask, mstate);
  SetPort (saveport);
}

main ()
{
  CWindowPtr wp;
  Rect r;
  short x, y, i;
  EventRecord ev;

  InitToolbox ();

  SetRect (&r, 10, 40, 600, 450);
  wp = (CWindowPtr) NewCWindow (wp, &r, "\ptest", true, documentProc,
				(WindowPtr) 0, true, 0);
  SelectWindow ((WindowPtr) wp);
  y = 20;
  for (i = FIRSTICON; i <= LASTICON; i++)
    {
      x = 10;
      dodrawcontrol (wp, x, y, 1, i, 32);
      x += 50;
      dodrawcontrol (wp, x, y, 4, i, 32);
      x += 50;
      dodrawcontrol (wp, x, y, 8, i, 32);
      x += 50;
      dodrawcontrol (wp, x, y, 1, i, 16);
      x += 25;
      dodrawcontrol (wp, x, y, 4, i, 16);
      x += 25;
      dodrawcontrol (wp, x, y, 8, i, 16);
      y += 40;
    }
  while (!WaitNextEvent (mDownMask, &ev, 0, (RgnHandle) 0))
    ;
  return 0;
}

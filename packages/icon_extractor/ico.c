#include "ico.h"

static void
init_ctab (ICONFILE *ifp)
{
  CTabHandle ctab;
  int n_entries, i;

  ctab = GetCTable (72);
  n_entries = (*ctab)->ctSize + 1;
  for (i = 0; i < n_entries; ++i)
    {
      RGBColor rgb;
      unsigned char xor_value;
      
      xor_value = (i == 0 || i == 255) ? 255 : 0;
      xor_value = 0;
      rgb = (*ctab)->ctTable[i^xor_value].rgb;
      ifp->IconData.Palette[i].rgbRed   = rgb.red   >> 8;
      ifp->IconData.Palette[i].rgbGreen = rgb.green >> 8;
      ifp->IconData.Palette[i].rgbBlue  = rgb.blue  >> 8;
    }
}

typedef short INTEGER;
typedef long LONGINT;

static void
write_file (const ICONFILE *ifp, short i)
{
  Str255 name;
  unsigned char *p;
  INTEGER rn;

  NumToString (i, name);
  p = name + name[0] + 1;
  p[0] = '.';
  p[1] = 'i';
  p[2] = 'c';
  p[3] = 'o';
  name[0] += 4;
  if (Create (name, 0, 'UNIX', 'TEXT') == noErr && FSOpen (name, 0, &rn) == noErr)
    {
      LONGINT count;

      count = sizeof *ifp;
      FSWrite (rn, &count, (Ptr) ifp);
      FSClose (rn);
    }
}

static void
write_an_ico (ICONFILE *ifp, short n, Handle icn_h, Handle icl8_h)
{
  int mask_offset;

  mask_offset = 32 * 32 / 8;
  LoadResource (icn_h);
  LoadResource (icl8_h);
  HLock (icn_h);
  HLock (icl8_h);
  {
    int i;
    unsigned char *ip;
    unsigned char *op;
    
    ip = (unsigned char *) *icl8_h;
    for (i = 31; i >= 0; --i)
      {
        int j;
        
        op = (unsigned char *) ifp->IconData.XorMap[i];
        for (j = 0; j < 32; ++j)
          {
          	unsigned char c;
          	
          	c = *ip++;
   /*
          	if (c == 0 || c == 255)
          		c ^= 255; */
          	*op++ = c;
          }
      }
    
  }

  {
      unsigned char *ip, *op;
      int i, j;
      
      ip = (unsigned char *)*icn_h + mask_offset;
      op = (unsigned char *)ifp->IconData.AndMap[31];
      for (i = 0; i < 32; ++i)
        {
          for (j = 0; j < 4; ++j)
            {
              unsigned char c;
              
              c = ~*ip++;
              *op++ = c;
              if (c & 128)
                ifp->IconData.XorMap[31-i][j*8+0] = 255;
              if (c & 64)
                ifp->IconData.XorMap[31-i][j*8+1] = 255;
              if (c & 32)
                ifp->IconData.XorMap[31-i][j*8+2] = 255;
              if (c & 16)
                ifp->IconData.XorMap[31-i][j*8+3] = 255;
              if (c & 8)
                ifp->IconData.XorMap[31-i][j*8+4] = 255;
              if (c & 4)
                ifp->IconData.XorMap[31-i][j*8+5] = 255;
              if (c & 2)
                ifp->IconData.XorMap[31-i][j*8+6] = 255;
              if (c & 1)
                ifp->IconData.XorMap[31-i][j*8+7] = 255;
            }
          op -= 8;
        }
  }

  write_file (ifp, n);

  HUnlock (icl8_h);
  HUnlock (icn_h);
}

static void
capture_icos (ICONFILE *ifp, INTEGER rn)
{
  int nres, n;
  INTEGER cur_rn;
  
  cur_rn = CurResFile ();

  UseResFile (rn);
  nres = Count1Resources('ICN#');
  for (n = 1; n <= nres; n++)
    {
      Handle icn_h;
      short i;
      ResType t;
      Str255 resname;

      icn_h = Get1IndResource('ICN#', n);
      if (icn_h)
	{
	  Handle icl8_h;

	  GetResInfo(icn_h, &i, &t, resname);
	  icl8_h = GetResource ('icl8', i);
	  UseResFile (cur_rn);
	  if (icl8_h)
	    write_an_ico (ifp, i, icn_h, icl8_h);
	  UseResFile (rn);
	}
    }
}

int
main(void)
{
  static ICONFILE ofile =
  {
    CWC (0),
    CWC (1),
    CWC (1),
    {
      CBC (32),
      CBC (32),
      CBC (0),
      CBC (0),
      CWC (0),
      CWC (0),
      CLC (2216),
      CLC (22),
    },
    {
      {
	CLC (40),
	CLC (32),
	CLC (64),
	CWC (1),
	CWC (8),
	CLC (0),
	CLC (1152),
	CLC (0),
	CLC (0),
	CWC (0),
	CWC (0),
      },
    },
  };
  
  int size = sizeof ofile;
  int iconDirSize = sizeof ofile.IconDir;
  int iconDataSize = sizeof ofile.IconData;
    INTEGER rn;
  
  	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
    init_ctab (&ofile);

  {
    Point p;
    /* SFTypeList sft; */
    SFReply sfp;
    INTEGER cur_rn;
    
    p.h = 0; p.v = 0;
  	SFGetFile (p, "\p", NULL, -1, (const unsigned long *) 0, NULL, &sfp);
#if 1
	cur_rn = CurResFile ();
	SetResLoad (false);
  	rn = HOpenResFile (sfp.vRefNum, 0, sfp.fName, fsCurPerm);
  	UseResFile (cur_rn);
  	SetResLoad (true);
#endif
  	}
  
  capture_icos (&ofile, rn);
  CloseResFile (rn);

  return 0;
}

#include "go.h"
#include "display.h"

#include "initdirs.proto.h"
#include "initicons.proto.h"
#include "init.proto.h"
#include "misc.proto.h"
#include "window.proto.h"

ControlHandle
addtolist (WindowPeek wp, Str255 s, long dirid, short vrefnum)
{
  ControlHandle c;
  opendirinfo **infoh;

  infoh = (opendirinfo **) wp->refCon;
  c = getnewiconcontrol ((WindowPtr) wp, (*infoh)->path, dirid, vrefnum, s);
  (**(*infoh)->items)[(*infoh)->numitems++] = c;
  return c;
}

void
updatemove (WindowPeek wp, Str255 sp, long dirid, short vrefnum)
{
  ControlHandle c;
  Str255 s;

  mystr255copy (s, sp);
  if (s[s[0]] == ':')
    --s[0];

  c = addtolist (wp, s, dirid, vrefnum);
  setoneicon (c);
  ShowControl (c);
  straightenwindow ((WindowPtr) wp);
}

void
removefromlist (WindowPeek wp, Str255 s, long dirid, short vrefnum)
{
  ControlHandle c, *cp;
  short i, n;

  cp = **(*(opendirinfo **) ((WindowPeek) wp)->refCon)->items;
  for (i = 0; RelString ((*cp[i])->contrlTitle, s, false, false); i++)
    ;
  c = cp[i];
  n = --(*(opendirinfo **) ((WindowPeek) wp)->refCon)->numitems;
  memmove(cp + i, cp + i + 1, n - i);
  DisposeControl (c);
}

void
initopendirs (FILE * f)
{
  CInfoPBRec cpb;
  CWindowPeek wp;
  short c;
  OSErr e;
  char **path;
  Rect r;
  short volume;

  while ((c = getc (f)) && (c != EOF))
    {
      ungetc (c, f);
      e = getonefileinfo (f, &cpb, &path, &volume);
      fscanf (f, "%d %d %d %d\n", &r.left, &r.top, &r.right, &r.bottom);
      if (!e && (cpb.hFileInfo.ioFlAttrib & DIRBIT))
	{
	  wp = (CWindowPeek) createdirwindow (&cpb, &r, path, volume);
	}
      for (wp = (CWindowPeek) WindowList; wp != 0; wp = wp->nextWindow)
	{
	  if (wp != (CWindowPeek) g_hotband)
	    setwindowicons (wp);
	  ShowWindow ((WindowPtr) wp);
	}
    }
}

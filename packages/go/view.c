#include "go.h"
#include "display.h"
#include <stdlib.h>
#include <packages.h>

#include "view.proto.h"
#include "misc.proto.h"

void
changeview (short view, Rect * r)
{
  WindowPtr wp;
  ControlHandle c;
  opendirinfo **ih;

  wp = FrontWindow ();
  ih = (opendirinfo **) ((WindowPeek) wp)->refCon;
  (*ih)->view = view;
  for (c = ((WindowPeek) wp)->controlList; c != (ControlHandle) 0;
       c = (*c)->nextControl)
    {
      if (c != (*ih)->sbar)
	{
	  MoveControl (c, 0, -5000);
	  (*(item **) (*c)->contrlData)->view = view;
	  SizeControl (c, r->right, r->bottom);
	}
    }
  straightenwindow (wp);
  ValidRect (&wp->portRect);
}

void
iconview (void)
{
  Rect r;

  SetRect (&r, 0, 0, ICONWIDTHUSED, ICONHEIGHTUSED);
  changeview (ICONVIEW, &r);
}

void
icsview (void)
{
  Rect r;

  SetRect (&r, 0, 0, ICSWIDTHUSED, ICSHEIGHTUSED);
  changeview (ICSVIEW, &r);
}

void
listingview (void)
{
  Rect r;

  SetRect (&r, 0, 0, LISTWIDTHUSED, LISTHEIGHTUSED);
  changeview (LISTVIEW, &r);
}

#if 0
void
updatewin (...)
{
  infoh = (opendirinfo **) ((WindowPeek) wp)->refCon;
  for (i = 0; i < (*infoh)->numitems; i++)
    {
      dir->hFileInfo.ioDirID = (*infoh)->iodirid;
      e = PBGetCatInfo (dir, false);
    }
}
#endif /* 0 */

void
sortwin (int (*f) (const void *, const void *), short order)
{
  WindowPtr wp;
  opendirinfo **infoh;
  bandinfo *p;
  unsigned char state;

  wp = FrontWindow ();
  if (wp == g_hotband)
    {
      p = &bands[g_currentband];
      p->sortorder = order;
      state = HGetState ((Handle) p->items);
      HLock ((Handle) p->items);
      qsort (**p->items, p->numitems, sizeof (ControlHandle), f);
      HSetState ((Handle) p->items, state);
      setband (g_currentband);
    }
  else
    {
      infoh = (opendirinfo **) ((WindowPeek) wp)->refCon;
      (*infoh)->sortorder = order;
      state = HGetState ((Handle) (*infoh)->items);
      HLock ((Handle) (*infoh)->items);
      qsort (**(*infoh)->items, (*infoh)->numitems, sizeof (ControlHandle), f);
      HSetState ((Handle) (*infoh)->items, state);
      straightenwindow (wp);
    }
}

int
namecmp (const void *x, const void *y)
{
  return RelString ((**(ControlHandle *) x)->contrlTitle,
		    (**(ControlHandle *) y)->contrlTitle, false, false);
}

void
namesort (void)
{
  sortwin (namecmp, ALPHABETIC);
}

int
datecmp (const void *x, const void *y)
{
  long xval, yval;

  xval = (*(item **) (**(ControlHandle *) x)->contrlData)->moddate;
  yval = (*(item **) (**(ControlHandle *) y)->contrlData)->moddate;
  if (xval > yval)
    return -1;
  else
    return xval < yval;
}

void
moddatesort (void)
{
  sortwin (datecmp, MODDATE);
}

int
sizecmp (const void *x, const void *y)
{
  long xval, yval;

  xval = (*(item **) (**(ControlHandle *) x)->contrlData)->size;
  yval = (*(item **) (**(ControlHandle *) y)->contrlData)->size;
  if (xval > yval)
    return -1;
  else
    return xval < yval;
}

void
sizesort (void)
{
  sortwin (sizecmp, SIZE);
}

void
defaultsort (void)
{
  WindowPtr wp;
  short order;

  wp = FrontWindow ();
  if (wp == g_hotband)
    order = bands[g_currentband].sortorder;
  else
    order = (*(opendirinfo **) ((WindowPeek) wp)->refCon)->sortorder;

  switch (order)
    {
    case ALPHABETIC:
      namesort ();
      break;
    case SIZE:
      sizesort ();
      break;
    case MODDATE:
      moddatesort ();
      break;
    }
}

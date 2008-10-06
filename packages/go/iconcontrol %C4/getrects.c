/* #include "go.h" */
#include "iconcontrol.h"
#include <assert.h>

#include "getrects.proto.h"

void
getrects (ControlHandle c, Rect * textr, Rect * iconr)
{
  short len, mid;
  short textsize, textfont;
  WindowPtr saveport;

  GetPort (&saveport);
  SetPort ((*c)->contrlOwner);
  textfont = (*c)->contrlOwner->txFont;
  textsize = (*c)->contrlOwner->txSize;
  TextSize (FONTSIZE);
  TextFont (applFont);
  len = StringWidth ((*c)->contrlTitle);
  switch ((*(item **) (*c)->contrlData)->view)
    {
    case ICONVIEW:
      mid = ((*c)->contrlRect.right + (long) (*c)->contrlRect.left) / 2;
      SetRect (textr, mid - len / 2 - 1, (*c)->contrlRect.top + ICONSIZE,
	       mid + len / 2 + 1, (*c)->contrlRect.bottom);
      SetRect (iconr, mid - ICONSIZE / 2, (*c)->contrlRect.top,
	       mid + ICONSIZE / 2, (*c)->contrlRect.top + ICONSIZE);
      break;
    case ICSVIEW:
      mid = ((*c)->contrlRect.top + (*c)->contrlRect.bottom) / 2;
      SetRect (textr, (*c)->contrlRect.left + ICSSIZE, (*c)->contrlRect.top,
	    (*c)->contrlRect.left + ICSSIZE + len, (*c)->contrlRect.bottom);
      SetRect (iconr, (*c)->contrlRect.left, mid - ICSSIZE / 2,
	       (*c)->contrlRect.left + ICSSIZE, mid + ICSSIZE / 2);
      break;
    case LISTVIEW:
      SetRect (textr, (*c)->contrlRect.left, (*c)->contrlRect.top,
	   (*c)->contrlOwner->portRect.right - 15, (*c)->contrlRect.bottom);
      SetRect (iconr, 0, 0, 0, 0);
      break;
    default:
      assert (0);
      break;
    }
  TextSize (textsize);
  TextFont (textfont);
  SetPort (saveport);
}

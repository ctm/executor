#include "go.h"

#include "easymenu.proto.h"

TEHandle g_currenttitle;

void
nothing (void)
{
}

void
aboutgo (void)
{
  short item, h, v;
  DialogPtr dp;

#define DATESTR (StringPtr)"\pApril 30, 1995"
  ParamText (DATESTR, 0, 0, 0);
  dp = GetNewDialog (ABOUTDIALOG, 0, (WindowPtr) - 1);
  h = (qd.screenBits.bounds.right - dp->portRect.right) / 2;
  v = (qd.screenBits.bounds.bottom - dp->portRect.bottom) / 3;
  MoveWindow (dp, h, v, true);
  ShowWindow (dp);
/* todo: if about adds a help button make this more complex */
  ModalDialog (0, &item);
  DisposDialog (dp);
}

void
quitgo (void)
{
  g_done = true;
}

void
gocut (void)
{
  TECut (g_currenttitle);
}

void
gocopy (void)
{
  TECopy (g_currenttitle);
}

void
gopaste (void)
{
  /* TODO: if ... + ... > 31 { ... } */
  TEPaste (g_currenttitle);
}

void
goclear (void)
{
  TEDelete (g_currenttitle);
}

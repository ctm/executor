#include "xfer.h"

#include "alerts.proto.h"

short abortflag;

extern long sizetocopy, sizecopied;
extern short BreakCopy;
extern DialogPtr piechartdp;

void
doerror (OSErr errno, StringPtr s)
{
  static errortable errormessages[] =
  {
    wPrErr, "\pThat volume is locked.",
    vLckdErr, "\pThat volume is locked.",
    permErr, "\pPermission error writing to locked file.",
    ioErr, "\pI/O Error.",
    bdNamErr, "\pBad name (1 <= name length <= 31, no colons).",
  };

  short i;
  Str255 s2;

  if (abortflag)
/*-->*/ return;

  for (i = 0; i < NELEM (errormessages) && errormessages[i].number != errno
       ; i++)
    ;
  if (i < NELEM (errormessages))
    {
      ParamText ((StringPtr) errormessages[i].message, 0, 0, 0);
#ifdef THINK_C
      abortflag = StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0) == ABORTITEM;
#else
      abortflag = StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0) == ABORTITEM;
#endif
    }
  else
    {
      NumToString ((long) errno, s2);
      ParamText (s2, (StringPtr) s, 0, 0);
#ifdef THINK_C
      abortflag = CautionAlert (DOERRORABORTALERT, (ProcPtr) 0) == ABORTITEM;
#else
      abortflag = CautionAlert (DOERRORABORTALERT, (ModalFilterUPP) 0) == ABORTITEM;
#endif
    }
}

void
noroom (long needed, long avail)
{
  Str255 s1, s2;

  NumToString (needed, s1);
  NumToString (avail, s2);
  ParamText (s1, s2, 0, 0);
#ifdef THINK_C
  StopAlert (NOROOMALERTID, (ProcPtr) 0);
#else
  StopAlert (NOROOMALERTID, (ModalFilterUPP) 0);
#endif
}

void
copyintochildrenwarning (void)
{
  ParamText ((StringPtr) "\pYou can't copy files onto themselves or copy folders into their children",
	     0, 0, 0);
#ifdef THINK_C
  NoteAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
  NoteAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
}

void
warnaboutincest (void)
{
  ParamText ((StringPtr) "\pFolders may not be transfered to their offspring.",
	     0, 0, 0);
#ifdef THINK_C
  StopAlert (FOUR_PARAM_ALERT, (ProcPtr) 0);
#else
  StopAlert (FOUR_PARAM_ALERT, (ModalFilterUPP) 0);
#endif
}

void
updatepiechart (void)
{
  Rect r;
  Handle h;
  short itype;
  GrafPtr saveport;
#ifdef THINK_C
  static LONGINT blackl[2] =
  {-1, -1};
#endif

  if (piechartdp)
    {
      GetPort (&saveport);
      SetPort (piechartdp);
      GetDItem (piechartdp, PIECHARTITEM, &itype, &h, &r);
      FrameOval (&r);
#ifdef THINK_C
      FillArc (&r, 0, (long) sizecopied * 360 / sizetocopy, (ConstPatternParam) blackl);
#else
      FillArc (&r, 0, (short) sizecopied * 360 / sizetocopy, &qd.black);
#endif
      SetPort (saveport);
    }
}

void
makepiechart (void)
{
  if (sizetocopy > 0)
    {
      piechartdp = GetNewDialog (PIECHARTID, (Ptr) 0, (WindowPtr) - 1);
      SetPort (piechartdp);
      DrawDialog (piechartdp);
    }
  else
    piechartdp = 0;
}

short
ask (StringPtr s1, StringPtr s2)
{
  short retval;

  ParamText ((StringPtr) s1, s2, (StringPtr) 0, (StringPtr) 0);
#ifdef THINK_C
  retval = CautionAlert (ASKALERT, (ProcPtr) 0);
#else
  retval = CautionAlert (ASKALERT, (ModalFilterUPP) 0);
#endif
  if (sizetocopy > 0)
    DrawDialog (piechartdp);
  return retval;
}

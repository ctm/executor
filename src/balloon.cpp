/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_balloon[] =
		"$Id: balloon.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "HelpMgr.h"

using namespace Executor;

P0 (PUBLIC pascal trap, BOOLEAN, HMGetBalloons)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P1 (PUBLIC pascal trap, OSErr, HMSetBalloons, BOOLEAN, flag)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P0 (PUBLIC pascal trap, BOOLEAN, HMIsBalloon)
{
  warning_unimplemented (NULL_STRING);
  return FALSE;
}

P7 (PUBLIC pascal trap, OSErr, HMShowBalloon, HMMessageRecord *, msgp,
    Point, tip, RectPtr, alternaterectp, Ptr, tipprocptr, INTEGER, proc,
    INTEGER, variant, INTEGER, method)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P9 (PUBLIC pascal trap, OSErr, HMShowMenuBalloon, INTEGER, item,
    INTEGER, menuid, LONGINT, flags, LONGINT, itemreserved, Point, tip,
    RectPtr, alternaterectp, Ptr, tipproc, INTEGER, proc, INTEGER, variant)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P0 (PUBLIC pascal trap, OSErr, HMRemoveBalloon)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMGetHelpMenuHandle, MenuHandle *, mhp)
{
  warning_unimplemented (NULL_STRING);
  *mhp = (MenuHandle)CLC (0);
  return noErr;
}

P1 (PUBLIC pascal trap, OSErr, HMGetFont, INTEGER *, fontp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMGetFontSize, INTEGER *, sizep)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMSetFont, INTEGER, font)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMSetFontSize, INTEGER, size)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMSetDialogResID, INTEGER, resid)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMGetDialogResID, INTEGER *, residp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P2 (PUBLIC pascal trap, OSErr, HMSetMenuResID, INTEGER, menuid, INTEGER, resid)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P2 (PUBLIC pascal trap, OSErr, HMGetMenuResID, INTEGER *, menuidp,
    INTEGER *, residp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P3 (PUBLIC pascal trap, OSErr, HMScanTemplateItems, INTEGER, whichid,
    INTEGER, whicresfile, ResType, whictype)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P2 (PUBLIC pascal trap, OSErr, HMBalloonRect, HMMessageRecord *,
    messp, Rect *, rectp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P2 (PUBLIC pascal trap, OSErr, HMBalloonPict, HMMessageRecord *, messp,
    PicHandle *, pictp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P1 (PUBLIC pascal trap, OSErr, HMGetBalloonWindow, WindowPtr *, windowpp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

P5 (PUBLIC pascal trap, OSErr, HMExtractHelpMsg, ResType, type, INTEGER, resid,
    INTEGER, msg, INTEGER, state, HMMessageRecord *, helpmsgp)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}

/* #warning HMGetIndHelpMsg totally broken -- it would need a P11 */

#if 0
P 9 (PUBLIC pascal trap, OSErr, HMGetIndHelpMsg, ResType, type, INTEGER, resid,
     INTEGER, msg, INTEGER, state, LONGINT *, options, Point, tip,
     Rect *, altrectp, INTEGER *, theprocp, INTEGER *, variantp,
     HMMessageRecord *, helpmsgp, INTEGER *, count)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}
#else
PUBLIC OSErr
HMGetIndHelpMsg (ResType type, INTEGER resid,
		 INTEGER msg, INTEGER state, LONGINT * options, Point tip,
		 Rect *altrectp, INTEGER *theprocp, INTEGER *variantp,
		 HMMessageRecord *helpmsgp, INTEGER * count)
{
  warning_unimplemented (NULL_STRING);
  return hmHelpManagerNotInited;
}
#endif

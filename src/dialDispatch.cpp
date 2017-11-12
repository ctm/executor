/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined(OMIT_RCSID_STRINGS)
char ROMlib_rcsid_dialDispatch[] = "$Id: dialDispatch.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "EventMgr.h"
#include "rsys/pstuff.h"

#include "DialogMgr.h"

using namespace Executor;

/* traps from the DialogDispatch trap */

P1(PUBLIC pascal trap, OSErr, GetStdFilterProc, GUEST<ProcPtr> *, proc)
{
    *proc = RM((ProcPtr)P_ROMlib_myfilt);
    warning_unimplemented("no specs"); /* i.e. no documentation on how this
					 routine is *supposed* to work, so
					 we may be blowing off something
					 important */
    return noErr;
}

P2(PUBLIC pascal trap, OSErr, SetDialogDefaultItem, DialogPtr, dialog,
   int16, new_item)
{
    DialogPeek dp;

    dp = (DialogPeek)dialog;

    dp->aDefItem = CW(new_item);
    warning_unimplemented("no specs");
    return noErr;
}

/* These two probably adjust stuff that doesn't appear in Stock System 7 */

P2(PUBLIC pascal trap, OSErr, SetDialogCancelItem, DialogPtr, dialog,
   int16, new_item)
{
    warning_unimplemented(NULL_STRING);
    return noErr; /* noErr is likely to be less upsetting than paramErr -- ctm */
}

P2(PUBLIC pascal trap, OSErr, SetDialogTracksCursor, DialogPtr, dialog,
   Boolean, tracks)
{
    warning_unimplemented(NULL_STRING);
    return noErr; /* paramErr is too harsh */
}

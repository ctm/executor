/* Copyright 1995 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#include "rsys/common.h"
#include "EventMgr.h"
#include "rsys/pstuff.h"

#include "DialogMgr.h"

using namespace Executor;

/* traps from the DialogDispatch trap */

OSErr Executor::C_GetStdFilterProc(GUEST<ProcPtr> *proc)
{
    *proc = RM((ProcPtr)&ROMlib_myfilt);
    warning_unimplemented("no specs"); /* i.e. no documentation on how this
					 routine is *supposed* to work, so
					 we may be blowing off something
					 important */
    return noErr;
}

OSErr Executor::C_SetDialogDefaultItem(DialogPtr dialog, int16_t new_item)
{
    DialogPeek dp;

    dp = (DialogPeek)dialog;

    dp->aDefItem = CW(new_item);
    warning_unimplemented("no specs");
    return noErr;
}

/* These two probably adjust stuff that doesn't appear in Stock System 7 */

OSErr Executor::C_SetDialogCancelItem(DialogPtr dialog, int16_t new_item)
{
    warning_unimplemented(NULL_STRING);
    return noErr; /* noErr is likely to be less upsetting than paramErr -- ctm */
}

OSErr Executor::C_SetDialogTracksCursor(DialogPtr dialog, Boolean tracks)
{
    warning_unimplemented(NULL_STRING);
    return noErr; /* paramErr is too harsh */
}

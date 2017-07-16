/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_qStandard[] =
		    "$Id: qStandard.c 63 2004-12-24 18:19:43Z ctm $";
#endif

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/pstuff.h"

using namespace Executor;

#if !defined (BINCOMPAT)
#define PTRCAST (Ptr)
#else /* BINCOMPAT */
#define PTRCAST
#endif /* BINCOMPAT */

P1(PUBLIC pascal trap, void, SetStdProcs, QDProcs *, procs)
{
    procs->textProc = (textProc_t)RM(PTRCAST P_StdText);
    procs->lineProc = (lineProc_t)RM(PTRCAST P_StdLine);
    procs->rectProc = (rectProc_t)RM(PTRCAST P_StdRect);
    procs->rRectProc = (rRectProc_t)RM(PTRCAST P_StdRRect);
    procs->ovalProc = (ovalProc_t)RM(PTRCAST P_StdOval);
    procs->arcProc = (arcProc_t)RM(PTRCAST P_StdArc);
    procs->polyProc = (polyProc_t)RM(PTRCAST P_StdPoly);
    procs->rgnProc = (rgnProc_t)RM(PTRCAST P_StdRgn);
    procs->bitsProc = (bitsProc_t)RM(PTRCAST P_StdBits);
    procs->commentProc = (commentProc_t)RM(PTRCAST P_StdComment);
    procs->txMeasProc = (txMeasProc_t)RM(PTRCAST P_StdTxMeas);
    procs->getPicProc = (getPicProc_t)RM(PTRCAST P_StdGetPic);
    procs->putPicProc = (putPicProc_t)RM(PTRCAST P_StdPutPic);
}

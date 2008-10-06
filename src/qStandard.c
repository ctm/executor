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

#if !defined (BINCOMPAT)
#define PTRCAST (Ptr)
#else /* BINCOMPAT */
#define PTRCAST
#endif /* BINCOMPAT */

P1(PUBLIC pascal trap, void, SetStdProcs, QDProcs *, procs)
{
    procs->textProc = RM(PTRCAST P_StdText);
    procs->lineProc = RM(PTRCAST P_StdLine);
    procs->rectProc = RM(PTRCAST P_StdRect);
    procs->rRectProc = RM(PTRCAST P_StdRRect);
    procs->ovalProc = RM(PTRCAST P_StdOval);
    procs->arcProc = RM(PTRCAST P_StdArc);
    procs->polyProc = RM(PTRCAST P_StdPoly);
    procs->rgnProc = RM(PTRCAST P_StdRgn);
    procs->bitsProc = RM(PTRCAST P_StdBits);
    procs->commentProc = RM(PTRCAST P_StdComment);
    procs->txMeasProc = RM(PTRCAST P_StdTxMeas);
    procs->getPicProc = RM(PTRCAST P_StdGetPic);
    procs->putPicProc = RM(PTRCAST P_StdPutPic);
}

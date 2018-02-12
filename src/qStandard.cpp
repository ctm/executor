/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"

using namespace Executor;

void Executor::C_SetStdProcs(QDProcs *procs)
{
    procs->textProc = RM(&StdText);
    procs->lineProc = RM(&StdLine);
    procs->rectProc = RM(&StdRect);
    procs->rRectProc = RM(&StdRRect);
    procs->ovalProc = RM(&StdOval);
    procs->arcProc = RM(&StdArc);
    procs->polyProc = RM(&StdPoly);
    procs->rgnProc = RM(&StdRgn);
    procs->bitsProc = RM(&StdBits);
    procs->commentProc = RM(&StdComment);
    procs->txMeasProc = RM(&StdTxMeas);
    procs->getPicProc = RM(&StdGetPic);
    procs->putPicProc = RM(&StdPutPic);
}

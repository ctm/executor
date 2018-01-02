/* Copyright 1986, 1988, 1989, 1990 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

/* Forward declarations in QuickDraw.h (DO NOT DELETE THIS LINE) */

#include "rsys/common.h"
#include "QuickDraw.h"
#include "rsys/pstuff.h"

using namespace Executor;

PUBLIC pascal trap void Executor::C_SetStdProcs(QDProcs * procs)
{
    procs->textProc = RM((textProc_t)P_StdText);
    procs->lineProc = RM((lineProc_t)P_StdLine);
    procs->rectProc = RM((rectProc_t)P_StdRect);
    procs->rRectProc = RM((rRectProc_t)P_StdRRect);
    procs->ovalProc = RM((ovalProc_t)P_StdOval);
    procs->arcProc = RM((arcProc_t)P_StdArc);
    procs->polyProc = RM((polyProc_t)P_StdPoly);
    procs->rgnProc = RM((rgnProc_t)P_StdRgn);
    procs->bitsProc = RM((bitsProc_t)P_StdBits);
    procs->commentProc = RM((commentProc_t)P_StdComment);
    procs->txMeasProc = RM((txMeasProc_t)P_StdTxMeas);
    procs->getPicProc = RM((getPicProc_t)P_StdGetPic);
    procs->putPicProc = RM((putPicProc_t)P_StdPutPic);
}

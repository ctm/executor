/* Copyright 1992 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_snth5[] =
	    "$Id: snth5.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"
#include "SoundMgr.h"
#include "rsys/soundopts.h"

using namespace Executor;

/*
 * It's not really clear how an actual synthesizer is expected to work
 * with the Sound Manager's queues.  In this implementation we start
 * the relavent command playing and see that the ROMLIB_soundcomplete()
 * function is called when we're done.
 */

typedef pascal void (*callbackfp)(SndChannelPtr, SndCommand *);

P3(PUBLIC pascal, BOOLEAN, snth5, SndChannelPtr, chanp, SndCommand *, cmdp,
							   ModifierStubPtr, mp)
{
#if defined(MACOSX_)
    soundbuffer_t *bufp;
    BOOLEAN done;
    static BOOLEAN beenhere = 0;

    if (!beenhere) {
	/* ROMlib_soundreserve(); */
	beenhere = 1;
    }
    done = TRUE;
    switch (Cx(cmdp->cmd)) {
    case initCmd:
	SoundActive = soundactive5;
	/* TODO */
	break;
    case freeCmd:
	SoundActive = 0;
	done = FALSE;
	/* TODO */
	break;
    case quietCmd:
	/* TODO */
	break;
    case flushCmd:
	/* TODO */
	break;
    case waitCmd:
	/* TODO */
	break;
    case pauseCmd:
	/* TODO */
	break;
    case resumeCmd:
	/* TODO */
	break;
    case callBackCmd:
#if 0
	printf("CB"); fflush(stdout);
#endif
	CToPascalCall((void*)MR(chanp->callBack), CTOP_StuffHex, chanp, cmdp);
	break;
    case syncCmd:
	/* TODO */
	break;
    case availableCmd:
	done = FALSE;
	/* TODO */
	break;
    case bufferCmd:
	bufp = (soundbuffer_t *) Cx(cmdp->param2);
#if 0
	printf("offset = %d, nsamples = %d, rate = 0x%x\n", Cx(bufp->offset),
						   Cx(bufp->nsamples), Cx(bufp->rate));
	printf("BU"); fflush(stdout);
#endif
	ROMlib_outbuffer((char *) bufp->buf, Cx(bufp->nsamples), Cx(bufp->rate),
									chanp);
	done = FALSE;
	break;

    case requestNextCmd:	/* not needed */
    case tickleCmd:		/* not implemented */
    case howOftenCmd:		/* not implemented */
    case wakeUpCmd:		/* not implemented */
    case noteCmd:		/* not implemented */
    case restCmd:		/* not implemented */
    case freqCmd:		/* not implemented */
    case ampCmd:		/* not implemented */
    case timbreCmd:		/* not implemented */
    case waveTableCmd:		/* not implemented */
    case phaseCmd:		/* not implemented */
    case soundCmd:		/* not implemented */
    case rateCmd:		/* not implemented */
    case emptyCmd:		/* does nothing */
    case nullCmd:		/* does nothing */
    case midiDataCmd:		/* not implemented */
    default:
#if 1
	printf("unexpected sound command %d\n", (LONGINT) Cx(cmdp->cmd));
#endif
	break;
    }
    if (done)
	ROMlib_callcompletion(chanp);
#endif
    return FALSE;
}

/* Copyright 1994 by Abacus Research and
 * Development, Inc.  All rights reserved.
 */

#if !defined (OMIT_RCSID_STRINGS)
char ROMlib_rcsid_priv[] =
		"$Id: priv.c 63 2004-12-24 18:19:43Z ctm $";
#endif

#include "rsys/common.h"

using namespace Executor;

#if !defined (SYN68K)


/*
 * dopriv:  emulate instructions that play with sr
 * NOTE:  this isn't complete but it gets all the cases I've seen.
 *
 * The reason we have to do this is on a 68020 it requires supervisor mode
 * to *read* from the SR, much less write to it.  This is so if you want to
 * make a "virtual" 68020 that runs in user mode, the virtual OS can trap
 * any access to the SR and then fudge things so it looks like the user
 * mode program is really in supervisor mode (we don't do that, hopefully
 * noone cares, although it would be simple enough).
 *
 * Should anyone add to this routine so that more addressing modes are
 * understood, we'd be grateful if we could include your mods for everyone
 * to use, but of course you are under no obligation to turn them over to us.
 */

#define MOVESRMASK	0xFFC0

#define MOVEFROMSR	0x40C0
#define MOVETOSR	0x46C0
#define ANDISR		0x027C
#define ORISR		0x007C

#define DREGDIRECT		000
#define AREGINDIR		020
#define AREGINDIRPOSTINC	030
#define PREDECAREGINDIR		040
#define AREGPLUSD16INDIR	050
#define MODE7			070

#define REGMASK	7	/* the part of the instruction containing a register */
#define ADDROFFSET	8	/* we have d0 - d7 saved before a0 - a7 */
#define ADDRMODEMASK	070	/* where we find the addressing mode */

#define ABSDATA		4	/* for move #nnnn, sr */

#define IPLMASK		0x700

/*
 * These routines are here for historical purposes (when this stuff was
 * in the kernel)
 */

static short fusword(short *addr)
{
    return *addr;
}

static void susword(short *addr, short value)
{
    *addr = value;
}

static void canthandle( void )
{
#if 0 && defined(MACOSX_)  /* unexpectedsignal is gone */
    signal(SIGILL, (void *)unexpectedsignal);
#endif
}

void dopriv(long *regs)
{
    static short ipl = 0, oldipl = 0;
    static long curmask = 0;
    char srmodded;
    register short *pc;
    register unsigned short word;	/* scratch short */
    register short **spp;	/* a pointer to one of the Address registers */
				/* specifically the one used indirectly */

    pc = (short *) regs[16];
    word = fusword(pc);
    if (word == 0xF4F8) {
	flushcache();
	++pc;
	regs[16] = (long) pc;
/*-->*/	return;
    }
    spp = (short **) &regs[(word & REGMASK) + ADDROFFSET];
    srmodded = true;	/* most of em do mod the sr */
    switch (word & MOVESRMASK) {
    case MOVEFROMSR:
	srmodded = false;
	switch (word & ADDRMODEMASK) {
	case DREGDIRECT:
	    *((short *) &regs[word & REGMASK] + 1) = regs[17] | ipl;
	    ++pc;
	    break;
	case AREGINDIR:
	    susword(*spp, regs[17] | ipl);
	    ++pc;
	    break;
	case AREGINDIRPOSTINC:
	    susword((*spp)++, regs[17] | ipl);
	    ++pc;
	    break;
	case PREDECAREGINDIR:
	    susword(--*spp, regs[17] | ipl);
	    ++pc;
	    break;
	case AREGPLUSD16INDIR:
	    susword(*spp + fusword(pc+1), regs[17] | ipl);
	    pc += 2;
	    break;
	default:
/*-->*/	    canthandle();
	}
	break;
    case MOVETOSR:
	switch (word & ADDRMODEMASK) {
	case DREGDIRECT:
	    word = regs[word & REGMASK];
	    ++pc;
	    break;
	case AREGINDIR:
	    word = fusword(*spp);
	    ++pc;
	    break;
	case AREGINDIRPOSTINC:
	    word = fusword((*spp)++);
	    ++pc;
	    break;
	case PREDECAREGINDIR:
	    word = fusword(--*spp);
	    ++pc;
	    break;
	case AREGPLUSD16INDIR:
	    word = fusword(*spp + fusword(pc+1));
	    pc += 2;
	    break;
	case MODE7:
	    if ((word & REGMASK) != ABSDATA)
/*-->*/		canthandle();
	    word = fusword(pc+1);
	    pc += 2;
	    break;
	default:
/*-->*/	    canthandle();
	}
	regs[17] = word;
	ipl = word & IPLMASK;
	break;
    case ANDISR & MOVESRMASK:
	if (word != ANDISR) {
/*-->*/	    canthandle();
	}
	regs[17] = (regs[17]|ipl) & fusword(pc+1);
	ipl = word & IPLMASK;
	pc += 2;
	break;
    case ORISR & MOVESRMASK:
	if (word != ORISR) {
/*-->*/	    canthandle();
	}
	regs[17] |= fusword(pc+1);
	ipl |= regs[17] & IPLMASK;
	pc += 2;
	break;
    default:
/*-->*/	    canthandle();	/* not move sr or CINV or CPUSH */
    }
    if (ipl != oldipl) {
	if ((ipl & IPLMASK) == IPLMASK)
	    curmask = sigblock(sigmask(SIGALRM));
	else if ((oldipl & IPLMASK) == IPLMASK)
	    sigsetmask(curmask);
    }
    oldipl = ipl;
    regs[16] = (long) pc;
}

#endif /* !SYN68K */

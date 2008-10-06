/*
 * Although written by Abacus R&D, the source code in this file is not
 * copyrighted by Abacus R&D, but is in the public domain.
 */
    
/*
 * Revised ardimods:
 *
 * V1.1.1:
 *   In some instances either the copyin or copyout was corrupting the
 *   user stack pointer.  My guess is that when copyin or copyout took
 *   a page fault, the fault code made assumptions about the amount of
 *   state that had already been saved.  The assumptions were not correct,
 *   because the ARDImods short-circuits the saving of much state (that's
 *   what makes them fast).  The new strategy is to spl7(), set a flag
 *   that says we're special and do the copyin and copyout ourselves,
 *   noting whether or not we get a bus error.  If we get the error we'll
 *   just do things the old-fashioned, slow way, using UNIX style signal
 *   handlers.  This will rarely be the case, so we will still see tremendous
 *   speedups using the ARDImods.
 *
 * V1.2:
 *   Running "Globes" with "Sync to VBL" chosen can lock up machines.  The
 *   symptoms suggest that somehow an important interrupt is being missed
 *   and from that point on the machine continues running without ever
 *   switching tasks.  In light of this, the new strategy is to not use
 *   spl7, but to look at the return pc to determine whether or not we're
 *   in this code.
 *
 * V1.3:
 *   The bottom line is that without access to the NeXT kernel source, there's
 *   an incredibly slim chance that the fastaline code will cause a machine
 *   to hang.  For that reason, we've disabled that section of the code.
 *   However, if you're interested, you can uncomment out the lines in
 *
 * V1.4:
 *   Replacing rte with bra ardifpsp_done fixes the potential hang problem.
 *   Also moved all of the illegal instruction simulation into user space
 *   (with the exception of "movc d0, cacr", which is now totally in assembly).
 *   FASTALINETRAPS is now de rigueur since there's not even a one instruction
 *   window of vulnerability.  NOTE: a bug in NeXTSTEP2.x makes hangs still
 *   possible.  We haven't seen any hangs, nor do I believe they're possible
 *   in NS3.0.
 *
 * V1.5:
 *   NEXTSTEP 3.1 has totally changed fpsp_done.  We now use theirs and I
 *   believe they've fixed their bugs.  If you have a pre 3.1 version of
 *   NEXTSTEP, then you might want to use "ardifpsp_done" instead of
 *   NeXT's fpsp_done.
 *
 * V1.6:
 *   Some people are going to want to use these mods on 3.0 systems, so
 *   instead of making the user patch a line, we'll try to detect 3.0 systems
 *   ourselves.
 *
 * V1.7:
 *   Got rid of the stuff that temporarily disables the cache.  There's no
 *   need for it.  Macs have had '040's in them for a while now.
 *
 * V1.8:
 *   newsrstuff sets up location 0x54 with a copy of dma_chip so we can
 *   test for the turbo monitors for accelerated graphics.  This is not
 *   a pretty hack, but it's painless and gets the job done, while we're
 *   waiting for a better solution from NeXT.
 *
 */

	.data
	.align 2
oldalinevec:	.long	0	/* for dispatching A-Line requests */
oldprivvec:	.long	0	/* for handling moves to/from SR */
oldbusvec:	.long	0	/* for detecting page faults */

troublevec:	.long	0	/* used when we may want to back out
				   of a bus error on an 040 */

busbadness:	.byte	0	/* used when we detect a bus error on an 030 */

	.text
	.even

.globl _replace_vectors
_replace_vectors:
	movc	vbr,			a0
	movel	a0@(0x08),		oldbusvec
	movel	#dobuscheck,		a0@(0x08)
	movel	a0@(0x28),		oldalinevec
	movel	#fastalinecode,		a0@(0x28)
	movel	a0@(0x20),		oldprivvec
	movel	#newsrstuff,		a0@(0x20)
	rts

.globl _restore_vectors
_restore_vectors:
	movc	vbr,			a0
	movel	oldprivvec,		a0@(0x20)
	movel	oldalinevec,		a0@(0x28)
	movel	oldbusvec,		a0@(0x08)
	rts

fastalinecode:
	clrb	busbadness
	moveml	d0/d1/a0/a1,	sp@-		/* C saving precautions */
	movew	sr,		sp@-		/* so we can SPL7 later */
	link	a6,		#0		/* so we can back out easily */
	movel	oldalinevec,	troublevec	/* so we'll back out to the */
	moveq	#1,		d1		/* D1: Constant: #1 */
	movec	d1,		sfc		/* source: user space */

	lea	0x5C,		a0
	movesl	a0@,		d0
	cmpl	#0x41524449,	d0	/* check for 'ARDI' in 0x5C */
	bne	newnogood

	movesl	@(0x28),	a1


	movec	d1,		dfc
	oriw	#0x700,		sr
	movc	usp,		a0	/* A0: user stack pointer */
	movel	a6@(24),	d1	/* D1: OLD PC */
	movesl	d1,		a0@-
	movew	a6@(22),	d1	/* D1: OLD status */
	movesw	d1,		a0@-
	tstb	busbadness
	bne	newnogood
	movc	a0,		usp
	movel	a1,		a6@(24)
	bra	amf
	
dobuscheck:
	cmpl	#fastalinecode,	sp@(2)
	ble	notours
	cmpl	#amf,		sp@(2)
	bgt	notours

/*
 * Check to make sure that we were in Supervisor mode.
 */

	btst	#5,	sp@
	beq	notours

	tstb	_cpu_type
	beq	dobus_30

dobus_40:
	movew	d0,		sp@-
	movew	sp@(0xe),	d0	/* SSW */
	andiw	#0xDA3F,	d0	/* ignore CT, RW, X, SIZE1 */
	cmpiw	#0x0001,	d0	/* TM = 1 */
	bne	not_ours2
	movew	sp@+,		d0
	lea	newnogood,	a0	/* this will get our code to end */
	movel	a0,		sp@(2)	/* properly */
	rte

dobus_30:
	bclr	#0,	sp@(0xA)	/* Axe DF bit */
	st	busbadness
	rte				/* too bad */

not_ours2:
	movew	sp@+,		d0
notours:
	movel	oldbusvec,	sp@-	/* It's not ours; set up oldbusvec */
	rts				/* and jump */


/*
 * In order to verify that we are indeed and Executor
 * process, we check out location 0x5C and look for
 * 'ARDI', but page zero can get swapped out, and even
 * if we find what we want, we could take a page fault
 * when we try to put stuff on the stack.
 *
 * So, here's the trick: 
 *
 * If we don't get what we want, we just let
 * the normal UNIX trap handler get it's hands on things,
 * but instead of actually responding to the ILL or ALINE,
 * the UNIX signal handler just touches the relevent
 * pages and restarts the instruction which should work
 * the second time around.
 *
 * Because we're paranoid, we SPL7 before we start putting stuff
 * on the user's stack, since even though it's unlikely, there
 * could be some interrupt driven routine that also munges stuff
 * on the users stack pointer.
 */

/*
 * this location is used when we weren't able to process an ALINE or
 * an ILL and we need to go through the UNIX mechanism
 */

newnogood:
	unlk	a6
	movew	sp@+,		sr
	moveml	sp@+,		d0/d1/a0/a1
	movel	troublevec,	sp@-
	rts

newsrstuff:
	clrb	busbadness
	moveml	d0/d1/a0/a1,	sp@-		/* C saving precautions */
	movew	sr,		sp@-		/* so we can SPL7 later */
	link	a6,		#0		/* so we can back out easily */
	movel	oldprivvec,	troublevec	/* so we'll back out to the */
						/* right place */
	moveq	#1,		d1		/* D1: Constant: #1 */
	movec	d1,		sfc		/* source: user space */
	movec	d1,		dfc		/* dest: user space */

	movesl	@(0x5C),	d1

	cmpl	#0x41524449,	d1	/* check for 'ARDI' in 0x5C */
	bne	newnogood

	movel	_dma_chip,	d1		/* For turbo detection */
	movesl	d1,		@(0x54)		/* to be picked up later... */

	movel	a6@(24),	a0		/* pick up faulting longword */
	movesl	a0@,		d1

	tstb	busbadness
	bne	newnogood

/* don't know what it is, hand it off */
/* leaving the old pc buried under the old sr-as-a-longword */

	oriw	#0x700,		sr
	movc	usp,		a0
	movew	a6@(22),	d0
	movesl	d0,		a0@-
	movel	a6@(24),	d0
	movesl	d0,		a0@-
	lea	0x20,		a1
	movesl	a1@,		d0
	tstb	busbadness
	bne	newnogood
	movel	d0,		a6@(24)
	movc	a0,		usp
	bra	amf

/* This is our implementation of movc	d0, cacr */

amf:
	unlk	a6
	movew	sp@+,		sr
	moveml	sp@+,		d0/d1/a0/a1
	cmpl	#0x040a3058,	fpsp_done+6	/* old value of
						   "active_threads".  NOTE:
						   This test may not work
						   for 2.x Systems.  If you
						   have a 2.x System, you may
						   want to change the following
						   "beq" to a "bra" */
	beq	ardifpsp_done	/* == 3.0 system, use our patched fpsp_done */
	bra	fpsp_done	/* != 3.1 system, use "real" fpsp_done */

/*
 * The code below is based on the functionality seen in fpsp_done, with
 * a few efficiencies tossed in, some bogus code removed and a fix for
 * what I suspect to be a NeXT bug.
 */

ardifpsp_done:
	movel	a0,sp@-
	lea	_active_threads,a0
	movel	a0@,a0	  	  // only one CPU
	movel	a0@(0x28:w),a0
	movew	#0x2700,sr	  // No interrupts
	btst	#2,a0@(0x1a2:w)	  // Is TRACE_AST set?
	bne	needtrace  	  // YES: need trace (NOTE: This is
				  //      fixs an annoying bug)
	bclr	#3,a0@(0x1a2:w)	  // Is AST_SCHEDULE set?
	beq 	out	  	  // NO: don't need trace
	bset	#2,a0@(0x1a2:w)   // Set TRACE_AST
needtrace:
	btst	#6,sp@(0x4:w)	  // Is T0 already set?
	bne	1f		  // YES: Don't set T1
	bset	#7,sp@(0x4:w)	  // Set the T1 bit so we'll be thrown back
				  // Into supervisor as soon as we RTE
1:
	bclr	#0,a0@(0x1a2:w)	  // Don't wait until the end of the
				  // System call; we're not in a system
				  // call and aren't guaranteed to be soon
out:
	movel	sp@+,a0
	rte

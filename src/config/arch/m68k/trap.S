.text

.ascii "Copyright 1992, 1995 by Abacus Research and Development, Inc.\
  All rights reserved.\0"



.text
	.even


/*
 * we get to privdispatcher through ardimods.s.  When we get there
 * we've pushed the old pc and then a 2 byte shim and then the 2 byte
 * sr, so we add to that by
 */

.globl _privdispatcher
_privdispatcher:
	pea	sp@(8)			/* "real" sp */
	moveml	d0-d7/a0-a6,	sp@-	/* save all regs to play with */
	movel	sp,		sp@-	/* pointer to all that mess */
	jbsr	_dopriv			/* C routine to do dirty work */
	addql	#4,		sp	/* get rid of argument */

	movew	sp@(70),	d0	/* new "sr" */
	movel	sp@(64),	a0	/* ret pc */
	movel	sp@(60),	a1	/* "real" sp */
	movel	a0,		a1@-	/* stash the retpc where we want */
	movew	d0,		a1@-	/* stash the new sr */
	movel	a1,		sp@(60)	/* update "real" sp */

	moveml	sp@+,		d0-d7/a0-a6	/* restore possibly new regs */
	movel	sp@,		sp
	movew	sp@+,	ccr		/* reset the ccr bits as appropriate */
	rts

;; low_level_callback_handler MUST be the first thing in this file!
;; this file gets tacked on the end of some machine-generated
;; assembly by a Perl script.
;; This handler should ONLY be called from the special bsr callback table.
	
low_level_callback_handler:
	; Save some important stuff we're about to smash, before we smash it
	movew	ccr,a7@-
	movel	a1,a7@-
	movel	a0,a7@-
	movel	d0,a7@-

	movel	a7,a0
	movel	_last_executor_stack_ptr,a7

	movel	#_cpu_state,a1
	moveql	#8,d0
L0:	movel	a1@+,a7@-
	movel	a1@+,a7@-
	dbra	d0,L0

	; Save the current registers into cpu_state
	movel	#_cpu_state+60,a1
	moveml	d0-d7/a0-a6,a1@-
	
	movel	a0@,_cpu_state		; d0
	movel	a0@(4),_cpu_state+32	; a0
	movel	a0@(8),_cpu_state+36	; a1
	movew	a0@(12),ccr		; ccr
	addaw	#14,a0
	movel	a0,_cpu_state+60	; a7

	sne	_cpu_state+64
	smi	_cpu_state+65
	scs	_cpu_state+66
	svs	_cpu_state+67
	clrb	d0
	addxb	d0,d0
	moveb	d0,_cpu_state+68	

	; Fetch bsr return address, so we know which stub we're in,
	; and call the specified function with the specified void *.
	movel	_cpu_state+60,a0
	movel	a0@+,d0
	movel	a0@,d1
	movel	a0,_cpu_state+60
	subl	#_callback_stubs+4,d0
	lea	_callback_data,a0
	movel	a0@(d0:l:2),a7@-	; Push void * argument
	movel	d1,a7@-			; Push default return address
	movel	a0@(4,d0:l:2),a0
	jsr	a0@			; Call the specified handler
	addqw	#8,a7
	movel	_cpu_state+60,a0
	subqw	#6,a0
	movel	a0,_cpu_state+60
	movel	d0,a0@(2)		; Save returned rts address for later
	
	movel	a7,a1
	movel	a0,a7

	; compute new ccr and push it onto the other stack
	clrw	d0
	tstb	_cpu_state+64
	bne	1f
	bset	#2,d0
1:	tstb	_cpu_state+65
	beq	1f
	bset	#3,d0
1:	tstb	_cpu_state+66
	beq	1f
	bset	#0,d0
1:	tstb	_cpu_state+67
	beq	1f
	bset	#1,d0
1:	tstb	_cpu_state+68
	beq	1f
	bset	#4,d0		
1:	movew	d0,a0@		; push new ccr

	lea	_cpu_state+72,a0
	moveql	#8,d0

L1:	movel	a0@-,a7@-
	movel	a1@+,a0@
	movel	a0@-,a7@-
	movel	a1@+,a0@
	dbra	d0,L1

	moveml	a7@+,d0-a7	; Doesn't actually reload a7, it can't be set
	addql	#8,a7		; Skip cpu_state cc bit flags
	rtr

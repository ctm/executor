.text
	.align	1
.globl _m68k_call
_m68k_call:
	movel	a7@(4),d0
	moveml	d1-a6,a7@-
	movel	_last_executor_stack_ptr,a7@-
	movel	a7,_last_executor_stack_ptr
	movel	_cpu_state+60,a7
	movel	#L0,a7@-
	movel	d0,a7@-
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
1:	movew	d0,ccr
	moveml	_cpu_state,d0-a6
	rts
L0:	movel	a7,_cpu_state+60
	movel	_last_executor_stack_ptr,a7
	movel	a7@+,_last_executor_stack_ptr
	movel	a0,a7@-
	lea	_cpu_state+60,a0
	moveml	d0-a6,a0@-
	sne	_cpu_state+64
	smi	_cpu_state+65
	scs	_cpu_state+66
	svs	_cpu_state+67
	clrb	d0
	addxb	d0,d0
	moveb	d0,_cpu_state+68	
	movel	a7@+,_cpu_state+32
	moveml	a7@+,d1-a6
	rts

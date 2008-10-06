.text
	.align	1

	.globl	_aline_stub
_aline_stub:
	# Because of the way our NEXT kernel mods work, we only have 6
	# bytes instead of the normal 8 byte RTE frame so we create
	# one by moving the stack down two bytes and sticking in the
	# missing bytes
	movel	_cpu_state+60,a0
	subqw	#2,a0
	movel	a0,_cpu_state+60
	movew	a0@(2),a0@
	movel	a0@(4),d0
	movel	d0,a0@(2)
	movew	#40,a0@(6)
	movel	d0,a7@(4)
	jmp	_alinehandler

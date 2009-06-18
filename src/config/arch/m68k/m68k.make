HOST_ARCH_OBJ = m68k-call-emulator.o m68k-callback-stubs.o m68k-callback.o\
	m68k-stack.o m68k-trap-handler.o m68k.o m68k-destroy.o trap.o

MAX_CALLBACKS=4352	# 4096 plus extra slop

m68k-callback-stubs.s:	make_callback_stubs.pl m68k-callback-handler.s
	$(HOST_ARCH_DIR)/make_callback_stubs.pl $(MAX_CALLBACKS)\
		$(HOST_ARCH_DIR)/m68k-callback-handler.s\
		./m68k-callback-stubs.s

m68k-callback.o:	m68k-callback.c
	$(BUILD_GCC) $(BUILD_CFLAGS) -DNUM_CALLBACK_SLOTS=$(MAX_CALLBACKS)\
		$(HOST_ARCH_DIR)/m68k-callback.c -c -o m68k-callback.o

clean::

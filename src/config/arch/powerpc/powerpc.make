HOST_ARCH_SRC = powerpc.c ppc_call.c ppc_stubs.c

HOST_ARCH_OBJ = $(strip $(notdir $(addsuffix .o,$(basename $(HOST_ARCH_SRC)))))

# _GNU_SOURCE needed for some simple math #defines
HOST_CFLAGS += -D_GNU_SOURCE

# NOTE: using -mcall-aix doesn't help here; it hurts
# ppc_call.o:	ppc_call.c
#	$(HOST_GCC) $(HOST_CFLAGS) -mcall-aix -c $<


# -mcall-aix was used when we were compiling on Linux and experimenting with
# running powerpc code using a native powerpc processor.  The ability to run
# native powerpc code is not likely to work (initially) under the Mac OS X
# port.

ifeq (,$(findstring macosx,$(HOST)))
  CALL_AIX_FLAG = -mcall-aix
endif

ppc_stubs.o:	ppc_stubs.c ppc_stubs.h
	$(HOST_GCC) $(HOST_CFLAGS) $(CALL_AIX_FLAG) -c $<

clean::
	rm -f $(HOST_ARCH_OBJ)

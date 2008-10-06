TARGET_ARCH_SRC = powerpc.c ppc_call.c ppc_stubs.c

TARGET_ARCH_OBJ = $(strip $(notdir $(addsuffix .o,$(basename $(TARGET_ARCH_SRC)))))

# _GNU_SOURCE needed for some simple math #defines
TARGET_CFLAGS += -D_GNU_SOURCE

# NOTE: using -mcall-aix doesn't help here; it hurts
# ppc_call.o:	ppc_call.c
#	$(TARGET_GCC) $(TARGET_CFLAGS) -mcall-aix -c $<

ppc_stubs.o:	ppc_stubs.c ppc_stubs.h
	$(TARGET_GCC) $(TARGET_CFLAGS) -mcall-aix -c $<

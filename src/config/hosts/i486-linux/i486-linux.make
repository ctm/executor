LOWGLOBALS_LD_OPTION = lowglobals.o

HOST_SRC = lowglobals-mem.c
HOST_OBJ = lowglobals-mem.o

HOST_LIBS = -ldbm -lm

clean::
	rm -f $(HOST_OBJ)

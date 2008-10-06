LOWGLOBALS_LD_OPTION = lowglobals.o

TARGET_SRC = lowglobals-mem.c
TARGET_OBJ = lowglobals-mem.o

TARGET_LIBS = -ldbm -lm

clean::
	rm -f $(TARGET_OBJ)

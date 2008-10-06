LOWGLOBALS_LD_OPTION = lowglobals.o

TARGET_OS_LD_FLAGS = -Wl,-no-keep-memory	# So we don't page so much

TARGET_OS_SRC = lowglobals-mem.c linux.c linux_except.c
TARGET_OS_OBJ = lowglobals-mem.o linux.o linux_except.o

TARGET_OS_LIBS = @libdb@ -lm @libg@

SDL_LIB_DIR=/usr/lib

licensetext.txt: licensetext.i extr.c
	$(HOST_GCC) -DLINUX -o extr extr.c licensetext.i
	./extr > licensetext.txt

# see the extended comment in linux.c as to why we REQUIRE -O6 below
# if you change this, you may introduce a very subtle bug.

linux.o: linux.c
	$(TARGET_GCC) $(TARGET_CFLAGS) -O6 -c -o $*.o $<

clean::
	rm -f $(TARGET_OS_OBJ)

LOWGLOBALS_LD_OPTION = lowglobals.o

HOST_OS_LD_FLAGS = -Wl,-no-keep-memory	# So we don't page so much

HOST_OS_SRC = lowglobals-mem.c linux.c
HOST_OS_OBJ = lowglobals-mem.o linux.o

HOST_OS_LIBS = @libdb@ -lm @libg@

SDL_LIB_DIR=/usr/lib

licensetext.txt: licensetext.i extr.c
	$(BUILD_GCC) -DLINUX -o extr extr.c licensetext.i
	./extr > licensetext.txt

# see the extended comment in linux.c as to why we REQUIRE -O6 below
# if you change this, you may introduce a very subtle bug.

linux.o: linux.c
	$(HOST_GCC) $(HOST_CFLAGS) -O6 -c -o $*.o $<

clean::
	rm -f $(HOST_OS_OBJ)

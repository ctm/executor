LOWGLOBALS_LD_OPTION = -Wl,lowglobals.o

HOST_OS_LD_FLAGS = -Wl,-Ttext,2000 -Wl,-no-keep-memory

HOST_OS_SRC = msdos.c dpmimem.c dpmicall.c openmany.c rmint70.S
HOST_OS_OBJ = $(strip $(addsuffix .o,$(basename $(HOST_OS_SRC))))

HOST_OS_LIBS =

rmint70.o: rmint70.S
	$(HOST_GCC) $(HOST_CFLAGS) -c $(HOST_OS_DIR)/rmint70.S

HOST_OS_POST_LD_OPTIONS = cp executor executor.bkup && $(HOST_STRIP) executor && stubify executor && mv executor.bkup executor

licensetext.txt: licensetext.i extr.c
	$(BUILD_GCC) -DMSDOS -o extr extr.c licensetext.i
	./extr > licensetext.txt

clean::
	rm -f $(HOST_OS_OBJ) executor.exe

LOWGLOBALS_LD_OPTION = -Wl,lowglobals.o

TARGET_OS_LD_FLAGS = -Wl,-Ttext,2000 -Wl,-no-keep-memory

TARGET_OS_SRC = msdos.c dpmimem.c dpmicall.c openmany.c rmint70.S
TARGET_OS_OBJ = $(strip $(addsuffix .o,$(basename $(TARGET_OS_SRC))))

TARGET_OS_LIBS =

rmint70.o: rmint70.S
	$(TARGET_GCC) $(TARGET_CFLAGS) -c $(TARGET_OS_DIR)/rmint70.S

TARGET_OS_POST_LD_OPTIONS = cp executor executor.bkup && $(TARGET_STRIP) executor && stubify executor && mv executor.bkup executor

licensetext.txt: licensetext.i extr.c
	$(HOST_GCC) -DMSDOS -o extr extr.c licensetext.i
	./extr > licensetext.txt

clean::
	rm -f $(TARGET_OS_OBJ) executor.exe

TARGET_OS_SRC = cygwin32.c winfs.c win_disk.c win_stat.c \
	win_memory.c win_serial.c win_ntcd.c win_print.c win_beep.c \
	win_cookie.c win_clip.c win_temp.c win_except.c win_time.c \
	win_dongle.c win_queue.c win_screen.c win_vxdiface.c win_keyboard.c \
	win_launch.c win_stdfile.c
TARGET_OS_OBJ = $(strip $(addsuffix .o,$(basename $(TARGET_OS_SRC))))

TARGET_OS_LIBS = -mwindows -luser32 -lgdi32 -lwinmm -ldxguid

licensetext.txt: licensetext.i extr.c
	$(HOST_GCC) -DCYGWIN32 -o extr extr.c licensetext.i
	./extr > licensetext.txt

exemove.exe:	exemove.o paramline.o
	$(TARGET_GCC) -o $@ $^
	$(TARGET_STRIP) $@

executor.exe:	executor
	cp executor executor.exe
	$(TARGET_STRIP) executor.exe

clean::
	rm -f $(TARGET_OS_OBJ)

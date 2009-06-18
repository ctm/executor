HOST_OS_SRC = cygwin32.c winfs.c win_disk.c win_stat.c \
	win_memory.c win_serial.c win_ntcd.c win_print.c win_beep.c \
	win_cookie.c win_clip.c win_temp.c win_except.c win_time.c \
	win_dongle.c win_queue.c win_screen.c win_vxdiface.c win_keyboard.c \
	win_launch.c win_stdfile.c
HOST_OS_OBJ = $(strip $(addsuffix .o,$(basename $(HOST_OS_SRC))))

HOST_OS_LIBS = -mwindows -luser32 -lgdi32 -lwinmm -ldxguid

licensetext.txt: licensetext.i extr.c
	$(BUILD_GCC) -DCYGWIN32 -o extr extr.c licensetext.i
	./extr > licensetext.txt

exemove.exe:	exemove.o paramline.o
	$(HOST_GCC) -o $@ $^
	$(HOST_STRIP) $@

executor.exe:	executor
	cp executor executor.exe
	$(HOST_STRIP) executor.exe

clean::
	rm -f $(HOST_OS_OBJ)

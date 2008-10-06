CC=i486-pc-cygwin32-gcc -windows
CFLAGS=-O -Wall
OBJS=win_print.o print_main.o

/home/samba/print.exe:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

print.o:	win_print.h win_print_private.h
print_main.o:	win_print.h

clean:
	-rm $(OBJS) /home/samba/print.exe

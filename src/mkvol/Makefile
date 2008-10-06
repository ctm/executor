# CFLAGS=-g -Wall -arch m68k -arch i386
CFLAGS=-g -Wall -O

all:	makehfv

mkvol.o:	mkvol.c mkvol.h ../include/rsys/parsenum.h
	$(CC) -DMKVOL_PROGRAM $(CFLAGS) -c -o mkvol.o mkvol.c

makehfv:	mkvol.o parsenum.o
	$(CC) $(LDFLAGS) -o makehfv mkvol.o parsenum.o

clean:
	rm -f makehfv mkvol.o

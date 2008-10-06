CC      =i386-msdos-go32-gcc
CFLAGS  =-O2 -g -Wall
SB_OBJ  =sb_dma.o sbdetect.o sbdriver.o
RM      =rm -f

mattest.exe: libsb.a mattest.o
	$(CC) mattest.o -o mattest -L. -lsb
	stubify mattest

sb_test.exe: libsb.a sb_test.o
	$(CC) sb_test.o -o sb_test.cof -L. -lsb
	stubify sb_test.cof

libsb.a: $(SB_OBJ)
	ar cq libsb.a $(SB_OBJ)
	ranlib libsb.a

clean:
	$(RM) *.o libsb.a sb_test.cof sb_test.exe mattest.exe mattest mattest.o

sb_dma.o        : sb_dma.c sbdetect.h
sbdetect.o      : sbdetect.c sb_defs.h
sbdriver.o      : sbdriver.c sbdetect.h sb_dma.h sb_defs.h
sb_test.o       : sb_test.c sbdriver.h sb_defs.h

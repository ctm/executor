CFLAGS=-g -O -Wall
CPPFLAGS=-I../../src/include

custom:	parse.o
	$(CC) $(LDFLAGS) -o custom parse.o

install:	custom
	install custom /usr/local/bin

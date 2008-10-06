CFLAGS=-g -Wall -ansi -pedantic -O

test:	test.o error.o
	$(CC) -o test test.o error.o

test.o error.o:	error.h MacTypes.h Makefile

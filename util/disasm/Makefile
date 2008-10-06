OBJS = main.o disasm.o codeseg.o label.o hash.o
CC = gcc
CFLAGS = -g -O2 -Wall

disasm:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o disasm

clean:
	rm -f $(OBJS) *~ disasm

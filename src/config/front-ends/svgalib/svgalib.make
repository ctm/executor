
FRONT_END_DEFINES =

SVGALIB_SRC = svgalib.c svgalibevent.c NEXT.c vgavdriver.c

FRONT_END_SRC = $(SVGALIB_SRC)
FRONT_END_OBJ = $(FRONT_END_SRC:.c=.o)

FRONT_END_LIBS = -lvga

clean::
	rm -f $(FRONT_END_OBJ)

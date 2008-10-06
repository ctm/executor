
FRONT_END_DEFINES =

FRONT_END_LIBS = -L$(FRONT_END_DIR) -lsv

METAASM_ARGS = -define 'DST_SEG=%es:'

DOS_SRC = \
  dosclip.c dosdisk.c dosevents.c vga.c vgavdriver.c aspi.c\
  dosevq.c dpmilock.c deintr.S dosmem.c dosserial.c

FRONT_END_SRC = $(DOS_SRC)
FRONT_END_OBJ = $(strip $(addsuffix .o,$(basename $(FRONT_END_SRC))))

deintr.o: deintr.S dosevq_defs.h
	$(TARGET_GCC) $(TARGET_CFLAGS) -c $(FRONT_END_DIR)/deintr.S

clean::
	rm -f $(FRONT_END_OBJ)

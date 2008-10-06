FRONT_END_DEFINES =

FRONT_END_LIBS =

WIN_SRC = winevents.c wincursor.c windriver.c

FRONT_END_SRC = $(WIN_SRC)
FRONT_END_OBJ = $(strip $(addsuffix .o,$(basename $(FRONT_END_SRC))))

clean::
	rm -f $(FRONT_END_OBJ)

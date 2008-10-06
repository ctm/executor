INCLUDES += -I$(SOUND_DIR)/sb_lib

SOUND_LIBS = -L$(SOUND_DIR)/sb_lib -lsb

SOUND_SRC = djgpp-sound.c

SOUND_OBJ = $(SOUND_SRC:.c=.o)

clean::
	rm -f $(SOUND_OBJ)

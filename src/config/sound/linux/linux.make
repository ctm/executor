SOUND_SRC = linux-sound.c

SOUND_OBJ = $(SOUND_SRC:.c=.o)

clean::
	rm -f $(SOUND_OBJ)

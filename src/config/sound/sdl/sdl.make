
SOUND_LIBS = # This should be used with the SDL front end

SOUND_SRC = sdl-sound.c

SOUND_OBJ = $(SOUND_SRC:.c=.o)

clean::
	rm -f $(SOUND_OBJ)

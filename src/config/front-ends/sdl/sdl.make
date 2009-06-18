INCLUDES += -I/usr/include/SDL

SDL_SRC = sdlwin.c sdlevents.c sdlwm.c sdlscrap.c sdlquit.c \
          syswm_map.c sdl_mem.c SDL_bmp.c

ifneq (,$(findstring linux,$(HOST)))
  SDL_SRC += sdlX.c
endif

FRONT_END_SRC = $(SDL_SRC)
FRONT_END_OBJ = $(FRONT_END_SRC:.c=.o) 

ifneq (,$(findstring macosx,$(HOST)))
  FRONT_END_OBJ += macosx_main.o
endif


ifneq (,$(findstring mingw,$(HOST)))
  FRONT_END_LIBS += -lmingw32
endif
# SDL_LIB_DIR is defined in the OS-specific makefile
ifneq (,$(SDL_LIB_DIR))
  FRONT_END_LIBS += -L$(SDL_LIB_DIR)
endif

# I'm not sure when the need for -lSDLmain went away, but it's not needed
# with the version of SDL on Fedora 9 (SDL 1.2.13)

# FRONT_END_LIBS += -lSDLmain -lSDL
ifeq (,$(findstring macosx,$(HOST)))
  FRONT_END_LIBS += -lSDL
endif
#
ifneq (,$(findstring linux,$(HOST)))
  FRONT_END_LIBS += -ldl -L/usr/X11R6/lib -lX11 -lpthread
  INCLUDES += -I/usr/X11R6/include
  CFLAGS += -D_REENTRANT
endif

ifneq (,$(findstring macosx,$(HOST)))
  FRONT_END_LIBS += -framework SDL -framework Cocoa
  CFLAGS += -D_REENTRANT
endif

clean::
	rm -f $(FRONT_END_OBJ)

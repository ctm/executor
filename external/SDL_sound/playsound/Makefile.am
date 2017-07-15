bin_PROGRAMS = playsound playsound_simple

INCLUDES = -I$(top_srcdir)

if USE_PHYSICSFS
PHYSFS_CFLG = -DSUPPORT_PHYSFS=1
PHYSFS_LIBS = -lphysfs
else
PHYSFS_CFLG =
PHYSFS_SRCS =
PHYSFS_LIBS =
endif

playsound_CFLAGS = $(PHYSFS_CFLG)
playsound_LDADD = ../libSDL_sound.la $(PHYSFS_LIBS)
playsound_SOURCES = playsound.c physfsrwops.c physfsrwops.h

playsound_simple_LDADD = ../libSDL_sound.la
playsound_simple_SOURCES = playsound_simple.c


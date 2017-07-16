lib_LTLIBRARIES = libSDL_sound.la

SUBDIRS = decoders . playsound

libSDL_soundincludedir = $(includedir)/SDL
libSDL_soundinclude_HEADERS =	\
	SDL_sound.h

libSDL_sound_la_SOURCES =	\
	SDL_sound.c	\
	SDL_sound_internal.h	\
    audio_convert.c   \
	extra_rwops.c	\
	extra_rwops.h

if USE_TIMIDITY
TIMIDITY_LIB = decoders/timidity/libtimidity.la
else
TIMIDITY_LIB =
endif

if USE_LIBMPG123
MPG123_LIB = decoders/libmpg123/libmpg123.la
else
MPG123_LIB =
endif

libSDL_sound_la_LDFLAGS = 		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
libSDL_sound_la_LIBADD =	\
	decoders/libdecoders.la	\
	$(TIMIDITY_LIB) $(MPG123_LIB)

EXTRA_DIST =	\
	CREDITS \
	COPYING \
	CHANGELOG \
	CWProject.sit \
	PBProjects.tar.gz \
	borland.zip \
	Doxyfile \
    VisualC

dist-hook:
	mkdir $(distdir)/docs
	echo "Docs are generated with the program "Doxygen" (http://www.doxygen.org/)," >> $(distdir)/docs/README
	echo " or can be read online at http://icculus.org/SDL_sound/docs/" >> $(distdir)/docs/README
	echo >> $(distdir)/docs/README
	rm -rf `find $(distdir) -type d -name ".svn"`


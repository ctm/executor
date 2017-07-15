if USE_LIBMPG123
noinst_LTLIBRARIES = libmpg123.la
endif

# !!! FIXME: get MMX/SSE/Altivec/etc stuff in here.

INCLUDES = -I$(top_srcdir)/decodes/libmpg123
libmpg123_la_CFLAGS = -DOPT_GENERIC -DREAL_IS_FLOAT -DMPG123_NO_CONFIGURE

libmpg123_la_SOURCES = \
	compat.c \
	compat.h \
	parse.c \
	parse.h \
	frame.c \
	format.c \
	frame.h \
	reader.h \
	debug.h \
	decode.h \
	decode_2to1.c \
	decode_4to1.c \
	decode_ntom.c \
	equalizer.c \
	huffman.h \
	icy.c \
	icy.h \
	icy2utf8.c \
	icy2utf8.h \
	id3.c \
	id3.h \
	true.h \
	l2tables.h \
	layer1.c \
	layer2.c \
	layer3.c \
	getbits.h \
	optimize.h \
	optimize.c \
	readers.c \
	tabinit.c \
	stringbuf.c \
	libmpg123.c \
	mpg123lib_intern.h \
	mangle.h \
	getcpuflags.h \
	index.h \
	index.c \
	mpg123.h \
	config.h \
	dct64.c \
	decode.c \
	libmpg123.sym

EXTRA_libmpg123_la_SOURCES = \
	dct36_3dnowext.S \
	dct36_3dnow.S \
	dct64_3dnowext.S \
	dct64_3dnow.S \
	dct64_altivec.c \
	dct64_i386.c \
	dct64_i486.c \
	dct64_mmx.S \
	dct64_sse.S \
	decode_3dnowext.S \
	decode_3dnow.S \
	decode_altivec.c \
	decode_i386.c \
	decode_i486.c \
	decode_i586_dither.S \
	decode_i586.S \
	decode_mmx.S \
	decode_sse3d.h \
	decode_sse.S \
	equalizer_3dnow.S \
	tabinit_mmx.S \
	getcpuflags.S

EXTRA_DIST = mpg123.h.in dnoise.sh dnoise.dat testcpu.c README-sdlsound.txt


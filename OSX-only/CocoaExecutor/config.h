//
//  config.h
//  CocoaExecutor
//
//  Created by C.W. Betts on 7/27/14.
//  Copyright (c) 2014 C.W. Betts. All rights reserved.
//

/* Enable the macros that store Macintosh addresses in a union of a 32-bit
 unsigned int and an array of zero pointers; this currently doesn't work,
 but eventually it is how we'll support 64-bit architectures */
//#define FORCE_EXPERIMENTAL_PACKED_MACROS 1

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `db' library (-ldb). */
/* #undef HAVE_LIBDB */

/* Define to 1 if you have the `dxguid' library (-ldxguid). */
/* #undef HAVE_LIBDXGUID */

/* Define to 1 if you have the `gdi32' library (-lgdi32). */
/* #undef HAVE_LIBGDI32 */

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `pthread' library (-lpthread). */
#define HAVE_LIBPTHREAD 1

/* Define to 1 if you have the `SDL' library (-lSDL). */
/* #undef HAVE_LIBSDL */

#define USE_BSD_SIGNALS 1

/* Define to 1 if you have the `syn68k' library (-lsyn68k). */
#define HAVE_LIBSYN68K 1

/* Define to 1 if you have the `user32' library (-luser32). */
/* #undef HAVE_LIBUSER32 */

/* Define to 1 if you have the `vga' library (-lvga). */
/* #undef HAVE_LIBVGA */

/* Define to 1 if you have the `winmm' library (-lwinmm). */
/* #undef HAVE_LIBWINMM */

/* Define to 1 if you have the `X11' library (-lX11). */
/* #undef HAVE_LIBX11 */

/* Define to 1 if you have the `Xext' library (-lXext). */
/* #undef HAVE_LIBXEXT */

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "executorosx"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "maddthesane@gmail.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "NeoClassic"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "executor 2.2.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "executor"

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.2.0"

/* The size of `char *', as computed by sizeof. */
#define SIZEOF_CHAR_P 4

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

/* Version number of package */
#define VERSION "2.2.0"

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

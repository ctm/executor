AC_INIT(executor, 2.1.17, ctm@ardi.com)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall])

AC_CONFIG_SRCDIR([executor.c])
AC_CONFIG_HEADERS([config.h])

# Normalize to our old names

case ${build_cpu} in
  i[[3456]]86)
    build_cpu=i386
  ;;
esac

case ${host_cpu} in
  i[[3456]]86)
    host_cpu=i386
  ;;
esac

case ${build_os} in
  darwin*)
    build_os=macosx
  ;;
  linux*)
    build_os=linux
  ;;
esac

case ${host_os} in
  darwin*)
    host_os=macosx
  ;;
  linux*)
    host_os=linux
  ;;
esac

# Checks for programs.
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
AM_PROG_AS
AC_PROG_YACC
AC_PROG_OBJC

AC_CHECK_PROG([PERL], [perl], [perl])
AC_ARG_VAR([PERL], [perl for generating blitters])

AC_GNU_SOURCE

AC_SYS_LARGEFILE

# Check for libraries

if test x$host_cpu != xm68k; then
  AC_CHECK_LIB(syn68k, initialize_68k_emulator,,[AC_MSG_ERROR([Can't build Executor for $host_cpu without Syn68k])])
fi

AC_CHECK_LIB(SDL, SDL_Init)
AC_CHECK_LIB(X11, XInitThreads)

AC_ARG_WITH(front-end,
            [AS_HELP_STRING([--with-front-end=arg],
                            [arg must be sdl or x (default is sdl)])],
            [front_end="$withval"],
            [if test x"$ac_cv_lib_SDL_SDL_Init" = xyes; then
               front_end=sdl
             elif test x"$ac_cv_lib_X11_XInitThreads" = xyes; then
               front_end=x
             elif test x$host_os = xmacosx; then
               front_end=sdl
             else
               AC_MSG_ERROR([Must use --with-front-end=sdl or --with-front-end=x])
             fi])

if test x$front_end = xsvgalib; then
  AC_CHECK_LIB(vga, vga_setmode,,[AC_MSG_ERROR([Can't build Executor with the SVGAlib front-end without svgalib])])
fi

if test x$front_end = xx; then
  AC_CHECK_LIB(Xext, XShmPutImage,,[AC_MSG_ERROR([Can't build Executor with the X front-end without libXext])])
fi

AC_ARG_WITH(sound,
            [AS_HELP_STRING([--with-sound=arg],
                            [arg must be linux or sdl (default is sdl)])],
            [sound_config="$withval"],
            [if test x"$ac_cv_lib_SDL_SDL_Init" = xyes; then
               sound_config=sdl
             elif test x"$host_os" = xxlinux; then
               sound_config=linux
             elif test x$host_os = xmacosx; then
               sound_config=sdl
             else
               AC_MSG_ERROR([Must use --with-sound=sdl or --with-sound=x])
             fi])

AC_DEFINE([FORCE_EXPERIMENTAL_PACKED_MACROS], [0], [Enable the macros that store Macintosh addresses in a union of a 32-bit unsigned int and an array of zero pointers; this currently doesn't work, but eventually it is how we'll support 64-bit architectures])

AC_ARG_WITH(incomplete-packed-macros,
            [AS_HELP_STRING([--with-incomplete-packed-macros],
                            [use macros that will eventually allow 64-bit ports of Executor (but will not work yet)])],
        [if test x"$withval" = xyes; then AC_DEFINE([FORCE_EXPERIMENTAL_PACKED_MACROS], [1])
fi])

if test x$host_os = xcygwin32; then
  # None of this is tested yet
  AC_CHECK_LIB(dxguid, GUID_Valid_Start_Time)
  AC_CHECK_LIB(winmm, OpenDriver)
  AC_CHECK_LIB(gdi32, CreateDIBitmap)
  AC_CHECK_LIB(user32, OpenClipboard)
fi

if test x"$sound_config" = xsdl; then
  AC_CHECK_LIB(pthread, pthread_create)
fi

AC_CHECK_LIB(db, __db_ndbm_open)
AC_CHECK_LIB(m, pow)


AC_SUBST(host_cpu)
AC_SUBST(host_os)
AC_SUBST(front_end)
AC_SUBST(sound_config)

AC_CONFIG_LINKS([build-arch-config.h:config/arch/$build_cpu/$build_cpu.h
                 build-os-config.h:config/os/$build_os/$build_os.h
                 host-arch-config.h:config/arch/$host_cpu/$host_cpu.h
                 host-os-config.h:config/os/$host_os/$host_os.h
                 front-end-config.h:config/front-ends/$front_end/$front_end.h
                 sound-config.h:config/sound/$sound_config/$sound_config-sound.h])

# Many of the config options here have not actually been made to work since
# we converted to the modern GNU build system.

AM_CONDITIONAL([CONFIG_ARCH_ALPHA], [test x$host_cpu = xalpha])
AM_CONDITIONAL([CONFIG_ARCH_I386], [test x$host_cpu = xi386])
AM_CONDITIONAL([CONFIG_ARCH_M68K], [test x$host_cpu = xm68k])
AM_CONDITIONAL([CONFIG_ARCH_POWERPC], [test x$host_cpu = xpowerpc])
AM_CONDITIONAL([CONFIG_ARCH_X86_64], [test x$host_cpu = xx86_64])

AM_CONDITIONAL([CONFIG_FRONT_END_DOS], [test x$front_end = xdos])
AM_CONDITIONAL([CONFIG_FRONT_END_NEXTSTEP], [test x$front_end = xnextstep])
AM_CONDITIONAL([CONFIG_FRONT_END_SDL], [test x$front_end = xsdl])
AM_CONDITIONAL([CONFIG_FRONT_END_SVGALIB], [test x$front_end = xsvgalib])
AM_CONDITIONAL([CONFIG_FRONT_END_WIN32], [test x$front_end = xwin32])
AM_CONDITIONAL([CONFIG_FRONT_END_X], [test x$front_end = xx])

AM_CONDITIONAL([CONFIG_HOST_ALPHA_DEC_OSF], [false])
AM_CONDITIONAL([CONFIG_HOST_I386_MSDOS_GO32], [test x$host_os = xmsdos])
AM_CONDITIONAL([CONFIG_HOST_I486_LINUX], [false])
AM_CONDITIONAL([CONFIG_HOST_I486_NEXT_NS3], [test x$host_os = xnext -a x$host_cpu = xi386])
AM_CONDITIONAL([CONFIG_HOST_M68K_NEXT_NS3], [test x$host_os = xnext -a x$host_cpu = xm68k])

AM_CONDITIONAL([CONFIG_OS_CYGWIN32], [test x$host_os = xcygwin32])
AM_CONDITIONAL([CONFIG_OS_LINUX], [test x$host_os = xlinux])
AM_CONDITIONAL([CONFIG_OS_MACOSX], [test x$host_os = xmacosx])
AM_CONDITIONAL([CONFIG_OS_MSDOS], [test x$host_os = xmsdos])
AM_CONDITIONAL([CONFIG_OS_NEXT], [test x$host_os = xnext])

AM_CONDITIONAL([CONFIG_SOUND_DJGPP], [test x$sound_config = xdjgpp])
AM_CONDITIONAL([CONFIG_SOUND_LINUX], [test x$sound_config = xlinux])
AM_CONDITIONAL([CONFIG_SOUND_SDL], [test x$sound_config = xsdl])

AC_CHECK_SIZEOF([char *])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

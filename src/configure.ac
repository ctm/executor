AC_INIT(executor, 2.1pr16, ctm@ardi.com)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall])

AM_PROG_AS

AC_CONFIG_SRCDIR([executor.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB
AM_PROG_AS
AC_PROG_YACC

AC_CHECK_PROG([PERL], [perl], [perl])
AC_ARG_VAR([PERL], [perl for generating blitters])

AC_GNU_SOURCE

AC_SYS_LARGEFILE

AC_CHECK_LIB(syn68k, initialize_68k_emulator)

# Right now we blindly look for the libraries we need for the SDL Linux
# port since that's the first one I'll get going with the this file.

AC_CHECK_LIB(SDL, SDL_Init)
AC_CHECK_LIB(db, __db_ndbm_open)
AC_CHECK_LIB(X11, XInitThreads)
AC_CHECK_LIB(m, pow)

AC_CONFIG_FILES([Makefile])


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

# This is a temporary cheat that forces us to use SDL.

front_end=sdl
sound_config=sdl

# end of cheat

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

AC_OUTPUT

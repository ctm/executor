AC_INIT(executor, 2.1pr16, ctm@ardi.com)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall])

AM_PROG_AS

AC_CONFIG_SRCDIR([executor.c])
AC_CONFIG_HEADERS([config.h])

AC_GNU_SOURCE

AC_SYS_LARGEFILE

AC_CONFIG_FILES([Makefile])

AC_CONFIG_LINKS([target-os-config.h:config/os/$target_os/$target_os.h
                target-arch-config.h:config/arch/$target_cpu/$target_cpu.h])

AC_OUTPUT

#
# libinputmapper - build configuration script
# Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
#

AC_PREREQ(2.68)

AC_INIT([libinputmapper],
        [1],
        [https://github.com/dvdhrm/libinputmapper],
        [libinputmapper],
        [http://dvdhrm.github.io/libinputmapper])
AC_CONFIG_SRCDIR([src/inmap_ctx.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER(config.h)
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign 1.11 subdir-objects dist-xz no-dist-gzip tar-pax -Wall -Werror -Wno-portability])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_SED
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_GREP
AC_PROG_AWK

LT_PREREQ(2.2)
LT_INIT

#
# Dependencies
#

PKG_CHECK_MODULES([UDEV], [libudev >= 172],
                  [have_udev=yes], [have_udev=no])
AC_SUBST(UDEV_CFLAGS)
AC_SUBST(UDEV_LIBS)
if test ! "x$have_udev" = "xyes" ; then
        AC_ERROR([libudev >= 172 is required for libinputmapper])
fi

# TODO: check for linux/input.h

#
# Makefile vars
# After everything is configured, we create all makefiles.
#

AC_CONFIG_FILES([Makefile
                 docs/libinputmapper.pc])
AC_OUTPUT

#
# Configuration output
# Show configuration to the user so they can check whether everything was
# configured as expected.
#

AC_MSG_NOTICE([Build configuration:

               prefix: $prefix
          exec-prefix: $exec_prefix
               libdir: $libdir
           includedir: $includedir

        Run "${MAKE-make}" to start compilation process])
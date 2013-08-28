#
# libinputmapper - Global Makefile
# Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
#

#
# Library Version Numbers
#

LIBINPUTMAPPER_CURRENT = 1
LIBINPUTMAPPER_REVISION = 0
LIBINPUTMAPPER_AGE = 0

#
# Global Configurations and Initializations
#

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests
AM_DISTCHECK_CONFIGURE_FLAGS =

.DELETE_ON_ERROR:

include_HEADERS =
EXTRA_DIST = \
	README \
	COPYING \
	NEWS \
	src/libinputmapper.pc.in \
	src/libinputmapper.sym
CLEANFILES =
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =
TPHONY =

check_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =

#
# Default CFlags
# Make all files include "config.h" by default. This shouldn't cause any
# problems and we cannot forget to include it anymore.
#
# Also make the linker discard all unused symbols.
#

AM_CFLAGS = \
	-Wall \
	-pipe \
	-fno-common \
	-ffast-math \
	-fdiagnostics-show-option \
	-fno-strict-aliasing \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections \
	-fstack-protector
AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-I $(srcdir)/src
AM_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--gc-sections \
	-Wl,-z,relro \
	-Wl,-z,now

#
# libinputmapper
# Main library build instructions
#

lib_LTLIBRARIES += libinputmapper.la
include_HEADERS += src/libinputmapper.h
pkgconfig_DATA += src/libinputmapper.pc

libinputmapper_la_SOURCES = \
	src/libinputmapper.h \
	src/libinputmapper_internal.h \
	src/inmap_ctx.c \
	src/inmap_mapping.c

libinputmapper_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(UDEV_CFLAGS) \
	$(LIBUDEV_CFLAGS)
libinputmapper_la_LIBADD = $(UDEV_LIBS) $(LIBUDEV_LIBS)
EXTRA_libinputmapper_la_DEPENDENCIES = $(top_srcdir)/src/libinputmapper.sym
libinputmapper_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBINPUTMAPPER_CURRENT):$(LIBINPUTMAPPER_REVISION):$(LIBINPUTMAPPER_AGE) \
	-Wl,--version-script="$(top_srcdir)/src/libinputmapper.sym"

#
# documentation
#

if HAVE_DOXYGEN

all-local: doxygen

doxygen: docs/libinputmapper.doxygen
	doxygen $<

clean-local:
	rm -rf docs/html

TPHONY += doxygen

doc_src = $(shell find docs/html -name "*" -printf "docs/html/%P\n" 2>/dev/null)
EXTRA_DIST += $(doc_src) docs/libinputmapper.doxygen
dist_noinst_DATA = docs/libinputmapper.doxygen.in

endif

#
# Phony targets
#

.PHONY: $(TPHONY)

#
# Empty .SECONDARY target causes alle intermediate files to be treated as
# secondary files. That is, they don't get deleted after make finished.
#

.SECONDARY:

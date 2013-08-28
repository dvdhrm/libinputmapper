#!/bin/sh
set -e
mkdir -p m4
autoreconf -is

if test -z "$NOCONFIGURE" ; then
    exec ./configure "$@"
fi

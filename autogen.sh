#!/bin/sh
set -e
mkdir -p m4
autoreconf -is --force

if test -z "$NOCONFIGURE" ; then
    exec ./configure "$@"
fi

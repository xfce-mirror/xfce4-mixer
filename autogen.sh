#!/bin/sh

export WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE="1.7"

aclocal -I m4 || exit 1
automake --add-missing || exit 1
autoconf || exit 1
./configure $*
exit "$?"

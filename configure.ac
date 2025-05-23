# vi:set et ai sw=2 sts=2 ts=2: */
#-
# Copyright (c) 2008-2011 Jannis Pohlmann <jannis@xfce.org>
# Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>
# Copyright (c) 2020 Ali Abdallah <ali.abdallah@suse.com>
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of 
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public 
# License along with this program; if not, write to the Free 
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

dnl ***************************
dnl *** Version information ***
dnl ***************************
m4_define([copyright_year], [2025])
XDT_VERSION_INIT([4.20.0], [git])

dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2008-2011 Jannis Pohlmann <jannis@xfce.org>.
Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>.
Copyright (c) 2013-copyright_year() The Xfce development team. All rights reserved.])
AC_INIT([xfce4-mixer], [xdt_version], [https://gitlab.xfce.org/apps/xfce4-mixer], [xfce4-mixer])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_REVISION([xdt_version_build])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 no-dist-gzip dist-bzip2 tar-ustar foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_USE_SYSTEM_EXTENSIONS

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AM_PROG_CC_C_O()
if test "x$ac_cv_prog_cc_c99" = xno; then
  AC_MSG_ERROR([$CC does not accept ISO C99])
fi
LT_PATH_LD()
AC_PROG_INSTALL()

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.4])
LT_INIT([disable-static])

dnl ********************
dnl *** Check for OS ***
dnl ********************
AC_ARG_WITH([host],
	    AS_HELP_STRING([--with-host=<option>],
			   [Default host to use linux, freebsd, openbsd]))
# default to a sane option
AC_CANONICAL_HOST
if test x$with_host = x; then
	AS_CASE([$host],
		[*-linux*],   [with_host=linux],
		[*-*freebsd*], [with_host=freebsd],
		[*-*dragonfly*], [with_host=freebsd],
		[*-openbsd*], [with_host=openbsd])
fi

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl **********************
dnl *** Check for libm ***
dnl **********************
AC_CHECK_HEADERS([math.h sys/sysctl.h])
AC_CHECK_LIB([m],[round])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.58.0])
XDT_CHECK_PACKAGE([GTHREAD], [gthread-2.0], [2.58.0])
XDT_CHECK_PACKAGE([GST], [gstreamer-1.0], [1.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.12.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.12.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-2.0], [4.14.0])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.12.0])

dnl ***********************************
dnl *** Check for optional packages ***
dnl ***********************************
XDT_CHECK_OPTIONAL_PACKAGE([KEYBINDER], [keybinder-3.0], [0.3], [keybinder],
                           [keybinder Support])

dnl ***************************
dnl *** PulseAudio backend  ***
dnl ***************************
XDT_CHECK_OPTIONAL_PACKAGE([PULSE], [libpulse-mainloop-glib], [0.9.19], [pulse],
                           [Pulse backend support], [yes])
AM_CONDITIONAL([PULSE_BACKEND], [test x"$PULSE_FOUND" = x"yes"])

dnl *************************
dnl *** OSS sound backend ***
dnl *************************
AC_ARG_ENABLE([oss], [AS_HELP_STRING([--disable-oss],[build without OSS sound support])])
oss_sound=no
if test x$with_host = xfreebsd; then
  if test x"$enable_oss" != x"no"; then
    oss_sound=yes
  fi
fi
AS_IF([test "x$oss_sound" = x"yes"], [AC_DEFINE([HAVE_OSS], [1], [Build with OSS sound support])])
AM_CONDITIONAL(OSS_BACKEND, [test x"$oss_sound" = x"yes"])

dnl **************************
dnl *** ALSA sound backend ***
dnl **************************
XDT_CHECK_OPTIONAL_PACKAGE([ALSA], [alsa], [0.9], [alsa], [Alsa backend support], [yes])
AM_CONDITIONAL([ALSA_BACKEND], [test x"$ALSA_FOUND" = x"yes"])

dnl ***************************
dnl *** SNDIO sound backend ***
dnl ***************************
XDT_CHECK_OPTIONAL_PACKAGE([SNDIO], [sndio], [1.7.0], [sndio], [Sndio backend support], [yes])
AM_CONDITIONAL([SNDIO_BACKEND], [test x"$SNDIO_FOUND" = x"yes"])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl **************************************
dnl *** Check for linker optimizations ***
dnl **************************************
XDT_FEATURE_LINKER_OPTS()

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
icons/Makefile
icons/16x16/Makefile
icons/scalable/Makefile
libxfce4mixer/Makefile
panel-plugin/Makefile
xfce4-mixer/Makefile
po/Makefile.in
])
AC_OUTPUT

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
echo " * keybinder Support:                    ${KEYBINDER_FOUND:-no}"
echo " * Debug Support:                        $enable_debug"
echo
echo " * Host                                  ${with_host}"
echo
echo "Backend configuration:"
echo
echo " * OSS sound backend:                    ${oss_sound}"
echo " * ALSA sound backend:                   ${ALSA_FOUND:-no}"
echo " * Pulse module backend:                 ${PULSE_FOUND:-no}"
echo " * Sndio module backend:                 ${SNDIO_FOUND:-no}"
echo

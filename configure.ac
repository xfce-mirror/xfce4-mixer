dnl Process this file with autoconf to produce a configure script.
AC_INIT([xfce4-mixer], [4.3.0], [xfce4-dev@xfce.org])
AC_REVISION([$Id$])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])
AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for header files.
AC_HEADER_STDC

dnl oss: machine/soundcard.h soundcard.h sys/soundcard.h
AC_CHECK_HEADERS([fcntl.h limits.h sys/ioctl.h errno.h sys/stat.h sys/types.h machine/soundcard.h soundcard.h sys/soundcard.h])

AC_DISABLE_STATIC

AC_PROG_LIBTOOL
AC_PROG_INTLTOOL


dnl Check what sound system to use
AC_ARG_WITH([sound], [
AC_HELP_STRING([--with-sound=auto|alsa|oss|sun], [Check what sound system to use (defaults to auto)])], [],[with_sound=auto])

if test x"$with_sound" != x"alsa" -a x"$with_sound" != x"oss" -a x"$with_sound" != x"sun"; then
  if test x`uname -s` != x"SunOS"; then
    with_sound=oss
  else
    with_sound=sun
  fi
fi

build_sound_system="$with_sound"

AC_MSG_CHECKING([what sound system to use])
if test x"$with_sound" = x"oss"; then
  AC_DEFINE([USE_OSS], [1], [Define for OSS sound system])
  AC_MSG_RESULT([oss])

  dnl Check for OSS audio includes and library
  AC_CHECK_LIB([ossaudio], [_oss_ioctl])
elif test x"$with_sound" = x"alsa"; then
  AC_DEFINE([USE_ALSA], [1], [Define for ALSA sound system])
  AC_MSG_RESULT([alsa])

  dnl Check for ALSA library
  XDT_CHECK_PACKAGE([ALSA], [alsa], [0.9.0])
  CFLAGS="$CFLAGS $ALSA_CFLAGS"
  LIBS="$LIBS $ALSA_LIBS"
elif test x"$with_sound" = x"sun"; then
  AC_DEFINE([USE_SUN], [1], [Define for Sun sound system])
  AC_MSG_RESULT([sun])
else
  AC_MSG_ERROR([Unknown sound system "$with_sound"])
fi

XDT_CHECK_PACKAGE([XML], [libxml-2.0], [2.4.0])
XDT_CHECK_PACKAGE([XFCE4_UTIL], [libxfce4util-1.0], [4.2.0])
XDT_CHECK_PACKAGE([XFCE4_GUI], [libxfcegui4-1.0], [4.2.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-2.0], [2.2.0])
XDT_CHECK_PACKAGE([XFCE_MCS_CLIENT], [libxfce4mcs-client-1.0], [4.2.0])
XDT_XFCE_PANEL_PLUGIN([XFCE4_PANEL], [4.2.0])
XDT_XFCE_MCS_PLUGIN([XFCE_MCS_MANAGER], [4.2.0])

dnl Check for i18n support
XDT_I18N([ar az be bg ca de en_GB es es_MX et eu fa fi fr gu he \
			    hu it ja ko lt ms nl nb_NO pa pt_BR pt_PT
			    ro ru sk sv tr \
			    uk vi zh_CN zh_TW])

AC_MSG_CHECKING([whether to build big mixer window])
AC_ARG_ENABLE([big-mixer],
  AC_HELP_STRING([--disable-big-mixer], [don't build big mixer window]),
[], [enable_big_mixer=yes])
AM_CONDITIONAL([BUILD_BIG_MIXER], [test x"$enable_big_mixer" = x"yes"])
AC_MSG_RESULT([$enable_big_mixer])
build_big_mixer="$enable_big_mixer"

dnl Check for debugging support
BM_DEBUG_SUPPORT()


dnl Checks for library functions.
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST



AC_OUTPUT([
Makefile
xfce4-mixer.spec
include/Makefile
lib/Makefile
src/Makefile
panel-plugin/Makefile
settings/Makefile
po/Makefile.in
])


echo
echo "Build Configuration:"
echo " Sound System:  $build_sound_system"
echo " Big Mixer:     $build_big_mixer"
echo

dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/main.c)

dnl Checks for programs.

dnl Checks for libraries.


AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([xfce4-mixer], [4.1.91])
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


dnl Check what sound system to use
AC_ARG_WITH([sound], [
AC_HELP_STRING([--with-sound=alsa|oss], [Check what sound system to use (defaults to OSS)])], [], [with_sound=oss])

AC_MSG_CHECKING([what sound system to use])
if test x"$with_sound" = x"oss"; then
  AC_DEFINE([USE_OSS], [1], [Define for OSS sound system])
  AC_MSG_RESULT([OSS])

  dnl Check for OSS audio includes and library
  AC_CHECK_LIB([ossaudio], [_oss_ioctl])
elif test x"$with_sound" = x"alsa"; then
  AC_DEFINE([USE_ALSA], [1], [Define for ALSA sound system])
  AC_MSG_RESULT([ALSA])

  dnl Check for ALSA library
  BM_DEPEND([ALSA], [alsa], [0.9.0])
  CFLAGS="$CFLAGS $ALSA_CFLAGS"
  LIBS="$LIBS $ALSA_LIBS"
else
  AC_MSG_ERROR([Unknown sound system "$with_sound"])
fi

dnl Check for i18n support
BM_I18N([xfce4-mixer], [ar az be bg ca de es es_MX et eu fa fi fr gu he hu it
			ja ko lt ms nl no pa pt_BR pt_PT ru sk tr vi zh_CN 
			zh_TW])

BM_DEPEND([XML], [libxml-2.0], [2.4.0])
BM_DEPEND([XFCE4_UTIL], [libxfce4util-1.0], [4.0.0])
BM_DEPEND([XFCE4_GUI], [libxfcegui4-1.0], [4.1.0])
BM_DEPEND([GTK], [gtk+-2.0], [2.0.6])
BM_DEPEND([XFCE_MCS_CLIENT], [libxfce4mcs-client-1.0], [4.0.0])
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [4.0.0])
XFCE_MCS_PLUGIN([XFCE_MCS_MANAGER], [4.0.0])

AC_MSG_CHECKING([whether to build big mixer window])
AC_ARG_ENABLE([big-mixer],
  AC_HELP_STRING([--disable-big-mixer], [don't build big mixer window]),
[], [enable_big_mixer=yes])
AM_CONDITIONAL([BUILD_BIG_MIXER], [test x"$enable_big_mixer" = x"yes"])
AC_MSG_RESULT([$enable_big_mixer])

dnl Check for debugging support
BM_DEBUG_SUPPORT


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

dnl configure.ac
dnl
dnl xfce4-mixer - Volume changer for XFce4
dnl
dnl 2003 Danny Milosavljevic <danny_milo@yahoo.com>
dnl 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
dnl

AC_INIT([configure])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([xfce4-mixer], [3.99.3])

AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for standard header files
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h sys/ioctl.h])

dnl Check what sound system to use
AC_ARG_WITH([sound], [
AC_HELP_STRING([--with-sound=alsa|oss], [Check what sound system to use (defaults to OSS)])], [], [with_sound=oss])

AC_MSG_CHECKING([what sound system to use])
if test x"$with_sound" = x"oss"; then
  AC_DEFINE([USE_OSS], [1], [Define for OSS sound system])
  AC_MSG_RESULT([OSS])

  dnl Check for OSS audio includes and library
  AC_CHECK_HEADERS([machine/soundcard.h soundcard.h sys/soundcard.h])
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
BM_I18N([xfce4-mixer], [ca de fr hu it lt nl ru tr zh_CN zh_TW])

dnl configure the panel plugin
XFCE_PANEL_PLUGIN([XFCE4_PANEL], [3.99.3])

dnl Check for startup notification
BM_DEPEND_CHECK(LIBSTARTUP_NOTIFICATION, libstartup-notification-1.0, 0.5,
  [startup-notification], [startup notification library], [yes])

dnl Check for debugging support
BM_DEBUG_SUPPORT

AC_OUTPUT([
Makefile
panel-plugin/Makefile
po/Makefile.in
xfce4-mixer.spec
])

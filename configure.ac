dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/main.c)

dnl Checks for programs.

dnl Checks for libraries.


AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([xfce4-mixer], [4.0.0])
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
AC_CHECK_HEADERS([fcntl.h limits.h sys/ioctl.h])

dnl required versions of other packages
dnl not AC_SUBST(PYGTK_REQ, 2.0.0)
AC_SUBST(GLIB_REQ,  2.0.0)
AC_SUBST(GTK_REQ,   2.0.0)

AC_DISABLE_STATIC

AC_PROG_LIBTOOL

dnl check for glib
AM_PATH_GLIB_2_0($GLIB_REQ,,[AC_MSG_ERROR(maybe you want the gtk-gnome-1-2 branch?)],$extra_mods)

dnl check to make sure we can find gtk
AM_PATH_GTK_2_0($GTK_REQ,,[AC_MSG_ERROR(maybe you want the gtk-gnome-1-2 branch?)],$extra_mods)

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
BM_I18N([xfce4-mixer], [ca de fr hu it lt ms nl pt_PT ru tr zh_CN zh_TW ko])


XFCE_PANEL_PLUGIN([XFCE4_PANEL], [0.10.1])

dnl Check for debugging support
BM_DEBUG_SUPPORT


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.

AC_OUTPUT([
Makefile
src/Makefile
panel-plugin/Makefile
settings/Makefile
])

@SET_MAKE@

CLEANFILES = *~ *.bak core *.core *.o *.lo

INCLUDES = -I$(top_srcdir)/include

plugindir = $(libdir)/xfce4/modules

plugin_LTLIBRARIES = libxfce4mixer.la

libxfce4mixer_la_SOURCES = trans.c vc_alsa.c vc_alsa_fallback.c vc.c vc_oss.c vcs.c vc_sun.c vc_sgi.c stringlist.inc

libxfce4mixer_la_LDFLAGS = -export-dynamic

libxfce4mixer_la_CFLAGS = @GTK_CFLAGS@ @XFCE4_UTIL_CFLAGS@
libxfce4mixer_la_LIBADD = @GTK_LIBS@ @XFCE4_UTIL_LIBS@

test_vc_CFLAGS = @GTK_CFLAGS@ @XFCE4_UTIL_CFLAGS@
test_vc_LDFLAGS = @GTK_LIBS@ @XFCE4_UTIL_LIBS@ libxfce4mixer.la
noinst_PROGRAMS = test_vc

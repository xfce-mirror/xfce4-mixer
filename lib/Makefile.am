@SET_MAKE@

CLEANFILES = *~ *.bak core *.core *.o *.lo

INCLUDES = -I../include

plugindir = $(libdir)/xfce4/modules

plugin_LTLIBRARIES = libxfce4mixer.la

libxfce4mixer_la_SOURCES = vc.c vc_oss.c vc_alsa.c vc_sun.c vcs.c
libxfce4mixer_la_LDFLAGS = -export-dynamic

libxfce4mixer_la_CFLAGS = @GTK_CFLAGS@ @XFCE4_UTIL_CFLAGS@
libxfce4mixer_la_LIBADD = @GTK_LIBS@ @XFCE4_UTIL_LIBS@

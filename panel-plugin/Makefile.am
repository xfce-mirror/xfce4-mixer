plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES =							\
	libmixer.la

libmixer_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-module

libmixer_la_SOURCES =							\
	mixer.c								\
	vc.c								\
	vc.h								\
	vc_alsa_fallback.c						\
	vc_alsa.c							\
	vc_oss.c

libmixer_la_CFLAGS =							\
	@XFCE4_PANEL_CFLAGS@

libmixer_la_LIBADD = 							\
	@XFCE4_PANEL_LIBS@

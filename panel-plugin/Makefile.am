plugindir = @XFCE4_PANEL_PLUGINSDIR@

plugin_LTLIBRARIES =							\
	libmixer.la

libmixer_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-module

libmixer_la_SOURCES =							\
	mixer.c								\
	mixer_window.c							\
	mixer_window.h							\
	mvisible_opts.c							\
	mvisible_opts.h							\
	vc_alsa.c							\
	vc_alsa_fallback.c						\
	vc.c								\
	vc.h								\
	vc_oss.c

libmixer_la_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	@XFCE4_PANEL_CFLAGS@                                            \
	@LIBSTARTUP_NOTIFICATION_CFLAGS@

libmixer_la_LIBADD = 							\
	@XFCE4_PANEL_LIBS@                                              \
	@LIBSTARTUP_NOTIFICATION_LIBS@

plugindir = @XFCE4_PANEL_PLUGINSDIR@

INCLUDES = -I../include

plugin_LTLIBRARIES =							\
	libmixer.la

libmixer_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-module

libmixer_la_SOURCES =							\
	xfce-mixer-preferences.c plugin.c xfce-mixer-cache-vc.c \
	xfce-mixer-pxml.c string_option_menu.c  xfce-mixer-control.c \
	xfce-mixer-slider-tiny.c xfce-mixer-prefbox.c \
	xfce-mixer-control-vc.c

libmixer_la_CFLAGS =							\
	@XFCE_MCS_CLIENT_CFLAGS@					\
	@XFCE4_PANEL_CFLAGS@

libmixer_la_LIBADD = 							\
	@XFCE_MCS_CLIENT_LIBS@						\
	@XFCE4_PANEL_LIBS@ \
	$(top_builddir)/lib/libxfce4mixer.la

libmixer_la_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la

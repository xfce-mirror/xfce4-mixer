plugindir = $(libdir)/xfce4/panel-plugins

INCLUDES = -I../include

plugin_LTLIBRARIES =							\
	libmixer.la

libmixer_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-module

libmixer_la_SOURCES =							\
			delayer.inc					\
			helpers_h.inc					\
			helpers.inc					\
			plugin.c					\
			stringlist.inc					\
			string_option_menu.c				\
			string_option_menu.h				\
			xfce-mixer-cache-vc.c				\
			xfce-mixer-cache-vc.h				\
			xfce-mixer-control.c				\
			xfce-mixer-control.h				\
			xfce-mixer-control-vc.c				\
			xfce-mixer-control-vc.h				\
			xfce-mixer-prefbox.c				\
			xfce-mixer-prefbox.h				\
			xfce-mixer-prefbox-private.h			\
			xfce-mixer-preferences.c			\
			xfce-mixer-preferences.h			\
			xfce-mixer-preferences-private.h		\
			xfce-mixer-pxml.c				\
			xfce-mixer-pxml.h				\
			xfce-mixer-pxml-private.h			\
			xfce-mixer-slider-tiny.c			\
			xfce-mixer-slider-tiny.h			\
			xfce-mixer-slider-tiny-private.h

libmixer_la_CFLAGS =							\
	-I$(top_srcdir)/include						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"                           \
	@XFCE_MCS_CLIENT_CFLAGS@					\
	@XFCE4_PANEL_CFLAGS@

libmixer_la_LIBADD = 							\
	@XFCE_MCS_CLIENT_LIBS@						\
	@XFCE4_PANEL_LIBS@ \
	$(top_builddir)/lib/libxfce4mixer.la

libmixer_la_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la

EXTRA_DIST = test_vc.c

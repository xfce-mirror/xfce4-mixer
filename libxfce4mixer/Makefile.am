# $Id$ 
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

noinst_LTLIBRARIES = 							\
	libxfce4mixer.la

libxfce4mixer_la_SOURCES =						\
	xfce-mixer-card-combo.h						\
	xfce-mixer-card-combo.c						\
	xfce-mixer-track-combo.h					\
	xfce-mixer-track-combo.c					\
	xfce-mixer-card.h						\
	xfce-mixer-card.c						\
	xfce-mixer-stock.h						\
	xfce-mixer-stock.c						\
	xfce-mixer-track-type.h						\
	xfce-mixer-track-type.c						\
	xfce-mixer-utilities.h						\
	xfce-mixer-utilities.c						\
	xfce-mixer-preferences.h					\
	xfce-mixer-preferences.c

libxfce4mixer_la_CFLAGS = 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DDATADIR=\"$(datadir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DG_LOG_DOMAIN=\"libxfce4mixer\"				\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCEGUI4_CFLAGS)						\
	$(GST_PLUGINS_BASE_CFLAGS)

libxfce4mixer_la_LDFLAGS =						\
	-no-undefined

libxfce4mixer_la_LIBADD =						\
	$(PLATFORM_LDFLAGS)						\
	$(GLIB_LIBS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCEGUI4_LIBS)						\
	$(GST_PLUGINS_BASE_LIBS)					\
	-lgstaudio-0.10							\
	-lgstinterfaces-0.10

# $Id$ 
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

noinst_LTLIBRARIES = 							\
	libxfce4mixer.la

libxfce4mixer_la_SOURCES =						\
	libxfce4mixer.h							\
	libxfce4mixer.c							\
	xfce-mixer-card-combo.h						\
	xfce-mixer-card-combo.c						\
	xfce-mixer-track-combo.h					\
	xfce-mixer-track-combo.c					\
	xfce-mixer-track-type.h						\
	xfce-mixer-track-type.c						\
	xfce-mixer-preferences.h					\
	xfce-mixer-preferences.c					\
	xfce-mixer-debug.h						\
	xfce-mixer-debug.c

libxfce4mixer_la_CPPFLAGS = 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DDATADIR=\"$(datadir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DG_LOG_DOMAIN=\"libxfce4mixer\"				\
	-DMIXER_DATADIR=\"$(datadir)/xfce4/mixer\"

libxfce4mixer_la_CFLAGS = 						\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(DBUS_GLIB_CFLAGS)						\
	$(GST_PLUGINS_BASE_CFLAGS)

libxfce4mixer_la_LDFLAGS =						\
	-no-undefined

libxfce4mixer_la_LIBADD =						\
	$(PLATFORM_LDFLAGS)						\
	$(GLIB_LIBS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(XFCONF_LIBS)							\
	$(DBUS_GLIB_LIBS)						\
	$(GST_PLUGINS_BASE_LIBS)					\
	-lgstaudio-0.10							\
	-lgstinterfaces-0.10

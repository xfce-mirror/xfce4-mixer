# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

bin_PROGRAMS =								\
	xfce4-mixer

xfce4_mixer_SOURCES =							\
	main.c								\
	xfce-mixer-track.c						\
	xfce-mixer-track.h						\
	xfce-mixer-switch.c						\
	xfce-mixer-switch.h						\
	xfce-mixer-option.c						\
	xfce-mixer-option.h						\
	xfce-mixer.c							\
	xfce-mixer.h							\
	xfce-mixer-controls-dialog.c					\
	xfce-mixer-controls-dialog.h					\
	xfce-mixer-window.h						\
	xfce-mixer-window.c

xfce4_mixer_CPPFLAGS = 							\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)\"					\
	-DMIXER_DATADIR=\"$(pkgdatadir)\"				\
	-DG_LOG_DOMAIN=\"xfce4-mixer\"

xfce4_mixer_CFLAGS = 							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(UNIQUE_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(GST_PLUGINS_BASE_CFLAGS)

xfce4_mixer_DEPENDENCIES = 						\
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la

xfce4_mixer_LDFLAGS = 							\
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la			\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GTK_LIBS)							\
	$(UNIQUE_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(XFCONF_LIBS)							\
	$(GST_PLUGINS_BASE_LIBS)					\
	-lgstaudio-0.10							\
	-lgstinterfaces-0.10

desktop_in_files = xfce4-mixer.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 								\
 	$(desktop_in_files)
 
DISTCLEANFILES =							\
	$(desktop_DATA)


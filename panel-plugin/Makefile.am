# $Id$
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

plugindir = $(libexecdir)/xfce4/panel-plugins

plugin_PROGRAMS =							\
	xfce4-mixer-plugin

xfce4_mixer_plugin_SOURCES =						\
	xfce-plugin-dialog.h						\
	xfce-plugin-dialog.c						\
	xfce-volume-button.h						\
	xfce-volume-button.c						\
	xfce-mixer-plugin.c

xfce4_mixer_plugin_CFLAGS =						\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DDATADIR=\"$(datadir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DG_LOG_DOMAIN=\"xfce4-mixer-plugin\"				\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)						\
	$(GST_PLUGINS_BASE_CFLAGS)

xfce4_mixer_plugin_DEPENDENCIES =					\
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la

xfce4_mixer_plugin_LDFLAGS =						\
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la			\
	$(PLATFORM_LDFLAGS)						\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)						\
	$(GST_PLUGINS_BASE_LIBS)					\
	-lgstaudio-0.10							\
	-lgstinterfaces-0.10

desktopdir = 								\
	$(datadir)/xfce4/panel-plugins

desktop_in_in_files = 							\
	xfce4-mixer-plugin.desktop.in.in

desktop_in_files = 							\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@

desktop_DATA = 								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

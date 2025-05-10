# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

plugindir = $(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = libmixer.la

libmixer_la_SOURCES =							\
	xfce-plugin-dialog.h						\
	xfce-plugin-dialog.c						\
	xfce-volume-button.h						\
	xfce-volume-button.c						\
	xfce-mixer-plugin.h						\
	xfce-mixer-plugin.c

libmixer_la_CPPFLAGS =							\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DDATADIR=\"$(datadir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DG_LOG_DOMAIN=\"xfce4-mixer-plugin\"

libmixer_la_CFLAGS =							\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(GST_CFLAGS)							\
	$(KEYBINDER_CFLAGS)

libmixer_la_DEPENDENCIES =						\
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la

libmixer_la_LDFLAGS =							\
	-avoid-version							\
	-module								\
	-no-undefined							\
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(top_builddir)/libxfce4mixer/libxfce4mixer.la			\
	$(PLATFORM_LDFLAGS)

libmixer_la_LIBADD =							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)						\
	$(XFCONF_LIBS)							\
	$(GST_LIBS)							\
	$(KEYBINDER_LIBS)

desktopdir = $(datadir)/xfce4/panel/plugins

desktop_in_files = mixer.desktop.in

desktop_DATA = 								\
	$(desktop_in_files:.desktop.in=.desktop)

mixer.desktop: mixer.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = 								\
	$(desktop_in_files)						\
	meson.build							\
	$(NULL)

CLEANFILES = $(desktop_DATA)

sounddir = @XFCE_MCS_MANAGER_PLUGINSDIR@

INCLUDES = -I../include

sound_LTLIBRARIES = 							\
	sound_settings.la

sound_settings_la_LDFLAGS =						\
	-avoid-version							\
	-module

sound_settings_la_SOURCES =						\
	sound.c								\
	xfce-mixer-settingsbox.c xfce-mixer-settingsbox.h xfce-mixer-settingsbox-private.h \
	string_option_menu.c string_option_menu.h \
	helpers3.inc stringlist.inc 

sound_settings_la_CFLAGS =						\
	@XFCE_MCS_MANAGER_CFLAGS@					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

sound_settings_la_LIBADD =						\
	$(top_builddir)/lib/libxfce4mixer.la

sound_settings_la_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la

desktopdir = $(datadir)/applications
desktop_DATA = xfce-mixer-settings.desktop

iconsdir = $(datadir)/icons/hicolor/48x48/apps
icons_DATA = xfce4-mixer.png

EXTRA_DIST =								\
	$(desktop_DATA)							\
	$(icons_DATA)


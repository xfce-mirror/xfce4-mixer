sounddir = @XFCE_MCS_MANAGER_PLUGINSDIR@

INCLUDES = -I../include

sound_LTLIBRARIES = 							\
	sound_settings.la

sound_settings_la_LDFLAGS =						\
	-avoid-version							\
	-module

sound_settings_la_SOURCES =						\
	sound-icon.h							\
	sound.c	\
	xfce-mixer-settingsbox.c \
	string_option_menu.c

sound_settings_la_CFLAGS =						\
	@XFCE_MCS_MANAGER_CFLAGS@					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

sound_settings_la_LIBADD =						\
	$(top_builddir)/lib/libxfce4mixer.la


sound.c: sound-icon.h


sound-icon.h: Makefile $(srcdir)/sound.png
	gdk-pixbuf-csource --raw --build-list				\
	sound_icon_data $(srcdir)/sound.png > sound-icon.h

noinst_DATA =								\
	sound-icon.h

EXTRA_DIST =								\
	sound.png								\
	xfce-mixer-settings.desktop

sound_settings_la_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la

appsdir = $(datadir)/applications
apps_DATA = xfce-mixer-settings.desktop

sounddir = @XFCE_MCS_MANAGER_PLUGINSDIR@

sound_LTLIBRARIES = 							\
	sound_settings.la

sound_settings_la_LDFLAGS =						\
	-avoid-version							\
	-module

sound_settings_la_SOURCES =						\
	sound-icon.h							\
	sound.c

sound_settings_la_CFLAGS =						\
	@XFCE_MCS_MANAGER_CFLAGS@					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

sound-icon.h: $(srcdir)/sound.png
	gdk-pixbuf-csource --raw --build-list				\
	sound_icon_data $(srcdir)/sound.png > sound-icon.h

noinst_DATA =								\
	sound-icon.h

EXTRA_DIST =								\
	sound.png

sounddir = @XFCE_MCS_MANAGER_PLUGINSDIR@

sound_LTLIBRARIES = 							\
	sound_settings.la

sound_settings_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module

sound_settings_la_SOURCES =						\
	sound-icon.h							\
	sound.c	\
	xfce-mixer-settingsbox.c \
	vc.c vc_alsa.c vc_oss.c

sound_settings_la_CFLAGS =						\
	@XFCE_MCS_MANAGER_CFLAGS@					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"

sound_settings_la_LIBADD =						\
	@XFCE_MCS_MANAGER_LIBS@

sound.c: sound-icon.h


sound-icon.h: Makefile $(srcdir)/sound.png
	gdk-pixbuf-csource --raw --build-list				\
	sound_icon_data $(srcdir)/sound.png > sound-icon.h

noinst_DATA =								\
	sound-icon.h

EXTRA_DIST =								\
	sound.png

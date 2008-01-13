# $Id$
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

bin_PROGRAMS =								\
	xfce4-mixer

xfce4_mixer_SOURCES =							\
	main.c								\
	xfce-mixer-card.c						\
	xfce-mixer-card.h						\
	xfce-mixer-stock.c						\
	xfce-mixer-stock.h						\
	xfce-mixer-track-type.c						\
	xfce-mixer-track-type.h						\
	xfce-mixer-track.c						\
	xfce-mixer-track.h						\
	xfce-mixer-switch.c						\
	xfce-mixer-switch.h						\
	xfce-mixer-option.c						\
	xfce-mixer-option.h						\
	xfce-mixer.c							\
	xfce-mixer.h							\
	xfce-mixer-preferences.c					\
	xfce-mixer-preferences.h					\
	xfce-mixer-controls-dialog.c					\
	xfce-mixer-controls-dialog.h					\
	xfce-mixer-window.h						\
	xfce-mixer-window.c

xfce4_mixer_CFLAGS = 							\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)\"					\
	@GLIB_CFLAGS@							\
	@GTHREAD_CFLAGS@						\
	@GST_PLUGINS_BASE_CFLAGS@					\
	@GTK_CFLAGS@							\
	@LIBXFCE4UTIL_CFLAGS@						\
	@LIBXFCEGUI4_CFLAGS@						

xfce4_mixer_LDFLAGS = 							\
	@GLIB_LIBS@							\
	@GTHREAD_LIBS@							\
	@GST_PLUGINS_BASE_LIBS@						\
	@GTK_LIBS@							\
	@LIBXFCE4UTIL_LIBS@						\
	@LIBXFCEGUI4_LIBS@						\
	-lgstaudio-0.10							\
	-lgstinterfaces-0.10

# .desktop file
#
# Some automake trickery here. Because we cannot use $(libexecdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = xfce4-mixer.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = 								\
 	$(desktop_in_in_files)
 
DISTCLEANFILES =							\
	$(desktop_DATA) $(desktop_in_files)

# Get full path into .desktop file
%.desktop.in: %.desktop.in.in
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/applications^" 		\
		$< > $@

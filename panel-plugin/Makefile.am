
plugindir = $(libexecdir)/xfce4/panel-plugins
plugin_PROGRAMS = xfce4-mixer-plugin

INCLUDES = -I../include

# @PANEL_EXTERNALPLUGINDIR@

desktopdatadir = @PANEL_DESKTOPDATADIR@
desktopdata_DATA = xfce4-mixer.desktop

xfce4_mixer_plugin_LDFLAGS = \
	@MIGHTY_MOUSE_LIBS@ \
	@XFCE_MCS_CLIENT_LIBS@                                          \
	@XFCE4_GUI_LIBS@ @GTK_LIBS@ @XML_LIBS@ \
	@MIGHTY_MOUSE_LIBS@ \
	$(top_builddir)/lib/libxfce4mixer.la
                                        

xfce4_mixer_plugin_SOURCES =							\
			delayer.inc					\
			helpers_h.inc					\
			helpers.inc					\
			plugin.c					\
			stringlist.inc					\
			string_option_menu.c				\
			string_option_menu.h				\
			xfce-mixer-cache-vc.c				\
			xfce-mixer-cache-vc.h				\
			xfce-mixer-control.c				\
			xfce-mixer-control.h				\
			xfce-mixer-control-vc.c				\
			xfce-mixer-control-vc.h				\
			xfce-mixer-prefbox.c				\
			xfce-mixer-prefbox.h				\
			xfce-mixer-prefbox-private.h			\
			xfce-mixer-preferences.c			\
			xfce-mixer-preferences.h			\
			xfce-mixer-preferences-private.h		\
			xfce-mixer-pxml.c				\
			xfce-mixer-pxml.h				\
			xfce-mixer-pxml-private.h			\
			xfce-mixer-slider-tiny.c			\
			xfce-mixer-slider-tiny.h			\
			xfce-mixer-slider-tiny-private.h

xfce4_mixer_plugin_CFLAGS =							\
	-I$(top_srcdir)/include						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"                           \
	@XFCE_MCS_CLIENT_CFLAGS@					\
	@XFCE4_GUI_CFLAGS@ @GTK_CFLAGS@ @XML_CFLAGS@ \
	@MIGHTY_MOUSE_CFLAGS@

dnl libmixer_la_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la


# .desktop file
#
# Some automake trickery here. Because we cannot use $(libexecdir) in the
# automake stage, we'll use sed to get the full path into the .desktop file.
# We also need to let intltool merge the translated fields, so we add an
# additional level of indirection: a <name>.desktop.in.in file.
# 
desktop_in_in_files = testplugin.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
	
desktopdir = $(datadir)/xfce4/panel-plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =  test_vc.c xfce4-mixer.desktop \
	$(desktop_in_in_files)

DISTCLEANFILES = \
	$(desktop_DATA) $(desktop_in_files)
	
# get full path into .desktop file
%.desktop.in: %.desktop.in.in
	sed -e "s^@EXTERNAL_PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" \
		$< > $@


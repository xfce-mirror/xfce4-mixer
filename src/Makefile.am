@SET_MAKE@

INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = xfce4-mixer

#GOB2 = gob2
xfce4_mixer_SOURCES = \
	xfce-mixer-control.c xfce-mixer-control.h xfce-mixer-control-private.h \
	xfce-mixer-slider.c xfce-mixer-slider.h xfce-mixer-slider-private.h \
	xfce-mixer-slider-tiny.c xfce-mixer-slider-tiny.h xfce-mixer-slider-tiny-private.h \
	xfce-mixer-profile.c xfce-mixer-profile.h \
	xfce-mixer-control-factory.c xfce-mixer-control-factory.h \
	xfce-mixer-view.c xfce-mixer-view.h xfce-mixer-view-private.h \
	xfce-mixer-view-tiny.c xfce-mixer-view-tiny.h \
	main.c main.h \
	xfce-mixer-cache-vc.c xfce-mixer-cache-vc.h \
	disclosure-widget.c disclosure-widget.h \
	menus.c menus.h \
	menu-callbacks.inc \
	xfce-mixer-window.c xfce-mixer-window.h xfce-mixer-window-private.h \
	xfce-mixer-switch.c xfce-mixer-switch.h xfce-mixer-switch-private.h \
	xfce-mixer-info.c xfce-mixer-info.h \
	xfce-mixer-control-vc.c xfce-mixer-control-vc.h \
	xfce-mixer-select.c xfce-mixer-select.h xfce-mixer-select-private.h \
	string_option_menu.c string_option_menu.h \
	xfce-mixer-mcs-client.c xfce-mixer-mcs-client.h xfce-mixer-mcs-client-private.h \
	xfce-mixer-profiledlg.c xfce-mixer-profiledlg.h xfce-mixer-profiledlg-private.h \
	xfce-mixer-profilebox.c xfce-mixer-profilebox.h xfce-mixer-profilebox-private.h \
	xfce-mixer-profilebox-detail.c xfce-mixer-profilebox-detail.h xfce-mixer-profilebox-detail-private.h \
	xfce-mixer-pxml.c xfce-mixer-pxml.h xfce-mixer-pxml-private.h \
	xfce-mixer-profiles.c xfce-mixer-profiles.h xfce-mixer-profiles-private.h \
	xfce_sizehook.c xfce_sizehook.h \
	my-config.c my-config.h \
	stringlist.inc helpers3.inc msgdlg.inc \
	launcher-entry.c launcher-entry.h

xfce4_mixer_GSOURCES = mixer-control.gob \
	mixer-slider.gob mixer-view.gob mixer-view-tiny.gob \
	mixer-slider-tiny.gob mixer-window.gob mixer-switch.gob \
	mixer-info.gob mixer-select.gob mixer-mcs-client.gob \
	mixer-profiledlg.gob mixer-profilebox.gob \
	mixer-profilebox-detail.gob mixer-pxml.gob \
	mixer-profiles.gob

xfce4_mixer_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"                           \
	@XFCE_MCS_CLIENT_CFLAGS@ \
	@XFCE4_UTIL_CFLAGS@						\
	@XFCE4_GUI_CFLAGS@ \
	@GTK_CFLAGS@ \
	@XML_CFLAGS@
	
xfce4_mixer_LDADD = 							\
	@XFCE_MCS_CLIENT_LIBS@ \
	@XFCE4_UTIL_LIBS@						\
	@XFCE4_GUI_LIBS@ \
	@GTK_LIBS@ \
	@XML_LIBS@ \
	$(top_builddir)/lib/libxfce4mixer.la
	
#OBJECTS = xfce-mixer-control.o xfce-mixer-slider.o xfce-mixer-profile.o \
#	xfce-mixer-slider-tiny.o xfce-mixer-view.o xfce-mixer-view-tiny.o \
#	xfce-mixer-control-factory.o vc.o vc_alsa.o disclosure-widget.o \
#	xfce-mixer-window.o xfce-mixer-switch.o xfce-mixer-info.o \
#	xfce-mixer-control-vc.o string_option_menu.o xfce-mixer-mcs-client.o

#%.c %.h %-private.h: %.gob
#	@GOB2@ $<

ACLOCAL_AMFLAGS = -I m4

xfce4_mixer_DEPENDENCIES = $(top_builddir)/lib/libxfce4mixer.la

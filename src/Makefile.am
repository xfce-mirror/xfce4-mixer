@SET_MAKE@

INCLUDES = -I../include

bin_PROGRAMS = xfce4-mixer

#GOB2 = gob2
xfce4_mixer_SOURCES = xfce-mixer-control.c xfce-mixer-slider.c \
	xfce-mixer-slider-tiny.c \
	xfce-mixer-profile.c xfce-mixer-control-factory.c \
	xfce-mixer-view.c xfce-mixer-view-tiny.c main.c \
	xfce-mixer-cache-vc.c disclosure-widget.c \
	menus.c menu-callbacks.inc xfce-mixer-window.c \
	xfce-mixer-switch.c xfce-mixer-info.c xfce-mixer-control-vc.c \
	xfce-mixer-select.c string_option_menu.c \
	xfce-mixer-mcs-client.c xfce-mixer-profiledlg.c \
	xfce-mixer-profilebox.c xfce-mixer-profilebox-detail.c \
	xfce-mixer-pxml.c

xfce4_mixer_GSOURCES = mixer-control.gob \
	mixer-slider.gob mixer-view.gob mixer-view-tiny.gob \
	mixer-slider-tiny.gob mixer-window.gob mixer-switch.gob \
	mixer-info.gob mixer-select.gob mixer-mcs-client.gob \
	mixer-profiledlg.gob mixer-profilebox.gob \
	mixer-profilebox-detail.gob mixer-pxml.gob

xfce4_mixer_CFLAGS =							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"                           \
	@XFCE_MCS_CLIENT_CFLAGS@ \
	@XFCE4_UTIL_CFLAGS@						\
	@GTK_CFLAGS@ \
	@XML_CFLAGS@
	
xfce4_mixer_LDADD = 							\
	@XFCE_MCS_CLIENT_LIBS@ \
	@XFCE4_UTIL_LIBS@						\
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

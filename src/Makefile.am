@SET_MAKE@

bin_PROGRAMS = xfce4-mixer

GOB2 = gob2
xfce4_mixer_CSOURCES = xfce-mixer-control.c xfce-mixer-slider.c \
	xfce-mixer-slider-tiny.c \
	xfce-mixer-profile.c xfce-mixer-control-factory.c \
	xfce-mixer-view.c xfce-mixer-view-tiny.c main.c \
	vc.c vc_alsa.c vc_oss.c vc_sun.c \
	xfce-mixer-cache-vc.c disclosure-widget.c \
	menus.c menu-callbacks.inc xfce-mixer-window.c \
	xfce-mixer-switch.c xfce-mixer-info.c xfce-mixer-control-vc.c \
	xfce-mixer-select.c string_option_menu.c

xfce4_mixer_SOURCES = $(xfce4_mixer_CSOURCES) mixer-control.gob \
	mixer-slider.gob mixer-view.gob mixer-view-tiny.gob \
	mixer-slider-tiny.gob mixer-window.gob mixer-switch.gob \
	mixer-info.gob mixer-select.gob

xfce4_mixer_CFLAGS =							\
	@XFCE4_UTIL_CFLAGS@						\
	@GTK_CFLAGS@
	
xfce4_mixer_LDADD = 							\
	@XFCE4_UTIL_LIBS@						\
	@GTK_LIBS@
	
OBJECTS = xfce-mixer-control.o xfce-mixer-slider.o xfce-mixer-profile.o \
	xfce-mixer-slider-tiny.o xfce-mixer-view.o xfce-mixer-view-tiny.o \
	xfce-mixer-control-factory.o vc.o vc_alsa.o disclosure-widget.o \
	xfce-mixer-window.o xfce-mixer-switch.o xfce-mixer-info.o \
	xfce-mixer-control-vc.o string_option_menu.o

#%.c %.h %-private.h: %.gob
#	@GOB2@ $<

ACLOCAL_AMFLAGS = -I m4

# $Id$ 
#
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

noinst_LTLIBRARIES = 							\
	libxfce4mixer.la

libxfce4mixer_la_SOURCES =						\
	libxfce4mixer.h							\
	libxfce4mixer.c							\
	gst-mixer.c							\
	gst-mixer.h							\
	gst-mixer-track.c						\
	gst-mixer-track.h						\
	gst-mixer-options.c						\
	gst-mixer-options.h						\
	xfce-mixer-card-combo.h						\
	xfce-mixer-card-combo.c						\
	xfce-mixer-track-combo.h					\
	xfce-mixer-track-combo.c					\
	xfce-mixer-track-type.h						\
	xfce-mixer-track-type.c						\
	xfce-mixer-preferences.h					\
	xfce-mixer-preferences.c					\
	xfce-mixer-debug.h						\
	xfce-mixer-debug.c

if ALSA_BACKEND
libxfce4mixer_la_SOURCES +=						\
	alsa/alsa-mixer.c						\
	alsa/alsa-mixer.h						\
	alsa/alsa-track.c						\
	alsa/alsa-track.h						\
	alsa/alsa-options.c						\
	alsa/alsa-options.h
endif

if OSS_BACKEND
libxfce4mixer_la_SOURCES +=						\
	oss/oss-mixer.c							\
	oss/oss-mixer.h							\
	oss/oss-track.c							\
	oss/oss-track.h							\
	oss/oss-options.c						\
	oss/oss-options.h
endif

if PULSE_BACKEND
libxfce4mixer_la_SOURCES +=						\
	pulse/pulse-mixer.c						\
	pulse/pulse-mixer.h						\
	pulse/pulse-track.c						\
	pulse/pulse-track.h						\
	pulse/pulse-options.c						\
	pulse/pulse-options.h
endif

if SNDIO_BACKEND
libxfce4mixer_la_SOURCES +=						\
	sndio/sndio-mixer.c						\
	sndio/sndio-mixer.h						\
	sndio/sndio-track.c						\
	sndio/sndio-track.h						\
	sndio/sndio-options.c						\
	sndio/sndio-options.h
endif

libxfce4mixer_la_CPPFLAGS = 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DDATADIR=\"$(datadir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DG_LOG_DOMAIN=\"libxfce4mixer\"				\
	-DMIXER_DATADIR=\"$(datadir)/xfce4/mixer\"

libxfce4mixer_la_CFLAGS = 						\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4UI_CFLAGS)						\
	$(XFCONF_CFLAGS)						\
	$(DBUS_GLIB_CFLAGS)						\
	$(GST_CFLAGS)

if ALSA_BACKEND
libxfce4mixer_la_CFLAGS +=						\
	-Ialsa								\
        $(ALSA_CFLAGS)
endif

if PULSE_BACKEND
libxfce4mixer_la_CFLAGS +=						\
	-Ipulse								\
        $(PULSE_CFLAGS)
endif

if OSS_BACKEND
libxfce4mixer_la_CFLAGS +=                                              \
        -Ioss
endif

if SNDIO_BACKEND
libxfce4mixer_la_CFLAGS +=                                              \
        -Isndio
endif
libxfce4mixer_la_LDFLAGS =						\
	-no-undefined

libxfce4mixer_la_LIBADD =						\
	$(PLATFORM_LDFLAGS)						\
	$(GLIB_LIBS)							\
	$(GTK_FLAGS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4UI_LIBS)						\
	$(XFCONF_LIBS)							\
	$(DBUS_GLIB_LIBS)						\
	$(GST_LIBS)

if ALSA_BACKEND
libxfce4mixer_la_LIBADD +=						\
        $(ALSA_LIBS)
endif

if PULSE_BACKEND
libxfce4mixer_la_LIBADD +=						\
        $(PULSE_LIBS)
endif

if SNDIO_BACKEND
libxfce4mixer_la_LIBADD +=						\
	-lsndio
endif

EXTRA_DIST = 								\
	meson.build							\
	$(NULL)

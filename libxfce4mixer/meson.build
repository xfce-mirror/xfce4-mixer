libxfce4mixer_sources = [
  'gst-mixer-options.c',
  'gst-mixer-options.h',
  'gst-mixer-track.c',
  'gst-mixer-track.h',
  'gst-mixer.c',
  'gst-mixer.h',
  'libxfce4mixer.c',
  'libxfce4mixer.h',
  'xfce-mixer-card-combo.c',
  'xfce-mixer-card-combo.h',
  'xfce-mixer-debug.c',
  'xfce-mixer-debug.h',
  'xfce-mixer-preferences.c',
  'xfce-mixer-preferences.h',
  'xfce-mixer-track-combo.c',
  'xfce-mixer-track-combo.h',
  'xfce-mixer-track-type.c',
  'xfce-mixer-track-type.h',
]

if have_alsa
  libxfce4mixer_sources += [
    'alsa' / 'alsa-mixer.c',
    'alsa' / 'alsa-mixer.h',
    'alsa' / 'alsa-options.c',
    'alsa' / 'alsa-options.h',
    'alsa' / 'alsa-track.c',
    'alsa' / 'alsa-track.h',
  ]
endif

if have_oss
  libxfce4mixer_sources += [
    'oss' / 'oss-mixer.c',
    'oss' / 'oss-mixer.h',
    'oss' / 'oss-options.c',
    'oss' / 'oss-options.h',
    'oss' / 'oss-track.c',
    'oss' / 'oss-track.h',
  ]
endif

if have_pulse
  libxfce4mixer_sources += [
    'pulse' / 'pulse-mixer.c',
    'pulse' / 'pulse-mixer.h',
    'pulse' / 'pulse-options.c',
    'pulse' / 'pulse-options.h',
    'pulse' / 'pulse-track.c',
    'pulse' / 'pulse-track.h',
  ]
endif

if have_sndio
  libxfce4mixer_sources += [
    'sndio' / 'sndio-mixer.c',
    'sndio' / 'sndio-mixer.h',
    'sndio' / 'sndio-options.c',
    'sndio' / 'sndio-options.h',
    'sndio' / 'sndio-track.c',
    'sndio' / 'sndio-track.h',
  ]
endif

libxfce4mixer = static_library(
  'xfce4mixer',
  libxfce4mixer_sources,
  c_args: [
    '-DMIXER_DATADIR="@0@"'.format(pkgdatadir),
    '-DG_LOG_DOMAIN="@0@"'.format('libxfce4mixer'),
  ],
  dependencies: [
    alsa,
    glib,
    gstreamer,
    gthread,
    gtk,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    pulse,
    sndio,
    xfconf,
  ],
  install: false,
)

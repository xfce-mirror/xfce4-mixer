xfce4_mixer_sources = [
  'main.c',
  'xfce-mixer-application.c',
  'xfce-mixer-application.h',
  'xfce-mixer-container.c',
  'xfce-mixer-container.h',
  'xfce-mixer-controls-dialog.c',
  'xfce-mixer-controls-dialog.h',
  'xfce-mixer-option.c',
  'xfce-mixer-option.h',
  'xfce-mixer-switch.c',
  'xfce-mixer-switch.h',
  'xfce-mixer-track.c',
  'xfce-mixer-track.h',
  'xfce-mixer-window.c',
  'xfce-mixer-window.h',
  xfce_revision_h,
]

executable(
  'xfce4-mixer',
  xfce4_mixer_sources,
  c_args: [
    '-DMIXER_DATADIR="@0@"'.format(pkgdatadir),
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-mixer'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gstreamer,
    gthread,
    gtk,
    libm,
    libxfce4ui,
    libxfce4util,
    xfconf,
  ],
  link_with: [
    libxfce4mixer,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)

i18n.merge_file(
    input: 'xfce4-mixer.desktop.in',
    output: 'xfce4-mixer.desktop',
    po_dir: '..' / 'po',
    type: 'desktop',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)

install_man('xfce4-mixer.1')

plugin_sources = [
  'xfce-mixer-plugin.c',
  'xfce-mixer-plugin.h',
  'xfce-plugin-dialog.c',
  'xfce-plugin-dialog.h',
  'xfce-volume-button.c',
  'xfce-volume-button.h',
  xfce_revision_h,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

shared_module(
  'mixer',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-mixer-plugin'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gstreamer,
    gtk,
    keybinder,
    libm,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    xfconf,
  ],
  link_with: [
    libxfce4mixer,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'mixer.desktop.in',
  output: 'mixer.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)

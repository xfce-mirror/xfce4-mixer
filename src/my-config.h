#ifndef __MIGRATE_XFCE4_MIXER_CONFIG_H
#define __MIGRATE_XFCE4_MIXER_CONFIG_H
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>

#define MIXER_CONFIG_SUBPATH "mixer/"

typedef enum {
	CONFIG_SAVE,
	CONFIG_LOAD
} ConfigAction;

gchar *get_config_path(gchar const *relpath, ConfigAction action);

#endif

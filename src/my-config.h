#ifndef __MIGRATE_XFCE4_MIXER_CONFIG_H
#define __MIGRATE_XFCE4_MIXER_CONFIG_H
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>

#define MIXER_CONFIG_SUBPATH "xfce4/mixer/"

typedef enum {
	CONFIG_SAVE,
	CONFIG_LOAD
} ConfigAction;

gchar *my_config_get_path(gchar const *relpath, ConfigAction action);
gchar *my_config_get_tmp_file_name(gchar const *origfilename);


#endif

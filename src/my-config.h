#ifndef __MIGRATE_XFCE4_MIXER_CONFIG_H
#define __MIGRATE_XFCE4_MIXER_CONFIG_H
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>

#define MIXER_CONFIG_SUBPATH "xfce4/mixer/"

typedef enum {
	CONFIG_SAVE,
	CONFIG_LOAD
} ConfigAction;

gchar *my_config_get_path(gchar const *relpath, ConfigAction action); /* direct */

/* with temp file */
gchar *my_config_get_temp_file_name(gchar const *origfilename);
int my_config_commit_file(gchar const *origfilename);

#endif

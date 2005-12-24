#ifndef __PANEL_LAUNCHER_ENTRY_H
#define __PANEL_LAUNCHER_ENTRY_H
#include <gtk/gtk.h>

/* copied from xfce4-panel launcher plugin as-is, added set_command and get_widget, get_command */

/* non-gobject */
typedef struct _LauncherEntry LauncherEntry;

LauncherEntry * launcher_entry_new (void);
void launcher_entry_free (LauncherEntry *e);
void launcher_entry_set_command (LauncherEntry *e, gchar const* command, gboolean terminal, gboolean startupnotification);
void launcher_entry_get_command (LauncherEntry *e, gchar** command, gboolean* terminal, gboolean* startupnotification);
GtkWidget* launcher_entry_get_widget (LauncherEntry* e);
/*void launcher_entry_execute (LauncherEntry *entry);*/

#endif

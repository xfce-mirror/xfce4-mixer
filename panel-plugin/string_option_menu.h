#ifndef _STRING_OPTION_MENU_H
#define _STRING_OPTION_MENU_H

#include <gtk/gtk.h>

GtkWidget *create_string_option_menu (GList *strings);
gchar *string_option_menu_get_selected(GtkWidget *w);
void string_option_menu_set_selected(GtkWidget *w, gchar const *which);

#endif

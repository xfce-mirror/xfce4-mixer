#ifndef _STRING_OPTION_MENU_H
#define _STRING_OPTION_MENU_H

#include <gtk/gtk.h>

GtkWidget *create_string_option_menu (GList *strings);
void fill_string_option_menu (GtkOptionMenu *om, GList *strings);
typedef gchar *(*FSOFunc)(gchar const *);
void fill_string_option_menu_2 (GtkOptionMenu *om, GList *strings, FSOFunc func);
gchar *string_option_menu_get_selected(GtkOptionMenu *w);
void string_option_menu_set_selected(GtkWidget *w, gchar const *which);

#endif

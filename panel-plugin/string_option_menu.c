#include <gtk/gtk.h>

GtkWidget *create_string_option_menu (GList *strings)
{
	GtkOptionMenu		*om;
	GtkWidget		*w;
	
	GtkMenu			*menu;
	GtkWidget		*it;
	
	w = gtk_option_menu_new ();
	
	om = GTK_OPTION_MENU (w);

	menu = GTK_MENU (gtk_menu_new ());

	while (strings) {
		it = gtk_menu_item_new_with_label ((gchar *)strings->data);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), it);
		gtk_widget_show (it);
		
		strings = g_list_next (strings);
	}

	gtk_option_menu_set_menu (om, GTK_WIDGET (menu));
	/*g_object_unref (menu); *//* FIXME is that right? */

	return w;
}

gchar *string_option_menu_get_selected(GtkWidget *w)
{
	GtkOptionMenu		*om;
	GtkMenu			*menu;
	GtkLabel		*label;
	gchar			*txt;
	
	om = GTK_OPTION_MENU (w);

	menu = GTK_MENU (gtk_option_menu_get_menu (om));

	label = GTK_LABEL (gtk_bin_get_child(GTK_BIN(om)));
	
	txt = g_strdup (gtk_label_get_text (label));
	
	return txt;
}

void string_option_menu_set_selected(GtkWidget *w, gchar const *which)
{
	/* TODO */
}

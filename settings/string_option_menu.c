#include <gtk/gtk.h>

void fill_string_option_menu (GtkOptionMenu *om, GList *strings)
{
	GtkMenu			*menu;
	GtkWidget		*it;
	
	menu = GTK_MENU (gtk_menu_new ());

	while (strings) {
		it = gtk_menu_item_new_with_label ((gchar *)strings->data);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), it);
		gtk_widget_show (it);
		
		strings = g_list_next (strings);
	}

	gtk_option_menu_set_menu (om, GTK_WIDGET (menu));
	/*g_object_unref (menu); *//* FIXME is that right? */
}

GtkWidget *create_string_option_menu (GList *strings)
{
	GtkOptionMenu		*om;
	GtkWidget		*w;
	
	w = gtk_option_menu_new ();
	
	om = GTK_OPTION_MENU (w);
	fill_string_option_menu (om, strings);

	return w;
}

gchar *string_option_menu_get_selected(GtkOptionMenu *w)
{
	GtkOptionMenu		*om;
	GtkMenu			*menu;
	GtkLabel		*label;
	gchar			*txt;
	
	/*om = GTK_OPTION_MENU (w);*/
	om = w;

	menu = GTK_MENU (gtk_option_menu_get_menu (om));

	label = GTK_LABEL (gtk_bin_get_child(GTK_BIN(om)));
	
	txt = g_strdup (gtk_label_get_text (label));
	
	return txt;
}

void string_option_menu_set_selected(GtkOptionMenu *w, gchar const *which)
{
	/* TODO */
}

typedef gchar *(*FSOFunc)(gchar const *);
void fill_string_option_menu_2 (GtkOptionMenu *om, GList *strings, FSOFunc func)
{
	GtkMenu			*menu;
	GtkWidget		*it;
	gchar	*s;
	gchar const *sc;
	
	menu = GTK_MENU (gtk_menu_new ());

	while (strings) {
		sc = (gchar const *)strings->data;
		if (func) 
			s = func (sc);
		else
			s = g_strdup (sc);
		if (s) {
			it = gtk_menu_item_new_with_label (s);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), it);
			gtk_widget_show (it);
			g_free (s);
		}
		
		strings = g_list_next (strings);
	}

	gtk_option_menu_set_menu (om, GTK_WIDGET (menu));
	/*g_object_unref (menu); *//* FIXME is that right? */
}

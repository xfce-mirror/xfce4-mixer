#include <gtk/gtk.h>
#include <libxfce4util/i18n.h>
#include "xfce-mixer-info.h"
#include "xfce-mixer-profiledlg.h"
#include "menu-callbacks.inc"

static GtkItemFactoryEntry menubar_items[] =
{
{"/_File", NULL, NULL, 0, "<Branch>"},
{"/File/_Options", "<control>o", options_activate_cb, 0, "<Item>"},
{"/File/_Exit", "<control>x", appexit_activate_cb, 0, "<Item>"},
{"/_View", NULL, NULL, 0, "<Branch>"},
{"/View/sep", NULL, NULL, 0, "<Separator>"},
{"/View/_Manage", "<control>p", profile_mgr_activate_cb, 0, "<Item>"},
{"/_Help", NULL, NULL, 0, "<Branch>"},
{"/Help/_Info", NULL, info_activate_cb, 0, "<Item>"}
};

static const char *
translate_menu (const char *msg, gpointer data)
{
	return dgettext (GETTEXT_PACKAGE, msg);
} 

GtkMenuBar *
xfce_mixer_create_main_menu (GtkWindow *win, GtkAccelGroup *accel_group)
{
	GtkMenuBar *menu;
	GtkItemFactory *ifactory;
	
	ifactory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<menu>", accel_group);
	gtk_item_factory_set_translate_func (
		ifactory,
		(GtkTranslateFunc) translate_menu, 
		NULL, NULL
	);
	
	gtk_item_factory_create_items (ifactory, G_N_ELEMENTS (menubar_items),
	                                       menubar_items, win);

	menu = GTK_MENU_BAR (gtk_item_factory_get_widget (ifactory, "<menu>"));

	return menu;
}

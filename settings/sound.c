#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libxfce4mcs/mcs-manager.h>
#include <libxfce4util/debug.h>
#include <libxfce4util/i18n.h> 
#include <libxfce4util/util.h> 
#include <libxfcegui4/libxfcegui4.h>
#include <xfce-mcs-manager/manager-plugin.h>

#include "sound-icon.h"

static void     run_dialog(McsPlugin *);
static gboolean save_settings(McsPlugin *);

static GtkWidget *dialog = NULL;

#define RCDIR	"settings"
#define CHANNEL	"sound"
#define RCFILE	"sound.xml"

static void
response_cb (McsPlugin *plugin)
{
	save_settings (plugin);
	gtk_widget_destroy (GTK_WIDGET (dialog));
	
	dialog = NULL;
}

/* settings */

/* 
  per device:
    to-be-visible control vcnames
*/

McsPluginInitResult
mcs_plugin_init(McsPlugin *plugin)
{
	gchar *file;
	
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
	
	file = xfce_get_userfile(RCDIR, RCFILE, NULL);
	mcs_manager_add_channel_from_file(plugin->manager, CHANNEL, file);
	g_free(file);

	plugin->plugin_name = g_strdup ("sound");
	plugin->caption = g_strdup ( _("Sound"));
	plugin->run_dialog = run_dialog;
	plugin->icon = inline_icon_at_size(sound_icon_data, 48, 48);
	return MCS_PLUGIN_INIT_OK;
}


static void     run_dialog(McsPlugin *plugin)
{
	if (dialog) {
		gtk_window_present (GTK_WINDOW (dialog));
		return;
	}
	
	dialog = gtk_dialog_new_with_buttons(_("Sound"), NULL,
                         GTK_DIALOG_NO_SEPARATOR,
                         GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
			NULL);
                                                                         

	gtk_window_set_icon (GTK_WINDOW (dialog), plugin->icon);
	gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	
	g_signal_connect_swapped(dialog, "response", G_CALLBACK(response_cb), plugin);
	g_signal_connect_swapped(dialog, "delete-event",G_CALLBACK(response_cb), plugin);

	gtk_widget_show_all (GTK_WIDGET (dialog));
}

static gboolean save_settings(McsPlugin *plugin)
{
}


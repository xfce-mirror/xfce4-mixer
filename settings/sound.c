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
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <xfce-mcs-manager/manager-plugin.h>
#include "xfce-mixer-settingsbox.h"
#include "vcs.h"

static void     run_dialog(McsPlugin *);
static gboolean save_settings(McsPlugin *);

static GtkWidget *dialog = NULL;
static XfceMixerSettingsbox *sb = NULL;

#define RCDIR	"settings"
#define CHANNEL	"sound"
#define RCFILE	"sound.xml"

static void
response_cb (McsPlugin *plugin, gpointer user_data)
{
	if (sb)
		xfce_mixer_settingsbox_save (sb);
		
	save_settings (plugin);
	gtk_widget_destroy (GTK_WIDGET (dialog));
	
	dialog = NULL;
	sb = NULL;
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
	plugin->icon = xfce_themed_icon_load ("xfce4-mixer", 48);
	
	register_vcs ();

	return MCS_PLUGIN_INIT_OK;
}


static void     run_dialog(McsPlugin *plugin)
{
	GtkWidget *header;	
	if (dialog) {
		gtk_window_present (GTK_WINDOW (dialog));
		return;
	}
	
	dialog = gtk_dialog_new_with_buttons(_("Sound"), NULL,
                         GTK_DIALOG_NO_SEPARATOR,
                         GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
			NULL);

	sb = xfce_mixer_settingsbox_new ();
	gtk_widget_show (GTK_WIDGET (sb));
	header = xfce_create_header (plugin->icon, _("Sound"));
	gtk_widget_show (header);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), GTK_WIDGET (header), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), GTK_WIDGET (sb), TRUE, FALSE, 0);

	gtk_window_set_icon (GTK_WINDOW (dialog), plugin->icon);
	gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	
	g_signal_connect_swapped(dialog, "response", G_CALLBACK(response_cb), plugin);
	g_signal_connect_swapped(dialog, "destroy",G_CALLBACK(response_cb), plugin);
	
	sb->manager = plugin->manager;
	xfce_mixer_settingsbox_load (sb);

	gtk_widget_show (GTK_WIDGET (dialog));
}

static gboolean save_settings(McsPlugin *plugin)
{
	gboolean result;
	gchar *file;

	if (!dialog || !sb)
		return TRUE;
		
	file = xfce_get_userfile(RCDIR, RCFILE, NULL);
	result = mcs_manager_save_channel_to_file(plugin->manager, CHANNEL,
	  file);
	g_free(file);

	if (sb)
		sb->manager = NULL;

	return(result);
}


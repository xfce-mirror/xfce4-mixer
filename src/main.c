#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include "xfce-mixer-profile.h"
#include "xfce-mixer-window.h"
#include "vcs.h"
#include "main.h"
#include "mcs_client.h"

GtkTooltips *tooltips;

static GtkWidget *mixer_window;

gboolean
delayed_refresh_cb (gpointer user_data)
{
	if (mixer_window) {
		xfce_mixer_window_refresh (XFCE_MIXER_WINDOW (mixer_window));
		xfce_mixer_window_reset_profile (XFCE_MIXER_WINDOW (mixer_window));
	}
	return FALSE;
}

static void
my_main_quit(GtkWidget *w, gpointer user_data)
{
	mcs_stop_watch ();
	mixer_window = NULL;
	gtk_main_quit ();
}

int main(int argc, char * argv[])
{

	register_vcs ();

	gtk_init (&argc, &argv);
	
	tooltips = gtk_tooltips_new ();
	
	if (argc >=2 && argv[1][0] != '-') {
		vc_set_device (argv[1]);
	}

	mixer_window = xfce_mixer_window_new ();
	xfce_mixer_profile_fill_defaults (XFCE_MIXER_WINDOW (mixer_window)->profile);
	
	g_signal_connect (G_OBJECT (mixer_window), "destroy", G_CALLBACK (my_main_quit), NULL);
	gtk_widget_show (GTK_WIDGET (mixer_window));

	mcs_watch_xfce_channel ();
		
	gtk_main ();
	return 0;
}

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include <libxfce4util/i18n.h>
#include "xfce-mixer-profile.h"
#include "xfce-mixer-window.h"
#include "xfce-mixer-mcs-client.h"
#include "vcs.h"
#include "main.h"

GtkTooltips *tooltips;

static GtkWidget *mixer_window;
XfceMixerMcsClient *mcsc = NULL;

static void
my_main_quit(GtkWidget *w, gpointer user_data)
{
	mixer_window = NULL;
	gtk_main_quit ();
}

int main(int argc, char * argv[])
{
	int rc;
	rc = register_vcs ();
	if (rc < -1) {
		g_warning (_ ("No working sound"));
	}

	gtk_init (&argc, &argv);
	
	tooltips = gtk_tooltips_new ();
	
	if (argc >=2 && argv[1][0] != '-') {
		vc_set_device (argv[1]);
	}

	mixer_window = xfce_mixer_window_new ();
	mcsc = XFCE_MIXER_WINDOW (mixer_window)->mcsc;
	xfce_mixer_profile_fill_defaults (XFCE_MIXER_WINDOW (mixer_window)->profile);
	
	g_signal_connect (G_OBJECT (mixer_window), "destroy", G_CALLBACK (my_main_quit), NULL);
	gtk_widget_show (GTK_WIDGET (mixer_window));

	gtk_main ();
	return 0;
}

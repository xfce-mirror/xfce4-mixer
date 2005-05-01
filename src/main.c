#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define UPDATE_TIMEOUT 1000

#include <gtk/gtk.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <libxfce4util/libxfce4util.h>
#include "xfce-mixer-profile.h"
#include "xfce-mixer-profiles.h"
#include "xfce-mixer-window.h"
#include "xfce-mixer-mcs-client.h"
#include "xfce-mixer-cache-vc.h"
#include "mixer-mcs-names.h"
#include "vcs.h"
#include "main.h"

GtkTooltips *tooltips;

static GtkWidget *mixer_window;
XfceMixerMcsClient *mcsc = NULL;
gchar *device = NULL;
XfceMixerProfiles *profiles = NULL;
static guint src;


static void vol_changed_cb (char const *vcname, void *privdata)
{
	if (mixer_window) {
		xfce_mixer_window_refresh_value (XFCE_MIXER_WINDOW (mixer_window), vcname);
	}
}

static gboolean
timer_vc_cb (gpointer data)
{
	vc_handle_events ();
	vol_changed_cb (NULL, NULL);
	return TRUE;
}

static void
my_main_quit(GtkWidget *w, gpointer user_data)
{
	xfce_mixer_profiles_save (profiles);
	g_object_unref (G_OBJECT (profiles));
	profiles = NULL;
	
	g_source_remove (src);
	
	mixer_window = NULL;

	g_free (device);
	xfce_mixer_cache_vc_free ();

	gtk_main_quit ();
}

static void 
select_default_device()
{
	XfceMixerMcsClient *mcs;
	gchar	*value;
	
	mcs = xfce_mixer_mcs_client_new ();
	value = xfce_mixer_mcs_client_get_setting (mcs, MIXER_DEFAULT_DEVICE_FIELD);
	if (value) {
		vc_set_device (value);
		g_free (value);
	}
}

int main(int argc, char * argv[])
{
	int rc;
	gchar const *dd;
	gboolean versionOnly;
	versionOnly = FALSE;

	if (argc > 1 && (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-V"))) {
		versionOnly = TRUE;

		g_print ("%s version %s using Xfce %s\n", 
			PACKAGE, VERSION,  xfce_version_string());
	}
	
	rc = register_vcs ();
	if (rc < -1) {
		g_warning (_ ("No working sound"));
	}

	if (versionOnly == TRUE) {
		if (selected_vc() != NULL)
			g_print ("  audio vc: %s\n", selected_vc()->name);
		else
			g_print ("  audio vc: none\n");
		exit (0);
	}

	gtk_init (&argc, &argv);

	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

	tooltips = gtk_tooltips_new ();
	
	if (argc >=2 && argv[1][0] != '-') {
		vc_set_device (argv[1]);
		device = g_strdup (argv[1]);
	} else {
		select_default_device ();
		
		dd = vc_get_device ();
		if (dd)
			device = g_strdup (dd);
		else
			device = NULL;
	}
	
  	profiles = xfce_mixer_profiles_new (device);
	xfce_mixer_profiles_load (profiles);
  
	mixer_window = xfce_mixer_window_new ();
	mcsc = XFCE_MIXER_WINDOW (mixer_window)->mcsc;
	xfce_mixer_profile_fill_defaults (XFCE_MIXER_WINDOW (mixer_window)->profile);

	vc_set_volume_callback (vol_changed_cb, mixer_window);
	
	g_signal_connect (G_OBJECT (mixer_window), "destroy", G_CALLBACK (my_main_quit), NULL);
	gtk_widget_show (GTK_WIDGET (mixer_window));
	
	src = g_timeout_add (UPDATE_TIMEOUT, (GSourceFunc) timer_vc_cb, NULL);

	gtk_main ();
	
	
	return 0;
}

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>  
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>
#include "xfce-mixer-slider-tiny.h"
#include "xfce-mixer-prefbox.h"
#include "xfce-mixer-preferences.h"
#include "xfce-mixer-control-vc.h"
#include "vcs.h"

/* DO: timeout -> update volume */
#define UPDATE_TIMEOUT 1000

typedef struct
{
	GtkWidget *box;
	XfceMixerControl *slider;
	XfceMixerPreferences *prefs;
	XfceIconbutton *ib;
	XfceMixerPrefbox *pb;
	gboolean broken;
	guint timer;
} t_mixer;

GtkTooltips *tooltips = NULL;

static void
swap_pixbuf_ptrs (GdkPixbuf **a, GdkPixbuf **b)
{
	GdkPixbuf *tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}
                                
static GdkPixbuf *
get_pixbuf_for(gboolean broken)
{
	GdkPixbuf	*pb;
	GdkPixbuf	*pb2;
	
	pb = get_pixbuf_by_id(SOUND_ICON);
	if (broken) {
		pb2 = gdk_pixbuf_copy(pb);
		gdk_pixbuf_saturate_and_pixelate(pb, pb2, 0, TRUE);

		/*saturation, pixelate)*/
		swap_pixbuf_ptrs(&pb, &pb2);

		g_object_unref(pb2);
	}
	return pb;
}

static void
xfce_mixer_launch_cb (GtkWidget *w, gpointer user_data)
{
	t_mixer *mixer;
	gchar *tmp;
	gboolean internal;
	
	mixer = (t_mixer *) user_data;
	if (!mixer || !mixer->prefs)
		return;
		
	internal = mixer->prefs->execu && 
	  g_str_has_prefix(mixer->prefs->execu, "xfce4-mixer");
	  
	if (mixer && mixer->prefs && mixer->prefs->device && internal) {
		tmp = g_strdup_printf ("xfce4-mixer \"%s\"", mixer->prefs->device); /* TODO: pass device from cfg */
	} else {
		if (mixer->prefs->execu)
			tmp = g_strdup (mixer->prefs->execu);
		else
			tmp = g_strdup ("xfce4-mixer");
	}
	/*g_spawn_command_line_async (tmp, NULL);*/
	exec_cmd(tmp, mixer->prefs->in_terminal, mixer->prefs->startup_nf);
	                        
	g_free (tmp);
}

static void
mixer_update_tips(t_mixer *mixer)
{
        gchar caption[128];
        
	g_snprintf(caption, sizeof(caption), _("Volume: %d%%"), 
	  xfce_mixer_control_calc_num_value (mixer->slider)
	);

	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->ib), caption, NULL);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(
		XFCE_MIXER_SLIDER_TINY (mixer->slider)->eb
	), caption,NULL);
}

static void
mixer_prefs_master_changed_cb (XfceMixerPreferences *prefs, gpointer whatsthat,  gpointer user_data)
{
	t_mixer *mixer;
	gchar *s;
	
	mixer = (t_mixer *) user_data;
	if (mixer && mixer->prefs) {
		g_object_get (G_OBJECT (mixer->prefs),
			"master", &s, NULL
		);
		
		g_object_set (G_OBJECT (mixer->slider), 
		"vcname", s,
		NULL
		);
		
		g_free (s);
		s = NULL;
		
		xfce_mixer_control_vc_feed_value (mixer->slider);
		mixer_update_tips (mixer);
	}
}

static void
mixer_value_changed_cb (GtkWidget *w, gpointer whatsthat, gpointer user_data)
{
	t_mixer *mixer;
	mixer = (t_mixer *) user_data;
	mixer_update_tips (mixer);
}

static gboolean
mixer_timer_cb (gpointer userdata)
{
	t_mixer *mixer;
	mixer = (t_mixer *) userdata;
	
	vc_handle_events ();

	xfce_mixer_control_vc_feed_value (mixer->slider);
	mixer_update_tips (mixer);
	
	return TRUE;
}

static void callback_vc_cb(char const *which, void *privdata)
{
	t_mixer *mixer;
	mixer = (t_mixer *) privdata;
	
	xfce_mixer_control_vc_feed_value (mixer->slider);
	mixer_update_tips (mixer);
}

static t_mixer *
mixer_new(void)
{
	t_mixer *mixer;
	GdkPixbuf *pb;
	GtkWidget *align;
	
	mixer = g_new0 (t_mixer, 1);

	mixer->prefs = XFCE_MIXER_PREFERENCES (xfce_mixer_preferences_new ());

	mixer->box = gtk_hbox_new (FALSE, 0);
	mixer->broken = FALSE; 
	gtk_widget_show (mixer->box);

	pb = get_pixbuf_for (mixer->broken);
	mixer->ib = (XfceIconbutton *)xfce_iconbutton_new_from_pixbuf (pb);
	g_object_unref (pb);
	gtk_button_set_relief (GTK_BUTTON(mixer->ib), GTK_RELIEF_NONE);
	gtk_widget_show (GTK_WIDGET (mixer->ib));
	
	g_signal_connect (G_OBJECT (mixer->ib), 
		"clicked", G_CALLBACK (xfce_mixer_launch_cb), mixer);

	gtk_box_pack_start (GTK_BOX (mixer->box), GTK_WIDGET (mixer->ib), TRUE, TRUE, 0);

	mixer->slider = XFCE_MIXER_CONTROL (xfce_mixer_slider_tiny_new ());
	g_signal_connect (
		G_OBJECT (mixer->slider), "notify::value", 
		G_CALLBACK (mixer_value_changed_cb), mixer
	);
	
	gtk_widget_show (GTK_WIDGET (mixer->slider));
	gtk_box_pack_start (GTK_BOX (mixer->box), GTK_WIDGET (mixer->slider), FALSE, TRUE, 0);
		
	g_signal_connect_swapped (
		G_OBJECT (mixer->ib), "scroll-event", 
		G_CALLBACK (xfce_mixer_slider_tiny_scroll_cb), 
		mixer->slider
	);

	align = gtk_alignment_new (0,0,0,0);
	gtk_widget_show (align);
	gtk_widget_set_size_request (align, border_width, -1);
	gtk_box_pack_start (GTK_BOX (mixer->box), align, FALSE, FALSE, 0);	
	

	g_signal_connect (G_OBJECT (mixer->prefs), "notify::master",
		G_CALLBACK (mixer_prefs_master_changed_cb), mixer 
	);
	
	mixer_prefs_master_changed_cb (mixer->prefs, NULL, mixer);

	xfce_mixer_control_vc_feed_value (mixer->slider);
	xfce_mixer_control_vc_attach (mixer->slider);
	mixer_update_tips (mixer);

	mixer->timer = g_timeout_add (UPDATE_TIMEOUT, 
		(GSourceFunc) mixer_timer_cb, 
		mixer
	);
	
	vc_set_volume_callback (callback_vc_cb, (void *) mixer);

	return mixer;
}

static gboolean
mixer_control_new (Control *ctrl)
{
	t_mixer *mixer;
	mixer = mixer_new ();
	gtk_container_add (GTK_CONTAINER(ctrl->base), mixer->box);
	ctrl->data = (gpointer) mixer;
	ctrl->with_popup = FALSE;

	gtk_widget_set_size_request (ctrl->base, -1, -1);

	return TRUE;	
}

static void
mixer_control_free (Control *ctrl)
{
	t_mixer *mixer;
	g_return_if_fail(ctrl != NULL);
	g_return_if_fail(ctrl->data != NULL);
	vc_set_volume_callback (NULL, NULL);

	mixer = (t_mixer *)ctrl->data;
	if (mixer) {
		if (mixer->timer) {
			g_source_remove (mixer->timer);
			mixer->timer = 0;
		}
		if (mixer->prefs)
			g_object_unref (G_OBJECT (mixer->prefs));
		mixer->prefs = NULL;
	}
	g_free (mixer);
}

static void
mixer_read_config (Control *ctrl, xmlNodePtr parent)
{
	t_mixer *mixer;
	mixer = (t_mixer *) ctrl->data;
	if (!mixer->prefs)
		return;
		
	xfce_mixer_preferences_load (mixer->prefs, parent);
}

static void
mixer_write_config (Control *ctrl, xmlNodePtr parent)
{
	t_mixer *mixer;
	mixer = (t_mixer *) ctrl->data;
	
	if (!mixer->prefs)
		return;
		
	xfce_mixer_preferences_save (mixer->prefs, parent);
}

static void
mixer_attach_callback(Control *ctrl, const gchar *signal, GCallback cb,
                gpointer data)
{
	t_mixer *mixer;

	mixer = (t_mixer *) ctrl->data;
	
	g_signal_connect(mixer->ib, signal, cb, data);
	g_signal_connect(mixer->slider, signal, cb, data);
}

/*#define BIG_ICON*/

static void
mixer_set_size(Control *ctrl, int size)
{
	t_mixer	*mixer = (t_mixer *)ctrl->data;
	
	int	slider_width;
#ifndef BIG_ICON
	int	all;
	int	r;
#endif
	
	/* size: 0..3(HUGE), with 4- taken as huge too */

	slider_width = 6 + 2 * size;
	
	if (slider_width < 0) slider_width = 1;
#ifdef BIG_ICON
	gtk_widget_set_size_request(GTK_WIDGET(mixer->ib), icon_size[size], icon_size[size]);
	gtk_widget_set_size_request(GTK_WIDGET(mixer->slider), 6 + 2 * size, icon_size[size]);
#else
	all = icon_size[size];
	r = all - slider_width;
	if (r < 0) r = 1;
		
	gtk_widget_set_size_request(GTK_WIDGET(mixer->ib), r, r);
	gtk_widget_set_size_request(GTK_WIDGET(mixer->slider), 6 + 2 * size, r /*icon_size[size]*/);
#endif

	gtk_widget_queue_resize (GTK_WIDGET (mixer->slider));
}

static void
mixer_prefs_ok_cb (GtkWidget *w, gpointer user_data)
{
	t_mixer *mixer = (t_mixer *) user_data;
	xfce_mixer_prefbox_save_preferences (mixer->pb, mixer->prefs);
}


static void
mixer_create_options (Control *ctrl, GtkContainer *con, GtkWidget *done)
{
	t_mixer	*mixer = (t_mixer *)ctrl->data;

	GtkWidget *w;
	XfceMixerPrefbox *pb;
	w = xfce_mixer_prefbox_new (ctrl);
	gtk_widget_show (w);
	gtk_container_add (GTK_CONTAINER (con), w);

	pb = XFCE_MIXER_PREFBOX (w);
	mixer->pb = pb;
	xfce_mixer_prefbox_fill_defaults (pb);
	
	xfce_mixer_prefbox_fill_preferences (pb, mixer->prefs);
	g_signal_connect (G_OBJECT (done), "clicked", G_CALLBACK (mixer_prefs_ok_cb), mixer);
}

static void
mixer_set_theme(Control * control, const char *theme)
{
	GdkPixbuf	*pb;
	t_mixer		*mixer;
	
	mixer = (t_mixer *)control->data;
	
	pb = get_pixbuf_for(mixer->broken);
	xfce_iconbutton_set_pixbuf(XFCE_ICONBUTTON(mixer->ib), pb);
	g_object_unref(pb);
}

/* initialization */
G_MODULE_EXPORT void
xfce_control_class_init(ControlClass *cc)
{
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

	if (!tooltips)
		tooltips = gtk_tooltips_new ();
	register_vcs ();

	/* these are required */
	cc->name                = "mixer";
	cc->caption             = _("Volume Control");
	cc->create_control      = (CreateControlFunc)mixer_control_new;
	cc->free                = mixer_control_free;
	cc->attach_callback     = mixer_attach_callback;

	/* options; don't define if you don't have any ;) */
	cc->read_config         = mixer_read_config;
	cc->write_config        = mixer_write_config;
	cc->create_options      = mixer_create_options;

	/* Don't use this function at all if you want xfce to
	 * do the sizing.
	 * Just define the set_size function to NULL, or rather, don't
         * set it to something else.
         */
	cc->set_size            = mixer_set_size;
	cc->set_theme		= mixer_set_theme;

	/* unused in the sample:
	 * ->set_orientation
	 */
	 
	control_class_set_unique (cc, TRUE);
}

/* required! defined in panel/plugins.h */
XFCE_PLUGIN_CHECK_INIT

/* this is the mixer window which pops up when clicking the panel plugin */

#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "vc.h"
#include "mixer_window.h"

void change_vol_cb(GtkRange *range, gpointer data)
{
	mixer_slider_control_t *s;
	int			vol;
	
	s = (mixer_slider_control_t *) data;
	 
	vol = (int)gtk_range_get_value(range);
	
	set_volume(s->name, vol);
}

static gchar*
format_value_callback (GtkScale *scale, gdouble   value)
{
	return g_strdup_printf ("%d %%", (int)value);
}

void mixer_window_slider_control_refresh_value_p (mixer_window_t *w, mixer_slider_control_t *c)
{
	int	vol;
	if (c) {
		vol = get_volume (c->name);
		gtk_range_set_value (GTK_RANGE (c->scale), (double) vol);
	}
}

void mixer_window_slider_control_refresh_value (mixer_window_t *w, char const *name)
{
	mixer_slider_control_t *c = w->controls;
	while (c) {
		if (!strcmp(c->name, name)) {
			mixer_window_slider_control_refresh_value_p (w, c);
		}
		c = c->next;
	}
}

mixer_slider_control_t *mixer_window_slider_control_new(mixer_window_t *w, char const *name)
{
	mixer_slider_control_t *s;
	s = g_new0 (mixer_slider_control_t, 1);
	if (s) {
		s->name = g_strdup(name);
	
		s->vbox = GTK_BOX (gtk_vbox_new (FALSE, 5));
		
		s->hbox = GTK_BOX (gtk_hbox_new (FALSE, 3));
		gtk_widget_show (GTK_WIDGET(s->hbox));

		s->scale = GTK_SCALE (gtk_vscale_new_with_range (0.0, 100.0, 1.0));

		gtk_scale_set_digits (GTK_SCALE(s->scale), 0);
		/*g_signal_connect (GTK_WIDGET(s->scale), "format-value", G_CALLBACK (format_value_callback), NULL);*/

		g_signal_connect (GTK_WIDGET(s->scale), "value-changed", G_CALLBACK (change_vol_cb), (gpointer) s);
		
		gtk_widget_set_size_request (GTK_WIDGET (s->scale), -1, 120);
		
		gtk_range_set_inverted (GTK_RANGE (s->scale), TRUE);
		
		gtk_widget_show (GTK_WIDGET (s->scale));
		
		s->label = GTK_BUTTON (gtk_button_new_with_label (name));
		gtk_button_set_relief (GTK_BUTTON (s->label), GTK_RELIEF_NONE);
		
		gtk_widget_show (GTK_WIDGET (s->label));
		
		gtk_box_pack_start (GTK_BOX (s->vbox), GTK_WIDGET (s->label), TRUE, TRUE, 3);
		gtk_box_pack_start (GTK_BOX (s->vbox), GTK_WIDGET (s->hbox), TRUE, FALSE, 3);
		
		gtk_box_pack_start (GTK_BOX (s->hbox), GTK_WIDGET (s->scale), TRUE, FALSE, 3);

		gtk_box_pack_start (GTK_BOX (w->hbox), GTK_WIDGET (s->vbox), TRUE, FALSE, 3);

		/* put into linked list: */
		s->prev = w->last_control;
		s->next = NULL;
		if (w->last_control) w->last_control->next = s;
		if (!w->controls) w->controls = s;
		w->last_control = s;
		
		
		mixer_window_slider_control_refresh_value_p (w, s);
	}
	return s;	
}

void show_slider(mixer_slider_control_t *s)
{
	gtk_widget_show (GTK_WIDGET (s->vbox));
}


void control_glist_foreach_cb(gpointer data, gpointer user_data)
{
	volcontrol_t *item = (volcontrol_t *)data;
	mixer_window_t *w = (mixer_window_t *)user_data;
	
	if (item) {
		show_slider (mixer_window_slider_control_new (w, item->name));
	}
}

mixer_window_t *mixer_window_new()
{
	GList *g;
	mixer_window_t * w;
	
	w = g_new0 (mixer_window_t, 1);
	
	if (w) {
		w->window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
		w->hbox = GTK_BOX (gtk_hbox_new (FALSE /* TRUE */, 5));
		gtk_widget_show (GTK_WIDGET (w->hbox));
		
		gtk_container_add (GTK_CONTAINER (w->window), GTK_WIDGET (w->hbox));
		
		w->controls = NULL;
		w->last_control = NULL;


		/* get the controls and fill them into the window */

		g = get_control_list();
		if (g) {
			g_list_foreach(g, control_glist_foreach_cb, w);
			free_control_list (g);
		}
	}
	
	return w;
}




#if 0
static int find_working_sound(void)
{
/*#ifdef USE_OSS*/
/*	extern int register_oss(void);*/
/*#endif*/
/*#ifdef USE_ALSA*/
	extern int register_alsa(void);
	extern int register_alsa_fallback(void);
/*#endif*/

	volchanger_t	**v;

	/* register desired sound system */
/*#ifdef USE_OSS*/
/*	register_oss();*/
/*#endif*/
/*#ifdef USE_ALSA*/
	register_alsa();
#if 0
	register_alsa_fallback();
#endif
/*#endif*/

	v = first_vc();
	while (v) {
		if ((*v)->reinit_device && (*(*v)->reinit_device)() == 0) {
			/* works */
			select_vc_direct(*v);
			return 0;
		}
		v = next_vc(v);
	}

	g_warning(("No working sound"));
	
	return -1;
}

int main(int argc, char *argv[])
{
	GList *g;
	
	gtk_set_locale ();
	gtk_init (&argc, &argv);

	if ( find_working_sound() == -1) {
		return 1;
	}

	mixer_window_t * w = mixer_window_new();
	
	g = get_control_list();
	if (g) g_list_foreach(g, control_glist_foreach_cb, w);

	
/*	show_slider (mixer_window_slider_control_new (w));
	show_slider (mixer_window_slider_control_new (w));
	show_slider (mixer_window_slider_control_new (w));
	show_slider (mixer_window_slider_control_new (w));
*/

	gtk_widget_show (GTK_WIDGET (w->window));


	gtk_main ();
	
	return 0;
}

#endif

#include <gtk/gtk.h>
#include "xfce_sizehook.h"

struct t_sizehook_priv {
	GCallback cb;
	gpointer cbdata;
	gboolean cbswapped;
	GDestroyNotify df;
};

static void
map_cb(GtkWindow *w, t_window_state *s);


static gboolean 
window_state_event_cb (GtkWidget *widget, GdkEvent *event, t_window_state *s)
{
	if (event->window_state.type == GDK_WINDOW_STATE) {
		s->sticky = event->window_state.new_window_state & GDK_WINDOW_STATE_STICKY;
	}
	return FALSE;
}

static gboolean
delete_event_cb (GtkWindow *w, GdkEvent *event, t_window_state *s)
{
	if (!s)
		return FALSE;

	gtk_window_get_size (w, &s->width, &s->height);
	gtk_window_get_position (w, &s->x, &s->y);
	s->valid = TRUE;

	if (!s->priv)
		return FALSE;
	
	if (s->priv->cb) {
		if (s->priv->cbswapped) {
			((XfceSizehookCallback)s->priv->cb) (w, s, s->priv->cbdata);
		} else {
			((XfceSizehookCallback)s->priv->cb) ((GtkWindow *)s->priv->cbdata, s, (gpointer) w);
		}
	}
	
	if (s->priv->df) {
		s->priv->df(s->priv->cbdata);
		s->priv->cbdata = NULL;
	}

	g_free (s->priv);
	s->priv = NULL;
	return FALSE;
}

static void
map_cb(GtkWindow *w, t_window_state *s)
{
	if (!s)
		return;

	if (s->valid) {
		if (s->width > 0 && s->height > 0)
			gtk_window_resize (w, s->width, s->height);

		gtk_window_move (w, s->x, s->y);
		
		if (s->sticky)
			gtk_window_stick (w);
		else
			gtk_window_unstick (w);
	}
}

void xfce_hook_window_state_full(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback cb, gpointer cbdata, 
	GDestroyNotify destroyer)
{
	struct t_sizehook_priv *priv;
	if (!s || s->priv || !w)
		return;
		
	priv = g_new0 (struct t_sizehook_priv, 1);

	s->priv = priv;
	
	priv->cb = G_CALLBACK (cb);
	priv->cbdata = cbdata;
	priv->df = destroyer;
	priv->cbswapped = FALSE;
	
	g_signal_connect (G_OBJECT (w), "delete-event", 
		G_CALLBACK (delete_event_cb), s);
/*	g_signal_connect (G_OBJECT (w), "map", 
		G_CALLBACK (map_cb), s);
*/
	g_signal_connect (G_OBJECT (w), "window-state-event", 
		G_CALLBACK (window_state_event_cb), s);

	map_cb (w, s);
}

void xfce_hook_window_state_full_swapped(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback cb, gpointer cbdata, 
	GDestroyNotify destroyer)
{
	xfce_hook_window_state_full (w, s, cb, cbdata, destroyer);
	if (s && s->priv)
		s->priv->cbswapped = TRUE;
}

void xfce_hook_window_state(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback cb, gpointer cbdata)
{
	xfce_hook_window_state_full (w, s, cb, cbdata, NULL);
}


void xfce_hook_window_state_swapped(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback cb, gpointer cbdata)
{
	xfce_hook_window_state_full_swapped (w, s, cb, cbdata, NULL);
}

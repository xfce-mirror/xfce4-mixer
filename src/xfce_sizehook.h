#ifndef __XFCE_SIZEHOOK_H
#define __XFCE_SIZEHOOK_H
#include <gtk/gtk.h>

/*
usage:

first create an instance of t_window_state (with g_new0(t_window_state, -1))

set width,height to -1
set sticky to FALSE

load saved position into the t_window_state

call one of xfce_hook_window_state*
The callback will be called to save the position (on window close).

*/

typedef struct {
	gboolean valid;
	gboolean sticky;
	gint x, y, width, height;
	/* gboolean maximized */
	/* gboolean iconic */
	/* icon_geometry */
	/* gravity */
	struct t_sizehook_priv *priv;
} t_window_state;

typedef void (*XfceSizehookCallback)(GtkWindow *w, t_window_state *s, gpointer userdata);

void xfce_hook_window_state(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback  cb, gpointer cbdata);

void xfce_hook_window_state_swapped(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback cb, gpointer cbdata);

void xfce_hook_window_state_full(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback  cb, gpointer cbdata, 
	GDestroyNotify destroyer);

void xfce_hook_window_state_full_swapped(GtkWindow *w, t_window_state *s,
	XfceSizehookCallback  cb, gpointer cbdata, 
	GDestroyNotify destroyer);

#endif

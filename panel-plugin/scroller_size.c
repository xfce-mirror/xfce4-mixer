#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <libxfcegui4/xinerama.h>
#include <libxfcegui4/netk-util.h>

#if GTK_MINOR_VERSION >= 2

typedef struct {
	double maxw; /* 1.0 = whole screen */
	double maxh; /* 1.0 = whole screen */
	GtkScrolledWindow *s; /* the scroller to set */
	gboolean autoh;
	gboolean autov;
} resize_gracefully_data_t;

static gboolean map_window_cb (GtkWidget *widget, gpointer d)
{
	gint	maxw, maxh;
	gint	w, h;
	gint	x, y;
	gint	ox, oy;
	
	GtkPolicyType	hpol, vpol;
	gboolean	clamp;
	resize_gracefully_data_t	*g;
	GdkScreen	*s;
	DesktopMargins	border;
	
	g = (resize_gracefully_data_t *)d;

	if (!g)
		return FALSE;
	
	s = gtk_widget_get_screen (widget);
	maxw = MyDisplayFullWidth (GDK_SCREEN_XDISPLAY (s), GDK_SCREEN_XNUMBER (s)) * g->maxw;
	maxh = MyDisplayFullHeight (GDK_SCREEN_XDISPLAY (s), GDK_SCREEN_XNUMBER (s)) * g->maxh;

	gtk_window_get_size (GTK_WINDOW (widget), &w, &h);

	gtk_scrolled_window_get_policy (g->s, &hpol, &vpol);
	clamp = FALSE;
	
	if (w > maxw && g->autoh) {
		w = maxw;
		hpol = GTK_POLICY_AUTOMATIC;
		clamp = TRUE;
	}
	
	if (h > maxh && g->autov) {
		h = maxh;
		vpol = GTK_POLICY_AUTOMATIC;
		clamp = TRUE;
	}

	if (clamp) {
		gtk_scrolled_window_set_policy (g->s, hpol, vpol);
		gtk_window_resize (GTK_WINDOW (widget), w, h);
		
		gtk_window_get_position (GTK_WINDOW (widget), &x, &y);
		border.right = 0;
		border.bottom = 0;
		border.left = 0;
		border.top = 0;
		
		if (netk_get_desktop_margins (GDK_SCREEN_XSCREEN (s), &border)) {
			ox = x;
			oy = y;

			g_warning ("border: %d, %d, %d, %d\n", border.left, border.top, border.right, border.bottom);		
			if (x + w > border.right) x = border.right - w;
			if (x < border.left) {
				x = border.left;
				g_warning ("yuppppy\n");
			}
		
			if (y + h > border.bottom) y = border.bottom - h;
			if (y < border.top) y = border.top;
			
			if (ox != x || oy != y) {
				gtk_window_move (GTK_WINDOW (widget), x, y);
			}
		} else {
			g_warning ("no margin information\n");
		}
	}
	
	return FALSE;	
}

static
void destroy_rgd_cb(gpointer data, GClosure *closure)
{
	g_free (data);
}

void scroller_resize_gracefully (
	GtkWindow *w, GtkScrolledWindow *s, 
	GtkPolicyType hpol, GtkPolicyType vpol, 
	double hmaxs, double vmaxs
)
{
	resize_gracefully_data_t	*rgd = g_new0(resize_gracefully_data_t, 1);
	rgd->s = s;
	rgd->maxw = hmaxs;
	rgd->maxh = vmaxs;
	rgd->autoh = FALSE;
	rgd->autov = FALSE;

	if (hpol == GTK_POLICY_AUTOMATIC) {
		rgd->autoh = TRUE;
		hpol = GTK_POLICY_NEVER;
	}
	
	if (vpol == GTK_POLICY_AUTOMATIC) {
		rgd->autov = TRUE;
		vpol = GTK_POLICY_NEVER;
	}

	gtk_scrolled_window_set_policy (rgd->s, hpol, vpol);

	g_signal_connect_data (
		G_OBJECT (w), 
		"map",
		G_CALLBACK (map_window_cb),
		rgd,
		destroy_rgd_cb,
		0);
}

#else
void scroller_resize_gracefully (
	GtkWindow *w, GtkScrolledWindow *s, 
	GtkPolicyType hpol, GtkPolicyType vpol, 
	double hmaxs, double vmaxs
)
{
}

#endif

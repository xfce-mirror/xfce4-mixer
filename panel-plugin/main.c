#include <gtk/gtk.h>
#include "xfce-mixer-prefbox.h"

GtkTooltips *tooltips;

int main(int argc, char *argv[])
{
	GtkWidget *w;
	GtkWidget *win;

	extern int register_alsa (void);

        register_alsa ();
        	
	gtk_init (&argc, &argv);
	
	tooltips = gtk_tooltips_new ();
	
	w = xfce_mixer_prefbox_new ();
	gtk_widget_show (w);

	xfce_mixer_prefbox_fill_defaults (XFCE_MIXER_PREFBOX (w));
	
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
	gtk_container_add (GTK_CONTAINER (win), w);

	gtk_widget_show (win);
	
	gtk_main ();
	return 0;
}

#ifndef __MIXER_WINDOW_H
#define __MIXER_WINDOW_H
#include <stdio.h>

#include <glib.h>
#include <gtk/gtk.h>

/* for now, hardcoded volume level range 0..100 */

typedef struct tagSCT {
	GtkBox *vbox;
	GtkWidget *label;
	GtkBox *hbox;
	GtkScale *scale;
	
	//gboolean visible; -> check hbox visibility
	
	struct tagSCT *next, *prev;
	char *name;
} mixer_slider_control_t;

typedef struct {
	mixer_slider_control_t *controls, *last_control;
	GtkScrolledWindow *scroller; /* scrolled_window */
	GtkBox *hbox;
	GtkWindow *window;
} mixer_window_t;

mixer_window_t *mixer_window_new(gboolean from_glist, GList *src);


#endif /* ndef __MIXER_WINDOW_H */

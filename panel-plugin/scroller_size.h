#ifndef __SCROLLER_SIZE_H
#define __SCROLLER_SIZE_H
#include <gtk/gtk.h>

void scroller_resize_gracefully (
	GtkWindow *w, GtkScrolledWindow *s, 
	GtkPolicyType hpol, GtkPolicyType vpol, 
	double hmaxs, double vmaxs
);
                        
#endif

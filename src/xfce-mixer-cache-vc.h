#ifndef __XFCE_MIXER_CACHE_VC_H
#define __XFCE_MIXER_CACHE_VC_H

#include <gtk/gtk.h>

gboolean xfce_mixer_cache_vc_valid (gchar const *vcname);
gchar xfce_mixer_cache_vc_get_type (gchar const *vcname); /* 'S'lider, 'C'hoice, 'O'noff, 'D'isabled */
void xfce_mixer_cache_vc_refresh (void);
void xfce_mixer_cache_vc_foreach (GFunc func, gpointer user_data); /* data=volcontrol_t * */
GList *xfce_mixer_cache_vc_get_choices (gchar const *vcname);
void xfce_mixer_cache_vc_free (void);

#endif

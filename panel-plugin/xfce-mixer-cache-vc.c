/* connector between worlds (vc). control_factory is the (value) part. */ 
#include <stdlib.h>
#include <gtk/gtk.h>
#include "xfce-mixer-cache-vc.h"
#include "vc.h"

/* needs xfce_mixer_cache_vc_free to be called atexit */

static GList *vc_cache;

/* TODO mcs */

gboolean xfce_mixer_cache_vc_valid (gchar const *vcname)
{
	GList *g;
	volcontrol_t *vci;
	
	if (!vc_cache)
		xfce_mixer_cache_vc_refresh ();

	if (vc_cache) {
		g = vc_cache;
		while (g) {
			vci = (volcontrol_t *)g->data;
			if (g_str_equal (vci->name, vcname))
				return TRUE;
			g = g_list_next (g);
		}
	}
	
	return FALSE;
}

void xfce_mixer_cache_vc_free (void)
{
	if (vc_cache) {
		vc_free_control_list (vc_cache);
		vc_cache = NULL;
	}
}

void xfce_mixer_cache_vc_refresh (void)
{
	xfce_mixer_cache_vc_free ();
	vc_cache = vc_get_control_list ();
}

void xfce_mixer_cache_vc_foreach (GFunc func, gpointer user_data)
{
	GList *g;
	volcontrol_t *vci;

	if (!func)
		return;
		
	if (!vc_cache)
		xfce_mixer_cache_vc_refresh ();

	if (!vc_cache)
		return;
		
	g = vc_cache;
	while (g) {
		vci = (volcontrol_t *)g->data;
		if (vci)
			(*func) ((gpointer) vci, user_data);

		g = g_list_next (g);
	}
}

gchar xfce_mixer_cache_vc_get_type (gchar const *vcname) /* 'S'lider, 'C'hoice, 'O'noff */
{
	GList *g;
	volcontrol_t *vci;

	if (!vc_cache)
		xfce_mixer_cache_vc_refresh ();

	g = vc_cache;
	while (g) {
		vci = (volcontrol_t *)g->data;
		if (g_str_equal (vci->name, vcname)) {
			if (vci->type == CT_SLIDER)
				return 'S'; 
			if (vci->type == CT_ONOFF)
				return 'O';	
			if (vci->type == CT_SELECT)
				return 'C';	
			
			/*	
			printf ("FoUnxD %s %d\n", vcname, (int) vci->type);
			
			doesnt work. compiler bug???
			
			switch (vci->type) {
			CT_SLIDER: return 'S';
			CT_ONOFF: return 'O';
			CT_SELECT: return 'C';
			}
			*/
		
			return 'D';
		}
		g = g_list_next (g);
	}


	return 'D';
}

GList *xfce_mixer_cache_vc_get_choices (gchar const *vcname)
{
	GList *g;
	GList *gn;
	volcontrol_t *vci;
	if (!vcname)
		return NULL;
		
	if (!vc_cache)
		xfce_mixer_cache_vc_refresh ();

	if (!vc_cache)
		return NULL;

	g = vc_cache;
	while (g) {
		vci = (volcontrol_t *)g->data;
		if (g_str_equal (vci->name, vcname)) {
			g = vci->choices;
			gn = NULL;
			
			while (g) {
				gn = g_list_append (gn, g_strdup ((gchar *)g->data));
				g = g_list_next (g);
			}
			
			return gn;
		}
	
		g = g_list_next (g);
	}
	return NULL;
}


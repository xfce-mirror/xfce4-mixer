#include <stdlib.h> /* qsort */
#include <gtk/gtk.h>
#include "xfce-mixer-profile.h"
#include "xfce-mixer-view.h"
#include "xfce-mixer-cache-vc.h"
#include "vc.h"


static t_mixer_profile_item * pi_new (void)
{
	t_mixer_profile_item *pin;
	
	pin = g_new0 (t_mixer_profile_item, 1);
	
	return pin;
}
 
static t_mixer_profile_item * pi_dup (t_mixer_profile_item const *pi)
{
	t_mixer_profile_item *pin;
	
	pin = g_new0 (t_mixer_profile_item, 1);
	pin->orderno = pi->orderno;
	if (pi->location) pin->location = g_strdup (pi->location);
	if (pi->vcname) pin->vcname = g_strdup (pi->vcname);

	return pin;
}

static void pi_free (t_mixer_profile_item *pi)
{
	if (pi) {
		if (pi->location) {
			g_free (pi->location);
			pi->location = NULL;
		}
		
		if (pi->vcname) {
			g_free (pi->vcname);
			pi->vcname = NULL;
		}
		
		g_free (pi);
	}
}

static int pi_compare (const void *a, const void *b)
{
	t_mixer_profile_item **api, **bpi, *ai, *bi;
	int c1, c2;
	
	api = (t_mixer_profile_item **)a;
	bpi = (t_mixer_profile_item **)b;
	ai = *api;
	bi = *bpi;
	
	c1 = ai->location - bi->location;
	c2 = ai->orderno - bi->orderno;
	
	if (c1 == 0) return c2;
	return c1;
}

/*typedef struct {
	char *vcname;
llllll	char *location;
} t_mixer_profile_item;

struct _XfceMixerProfile 
{
	GObject	super;
	
	char *title;

	GList *controls; of t_mixer_control *
};

struct _XfceMixerProfileClass
{
	GObjectClass parent_class;
};
*/

static void xfce_mixer_profile_class_init (XfceMixerProfileClass *class);
static void xfce_mixer_profile_init (XfceMixerProfile *profile);
static void xfce_mixer_profile_finalize (GObject *object);

static GObjectClass *parent_class = NULL;
	
GtkType
xfce_mixer_profile_get_type (void)
{
	static GtkType mixer_profile_type = 0;
	
	if (!mixer_profile_type) {
		static const GTypeInfo mixer_profile_info = {
			sizeof (XfceMixerProfileClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) xfce_mixer_profile_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (XfceMixerProfile),
			0, /* n_preallocs */
			(GInstanceInitFunc) xfce_mixer_profile_init
		};
		
		mixer_profile_type = g_type_register_static (
			G_TYPE_OBJECT, "XfceMixerProfile",
			&mixer_profile_info,
		0);
	}
	
	return mixer_profile_type;
}

static void
xfce_mixer_profile_class_init (XfceMixerProfileClass *klass)
{
	GObjectClass *gobject_class;
	
	gobject_class = G_OBJECT_CLASS (klass);
	
	parent_class = gobject_class; /*G_OBJECT_GET_CLASS ();*/ /*gtk_type_class (*get_type ());*/
	gobject_class->finalize = xfce_mixer_profile_finalize;
}

static void
xfce_mixer_profile_init (XfceMixerProfile *profile)
{
	profile->title = NULL;
	profile->controls = NULL;
	profile->views = NULL;
}

XfceMixerProfile *
xfce_mixer_profile_new (const gchar *title)
{
	GObject *obj;
	obj = G_OBJECT (g_object_new (xfce_mixer_profile_get_type (), NULL));

	xfce_mixer_profile_set_title (XFCE_MIXER_PROFILE (obj), title);	

	return XFCE_MIXER_PROFILE (obj);
}

void xfce_mixer_profile_set_title(XfceMixerProfile *profile, const gchar *title)
{
	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));
	
	if (profile->title) {
		g_free (profile->title);
		profile->title = NULL;
	}
	
	profile->title = g_strdup (title);
}

gchar const *xfce_mixer_profile_get_title(XfceMixerProfile *profile)
{
	g_return_val_if_fail (profile != NULL, NULL);
	g_return_val_if_fail (XFCE_IS_MIXER_PROFILE (profile), NULL);
	
	return profile->title;
}

static void
pforeach_free_cb (gpointer data, gpointer user_data)
{
	t_mixer_profile_item * item = (t_mixer_profile_item *) data;
	
	if (item) {
		pi_free (item);
	}
}

void xfce_mixer_profile_remove_control (XfceMixerProfile *profile, const gchar *vcname)
{
	GList	*g;
	t_mixer_profile_item *item;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));

	g = profile->controls;
	while (g) {
		item = (t_mixer_profile_item *) g->data;
		if (item && item->vcname) {
			if (g_str_equal (vcname, item->vcname)) {
				pforeach_free_cb ((gpointer) item, NULL);
				g = g_list_remove (g, item);
			}
		}
		if (g) {
			g = g_list_next (g);
		}
	}
}



GList *xfce_mixer_profile_get_control_list (XfceMixerProfile *profile)
{
	t_mixer_profile_item **items;
	t_mixer_profile_item *item;
	GList *g;
	GList *gn;
	int cnt, i;
	g_return_val_if_fail (profile != NULL, NULL);
	g_return_val_if_fail (XFCE_IS_MIXER_PROFILE (profile), NULL);

	g = profile->controls;
	cnt = g_list_length (g);

	items = g_new0 (t_mixer_profile_item *, cnt);
	if (!items) return NULL;

	i = 0;
	while (g) {
		item = (t_mixer_profile_item *)g->data;
		items[i++] = pi_dup (item);
		
		g = g_list_next (g);
	}


	qsort (items, cnt, sizeof(t_mixer_profile_item *), pi_compare);


	gn = NULL;
	for(i = 0; i < cnt; i++) {
		gn = g_list_append (gn, items[i]);
	}
	g_free (items);

	return gn;
}

/* todo load/save from/to xml */


static void
pcontrol_list_free (GList *c)
{
	g_list_foreach (c, (GFunc) pforeach_free_cb, NULL);
	g_list_free (c);
}

static void
xfce_mixer_profile_finalize (GObject *obj)
{
	XfceMixerProfile *profile;
	
	g_return_if_fail (obj != NULL);
	profile = XFCE_MIXER_PROFILE (obj);

	if (profile->title) {
		g_free (profile->title);
		profile->title = NULL;
	}
	
	if (profile->controls) {
		pcontrol_list_free (profile->controls);
		profile->controls = NULL;
	}
}

void
xfce_mixer_profile_free_control_list(XfceMixerProfile *profile, GList *go)
{
	GList *g;
	t_mixer_profile_item *pi;
	if (go) {
		g = go;
		while (g) {
			pi = (t_mixer_profile_item *)g->data;
			if (pi) {
				pi_free (pi);
			}
			g = g_list_next (g);
		}
		g_list_free (go);
	}
}

void
xfce_mixer_profile_register_view(XfceMixerProfile *profile, XfceMixerView *view)
{
	XfceMixerView *v;
	GList *go;
	GList *g;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));
	
	go = profile->views;
	g = go;
	while (g) {
		v = XFCE_MIXER_VIEW (g->data);
		
		if (v == view) return;
		
		g = g_list_next (g);
	}
	
	profile->views = g_list_append (profile->views, view);

	xfce_mixer_profile_refresh_view(profile, view);
}

void
xfce_mixer_profile_unregister_view(XfceMixerProfile *profile, XfceMixerView *view)
{
	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));

	profile->views = g_list_remove (profile->views, view);
}

static
t_mixer_profile_item *find_item_by_vcname(XfceMixerProfile *profile, gchar const *vcname)
{
	t_mixer_profile_item *item;
	
	GList *g;
	
	if (!vcname) 
		return NULL;
		
	g = profile->controls;
	
	while (g) {
		item = (t_mixer_profile_item *) g->data;
		
		if (item->vcname && g_str_equal (item->vcname, vcname)) {
			return item;
		}
	
		g = g_list_next (g);
	}
	
	return NULL;
}

/*
static void pi_foreach_cb (gpointer data, gpointer user_data)
{
	t_mixer_profile_item * item = (t_mixer_profile_item *) data;
	XfceMixerProfile *profile;

	profile = XFCE_MIXER_PROFILE (user_data);
	
	xfce_mixer_profile_update_views_p (profile, item);
}
*/

void xfce_mixer_profile_update_views_p (
	XfceMixerProfile *profile, 
	t_mixer_profile_item *p
)
{
	GList *g;
	XfceMixerView *view;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));

/*	TRACEY ("xfce_mixer_profile_update_views_p: enter\n");*/
		g = profile->views;
		while (g) {
			view = XFCE_MIXER_VIEW (g->data);
			
/*	TRACEY ("xfce_mixer_profile_update_views_p: item\n");*/
			xfce_mixer_view_profile_item_updated (view, p);
			
			g = g_list_next (g);
		}

/*	TRACEY ("xfce_mixer_profile_update_views_p: leave\n");*/
}

void xfce_mixer_profile_refresh_view(XfceMixerProfile *profile, XfceMixerView *view)
{
	t_mixer_profile_item *item;
	GList *g2;
	xfce_mixer_view_profile_cleared (view);
	g2 = profile->controls;
	while (g2) {
		item = (t_mixer_profile_item *) g2->data;
			
		xfce_mixer_view_profile_item_updated (view, item);
		g2 = g_list_next (g2);
	}
			
}

void xfce_mixer_profile_refresh_views (XfceMixerProfile *profile)
{
	GList *g;
	XfceMixerView *view;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));

		g = profile->views;
		while (g) {
			view = XFCE_MIXER_VIEW (g->data);
			
			g = g_list_next (g);
		}

	
}

/* add/update/delete item by t_mixer_profile_item */
void
xfce_mixer_profile_update_control (XfceMixerProfile *profile, t_mixer_profile_item const *item)
{
	t_mixer_profile_item *p;
/*	GList *g;*/
	/*XfceMixerView *view;*/
	gboolean dodel;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));
	
	
	if (!item || !item->vcname)
		return;
		
	dodel = FALSE;

	/* DO:
	orderno = -1 : delete
	           0 : automatic orderno
	
	search for vcname in list, create if not found
	update list and views
	*/
	
	p = find_item_by_vcname (profile, item->vcname);
	if (!p) {
		p = pi_dup (item);
		profile->controls = g_list_append (profile->controls, p);
	} else {
		p->orderno = item->orderno;
		if (p->location) {
			g_free (p->location);
			p->location = NULL;
		}
		if (item->location) {
			p->location = g_strdup (item->location);
		}
	}

	if (p) {
		if (!p->orderno) {
			p->orderno = g_list_length (profile->controls);
		} else if (p->orderno == -1) { /* delete */
			profile->controls = g_list_remove (profile->controls, p);
			dodel = TRUE;
		}

		xfce_mixer_profile_update_views_p (profile, p);
		
		if (dodel) {
			pi_free (p);
		}
	}
}

/*
TODO sort defaults like:
	sliders first
	onoff second
	choices last.
	
typedef struct {
	GList *s;
	GList *o;
	GList *c;
} t_default_list;
static void default_list_new ()
{
}
*/

static void 
xfce_mixer_profile_fill_default (gpointer data, gpointer user_data)
{
	volcontrol_t *vci;
	XfceMixerProfile *profile;
	t_mixer_profile_item *pi;
	
	vci = (volcontrol_t *) data;
	profile = XFCE_MIXER_PROFILE (user_data);
	
	pi = pi_new ();
	pi->vcname = g_strdup (vci->name);
	pi->location = g_strdup ("default");
	
	xfce_mixer_profile_update_control (profile, pi);

	pi_free (pi);
	
	/*vci->type;*/
}

void
xfce_mixer_profile_fill_defaults (XfceMixerProfile *profile)
{
	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));
	
	xfce_mixer_profile_clear (profile);
	xfce_mixer_cache_vc_foreach (xfce_mixer_profile_fill_default, (gpointer) profile);
}

void
xfce_mixer_profile_clear (XfceMixerProfile *profile)
{
	GList *g;
	GList *gp;
	t_mixer_profile_item *item;

	g_return_if_fail (profile != NULL);
	g_return_if_fail (XFCE_IS_MIXER_PROFILE (profile));

 	g = profile->controls;
 	if (g)
 		g = g_list_last (g);
	
	while (g) {
		item = (t_mixer_profile_item *) g->data;

		gp = g_list_previous (g);

		item = pi_dup (item);
		item->orderno = -1;
				
		xfce_mixer_profile_update_control (profile, item);
		
		pi_free (item);

		g = gp;
	}
	
}

void xfce_mixer_profile_load(XfceMixerProfile *profile, XfceMixerPxml *xml)
{
		/* read data etc */
		gchar *vcname;
		gchar *location;
		gchar *sorderno;
		gint orderno;
		t_mixer_profile_item *item;
		
	xfce_mixer_profile_clear (profile);


		xfce_mixer_pxml_goto_child_tag (xml, "controls");
		if (!xml->node)
			return;

		xfce_mixer_pxml_goto_children (xml);
		if (!xml->node)
			return;

		while (xml->node) {
			if (xfce_mixer_pxml_check_tag (xml, "control")) {
				vcname = xfce_mixer_pxml_get_prop (xml, "vcname");

				if (vcname) {
					location = xfce_mixer_pxml_get_prop (xml, "location");

					sorderno = xfce_mixer_pxml_get_prop (xml, "orderno");
					if (sorderno) {
						orderno = atoi (sorderno);
						g_free (sorderno);
					} else
						orderno = -1;

					item = pi_new ();
					if (item) {
						item->vcname = vcname; vcname = NULL;
						item->location = location; location = NULL;
						item->orderno = orderno;
						xfce_mixer_profile_update_control (profile, item);
						g_free (item);
					}

					if (location)
						g_free (location);

					g_free (vcname);
				}
			}

			xfce_mixer_pxml_goto_next (xml);
		}
}

void xfce_mixer_profile_save(XfceMixerProfile *profile, XfceMixerPxml *xml)
{
		/* save actual data */

		GList *go, *g;
		xmlNodePtr parent;
		t_mixer_profile_item *item;
		gchar sorderno[20];
		
		xml->node = xfce_mixer_pxml_create_text_child (xml, "controls", NULL);
		parent = xml->node;

		go = xfce_mixer_profile_get_control_list (profile);
		g = go;
		while (g) {
			item = (t_mixer_profile_item *)g->data;

			if (item && item->vcname) {
				xfce_mixer_pxml_goto_node (xml, parent);
				xml->node = xfce_mixer_pxml_create_text_child (xml, "control", NULL);

				g_snprintf (sorderno, sizeof(sorderno), "%d", item->orderno);

				xfce_mixer_pxml_set_prop (xml, "vcname", item->vcname);
				xfce_mixer_pxml_set_prop (xml, "orderno", sorderno);
				if (item->location)
					xfce_mixer_pxml_set_prop (xml, "location", item->location);
			}
			
			g = g_list_next (g);
		}
		if (go)
			xfce_mixer_profile_free_control_list (profile, go);
}


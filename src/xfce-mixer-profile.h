#ifndef __XFCE_MIXER_PROFILE_H
#define __XFCE_MIXER_PROFILE_H
#include <gtk/gtk.h>
#ifdef __cplusplus
extern "C" {
#endif

#define XFCE_MIXER_PROFILE(obj) G_TYPE_CHECK_INSTANCE_CAST (obj, xfce_mixer_profile_get_type (), XfceMixerProfile)
#define XFCE_MIXER_PROFILE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, xfce_mixer_profile_get_type (), XfceMixerProfile)
#define XFCE_IS_MIXER_PROFILE(obj) G_TYPE_CHECK_INSTANCE_TYPE (obj, xfce_mixer_profile_get_type ())

typedef struct {
	char *vcname;
	char *location;
	int orderno;
} t_mixer_profile_item;

struct _XfceMixerProfile 
{
	GObject	super;
	
	char *title;

	GList *controls; /* of t_mixer_profile_item * */
	
	GList *views; /* of XfceMixerView */
};

struct _XfceMixerProfileClass
{
	GObjectClass parent_class;
};

typedef struct _XfceMixerProfile XfceMixerProfile;
typedef struct _XfceMixerProfileClass XfceMixerProfileClass;


GtkType xfce_mixer_profile_get_type (void);
XfceMixerProfile *xfce_mixer_profile_new (const gchar *title);
void xfce_mixer_profile_set_title(XfceMixerProfile *profile, const gchar *title);
gchar const *xfce_mixer_profile_get_title(XfceMixerProfile *profile);
void xfce_mixer_profile_remove_control (XfceMixerProfile *profile, const gchar *vcname);
void xfce_mixer_profile_clear (XfceMixerProfile *profile);
void xfce_mixer_profile_update_control (XfceMixerProfile *profile, t_mixer_profile_item const *item);
void xfce_mixer_profile_refresh_views (XfceMixerProfile *profile);
void xfce_mixer_profile_update_views_p (
	XfceMixerProfile *profile, 
	t_mixer_profile_item *p  
);

void xfce_mixer_profile_fill_defaults (XfceMixerProfile *profile);

GList *xfce_mixer_profile_get_control_list (XfceMixerProfile *profile);
void xfce_mixer_profile_free_control_list(XfceMixerProfile *profile, GList *go);

#ifndef __TYPEDEF_XFCE_MIXER_VIEW__
#define __TYPEDEF_XFCE_MIXER_VIEW__
typedef struct _XfceMixerView XfceMixerView;
#endif

void xfce_mixer_profile_register_view(XfceMixerProfile *profile, XfceMixerView *view);
void xfce_mixer_profile_unregister_view(XfceMixerProfile *profile, XfceMixerView *view);

/* todo load/save from/to xml */

#endif /* ndef __XFCE_MIXER_PROFILE_H */

/* Generated by GOB (v2.0.6)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>
#ifndef __XFCE_MIXER_PROFILES_H__
#define __XFCE_MIXER_PROFILES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include <gtk/gtk.h>
#include "xfce-mixer-profile.h"
#include "xfce-mixer-pxml.h"


/*
 * Type checking and casting macros
 */
#define XFCE_TYPE_MIXER_PROFILES	(xfce_mixer_profiles_get_type())
#define XFCE_MIXER_PROFILES(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_profiles_get_type(), XfceMixerProfiles)
#define XFCE_MIXER_PROFILES_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_profiles_get_type(), XfceMixerProfiles const)
#define XFCE_MIXER_PROFILES_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), xfce_mixer_profiles_get_type(), XfceMixerProfilesClass)
#define XFCE_IS_MIXER_PROFILES(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), xfce_mixer_profiles_get_type ())

#define XFCE_MIXER_PROFILES_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), xfce_mixer_profiles_get_type(), XfceMixerProfilesClass)

/*
 * Main object structure
 */
#ifndef __TYPEDEF_XFCE_MIXER_PROFILES__
#define __TYPEDEF_XFCE_MIXER_PROFILES__
typedef struct _XfceMixerProfiles XfceMixerProfiles;
#endif
struct _XfceMixerProfiles {
	GObject __parent__;
	/*< private >*/
	gchar * device; /* protected */
	gchar * fname; /* protected */
	gboolean modified; /* protected */
	gint weird_number; /* protected */
	GList * profiles; /* protected */
	XfceMixerPxml * xml; /* protected */
};

/*
 * Class definition
 */
typedef struct _XfceMixerProfilesClass XfceMixerProfilesClass;
struct _XfceMixerProfilesClass {
	GObjectClass __parent__;
};


/*
 * Public methods
 */
GType	xfce_mixer_profiles_get_type	(void);
void 	xfce_mixer_profiles_clear	(XfceMixerProfiles * self);
void 	xfce_mixer_profiles_load	(XfceMixerProfiles * self);
void 	xfce_mixer_profiles_save	(XfceMixerProfiles * self);
XfceMixerProfile * 	xfce_mixer_profiles_create_profile	(XfceMixerProfiles * self,
					gchar const * fixedname);
XfceMixerProfile * 	xfce_mixer_profiles_get_profile	(XfceMixerProfiles * self,
					gchar const * name);
void 	xfce_mixer_profiles_delete_profile	(XfceMixerProfiles * self,
					gchar const * name);
void 	xfce_mixer_profiles_set_modified	(XfceMixerProfiles * self,
					gboolean modi);
gboolean 	xfce_mixer_profiles_get_modified	(XfceMixerProfiles * self);
XfceMixerProfiles * 	xfce_mixer_profiles_new	(gchar const * device);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

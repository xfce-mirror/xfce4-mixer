/* Generated by GOB (v2.0.6)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>
#ifndef __XFCE_MIXER_WINDOW_H__
#define __XFCE_MIXER_WINDOW_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include <gtk/gtk.h>
#include "xfce-mixer-view.h"
#include "xfce-mixer-profile.h"
#include "xfce-mixer-mcs-client.h"


/*
 * Type checking and casting macros
 */
#define XFCE_TYPE_MIXER_WINDOW	(xfce_mixer_window_get_type())
#define XFCE_MIXER_WINDOW(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_window_get_type(), XfceMixerWindow)
#define XFCE_MIXER_WINDOW_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_window_get_type(), XfceMixerWindow const)
#define XFCE_MIXER_WINDOW_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), xfce_mixer_window_get_type(), XfceMixerWindowClass)
#define XFCE_IS_MIXER_WINDOW(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), xfce_mixer_window_get_type ())

#define XFCE_MIXER_WINDOW_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), xfce_mixer_window_get_type(), XfceMixerWindowClass)

/*
 * Main object structure
 */
#ifndef __TYPEDEF_XFCE_MIXER_WINDOW__
#define __TYPEDEF_XFCE_MIXER_WINDOW__
typedef struct _XfceMixerWindow XfceMixerWindow;
#endif
struct _XfceMixerWindow {
	GtkWindow __parent__;
	/*< public >*/
	XfceMixerView * view;
	GtkBox * box;
	XfceMixerProfile * profile;
	/*< private >*/
	GtkMenuBar * menubar; /* protected */
	GtkAccelGroup * accelgroup; /* protected */
	XfceMixerMcsClient * mcsc; /* protected */
};

/*
 * Class definition
 */
typedef struct _XfceMixerWindowClass XfceMixerWindowClass;
struct _XfceMixerWindowClass {
	GtkWindowClass __parent__;
};


/*
 * Public methods
 */
GType	xfce_mixer_window_get_type	(void);
GtkWidget * 	xfce_mixer_window_new	(void);
void 	xfce_mixer_window_refresh	(XfceMixerWindow * self);
void 	xfce_mixer_window_reset_profile	(XfceMixerWindow * self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

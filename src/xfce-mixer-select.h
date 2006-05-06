/* Generated by GOB (v2.0.14)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>
#ifndef __XFCE_MIXER_SELECT_H__
#define __XFCE_MIXER_SELECT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include <gtk/gtk.h>
#include "xfce-mixer-control.h"
#include "xfce-mixer-cache-vc.h"
#include "vc.h"


/*
 * Type checking and casting macros
 */
#define XFCE_TYPE_MIXER_SELECT	(xfce_mixer_select_get_type())
#define XFCE_MIXER_SELECT(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_select_get_type(), XfceMixerSelect)
#define XFCE_MIXER_SELECT_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_select_get_type(), XfceMixerSelect const)
#define XFCE_MIXER_SELECT_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), xfce_mixer_select_get_type(), XfceMixerSelectClass)
#define XFCE_IS_MIXER_SELECT(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), xfce_mixer_select_get_type ())

#define XFCE_MIXER_SELECT_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), xfce_mixer_select_get_type(), XfceMixerSelectClass)

/*
 * Main object structure
 */
#ifndef __TYPEDEF_XFCE_MIXER_SELECT__
#define __TYPEDEF_XFCE_MIXER_SELECT__
typedef struct _XfceMixerSelect XfceMixerSelect;
#endif
struct _XfceMixerSelect {
	XfceMixerControl __parent__;
	/*< private >*/
	GtkOptionMenu * om; /* protected */
	GtkLabel * label; /* protected */
	GList * choices; /* protected */
};

/*
 * Class definition
 */
typedef struct _XfceMixerSelectClass XfceMixerSelectClass;
struct _XfceMixerSelectClass {
	XfceMixerControlClass __parent__;
};


/*
 * Public methods
 */
GType	xfce_mixer_select_get_type	(void);
XfceMixerControl * 	xfce_mixer_select_new	(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

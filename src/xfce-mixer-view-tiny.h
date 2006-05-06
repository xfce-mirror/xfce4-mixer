/* Generated by GOB (v2.0.14)   (do not edit directly) */

#include <glib.h>
#include <glib-object.h>
#ifndef __XFCE_MIXER_VIEW_TINY_H__
#define __XFCE_MIXER_VIEW_TINY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#include "xfce-mixer-view.h"
#include "xfce-mixer-control-factory.h"


/*
 * Type checking and casting macros
 */
#define XFCE_TYPE_MIXER_VIEW_TINY	(xfce_mixer_view_tiny_get_type())
#define XFCE_MIXER_VIEW_TINY(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_view_tiny_get_type(), XfceMixerViewTiny)
#define XFCE_MIXER_VIEW_TINY_CONST(obj)	G_TYPE_CHECK_INSTANCE_CAST((obj), xfce_mixer_view_tiny_get_type(), XfceMixerViewTiny const)
#define XFCE_MIXER_VIEW_TINY_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST((klass), xfce_mixer_view_tiny_get_type(), XfceMixerViewTinyClass)
#define XFCE_IS_MIXER_VIEW_TINY(obj)	G_TYPE_CHECK_INSTANCE_TYPE((obj), xfce_mixer_view_tiny_get_type ())

#define XFCE_MIXER_VIEW_TINY_GET_CLASS(obj)	G_TYPE_INSTANCE_GET_CLASS((obj), xfce_mixer_view_tiny_get_type(), XfceMixerViewTinyClass)

/*
 * Main object structure
 */
#ifndef __TYPEDEF_XFCE_MIXER_VIEW_TINY__
#define __TYPEDEF_XFCE_MIXER_VIEW_TINY__
typedef struct _XfceMixerViewTiny XfceMixerViewTiny;
#endif
struct _XfceMixerViewTiny {
	XfceMixerView __parent__;
};

/*
 * Class definition
 */
typedef struct _XfceMixerViewTinyClass XfceMixerViewTinyClass;
struct _XfceMixerViewTinyClass {
	XfceMixerViewClass __parent__;
};


/*
 * Public methods
 */
GType	xfce_mixer_view_tiny_get_type	(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

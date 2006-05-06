/* Generated by GOB (v2.0.14)   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 14

#define selfp (self->_priv)

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h> /* memset() */

#include "xfce-mixer-view-tiny.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 1 "mixer-view-tiny.gob"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#line 33 "xfce-mixer-view-tiny.c"

#line 7 "mixer-view-tiny.gob"

#include "xfce-mixer-view.h"
#include "xfce-mixer-control-factory.h"

#line 40 "xfce-mixer-view-tiny.c"
/* self casting macros */
#define SELF(x) XFCE_MIXER_VIEW_TINY(x)
#define SELF_CONST(x) XFCE_MIXER_VIEW_TINY_CONST(x)
#define IS_SELF(x) XFCE_IS_MIXER_VIEW_TINY(x)
#define TYPE_SELF XFCE_TYPE_MIXER_VIEW_TINY
#define SELF_CLASS(x) XFCE_MIXER_VIEW_TINY_CLASS(x)

#define SELF_GET_CLASS(x) XFCE_MIXER_VIEW_TINY_GET_CLASS(x)

/* self typedefs */
typedef XfceMixerViewTiny Self;
typedef XfceMixerViewTinyClass SelfClass;

/* here are local prototypes */
static void xfce_mixer_view_tiny_init (XfceMixerViewTiny * o) G_GNUC_UNUSED;
static void xfce_mixer_view_tiny_class_init (XfceMixerViewTinyClass * c) G_GNUC_UNUSED;
static void ___1_xfce_mixer_view_tiny_init_containers (XfceMixerView * pself) G_GNUC_UNUSED;

/* pointer to the class of our parent */
static XfceMixerViewClass *parent_class = NULL;

GType
xfce_mixer_view_tiny_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (XfceMixerViewTinyClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) xfce_mixer_view_tiny_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (XfceMixerViewTiny),
			0 /* n_preallocs */,
			(GInstanceInitFunc) xfce_mixer_view_tiny_init,
			NULL
		};

		type = g_type_register_static (XFCE_TYPE_MIXER_VIEW, "XfceMixerViewTiny", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((XfceMixerViewTiny *)g_object_new(xfce_mixer_view_tiny_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static XfceMixerViewTiny * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static XfceMixerViewTiny *
GET_NEW_VARG (const char *first, ...)
{
	XfceMixerViewTiny *ret;
	va_list ap;
	va_start (ap, first);
	ret = (XfceMixerViewTiny *)g_object_new_valist (xfce_mixer_view_tiny_get_type (), first, ap);
	va_end (ap);
	return ret;
}

static void 
xfce_mixer_view_tiny_init (XfceMixerViewTiny * o G_GNUC_UNUSED)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:View:Tiny::init"
}
#undef __GOB_FUNCTION__
static void 
xfce_mixer_view_tiny_class_init (XfceMixerViewTinyClass * c G_GNUC_UNUSED)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:View:Tiny::class_init"
	XfceMixerViewClass *xfce_mixer_view_class = (XfceMixerViewClass *)c;

	parent_class = g_type_class_ref (XFCE_TYPE_MIXER_VIEW);

#line 13 "mixer-view-tiny.gob"
	xfce_mixer_view_class->init_containers = ___1_xfce_mixer_view_tiny_init_containers;
#line 120 "xfce-mixer-view-tiny.c"
}
#undef __GOB_FUNCTION__



#line 13 "mixer-view-tiny.gob"
static void 
___1_xfce_mixer_view_tiny_init_containers (XfceMixerView * pself G_GNUC_UNUSED)
#line 129 "xfce-mixer-view-tiny.c"
#define PARENT_HANDLER(___pself) \
	{ if(XFCE_MIXER_VIEW_CLASS(parent_class)->init_containers) \
		(* XFCE_MIXER_VIEW_CLASS(parent_class)->init_containers)(___pself); }
{
#define __GOB_FUNCTION__ "Xfce:Mixer:View:Tiny::init_containers"
#line 13 "mixer-view-tiny.gob"
	g_return_if_fail (pself != NULL);
#line 13 "mixer-view-tiny.gob"
	g_return_if_fail (XFCE_IS_MIXER_VIEW (pself));
#line 139 "xfce-mixer-view-tiny.c"
{
#line 14 "mixer-view-tiny.gob"
	
		pself->kind = K_TINY;

		pself->sliders = GTK_BOX (gtk_vbox_new (FALSE, 0));
		gtk_widget_show (GTK_WIDGET (pself->sliders));
		pself->switches = NULL;

		/*gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->vbox));*/
		gtk_box_pack_start (GTK_BOX (pself), GTK_WIDGET (pself->sliders), TRUE, TRUE, 0);

		gtk_widget_set_name (GTK_WIDGET (pself->sliders), "sliders");
	}}
#line 154 "xfce-mixer-view-tiny.c"
#undef __GOB_FUNCTION__
#undef PARENT_HANDLER

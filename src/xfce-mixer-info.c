/* Generated by GOB (v2.0.6) on Mon Jan  5 20:48:50 2004
   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 6

#define selfp (self->_priv)

#include "xfce-mixer-info.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 2 "mixer-info.gob"

#include <gtk/gtk.h>

#line 27 "xfce-mixer-info.c"
/* self casting macros */
#define SELF(x) XFCE_MIXER_INFO(x)
#define SELF_CONST(x) XFCE_MIXER_INFO_CONST(x)
#define IS_SELF(x) XFCE_IS_MIXER_INFO(x)
#define TYPE_SELF XFCE_TYPE_MIXER_INFO
#define SELF_CLASS(x) XFCE_MIXER_INFO_CLASS(x)

#define SELF_GET_CLASS(x) XFCE_MIXER_INFO_GET_CLASS(x)

/* self typedefs */
typedef XfceMixerInfo Self;
typedef XfceMixerInfoClass SelfClass;

/* here are local prototypes */
static void xfce_mixer_info_class_init (XfceMixerInfoClass * c) G_GNUC_UNUSED;
static void xfce_mixer_info_init (XfceMixerInfo * self) G_GNUC_UNUSED;

/* pointer to the class of our parent */
static GtkDialogClass *parent_class = NULL;

/* Short form macros */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define self_new() xfce_mixer_info_new()
#endif /* __GNUC__ && !__STRICT_ANSI__ */

/* Short form pointers */
static GtkWidget * (* const self_new) (void) = xfce_mixer_info_new;

GType
xfce_mixer_info_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (XfceMixerInfoClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) xfce_mixer_info_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (XfceMixerInfo),
			0 /* n_preallocs */,
			(GInstanceInitFunc) xfce_mixer_info_init,
		};

		type = g_type_register_static (GTK_TYPE_DIALOG, "XfceMixerInfo", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((XfceMixerInfo *)g_object_new(xfce_mixer_info_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static XfceMixerInfo * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static XfceMixerInfo *
GET_NEW_VARG (const char *first, ...)
{
	XfceMixerInfo *ret;
	va_list ap;
	va_start (ap, first);
	ret = (XfceMixerInfo *)g_object_new_valist (xfce_mixer_info_get_type (), first, ap);
	va_end (ap);
	return ret;
}

static void 
xfce_mixer_info_class_init (XfceMixerInfoClass * c)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Info::class_init"

	parent_class = g_type_class_ref (GTK_TYPE_DIALOG);

	return;
	c = NULL;
}
#undef __GOB_FUNCTION__
#line 11 "mixer-info.gob"
static void 
xfce_mixer_info_init (XfceMixerInfo * self)
#line 111 "xfce-mixer-info.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Info::init"
 {
#line 12 "mixer-info.gob"

		gchar *txt;

		self->box = GTK_BOX (gtk_hbox_new (FALSE, 5));
		self->image = GTK_IMAGE (gtk_image_new_from_stock 
			(GTK_STOCK_DIALOG_INFO,
			GTK_ICON_SIZE_DIALOG
			)
		);

		txt = "This is xfce4-mixer, a volume control program.\n"
			"It is released under the GPL-2 License which you find in the source directory.\n"
			"No warranty in any way.";

		self->label = GTK_LABEL (gtk_label_new (txt));

		gtk_box_pack_start (self->box, GTK_WIDGET (self->image), FALSE, FALSE, 0);
		gtk_box_pack_start (self->box, GTK_WIDGET (self->label), FALSE, FALSE, 0);

		gtk_widget_show (GTK_WIDGET (self->box));
		gtk_widget_show (GTK_WIDGET (self->image));
		gtk_widget_show (GTK_WIDGET (self->label));

		gtk_box_pack_start (
			GTK_BOX (GTK_DIALOG (self)->vbox), 
			GTK_WIDGET (self->box), FALSE, FALSE, 0
		);

		gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);
		gtk_dialog_set_has_separator (GTK_DIALOG (self), FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (self), 10);
	
#line 148 "xfce-mixer-info.c"
 }
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__



#line 45 "mixer-info.gob"
GtkWidget * 
xfce_mixer_info_new (void)
#line 160 "xfce-mixer-info.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Info::new"
{
#line 46 "mixer-info.gob"
	
		return GTK_WIDGET (GET_NEW);
	}}
#line 168 "xfce-mixer-info.c"
#undef __GOB_FUNCTION__


#if (!defined __GNUC__) || (defined __GNUC__ && defined __STRICT_ANSI__)
/*REALLY BAD HACK
  This is to avoid unused warnings if you don't call
  some method.  I need to find a better way to do
  this, not needed in GCC since we use some gcc
  extentions to make saner, faster code */
static void
___xfce_mixer_info_really_bad_hack_to_avoid_warnings(void)
{
	((void (*)(void))GET_NEW_VARG)();
	((void (*)(void))self_new)();
	___xfce_mixer_info_really_bad_hack_to_avoid_warnings();
}
#endif /* !__GNUC__ || (__GNUC__ && __STRICT_ANSI__) */


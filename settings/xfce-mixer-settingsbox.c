/* Generated by GOB (v2.0.6) on Mon Jan  5 20:40:20 2004
   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 6

#define selfp (self->_priv)

#include <string.h> /* memset() */

#include "xfce-mixer-settingsbox.h"

#include "xfce-mixer-settingsbox-private.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 1 "mixer-settingsbox.gob"

#include <gtk/gtk.h>
#include <libxfce4util/i18n.h>
#include <libxfcegui4/xfce_framebox.h>
#include <libxfce4mcs/mcs-manager.h>

#line 34 "xfce-mixer-settingsbox.c"

#line 7 "mixer-settingsbox.gob"

#include "helpers3.inc"
#include "vc.h"
#include "stringlist.inc"

#undef SHOW_DEFAULT_DEVICE_ENTRY


#line 45 "xfce-mixer-settingsbox.c"
/* self casting macros */
#define SELF(x) XFCE_MIXER_SETTINGSBOX(x)
#define SELF_CONST(x) XFCE_MIXER_SETTINGSBOX_CONST(x)
#define IS_SELF(x) XFCE_IS_MIXER_SETTINGSBOX(x)
#define TYPE_SELF XFCE_TYPE_MIXER_SETTINGSBOX
#define SELF_CLASS(x) XFCE_MIXER_SETTINGSBOX_CLASS(x)

#define SELF_GET_CLASS(x) XFCE_MIXER_SETTINGSBOX_GET_CLASS(x)

/* self typedefs */
typedef XfceMixerSettingsbox Self;
typedef XfceMixerSettingsboxClass SelfClass;

/* here are local prototypes */
static void xfce_mixer_settingsbox_class_init (XfceMixerSettingsboxClass * c) G_GNUC_UNUSED;
static void xfce_mixer_settingsbox_init (XfceMixerSettingsbox * self) G_GNUC_UNUSED;
static void xfce_mixer_settingsbox_device_changed_d_cb (XfceMixerSettingsbox * self, gpointer user_data) G_GNUC_UNUSED;
static void xfce_mixer_settingsbox_device_changed_t_cb (XfceMixerSettingsbox * self, GtkTreeSelection * arg1) G_GNUC_UNUSED;

/* pointer to the class of our parent */
static GtkVBoxClass *parent_class = NULL;

/* Short form macros */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define self_load(args...) xfce_mixer_settingsbox_load(args)
#define self_device_changed_d_cb(args...) xfce_mixer_settingsbox_device_changed_d_cb(args)
#define self_device_changed_t_cb(args...) xfce_mixer_settingsbox_device_changed_t_cb(args)
#define self_device_changed_cb(args...) xfce_mixer_settingsbox_device_changed_cb(args)
#define self_devicelst_updated(args...) xfce_mixer_settingsbox_devicelst_updated(args)
#define self_apply_right_box(args...) xfce_mixer_settingsbox_apply_right_box(args)
#define self_new() xfce_mixer_settingsbox_new()
#endif /* __GNUC__ && !__STRICT_ANSI__ */

/* Short form pointers */
static void (* const self_load) (XfceMixerSettingsbox * self) = xfce_mixer_settingsbox_load;
static void (* const self_device_changed_d_cb) (XfceMixerSettingsbox * self, gpointer user_data) = xfce_mixer_settingsbox_device_changed_d_cb;
static void (* const self_device_changed_t_cb) (XfceMixerSettingsbox * self, GtkTreeSelection * arg1) = xfce_mixer_settingsbox_device_changed_t_cb;
static void (* const self_device_changed_cb) (XfceMixerSettingsbox * self) = xfce_mixer_settingsbox_device_changed_cb;
static void (* const self_devicelst_updated) (XfceMixerSettingsbox * self) = xfce_mixer_settingsbox_devicelst_updated;
static void (* const self_apply_right_box) (XfceMixerSettingsbox * self) = xfce_mixer_settingsbox_apply_right_box;
static XfceMixerSettingsbox * (* const self_new) (void) = xfce_mixer_settingsbox_new;

GType
xfce_mixer_settingsbox_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (XfceMixerSettingsboxClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) xfce_mixer_settingsbox_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (XfceMixerSettingsbox),
			0 /* n_preallocs */,
			(GInstanceInitFunc) xfce_mixer_settingsbox_init,
		};

		type = g_type_register_static (GTK_TYPE_VBOX, "XfceMixerSettingsbox", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((XfceMixerSettingsbox *)g_object_new(xfce_mixer_settingsbox_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static XfceMixerSettingsbox * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static XfceMixerSettingsbox *
GET_NEW_VARG (const char *first, ...)
{
	XfceMixerSettingsbox *ret;
	va_list ap;
	va_start (ap, first);
	ret = (XfceMixerSettingsbox *)g_object_new_valist (xfce_mixer_settingsbox_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___finalize(GObject *obj_self)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::finalize"
	XfceMixerSettingsbox *self = XFCE_MIXER_SETTINGSBOX (obj_self);
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
#line 30 "mixer-settingsbox.gob"
	if(self->device_lst) { ((*(void (*)(void *))stringlist_free)) (self->device_lst); self->device_lst = NULL; }
#line 139 "xfce-mixer-settingsbox.c"
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__

static void 
xfce_mixer_settingsbox_class_init (XfceMixerSettingsboxClass * c)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::class_init"
	GObjectClass *g_object_class = (GObjectClass*) c;

	parent_class = g_type_class_ref (GTK_TYPE_VBOX);

	g_object_class->finalize = ___finalize;
	return;
	c = NULL;
	g_object_class = NULL;
}
#undef __GOB_FUNCTION__
#line 40 "mixer-settingsbox.gob"
static void 
xfce_mixer_settingsbox_init (XfceMixerSettingsbox * self)
#line 162 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::init"
#line 7 "mixer-settingsbox.gob"
	self->dev_frame = NULL;
#line 167 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->dev_label = NULL;
#line 170 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->useful_frame = NULL;
#line 173 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->useful_tv = NULL;
#line 176 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->useful_model = NULL;
#line 179 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->useful_sc = NULL;
#line 182 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->devlist_model = NULL;
#line 185 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->devlist_tv = NULL;
#line 188 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->devlist_sc = NULL;
#line 191 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->cols = NULL;
#line 194 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->right_box = NULL;
#line 197 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->sep = NULL;
#line 200 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->device_lst = NULL;
#line 203 "xfce-mixer-settingsbox.c"
#line 7 "mixer-settingsbox.gob"
	self->manager = NULL;
#line 206 "xfce-mixer-settingsbox.c"
 {
#line 41 "mixer-settingsbox.gob"

		GtkTreeSelection *sels;

		self->cols = GTK_BOX (gtk_hbox_new (FALSE, 5));
		gtk_widget_show (GTK_WIDGET (self->cols));

		self->sep = gtk_vseparator_new ();
		gtk_widget_show (GTK_WIDGET (self->sep));

		self->devlist_tv = GTK_TREE_VIEW (gtk_tree_view_new ());
		gtk_tree_view_append_column (self->devlist_tv, 
			tree_new_text_column (_ ("Device"), 0
			)
		);

		gtk_widget_show (GTK_WIDGET (self->devlist_tv));

		self->devlist_sc = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
		gtk_scrolled_window_set_policy (self->devlist_sc, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (self->devlist_sc), GTK_WIDGET (self->devlist_tv));
		gtk_widget_show (GTK_WIDGET (self->devlist_sc));

		self->devlist_model = GTK_TREE_STORE (gtk_tree_store_new (1, G_TYPE_STRING));
		gtk_tree_view_set_model (self->devlist_tv, GTK_TREE_MODEL (self->devlist_model));

		self->right_box = GTK_BOX (gtk_vbox_new (FALSE, 5));
		gtk_widget_show (GTK_WIDGET (self->right_box));

		self->dev_frame = xfce_framebox_new (_ ("Device"), TRUE);
		gtk_widget_show (GTK_WIDGET (self->dev_frame));

		self->dev_label = GTK_LABEL (gtk_label_new (_("None")));
		gtk_widget_show (GTK_WIDGET (self->dev_label));

		self->useful_frame = xfce_framebox_new (_ ("Useful Controls"), TRUE);
		gtk_widget_show (GTK_WIDGET (self->useful_frame));

		self->useful_tv = GTK_TREE_VIEW (gtk_tree_view_new ());

		gtk_tree_view_set_headers_visible (self->useful_tv, FALSE);

		gtk_widget_show (GTK_WIDGET (self->useful_tv));

		self->useful_model = GTK_TREE_STORE (gtk_tree_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING));
		gtk_tree_view_set_model (self->useful_tv, GTK_TREE_MODEL (self->useful_model));

		gtk_tree_view_append_column (self->useful_tv, 
			tree_new_check_column ("", 0, GTK_TREE_MODEL (self->useful_model)
		));
		gtk_tree_view_append_column (self->useful_tv, 
			tree_new_text_column ("", 1
		));


		self->useful_sc = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
		gtk_scrolled_window_set_policy (self->useful_sc, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_container_add (GTK_CONTAINER (self->useful_sc), GTK_WIDGET (self->useful_tv));
		gtk_widget_show (GTK_WIDGET (self->useful_sc));

		xfce_framebox_add (XFCE_FRAMEBOX (self->useful_frame), GTK_WIDGET (self->useful_sc));
		xfce_framebox_add (XFCE_FRAMEBOX (self->dev_frame), GTK_WIDGET (self->dev_label));

		gtk_box_pack_start (GTK_BOX (self->right_box), GTK_WIDGET (self->dev_frame), FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (self->right_box), GTK_WIDGET (self->useful_frame), TRUE, TRUE, 0);

		gtk_box_pack_start (GTK_BOX (self->cols), GTK_WIDGET (self->devlist_sc), FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (self->cols), GTK_WIDGET (self->sep), FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (self->cols), GTK_WIDGET (self->right_box), FALSE, FALSE, 0);

		/*g_signal_connect_swapped (G_OBJECT (self->devlist_model), "row-changed", G_CALLBACK (self_device_changed_t_cb), self);*/

		sels = gtk_tree_view_get_selection (self->devlist_tv);
		if (sels) {
			g_signal_connect_swapped (G_OBJECT (sels), "changed", G_CALLBACK (self_device_changed_t_cb), self);
		}

		g_signal_connect_swapped (G_OBJECT (self), "destroy", G_CALLBACK (self_device_changed_d_cb), self);

		gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (self->cols), TRUE, TRUE, 0);

		self_devicelst_updated (self);


		gtk_widget_set_size_request (GTK_WIDGET (self), -1, 300);
	
#line 294 "xfce-mixer-settingsbox.c"
 }
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__


#line 33 "mixer-settingsbox.gob"
void 
xfce_mixer_settingsbox_load (XfceMixerSettingsbox * self)
#line 305 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::load"
#line 33 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 33 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 312 "xfce-mixer-settingsbox.c"
{
#line 34 "mixer-settingsbox.gob"
	
		if (!self->manager)
			return;

	}}
#line 320 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__


#line 127 "mixer-settingsbox.gob"
static void 
xfce_mixer_settingsbox_device_changed_d_cb (XfceMixerSettingsbox * self, gpointer user_data)
#line 327 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::device_changed_d_cb"
#line 127 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 127 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 334 "xfce-mixer-settingsbox.c"
{
#line 128 "mixer-settingsbox.gob"
	
		self_device_changed_cb (self);
	}}
#line 340 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__

#line 132 "mixer-settingsbox.gob"
static void 
xfce_mixer_settingsbox_device_changed_t_cb (XfceMixerSettingsbox * self, GtkTreeSelection * arg1)
#line 346 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::device_changed_t_cb"
#line 132 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 132 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 353 "xfce-mixer-settingsbox.c"
{
#line 135 "mixer-settingsbox.gob"
	
		self_device_changed_cb (self);
	}}
#line 359 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__

#line 139 "mixer-settingsbox.gob"
void 
xfce_mixer_settingsbox_device_changed_cb (XfceMixerSettingsbox * self)
#line 365 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::device_changed_cb"
#line 139 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 139 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 372 "xfce-mixer-settingsbox.c"
{
#line 140 "mixer-settingsbox.gob"
	
		GtkTreeSelection *sels;
		GtkTreeModel *m;
		GtkTreeIter iter;
		gchar *s;
		GList *g;
		GList *cl;
		volcontrol_t *vci;
		self_apply_right_box (self);

		sels = gtk_tree_view_get_selection (self->devlist_tv);
		if (!sels)
			return;

		if (!gtk_tree_selection_get_selected (sels, &m, &iter))
			return;

		gtk_tree_model_get (m, &iter, 0, &s, -1);
		if (s) {
			gtk_label_set_text (self->dev_label, s);
			vc_set_device (s);
			g_free (s);
		}

		/* fill "useful controls" list */
		gtk_tree_store_clear (self->useful_model);

		cl = vc_get_control_list ();
		g = cl;
		while (g) {
			vci = (volcontrol_t *) g->data;

			if (vci && vci->name) {
				gtk_tree_store_append (self->useful_model, &iter, NULL);
				gtk_tree_store_set (self->useful_model, &iter, 
					0, TRUE,
					1, vci->name, 
				-1);

				/*g_warning ("vci %s", vci->name);*/
			}

			g = g_list_next (g);
		}
		if (cl)
			vc_free_control_list (cl);
	}}
#line 422 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__

#line 188 "mixer-settingsbox.gob"
void 
xfce_mixer_settingsbox_devicelst_updated (XfceMixerSettingsbox * self)
#line 428 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::devicelst_updated"
#line 188 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 188 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 435 "xfce-mixer-settingsbox.c"
{
#line 189 "mixer-settingsbox.gob"
	
		GList *g;
		gchar const *s;
		GtkTreeIter iter;
		if (self->device_lst) {
			stringlist_free (self->device_lst);
			self->device_lst = NULL;
		}
		self->device_lst = vc_get_device_list ();
		gtk_tree_store_clear (self->devlist_model);
		g = self->device_lst;
		while (g) {
			s = (gchar const *) g->data;
			if (s
#ifndef SHOW_DEFAULT_DEVICE_ENTRY
	&& !g_str_equal (s, "default")
#endif
			) {
				gtk_tree_store_append (self->devlist_model, &iter, NULL);
				gtk_tree_store_set (self->devlist_model, &iter, 0, s, -1);
			}

			g = g_list_next (g);
		}
	}}
#line 463 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__

#line 215 "mixer-settingsbox.gob"
void 
xfce_mixer_settingsbox_apply_right_box (XfceMixerSettingsbox * self)
#line 469 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::apply_right_box"
#line 215 "mixer-settingsbox.gob"
	g_return_if_fail (self != NULL);
#line 215 "mixer-settingsbox.gob"
	g_return_if_fail (XFCE_IS_MIXER_SETTINGSBOX (self));
#line 476 "xfce-mixer-settingsbox.c"
{
#line 216 "mixer-settingsbox.gob"
	
		/* TODO */
	}}
#line 482 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__

#line 220 "mixer-settingsbox.gob"
XfceMixerSettingsbox * 
xfce_mixer_settingsbox_new (void)
#line 488 "xfce-mixer-settingsbox.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Settingsbox::new"
{
#line 221 "mixer-settingsbox.gob"
	
		return XFCE_MIXER_SETTINGSBOX (GET_NEW);
	}}
#line 496 "xfce-mixer-settingsbox.c"
#undef __GOB_FUNCTION__


#if (!defined __GNUC__) || (defined __GNUC__ && defined __STRICT_ANSI__)
/*REALLY BAD HACK
  This is to avoid unused warnings if you don't call
  some method.  I need to find a better way to do
  this, not needed in GCC since we use some gcc
  extentions to make saner, faster code */
static void
___xfce_mixer_settingsbox_really_bad_hack_to_avoid_warnings(void)
{
	((void (*)(void))GET_NEW_VARG)();
	((void (*)(void))self_load)();
	((void (*)(void))self_device_changed_d_cb)();
	((void (*)(void))self_device_changed_t_cb)();
	((void (*)(void))self_device_changed_cb)();
	((void (*)(void))self_devicelst_updated)();
	((void (*)(void))self_apply_right_box)();
	((void (*)(void))self_new)();
	___xfce_mixer_settingsbox_really_bad_hack_to_avoid_warnings();
}
#endif /* !__GNUC__ || (__GNUC__ && __STRICT_ANSI__) */

/* Generated by GOB (v2.0.6) on Wed Jan 21 01:45:20 2004
   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 6

#define selfp (self->_priv)

#include <string.h> /* memset() */

#include "xfce-mixer-pxml.h"

#include "xfce-mixer-pxml-private.h"

#ifdef G_LIKELY
#define ___GOB_LIKELY(expr) G_LIKELY(expr)
#define ___GOB_UNLIKELY(expr) G_UNLIKELY(expr)
#else /* ! G_LIKELY */
#define ___GOB_LIKELY(expr) (expr)
#define ___GOB_UNLIKELY(expr) (expr)
#endif /* G_LIKELY */

#line 1 "mixer-pxml.gob"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#line 33 "xfce-mixer-pxml.c"

#line 6 "mixer-pxml.gob"


#include <gtk/gtk.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


#line 43 "xfce-mixer-pxml.c"
/* self casting macros */
#define SELF(x) XFCE_MIXER_PXML(x)
#define SELF_CONST(x) XFCE_MIXER_PXML_CONST(x)
#define IS_SELF(x) XFCE_IS_MIXER_PXML(x)
#define TYPE_SELF XFCE_TYPE_MIXER_PXML
#define SELF_CLASS(x) XFCE_MIXER_PXML_CLASS(x)

#define SELF_GET_CLASS(x) XFCE_MIXER_PXML_GET_CLASS(x)

/* self typedefs */
typedef XfceMixerPxml Self;
typedef XfceMixerPxmlClass SelfClass;

/* here are local prototypes */
static void ___object_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void ___object_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void xfce_mixer_pxml_class_init (XfceMixerPxmlClass * c) G_GNUC_UNUSED;
static void xfce_mixer_pxml_init (XfceMixerPxml * self) G_GNUC_UNUSED;

enum {
	PROP_0,
	PROP_FNAME
};

/* pointer to the class of our parent */
static GObjectClass *parent_class = NULL;

/* Short form macros */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define self_set_document(args...) xfce_mixer_pxml_set_document(args)
#define self_set_file(args...) xfce_mixer_pxml_set_file(args)
#define self_eat_children(args...) xfce_mixer_pxml_eat_children(args)
#define self_goto_next(args...) xfce_mixer_pxml_goto_next(args)
#define self_check_tag(args...) xfce_mixer_pxml_check_tag(args)
#define self_goto_child_tag(args...) xfce_mixer_pxml_goto_child_tag(args)
#define self_has_parent(args...) xfce_mixer_pxml_has_parent(args)
#define self_goto_parent(args...) xfce_mixer_pxml_goto_parent(args)
#define self_goto_children(args...) xfce_mixer_pxml_goto_children(args)
#define self_goto_root(args...) xfce_mixer_pxml_goto_root(args)
#define self_goto_node(args...) xfce_mixer_pxml_goto_node(args)
#define self_has_children(args...) xfce_mixer_pxml_has_children(args)
#define self_get_prop(args...) xfce_mixer_pxml_get_prop(args)
#define self_set_prop(args...) xfce_mixer_pxml_set_prop(args)
#define self_create_text_child(args...) xfce_mixer_pxml_create_text_child(args)
#define self_new(args...) xfce_mixer_pxml_new(args)
#define self_get_text_child(args...) xfce_mixer_pxml_get_text_child(args)
#endif /* __GNUC__ && !__STRICT_ANSI__ */

/* Short form pointers */
static void (* const self_set_document) (XfceMixerPxml * self, xmlDocPtr doc) = xfce_mixer_pxml_set_document;
static void (* const self_set_file) (XfceMixerPxml * self, gchar const * fnamen) = xfce_mixer_pxml_set_file;
static void (* const self_eat_children) (XfceMixerPxml * self, GList * exceptions) = xfce_mixer_pxml_eat_children;
static gboolean (* const self_goto_next) (XfceMixerPxml * self) = xfce_mixer_pxml_goto_next;
static gboolean (* const self_check_tag) (XfceMixerPxml * self, gchar const * tagname) = xfce_mixer_pxml_check_tag;
static gboolean (* const self_goto_child_tag) (XfceMixerPxml * self, gchar const * tagname) = xfce_mixer_pxml_goto_child_tag;
static gboolean (* const self_has_parent) (XfceMixerPxml * self) = xfce_mixer_pxml_has_parent;
static void (* const self_goto_parent) (XfceMixerPxml * self) = xfce_mixer_pxml_goto_parent;
static void (* const self_goto_children) (XfceMixerPxml * self) = xfce_mixer_pxml_goto_children;
static void (* const self_goto_root) (XfceMixerPxml * self) = xfce_mixer_pxml_goto_root;
static void (* const self_goto_node) (XfceMixerPxml * self, xmlNodePtr nn) = xfce_mixer_pxml_goto_node;
static gboolean (* const self_has_children) (XfceMixerPxml * self) = xfce_mixer_pxml_has_children;
static gchar * (* const self_get_prop) (XfceMixerPxml * self, gchar const * pname) = xfce_mixer_pxml_get_prop;
static void (* const self_set_prop) (XfceMixerPxml * self, gchar const * pname, gchar const * value) = xfce_mixer_pxml_set_prop;
static xmlNodePtr (* const self_create_text_child) (XfceMixerPxml * self, gchar const * name, gchar const * opt_text) = xfce_mixer_pxml_create_text_child;
static XfceMixerPxml * (* const self_new) (gchar const * root) = xfce_mixer_pxml_new;
static gchar * (* const self_get_text_child) (XfceMixerPxml * self) = xfce_mixer_pxml_get_text_child;

GType
xfce_mixer_pxml_get_type (void)
{
	static GType type = 0;

	if ___GOB_UNLIKELY(type == 0) {
		static const GTypeInfo info = {
			sizeof (XfceMixerPxmlClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) xfce_mixer_pxml_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (XfceMixerPxml),
			0 /* n_preallocs */,
			(GInstanceInitFunc) xfce_mixer_pxml_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT, "XfceMixerPxml", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((XfceMixerPxml *)g_object_new(xfce_mixer_pxml_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static XfceMixerPxml * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static XfceMixerPxml *
GET_NEW_VARG (const char *first, ...)
{
	XfceMixerPxml *ret;
	va_list ap;
	va_start (ap, first);
	ret = (XfceMixerPxml *)g_object_new_valist (xfce_mixer_pxml_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___finalize(GObject *obj_self)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::finalize"
	XfceMixerPxml *self = XFCE_MIXER_PXML (obj_self);
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
#line 16 "mixer-pxml.gob"
	if(self->root) { ((*(void (*)(void *))g_free)) (self->root); self->root = NULL; }
#line 162 "xfce-mixer-pxml.c"
#define fname (self->fname)
#define VAR fname
	{
#line 19 "mixer-pxml.gob"
	
			self_set_file (self, NULL);
		}
#line 170 "xfce-mixer-pxml.c"
	memset(&fname, 0, sizeof(fname));
#undef VAR
#undef fname
#line 36 "mixer-pxml.gob"
	if(self->document) { ((*(void (*)(void *))xmlFreeDoc)) (self->document); self->document = NULL; }
#line 176 "xfce-mixer-pxml.c"
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__

static void 
xfce_mixer_pxml_class_init (XfceMixerPxmlClass * c)
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::class_init"
	GObjectClass *g_object_class = (GObjectClass*) c;

	parent_class = g_type_class_ref (G_TYPE_OBJECT);

	g_object_class->finalize = ___finalize;
	g_object_class->get_property = ___object_get_property;
	g_object_class->set_property = ___object_set_property;
    {
	GParamSpec   *param_spec;

	param_spec = g_param_spec_string
		("fname" /* name */,
		 "File name and path" /* nick */,
		 "Filename and path to use, if any" /* blurb */,
		 NULL /* default_value */,
		 (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property (g_object_class,
		PROP_FNAME,
		param_spec);
    }
	return;
	c = NULL;
	g_object_class = NULL;
}
#undef __GOB_FUNCTION__
#line 97 "mixer-pxml.gob"
static void 
xfce_mixer_pxml_init (XfceMixerPxml * self)
#line 214 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::init"
#line 6 "mixer-pxml.gob"
	self->root = NULL;
#line 219 "xfce-mixer-pxml.c"
#line 6 "mixer-pxml.gob"
	self->fname = NULL;
#line 222 "xfce-mixer-pxml.c"
#line 30 "mixer-pxml.gob"
	self->node = NULL;
#line 225 "xfce-mixer-pxml.c"
#line 30 "mixer-pxml.gob"
	self->document = NULL;
#line 228 "xfce-mixer-pxml.c"
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__

static void
___object_set_property (GObject *object,
	guint property_id,
	const GValue *VAL,
	GParamSpec *pspec)
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::set_property"
{
	XfceMixerPxml *self;

	self = XFCE_MIXER_PXML (object);

	switch (property_id) {
	case PROP_FNAME:
		{
#line 30 "mixer-pxml.gob"

		self_set_file (self, g_value_get_string (VAL));
	
#line 252 "xfce-mixer-pxml.c"
		}
		break;
	default:
/* Apparently in g++ this is needed, glib is b0rk */
#ifndef __PRETTY_FUNCTION__
#  undef G_STRLOC
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
	return;
	self = NULL;
	VAL = NULL;
	pspec = NULL;
}
#undef __GOB_FUNCTION__

static void
___object_get_property (GObject *object,
	guint property_id,
	GValue *VAL,
	GParamSpec *pspec)
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::get_property"
{
	XfceMixerPxml *self;

	self = XFCE_MIXER_PXML (object);

	switch (property_id) {
	case PROP_FNAME:
		{
#line 27 "mixer-pxml.gob"

		g_value_set_string (VAL, self->fname);
	
#line 289 "xfce-mixer-pxml.c"
		}
		break;
	default:
/* Apparently in g++ this is needed, glib is b0rk */
#ifndef __PRETTY_FUNCTION__
#  undef G_STRLOC
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
	return;
	self = NULL;
	VAL = NULL;
	pspec = NULL;
}
#undef __GOB_FUNCTION__


#line 38 "mixer-pxml.gob"
void 
xfce_mixer_pxml_set_document (XfceMixerPxml * self, xmlDocPtr doc)
#line 312 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::set_document"
#line 38 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 38 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 319 "xfce-mixer-pxml.c"
{
#line 39 "mixer-pxml.gob"
	
		self->document = doc;
	}}
#line 325 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 43 "mixer-pxml.gob"
void 
xfce_mixer_pxml_set_file (XfceMixerPxml * self, gchar const * fnamen)
#line 331 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::set_file"
#line 43 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 43 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 338 "xfce-mixer-pxml.c"
{
#line 44 "mixer-pxml.gob"
	
		gchar *dir;
		self->node = NULL;

		if (self->fname && self->document) {
			/* store blahblah if required */
			xmlSaveFormatFile (self->fname, self->document, 1);
			xmlFreeDoc (self->document);
			self->document = NULL;

			g_free (self->fname);
			self->fname = NULL;
		}

		if (fnamen) {
			self->fname = g_strdup (fnamen);
			if (!self->fname) {
				/* eeek TODO exception */
			}

			if (g_file_test (self->fname, G_FILE_TEST_EXISTS)) {
				/* TODO backup */
			} else {
				/* create directory */
				dir = g_path_get_dirname (self->fname);
				if (dir && !g_file_test (dir, G_FILE_TEST_IS_DIR))
					mkdir (dir, S_IRWXU | S_IRGRP 
					| S_IXGRP | S_IROTH | S_IXOTH);

				if (dir)
					g_free (dir);
			}

			/* create document etc */
			self->document = xmlParseFile (self->fname);

			if (!self->document && self->root) {
				self->document = xmlNewDoc ("1.0");
				self->document->children = xmlNewDocRawNode (self->document, NULL, self->root, NULL);
				xmlDocSetRootElement (self->document, self->document->children);
			}

			if (!self->document)
				return;

			self_goto_root (self);
			if (!self->node) { /* no root */
				xmlFreeDoc (self->document);
				self->document = NULL;
			}
		}
	}}
#line 393 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__


#line 101 "mixer-pxml.gob"
void 
xfce_mixer_pxml_eat_children (XfceMixerPxml * self, GList * exceptions)
#line 400 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::eat_children"
#line 101 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 101 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 407 "xfce-mixer-pxml.c"
{
#line 102 "mixer-pxml.gob"
	
		xmlNodePtr nodex;
		xmlNodePtr nnodex;
		GList *g;
		gchar *tagname;
		gboolean found;
		/* todo */
		if (!self->node)
			return;

		nodex = self->node->children;
		if (!nodex)
			return;

		while (nodex) {
			nnodex = nodex->next;

			/* if not nodex.tagname in exceptions:
				nodex.del()
			*/
			tagname = (gchar *)nodex->name;
			found = FALSE;
			g = exceptions;
			while (g) {
				if (g_str_equal ((gchar *)g->data, tagname)) {
					found = TRUE;
					break;
				}
				g = g_list_next (g);
			}

			if (!found) {
				/* delete node */
				xmlUnlinkNode (nodex);
				xmlFreeNode (nodex);
			}

			nodex = nnodex;
		}
	}}
#line 450 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 143 "mixer-pxml.gob"
gboolean 
xfce_mixer_pxml_goto_next (XfceMixerPxml * self)
#line 456 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_next"
#line 143 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 143 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gboolean )0);
#line 463 "xfce-mixer-pxml.c"
{
#line 144 "mixer-pxml.gob"
	
		if (!self->node)
			return FALSE;

		self->node = self->node->next;
		if (self->node)
			return TRUE;

		return FALSE;
	}}
#line 476 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 155 "mixer-pxml.gob"
gboolean 
xfce_mixer_pxml_check_tag (XfceMixerPxml * self, gchar const * tagname)
#line 482 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::check_tag"
#line 155 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 155 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gboolean )0);
#line 489 "xfce-mixer-pxml.c"
{
#line 156 "mixer-pxml.gob"
	
		if (!self->node)
			return FALSE;

		return (xmlStrEqual(self->node->name, (const xmlChar *)tagname));
	}}
#line 498 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 163 "mixer-pxml.gob"
gboolean 
xfce_mixer_pxml_goto_child_tag (XfceMixerPxml * self, gchar const * tagname)
#line 504 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_child_tag"
#line 163 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 163 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gboolean )0);
#line 511 "xfce-mixer-pxml.c"
{
#line 164 "mixer-pxml.gob"
	
		xmlNodePtr nodex;
		if (!self->node)
			return FALSE;

		nodex = self->node->children;
		while (nodex) {
			if (xmlStrEqual(self->node->name, (const xmlChar *)tagname)) {
				self->node = nodex;
				return TRUE;
			}
			nodex = nodex->next;
		}

		return FALSE;
	}}
#line 530 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 181 "mixer-pxml.gob"
gboolean 
xfce_mixer_pxml_has_parent (XfceMixerPxml * self)
#line 536 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::has_parent"
#line 181 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 181 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gboolean )0);
#line 543 "xfce-mixer-pxml.c"
{
#line 182 "mixer-pxml.gob"
	
		if (!self->node)
			return FALSE;

		return (self->node->parent != NULL);
	}}
#line 552 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 189 "mixer-pxml.gob"
void 
xfce_mixer_pxml_goto_parent (XfceMixerPxml * self)
#line 558 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_parent"
#line 189 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 189 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 565 "xfce-mixer-pxml.c"
{
#line 190 "mixer-pxml.gob"
	
		if (!self->node)
			return;

		self->node = self->node->parent;
	}}
#line 574 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 197 "mixer-pxml.gob"
void 
xfce_mixer_pxml_goto_children (XfceMixerPxml * self)
#line 580 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_children"
#line 197 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 197 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 587 "xfce-mixer-pxml.c"
{
#line 198 "mixer-pxml.gob"
	
		if (!self->node)
			return;

		self->node = self->node->children;
	}}
#line 596 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 205 "mixer-pxml.gob"
void 
xfce_mixer_pxml_goto_root (XfceMixerPxml * self)
#line 602 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_root"
#line 205 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 205 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 609 "xfce-mixer-pxml.c"
{
#line 206 "mixer-pxml.gob"
	
		if (!self->document)
			return;

		self->node = xmlDocGetRootElement (self->document);

		if (self->root && !self_check_tag (self, self->root)) {
			self->node = NULL;
		}
	}}
#line 622 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 217 "mixer-pxml.gob"
void 
xfce_mixer_pxml_goto_node (XfceMixerPxml * self, xmlNodePtr nn)
#line 628 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::goto_node"
#line 217 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 217 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 635 "xfce-mixer-pxml.c"
{
#line 218 "mixer-pxml.gob"
	
		self->node = nn;
	}}
#line 641 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 222 "mixer-pxml.gob"
gboolean 
xfce_mixer_pxml_has_children (XfceMixerPxml * self)
#line 647 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::has_children"
#line 222 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 222 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gboolean )0);
#line 654 "xfce-mixer-pxml.c"
{
#line 223 "mixer-pxml.gob"
	
		if (!self->node)
			return FALSE;

		return (self->node->children != NULL);
	}}
#line 663 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 230 "mixer-pxml.gob"
gchar * 
xfce_mixer_pxml_get_prop (XfceMixerPxml * self, gchar const * pname)
#line 669 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::get_prop"
#line 230 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gchar * )0);
#line 230 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gchar * )0);
#line 676 "xfce-mixer-pxml.c"
{
#line 231 "mixer-pxml.gob"
	
		return xmlGetProp (self->node, pname);
	}}
#line 682 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 235 "mixer-pxml.gob"
void 
xfce_mixer_pxml_set_prop (XfceMixerPxml * self, gchar const * pname, gchar const * value)
#line 688 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::set_prop"
#line 235 "mixer-pxml.gob"
	g_return_if_fail (self != NULL);
#line 235 "mixer-pxml.gob"
	g_return_if_fail (XFCE_IS_MIXER_PXML (self));
#line 695 "xfce-mixer-pxml.c"
{
#line 236 "mixer-pxml.gob"
	
		xmlSetProp (self->node, pname, (const xmlChar *) value);
	}}
#line 701 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 240 "mixer-pxml.gob"
xmlNodePtr 
xfce_mixer_pxml_create_text_child (XfceMixerPxml * self, gchar const * name, gchar const * opt_text)
#line 707 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::create_text_child"
#line 240 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (xmlNodePtr )0);
#line 240 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (xmlNodePtr )0);
#line 714 "xfce-mixer-pxml.c"
{
#line 241 "mixer-pxml.gob"
	
		if (!self->node)
			return NULL;

		return xmlNewTextChild (self->node, NULL, (const xmlChar *)name, (const xmlChar *) opt_text);
	}}
#line 723 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 248 "mixer-pxml.gob"
XfceMixerPxml * 
xfce_mixer_pxml_new (gchar const * root)
#line 729 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::new"
{
#line 249 "mixer-pxml.gob"
	
		XfceMixerPxml *p;
		p = XFCE_MIXER_PXML (GET_NEW);
		if (root)
			p->root = g_strdup (root);
		return p;
	}}
#line 741 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__

#line 257 "mixer-pxml.gob"
gchar * 
xfce_mixer_pxml_get_text_child (XfceMixerPxml * self)
#line 747 "xfce-mixer-pxml.c"
{
#define __GOB_FUNCTION__ "Xfce:Mixer:Pxml::get_text_child"
#line 257 "mixer-pxml.gob"
	g_return_val_if_fail (self != NULL, (gchar * )0);
#line 257 "mixer-pxml.gob"
	g_return_val_if_fail (XFCE_IS_MIXER_PXML (self), (gchar * )0);
#line 754 "xfce-mixer-pxml.c"
{
#line 258 "mixer-pxml.gob"
	
		if (!self->node)
			return NULL;

		return xmlNodeListGetString(self->document, self->node->children, 1);
	}}
#line 763 "xfce-mixer-pxml.c"
#undef __GOB_FUNCTION__


#if (!defined __GNUC__) || (defined __GNUC__ && defined __STRICT_ANSI__)
/*REALLY BAD HACK
  This is to avoid unused warnings if you don't call
  some method.  I need to find a better way to do
  this, not needed in GCC since we use some gcc
  extentions to make saner, faster code */
static void
___xfce_mixer_pxml_really_bad_hack_to_avoid_warnings(void)
{
	((void (*)(void))GET_NEW_VARG)();
	((void (*)(void))self_set_document)();
	((void (*)(void))self_set_file)();
	((void (*)(void))self_eat_children)();
	((void (*)(void))self_goto_next)();
	((void (*)(void))self_check_tag)();
	((void (*)(void))self_goto_child_tag)();
	((void (*)(void))self_has_parent)();
	((void (*)(void))self_goto_parent)();
	((void (*)(void))self_goto_children)();
	((void (*)(void))self_goto_root)();
	((void (*)(void))self_goto_node)();
	((void (*)(void))self_has_children)();
	((void (*)(void))self_get_prop)();
	((void (*)(void))self_set_prop)();
	((void (*)(void))self_create_text_child)();
	((void (*)(void))self_new)();
	((void (*)(void))self_get_text_child)();
	___xfce_mixer_pxml_really_bad_hack_to_avoid_warnings();
}
#endif /* !__GNUC__ || (__GNUC__ && __STRICT_ANSI__) */


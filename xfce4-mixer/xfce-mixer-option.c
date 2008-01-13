/* $Id: xfce-menu.h 25273 2008-03-23 19:20:47Z jannis $ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include "xfce-mixer-option.h"



static void xfce_mixer_option_class_init      (XfceMixerOptionClass *klass);
static void xfce_mixer_option_init            (XfceMixerOption      *option);
static void xfce_mixer_option_dispose         (GObject              *object);
static void xfce_mixer_option_finalize        (GObject              *object);
static void xfce_mixer_option_create_contents (XfceMixerOption      *option);
static void xfce_mixer_option_changed         (GtkComboBox          *combo,
                                               XfceMixerOption      *option);



struct _XfceMixerOptionClass
{
  GtkHBoxClass __parent__;
};

struct _XfceMixerOption
{
  GtkHBox __parent__;

  GstMixer      *element;
  GstMixerTrack *track;
};



static GObjectClass *xfce_mixer_option_parent_class = NULL;



GType
xfce_mixer_option_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerOptionClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_option_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerOption),
          0,
          (GInstanceInitFunc) xfce_mixer_option_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_HBOX, "XfceMixerOption", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_option_class_init (XfceMixerOptionClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_option_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_option_dispose;
  gobject_class->finalize = xfce_mixer_option_finalize;
}



static void
xfce_mixer_option_init (XfceMixerOption *option)
{
}



static void
xfce_mixer_option_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_option_parent_class)->dispose) (object);
}



static void
xfce_mixer_option_finalize (GObject *object)
{
  XfceMixerOption *option = XFCE_MIXER_OPTION (object);

  gst_object_unref (option->element);
  gst_object_unref (option->track);

  (*G_OBJECT_CLASS (xfce_mixer_option_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_option_new (GstElement    *element,
                       GstMixerTrack *track)
{
  XfceMixerOption *option;

  g_return_val_if_fail (GST_IS_MIXER (element), NULL);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), NULL);
  
  option = g_object_new (TYPE_XFCE_MIXER_OPTION, NULL);
  option->element = gst_object_ref (element);
  option->track = gst_object_ref (track);

  xfce_mixer_option_create_contents (option);

  return GTK_WIDGET (option);
}



static void
xfce_mixer_option_create_contents (XfceMixerOption *option)
{
  GstMixerOptions *options;
  GtkWidget       *combo;
  GtkWidget       *label;
  const GList     *iter;
  const gchar     *active_option;
  gchar           *title;
  gint             i;

  gtk_box_set_homogeneous (GTK_BOX (option), FALSE);
  gtk_box_set_spacing (GTK_BOX (option), 12);

  title = g_strdup_printf ("%s:", option->track->label);

  label = gtk_label_new (title);
  gtk_box_pack_start (GTK_BOX (option), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  options = GST_MIXER_OPTIONS (option->track);
  active_option = gst_mixer_get_option (GST_MIXER (option->element), GST_MIXER_OPTIONS (option->track));

  combo = gtk_combo_box_new_text ();

  for (iter = options->values, i = 0; iter != NULL; iter = g_list_next (iter), ++i)
    {
      gtk_combo_box_append_text (GTK_COMBO_BOX (combo), iter->data);

      if (G_UNLIKELY (g_utf8_collate (active_option, iter->data) == 0))
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), i);
    }
  
  gtk_box_pack_start (GTK_BOX (option), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);

  g_signal_connect (combo, "changed", G_CALLBACK (xfce_mixer_option_changed), option);

  g_free (title);
}



static void 
xfce_mixer_option_changed (GtkComboBox     *combo,
                           XfceMixerOption *option)
{
  gchar *active_option;

  active_option = gtk_combo_box_get_active_text (combo);

  if (G_LIKELY (active_option != NULL))
    {
      gst_mixer_set_option (GST_MIXER (option->element), GST_MIXER_OPTIONS (option->track), active_option);
      g_free (active_option);
    }
}

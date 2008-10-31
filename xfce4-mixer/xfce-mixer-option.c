/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/libxfce4mixer.h"

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

  GstElement    *card;
  GstMixerTrack *track;

  GtkWidget     *combo;

  gboolean       ignore_signals;
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
  option->ignore_signals = FALSE;
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

  (*G_OBJECT_CLASS (xfce_mixer_option_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_option_new (GstElement    *card,
                       GstMixerTrack *track)
{
  XfceMixerOption *option;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), NULL);
  
  option = g_object_new (TYPE_XFCE_MIXER_OPTION, NULL);
  option->card = card;
  option->track = track;

  xfce_mixer_option_create_contents (option);

  return GTK_WIDGET (option);
}



static void
xfce_mixer_option_create_contents (XfceMixerOption *option)
{
  GstMixerOptions *options;
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
  active_option = gst_mixer_get_option (GST_MIXER (option->card), options);

  option->combo = gtk_combo_box_new_text ();

  for (iter = options->values, i = 0; iter != NULL; iter = g_list_next (iter), ++i)
    {
      gtk_combo_box_append_text (GTK_COMBO_BOX (option->combo), iter->data);

      if (G_UNLIKELY (g_utf8_collate (active_option, iter->data) == 0))
        gtk_combo_box_set_active (GTK_COMBO_BOX (option->combo), i);
    }
  
  gtk_box_pack_start (GTK_BOX (option), option->combo, FALSE, FALSE, 0);
  gtk_widget_show (option->combo);

  g_signal_connect (option->combo, "changed", G_CALLBACK (xfce_mixer_option_changed), option);

  g_free (title);
}



static void 
xfce_mixer_option_changed (GtkComboBox     *combo,
                           XfceMixerOption *option)
{
  gchar *active_option;

  if (G_UNLIKELY (option->ignore_signals))
    return;

  active_option = gtk_combo_box_get_active_text (combo);

  if (G_LIKELY (active_option != NULL))
    {
      gst_mixer_set_option (GST_MIXER (option->card), option->track, active_option);
      g_free (active_option);
    }
}



void
xfce_mixer_option_update (XfceMixerOption *option)
{
  GstMixerOptions *options;
  GtkTreeModel    *model;
  GtkTreeIter      iter;
  const gchar     *active_option;
  gchar           *str;

  g_return_if_fail (IS_XFCE_MIXER_OPTION (option));

  options = GST_MIXER_OPTIONS (option->track);
  active_option = gst_mixer_get_option (GST_MIXER (option->card), options);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (option->combo));

  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      do 
        {
          gtk_tree_model_get (model, &iter, 0, &str, -1);

          if (G_UNLIKELY (g_utf8_collate (str, active_option) == 0))
            {
              option->ignore_signals = TRUE;
              gtk_combo_box_set_active_iter (GTK_COMBO_BOX (option->combo), &iter);
              option->ignore_signals = FALSE;

              g_free (str);
              
              break;
            }

          g_free (str);
        }
      while (gtk_tree_model_iter_next (model, &iter));
    }
}

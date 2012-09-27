/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-option.h"



#define OPTION_COLUMN 0



static void xfce_mixer_option_class_init      (XfceMixerOptionClass *klass);
static void xfce_mixer_option_init            (XfceMixerOption      *option);
static void xfce_mixer_option_dispose         (GObject              *object);
static void xfce_mixer_option_finalize        (GObject              *object);
static void xfce_mixer_option_create_contents (XfceMixerOption      *option);
static void xfce_mixer_option_changed         (GtkComboBox          *combo,
                                               XfceMixerOption      *option);
static void xfce_mixer_option_bus_message     (GstBus               *bus,
                                               GstMessage           *message,
                                               XfceMixerOption      *option);



struct _XfceMixerOptionClass
{
  GtkHBoxClass __parent__;
};

struct _XfceMixerOption
{
  GtkHBox __parent__;

  GtkListStore  *list_store;

  GstElement    *card;
  GstMixerTrack *track;
  guint          signal_handler_id;

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

  option->signal_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_option_bus_message), option);
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

  if (option->signal_handler_id > 0)
    {
      xfce_mixer_bus_disconnect (option->signal_handler_id);
      option->signal_handler_id = 0;
    }

  gtk_list_store_clear (option->list_store);
  g_object_unref (option->list_store);
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
  GtkCellRenderer *renderer;
  const GList     *options_iter;
  GtkTreeIter      tree_iter;
  const gchar     *active_option;
  const gchar     *track_label;
  gchar           *title;
  gint             i;
  gint             active_index = 0;

  gtk_box_set_homogeneous (GTK_BOX (option), FALSE);
  gtk_box_set_spacing (GTK_BOX (option), 12);

  track_label = xfce_mixer_get_track_label (option->track);
  title = g_strdup_printf ("%s:", track_label);

  label = gtk_label_new (title);
  gtk_box_pack_start (GTK_BOX (option), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  options = GST_MIXER_OPTIONS (option->track);
  active_option = gst_mixer_get_option (GST_MIXER (option->card), options);

  option->list_store = gtk_list_store_new (2, G_TYPE_STRING, GST_TYPE_MIXER_TRACK);

  for (options_iter = gst_mixer_options_get_values (options), i = 0; options_iter != NULL; options_iter = g_list_next (options_iter), ++i)
    {
      gtk_list_store_append (option->list_store, &tree_iter);
      gtk_list_store_set (option->list_store, &tree_iter, OPTION_COLUMN, options_iter->data, -1);

      if (G_UNLIKELY (g_utf8_collate (active_option, options_iter->data) == 0))
        active_index = i;
    }

  option->combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (option->list_store));
  gtk_box_pack_start (GTK_BOX (option), option->combo, FALSE, FALSE, 0);
  /* Make read-only options insensitive */
  if (GST_MIXER_TRACK_HAS_FLAG (option->track, GST_MIXER_TRACK_READONLY))
    gtk_widget_set_sensitive (option->combo, FALSE);
  gtk_combo_box_set_active (GTK_COMBO_BOX (option->combo), active_index);
  gtk_widget_show (option->combo);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (option->combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (option->combo), renderer, "text", OPTION_COLUMN);

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
      gst_mixer_set_option (GST_MIXER (option->card), GST_MIXER_OPTIONS (option->track), 
                            active_option);
      g_free (active_option);
    }
}



void
xfce_mixer_option_update (XfceMixerOption *option)
{
  GstMixerOptions *options;
  GtkTreeIter      iter;
  gboolean         valid_iter;
  const gchar     *active_option;
  gchar           *current_option;

  g_return_if_fail (IS_XFCE_MIXER_OPTION (option));

  options = GST_MIXER_OPTIONS (option->track);
  active_option = gst_mixer_get_option (GST_MIXER (option->card), options);

  valid_iter = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (option->list_store), &iter);

  while (valid_iter)
    {
      gtk_tree_model_get (GTK_TREE_MODEL (option->list_store), &iter, OPTION_COLUMN, &current_option, -1);

      if (G_UNLIKELY (g_utf8_collate (current_option, active_option) == 0))
        {
          option->ignore_signals = TRUE;
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (option->combo), &iter);
          option->ignore_signals = FALSE;

          g_free (current_option);

          break;
        }

      g_free (current_option);

      valid_iter = gtk_tree_model_iter_next (GTK_TREE_MODEL (option->list_store), &iter);
    }
}



static void
xfce_mixer_option_bus_message (GstBus          *bus,
                               GstMessage      *message,
                               XfceMixerOption *option)
{
  GstMixerOptions *options = NULL;
  const gchar     *active_option;
  const GList     *options_iter;
  gint             i;
  GtkTreeIter      tree_iter;

  if (!GST_IS_MIXER (option->card) || !GST_IS_MIXER_TRACK (option->track) || GST_MESSAGE_SRC (message) != GST_OBJECT (option->card))
    return;

  /* Rebuild option list if the options have changed */
  if (gst_mixer_message_get_type (message) == GST_MIXER_MESSAGE_OPTIONS_LIST_CHANGED)
    {
      gst_mixer_message_parse_options_list_changed (message, &options);
      if (GST_MIXER_TRACK (options) == option->track)
        {
          /*
           * Remember the active option and try to restore it while updating
           * the list of options
           */
          active_option = gst_mixer_get_option (GST_MIXER (option->card), options);

          gtk_list_store_clear (option->list_store);

          for (options_iter = gst_mixer_options_get_values (options), i = 0; options_iter != NULL; options_iter = g_list_next (options_iter), ++i)
            {
              gtk_list_store_append (option->list_store, &tree_iter);
              gtk_list_store_set (option->list_store, &tree_iter, OPTION_COLUMN, options_iter->data, -1);

              if (G_UNLIKELY (g_utf8_collate (active_option, options_iter->data) == 0))
                gtk_combo_box_set_active (GTK_COMBO_BOX (option->combo), i);
            }
        }
    }
}


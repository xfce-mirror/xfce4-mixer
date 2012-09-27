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

#include <glib.h>

#include <gtk/gtk.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include "libxfce4mixer.h"
#include "xfce-mixer-track-type.h"
#include "xfce-mixer-track-combo.h"



#define NAME_COLUMN  0
#define TRACK_COLUMN 1



enum 
{
  TRACK_CHANGED,
  LAST_SIGNAL,
};



static guint combo_signals[LAST_SIGNAL];



static void  xfce_mixer_track_combo_class_init (XfceMixerTrackComboClass *klass);
static void  xfce_mixer_track_combo_init       (XfceMixerTrackCombo      *combo);
static void  xfce_mixer_track_combo_finalize   (GObject                  *object);
static void  xfce_mixer_track_combo_changed    (XfceMixerTrackCombo      *combo);



struct _XfceMixerTrackComboClass
{
  GtkComboBoxClass __parent__;
};

struct _XfceMixerTrackCombo
{
  GtkComboBox    __parent__;

  GtkListStore  *list_store;

  GstElement    *card;
  GstMixerTrack *track;
};



static GObjectClass *xfce_mixer_track_combo_parent_class = NULL;



GType
xfce_mixer_track_combo_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerTrackComboClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_track_combo_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerTrackCombo),
          0,
          (GInstanceInitFunc) xfce_mixer_track_combo_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_COMBO_BOX, "XfceMixerTrackCombo", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_track_combo_class_init (XfceMixerTrackComboClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_track_combo_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_mixer_track_combo_finalize;

  combo_signals[TRACK_CHANGED] = g_signal_new ("track-changed",
                                               G_TYPE_FROM_CLASS (klass),
                                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                               0,
                                               NULL,
                                               NULL,
                                               g_cclosure_marshal_VOID__OBJECT,
                                               G_TYPE_NONE,
                                               1,
                                               GST_TYPE_MIXER_TRACK);
}



static void
xfce_mixer_track_combo_init (XfceMixerTrackCombo *combo)
{
  GtkCellRenderer *renderer;

  combo->card = NULL;

  combo->list_store = gtk_list_store_new (2, G_TYPE_STRING, GST_TYPE_MIXER_TRACK);
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (combo->list_store));

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", NAME_COLUMN);

  g_signal_connect_swapped (combo, "changed", G_CALLBACK (xfce_mixer_track_combo_changed), combo);
}



static void
xfce_mixer_track_combo_finalize (GObject *object)
{
  XfceMixerTrackCombo *combo = XFCE_MIXER_TRACK_COMBO (object);

  gtk_list_store_clear (combo->list_store);
  g_object_unref (combo->list_store);

  (*G_OBJECT_CLASS (xfce_mixer_track_combo_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_track_combo_new (GstElement    *card,
                            GstMixerTrack *track)
{
  GtkWidget           *widget;
  XfceMixerTrackCombo *combo;

  widget = g_object_new (TYPE_XFCE_MIXER_TRACK_COMBO, NULL);
  combo = XFCE_MIXER_TRACK_COMBO (widget);

  xfce_mixer_track_combo_set_soundcard (combo, card);
  xfce_mixer_track_combo_set_active_track (combo, track);

  return widget;
}



void
xfce_mixer_track_combo_set_soundcard (XfceMixerTrackCombo *combo,
                                      GstElement          *card)
{
  XfceMixerTrackType type;
  GtkTreeIter        tree_iter;
  const GList       *iter;
  gchar             *label;
  gint               counter;
  gint               active_index = 0;
  GstMixerTrack     *track;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  /* Remember card. If the card is invalid, use the first one available */
  if (GST_IS_MIXER (card))
    combo->card = card;
  else
    {
      card = xfce_mixer_get_default_card ();

      if (GST_IS_MIXER (card))
        combo->card = card;
    }

  /* Try to re-use the current track */
  track = xfce_mixer_track_combo_get_active_track (combo);

  /* Clear the list store data */
  gtk_list_store_clear (combo->list_store);

  for (iter = gst_mixer_list_tracks (GST_MIXER (combo->card)), counter = 0; iter != NULL; iter = g_list_next (iter))
    {
      type = xfce_mixer_track_type_new (iter->data);

      if (type == XFCE_MIXER_TRACK_TYPE_PLAYBACK || type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
        {
          g_object_get (GST_MIXER_TRACK (iter->data), "label", &label, NULL);

          gtk_list_store_append (combo->list_store, &tree_iter);
          gtk_list_store_set (combo->list_store, &tree_iter, 
                              NAME_COLUMN, label, 
                              TRACK_COLUMN, GST_MIXER_TRACK (iter->data), -1);

          g_free (label);

          if (G_UNLIKELY (track != NULL && track == GST_MIXER_TRACK (iter->data)))
            active_index = counter;

          ++counter;
        }
    }

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active_index);
}



static void
xfce_mixer_track_combo_changed (XfceMixerTrackCombo *combo)
{
  GstMixerTrack *track;
  GtkTreeIter    iter;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, TRACK_COLUMN, &track, -1);
      g_signal_emit_by_name (combo, "track-changed", track);
    }
}



GstMixerTrack*
xfce_mixer_track_combo_get_active_track (XfceMixerTrackCombo *combo)
{
  GstMixerTrack *track = NULL;
  GtkTreeIter    iter;

  g_return_val_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo), NULL);

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, TRACK_COLUMN, &track, -1);

  return track;
}

void
xfce_mixer_track_combo_set_active_track (XfceMixerTrackCombo *combo,
                                         GstMixerTrack       *track)
{
  GstMixerTrack *current_track = NULL;
  GtkTreeIter    iter;
  gboolean       valid_iter;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  if (G_UNLIKELY (!GST_IS_MIXER_TRACK (track)))
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
      return;
    }

  valid_iter = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (combo->list_store), &iter);

  while (valid_iter)
    {
      gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, TRACK_COLUMN, &current_track, -1);

      if (G_UNLIKELY (current_track == track))
        break;

      valid_iter = gtk_tree_model_iter_next (GTK_TREE_MODEL (combo->list_store), &iter);
    }

  if (G_LIKELY (current_track == track))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
}

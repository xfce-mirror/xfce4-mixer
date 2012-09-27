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



enum
{
  NAME_COLUMN,
  TRACK_COLUMN
};

enum 
{
  TRACK_CHANGED,
  LAST_SIGNAL,
};



static guint combo_signals[LAST_SIGNAL];



static void  xfce_mixer_track_combo_finalize          (GObject                  *object);
static void  xfce_mixer_track_combo_update_track_list (XfceMixerTrackCombo      *combo);
static void  xfce_mixer_track_combo_bus_message       (GstBus                   *bus,
                                                       GstMessage               *message,
                                                       XfceMixerTrackCombo      *combo);
static void  xfce_mixer_track_combo_changed           (XfceMixerTrackCombo      *combo);



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
  gulong         signal_handler_id;
};



G_DEFINE_TYPE (XfceMixerTrackCombo, xfce_mixer_track_combo, GTK_TYPE_COMBO_BOX)



static void
xfce_mixer_track_combo_class_init (XfceMixerTrackComboClass *klass)
{
  GObjectClass *gobject_class;

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

  combo->signal_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_track_combo_bus_message), combo);

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

  if (combo->signal_handler_id > 0)
    {
      xfce_mixer_bus_disconnect (combo->signal_handler_id);
      combo->signal_handler_id = 0;
    }

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



static void
xfce_mixer_track_combo_update_track_list (XfceMixerTrackCombo *combo)
{
  XfceMixerTrackType type;
  GtkTreeIter        tree_iter;
  const GList       *iter;
  gint               counter;
  gint               active_index = 0;
  GstMixerTrack     *track;
  GstMixerTrack     *track_new;

  g_return_if_fail (GST_IS_MIXER (combo->card));

  /* Try to re-use the current track */
  track = xfce_mixer_track_combo_get_active_track (combo);

  /* Clear the list store data */
  gtk_list_store_clear (combo->list_store);

  for (iter = gst_mixer_list_tracks (GST_MIXER (combo->card)), counter = 0; iter != NULL; iter = g_list_next (iter))
    {
      track_new = GST_MIXER_TRACK (iter->data);
      type = xfce_mixer_track_type_new (track_new);

      /* Only include writable playback or capture tracks */
      if ((type == XFCE_MIXER_TRACK_TYPE_PLAYBACK &&
           !GST_MIXER_TRACK_HAS_FLAG (track_new, GST_MIXER_TRACK_READONLY)) ||
          (type == XFCE_MIXER_TRACK_TYPE_CAPTURE &&
           !GST_MIXER_TRACK_HAS_FLAG (track_new, GST_MIXER_TRACK_READONLY)))
        {
          gtk_list_store_append (combo->list_store, &tree_iter);
          gtk_list_store_set (combo->list_store, &tree_iter, 
                              NAME_COLUMN, xfce_mixer_get_track_label (track_new),
                              TRACK_COLUMN, GST_MIXER_TRACK (iter->data), -1);

          if (G_UNLIKELY (GST_IS_MIXER_TRACK (track) && track == track_new))
            active_index = counter;

          ++counter;
        }
    }

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active_index);
}



void
xfce_mixer_track_combo_set_soundcard (XfceMixerTrackCombo *combo,
                                      GstElement          *card)
{
  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  /* Remember card. If the card is invalid, use the first one available */
  if (GST_IS_MIXER (card))
    combo->card = card;
  else
    {
      card = xfce_mixer_get_default_card ();

      if (GST_IS_MIXER (card))
        combo->card = card;
      else
        return;
    }

  xfce_mixer_track_combo_update_track_list (combo);
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



static void
xfce_mixer_track_combo_bus_message (GstBus              *bus,
                                    GstMessage          *message,
                                    XfceMixerTrackCombo *combo)
{
  if (!GST_IS_MIXER (combo->card) || GST_MESSAGE_SRC (message) != GST_OBJECT (combo->card))
    return;

  /* Rebuild track list if the tracks for this card have changed */
  if (gst_mixer_message_get_type (message) == GST_MIXER_MESSAGE_MIXER_CHANGED)
    xfce_mixer_track_combo_update_track_list (combo);
}

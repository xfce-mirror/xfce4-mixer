/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
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
static void _xfce_mixer_track_combo_set_track  (XfceMixerTrackCombo      *combo,
                                                GstMixerTrack            *track);



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
  combo->track = NULL;

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
  GtkWidget *combo;

  combo = g_object_new (TYPE_XFCE_MIXER_TRACK_COMBO, NULL);

  _xfce_mixer_track_combo_set_track (XFCE_MIXER_TRACK_COMBO (combo), track);
  xfce_mixer_track_combo_set_soundcard (XFCE_MIXER_TRACK_COMBO (combo), card);

  return combo;
}



void
xfce_mixer_track_combo_set_soundcard (XfceMixerTrackCombo *combo,
                                      GstElement          *card)
{
  XfceMixerTrackType type;
  GtkTreeIter        tree_iter;
  const GList       *iter;
  GList             *cards;
  gint               counter;
  gint               active_index = 0;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  /* Remember card. If the card is invalid, use the first one available */
  if (GST_IS_MIXER (card))
    combo->card = card;
  else
    {
      cards = xfce_mixer_get_cards ();

      if (G_LIKELY (g_list_length (cards) > 0))
        combo->card = g_list_first (cards)->data;
    }

  /* Clear the list store data */
  gtk_list_store_clear (combo->list_store);

  for (iter = gst_mixer_list_tracks (GST_MIXER (combo->card)), counter = 0; iter != NULL; iter = g_list_next (iter))
    {
      type = xfce_mixer_track_type_new (iter->data);

      if (type == XFCE_MIXER_TRACK_TYPE_PLAYBACK || type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
        {
          gtk_list_store_append (combo->list_store, &tree_iter);
          gtk_list_store_set (combo->list_store, &tree_iter, 
                              NAME_COLUMN, GST_MIXER_TRACK (iter->data)->label, 
                              TRACK_COLUMN, GST_MIXER_TRACK (iter->data), -1);

          if (G_UNLIKELY (combo->track != NULL && combo->track == GST_MIXER_TRACK (iter->data)))
            active_index = counter;

          ++counter;
        }
    }

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active_index);
}



void
_xfce_mixer_track_combo_set_track (XfceMixerTrackCombo *combo,
                                   GstMixerTrack       *track)
{
  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));
  combo->track = track;
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

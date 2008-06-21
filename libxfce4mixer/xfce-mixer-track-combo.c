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

#include <gtk/gtk.h>

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



static void xfce_mixer_track_combo_class_init    (XfceMixerTrackComboClass *klass);
static void xfce_mixer_track_combo_init          (XfceMixerTrackCombo      *combo);
static void xfce_mixer_track_combo_dispose       (GObject                  *object);
static void xfce_mixer_track_combo_finalize      (GObject                  *object);
static void xfce_mixer_track_combo_changed       (XfceMixerTrackCombo      *combo);
static void xfce_mixer_track_combo_track_changed (XfceMixerTrackCombo      *combo,
                                                  GstMixerTrack            *track);



struct _XfceMixerTrackComboClass
{
  GtkComboBoxClass __parent__;

  /* Signals */
  void (*track_changed) (XfceMixerTrackCombo *combo,
                         GstMixerTrack       *track);
};

struct _XfceMixerTrackCombo
{
  GtkComboBox __parent__;

  GtkListStore  *model;

  XfceMixerCard *card;

  gchar         *track_name;
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
  gobject_class->dispose = xfce_mixer_track_combo_dispose;
  gobject_class->finalize = xfce_mixer_track_combo_finalize;

  klass->track_changed = xfce_mixer_track_combo_track_changed;

  combo_signals[TRACK_CHANGED] = g_signal_new ("track-changed",
                                               G_TYPE_FROM_CLASS (klass),
                                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                               G_STRUCT_OFFSET (XfceMixerTrackComboClass, track_changed),
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
  combo->track_name = NULL;

  combo->model = gtk_list_store_new (2, G_TYPE_STRING, GST_TYPE_MIXER_TRACK);
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (combo->model));

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", NAME_COLUMN);

  g_signal_connect_swapped (combo, "changed", G_CALLBACK (xfce_mixer_track_combo_changed), combo);
}



static void
xfce_mixer_track_combo_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_track_combo_parent_class)->dispose) (object);
}



static void
xfce_mixer_track_combo_finalize (GObject *object)
{
  XfceMixerTrackCombo *combo = XFCE_MIXER_TRACK_COMBO (object);

  gtk_list_store_clear (combo->model);
  g_object_unref (G_OBJECT (combo->model));

  (*G_OBJECT_CLASS (xfce_mixer_track_combo_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_track_combo_new (XfceMixerCard *card,
                            const gchar   *track_name)
{
  XfceMixerTrackCombo *combo;

  combo = XFCE_MIXER_TRACK_COMBO (g_object_new (TYPE_XFCE_MIXER_TRACK_COMBO, NULL));

  xfce_mixer_track_combo_set_track (combo, track_name);
  xfce_mixer_track_combo_set_soundcard (combo, card);

  return GTK_WIDGET (combo);
}



void
xfce_mixer_track_combo_set_soundcard (XfceMixerTrackCombo *combo,
                                      XfceMixerCard       *card)
{
  XfceMixerTrackType type;
  GList             *cards;
  const GList       *titer;
  GtkTreeIter        iter;
  gint               counter;
  gint               active_index = 0;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  g_message ("set_soundcard: combo->track_name = %s", combo->track_name);

  if (G_LIKELY (combo->card != NULL))
    g_object_unref (G_OBJECT (combo->card));

  if (IS_XFCE_MIXER_CARD (card))
    combo->card = XFCE_MIXER_CARD (g_object_ref (G_OBJECT (card)));
  else
    {
      cards = xfce_mixer_utilities_get_cards ();
      
      combo->card = XFCE_MIXER_CARD (g_object_ref (G_OBJECT ( (g_list_first (cards)->data))));
      xfce_mixer_card_set_ready (combo->card);

      g_list_foreach (cards, (GFunc) g_object_unref, NULL);
      g_list_free (cards);
    }

  gtk_list_store_clear (combo->model);

  for (titer = xfce_mixer_card_get_tracks (combo->card), counter = 0; titer != NULL; titer = g_list_next (titer))
    {
      type = xfce_mixer_track_type_new (titer->data);

      if (type == XFCE_MIXER_TRACK_TYPE_PLAYBACK || type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
        {
          gtk_list_store_append (combo->model, &iter);
          gtk_list_store_set (combo->model, &iter, 
                              NAME_COLUMN, GST_MIXER_TRACK (titer->data)->label, 
                              TRACK_COLUMN, GST_MIXER_TRACK (titer->data), -1);

          g_message ("compare %s to %s", combo->track_name, GST_MIXER_TRACK (titer->data)->label);

          if (G_UNLIKELY (combo->track_name != NULL && g_utf8_collate (GST_MIXER_TRACK (titer->data)->label, combo->track_name) == 0))
            {
              g_message ("equal => active_index = %i", counter);
              active_index = counter;
            }

          ++counter;
        }
    }

  g_message ("active_index = %i", active_index);

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active_index);
}



void
xfce_mixer_track_combo_set_track (XfceMixerTrackCombo *combo,
                                  const gchar         *track_name)
{
  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  g_free (combo->track_name);

  if (G_UNLIKELY (track_name != NULL))
    {
      combo->track_name = g_strdup (track_name);
      
      /* TODO: Reset the active iter */
    }
}



static void
xfce_mixer_track_combo_changed (XfceMixerTrackCombo *combo)
{
  GstMixerTrack *track;
  GtkTreeIter    iter;

  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo));

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (combo->model), &iter, TRACK_COLUMN, &track, -1);

      g_signal_emit_by_name (combo, "track-changed", track);
    }
}



static void 
xfce_mixer_track_combo_track_changed (XfceMixerTrackCombo *combo,
                                      GstMixerTrack       *track)
{
}



GstMixerTrack*
xfce_mixer_track_combo_get_active_track (XfceMixerTrackCombo *combo)
{
  GstMixerTrack *track = NULL;
  GtkTreeIter    iter;

  g_return_val_if_fail (IS_XFCE_MIXER_TRACK_COMBO (combo), NULL);

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    gtk_tree_model_get (GTK_TREE_MODEL (combo->model), &iter, TRACK_COLUMN, &track, -1);

  return track;
}

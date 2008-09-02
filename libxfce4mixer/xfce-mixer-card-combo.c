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

#include <gst/gst.h>

#include "xfce-mixer-card.h"
#include "xfce-mixer-utilities.h"
#include "xfce-mixer-card-combo.h"



#define NAME_COLUMN 0
#define CARD_COLUMN 1



enum
{
  SOUNDCARD_CHANGED,
  LAST_SIGNAL,
};



static guint combo_signals[LAST_SIGNAL];



static void xfce_mixer_card_combo_class_init        (XfceMixerCardComboClass *klass);
static void xfce_mixer_card_combo_init              (XfceMixerCardCombo      *combo);
static void xfce_mixer_card_combo_dispose           (GObject                 *object);
static void xfce_mixer_card_combo_finalize          (GObject                 *object);
static void xfce_mixer_card_combo_load_soundcards   (XfceMixerCardCombo      *combo,
                                                     const gchar             *card_name);
static void xfce_mixer_card_combo_soundcard_changed (XfceMixerCardCombo      *combo,
                                                     XfceMixerCard           *card);
static void xfce_mixer_card_combo_changed           (XfceMixerCardCombo      *combo);



struct _XfceMixerCardComboClass
{
  GtkComboBoxClass __parent__;

  /* Signals */
  void (*soundcard_changed) (XfceMixerCardCombo *combo,
                             XfceMixerCard      *card);
};

struct _XfceMixerCardCombo
{
  GtkComboBox __parent__;

  GtkListStore *model;

  GList        *cards;
};



static GObjectClass *xfce_mixer_card_combo_parent_class = NULL;



GType
xfce_mixer_card_combo_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerCardComboClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_card_combo_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerCardCombo),
          0,
          (GInstanceInitFunc) xfce_mixer_card_combo_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_COMBO_BOX, "XfceMixerCardCombo", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_card_combo_class_init (XfceMixerCardComboClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_card_combo_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_card_combo_dispose;
  gobject_class->finalize = xfce_mixer_card_combo_finalize;

  klass->soundcard_changed = xfce_mixer_card_combo_soundcard_changed;

  combo_signals[SOUNDCARD_CHANGED] = g_signal_new ("soundcard-changed",
                                                   G_TYPE_FROM_CLASS (klass),
                                                   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                   G_STRUCT_OFFSET (XfceMixerCardComboClass, soundcard_changed),
                                                   NULL,
                                                   NULL,
                                                   g_cclosure_marshal_VOID__OBJECT,
                                                   G_TYPE_NONE,
                                                   1,
                                                   TYPE_XFCE_MIXER_CARD);
}



static void
xfce_mixer_card_combo_init (XfceMixerCardCombo *combo)
{
  GtkCellRenderer *renderer;

  combo->model = gtk_list_store_new (2, G_TYPE_STRING, TYPE_XFCE_MIXER_CARD);
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (combo->model));

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", NAME_COLUMN);

  combo->cards = xfce_mixer_utilities_get_cards ();

  g_signal_connect_swapped (combo, "changed", G_CALLBACK (xfce_mixer_card_combo_changed), combo);
}



static void
xfce_mixer_card_combo_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_card_combo_parent_class)->dispose) (object);
}



static void
xfce_mixer_card_combo_finalize (GObject *object)
{
  XfceMixerCardCombo *combo = XFCE_MIXER_CARD_COMBO (object);

  gtk_list_store_clear (combo->model);
  g_object_unref (G_OBJECT (combo->model));

  g_list_foreach (combo->cards, (GFunc) g_object_unref, NULL);
  g_list_free (combo->cards);

  (*G_OBJECT_CLASS (xfce_mixer_card_combo_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_card_combo_new (const gchar *card_name)
{
  XfceMixerCardCombo *combo;
  
  combo = XFCE_MIXER_CARD_COMBO (g_object_new (TYPE_XFCE_MIXER_CARD_COMBO, NULL));

  xfce_mixer_card_combo_load_soundcards (combo, card_name);

  return GTK_WIDGET (combo);
}



static void 
xfce_mixer_card_combo_load_soundcards (XfceMixerCardCombo *combo,
                                       const gchar        *card_name)
{
  XfceMixerCard *card;
  GtkTreeIter    iter;
  GList         *citer;
  gint           counter;
  gint           active_index = 0;

  for (citer = g_list_first (combo->cards), counter = 0; citer != NULL; citer = g_list_next (citer), ++counter)
    {
      card = XFCE_MIXER_CARD (citer->data);

      g_debug ("Adding sound card: %s", xfce_mixer_card_get_display_name (card));

      gtk_list_store_append (combo->model, &iter);
      gtk_list_store_set (combo->model, &iter, NAME_COLUMN, xfce_mixer_card_get_display_name (card), CARD_COLUMN, card, -1);

      if (G_UNLIKELY (card_name != NULL && g_utf8_collate (xfce_mixer_card_get_name (card), card_name) == 0))
        active_index = counter;
    }

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active_index);
}



static void
xfce_mixer_card_combo_soundcard_changed (XfceMixerCardCombo *combo,
                                         XfceMixerCard      *card)
{
}



static void
xfce_mixer_card_combo_changed (XfceMixerCardCombo *combo)
{
  XfceMixerCard *card;
  GtkTreeIter    iter;

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (combo->model), &iter, CARD_COLUMN, &card, -1);

      xfce_mixer_card_set_ready (card);

      g_signal_emit_by_name (combo, "soundcard-changed", card);
    }
}



XfceMixerCard*
xfce_mixer_card_combo_get_active_card (XfceMixerCardCombo *combo)
{
  XfceMixerCard *card = NULL;
  GtkTreeIter    iter;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD_COMBO (combo), NULL);
  
  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    gtk_tree_model_get (GTK_TREE_MODEL (combo->model), &iter, CARD_COLUMN, &card, -1);

  return card;
}



gint
xfce_mixer_card_combo_get_n_cards (XfceMixerCardCombo *combo)
{
  g_return_val_if_fail (IS_XFCE_MIXER_CARD_COMBO (combo), 0);
  return gtk_tree_model_iter_n_children (GTK_TREE_MODEL (combo->model), NULL);
}

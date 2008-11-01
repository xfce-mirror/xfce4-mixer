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

#include <gtk/gtk.h>

#include <gst/gst.h>

#include "libxfce4mixer.h"
#include "xfce-mixer-card-combo.h"



#define NAME_COLUMN 0
#define CARD_COLUMN 1



enum
{
  SOUNDCARD_CHANGED,
  LAST_SIGNAL,
};



static guint combo_signals[LAST_SIGNAL];



static void  xfce_mixer_card_combo_class_init        (XfceMixerCardComboClass *klass);
static void  xfce_mixer_card_combo_init              (XfceMixerCardCombo      *combo);
static void  xfce_mixer_card_combo_finalize          (GObject                 *object);
static void  xfce_mixer_card_combo_load_soundcards   (XfceMixerCardCombo      *combo,
                                                      const gchar             *card_name);
static void  xfce_mixer_card_combo_changed           (XfceMixerCardCombo      *combo);
static void _xfce_mixer_card_combo_set_active_card   (XfceMixerCardCombo      *combo,
                                                      GstElement              *card);



struct _XfceMixerCardComboClass
{
  GtkComboBoxClass __parent__;
};

struct _XfceMixerCardCombo
{
  GtkComboBox __parent__;

  GtkListStore *list_store;
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
  gobject_class->finalize = xfce_mixer_card_combo_finalize;

  combo_signals[SOUNDCARD_CHANGED] = g_signal_new ("soundcard-changed",
                                                   G_TYPE_FROM_CLASS (klass),
                                                   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                   0,
                                                   NULL,
                                                   NULL,
                                                   g_cclosure_marshal_VOID__OBJECT,
                                                   G_TYPE_NONE,
                                                   1,
                                                   GST_TYPE_ELEMENT);
}



static void
xfce_mixer_card_combo_init (XfceMixerCardCombo *combo)
{
  GtkCellRenderer *renderer;
  GtkTreeIter      tree_iter;
  GList           *iter;

  combo->list_store = gtk_list_store_new (2, G_TYPE_STRING, GST_TYPE_ELEMENT);
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL (combo->list_store));

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo), renderer, "text", NAME_COLUMN);

  for (iter = xfce_mixer_get_cards (); iter != NULL; iter = g_list_next (iter))
    {
      gtk_list_store_append (combo->list_store, &tree_iter);
      gtk_list_store_set (combo->list_store, &tree_iter, 
                          NAME_COLUMN, xfce_mixer_get_card_display_name (iter->data),
                          CARD_COLUMN, iter->data, -1);
    }

  g_signal_connect_swapped (combo, "changed", G_CALLBACK (xfce_mixer_card_combo_changed), combo);
}



static void
xfce_mixer_card_combo_finalize (GObject *object)
{
  XfceMixerCardCombo *combo = XFCE_MIXER_CARD_COMBO (object);

  gtk_list_store_clear (combo->list_store);
  g_object_unref (combo->list_store);

  (*G_OBJECT_CLASS (xfce_mixer_card_combo_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_card_combo_new (GstElement *card)
{
  GtkWidget *combo;
  
  combo = g_object_new (TYPE_XFCE_MIXER_CARD_COMBO, NULL);

  _xfce_mixer_card_combo_set_active_card (XFCE_MIXER_CARD_COMBO (combo), card);

  return combo;
}



static void
xfce_mixer_card_combo_changed (XfceMixerCardCombo *combo)
{
  GstElement *card;
  GtkTreeIter iter;

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, CARD_COLUMN, &card, -1);
      g_signal_emit_by_name (combo, "soundcard-changed", card);
    }
}



GstElement *
xfce_mixer_card_combo_get_active_card (XfceMixerCardCombo *combo)
{
  GstElement *card = NULL;
  GtkTreeIter iter;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD_COMBO (combo), NULL);
  
  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)))
    gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, CARD_COLUMN, &card, -1);

  return card;
}



static void
_xfce_mixer_card_combo_set_active_card (XfceMixerCardCombo *combo,
                                        GstElement         *card)
{
  GstElement *current_card = NULL;
  GtkTreeIter iter;
  gboolean    valid_iter;

  g_return_if_fail (IS_XFCE_MIXER_CARD_COMBO (combo));

  if (G_LIKELY (GST_IS_MIXER (card)))
    {
      valid_iter = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (combo->list_store), &iter);

      while (valid_iter)
        {
          gtk_tree_model_get (GTK_TREE_MODEL (combo->list_store), &iter, CARD_COLUMN, &current_card, -1);

          if (G_UNLIKELY (current_card == card))
            break;

          valid_iter = gtk_tree_model_iter_next (GTK_TREE_MODEL (combo->list_store), &iter);
        }
    }

  if (G_LIKELY (card != NULL && current_card == card))
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
}

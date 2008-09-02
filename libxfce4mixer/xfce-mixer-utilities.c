/* $Id$ */
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

#include <glib.h>
#include <glib/gi18n.h>

#include <gst/gst.h>
#include <gst/audio/mixerutils.h>
#include <gst/interfaces/mixer.h>

#include "xfce-mixer-card.h"



static gboolean xfce_mixer_utilities_filter_mixer (GstMixer *mixer,
                                                   gpointer  user_data);



GList*
xfce_mixer_utilities_get_cards (void)
{
  GList *cards = NULL;
  GList *mixers = NULL;
  GList *iter;
  guint  counter = 0;

  /* Get list of all available sound cards */
  mixers = gst_audio_default_registry_mixer_filter (xfce_mixer_utilities_filter_mixer, FALSE, &counter);

  for (iter = g_list_first (mixers); iter != NULL; iter = g_list_next (iter))
    cards = g_list_append (cards, xfce_mixer_card_new (iter->data));

  g_list_free (mixers);

  return cards;
}



static gboolean
xfce_mixer_utilities_filter_mixer (GstMixer *mixer,
                                   gpointer  user_data)
{
  GstElementFactory *factory;
  const gchar       *long_name;
  gchar             *device_name = NULL;
  gchar             *name;
  gchar             *title;
  guint             *counter = (guint *) user_data;

  /* Get long name of the mixer element */
  factory = gst_element_get_factory (GST_ELEMENT (mixer));
  long_name = gst_element_factory_get_longname (factory);

  /* Get device name of the mixer element */
  g_object_get (G_OBJECT (mixer), "device-name", &device_name, NULL);

  /* Build full name for this element */
  if (G_LIKELY (device_name != NULL))
    {
      name = g_strdup_printf ("%s (%s)", device_name, long_name);
      g_free (device_name);
    }
  else
    {
      title = g_strdup_printf (_("Unknown Volume Control %d"), (*counter)++);
      name = g_strdup_printf ("%s (%s)", title, long_name);
      g_free (title);
    }

  /* Set name to be used in xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-control-name", name, (GDestroyNotify) g_free);

  gst_element_set_state (GST_ELEMENT (mixer), GST_STATE_NULL);

  return TRUE;
}



XfceMixerCard*
xfce_mixer_utilities_get_card_by_name (const gchar *card_name)
{
  XfceMixerCard *card = NULL;
  GList         *cards;
  GList         *iter;

  g_return_val_if_fail (card_name != NULL, NULL);

  cards = xfce_mixer_utilities_get_cards ();

  for (iter = g_list_first (cards); iter != NULL; iter = g_list_next (iter))
    if (G_UNLIKELY (g_utf8_collate (xfce_mixer_card_get_name (XFCE_MIXER_CARD (iter->data)), card_name) == 0))
      {
        card = XFCE_MIXER_CARD (g_object_ref (G_OBJECT (iter->data)));
        xfce_mixer_card_set_ready (card);
        break;
      }

  g_list_foreach (cards, (GFunc) g_object_unref, NULL);
  g_list_free (cards);

  return card;
}



gint
xfce_mixer_utilities_get_max_volume (gint *volumes,
                                     gint  num_channels)
{
  gint max = 0;

  g_return_val_if_fail (volumes != NULL, 0);

  for (--num_channels; num_channels >= 0; --num_channels)
    if (volumes[num_channels] > max)
      max = volumes[num_channels];

  return max;
}



gint
xfce_mixer_utilities_get_n_cards (void)
{
  GList *cards = NULL;
  gint   number = 0;

  cards = xfce_mixer_utilities_get_cards ();

  number = g_list_length (cards);

  g_list_foreach (cards, (GFunc) g_object_unref, NULL);
  g_list_free (cards);

  return number;
}

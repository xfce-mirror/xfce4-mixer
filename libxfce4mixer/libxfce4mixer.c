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

#include <gst/audio/mixerutils.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>

#include "libxfce4mixer.h"



static gboolean _xfce_mixer_filter_mixer  (GstMixer *mixer,
                                           gpointer  user_data);
static void     _xfce_mixer_destroy_mixer (GstMixer *mixer);



static guint       refcount = 0;
static GList      *mixers = NULL;
#ifdef HAVE_GST_MIXER_NOTIFICATION
static GstBus     *bus = NULL;
static GstElement *selected_card = NULL;
#endif



void
xfce_mixer_init (void)
{
  GtkIconTheme *icon_theme;
  gint          counter = 0;

  if (G_LIKELY (refcount++ == 0))
    {
      /* Append application icons to the search path */
      icon_theme = gtk_icon_theme_get_default ();
      gtk_icon_theme_append_search_path (icon_theme, MIXER_DATADIR G_DIR_SEPARATOR_S "icons");

      /* Get list of all available mixer devices */
      mixers = gst_audio_default_registry_mixer_filter (_xfce_mixer_filter_mixer, FALSE, &counter);

#ifdef HAVE_GST_MIXER_NOTIFICATION
      /* Create a GstBus for notifications */
      bus = gst_bus_new ();
      gst_bus_add_signal_watch (bus);
#endif
    }
}



void
xfce_mixer_shutdown (void)
{
  if (G_LIKELY (--refcount <= 0))
    {
      g_list_foreach (mixers, (GFunc) _xfce_mixer_destroy_mixer, NULL);
      g_list_free (mixers);

#ifdef HAVE_GST_MIXER_NOTIFICATION
      gst_bus_remove_signal_watch (bus);
      gst_object_unref (bus);
#endif
    }
}



GList *
xfce_mixer_get_cards (void)
{
  g_return_val_if_fail (refcount > 0, NULL);
  return mixers;
}



GstElement *
xfce_mixer_get_card (const gchar *name)
{
  GstElement *element = NULL;
  GList      *iter;
  gchar      *card_name;

  g_return_val_if_fail (refcount > 0, NULL);

  if (G_UNLIKELY (name == NULL))
    return NULL;

  for (iter = g_list_first (mixers); iter != NULL; iter = g_list_next (iter))
    {
      card_name = g_object_get_data (G_OBJECT (iter->data), "xfce-mixer-internal-name");

      if (G_UNLIKELY (g_utf8_collate (name, card_name) == 0))
        {
          element = iter->data;
          break;
        }
    }

  return element;
}



GstElement *
xfce_mixer_get_default_card (void)
{
  GList      *cards;
  GstElement *card = NULL;

  cards = xfce_mixer_get_cards ();

  /* Try to get the first card */
  if (g_list_length (cards) > 0)
    card = g_list_first (cards)->data;

  return card;
}



const gchar *
xfce_mixer_get_card_display_name (GstElement *card)
{
  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  return g_object_get_data (G_OBJECT (card), "xfce-mixer-name");
}



const gchar *
xfce_mixer_get_card_internal_name (GstElement *card)
{
  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  return g_object_get_data (G_OBJECT (card), "xfce-mixer-internal-name");
}



void
xfce_mixer_select_card (GstElement *card)
{
  g_return_if_fail (GST_IS_MIXER (card));

#ifdef HAVE_GST_MIXER_NOTIFICATION
  gst_element_set_bus (card, bus);
  selected_card = card;
#endif
}



GstMixerTrack *
xfce_mixer_get_track (GstElement  *card,
                      const gchar *track_name)
{
  GstMixerTrack *track = NULL;
  const GList   *iter;
  gchar         *label;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (track_name != NULL, NULL);

  for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
    {
      g_object_get (GST_MIXER_TRACK (iter->data), "label", &label, NULL);

      if (g_utf8_collate (label, track_name) == 0)
        {
          track = iter->data;
          g_free (label);
          break;
        }
      
      g_free (label);
    }

  return track;
}



GstMixerTrack *
xfce_mixer_get_default_track (GstElement *card)
{
  GstMixerTrack *track = NULL;
  const GList   *iter;
  GstMixerTrack *track_tmp;
  const GList   *tracks;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);

  /* Try to get the master track */
  for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
    {
      track_tmp = GST_MIXER_TRACK (iter->data);

      if (GST_MIXER_TRACK_HAS_FLAG (track_tmp, GST_MIXER_TRACK_MASTER))
        {
          track = track_tmp;
          break;
        }
    }

  /* If there is no master track, try to get the first track */
  if (!GST_IS_MIXER_TRACK (track))
    {
      tracks = gst_mixer_list_tracks (GST_MIXER (card));
      if (g_list_length (tracks) > 0)
        track = g_list_first (tracks)->data;
    }

  return track;
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
guint
xfce_mixer_bus_connect (GCallback callback,
                        gpointer  user_data)
{
  g_return_val_if_fail (refcount > 0, 0);
  return g_signal_connect (bus, "message::element", callback, user_data);
}



void
xfce_mixer_bus_disconnect (guint signal_handler_id)
{
  g_return_if_fail (refcount > 0);
  if (signal_handler_id != 0)
    g_signal_handler_disconnect (bus, signal_handler_id);
}
#endif



gint
xfce_mixer_get_max_volume (gint *volumes,
                           gint  num_channels)
{
  gint max = 0;

  g_return_val_if_fail (volumes != NULL, 0);

  if (num_channels > 0)
    max = volumes[0];

  for (--num_channels; num_channels >= 0; --num_channels)
    if (volumes[num_channels] > max)
      max = volumes[num_channels];

  return max;
}



static gboolean 
_xfce_mixer_filter_mixer (GstMixer *mixer,
                          gpointer  user_data)
{
  GstElementFactory *factory;
  const gchar       *long_name;
  gchar             *device_name = NULL;
  gchar             *internal_name;
  gchar             *name;
  gchar             *p;
  gint               length;
  gint              *counter = user_data;

  /* Get long name of the mixer element */
  factory = gst_element_get_factory (GST_ELEMENT (mixer));
  long_name = gst_element_factory_get_longname (factory);

  /* Get the device name of the mixer element */
  if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (mixer)), "device-name"))
    g_object_get (mixer, "device-name", &device_name, NULL);
  
  /* Fall back to default name if neccessary */
  if (G_UNLIKELY (device_name == NULL))
    device_name = g_strdup_printf (_("Unknown Volume Control %d"), (*counter)++);

  /* Build display name */
  name = g_strdup_printf ("%s (%s)", device_name, long_name);

  /* Free device name */
  g_free (device_name);

  /* Set name to be used by xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-name", name, (GDestroyNotify) g_free);

  /* Count alpha-numeric characters in the name */
  for (length = 0, p = name; *p != '\0'; ++p)
    if (g_ascii_isalnum (*p))
      ++length;

  /* Generate internal name */
  internal_name = g_new0 (gchar, length+1);
  for (length = 0, p = name; *p != '\0'; ++p)
    if (g_ascii_isalnum (*p))
      internal_name[length++] = *p;
  internal_name[length] = '\0';

  /* Remember name for use by xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-internal-name", internal_name, (GDestroyNotify) g_free);

  /* Keep the mixer (we want all devices to be visible) */
  return TRUE;
}



static void
_xfce_mixer_destroy_mixer (GstMixer *mixer)
{
  gst_element_set_state (GST_ELEMENT (mixer), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (mixer));
}



int
xfce_mixer_utf8_cmp (const gchar *s1, const gchar *s2)
{
  if (s1 == NULL && s2 != NULL)
    return 1;
  else if (s1 != NULL && s2 == NULL)
    return -1;
  else if (s1 == NULL && s2 == NULL)
    return 0;

  return g_utf8_collate (s1, s2);
}


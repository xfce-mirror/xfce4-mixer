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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>


#include <libxfce4util/libxfce4util.h>

#include "libxfce4mixer.h"

#ifdef HAVE_ALSA
#include "alsa/alsa-mixer.h"
#endif

#ifdef HAVE_OSS
#include "oss/oss-mixer.h"
#endif

#ifdef HAVE_PULSE
#include "pulse/pulse-mixer.h"
#endif

#ifdef HAVE_SNDIO
#include "sndio/sndio-mixer.h"
#endif

static void     _xfce_mixer_init_mixer       (gpointer data,
                                              gpointer user_data);
static void     _xfce_mixer_destroy_mixer    (gpointer data);



static guint        refcount = 0;
static GList       *mixers = NULL;
static GstBus      *bus = NULL;
static GstElement  *selected_card = NULL;
static const gchar *tracks_whitelist[] =
{
  "cd",
  "digital output",
  "front",
  "headphone",
  "line",
  "master",
  "mic",
  "pcm",
  "recording",
  "speaker",
  "volume",
  NULL
};



void
xfce_mixer_init (void)
{
  GtkIconTheme *icon_theme;

  if (G_LIKELY (refcount++ == 0))
    {
      /* Append application icons to the search path */
      icon_theme = gtk_icon_theme_get_default ();
      gtk_icon_theme_append_search_path (icon_theme, MIXER_DATADIR G_DIR_SEPARATOR_S "icons");

#ifdef HAVE_ALSA
      mixers = gst_mixer_alsa_probe (mixers);
#endif

#ifdef HAVE_OSS
      mixers = gst_mixer_oss_probe (mixers);
#endif

#ifdef HAVE_PULSE
      mixers = gst_mixer_pulse_probe (mixers);
#endif

#ifdef HAVE_SNDIO
      mixers = gst_mixer_sndio_probe (mixers);
#endif

      /* Create a GstBus for notifications */
      bus = gst_bus_new ();
      gst_bus_add_signal_watch (bus);

      /*
       * Add custom labels to all tracks of all mixers and create a GstBus for
       * each mixer device and connect an internal signal handler to receive
       * notifications about track list changes
       */
      g_list_foreach (mixers, (GFunc) _xfce_mixer_init_mixer, NULL);
    }
}



void
xfce_mixer_shutdown (void)
{
  if (G_LIKELY (--refcount <= 0))
    {
      g_list_free_full (mixers, _xfce_mixer_destroy_mixer);

      gst_bus_remove_signal_watch (bus);
      gst_object_unref (bus);
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

  selected_card = card;
}



GstMixerTrack *
xfce_mixer_get_track (GstElement  *card,
                      const gchar *track_name)
{
  GstMixerTrack *track = NULL;
  const GList   *iter;
  const gchar   *label;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (track_name != NULL, NULL);

  for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
    {
      label = xfce_mixer_get_track_label (GST_MIXER_TRACK (iter->data));

      if (g_utf8_collate (label, track_name) == 0)
        {
          track = iter->data;
          break;
        }
    }

  return track;
}



GstMixerTrack *
xfce_mixer_get_default_track (GstElement *card)
{
  GstMixerTrack      *track = NULL;
  XfceMixerTrackType  track_type = G_TYPE_INVALID;
  const GList        *iter;
  GstMixerTrack      *track_tmp;
  XfceMixerTrackType  track_type_tmp;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);

  /*
   * Try to get the master track if it is a playback or capture track and not
   * read-only
   */
  for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
    {
      track_tmp = GST_MIXER_TRACK (iter->data);
      track_type_tmp = xfce_mixer_track_type_new (track_tmp);

      if (GST_MIXER_TRACK_HAS_FLAG (track_tmp, GST_MIXER_TRACK_MASTER) &&
          (track_type_tmp == XFCE_MIXER_TRACK_TYPE_PLAYBACK ||
           track_type_tmp == XFCE_MIXER_TRACK_TYPE_CAPTURE) &&
          !GST_MIXER_TRACK_HAS_FLAG (track_tmp, GST_MIXER_TRACK_READONLY))
        {
          track = track_tmp;
          track_type = track_type_tmp;
          break;
        }
    }

  /*
   * If there is no master track, try to get the first track which is a
   * playback or capture track and not read-only
   */
  if (!GST_IS_MIXER_TRACK (track) ||
      (track_type != XFCE_MIXER_TRACK_TYPE_PLAYBACK &&
       track_type != XFCE_MIXER_TRACK_TYPE_CAPTURE) ||
      GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_READONLY))
    {
      for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
        {
          track_tmp = GST_MIXER_TRACK (iter->data);
          track_type = xfce_mixer_track_type_new (track_tmp);

          if ((track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK ||
               track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE) &&
              !GST_MIXER_TRACK_HAS_FLAG (track_tmp, GST_MIXER_TRACK_READONLY))
            {
              track = track_tmp;
              break;
            }
        }
    }

  return track;
}



GList *
xfce_mixer_get_default_track_list (GstElement *card)
{
  gboolean       mixer_has_whitelist = FALSE;
  const GList   *iter;
  GList         *track_list = NULL;
  GstMixerTrack *track;
  gchar         *track_label;
  gchar         *track_label_lower;
  gint           i;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);

  if (gst_mixer_get_mixer_flags (GST_MIXER (card)) & GST_MIXER_FLAG_HAS_WHITELIST)
    mixer_has_whitelist = TRUE;

  for (iter = gst_mixer_list_tracks (GST_MIXER (card)); iter != NULL; iter = g_list_next (iter))
    {
      track = GST_MIXER_TRACK (iter->data);

      /* Use the whitelist flag when available and fall back to a static whitelist */
      if (mixer_has_whitelist)
        {
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_WHITELIST))
            track_list = g_list_prepend (track_list, track);
        }
      else
        {
          track_label = NULL;

          if (g_object_class_find_property (G_OBJECT_GET_CLASS (track), "untranslated-label"))
            g_object_get (track, "untranslated-label", &track_label, NULL);

          if (track_label == NULL)
            g_object_get (track, "label", &track_label, NULL);

          track_label_lower = g_utf8_strdown (track_label, -1);

          for (i = 0; tracks_whitelist[i] != NULL; ++i)
            {
              if (strstr (track_label_lower, tracks_whitelist[i]) != NULL)
                {
                  track_list = g_list_prepend (track_list, track);
                  break;
                }
            }

          g_free (track_label_lower);
          g_free (track_label);
        }
    }

  return track_list;
}



const gchar *
xfce_mixer_get_track_label (GstMixerTrack *track)
{
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), NULL);
  return gst_mixer_track_get_name (track);
}



gulong
xfce_mixer_bus_connect (GCallback callback,
                        gpointer  user_data)
{
  g_return_val_if_fail (refcount > 0, 0);
  return g_signal_connect (bus, "message::element", callback, user_data);
}



void
xfce_mixer_bus_disconnect (gulong signal_handler_id)
{
  g_return_if_fail (refcount > 0);

  if (signal_handler_id != 0)
    g_signal_handler_disconnect (bus, signal_handler_id);
}



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

static void
set_mixer_name (GstMixer *mixer, const gchar *name)
{
  gint               length;
  const gchar       *p;
  gchar             *internal_name;

  /* Set name to be used by xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-name",
                          g_strdup (name), (GDestroyNotify) g_free);

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

  /* Set name to be used by xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-internal-name", internal_name, (GDestroyNotify) g_free);
}



static void
_xfce_mixer_init_mixer (gpointer data,
                        gpointer user_data)
{
  GstMixer *card = GST_MIXER (data);

  set_mixer_name (card, gst_mixer_get_card_name (card));
  gst_element_set_bus (GST_ELEMENT (card), bus);
}



static void
_xfce_mixer_destroy_mixer (gpointer data)
{
  gst_element_set_state (GST_ELEMENT (data), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (data));
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

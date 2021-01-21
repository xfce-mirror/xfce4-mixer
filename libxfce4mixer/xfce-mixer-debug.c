/* vi:set expandtab sw=2 sts=2: */
/*-
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
#include <stdarg.h>

#include <gst/gst.h>

#include "libxfce4mixer.h"
#include "xfce-mixer-debug.h"



#if !GLIB_CHECK_VERSION (2,32,0)
static void
xfce_mixer_dummy_log_handler (const gchar    *log_domain,
                              GLogLevelFlags  log_level,
                              const gchar    *message,
                              gpointer        unused_data)
{
  /* Swallow all messages */
}
#endif



void
xfce_mixer_debug_init (const gchar *log_domain,
                       gboolean     debug_mode)
{
  /*
   * glib >= 2.32 only shows debug messages if G_MESSAGES_DEBUG contains the
   * log domain or "all", earlier glib version always show debugging output
   */
#if GLIB_CHECK_VERSION (2,32,0)
  const gchar        *debug_env;
  GString            *debug_env_new;

  if (debug_mode)
    {
      debug_env_new = g_string_new (NULL);
      debug_env = g_getenv ("G_MESSAGES_DEBUG");

      if (log_domain == NULL)
        g_string_append (debug_env_new, "all");
      else
        {
          if (debug_env != NULL)
            g_string_append (debug_env_new, debug_env);
          if (debug_env == NULL || strstr (debug_env, log_domain) == NULL)
            g_string_append (debug_env_new, log_domain);
          if (debug_env == NULL || strstr (debug_env, G_LOG_DOMAIN) == NULL)
            g_string_append (debug_env_new, G_LOG_DOMAIN);
        }
      g_setenv ("G_MESSAGES_DEBUG", debug_env_new->str, TRUE);
      g_string_free (debug_env_new, TRUE);
    }
#else
  if (!debug_mode)
    g_log_set_handler (log_domain, G_LOG_LEVEL_DEBUG, xfce_mixer_dummy_log_handler, NULL);
#endif /* GLIB_CHECK_VERSION (2,32,0) */
}



void
xfce_mixer_debug_real (const gchar *log_domain,
                       const gchar *file,
                       const gchar *func,
                       gint line,
                       const gchar *format, ...)
{
  va_list args;
  gchar *prefixed_format;

  va_start (args, format);
  prefixed_format = g_strdup_printf ("[%s:%d %s]: %s", file, line, func, format);
  g_logv (log_domain, G_LOG_LEVEL_DEBUG, prefixed_format, args);
  va_end (args);

  g_free (prefixed_format);
}



void
xfce_mixer_dump_gst_data (void)
{
  GString            *result;
  GList              *cards;
  GList              *cards_iter;
  GstElementFactory  *factory;
  GstElement         *card;
  gchar              *card_device_name = NULL;
  GstElement         *default_card;
  GstMixerTrack      *default_track;
  GList              *default_track_list;
  const gchar        *card_long_name;
  GstMixerFlags       mixer_flags;
  const GList        *tracks;
  const GList        *tracks_iter;
  GstMixerTrack      *track;
  gchar              *track_label;
  gchar              *track_untranslated_label;
  guint               track_index;
  GstMixerTrackFlags  track_flags;
  gint                i;
  gint                max_volume;
  gint                min_volume;
  gint               *volumes;
  GList              *options;
  GList              *options_iter;

  result = g_string_sized_new (4096);
  g_string_assign (result, "GStreamer data:\n");

  cards = xfce_mixer_get_cards ();
  default_card = xfce_mixer_get_default_card ();

  if (cards == NULL || g_list_length (cards) == 0)
    g_string_append (result, "\tno mixers found\n");

  for (cards_iter = g_list_first (cards); cards_iter != NULL; cards_iter = g_list_next (cards_iter))
    {
      card = GST_ELEMENT (cards_iter->data);

      factory = gst_element_get_factory (card);

      default_track = xfce_mixer_get_default_track (card);
      default_track_list = xfce_mixer_get_default_track_list (card);

      g_string_append (result, "\tmixer:\n");

      if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (card)), "device-name"))
        g_object_get (G_OBJECT (card), "device-name", &card_device_name, NULL);
      g_string_append_printf (result, "\t\tdevice-name: \"%s\"\n", (card_device_name != NULL) ? card_device_name : "<unknown>");

      card_long_name = gst_element_factory_get_longname (factory);
      g_string_append_printf (result, "\t\tlongname: \"%s\"\n", (card_long_name != NULL) ? card_long_name : "<unknown>");

      mixer_flags = gst_mixer_get_mixer_flags (GST_MIXER (card));
      if (mixer_flags & GST_MIXER_FLAG_AUTO_NOTIFICATIONS)
        g_string_append (result, "\t\tflag: GST_MIXER_FLAG_AUTO_NOTIFICATIONS\n");
      if (mixer_flags & GST_MIXER_FLAG_HAS_WHITELIST)
        g_string_append (result, "\t\tflag: GST_MIXER_FLAG_HAS_WHITELIST\n");

      if (card == default_card)
        g_string_append (result, "\t\txfce4-mixer default mixer\n");

      tracks = gst_mixer_list_tracks (GST_MIXER (card));
      for (tracks_iter = tracks; tracks_iter != NULL; tracks_iter = g_list_next (tracks_iter))
        {
          track = GST_MIXER_TRACK (tracks_iter->data);

          g_string_append (result, "\t\ttrack:\n");

          g_object_get (G_OBJECT (track), "label", &track_label,
                        "untranslated-label", &track_untranslated_label,
                        "index", &track_index,
                        "flags", &track_flags,
                        "max-volume", &max_volume,
                        "min-volume", &min_volume, NULL);
          g_string_append_printf (result, "\t\t\tlabel: \"%s\"\n", track_label);
          g_string_append_printf (result, "\t\t\tindex: %d\n", track_index);
          g_string_append_printf (result, "\t\t\tuntranslated-label: \"%s\"\n", track_untranslated_label);

          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_INPUT))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_INPUT\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_OUTPUT))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_OUTPUT\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_MUTE))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_MUTE\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_RECORD))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_RECORD\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_MASTER))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_MASTER\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_NO_RECORD))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_NO_RECORD\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_NO_MUTE))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_NO_MUTE\n");
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_WHITELIST))
            g_string_append (result, "\t\t\tflag: GST_MIXER_TRACK_WHITELIST\n");

          if (GST_IS_MIXER_OPTIONS (track))
            {
              g_string_append (result, "\t\t\ttype: options\n");
              options = gst_mixer_options_get_values (GST_MIXER_OPTIONS (track));
              for (options_iter = options; options_iter != NULL; options_iter = g_list_next (options_iter))
                g_string_append_printf (result, "\t\t\tvalue: \"%s\"\n", (gchar *) options_iter->data);
              g_string_append_printf (result, "\t\t\tcurrent value: \"%s\"\n", gst_mixer_get_option (GST_MIXER (card), GST_MIXER_OPTIONS (track)));
            }
          else
            {
              gint num_channels;

              num_channels = gst_mixer_track_get_num_channels(track);
              if (num_channels == 0)
                  g_string_append (result, "\t\t\ttype: switch\n");
              else
                {
                  g_string_append (result, "\t\t\ttype: volume\n");
                  g_string_append_printf (result, "\t\t\tchannels: %d\n", num_channels);
                  g_string_append_printf (result, "\t\t\tmin-volume: %d\n", min_volume);
                  g_string_append_printf (result, "\t\t\tmax-volume: %d\n", max_volume);

                  volumes = g_new0 (gint, num_channels);
                  gst_mixer_get_volume (GST_MIXER (card), track, volumes);
                  for (i = 0; i < num_channels; ++i)
                    g_string_append_printf (result, "\t\t\tvolume channel[%d]: %d\n", i, volumes[i]);

                  g_free (volumes);
                }
            }

          if (track == default_track)
            g_string_append (result, "\t\t\txfce4-mixer-plugin default track\n");

          if (g_list_find (default_track_list, track) != NULL)
            g_string_append (result, "\t\t\txfce4-mixer default mixer\n");

          g_free (track_label);
          g_free (track_untranslated_label);
        }

      g_free (card_device_name);
      card_device_name = NULL;
    }

  /* Remove trailing newline */
  if (result->str[result->len - 1] == '\n')
    result->str[--result->len] = '\0';
  g_debug ("%s", result->str);

  g_string_free (result, TRUE);
}


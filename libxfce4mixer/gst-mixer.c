/*
 * Copyright (C) 2020  Ali Abdallah <ali.abdallah@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gst-mixer.h"

typedef struct
{
  GList *tracklist;
  gchar *name;
  gchar *card_name;

} GstMixerPrivate;

enum
{
  PROP_0,
  PROP_NAME,
  PROP_CARD_NAME,
  LAST_PROP
};


G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GstMixer, gst_mixer, GST_TYPE_ELEMENT);
#define GET_PRIV(o) gst_mixer_get_instance_private(GST_MIXER(o))


static void gst_mixer_recording_changed (GstMixerTrack *track,
                                         gboolean recording,
                                         GstMixer *mixer)
{
  GstStructure *s;
  GstMessage *m;

  s = gst_structure_new (GST_MIXER_MESSAGE_NAME,
                         "type", G_TYPE_STRING, "record-toggled",
                         "track", GST_TYPE_MIXER_TRACK, track,
                         "record", G_TYPE_BOOLEAN, recording,
                         NULL);
  m = gst_message_new_element (GST_OBJECT (mixer), s);
  gst_element_post_message (GST_ELEMENT (mixer), m);
}


static void gst_mixer_volume_changed (GstMixerTrack *track,
                                      GstMixer *mixer)
{
  GstStructure *s;
  GstMessage *m;
  gint *volumes;
  GValue l = { 0, };
  GValue v = { 0, };
  int i;

  s = gst_structure_new (GST_MIXER_MESSAGE_NAME,
                         "type", G_TYPE_STRING, "volume-changed",
                         "track", GST_TYPE_MIXER_TRACK, track,
                         NULL);
  g_value_init (&l, GST_TYPE_ARRAY);
  g_value_init (&v, G_TYPE_INT);

  volumes = track->volumes;

  for (i = 0; i < NUM_CHANNELS(track); i++)
  {
    g_value_set_int (&v, volumes[i]);
    gst_value_array_append_value (&l, &v);
  }

  gst_structure_set_value (s, "volumes", &l);
  g_value_unset (&v);
  g_value_unset (&l);

  m = gst_message_new_element (GST_OBJECT (mixer), s);
  gst_element_post_message (GST_ELEMENT (mixer), m);
}


static void
gst_mixer_mute_changed (GstMixerTrack *track, gboolean mute, GstMixer *mixer)
{
  GstStructure *s;
  GstMessage *m;
  s = gst_structure_new (GST_MIXER_MESSAGE_NAME,
                         "type", G_TYPE_STRING, "mute-toggled",
                         "track", GST_TYPE_MIXER_TRACK, track,
                         "mute", G_TYPE_BOOLEAN, mute,
                         NULL);
  m = gst_message_new_element (GST_OBJECT (mixer), s);
  gst_element_post_message (GST_ELEMENT (mixer), m);
}


static void
gst_mixer_init (GstMixer *mixer)
{
  GstMixerPrivate *priv;

  priv = GET_PRIV (mixer);

  priv->tracklist = NULL;
  priv->name      = NULL;
  priv->card_name = NULL;

}


static void
gst_mixer_finalize (GObject *self)
{
  GstMixerPrivate *priv;

  priv = GET_PRIV(GST_MIXER(self));

  g_list_free_full (priv->tracklist, g_object_unref);

  g_free (priv->name);

  g_free (priv->card_name);

  G_OBJECT_CLASS (gst_mixer_parent_class)->finalize (self);
}


static void
gst_mixer_get_property (GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  GstMixerPrivate *priv = GET_PRIV(object);

  switch (prop_id)
  {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;
    case PROP_CARD_NAME:
      g_value_set_string (value, priv->card_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_mixer_set_property (GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  GstMixerPrivate *priv = GET_PRIV(object);

  switch (prop_id)
  {
    case PROP_NAME:
      priv->name = g_value_dup_string (value);
      break;
    case PROP_CARD_NAME:
      priv->card_name = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_mixer_class_init (GstMixerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstMixerClass *mixer_class = GST_MIXER_CLASS (klass);
  GParamSpec *properties[LAST_PROP] = { NULL, 0 };

  object_class->set_property = gst_mixer_set_property;
  object_class->get_property = gst_mixer_get_property;

  mixer_class->get_volume  = gst_mixer_get_volume;
  mixer_class->set_volume  = gst_mixer_set_volume;
  mixer_class->set_mute    = gst_mixer_set_mute;
  mixer_class->set_record  = gst_mixer_set_record;
  mixer_class->set_option  = gst_mixer_set_option;
  mixer_class->get_option  = gst_mixer_get_option;

  properties[PROP_NAME] =
    g_param_spec_string ("name",
                         NULL,
                         NULL,
                         NULL,
                         G_PARAM_READWRITE|
                         G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_CARD_NAME] =
    g_param_spec_string ("card-name",
                         NULL,
                         NULL,
                         NULL,
                         G_PARAM_READWRITE|
                         G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class,
                                     LAST_PROP,
                                     properties);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_finalize;
}


const gchar *gst_mixer_get_card_name (GstMixer *mixer)
{
  GstMixerPrivate *priv;
  g_return_val_if_fail (GST_IS_MIXER(mixer), NULL);

  priv = GET_PRIV(mixer);

  return priv->card_name;
}


GList *gst_mixer_list_tracks (GstMixer *mixer)
{
  GstMixerPrivate *priv;

  g_return_val_if_fail (GST_IS_MIXER(mixer), NULL);

  priv = GET_PRIV(mixer);
  return priv->tracklist;
}


GstMixerFlags gst_mixer_get_mixer_flags (GstMixer *mixer)
{
  g_return_val_if_fail (GST_IS_MIXER(mixer), GST_MIXER_FLAG_NONE);

  return GST_MIXER_GET_CLASS(mixer)->get_mixer_flags(mixer);
}


void gst_mixer_get_volume (GstMixer *mixer,
                           GstMixerTrack *track,
                           gint *volumes)
{
  g_return_if_fail(GST_IS_MIXER(mixer));
  g_return_if_fail(GST_IS_MIXER_TRACK(track));

  GST_MIXER_GET_CLASS(mixer)->get_volume(mixer, track, volumes);
}


void gst_mixer_set_volume (GstMixer *mixer,
                           GstMixerTrack *track,
                           gint *volumes)
{
  gboolean muted = TRUE;
  int i;

  g_return_if_fail(GST_IS_MIXER(mixer));
  g_return_if_fail(GST_IS_MIXER_TRACK(track));

  GST_MIXER_GET_CLASS(mixer)->set_volume(mixer, track, volumes);

  for (i = 0; i < NUM_CHANNELS(track); i++)
  {
    if (track->volumes[i] != track->min_volume)
    {
      muted = FALSE;
      break;
    }
  }

  if (IS_OUTPUT(track) && (muted != (gboolean)IS_MUTE(track)))
    gst_mixer_track_update_mute (track, muted);
  else if (IS_INPUT(track) && (muted != (gboolean)IS_RECORD(track)))
    gst_mixer_track_update_recording (track, muted);
}


void gst_mixer_set_mute (GstMixer *mixer,
                         GstMixerTrack *track,
                         gboolean mute)
{
  g_return_if_fail(GST_IS_MIXER(mixer));
  g_return_if_fail(GST_IS_MIXER_TRACK(track));

  if (IS_OUTPUT(track))
    GST_MIXER_GET_CLASS(mixer)->set_mute(mixer, track, mute);
}


void gst_mixer_set_record (GstMixer *mixer,
                           GstMixerTrack *track,
                           gboolean record)
{
  g_return_if_fail(GST_IS_MIXER(mixer));
  g_return_if_fail(GST_IS_MIXER_TRACK(track));

  if (IS_INPUT(track))
    GST_MIXER_GET_CLASS(mixer)->set_record(mixer, track, record);
}


void gst_mixer_set_option (GstMixer *mixer,
                            GstMixerOptions *opts,
                            gchar *value)
{
  g_return_if_fail(GST_IS_MIXER(mixer));
  g_return_if_fail(GST_IS_MIXER_OPTIONS(opts));

  GST_MIXER_GET_CLASS(mixer)->set_option(mixer, opts, value);
}


const gchar* gst_mixer_get_option (GstMixer *mixer,
                                   GstMixerOptions *opts)
{
  g_return_val_if_fail(GST_IS_MIXER(mixer), NULL);
  g_return_val_if_fail(GST_IS_MIXER_OPTIONS(opts), NULL);

  return GST_MIXER_GET_CLASS(mixer)->get_option(mixer, opts);
}


GstMixerMessageType
gst_mixer_message_get_type (GstMessage * message)
{
  const GstStructure *s;
  const gchar *m_type;

  s = gst_message_get_structure (message);
  m_type = gst_structure_get_string (s, "type");
  if (!m_type)
    return GST_MIXER_MESSAGE_INVALID;

  if (g_str_equal (m_type, "mute-toggled"))
    return GST_MIXER_MESSAGE_MUTE_TOGGLED;
  else if (g_str_equal (m_type, "record-toggled"))
    return GST_MIXER_MESSAGE_RECORD_TOGGLED;
  else if (g_str_equal (m_type, "volume-changed"))
    return GST_MIXER_MESSAGE_VOLUME_CHANGED;
  else if (g_str_equal (m_type, "option-changed"))
    return GST_MIXER_MESSAGE_OPTION_CHANGED;
  else if (g_str_equal (m_type, "options-list-changed"))
    return GST_MIXER_MESSAGE_OPTIONS_LIST_CHANGED;
  else if (g_str_equal (m_type, "mixer-changed"))
    return GST_MIXER_MESSAGE_MIXER_CHANGED;

  return GST_MIXER_MESSAGE_INVALID;
}


static void message_parse_track (const GstStructure *s, GstMixerTrack **track)
{
  if (track) {
    const GValue *v = gst_structure_get_value (s, "track");
    *track = (GstMixerTrack *)g_value_get_object (v);
  }
}




void gst_mixer_add_track (GstMixer *mixer,
                          GstMixerTrack *track)
{
  GstMixerPrivate *priv;

  g_return_if_fail (GST_IS_MIXER(mixer));
  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  priv = GET_PRIV(mixer);

  if (IS_OUTPUT(track))
  {
    g_signal_connect (track, "volume-changed",
                      G_CALLBACK (gst_mixer_volume_changed),
                      mixer);

    g_signal_connect (track, "mute-changed",
                      G_CALLBACK (gst_mixer_mute_changed),
                      mixer);
  }

  if (IS_INPUT(track))
  {
    g_signal_connect (track, "recording-changed",
                      G_CALLBACK (gst_mixer_recording_changed),
                      mixer);

    g_signal_connect (track, "mute-changed",
                      G_CALLBACK (gst_mixer_mute_changed),
                      mixer);
  }

  priv->tracklist = g_list_append (priv->tracklist, track);
}


void gst_mixer_message_parse_mute_toggled (GstMessage *message,
                                           GstMixerTrack **track,
                                           gboolean *mute)
{
  const GstStructure *s = gst_message_get_structure (message);

  message_parse_track (s, track);
  if (mute)
    gst_structure_get_boolean (s, "mute", mute);
}


void gst_mixer_message_parse_record_toggled (GstMessage *message,
                                             GstMixerTrack **track,
                                             gboolean *record)
{
  const GstStructure *s = gst_message_get_structure (message);

  message_parse_track (s, track);
  if (record)
    gst_structure_get_boolean (s, "record", record);
}


void gst_mixer_message_parse_volume_changed (GstMessage *message,
                                             GstMixerTrack **track,
                                             gint **volumes,
                                             gint *num_channels)
{
  const GstStructure *s = gst_message_get_structure (message);

  message_parse_track (s, track);
  if (volumes || num_channels) {
    gint n_chans, i;
    const GValue *v = gst_structure_get_value (s, "volumes");

    n_chans = gst_value_array_get_size (v);
    if (num_channels)
      *num_channels = n_chans;

    if (volumes) {
      *volumes = g_new (gint, n_chans);
      for (i = 0; i < n_chans; i++) {
        const GValue *e = gst_value_array_get_value (v, i);

        (*volumes)[i] = g_value_get_int (e);
      }
    }
  }
}


static void message_parse_options (const GstStructure *s,
                                   GstMixerOptions ** options)
{
  if (options) {
    const GValue *v = gst_structure_get_value (s, "options");
    *options = (GstMixerOptions *) g_value_get_object (v);
  }
}


void gst_mixer_message_parse_option_changed (GstMessage *message,
                                             GstMixerOptions ** options,
                                             const gchar **value)
{
  const GstStructure *s = gst_message_get_structure (message);

  message_parse_options (s, options);
  if (value)
    *value = gst_structure_get_string (s, "value");
}


void gst_mixer_message_parse_options_list_changed (GstMessage *message,
               GstMixerOptions **options)
{
  const GstStructure *s = gst_message_get_structure (message);

  message_parse_options (s, options);
}

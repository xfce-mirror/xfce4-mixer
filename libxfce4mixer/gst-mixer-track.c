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

#include "gst-mixer-track.h"

enum {
  PROP_0,
  PROP_LABEL,
  PROP_UNTRANSLATED_LABEL,
  PROP_INDEX,
  PROP_FLAGS,
  PROP_HAS_VOLUME,
  PROP_HAS_SWITCH,
  PROP_NUM_CHANNELS,
  PROP_MAX_VOLUME,
  PROP_MIN_VOLUME,

  LAST_PROP,
};

enum
{
  VOLUME_CHANGED,
  MUTE_CHANGED,
  RECORDING_CHANGED,
  LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

G_DEFINE_ABSTRACT_TYPE (GstMixerTrack, gst_mixer_track, G_TYPE_OBJECT)

static void
gst_mixer_track_init (GstMixerTrack *track)
{

  track->label = NULL;
  track->untranslated_label = NULL;

  track->volumes = 0;
  track->shared_mute = NULL;
}


static void
gst_mixer_track_finalize (GObject *self)
{
  G_OBJECT_CLASS (gst_mixer_track_parent_class)->finalize (self);
}


static void gst_mixer_track_get_property (GObject *object,
                                          guint prop_id,
                                          GValue *value,
                                          GParamSpec *pspec)
{
  GstMixerTrack *track = GST_MIXER_TRACK (object);

  switch (prop_id)
  {
    case PROP_LABEL:
      g_value_set_string (value, track->label);
      break;
    case PROP_UNTRANSLATED_LABEL:
      g_value_set_string (value, track->untranslated_label);
      break;
    case PROP_INDEX:
      g_value_set_int (value, track->index);
      break;
    case PROP_FLAGS:
      g_value_set_int (value, track->flags);
      break;
    case PROP_HAS_VOLUME:
      g_value_set_boolean (value, track->has_volume);
      break;
    case PROP_HAS_SWITCH:
      g_value_set_boolean (value, track->has_switch);
      break;
    case PROP_NUM_CHANNELS:
      g_value_set_int (value, track->num_channels);
      break;
    case PROP_MIN_VOLUME:
      g_value_set_int (value, track->min_volume);
      break;
    case PROP_MAX_VOLUME:
      g_value_set_int (value, track->max_volume);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void gst_mixer_track_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec)
{
  GstMixerTrack *track = GST_MIXER_TRACK (object);

  switch (prop_id)
  {
    case PROP_LABEL:
      track->label = g_value_dup_string (value);
      break;
    case PROP_UNTRANSLATED_LABEL:
      track->untranslated_label = g_value_dup_string (value);
      break;
    case PROP_INDEX:
      track->index = g_value_get_int (value);
      break;
    case PROP_FLAGS:
      track->flags = g_value_get_int (value);
      break;
    case PROP_HAS_VOLUME:
      track->has_volume = g_value_get_boolean (value);
      break;
    case PROP_HAS_SWITCH:
      track->has_switch = g_value_get_boolean (value);
      break;
    case PROP_NUM_CHANNELS:
      track->num_channels = g_value_get_int (value);
      break;
    case PROP_MIN_VOLUME:
      track->min_volume = g_value_get_int (value);
      break;
    case PROP_MAX_VOLUME:
      track->max_volume = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_mixer_track_class_init (GstMixerTrackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *properties[LAST_PROP] = { NULL, 0 };

  object_class->set_property = gst_mixer_track_set_property;
  object_class->get_property = gst_mixer_track_get_property;

  signals [VOLUME_CHANGED] =
    g_signal_new ("volume-changed",
                  GST_TYPE_MIXER_TRACK,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET(GstMixerTrackClass,
                                  volume_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  signals [MUTE_CHANGED] =
    g_signal_new ("mute-changed",
                  GST_TYPE_MIXER_TRACK,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET(GstMixerTrackClass,
                                  mute_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  signals [RECORDING_CHANGED] =
    g_signal_new ("recording-changed",
                  GST_TYPE_MIXER_TRACK,
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET(GstMixerTrackClass,
                                  recording_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__BOOLEAN,
                  G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

  properties[PROP_LABEL] =
    g_param_spec_string ("label",
                         NULL,
                         NULL,
                         NULL,
                         G_PARAM_READWRITE|
                         G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_UNTRANSLATED_LABEL] =
    g_param_spec_string ("untranslated-label",
                         NULL,
                         NULL,
                         NULL,
                         G_PARAM_READWRITE|
                         G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_INDEX] =
    g_param_spec_int ("index",
                      NULL,
                      NULL,
                      0, G_MAXINT, 1,
                      G_PARAM_READWRITE|
                      G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_FLAGS] =
    g_param_spec_int ("flags",
                       NULL,
                       NULL,
                       0, 100, GST_MIXER_TRACK_INVALID,
                       G_PARAM_READWRITE|
                       G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_HAS_VOLUME] =
    g_param_spec_boolean ("has-volume",
                          NULL,
                          NULL,
                          FALSE,
                          G_PARAM_READWRITE|
                          G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_HAS_SWITCH] =
    g_param_spec_boolean ("has-switch",
                          NULL,
                          NULL,
                          FALSE,
                          G_PARAM_READWRITE|
                          G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_NUM_CHANNELS] =
    g_param_spec_int ("num-channels",
                      NULL,
                      NULL,
                      0, 255, 0,
                      G_PARAM_READWRITE|
                      G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_MIN_VOLUME] =
    g_param_spec_int ("min-volume",
                      NULL,
                      NULL,
                      0, 0, 0,
                      G_PARAM_READWRITE|
                      G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_MAX_VOLUME] =
    g_param_spec_int ("max-volume",
                      NULL,
                      NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE|
                      G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class,
                                     LAST_PROP,
                                     properties);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_track_finalize;
}


const gchar *gst_mixer_track_get_name (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), NULL);

  return track->label;
}


GstMixerTrackFlags gst_mixer_track_get_flags (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), GST_MIXER_TRACK_INVALID);

  return track->flags;
}


gint gst_mixer_track_get_num_channels (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), 0);

  return track->num_channels;
}


gboolean gst_mixer_track_get_has_volume (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), FALSE);

  return track->has_volume;
}


gboolean gst_mixer_track_get_has_switch (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), FALSE);

  return track->has_switch;
}


gint gst_mixer_track_get_min_volume (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), 0);

  return track->min_volume;
}


gint gst_mixer_track_get_max_volume (GstMixerTrack *track)
{
  g_return_val_if_fail(GST_IS_MIXER_TRACK(track), 0);

  return track->max_volume;
}


void gst_mixer_track_update_recording (GstMixerTrack *track, gboolean recording)
{
  GstMixerTrackFlags old_flag = track->flags & GST_MIXER_TRACK_RECORD;
  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  if (recording)
    track->flags |= GST_MIXER_TRACK_RECORD;
  else
    track->flags &= ~GST_MIXER_TRACK_RECORD;

  if ((track->flags & GST_MIXER_TRACK_RECORD) != old_flag)
    g_signal_emit_by_name (track, "recording-changed", 0, recording);
}


void gst_mixer_track_update_mute (GstMixerTrack *track, gboolean mute)
{
  GstMixerTrackFlags old_flag = track->flags & GST_MIXER_TRACK_MUTE;

  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  if (mute)
  {
    track->flags |= GST_MIXER_TRACK_MUTE;
    if (track->shared_mute)
      track->shared_mute->flags |= GST_MIXER_TRACK_MUTE;
  }
  else
  {
    track->flags &= ~GST_MIXER_TRACK_MUTE;
    if (track->shared_mute)
      track->shared_mute->flags &= ~GST_MIXER_TRACK_MUTE;
  }

  if ((track->flags & GST_MIXER_TRACK_MUTE) != old_flag)
    g_signal_emit_by_name (track, "mute-changed", 0, mute);
}


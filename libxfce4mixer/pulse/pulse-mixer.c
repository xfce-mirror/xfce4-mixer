/*
 * Copyright (C) 2020-2021  Ali Abdallah <ali.abdallah@suse.com>
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


#include <pulse/glib-mainloop.h>

#include <libxfce4ui/libxfce4ui.h>

#include "pulse-mixer.h"
#include "pulse-track.h"
#include "pulse-options.h"

static void gst_mixer_pulse_state_cb (pa_context    *context,
                                      GstMixerPulse *pulse);

struct _GstMixerPulse
{
  GstMixer parent;

  pa_threaded_mainloop     *mainloop;
  pa_context               *context;
  GHashTable               *clients;
};

G_DEFINE_TYPE (GstMixerPulse, gst_mixer_pulse, GST_TYPE_MIXER)

static void
gst_mixer_pulse_finalize (GObject *self)
{
  GstMixerPulse *mixer = GST_MIXER_PULSE (self);

  g_hash_table_unref (mixer->clients);

  pa_threaded_mainloop_free (mixer->mainloop);

  G_OBJECT_CLASS (gst_mixer_pulse_parent_class)->finalize (self);
}


static void
gst_mixer_pulse_init (GstMixerPulse *mixer)
{
  mixer->clients = g_hash_table_new_full (g_direct_hash,
                                          g_direct_equal,
                                          NULL,
                                          g_free);

  mixer->mainloop = pa_threaded_mainloop_new();
  g_assert (mixer->mainloop);

  mixer->context = pa_context_new (pa_threaded_mainloop_get_api (mixer->mainloop), NULL);
  g_assert (mixer->context);

  pa_context_set_state_callback(mixer->context,
                                (pa_context_notify_cb_t)gst_mixer_pulse_state_cb,
                                mixer);
}


static GstMixerFlags
gst_mixer_pulse_get_mixer_flags (GstMixer *mixer)
{
  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}


static void gst_mixer_pulse_set_sink_input_volume (pa_context *context,
                                                   const pa_sink_input_info *info,
                                                   int eol,
                                                   void *userdata)
{
  GstMixerTrack *track;
  pa_operation *o;
  pa_cvolume vol;
  int i;


  if (info == NULL) return;

  track = GST_MIXER_TRACK(userdata);

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) track->volumes[i];
  }

  o = pa_context_set_sink_input_volume(context,
                                       info->index,
                                       &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
  {
    g_signal_emit_by_name (track, "volume-changed", 0);
    pa_operation_unref (o);
  }

}


static void gst_mixer_pulse_set_sink_volume (pa_context *context,
                                             const pa_sink_info *info,
                                             int eol,
                                             void *userdata)
{
  GstMixerTrack *track;
  pa_operation *o;
  pa_cvolume vol;
  int i;


  if (info == NULL) return;

  track = GST_MIXER_TRACK(userdata);

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) track->volumes[i];
  }

  o = pa_context_set_sink_volume_by_index(context,
                                          info->index,
                                          &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
  {
    g_signal_emit_by_name (track, "volume-changed", 0);
    pa_operation_unref (o);
  }
}


static void gst_mixer_pulse_set_source_volume (pa_context *context,
                                               const pa_source_info *info,
                                               int eol,
                                               void *userdata)
{
  GstMixerTrack *track;
  pa_operation *o;
  pa_cvolume vol;
  int i;

  if (info == NULL) return;

  track = GST_MIXER_TRACK(userdata);

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) track->volumes[i];
  }

  o = pa_context_set_source_volume_by_index(context,
                                            info->index,
                                            &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
  {
    g_signal_emit_by_name (track, "volume-changed", 0);
    pa_operation_unref (o);
  }
}


static void gst_mixer_pulse_set_source_output_volume (pa_context *context,
                                                      const pa_source_output_info *info,
                                                      int eol,
                                                      void *userdata)
{
  GstMixerTrack *track;
  pa_operation *o;
  pa_cvolume vol;
  int i;

  if (info == NULL) return;

  track = GST_MIXER_TRACK(userdata);

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) track->volumes[i];
  }

  o = pa_context_set_source_volume_by_index(context,
                                            info->index,
                                            &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
  {
    g_signal_emit_by_name (track, "volume-changed", 0);
    pa_operation_unref (o);
  }
}


static void
gst_mixer_pulse_set_volume (GstMixer *mixer, GstMixerTrack *track, gint num_channels, gint *volumes)
{
  GstMixerPulse *pulse = GST_MIXER_PULSE (mixer);
  int i;

  for (i = 0; i < num_channels; i++)
    track->volumes[i] = volumes[i];

  if (IS_OUTPUT(track))
  {
    /* Software track */
    if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_SOFTWARE))
    {
      g_debug ("track index %d\n", track->index);
      pa_context_get_sink_input_info (pulse->context,
                                      track->index,
                                      gst_mixer_pulse_set_sink_input_volume,
                                      track);
    }
    else
    {
      pa_context_get_sink_info_by_name (pulse->context,
                                        track->untranslated_label,
                                        gst_mixer_pulse_set_sink_volume,
                                        track);
    }
  }
  else
  {
    /* Software track */
    if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_SOFTWARE))
    {
      pa_context_get_source_output_info (pulse->context,
                                         track->index,
                                         gst_mixer_pulse_set_source_output_volume,
                                         track);
    }
    else
    {
      pa_context_get_source_info_by_name (pulse->context,
                                          track->untranslated_label,
                                          gst_mixer_pulse_set_source_volume,
                                          track);
    }
  }
}


static void
gst_mixer_pulse_get_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  int i;

  for (i = 0; i < NUM_CHANNELS(track); i++)
  {
    volumes[i] = track->volumes[i];
  }
}


static void gst_mixer_pulse_track_record_changed_cb (pa_context *context,
                                                     int success,
                                                     void *userdata)
{
  GstMixerTrack *track;

  track = GST_MIXER_TRACK(userdata);

  if (success)
    gst_mixer_track_update_recording (track, !(track->flags & GST_MIXER_TRACK_RECORD));

}


static void gst_mixer_pulse_track_mute_changed_cb (pa_context *context,
                                                   int success,
                                                   void *userdata)
{
  GstMixerTrack *track;

  track = GST_MIXER_TRACK(userdata);

  if (success)
    gst_mixer_track_update_mute (track, !(track->flags & GST_MIXER_TRACK_MUTE));

}


static void
gst_mixer_pulse_set_record (GstMixer * mixer, GstMixerTrack *track, gboolean record)
{
  GstMixerPulse *pulse = GST_MIXER_PULSE (mixer);

  pa_context_set_source_mute_by_name (pulse->context,
                                      track->untranslated_label,
                                      record,
                                      gst_mixer_pulse_track_record_changed_cb,
                                      track);
}


static void
gst_mixer_pulse_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  GstMixerPulse *pulse = GST_MIXER_PULSE (mixer);

  pa_context_set_sink_mute_by_name (pulse->context,
                                    track->untranslated_label,
                                    mute,
                                    gst_mixer_pulse_track_mute_changed_cb,
                                    track);
}

static void gst_mixer_track_input_moved (pa_context *context,
                                         const pa_sink_input_info *info,
                                         int eol,
                                         void *userdata)
{
  GstMixerTrack *track;

  if (info == NULL) return;

  track = GST_MIXER_TRACK(userdata);

  g_object_set (G_OBJECT(track),
                "parent-track-id", info->sink,
                NULL);
}

static void gst_mixer_pulse_track_input_moved_cb (pa_context *context,
                                                  int success,
                                                  void *userdata)
{
  GstMixerTrack *track;

  track = GST_MIXER_TRACK(userdata);

  /* FIXME: Handle errors */
  if (success)
    pa_context_get_sink_input_info (context,
                                    gst_mixer_track_get_id(track),
                                    gst_mixer_track_input_moved,
                                    track);
}

static const gchar *
gst_mixer_pulse_get_option (GstMixer *mixer, GstMixerOptions *opts)
{
  return NULL;
}


static void
gst_mixer_pulse_set_option (GstMixer *mixer, GstMixerOptions *opts, gchar *value)
{
}

static void gst_mixer_pulse_move_track (GstMixer *mixer,
                                        GstMixerTrack *track,
                                        gint track_number)
{
  GstMixerPulse *pulse = GST_MIXER_PULSE (mixer);
  pa_context_move_sink_input_by_index (pulse->context,
                                       gst_mixer_track_get_id(track),
                                       track_number,
                                       gst_mixer_pulse_track_input_moved_cb,
                                       track);
}


static void
gst_mixer_pulse_class_init (GstMixerPulseClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_klass = GST_ELEMENT_CLASS (klass);
  GstMixerClass *mixer_class = GST_MIXER_CLASS (klass);

  gst_element_class_set_static_metadata (element_klass,
                                         "PULSE mixer", "Generic/Audio",
                                         "Control audio mixer via PULSE API",
                                         "Ali Abdallah <ali.abdallah@suse.com>");

  mixer_class->get_mixer_flags = gst_mixer_pulse_get_mixer_flags;
  mixer_class->set_volume  = gst_mixer_pulse_set_volume;
  mixer_class->get_volume  = gst_mixer_pulse_get_volume;
  mixer_class->set_record  = gst_mixer_pulse_set_record;
  mixer_class->set_mute    = gst_mixer_pulse_set_mute;
  mixer_class->get_option  = gst_mixer_pulse_get_option;
  mixer_class->set_option  = gst_mixer_pulse_set_option;
  mixer_class->move_track  = gst_mixer_pulse_move_track;

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_pulse_finalize;
}


static GstMixerTrack *
gst_mixer_pulse_get_track (GstMixer *mixer, int index, int flag)
{
  GstMixerTrack *track = NULL;
  GList *tracks, *l;

  tracks = gst_mixer_list_tracks(mixer);
  for ( l = tracks; l; l = l->next)
  {
    track = GST_MIXER_TRACK(l->data);
    if (track->index == index && GST_MIXER_TRACK_HAS_FLAG (track, flag))
    {
      break;
    }
  }
  return track;
}


static void
gst_mixer_pulse_sink_changed (pa_context         *context,
                              const pa_sink_info *info,
                              int                 eol,
                              void               *userdata)
{
  GstMixerPulse *pulse;
  GstMixerTrack *track;
  gboolean vol_changed = FALSE;

  pulse = GST_MIXER_PULSE(userdata);

  if (info == NULL || eol > 0)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  track = gst_mixer_pulse_get_track (GST_MIXER(pulse), info->index, GST_MIXER_TRACK_OUTPUT);

  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  for (int i = 0; i < info->channel_map.channels; i++)
  {
    if (track->volumes[i] != (int)info->volume.values[i])
    {
      vol_changed = TRUE;
      track->volumes[i] = info->volume.values[i];
    }
  }

  if (vol_changed)
    g_signal_emit_by_name (track, "volume-changed", 0);
}


static void
gst_mixer_pulse_sink_input_changed (pa_context               *context,
                                    const pa_sink_input_info *info,
                                    int                       eol,
                                    void                     *userdata)
{
  GstMixerPulse *pulse;
  GstMixerTrack *track;
  gboolean vol_changed = FALSE;

  pulse = GST_MIXER_PULSE(userdata);

  if (info == NULL || eol > 0 || info->client == PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  track = gst_mixer_pulse_get_track (GST_MIXER(pulse), info->index, GST_MIXER_TRACK_OUTPUT | GST_MIXER_TRACK_SOFTWARE);

  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  for (int i = 0; i < info->channel_map.channels; i++)
  {
    if (track->volumes[i] != (int)info->volume.values[i])
    {
      vol_changed = TRUE;
      track->volumes[i] = info->volume.values[i];
    }
  }

  if (vol_changed)
    g_signal_emit_by_name (track, "volume-changed", 0);
}


static void
gst_mixer_pulse_source_changed (pa_context           *context,
                                const pa_source_info *info,
                                int                   eol,
                                void                 *userdata)
{
  GstMixerPulse *pulse;
  GstMixerTrack *track;
  gboolean vol_changed = FALSE;

  pulse = GST_MIXER_PULSE(userdata);

  if (info == NULL || eol > 0)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  track = gst_mixer_pulse_get_track (GST_MIXER(pulse), info->index, GST_MIXER_TRACK_INPUT);

  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  for (int i = 0; i < info->channel_map.channels; i++)
  {
    if (track->volumes[i] != (int)info->volume.values[i])
    {
      vol_changed = TRUE;
      track->volumes[i] = info->volume.values[i];
    }
  }

  if (vol_changed)
    g_signal_emit_by_name (track, "volume-changed", 0);
}


static void
gst_mixer_pulse_source_output_changed (pa_context                  *context,
                                       const pa_source_output_info *info,
                                       int                          eol,
                                       void                        *userdata)
{
  GstMixerPulse *pulse;
  GstMixerTrack *track;
  const gchar *prop;
  gboolean vol_changed = FALSE;

  pulse = GST_MIXER_PULSE(userdata);

  if (info == NULL || eol > 0 || info->client == PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  if ((prop = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_ID))) {
    if (strcmp(prop, "org.PulseAudio.pavucontrol") == 0
        || strcmp(prop, "org.gnome.VolumeControl") == 0
        || strcmp(prop, "org.kde.kmixd") == 0)
    {
      return;
    }
  }

  track = gst_mixer_pulse_get_track (GST_MIXER(pulse), info->index, GST_MIXER_TRACK_INPUT | GST_MIXER_TRACK_SOFTWARE);

  g_return_if_fail (GST_IS_MIXER_TRACK(track));

  for (int i = 0; i < info->channel_map.channels; i++)
  {
    if (track->volumes[i] != (int)info->volume.values[i])
    {
      vol_changed = TRUE;
      track->volumes[i] = info->volume.values[i];
    }
  }

  if (vol_changed)
    g_signal_emit_by_name (track, "volume-changed", 0);
}


static void
gst_mixer_pulse_get_sink_input_cb (pa_context               *context,
                                   const pa_sink_input_info *info,
                                   int                       eol,
                                   GstMixerPulse            *pulse)
{
  GstMixerPulseTrack *track;
  const gchar *name;
  gchar *full_name;
  int i;

  if (info == NULL || eol > 0 || info->client == PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  name = (const gchar *)g_hash_table_lookup(pulse->clients, GINT_TO_POINTER(info->client));

  if(name)
    full_name = g_strdup_printf ("%s : %s", name, info->name);
  else
    full_name = g_strdup (info->name);

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", full_name,
                        "untranslated-label", info->name,
                        "index", info->index,
                        "flags", GST_MIXER_TRACK_OUTPUT | GST_MIXER_TRACK_SOFTWARE,
                        "parent-track-id", info->sink,
                        "num-channels", info->channel_map.channels,
                        "has-volume", TRUE,
                        "has-switch", TRUE,
                        "min-volume", PA_VOLUME_MUTED,
                        "max-volume", PA_VOLUME_NORM,
                        NULL);

  GST_MIXER_TRACK(track)->volumes = g_new (gint, info->channel_map.channels);

  for (i = 0; i < info->channel_map.channels; i++)
  {
    GST_MIXER_TRACK(track)->volumes[i] = info->volume.values[i];
  }

  g_free(full_name);

  pa_threaded_mainloop_signal(pulse->mainloop, 0);

  gst_mixer_track_added (GST_MIXER(pulse), GST_MIXER_TRACK(track));
}


static void
gst_mixer_pulse_get_source_output_cb (pa_context                  *context,
                                      const pa_source_output_info *info,
                                      int                          eol,
                                      GstMixerPulse               *pulse)
{
  GstMixerPulseTrack *track;
  const gchar *name;
  const gchar *prop;
  gchar *full_name;
  int i;

  if (info == NULL || eol < 0 || info->client == PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  if (eol < 0)
  {
    if (pa_context_errno(context) == PA_ERR_NOENTITY)
    {
      pa_threaded_mainloop_signal(pulse->mainloop, 0);
      return;
    }
  }

  if ((prop = pa_proplist_gets(info->proplist, "module-stream-restore.id")))
  {
    if (strcmp(prop, "sink-input-by-media-role:event") == 0) {
      pa_threaded_mainloop_signal(pulse->mainloop, 0);
      return;
    }
  }

  if ((prop = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_ID))) {
    if (strcmp(prop, "org.PulseAudio.pavucontrol") == 0
        || strcmp(prop, "org.gnome.VolumeControl") == 0
        || strcmp(prop, "org.kde.kmixd") == 0)
    {
      return;
    }
  }

  name = (const gchar *)g_hash_table_lookup(pulse->clients, GINT_TO_POINTER(info->client));

  if(name)
    full_name = g_strdup_printf ("%s : %s", name, info->name);
  else
    full_name = g_strdup (info->name);

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", full_name,
                        "untranslated-label", info->name,
                        "index", info->index,
                        "flags", GST_MIXER_TRACK_INPUT | GST_MIXER_TRACK_SOFTWARE,
                        "parent-track-id", info->source,
                        "num-channels", info->channel_map.channels,
                        "has-volume", TRUE,
                        "has-switch", TRUE,
                        "min-volume", PA_VOLUME_MUTED,
                        "max-volume", PA_VOLUME_NORM,
                        NULL);

  g_free(full_name);

  GST_MIXER_TRACK(track)->volumes = g_new (gint, info->channel_map.channels);

  for (i = 0; i < info->channel_map.channels; i++)
  {
    GST_MIXER_TRACK(track)->volumes[i] = info->volume.values[i];
  }

  pa_threaded_mainloop_signal(pulse->mainloop, 0);

  gst_mixer_track_added (GST_MIXER(pulse), GST_MIXER_TRACK(track));
}


static void
gst_mixer_pulse_event_cb (pa_context                   *context,
                          pa_subscription_event_type_t  t,
                          uint32_t                      index,
                          GstMixerPulse                *pulse)
{
  GstMixer *mixer;
  pa_operation *o = NULL;

  mixer = GST_MIXER(pulse);

  switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
  {
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
      if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
      {
        g_debug ("Removing sink track index %d\n", index);
        gst_mixer_remove_track_with_flags (mixer, GST_MIXER_TRACK_OUTPUT, index);
      }
      else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW)
      {
        g_debug ("New sink track index %d\n", index);
        o =
          pa_context_get_sink_input_info(pulse->context,
                                         index,
                                         (pa_sink_input_info_cb_t)gst_mixer_pulse_get_sink_input_cb,
                                         pulse);
      }
      else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE)
      {
        o =
          pa_context_get_sink_input_info (pulse->context,
                                          index,
                                          gst_mixer_pulse_sink_input_changed,
                                          pulse);
      }
      break;
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
      if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
      {
        g_debug ("Removing source track index %d\n", index);
        gst_mixer_remove_track_with_flags (mixer, GST_MIXER_TRACK_INPUT, index);
      }
      else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW)
      {
        g_debug ("New source track index %d\n", index);
        o =
          pa_context_get_source_output_info(pulse->context,
                                            index,
                                            (pa_source_output_info_cb_t)gst_mixer_pulse_get_source_output_cb,
                                            pulse);
      }
      else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE)
      {
        o =
          pa_context_get_source_output_info (pulse->context,
                                             index,
                                             gst_mixer_pulse_source_output_changed,
                                             pulse);
      }
      break;
    case PA_SUBSCRIPTION_EVENT_SINK:
      o =
        pa_context_get_sink_info_by_index (pulse->context,
                                           index,
                                           gst_mixer_pulse_sink_changed,
                                           pulse);
      break;
    case PA_SUBSCRIPTION_EVENT_SOURCE:
      o =
        pa_context_get_source_info_by_index (pulse->context,
                                             index,
                                             gst_mixer_pulse_source_changed,
                                             pulse);
      break;
    default:
      break;
  }

  if (o)
    pa_operation_unref(o);
}


static void
gst_mixer_pulse_state_cb (pa_context    *context,
                          GstMixerPulse *pulse)
{
  pa_operation *o;

  switch (pa_context_get_state(context))
  {
    case PA_CONTEXT_READY:
      pa_context_set_subscribe_callback(context,
                                        (pa_context_subscribe_cb_t)gst_mixer_pulse_event_cb,
                                        pulse);

      if (!(o = pa_context_subscribe(context, (pa_subscription_mask_t)
                                     (PA_SUBSCRIPTION_MASK_SINK|
                                      PA_SUBSCRIPTION_MASK_SOURCE|
                                      PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                      PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                      PA_SUBSCRIPTION_MASK_CLIENT|
                                      PA_SUBSCRIPTION_MASK_SERVER|
                                      PA_SUBSCRIPTION_MASK_CARD),
                                     NULL, NULL)))
      {
        g_warning ("pa_context_subscribe() failed");
        return;
      }
      pa_operation_unref(o);
      pa_threaded_mainloop_signal(pulse->mainloop, 0);
      break;

    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED:
      pa_threaded_mainloop_signal(pulse->mainloop, 0);
      break;

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
      break;
  }
}


static void
gst_mixer_pulse_get_sink_cb (pa_context           *context,
                             const pa_sink_info   *info,
                             int                   eol,
                             GstMixerPulse        *pulse)
{
  GstMixerPulseTrack *track;
  gchar *name;
  int i;

  if (info == NULL || eol > 0)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  if (info->card != PA_INVALID_INDEX)
    name = g_strdup_printf("%s (%s:%d)", info->description, _("Card"), info->card);
  else
    name = g_strdup(info->description);

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", name,
                        "untranslated-label", info->name,
                        "index", info->index,
                        "flags", GST_MIXER_TRACK_OUTPUT,
                        /*(info->index == 0) ? GST_MIXER_TRACK_MASTER : GST_MIXER_TRACK_NONE,*/
                        "num-channels", info->channel_map.channels,
                        "has-volume", TRUE,
                        "has-switch", TRUE,
                        "min-volume", PA_VOLUME_MUTED,
                        "max-volume", PA_VOLUME_NORM,
                        NULL);

  g_free(name);

  gst_mixer_new_track (GST_MIXER(pulse), GST_MIXER_TRACK(track));

  GST_MIXER_TRACK(track)->volumes = g_new (gint, info->channel_map.channels);

  for (i = 0; i < info->channel_map.channels; i++)
  {
    GST_MIXER_TRACK(track)->volumes[i] = info->volume.values[i];
  }

  pa_threaded_mainloop_signal(pulse->mainloop, 0);
}


static void
gst_mixer_pulse_get_source_cb (pa_context             *context,
                               const pa_source_info   *info,
                               int                     eol,
                               GstMixerPulse          *pulse)
{
  GstMixerPulseTrack *track;
  gchar *name;
  int i;

  if (info == NULL || eol > 0 || info->monitor_of_sink != PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  if (info->card != PA_INVALID_INDEX)
    name = g_strdup_printf("%s (%s:%d)", info->description, _("Card"), info->card);
  else
    name = g_strdup(info->description);

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", name,
                        "untranslated-label", info->name,
                        "index", info->index,
                        "flags", GST_MIXER_TRACK_INPUT,
                        "num-channels", info->channel_map.channels,
                        "has-volume", TRUE,
                        "has-switch", TRUE,
                        "min-volume", PA_VOLUME_MUTED,
                        "max-volume", PA_VOLUME_NORM,
                        NULL);

  g_free(name);
  gst_mixer_new_track (GST_MIXER(pulse), GST_MIXER_TRACK(track));

  GST_MIXER_TRACK(track)->volumes = g_new (gint, info->channel_map.channels);

  for (i = 0; i < info->channel_map.channels; i++)
  {
    GST_MIXER_TRACK(track)->volumes[i] = info->volume.values[i];
  }

  pa_threaded_mainloop_signal(pulse->mainloop, 0);
}


static void
gst_mixer_pulse_get_client_cb (pa_context           *context,
                               const pa_client_info *info,
                               int                   eol,
                               GstMixerPulse        *pulse)
{
  if (info == NULL || eol > 0)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  g_debug ("Inserting %s %d\n", info->name, info->index);

  g_hash_table_insert (pulse->clients,
                       GINT_TO_POINTER(info->index),
                       g_strdup(info->name));

  pa_threaded_mainloop_signal(pulse->mainloop, 0);
}




static int
gst_mixer_pulse_new (GstMixer **mixer_ret)
{
  GstMixerPulse *pulse;
  pa_operation *o;
  int err;

  pulse = g_object_new (GST_MIXER_TYPE_PULSE,
                        "name", "pulse",
                        "card-name", g_strdup (_("Pulse Audio Volume Control")),
                        NULL);

  pa_threaded_mainloop_start(pulse->mainloop);
  pa_threaded_mainloop_lock(pulse->mainloop);

  err = pa_context_connect (pulse->context, NULL, PA_CONTEXT_NOFAIL, NULL);

  if (err < 0)
  {
    g_warning ("pa_context_connect() failed: %s", pa_strerror (err));
    goto error;
  }

  pa_threaded_mainloop_wait(pulse->mainloop);

  if (pa_context_get_state(pulse->context) != PA_CONTEXT_READY)
  {
    g_warning ("Failed to get ready: %s", pa_strerror(pa_context_errno(pulse->context)));
    goto error;
  }

  /* Get Sink */
  o =
  pa_context_get_sink_info_list (pulse->context,
                                 (pa_sink_info_cb_t)gst_mixer_pulse_get_sink_cb,
                                 pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  /* Get Sources */
  o =
  pa_context_get_source_info_list (pulse->context,
                                   (pa_source_info_cb_t)gst_mixer_pulse_get_source_cb,
                                   pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  /* Get Client info */
  o = pa_context_get_client_info_list (pulse->context,
                                       (pa_client_info_cb_t)gst_mixer_pulse_get_client_cb,
                                       pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  /* Get Sink input */
  o =
  pa_context_get_sink_input_info_list(pulse->context,
                                     (pa_sink_input_info_cb_t)gst_mixer_pulse_get_sink_input_cb,
                                     pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  /* Get Source outputs */
  o =
  pa_context_get_source_output_info_list(pulse->context,
                                         (pa_source_output_info_cb_t)gst_mixer_pulse_get_source_output_cb,
                                         pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  *mixer_ret = (GstMixer*)pulse;
  pa_threaded_mainloop_unlock (pulse->mainloop);
  return 0;

error:
  pa_threaded_mainloop_unlock (pulse->mainloop);
  g_object_unref (pulse);
  return err;
}


GList *gst_mixer_pulse_probe (GList *card_list)
{
  GstMixer *mixer = NULL;
  int ret;

  ret = gst_mixer_pulse_new(&mixer);

  if (ret < 0)
    return NULL;

  card_list = g_list_append (card_list, mixer);
  return card_list;
}


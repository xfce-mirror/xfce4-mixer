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
#include "config.h"
#endif

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
};

G_DEFINE_TYPE (GstMixerPulse, gst_mixer_pulse, GST_TYPE_MIXER)

static void
gst_mixer_pulse_finalize (GObject *self)
{
  GstMixerPulse *mixer = GST_MIXER_PULSE (self);

  pa_threaded_mainloop_free (mixer->mainloop);

  G_OBJECT_CLASS (gst_mixer_pulse_parent_class)->finalize (self);
}


static void
gst_mixer_pulse_init (GstMixerPulse *mixer)
{
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


static void gst_mixer_pulse_set_sink_volume (pa_context *context,
                                             const pa_sink_info *info,
                                             int eol,
                                             void *userdata)
{
  gint *volumes;
  pa_operation *o;
  pa_cvolume vol;
  int i;


  if (info == NULL) return;


  volumes = (gint*)userdata;

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) volumes[i];
  }

  o = pa_context_set_sink_volume_by_index(context,
                                          info->index,
                                          &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
    pa_operation_unref (o);

}


static void gst_mixer_pulse_set_source_volume (pa_context *context,
                                               const pa_source_info *info,
                                               int eol,
                                               void *userdata)
{
  gint *volumes;
  pa_operation *o;
  pa_cvolume vol;
  int i;

  if (info == NULL) return;

  volumes = (gint*)userdata;

  vol.channels = info->channel_map.channels;

  for (i = 0; i < info->channel_map.channels; i++)
  {
    vol.values[i] = (pa_volume_t) volumes[i];
  }


  o = pa_context_set_source_volume_by_index(context,
                                            info->index,
                                            &vol, NULL, NULL);

  if (!o)
    g_warning ("Failed to set volume on track '%s' => '%s'",
               info->name,
               pa_strerror(pa_context_errno(context)));
  else
    pa_operation_unref (o);

}


static void gst_mixer_pulse_set_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  GstMixerPulse *pulse = GST_MIXER_PULSE (mixer);
  int i;

  for (i = 0; i < NUM_CHANNELS(track); i++)
    track->volumes[i] = volumes[i];

  if (IS_OUTPUT(track))
  {
    pa_context_get_sink_info_by_name (pulse->context,
                                      track->untranslated_label,
                                      gst_mixer_pulse_set_sink_volume,
                                      track->volumes);
  }
  else
  {
    pa_context_get_source_info_by_name (pulse->context,
                                        track->untranslated_label,
                                        gst_mixer_pulse_set_source_volume,
                                        track->volumes);
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


static const gchar *
gst_mixer_pulse_get_option (GstMixer *mixer, GstMixerOptions *opts)
{
  return NULL;
}


static void
gst_mixer_pulse_set_option (GstMixer *mixer, GstMixerOptions *opts, gchar *value)
{
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

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_pulse_finalize;
}


static void
gst_mixer_pulse_state_cb (pa_context    *context,
                          GstMixerPulse *pulse)
{
  switch (pa_context_get_state(context))
  {
    case PA_CONTEXT_READY:
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
  int i;

  if (info == NULL || eol > 0)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", info->description,
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

  gst_mixer_add_track (GST_MIXER(pulse), GST_MIXER_TRACK(track));

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
  int i;

  if (info == NULL || eol > 0 || info->monitor_of_sink != PA_INVALID_INDEX)
  {
    pa_threaded_mainloop_signal(pulse->mainloop, 0);
    return;
  }
  printf("Port %s\n", info->ports[0]->name);

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK,
                        "label", info->description,
                        "untranslated-label", info->name,
                        "index", info->index,
                        "flags", GST_MIXER_TRACK_INPUT,
                        "num-channels", info->channel_map.channels,
                        "has-volume", TRUE,
                        "has-switch", TRUE,
                        "min-volume", PA_VOLUME_MUTED,
                        "max-volume", PA_VOLUME_NORM,
                        NULL);

  gst_mixer_add_track (GST_MIXER(pulse), GST_MIXER_TRACK(track));

  GST_MIXER_TRACK(track)->volumes = g_new (gint, info->channel_map.channels);

  for (i = 0; i < info->channel_map.channels; i++)
  {
    GST_MIXER_TRACK(track)->volumes[i] = info->volume.values[i];
  }

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

  o =
  pa_context_get_sink_info_list (pulse->context,
                                 (pa_sink_info_cb_t)gst_mixer_pulse_get_sink_cb,
                                 pulse);
  while (pa_operation_get_state(o) != PA_OPERATION_DONE)
    pa_threaded_mainloop_wait(pulse->mainloop);
  pa_operation_unref(o);

  o =
  pa_context_get_source_info_list (pulse->context,
                                   (pa_source_info_cb_t)gst_mixer_pulse_get_source_cb,
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


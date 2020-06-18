/*
 * Copyright (C) 2020  Takashi Iwai <tiwai@suse.de>
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

#include <alsa/asoundlib.h>

#include "alsa-track.h"
#include "gst-mixer.h"


G_DEFINE_TYPE (GstMixerAlsaTrack, gst_mixer_alsa_track, GST_TYPE_MIXER_TRACK)

static void
gst_mixer_alsa_track_finalize (GObject *self)
{
  /*GstMixerAlsaTrack *track = GST_MIXER_ALSA_TRACK (self);*/

  G_OBJECT_CLASS (gst_mixer_alsa_track_parent_class)->finalize (self);
}


static void
gst_mixer_alsa_track_init (GstMixerAlsaTrack *track)
{
  track->element = NULL;
}


static void
gst_mixer_alsa_track_class_init (GstMixerAlsaTrackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_alsa_track_finalize;
}


static gboolean same_volumes (gint num_channels, const gint *volumes)
{
  gint i;

  for (i = 1; i < num_channels; i++) {
    if (volumes[0] != volumes[i])
      return FALSE;
  }

  return TRUE;
}


static void gst_mixer_alsa_track_update (GstMixerAlsaTrack *track)
{
  gboolean vol_changed = FALSE;
  int i;

  if (IS_OUTPUT(track))
  {
    int audible = 0;
    if (HAS_SWITCH(track))
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
      {
        int v = 0;
        snd_mixer_selem_get_playback_switch (track->element, i, &v);
        if (v)
          audible = 1;
      }
    }

    if (HAS_VOLUME(track))
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
      {
        long vol = 0;
        snd_mixer_selem_get_playback_volume (track->element, i, &vol);
        if (GST_MIXER_TRACK(track)->volumes[i] != vol)
          vol_changed = TRUE;
        GST_MIXER_TRACK(track)->volumes[i] = vol;
        if (!HAS_SWITCH(track) &&
            vol > GST_MIXER_TRACK(track)->min_volume)
          audible = 1;
      }
    }

    gst_mixer_track_update_mute (GST_MIXER_TRACK(track), !audible);
  }

  if (IS_INPUT(track))
  {
    int recording = 0;
    if (HAS_SWITCH(track))
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
      {
        int v = 0;
        snd_mixer_selem_get_capture_switch (track->element, i, &v);
        if (v)
          recording = 1;
      }
    }

    if (HAS_VOLUME(track))
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
      {
        long vol = 0;
        snd_mixer_selem_get_capture_volume (track->element, i, &vol);
        if (GST_MIXER_TRACK(track)->volumes[i] != vol)
          vol_changed = TRUE;
        GST_MIXER_TRACK(track)->volumes[i] = vol;
        if (!HAS_SWITCH(track) &&
            vol > GST_MIXER_TRACK(track)->min_volume)
          recording = 1;
      }
    }

    gst_mixer_track_update_recording (GST_MIXER_TRACK(track), recording);
  }

  if (vol_changed)
    g_signal_emit_by_name (track, "volume-changed", 0);
}


GstMixerAlsaTrack *gst_mixer_alsa_track_new (snd_mixer_elem_t *element,
                                             guint index,
                                             GstMixerTrackFlags flags,
                                             gboolean has_volume,
                                             gboolean has_switch,
                                             gboolean append_capture)
{
  GstMixerAlsaTrack *track;
  long min = 0, max = 0;
  gint num_channels = 0;
  const char *name;
  char *label;

  if (flags & GST_MIXER_TRACK_OUTPUT)
  {
    if (has_volume)
      snd_mixer_selem_get_playback_volume_range (element, &min, &max);
    while (snd_mixer_selem_has_playback_channel (element,
                                                 num_channels))
      num_channels++;
  }
  else if (flags & GST_MIXER_TRACK_INPUT)
  {
    if (has_volume)
      snd_mixer_selem_get_capture_volume_range (element, &min, &max);
    while (snd_mixer_selem_has_capture_channel (element,
                                                num_channels))
      num_channels++;
  }

  name = snd_mixer_selem_get_name (element);

  if (!index)
    label = g_strdup_printf ("%s%s", name,
                             append_capture ? " Capture" : "");
  else
    label = g_strdup_printf ("%s%s %d", name,
                             append_capture ? " Capture" : "",
                             index);
  printf("name %s has_volume %d has_switch %d nch %d min %d max %d\n",
         label, has_volume, has_switch, num_channels,min, max);

  track = g_object_new (GST_MIXER_TYPE_ALSA_TRACK,
                        "label", label,
                        "untranslated-label", name,
                        "index", index,
                        "flags", flags,
                        "has-volume", has_volume,
                        "has-switch", has_switch,
                        "num-channels", num_channels,
                        "min-volume", min,
                        "max-volume", max,
                        NULL);

  GST_MIXER_TRACK(track)->volumes = g_new (gint, NUM_CHANNELS(track));
  track->element = element;

  g_free (label);

  return track;
}


void gst_mixer_alsa_track_set_volumes (GstMixerAlsaTrack *track,
                                       gint *volumes)
{
  int i;

  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (track));

  gst_mixer_alsa_track_update (track);

  if (!HAS_VOLUME(track))
    return;

  for (i = 0; i < NUM_CHANNELS(track); i++)
    GST_MIXER_TRACK(track)->volumes[i] = volumes[i];

  if (IS_OUTPUT(track))
  {
    if (HAS_SWITCH(track) && IS_MUTE(track))
      return;

    if (same_volumes (NUM_CHANNELS(track), volumes))
    {
      snd_mixer_selem_set_playback_volume_all (track->element,
                                               volumes[0]);
    }
    else
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
        snd_mixer_selem_set_playback_volume (track->element,
                                             i,
                                             volumes[i]);
    }
  }
  else
  {
    if (HAS_SWITCH(track) && !IS_RECORD(track))
      return;
    if (same_volumes (NUM_CHANNELS(track), volumes))
    {
      snd_mixer_selem_set_capture_volume_all (track->element,
                                              volumes[0]);
    }
    else
    {
      for (i = 0; i < NUM_CHANNELS(track); i++)
        snd_mixer_selem_set_capture_volume (track->element,
                                            i,
                                            volumes[i]);
    }
  }
}


gint *gst_mixer_alsa_track_get_volumes (GstMixerAlsaTrack *track)
{
  g_return_val_if_fail (GST_MIXER_IS_ALSA_TRACK (track), NULL);

  gst_mixer_alsa_track_update (track);

  return GST_MIXER_TRACK(track)->volumes;
}


void gst_mixer_alsa_track_set_master (GstMixerAlsaTrack *track)
{
  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (track));

  GST_MIXER_TRACK(track)->flags |= GST_MIXER_TRACK_MASTER;
}


void gst_mixer_alsa_track_set_element (GstMixerAlsaTrack *track,
                                        snd_mixer_elem_t   *element)
{
  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (track));
  track->element = element;
}


snd_mixer_elem_t *gst_mixer_alsa_track_get_element (GstMixerAlsaTrack *track)
{
  g_return_val_if_fail (GST_MIXER_IS_ALSA_TRACK (track), NULL);
  return track->element;
}


void gst_mixer_alsa_track_element_cb (GstMixerAlsaTrack *track,
                                       snd_mixer_elem_t   *elem)
{
  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (track));

  if (track->element == elem)
    gst_mixer_alsa_track_update (track);
}


void gst_mixer_alsa_track_set_shared_mute(GstMixerAlsaTrack *track,
                                          GstMixerAlsaTrack *shared)
{
  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (track));
  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (shared));

  track->shared_mute = shared;
}

void gst_mixer_alsa_track_set_record (GstMixerAlsaTrack *alsa_track,
                                      gboolean record)
{
  GstMixerTrack *track;
  int i;

  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (alsa_track));

  track = GST_MIXER_TRACK(alsa_track);

  if (! (track->flags & GST_MIXER_TRACK_INPUT))
    return;

  gst_mixer_alsa_track_update (alsa_track);

  record = !!record;
  if (record == !! (track->flags & GST_MIXER_TRACK_RECORD))
    return;

  if (record)
    track->flags |= GST_MIXER_TRACK_RECORD;
  else
    track->flags &= ~GST_MIXER_TRACK_RECORD;

  if (track->has_switch)
  {
    snd_mixer_selem_set_capture_switch_all (alsa_track->element, record);
  }
  else
  {
    for (i = 0; i < track->num_channels; i++)
    {
      long vol = record ? track->volumes[i] : track->min_volume;
      snd_mixer_selem_set_capture_volume (alsa_track->element, i, vol);
    }
  }
}


void gst_mixer_alsa_track_set_mute (GstMixerAlsaTrack *alsa_track,
                                    gboolean mute)
{
  GstMixerTrack *track;
  int i;

  g_return_if_fail (GST_MIXER_IS_ALSA_TRACK (alsa_track));

  track = GST_MIXER_TRACK(alsa_track);

  if (track->flags & GST_MIXER_TRACK_INPUT)
  {
    if (track->shared_mute)
      track = track->shared_mute;
    else
      return;
  }

  gst_mixer_alsa_track_update (alsa_track);

  mute = !!mute;
  if (mute == !! (track->flags & GST_MIXER_TRACK_MUTE))
    return;

  gst_mixer_track_update_mute (track, mute);

  if (track->has_switch)
  {
    snd_mixer_selem_set_playback_switch_all (alsa_track->element, !mute);
  }
  else
  {
    for (i = 0; i < track->num_channels; i++)
    {
      long vol = mute ? track->min_volume : track->volumes[i];
      snd_mixer_selem_set_playback_volume (alsa_track->element, i, vol);
    }
  }
}


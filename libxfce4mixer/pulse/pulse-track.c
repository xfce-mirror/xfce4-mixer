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


#include "pulse-track.h"
#include "gst-mixer.h"


G_DEFINE_TYPE (GstMixerPulseTrack, gst_mixer_pulse_track, GST_TYPE_MIXER_TRACK)

static void
gst_mixer_pulse_track_finalize (GObject *self)
{
  /*GstMixerPulseTrack *track = GST_MIXER_PULSE_TRACK (self);*/

  G_OBJECT_CLASS (gst_mixer_pulse_track_parent_class)->finalize (self);
}


static void
gst_mixer_pulse_track_init (GstMixerPulseTrack *track)
{
}


static void
gst_mixer_pulse_track_class_init (GstMixerPulseTrackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_pulse_track_finalize;
}


GstMixerPulseTrack *gst_mixer_pulse_track_new (gpointer info, GstMixerTrackFlags flags)
{
  GstMixerPulseTrack *track = NULL;

  track = g_object_new (GST_MIXER_TYPE_PULSE_TRACK, NULL);

  return track;
}



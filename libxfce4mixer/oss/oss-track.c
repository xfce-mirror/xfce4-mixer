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

#include "oss-track.h"
#include "gst-mixer.h"

G_DEFINE_TYPE (GstMixerOssTrack, gst_mixer_oss_track, GST_TYPE_MIXER_TRACK)

static void
gst_mixer_oss_track_finalize (GObject *self)
{
  /*GstMixerOssTrack *track = GST_MIXER_OSS_TRACK (self);*/

  G_OBJECT_CLASS (gst_mixer_oss_track_parent_class)->finalize (self);
}


static void
gst_mixer_oss_track_init (GstMixerOssTrack *track)
{
}


static void
gst_mixer_oss_track_class_init (GstMixerOssTrackClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_oss_track_finalize;
}


GstMixerOssTrack *gst_mixer_oss_track_new (void)
{
  GstMixerOssTrack *track = NULL;

  track = g_object_new (GST_MIXER_TYPE_OSS_TRACK, NULL);

  return track;
}


void gst_mixer_oss_track_set_volumes (GstMixerOssTrack *track,
                                      gint *volumes)
{
}


gint *gst_mixer_oss_track_get_volumes (GstMixerOssTrack *track)
{
  g_return_val_if_fail (GST_MIXER_IS_OSS_TRACK (track), NULL);

  return GST_MIXER_TRACK(track)->volumes;
}


void gst_mixer_oss_track_set_master (GstMixerOssTrack *track)
{
  g_return_if_fail (GST_MIXER_IS_OSS_TRACK (track));

  GST_MIXER_TRACK(track)->flags |= GST_MIXER_TRACK_MASTER;
}


void gst_mixer_oss_track_set_shared_mute (GstMixerOssTrack *track,
                                          GstMixerOssTrack *shared)
{
  g_return_if_fail (GST_MIXER_IS_OSS_TRACK (track));
  g_return_if_fail (GST_MIXER_IS_OSS_TRACK (shared));

  track->shared_mute = shared;
}

void gst_mixer_oss_track_set_record (GstMixerOssTrack *oss_track,
                                     gboolean record)
{
}


void gst_mixer_oss_track_set_mute (GstMixerOssTrack *oss_track,
                                   gboolean mute)
{
}


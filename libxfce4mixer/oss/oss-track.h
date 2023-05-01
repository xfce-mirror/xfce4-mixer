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

#ifndef GST_MIXER_OSS_TRACK_H
#define GST_MIXER_OSS_TRACK_H

#include <gst/gst.h>

#include "gst-mixer-track.h"

typedef struct _GstMixerOssTrack GstMixerOssTrack;
typedef struct _GstMixerOssTrackClass GstMixerOssTrackClass;

#define GST_MIXER_TYPE_OSS_TRACK            gst_mixer_oss_track_get_type ()
#define GST_MIXER_OSS_TRACK(o)             (G_TYPE_CHECK_INSTANCE_CAST ((o), GST_MIXER_TYPE_OSS_TRACK, GstMixerOssTrack))
#define GST_MIXER_IS_OSS_TRACK(o)          (G_TYPE_CHECK_INSTANCE_TYPE ((o), GST_MIXER_TYPE_OSS_TRACK))
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GstMixerOssTrack, g_object_unref)

struct _GstMixerOssTrack
{
  GstMixerTrack parent;

  GstMixerOssTrack *shared_mute;
  gint id;

};

struct _GstMixerOssTrackClass
{
  GstMixerTrackClass parent;
};

GType               gst_mixer_oss_track_get_type          (void);
GstMixerOssTrack   *gst_mixer_oss_track_new               ();
void                gst_mixer_oss_track_set_volumes       (GstMixerOssTrack *track,
                                                           gint *volumes);
gint               *gst_mixer_oss_track_get_volumes       (GstMixerOssTrack *track);
void                gst_mixer_oss_track_set_master        (GstMixerOssTrack *track);
void                gst_mixer_oss_track_set_shared_mute   (GstMixerOssTrack *track,
                                                           GstMixerOssTrack *share);
void                gst_mixer_oss_track_set_record        (GstMixerOssTrack *track,
                                                           gboolean record);
void                gst_mixer_oss_track_set_mute          (GstMixerOssTrack *track,
                                                           gboolean mute);
#endif /* GST_MIXER_OSS_TRACK_H */

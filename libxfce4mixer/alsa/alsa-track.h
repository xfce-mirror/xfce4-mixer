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

#ifndef GST_MIXER_ALSA_TRACK_H
#define GST_MIXER_ALSA_TRACK_H

#include <gst/gst.h>

#include "gst-mixer-track.h"

typedef struct _GstMixerAlsaTrack GstMixerAlsaTrack;
typedef struct _GstMixerAlsaTrackClass GstMixerAlsaTrackClass;

#define GST_MIXER_TYPE_ALSA_TRACK            gst_mixer_alsa_track_get_type ()
#define GST_MIXER_ALSA_TRACK(o)             (G_TYPE_CHECK_INSTANCE_CAST ((o), GST_MIXER_TYPE_ALSA_TRACK, GstMixerAlsaTrack))
#define GST_MIXER_IS_ALSA_TRACK(o)          (G_TYPE_CHECK_INSTANCE_TYPE ((o), GST_MIXER_TYPE_ALSA_TRACK))
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GstMixerAlsaTrack, g_object_unref)

struct _GstMixerAlsaTrack
{
  GstMixerTrack parent;

  GstMixerAlsaTrack *shared_mute;

  snd_mixer_elem_t *element;
};

struct _GstMixerAlsaTrackClass
{
  GstMixerTrackClass parent;
};

GType               gst_mixer_alsa_track_get_type          (void);
GstMixerAlsaTrack  *gst_mixer_alsa_track_new               (snd_mixer_elem_t *element,
                                                            guint index,
                                                            GstMixerTrackFlags flags,
                                                            gboolean has_volume,
                                                            gboolean has_switch,
                                                            gboolean append_capture);
void                gst_mixer_alsa_track_set_volumes       (GstMixerAlsaTrack *track,
                                                            gint *volumes);
gint               *gst_mixer_alsa_track_get_volumes       (GstMixerAlsaTrack *track);
void                gst_mixer_alsa_track_set_master        (GstMixerAlsaTrack *track);
void                gst_mixer_alsa_track_set_element       (GstMixerAlsaTrack *track,
                                                            snd_mixer_elem_t   *element);
snd_mixer_elem_t   *gst_mixer_alsa_track_get_element       (GstMixerAlsaTrack *track);
void                gst_mixer_alsa_track_element_cb        (GstMixerAlsaTrack *track,
                                                            snd_mixer_elem_t   *elem);
void                gst_mixer_alsa_track_set_shared_mute   (GstMixerAlsaTrack *track,
                                                            GstMixerAlsaTrack *share);
void                gst_mixer_alsa_track_set_record        (GstMixerAlsaTrack *track,
                                                            gboolean record);
void                gst_mixer_alsa_track_set_mute          (GstMixerAlsaTrack *track,
                                                            gboolean mute);
#endif /* GST_MIXER_ALSA_TRACK_H */

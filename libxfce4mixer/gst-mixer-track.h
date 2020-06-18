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

#ifndef GST_MIXER_TRACK_H__
#define GST_MIXER_TRACK_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_MIXER_TRACK                (gst_mixer_track_get_type ())
#define GST_MIXER_TRACK(o)                  (G_TYPE_CHECK_INSTANCE_CAST ((o), GST_TYPE_MIXER_TRACK, GstMixerTrack))
#define GST_MIXER_TRACK_CLASS(k)            (G_TYPE_CHECK_CLASS_CAST((k), GST_TYPE_MIXER_TRACK, GstMixerTrackClass))
#define GST_IS_MIXER_TRACK(o)               (G_TYPE_CHECK_INSTANCE_TYPE ((o), GST_TYPE_MIXER_TRACK))
#define GST_IS_MIXER_TRACK_CLASS(k)         (G_TYPE_CHECK_CLASS_TYPE ((k), GST_TYPE_MIXER_TRACK))
#define GST_MIXER_TRACK_GET_CLASS(o)        (G_TYPE_INSTANCE_GET_CLASS ((o), GST_TYPE_MIXER_TRACK, GstMixerTrackClass))

typedef enum {
  GST_MIXER_TRACK_INVALID = (1<<0),
  GST_MIXER_TRACK_INPUT  = (1<<1),
  GST_MIXER_TRACK_OUTPUT = (1<<2),
  GST_MIXER_TRACK_MUTE   = (1<<3),
  GST_MIXER_TRACK_RECORD = (1<<4),
  GST_MIXER_TRACK_MASTER = (1<<5),
  GST_MIXER_TRACK_SOFTWARE = (1<<6),
  GST_MIXER_TRACK_NO_RECORD = (1<<7),
  GST_MIXER_TRACK_NO_MUTE = (1<<8),
  GST_MIXER_TRACK_WHITELIST = (1<<9),
  GST_MIXER_TRACK_READONLY = (1<<10),
  GST_MIXER_TRACK_WRITEONLY = (1<<11)
} GstMixerTrackFlags;

#define GST_MIXER_TRACK_HAS_FLAG(track, flag) \
  (gst_mixer_track_get_flags(GST_MIXER_TRACK(track)) & (flag))
  /*((track)->flags & (flag))*/

#define IS_OUTPUT(track) \
  (gst_mixer_track_get_flags(GST_MIXER_TRACK(track)) & GST_MIXER_TRACK_OUTPUT)

#define IS_INPUT(track) \
  (gst_mixer_track_get_flags(GST_MIXER_TRACK(track)) & GST_MIXER_TRACK_INPUT)

#define IS_RECORD(track) \
  (gst_mixer_track_get_flags(GST_MIXER_TRACK(track)) & GST_MIXER_TRACK_RECORD)

#define IS_MUTE(track) \
  (gst_mixer_track_get_flags(GST_MIXER_TRACK(track)) & GST_MIXER_TRACK_MUTE)

#define HAS_SWITCH(track) gst_mixer_track_get_has_switch(GST_MIXER_TRACK(track))
#define HAS_VOLUME(track) gst_mixer_track_get_has_volume(GST_MIXER_TRACK(track))
#define NUM_CHANNELS(track) gst_mixer_track_get_num_channels(GST_MIXER_TRACK(track))

typedef struct _GstMixerTrack GstMixerTrack;
typedef struct _GstMixerTrackClass GstMixerTrackClass;

struct _GstMixerTrack
{
  GObject parent;

  GstMixerTrackFlags flags;
  gchar *label;
  gchar *untranslated_label;
  guint index;
  gint num_channels;
  gint *volumes;
  gint min_volume;
  gint max_volume;
  GstMixerTrack *shared_mute;
  gboolean has_volume:1;
  gboolean has_switch:1;

};

struct _GstMixerTrackClass
{
  GObjectClass parent;

  void              (*volume_changed)    (GstMixerTrack *track);
  void              (*mute_changed)      (GstMixerTrack *track,
                                          gboolean        muted);
  void              (*recording_changed) (GstMixerTrack *track,
                                          gboolean        recording);
};


GType                gst_mixer_track_get_type            (void);
const gchar         *gst_mixer_track_get_name            (GstMixerTrack *mixer_track);

GstMixerTrackFlags   gst_mixer_track_get_flags           (GstMixerTrack *mixer_track);

gint                 gst_mixer_track_get_num_channels    (GstMixerTrack *track);

gboolean             gst_mixer_track_get_has_volume      (GstMixerTrack *track);
gboolean             gst_mixer_track_get_has_switch      (GstMixerTrack *track);
gint                 gst_mixer_track_get_min_volume      (GstMixerTrack *track);
gint                 gst_mixer_track_get_max_volume      (GstMixerTrack *track);

/* Private methods, called by subclass implementations */
void                 gst_mixer_track_update_recording    (GstMixerTrack *track,
                                                          gboolean recording);
void                 gst_mixer_track_update_mute         (GstMixerTrack *track,
                                                          gboolean mute);


G_END_DECLS

#endif /* GST_MIXER_TRACK_H__ */

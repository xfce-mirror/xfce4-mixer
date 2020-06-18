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

#ifndef GST_MIXER_H__
#define GST_MIXER_H__

#include <gst/gst.h>

#include "gst-mixer-options.h"
#include "gst-mixer-track.h"

G_BEGIN_DECLS

#define GST_TYPE_MIXER (gst_mixer_get_type ())

G_DECLARE_DERIVABLE_TYPE (GstMixer, gst_mixer, GST, MIXER, GstElement)

#define GST_MIXER_MESSAGE_NAME "gst-mixer-message"

typedef enum {
  GST_MIXER_FLAG_NONE                = 0,
  GST_MIXER_FLAG_AUTO_NOTIFICATIONS  = (1<<0),
  GST_MIXER_FLAG_HAS_WHITELIST       = (1<<1),
  GST_MIXER_FLAG_GROUPING            = (1<<2),
} GstMixerFlags;


typedef enum {
  GST_MIXER_MESSAGE_INVALID,
  GST_MIXER_MESSAGE_MUTE_TOGGLED,
  GST_MIXER_MESSAGE_RECORD_TOGGLED,
  GST_MIXER_MESSAGE_VOLUME_CHANGED,
  GST_MIXER_MESSAGE_OPTION_CHANGED,
  GST_MIXER_MESSAGE_OPTIONS_LIST_CHANGED,
  GST_MIXER_MESSAGE_MIXER_CHANGED
} GstMixerMessageType;


typedef struct _GstMixer GstMixer;


struct _GstMixerClass
{
  GstElementClass parent_class;

  GstMixerFlags        (*get_mixer_flags) (GstMixer *mixer);
  void                 (*get_volume)      (GstMixer *mixer,
                                           GstMixerTrack *track,
                                           gint *volumes);
  void                 (*set_volume)      (GstMixer *mixer,
                                           GstMixerTrack *track,
                                           gint *volumes);
  void                 (*set_mute)        (GstMixer *mixer,
                                           GstMixerTrack *track,
                                           gboolean mute);
  void                 (*set_record)      (GstMixer *mixer,
                                           GstMixerTrack *track,
                                           gboolean record);
  void                 (*set_option)      (GstMixer *mixer,
                                           GstMixerOptions *opts,
                                           gchar *value);
  const gchar*         (*get_option)      (GstMixer *mixer,
                                           GstMixerOptions *opts);
};


const gchar     *gst_mixer_get_card_name      (GstMixer *mixer);
GList           *gst_mixer_list_tracks        (GstMixer *mixer);

/* Overwritable methods */
GstMixerFlags    gst_mixer_get_mixer_flags    (GstMixer *mixer);
void             gst_mixer_get_volume         (GstMixer *mixer,
                                                GstMixerTrack *track,
                                                gint *volumes);
void             gst_mixer_set_volume         (GstMixer *mixer,
                                                GstMixerTrack *track,
                                                gint *volumes);
void             gst_mixer_set_mute           (GstMixer *mixer,
                                                GstMixerTrack *track,
                                                gboolean mute);
void             gst_mixer_set_record         (GstMixer *mixer,
                                                GstMixerTrack *track,
                                                gboolean record);
void             gst_mixer_set_option         (GstMixer *mixer,
                                                GstMixerOptions *opts,
                                                gchar *value);
const gchar*     gst_mixer_get_option         (GstMixer *mixer,
                                                GstMixerOptions *opts);

/* Private methods */
void gst_mixer_add_track                       (GstMixer *mixer,
                                                GstMixerTrack *track);

void gst_mixer_message_parse_mute_toggled      (GstMessage *message,
                                                GstMixerTrack **track,
                                                gboolean *mute);
void gst_mixer_message_parse_record_toggled    (GstMessage *message,
                                                GstMixerTrack **track,
                                                gboolean *record);
void gst_mixer_message_parse_volume_changed    (GstMessage *message,
                                                GstMixerTrack **track,
                                                gint **volumes,
                                                gint *num_channels);

GstMixerMessageType gst_mixer_message_get_type (GstMessage * message);
void gst_mixer_message_parse_option_changed    (GstMessage *message,
                                                GstMixerOptions ** options,
                                                const gchar **value);
void gst_mixer_message_parse_options_list_changed (GstMessage *message,
                                                   GstMixerOptions **options);

G_END_DECLS

#endif /* GST_MIXER_H__ */

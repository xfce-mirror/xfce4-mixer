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

#ifndef GST_MIXER_OPTIONS_H__
#define GST_MIXER_OPTIONS_H__

#include "gst-mixer-track.h"

G_BEGIN_DECLS

#define GST_TYPE_MIXER_OPTIONS (gst_mixer_options_get_type ())
G_DECLARE_DERIVABLE_TYPE (GstMixerOptions, gst_mixer_options, GST, MIXER_OPTIONS, GstMixerTrack)

struct _GstMixerOptionsClass
{
  GstMixerTrackClass parent;

  const gchar *        (*get_name)   (GstMixerOptions *mixer_options);
  GList *              (*get_values) (GstMixerOptions *mixer_options);
};


const gchar *gst_mixer_options_get_name   (GstMixerOptions *mixer_options);
GList *      gst_mixer_options_get_values (GstMixerOptions  *options);

G_END_DECLS

#endif /* GST_MIXER_OPTIONS_H__ */

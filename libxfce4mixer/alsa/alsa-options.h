/*
 * Copyright (C) 2020  Takashi Iwai <tiwai@suse.de>
 * Copyright (C) 2020  Ali Abdallah <ali.abdallah@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your options) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GST_MIXER_ALSA_OPTIONS_H
#define GST_MIXER_ALSA_OPTIONS_H

#include <gst/gst.h>

#include "gst-mixer-options.h"
#include "alsa-track.h"

#define GST_MIXER_TYPE_ALSA_OPTIONS (gst_mixer_alsa_options_get_type ())
G_DECLARE_FINAL_TYPE (GstMixerAlsaOptions, gst_mixer_alsa_options, GST_MIXER, ALSA_OPTIONS, GstMixerAlsaTrack)

GstMixerAlsaOptions  *gst_mixer_alsa_options_new            (snd_mixer_elem_t *element,
                                                             guint index);
GList                *gst_mixer_alsa_options_get_values     (GstMixerAlsaOptions *opts);

#endif /* GST_MIXER_ALSA_OPTIONS_H */

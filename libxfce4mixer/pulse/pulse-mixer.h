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

#ifndef GST_MIXER_PULSE_H
#define GST_MIXER_PULSE_H

#include <gst/gst.h>

#include "gst-mixer.h"

#define GST_MIXER_TYPE_PULSE  gst_mixer_pulse_get_type ()
G_DECLARE_FINAL_TYPE (GstMixerPulse, gst_mixer_pulse, GST_MIXER, PULSE, GstMixer)

GList *gst_mixer_pulse_probe (GList *card_list);

#endif /* GST_MIXER_PULSE_H */

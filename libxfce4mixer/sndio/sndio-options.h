/*
 * Copyright (C) 2020 Landry Breuil <landry@xfce.org>
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

#ifndef GST_MIXER_SNDIO_OPTIONS_H
#define GST_MIXER_SNDIO_OPTIONS_H

#include <gst/gst.h>

#include "gst-mixer-options.h"
#include "sndio-track.h"

#define GST_MIXER_TYPE_SNDIO_OPTIONS  gst_mixer_sndio_options_get_type ()
#define GST_MIXER_SNDIO_OPTIONS(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), GST_MIXER_TYPE_PULSE_OPTIONS, GstMixerSndioOptions))

typedef struct _GstMixerSndioOptions GstMixerSndioOptions;
typedef struct _GstMixerSndioOptionsClass GstMixerSndioOptionsClass;

struct _GstMixerSndioOptions
{
  GstMixerSndioTrack parent;
  GList *values;
};

struct _GstMixerSndioOptionsClass
{
  GstMixerSndioTrack parent;
};

GType                 gst_mixer_sndio_options_get_type (void);

GstMixerSndioOptions *gst_mixer_sndio_options_new      (void);

#endif /* GST_MIXER_SNDIO_OPTIONS_H */

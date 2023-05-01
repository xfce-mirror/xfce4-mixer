/*
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pulse-options.h"
#include "gst-mixer.h"

struct _GstMixerPulseOptions
{
  GstMixerPulseTrack parent;
  GList *values;
};

G_DEFINE_TYPE (GstMixerPulseOptions, gst_mixer_pulse_options, GST_MIXER_TYPE_PULSE_TRACK)

static void
gst_mixer_pulse_options_finalize (GObject *self)
{
  /*GstMixerPulseOptions *options = GST_MIXER_PULSE_OPTIONS (self);*/

  G_OBJECT_CLASS (gst_mixer_pulse_options_parent_class)->finalize (self);
}


static void
gst_mixer_pulse_options_init (GstMixerPulseOptions *mixer)
{

}


static void
gst_mixer_pulse_options_class_init (GstMixerPulseOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_pulse_options_finalize;
}


GstMixerPulseOptions *gst_mixer_pulse_options_new (void)
{
  GstMixerPulseOptions *options = NULL;
  return options;
}


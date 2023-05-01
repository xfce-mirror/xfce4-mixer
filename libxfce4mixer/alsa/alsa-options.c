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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <alsa/asoundlib.h>

#include "alsa-options.h"
#include "gst-mixer.h"

struct _GstMixerAlsaOptions
{
  GstMixerAlsaTrack parent;
  GList *values;
};

G_DEFINE_TYPE (GstMixerAlsaOptions, gst_mixer_alsa_options, GST_MIXER_TYPE_ALSA_TRACK)

static void
gst_mixer_alsa_options_finalize (GObject *self)
{
  /*GstMixerAlsaOptions *options = GST_MIXER_ALSA_OPTIONS (self);*/

  G_OBJECT_CLASS (gst_mixer_alsa_options_parent_class)->finalize (self);
}


static void
gst_mixer_alsa_options_init (GstMixerAlsaOptions *mixer)
{

}


static void
gst_mixer_alsa_options_class_init (GstMixerAlsaOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_alsa_options_finalize;
}

GstMixerAlsaOptions *gst_mixer_alsa_options_new (snd_mixer_elem_t *element,
                                                   guint index)
{
  GstMixerAlsaOptions *options;
  GstMixerAlsaTrack *alsa_track;
  GstMixerTrack *track;

  const char *label;
  int nitems;
  int i;

  label = snd_mixer_selem_get_name (element);
  options = g_object_new (GST_MIXER_TYPE_ALSA_OPTIONS,
                          "untranslated-label", label,
                          "index", snd_mixer_selem_get_index (element),
                          NULL);

  alsa_track = GST_MIXER_ALSA_TRACK (options);
  track = GST_MIXER_TRACK (options);
  gst_mixer_alsa_track_set_element (alsa_track, element);

  if (!index)
    track->label = g_strdup (label);
  else
    track->label = g_strdup_printf ("%s %d", label, index);

  nitems = snd_mixer_selem_get_enum_items (element);
  for (i = 0; i < nitems; i++)
  {
    char str[256];
    if (snd_mixer_selem_get_enum_item_name (element, i, sizeof(str), str) < 0)
      break;
    options->values = g_list_append (options->values, g_strdup (str));
  }

  return options;
}

GList *gst_mixer_alsa_options_get_values (GstMixerAlsaOptions *opts)
{
  g_return_val_if_fail (GST_MIXER_IS_ALSA_OPTIONS (opts), NULL);
  return opts->values;
}

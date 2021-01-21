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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gst-mixer-options.h"

G_DEFINE_ABSTRACT_TYPE (GstMixerOptions, gst_mixer_options, GST_TYPE_MIXER_TRACK);

static void
gst_mixer_options_init (GstMixerOptions *opt)
{

}

static void
gst_mixer_options_finalize (GObject *self)
{
  G_OBJECT_CLASS (gst_mixer_options_parent_class)->finalize (self);
}

static void
gst_mixer_options_class_init (GstMixerOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstMixerOptionsClass *opt_class = GST_MIXER_OPTIONS_CLASS (klass);

  opt_class->get_name = gst_mixer_options_get_name;
  opt_class->get_values = gst_mixer_options_get_values;

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_options_finalize;
}


const gchar *gst_mixer_options_get_name (GstMixerOptions *mixer_options)
{
  g_return_val_if_fail (GST_IS_MIXER_OPTIONS(mixer_options), NULL);

  g_warning("%s not implemented", __func__);

  return NULL;
}

GList *gst_mixer_options_get_values (GstMixerOptions *mixer_options)
{
  g_return_val_if_fail (GST_IS_MIXER_OPTIONS(mixer_options), NULL);

  g_warning("%s not implemented", __func__);

  return NULL;
}


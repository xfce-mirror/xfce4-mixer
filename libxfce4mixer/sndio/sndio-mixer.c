/*
 * Copyright (C) 2020 Landry Breuil <landry@xfce.org>
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
#include "config.h"
#endif

#include <sndio.h>

#include <libxfce4ui/libxfce4ui.h>

#include "sndio-mixer.h"
#include "sndio-track.h"
#include "sndio-options.h"

struct _GstMixerSndio
{
  GstMixer parent;

};

G_DEFINE_TYPE (GstMixerSndio, gst_mixer_sndio, GST_TYPE_MIXER)

static void
gst_mixer_sndio_finalize (GObject *self)
{
  GstMixerSndio *mixer = GST_MIXER_SNDIO (self);

  G_OBJECT_CLASS (gst_mixer_sndio_parent_class)->finalize (self);
}


static void
gst_mixer_sndio_init (GstMixerSndio *mixer)
{
}


static GstMixerFlags
gst_mixer_sndio_get_mixer_flags (GstMixer *mixer)
{
  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}

static void gst_mixer_sndio_set_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
  int i;

  for (i = 0; i < NUM_CHANNELS(track); i++)
    track->volumes[i] = volumes[i];
}


static void
gst_mixer_sndio_get_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  int i;

  for (i = 0; i < NUM_CHANNELS(track); i++)
  {
    volumes[i] = track->volumes[i];
  }
}

static void
gst_mixer_sndio_set_record (GstMixer * mixer, GstMixerTrack *track, gboolean record)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
}


static void
gst_mixer_sndio_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
}


static const gchar *
gst_mixer_sndio_get_option (GstMixer *mixer, GstMixerOptions *opts)
{
  return NULL;
}


static void
gst_mixer_sndio_set_option (GstMixer *mixer, GstMixerOptions *opts, gchar *value)
{
}


static void
gst_mixer_sndio_class_init (GstMixerSndioClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_klass = GST_ELEMENT_CLASS (klass);
  GstMixerClass *mixer_class = GST_MIXER_CLASS (klass);

  gst_element_class_set_static_metadata (element_klass,
                                         "Sndio mixer", "Generic/Audio",
                                         "Control audio mixer via sndio API",
                                         "Landry Breuil <landry@xfce.org>");

  mixer_class->get_mixer_flags = gst_mixer_sndio_get_mixer_flags;
  mixer_class->set_volume  = gst_mixer_sndio_set_volume;
  mixer_class->get_volume  = gst_mixer_sndio_get_volume;
  mixer_class->set_record  = gst_mixer_sndio_set_record;
  mixer_class->set_mute    = gst_mixer_sndio_set_mute;
  mixer_class->get_option  = gst_mixer_sndio_get_option;
  mixer_class->set_option  = gst_mixer_sndio_set_option;

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_sndio_finalize;
}

static int
gst_mixer_sndio_new (GstMixer **mixer_ret)
{
  GstMixerSndio *sndio;

  sndio = g_object_new (GST_MIXER_TYPE_SNDIO,
                        "name", "sndio",
                        "card-name", g_strdup (_("Sndio Volume Control")),
                        NULL);
}

GList *gst_mixer_sndio_probe (GList *card_list)
{
  GstMixer *mixer = NULL;
  int ret;

  ret = gst_mixer_sndio_new(&mixer);

  if (ret < 0)
    return NULL;

  card_list = g_list_append (card_list, mixer);
  return card_list;
}


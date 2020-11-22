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
#include <poll.h>
/* needed for g_unix_fd_source_new () */
#include <glib-unix.h>

#include <libxfce4ui/libxfce4ui.h>

#include "sndio-mixer.h"
#include "sndio-track.h"
#include "sndio-options.h"

/* globals */
struct _GstMixerSndio
{
  GstMixer parent;
  struct sioctl_hdl *hdl;
  struct pollfd pfd;
  GSource *src;
};

G_DEFINE_TYPE (GstMixerSndio, gst_mixer_sndio, GST_TYPE_MIXER)

/* sndio callbacks */
void
onval(void *arg, unsigned addr, unsigned val)
{
  GstMixerSndio *mixer = GST_MIXER_SNDIO (arg);
  mixer = NULL;
  g_print("onval called: addr=%d, val=%d\n", addr, val);
}

/* callback called when a description changes - e.g when a new track is added/removed */
void
ondesc(void *arg, struct sioctl_desc *d, int curval)
{
  GstMixerSndio *mixer = GST_MIXER_SNDIO (arg);
  mixer = NULL;
  if (d == NULL) {
    g_warning("got the full set of track descriptions");
    return;
  }
  g_print("ondesc: addr=%d, %s/%s.%s[%d]=%d (max=%d)\n", d->addr, d->group, d->node0.name, d->func, d->node0.unit, curval, d->maxval);
/* create a gst_track_sndio by distinct desc (desc is appname, input, output..)
group is null or app
func is level or mute

ignore server.device for now

input.level=0.486
input.mute=0
output.level=1.000
output.mute=0
server.device=0
app/mplayer0.level=1.000

-> 3 tracks, input has GST_MIXER_TRACK_INPUT flag, all others have GST_MIXER_TRACK_OUTPUT flag
input and output have a real mute control, appname doesnt (eg set volume to 0)
*/
}

static void
gst_mixer_sndio_finalize (GObject *self)
{
  GstMixerSndio *mixer = GST_MIXER_SNDIO (self);
  mixer = NULL;

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
  sndio = NULL;
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
  sndio = NULL;
}


static void
gst_mixer_sndio_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
  sndio = NULL;
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

static gboolean gst_mixer_sndio_src_callback (gint fd, GIOCondition condition, gpointer user_data)
{
  int rc, revents;
  GstMixerSndio *sndio = (GstMixerSndio *)user_data;
  g_print("gst_mixer_sndio_src_callback called with condition %d on fd %d!\n", condition, fd);
  rc = poll(&(sndio->pfd), 1, 0);
  if (rc == 0) {
    g_critical("timeout? cant happen");
    return G_SOURCE_REMOVE;
  } else if (rc == -1) {
    g_critical("poll() error: %s", g_strerror(errno));
    return G_SOURCE_REMOVE;
  } else {
    revents = sioctl_revents(sndio->hdl, &(sndio->pfd));
    g_print("sioctl_revents returned %d, pfd.revents=%d\n", revents, sndio->pfd.revents);
  }
  return G_SOURCE_CONTINUE;
}

GstMixer*
gst_mixer_sndio_new (struct sioctl_hdl *hdl)
{
  GstMixerSndio *sndio;
  char *devname = SIO_DEVANY;
  int rc;

  sndio = g_object_new (GST_MIXER_TYPE_SNDIO,
                        "name", "sndio",
                        "card-name", g_strdup (_("Sndio Volume Control")),
                        NULL);
  sndio->hdl = hdl;
  if (!sioctl_ondesc(hdl, ondesc, sndio)) {
    g_critical("%s: can't get device description", devname);
    return NULL;
  }

  sioctl_onval(sndio->hdl, onval, sndio);
  rc = sioctl_pollfd(sndio->hdl, &(sndio->pfd), POLLIN);
  if (rc != 1) {
    g_critical("[sndio] sioctl_pollfd failed: %d", rc);
    return NULL;
  }

  sndio->src = g_unix_fd_source_new (sndio->pfd.fd, G_IO_IN);
  g_source_set_callback (sndio->src, G_SOURCE_FUNC(gst_mixer_sndio_src_callback), sndio, NULL);
  g_source_attach (sndio->src, g_main_context_default ());
  return GST_MIXER(sndio);
}

GList *gst_mixer_sndio_probe (GList *card_list)
{
  GstMixer *mixer = NULL;
  struct sioctl_hdl *hdl;
  char *devname = SIO_DEVANY;
  hdl = sioctl_open(devname, SIOCTL_READ | SIOCTL_WRITE, 0);
  if (hdl == NULL) {
    g_critical ("Failed to open device '%s'", devname);
    return NULL;
  }

  mixer = gst_mixer_sndio_new(hdl);

  if (mixer == NULL)
    return NULL;

  card_list = g_list_append (card_list, mixer);
  return card_list;
}


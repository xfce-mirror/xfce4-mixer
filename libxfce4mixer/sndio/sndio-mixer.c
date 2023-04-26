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
  GHashTable *tracks;
  GHashTable *tracks_by_addr;
};

static gboolean gst_mixer_sndio_src_callback (gint, GIOCondition, gpointer);
static gboolean gst_mixer_sndio_reconnect(gpointer);

G_DEFINE_TYPE (GstMixerSndio, gst_mixer_sndio, GST_TYPE_MIXER)

/* sndio callbacks */
static void
onval(void *arg, unsigned addr, unsigned val)
{
  int i;
  GstMixerSndioTrack *track;
  GstMixerSndio *mixer = GST_MIXER_SNDIO (arg);
  g_debug("onval callback called: addr=%d, val=%d", addr, val);
  /* find the track by its addr in the tracks_by_addr hash and update the corresponding flag */
  track = g_hash_table_lookup(mixer->tracks_by_addr, GINT_TO_POINTER(addr));
  if (!track) {
    g_critical("found no track by addr %d ?", addr);
  } else {
    g_debug("for addr %d found track %s", addr, GST_MIXER_TRACK(track)->label);

    /* lookup addr in vol_addr and then mute_addr */
    for (i = 0; i < NUM_CHANNELS(track); i++) {
      if (track->vol_addr[i] == addr) {
        g_debug("%d is a level control for chan %d, updating value with %d", addr, i, val);
        GST_MIXER_TRACK(track)->volumes[i] = val;
        g_signal_emit_by_name (track, "volume-changed", 0);
      } else if (track->mute_addr[i] == addr) {
        if (IS_INPUT(track)) {
          g_debug("%d is a mute control for an input track, updating recording flag with %d", addr, val);
          gst_mixer_track_update_recording(GST_MIXER_TRACK(track), val);
        } else if (IS_OUTPUT(track)) {
          g_debug("%d is a mute control for an output track, updating mute flag with %d", addr, val);
          gst_mixer_track_update_mute(GST_MIXER_TRACK(track), val);
        }
      }
    }
  }
}

/* callback called when a description changes - e.g when a new track is added/removed */
static void
ondesc(void *arg, struct sioctl_desc *d, int curval)
{
  
  GstMixerSndioTrack *track;
  int chan, nchan;
  GstMixerTrackFlags flags = 0;
  GstMixerSndio *mixer = GST_MIXER_SNDIO (arg);
  if (d == NULL) {
    g_debug("got the full set of track descriptions");
    return;
  }
  g_debug("ondesc callback called: addr=%d, type=%d, %s/%s.%s[%d]=%d (max=%d)", d->addr, d->type, d->group, d->node0.name, d->func, d->node0.unit, curval, d->maxval);
  if (d->node0.unit == -1 ) {
    nchan = 1;
    chan = 0;
  } else {
    nchan = 2;
    chan = d->node0.unit;
  }

  /* skip server.device for now */
  if (!g_strcmp0 (d->func, "device"))
    return;

  track = g_hash_table_lookup(mixer->tracks, d->node0.name);
  if (!track) {
    track = gst_mixer_sndio_track_new ();

    if (! g_strcmp0(d->node0.name, "input"))
      flags |= GST_MIXER_TRACK_INPUT;
    else
      flags |= GST_MIXER_TRACK_OUTPUT;

    if (! g_strcmp0(d->node0.name, "output"))
      flags |= GST_MIXER_TRACK_MASTER;

    /* app tracks dont have a mute control but that just makes the mute button
     * insensitive in the UI
    if (! g_strcmp0(d->group, "app"))
      flags |= GST_MIXER_TRACK_NO_MUTE;
     */

    GST_MIXER_TRACK(track)->index = 0;
    GST_MIXER_TRACK(track)->min_volume = 0;
    GST_MIXER_TRACK(track)->max_volume = d->maxval;
    GST_MIXER_TRACK(track)->has_volume = TRUE;
    GST_MIXER_TRACK(track)->has_switch = FALSE;
    GST_MIXER_TRACK(track)->label = g_strdup(d->node0.name);
    GST_MIXER_TRACK(track)->untranslated_label = g_strdup (d->node0.name);
    GST_MIXER_TRACK(track)->flags = flags;
    GST_MIXER_TRACK(track)->num_channels = nchan;
    GST_MIXER_TRACK(track)->volumes = g_new (gint, nchan);
    track->vol_addr = g_new (guint, nchan);
    track->mute_addr = g_new (guint, nchan);
    track->saved_volumes = g_new (gint, nchan);
    g_debug("Inserting new track in hashtable for %s", d->node0.name);
    g_hash_table_insert (mixer->tracks, g_strdup(d->node0.name), track);

    gst_mixer_new_track (GST_MIXER(mixer),
      GST_MIXER_TRACK(track));
  }

  /* now we have a valid track, update volume/mute/recording status */
  if (!g_strcmp0 (d->func, "level")) {
    GST_MIXER_TRACK(track)->volumes[chan] = curval;
    track->vol_addr[chan] = d->addr;
  }
  if (!g_strcmp0 (d->func, "mute")) {
    GST_MIXER_TRACK(track)->has_switch = TRUE;
    track->mute_addr[chan] = d->addr;
    if (IS_INPUT(track)) {
      gst_mixer_track_update_recording(GST_MIXER_TRACK(track), curval);
    } else if (IS_OUTPUT(track)) {
      gst_mixer_track_update_mute(GST_MIXER_TRACK(track), curval);
    }
  }

  /* insert track in tracks_by_addr hash if its not already there */
  if (! g_hash_table_contains(mixer->tracks_by_addr, GINT_TO_POINTER(d->addr)))
    g_hash_table_insert(mixer->tracks_by_addr, GINT_TO_POINTER(d->addr), track);
}

static void
gst_mixer_sndio_finalize (GObject *self)
{
  GstMixerSndio *mixer = GST_MIXER_SNDIO (self);
  g_hash_table_unref (mixer->tracks_by_addr);
  g_hash_table_unref (mixer->tracks);
  G_OBJECT_CLASS (gst_mixer_sndio_parent_class)->finalize (self);
}


static void
gst_mixer_sndio_init (GstMixerSndio *mixer)
{
  mixer->tracks_by_addr = g_hash_table_new (g_direct_hash, g_direct_equal);
  mixer->tracks = g_hash_table_new (g_str_hash, g_str_equal);
}


static GstMixerFlags
gst_mixer_sndio_get_mixer_flags (GstMixer *mixer)
{
  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}

static void
gst_mixer_sndio_set_volume (GstMixer *mixer, GstMixerTrack *track, gint num_channels, gint *volumes)
{
  int i;
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
  if (num_channels == 2)
    g_debug("gst_mixer_sndio_set_volume called on track %s with vol[]=(%d,%d)", track->label, volumes[0], volumes[1]);
  else if (num_channels == 1)
    g_debug("gst_mixer_sndio_set_volume called on track %s with vol[0]=%d", track->label, volumes[0]);

  /* call sioctl_setval for each volume addr */
  for (i = 0; i < num_channels; i++) {
    sioctl_setval(sndio->hdl, GST_MIXER_SNDIO_TRACK(track)->vol_addr[i], volumes[i]);
    track->volumes[i] = volumes[i];
  }

  g_signal_emit_by_name (track, "volume-changed", 0);
}


static void
gst_mixer_sndio_get_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  gint num_channels = NUM_CHANNELS(track);
  int i;

  for (i = 0; i < num_channels; i++)
  {
    volumes[i] = track->volumes[i];
  }
  if (num_channels == 2)
    g_debug("gst_mixer_sndio_get_volume called on track %s filled vol[]=(%d,%d)", track->label, volumes[0], volumes[1]);
  else if (num_channels == 1)
    g_debug("gst_mixer_sndio_get_volume called on track %s filled vol[0]=%d", track->label, volumes[0]);
}

static void
gst_mixer_sndio_set_record (GstMixer * mixer, GstMixerTrack *track, gboolean record)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
  g_debug("gst_mixer_sndio_set_record called on track %s with record=%d", track->label, record);
  if (IS_INPUT(track)) {
    /* call sioctl_setval for the first mute addr */
    sioctl_setval(sndio->hdl, GST_MIXER_SNDIO_TRACK(track)->mute_addr[0], record);
    gst_mixer_track_update_recording (track, record);
  } else
    g_critical ("%s isnt an input track, cant set recording status to %d", track->label, record);
}


static void
gst_mixer_sndio_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  GstMixerSndio *sndio = GST_MIXER_SNDIO (mixer);
  gint num_channels = NUM_CHANNELS(track);
  g_debug("gst_mixer_sndio_set_mute called on track %s with mute=%d, track has switch=%d, nchan=%d", track->label, mute, HAS_SWITCH(track), num_channels);
  if (IS_OUTPUT(track)) {
    if (HAS_SWITCH(track)) {
      /* call sioctl_setval for the first mute addr */
      sioctl_setval(sndio->hdl, GST_MIXER_SNDIO_TRACK(track)->mute_addr[0], mute);
    } else {
      int i;
      gint* volumes = g_new(gint, num_channels);
      if (mute) {
        for (i = 0; i < num_channels; i++)
        {
          GST_MIXER_SNDIO_TRACK(track)->saved_volumes[i] = (track->volumes[i] > 0 ? track->volumes[i] : 1);
          volumes[i] = 0;
        }
        g_debug("saving volume (%d) and setting values to 0 on track not having a switch", GST_MIXER_SNDIO_TRACK(track)->saved_volumes[0]);
      } else {
        for (i = 0; i < num_channels; i++)
        {
          volumes[i] = GST_MIXER_SNDIO_TRACK(track)->saved_volumes[i];
        }
        g_debug("restoring volume to saved value (%d) on track not having a switch", GST_MIXER_SNDIO_TRACK(track)->saved_volumes[0]);
      }
      gst_mixer_sndio_set_volume(mixer, track, num_channels, volumes);
      g_free(volumes);
    }
    gst_mixer_track_update_mute(track, mute);
  } else
    g_critical ("%s isnt an output track, cant set mute status to %d", track->label, mute);
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

static gboolean gst_mixer_sndio_connect (GstMixerSndio *sndio)
{
  struct sioctl_hdl *hdl;
  char *devname = SIO_DEVANY;
  int rc;
  hdl = sioctl_open(devname, SIOCTL_READ | SIOCTL_WRITE, 0);
  if (hdl == NULL) {
    g_critical ("Failed to open device '%s'", devname);
    return FALSE;
  }
  sndio->hdl = hdl;
  if (!sioctl_ondesc(hdl, ondesc, sndio)) {
    g_critical("%s: can't get device description", devname);
    return FALSE;
  }

  sioctl_onval(sndio->hdl, onval, sndio);
  rc = sioctl_pollfd(sndio->hdl, &(sndio->pfd), POLLIN);
  if (rc != 1) {
    g_critical("[sndio] sioctl_pollfd failed: %d", rc);
    return FALSE;
  }

  sndio->src = g_unix_fd_source_new (sndio->pfd.fd, G_IO_IN);
  g_source_set_callback (sndio->src, G_SOURCE_FUNC(gst_mixer_sndio_src_callback), sndio, NULL);
  g_source_attach (sndio->src, g_main_context_default ());
  g_debug("[sndio] attached g_source with id %d", g_source_get_id(sndio->src));
  return TRUE;
}

gboolean gst_mixer_sndio_reconnect(gpointer user_data)
{
  GstMixerSndio *sndio = (GstMixerSndio *)user_data;

  g_debug("[sndio] tearing down old resources");
  sioctl_close(sndio->hdl);
  g_debug("[sndio] trying to reconnect to server");
  gst_mixer_sndio_connect(sndio);
  return G_SOURCE_REMOVE;
}

static gboolean gst_mixer_sndio_src_callback (gint fd, GIOCondition condition, gpointer user_data)
{
  int rc, revents;
  GstMixerSndio *sndio = (GstMixerSndio *)user_data;
  rc = poll(&(sndio->pfd), 1, 0);
  if (rc == 0) {
    g_critical("timeout? cant happen");
    return G_SOURCE_REMOVE;
  } else if (rc == -1) {
    g_critical("poll() error: %s", g_strerror(errno));
    return G_SOURCE_REMOVE;
  } else {
    revents = sioctl_revents(sndio->hdl, &(sndio->pfd));
    if (revents & POLLHUP) {
      g_warning("disconnected ? queuing reconnect in 1s");
      g_timeout_add_seconds(1, G_SOURCE_FUNC(gst_mixer_sndio_reconnect), sndio);
      return G_SOURCE_REMOVE;
    }
  }
  return G_SOURCE_CONTINUE;
}

static GstMixer*
gst_mixer_sndio_new (void)
{
  GstMixerSndio *sndio;
  gboolean rc;

  sndio = g_object_new (GST_MIXER_TYPE_SNDIO,
                        "name", "sndio",
                        "card-name", g_strdup (_("Sndio Volume Control")),
                        NULL);
  rc = gst_mixer_sndio_connect(sndio);
  if (rc == FALSE) {
    return NULL;
  }
  return GST_MIXER(sndio);
}

GList *gst_mixer_sndio_probe (GList *card_list)
{
  GstMixer *mixer = NULL;
  mixer = gst_mixer_sndio_new();

  if (mixer == NULL)
    return NULL;

  card_list = g_list_append (card_list, mixer);
  return card_list;
}

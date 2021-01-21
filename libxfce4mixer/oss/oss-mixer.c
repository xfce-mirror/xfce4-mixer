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
#include "config.h"
#endif

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include <errno.h>

#if HOST_TYPE_FREEBSD
#include <sys/sysctl.h>
#endif

#include "oss-mixer.h"
#include "oss-track.h"
#include "oss-options.h"

#define MAX_DEVS     16
#define POLL_TIME_MS 250

static const char *names[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_NAMES;
static const char *labels[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_LABELS;

static GSource *mixers_source = NULL;

struct _GstMixerOss
{
  GstMixer parent;

  int devfd;
  int card_id;
};

G_DEFINE_TYPE (GstMixerOss, gst_mixer_oss, GST_TYPE_MIXER)

static void
gst_mixer_oss_finalize (GObject *self)
{
  G_OBJECT_CLASS (gst_mixer_oss_parent_class)->finalize (self);
}


static void
gst_mixer_oss_init (GstMixerOss *mixer)
{
}


static GstMixerFlags
gst_mixer_oss_get_mixer_flags (GstMixer *mixer)
{
  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}


static void gst_mixer_oss_set_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  int vol;
  int l = volumes[0], r = volumes[1];

  if (l < 0)
    l = 0;
  else if (l > 100)
    l = 100;

  if (r < 0)
    r = 0;
  else if (r > 100)
    r = 100;

  vol = l | r << 8;

  if (ioctl(GST_MIXER_OSS(mixer)->devfd, MIXER_WRITE(GST_MIXER_OSS_TRACK(track)->id), &vol) != -1)
  {
    track->volumes[0] = l;
    track->volumes[1] = r;
    g_signal_emit_by_name (track, "volume-changed", 0);
  }
  else
  {
    g_warning ("MIXER_WRITE failed on device_id : %d", GST_MIXER_OSS_TRACK(track)->id);
  }
}


static void
gst_mixer_oss_get_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  gint *vol;
  int i;

  if (!HAS_VOLUME(track))
    return;

  vol = GST_MIXER_TRACK(track)->volumes;

  for (i = 0; i < NUM_CHANNELS(track); i++)
    volumes[i] = vol[i];
}


static void
gst_mixer_oss_set_record (GstMixer * mixer, GstMixerTrack *track, gboolean record)
{
  int vol = 0;

  /* Just set the volumes back */
  if (!record)
    vol = track->volumes[0] | track->volumes[1] << 8;

  if (ioctl(GST_MIXER_OSS(mixer)->devfd, MIXER_WRITE(GST_MIXER_OSS_TRACK(track)->id), &vol) != -1)
    gst_mixer_track_update_recording (track, record);
  else
    g_warning ("MIXER_WRITE failed on device_id : %d", GST_MIXER_OSS_TRACK(track)->id);
}


static void
gst_mixer_oss_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  int vol = 0;

  /* Just set the volumes back */
  if (!mute)
    vol = track->volumes[0] | track->volumes[1] << 8;

  if (ioctl(GST_MIXER_OSS(mixer)->devfd, MIXER_WRITE(GST_MIXER_OSS_TRACK(track)->id), &vol) != -1)
    gst_mixer_track_update_mute (track, mute);
  else
    g_warning ("MIXER_WRITE failed on device_id : %d", GST_MIXER_OSS_TRACK(track)->id);
}


static const gchar *
gst_mixer_oss_get_option (GstMixer *mixer, GstMixerOptions *opts)
{
  return NULL;
}


static void
gst_mixer_oss_set_option (GstMixer *mixer, GstMixerOptions *opts, gchar *value)
{
}


static void
gst_mixer_oss_class_init (GstMixerOssClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_klass = GST_ELEMENT_CLASS (klass);
  GstMixerClass *mixer_class = GST_MIXER_CLASS (klass);

  gst_element_class_set_static_metadata (element_klass,
                                         "OSS mixer", "Generic/Audio",
                                         "Control audio mixer via OSS API",
                                         "Ali Abdallah <ali.abdallah@suse.com>");

  mixer_class->get_mixer_flags = gst_mixer_oss_get_mixer_flags;
  mixer_class->set_volume  = gst_mixer_oss_set_volume;
  mixer_class->get_volume  = gst_mixer_oss_get_volume;
  mixer_class->set_record  = gst_mixer_oss_set_record;
  mixer_class->set_mute    = gst_mixer_oss_set_mute;
  mixer_class->get_option  = gst_mixer_oss_get_option;
  mixer_class->set_option  = gst_mixer_oss_set_option;

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_oss_finalize;
}


static void
gst_mixer_oss_poll (GstMixerOss *mixer, gpointer data)
{

}


/* FIXME, Look for newly/removed attached mixer devices */
static gboolean
gst_mixer_oss_poll_all (GList *card_list)
{
  g_list_foreach (card_list,
                  (GFunc) gst_mixer_oss_poll,
                  NULL);
  return TRUE;
}


static void gst_mixer_oss_create_track_list (GstMixerOss *mixer)
{
  int i;
  gint devmask = 0;
  gint recmask = 0;
  gint recsrc  = 0;

  if (ioctl(mixer->devfd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1)
  {
    perror ("SOUND_MIXER_READ_DEVMASK");
  }

  if (ioctl(mixer->devfd, SOUND_MIXER_READ_RECMASK, &recmask) == -1)
  {
    perror ("SOUND_MIXER_READ_RECMASK");
  }

  if (ioctl(mixer->devfd, SOUND_MIXER_READ_RECSRC, &recsrc) == -1)
  {
    perror("SOUND_MIXER_READ_RECSRC");
  }

  for (i = 0; i < SOUND_MIXER_NRDEVICES; i++)
  {
    GstMixerOssTrack *track;
    GstMixerTrackFlags flags = GST_MIXER_TRACK_NONE;
    gint vol = 0;

    if (!((1 << i) & devmask))
      continue;

    track = gst_mixer_oss_track_new ();
    track->id = i;

    GST_MIXER_TRACK(track)->min_volume = 0;
    GST_MIXER_TRACK(track)->max_volume = 100;
    GST_MIXER_TRACK(track)->has_volume = TRUE;
    GST_MIXER_TRACK(track)->has_switch = TRUE;

    if (((1 << i) & recmask))
      flags |= GST_MIXER_TRACK_INPUT;

    /* Default recording source */
    if ((1 << i) & recsrc)
      flags |= GST_MIXER_TRACK_MASTER;

    /* Master output */
    if (!g_strcmp0(names[i], "vol"))
    {
      flags |= GST_MIXER_TRACK_MASTER;
    }

    if (!g_strcmp0(names[i], "vol") || !g_strcmp0 (names[i], "pcm"))
    {
      flags |= GST_MIXER_TRACK_OUTPUT;
    }

    GST_MIXER_TRACK(track)->label = g_strdup_printf ("%s%s", labels[i], "");
    GST_MIXER_TRACK(track)->untranslated_label = g_strdup (names[i]);
    GST_MIXER_TRACK(track)->flags = flags;
    GST_MIXER_TRACK(track)->index = 0;
    GST_MIXER_TRACK(track)->num_channels = 2;
    GST_MIXER_TRACK(track)->volumes = g_new (gint, 2);

    if (ioctl(mixer->devfd, MIXER_READ(i), &vol) == -1)
    {
      g_warning ("MIXER_READ failed %s", g_strerror(errno));
    }
    else
    {
      GST_MIXER_TRACK(track)->volumes[0] = vol & 0x7f;
      GST_MIXER_TRACK(track)->volumes[1] = (vol >> 8 ) & 0x7f;
    }

    gst_mixer_new_track (GST_MIXER(mixer), GST_MIXER_TRACK(track));
  }
}


static GstMixer *
gst_mixer_oss_new (gint devfd, gint card_id)
{
  GstMixerOss *mixer;
  oss_card_info inf;

  inf.card = card_id;

  if (ioctl(devfd, SNDCTL_CARDINFO, &inf) == -1)
  {
    perror ("SNDCTL_CARDINFO");
    mixer = g_object_new (GST_MIXER_TYPE_OSS,
                          "name", g_strdup_printf ("card%i", card_id),
                          "card-name", g_strdup_printf ("OSS Mixer Card %i", card_id),
                          NULL);
  }
  else
  {
    mixer = g_object_new (GST_MIXER_TYPE_OSS,
                          "name", g_strdup (inf.shortname),
                          "card-name", g_strdup (inf.longname),
                          NULL);
  }

  mixer->devfd = devfd;
  mixer->card_id = card_id;

  gst_mixer_oss_create_track_list (mixer);

  return GST_MIXER(mixer);
}


GList *gst_mixer_oss_probe (GList *card_list)
{
  int i;
  int fd;
  oss_sysinfo inf;
  int ndev;

  /* Open the main mixer device to ask the OS how many mixers do we have */
  fd = open("/dev/mixer", O_RDWR);
  if (ioctl(fd, SNDCTL_SYSINFO, &inf) == -1)
  {
    perror ("SNDCTL_SYSINFO");
    /* Set to max, then we open mixer%i one by one */
    ndev = MAX_DEVS;
  }
  else
    ndev = inf.numcards;

  for (i = 0; i < ndev; i++)
  {
    GstMixer *mixer;
    gchar *dev;
    int devfd;
    dev = g_strdup_printf("/dev/mixer%i", i);
    if (g_file_test (dev, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
    {
      devfd = open (dev, O_RDWR);
      if (devfd < 0)
      {
        g_critical ("Failed to open device '%s' : '%s'", dev, g_strerror(errno));
        g_free (dev);
      }
      else
      {
        g_debug ("New mixer device '%s'", dev);
        mixer = gst_mixer_oss_new(devfd, i);
        card_list = g_list_append (card_list, mixer);
      }
    }
    else
      break;
  }

  mixers_source = g_timeout_source_new (POLL_TIME_MS);
  g_source_set_callback (mixers_source,
                         (GSourceFunc) gst_mixer_oss_poll_all,
                         card_list,
                         NULL);
  g_source_attach (mixers_source, g_main_context_get_thread_default());
  return card_list;
}


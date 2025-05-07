/*
 * Copyright (C) 2020  Takashi Iwai <tiwai@suse.de>
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

#include <alsa/asoundlib.h>

#include "alsa-mixer.h"
#include "alsa-track.h"
#include "alsa-options.h"

struct _GstMixerAlsa
{
  GstMixer parent;

  void *handle; /* snd_mixer_t */
  GSource *src;

};

G_DEFINE_TYPE (GstMixerAlsa, gst_mixer_alsa, GST_TYPE_MIXER)

static void
gst_mixer_alsa_finalize (GObject *self)
{
  GstMixerAlsa *mixer = GST_MIXER_ALSA (self);

  if (mixer->src)
  {
    g_source_destroy (mixer->src);
  }

  if (mixer->handle)
  {
    snd_mixer_close (mixer->handle);
  }

  G_OBJECT_CLASS (gst_mixer_alsa_parent_class)->finalize (self);
}


static void
gst_mixer_alsa_init (GstMixerAlsa *mixer)
{
  mixer->src = NULL;
}


static GstMixerFlags
gst_mixer_alsa_get_mixer_flags (GstMixer *mixer)
{
  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}


static void
gst_mixer_alsa_set_volume (GstMixer *mixer, GstMixerTrack *track, gint num_channels, gint *volumes)
{
  gst_mixer_alsa_track_set_volumes (GST_MIXER_ALSA_TRACK(track), volumes);
}


static void
gst_mixer_alsa_get_volume (GstMixer *mixer, GstMixerTrack *track, gint *volumes)
{
  gint *vol;
  int i;

  if (!HAS_VOLUME(track))
    return;

  vol = gst_mixer_alsa_track_get_volumes (GST_MIXER_ALSA_TRACK(track));

  for (i = 0; i < NUM_CHANNELS(track); i++)
    volumes[i] = vol[i];
}


static void
gst_mixer_alsa_set_record (GstMixer * mixer, GstMixerTrack *track, gboolean record)
{
  gst_mixer_alsa_track_set_record (GST_MIXER_ALSA_TRACK(track), record);
}


static void
gst_mixer_alsa_set_mute (GstMixer *mixer, GstMixerTrack *track, gboolean mute)
{
  gst_mixer_alsa_track_set_mute (GST_MIXER_ALSA_TRACK(track), mute);
}


static const gchar *
gst_mixer_alsa_get_option (GstMixer *mixer, GstMixerOptions *opts)
{
  GstMixerAlsaOptions *alsa_opts;
  GstMixerAlsaTrack *track;
  unsigned int idx;

  alsa_opts = GST_MIXER_ALSA_OPTIONS(opts);
  track = GST_MIXER_ALSA_TRACK (opts);
  if (snd_mixer_selem_get_enum_item (track->element, 0, &idx) < 0)
    return "error";
  return g_list_nth_data (gst_mixer_alsa_options_get_values (alsa_opts), idx);
}


static void
gst_mixer_alsa_set_option (GstMixer *mixer, GstMixerOptions *opts, gchar *value)
{
  int n = 0;
  GstMixerAlsaOptions *alsa_opts;
  GstMixerAlsaTrack *track;
  GList *item;

  alsa_opts = GST_MIXER_ALSA_OPTIONS(opts);
  track = GST_MIXER_ALSA_TRACK (opts);

  for (item = gst_mixer_alsa_options_get_values (alsa_opts); item; item = item->next, n++) {
    if (!strcmp (item->data, value)) {
      snd_mixer_selem_set_enum_item (track->element, 0, n);
      break;
    }
  }
}


static void
gst_mixer_alsa_class_init (GstMixerAlsaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_klass = GST_ELEMENT_CLASS (klass);
  GstMixerClass *mixer_class = GST_MIXER_CLASS (klass);

  gst_element_class_set_static_metadata (element_klass,
                                         "ALSA mixer", "Generic/Audio",
                                         "Control audio mixer via ALSA API",
                                         "Takashi Iwai <tiwai@suse.de>");

  mixer_class->get_mixer_flags = gst_mixer_alsa_get_mixer_flags;
  mixer_class->set_volume  = gst_mixer_alsa_set_volume;
  mixer_class->get_volume  = gst_mixer_alsa_get_volume;
  mixer_class->set_record  = gst_mixer_alsa_set_record;
  mixer_class->set_mute    = gst_mixer_alsa_set_mute;
  mixer_class->get_option  = gst_mixer_alsa_get_option;
  mixer_class->set_option  = gst_mixer_alsa_set_option;

  object_class->finalize = (void (*) (GObject *object)) gst_mixer_alsa_finalize;
}


static int gst_mixer_alsa_callback (snd_mixer_t *ctl,
                                     unsigned int mask,
                                     snd_mixer_elem_t *elem)
{
  GstMixerAlsa *mixer = snd_mixer_get_callback_private (ctl);

  snd_mixer_handle_events (mixer->handle);
  return 0;
}


static int mixer_elem_callback (snd_mixer_elem_t *elem, unsigned int mask)
{
  GstMixerAlsa *mixer = snd_mixer_elem_get_callback_private (elem);
  GList *tracklist = gst_mixer_list_tracks (GST_MIXER(mixer));
  GList *item;

  for (item = tracklist; item; item = item->next) {
    GstMixerAlsaTrack *track = GST_MIXER_ALSA_TRACK (item->data);
    gst_mixer_alsa_track_element_cb(track, elem);
  }

  return 0;
}


static GstMixerAlsaTrack *get_named_playback_track (GstMixerAlsa *mixer,
                                                     const char *name)
{
  GList *tracklist = gst_mixer_list_tracks (GST_MIXER(mixer));
  GList *item;
  GstMixerTrack *track;

  for (item = tracklist; item; item = item->next) {
    track = GST_MIXER_TRACK (item->data);
    if (! IS_OUTPUT(track))
      continue;
    if (!strcmp (gst_mixer_track_get_name (track), name))
      return GST_MIXER_ALSA_TRACK(track);
  }
  return NULL;
}


static void gst_mixer_alsa_mark_master_track (GstMixerAlsa *mixer)
{
  GList *item;
  GstMixerAlsaTrack *track;
  GList *tracklist = gst_mixer_list_tracks (GST_MIXER(mixer));

  if ((track = get_named_playback_track (mixer, "Master")) ||
      (track = get_named_playback_track (mixer, "Front")) ||
      (track = get_named_playback_track (mixer, "PCM")) ||
      (track = get_named_playback_track (mixer, "Speaker")))
    goto found;

  /* If not found, take a mono track with both volume and switch */
  for (item = tracklist; item; item = item->next)
  {
    track = GST_MIXER_ALSA_TRACK (item->data);
    if (! IS_OUTPUT(track))
      continue;
    if (HAS_VOLUME(track) && HAS_SWITCH(track) &&
        NUM_CHANNELS(track) == 1)
      goto found;
  }

  /* If not found, take any track with both volume and switch */
  for (item = tracklist; item; item = item->next)
  {
    track = GST_MIXER_ALSA_TRACK (item->data);
    if (! IS_OUTPUT(track))
      continue;
    if (HAS_VOLUME(track) && HAS_SWITCH(track))
      goto found;
  }

  /* If not found, take any track with volume */
  for (item = tracklist; item; item = item->next)
  {
    track = GST_MIXER_ALSA_TRACK (item->data);
    if (! IS_OUTPUT(track))
      continue;
    if (HAS_VOLUME(track))
      goto found;
  }

  return;

 found:
  gst_mixer_alsa_track_set_master (track);
  return;
}


static void gst_mixer_alsa_create_track_list (GstMixerAlsa *mixer)
{
  GList *tracklist = gst_mixer_list_tracks (GST_MIXER(mixer));
  snd_mixer_elem_t *element, *temp;
  GList *item;

  if (tracklist)
    return;

  for (element = snd_mixer_first_elem (mixer->handle); element;
       element = snd_mixer_elem_next (element))
  {
    GstMixerAlsaTrack *play_track = NULL;
    GstMixerAlsaTrack *cap_track = NULL;
    const gchar *name = snd_mixer_selem_get_name (element);
    int index = 0;
    int has_volume, has_switch;

    for (item = tracklist; item; item = item->next)
    {
      temp = gst_mixer_alsa_track_get_element (GST_MIXER_ALSA_TRACK (item->data));
      if (strcmp (name, snd_mixer_selem_get_name (temp)) == 0)
        index++;
    }

    has_volume = snd_mixer_selem_has_playback_volume (element);
    has_switch = snd_mixer_selem_has_playback_switch (element);
    if (has_volume || has_switch)
    {
      play_track = gst_mixer_alsa_track_new (element,
                                             index,
                                             GST_MIXER_TRACK_OUTPUT,
                                             has_volume,
                                             has_switch,
                                             FALSE);

      gst_mixer_new_track (GST_MIXER(mixer),
                           GST_MIXER_TRACK(play_track));
    }

    has_volume = snd_mixer_selem_has_capture_volume (element);
    has_switch = snd_mixer_selem_has_capture_switch (element);
    if (play_track && snd_mixer_selem_has_common_volume (element))
      has_volume = 0;
    if (play_track && snd_mixer_selem_has_common_switch (element))
      has_switch = 0;
    if (has_volume || has_switch)
    {
      cap_track = gst_mixer_alsa_track_new  (element,
                                             index,
                                             GST_MIXER_TRACK_INPUT,
                                             has_volume,
                                             has_switch,
                                             play_track != NULL);
      gst_mixer_new_track (GST_MIXER(mixer),
                           GST_MIXER_TRACK(cap_track));
    }

    if (play_track && cap_track)
    {
      gst_mixer_alsa_track_set_shared_mute (play_track, cap_track);
      gst_mixer_alsa_track_set_shared_mute (cap_track, play_track);
    }

    if (snd_mixer_selem_is_enumerated (element))
    {
      gst_mixer_new_track (GST_MIXER(mixer),
                           GST_MIXER_TRACK(gst_mixer_alsa_options_new (element, index)));
    }

    snd_mixer_elem_set_callback_private (element, mixer);
    snd_mixer_elem_set_callback (element, mixer_elem_callback);
  }

  gst_mixer_alsa_mark_master_track (mixer);
}


static gboolean gst_mixer_alsa_src_callback (gpointer user_data)
{
  GstMixerAlsa *mixer = (GstMixerAlsa *)user_data;

  snd_mixer_handle_events (mixer->handle);
  return TRUE;
}


static gboolean gst_mixer_alsa_src_dispatch (GSource *source,
                                              GSourceFunc callback,
                                              gpointer user_data)
{
  return callback (user_data);
}

static void gst_mixer_alsa_src_attach (GstMixerAlsa *mixer)
{
  static GSourceFuncs func = {
    .dispatch = gst_mixer_alsa_src_dispatch,
  };
  struct pollfd pfd;

  if (snd_mixer_poll_descriptors (mixer->handle, &pfd, 1) != 1)
    return;

  mixer->src = g_source_new (&func, sizeof (*mixer->src));
  g_source_add_unix_fd (mixer->src, pfd.fd, G_IO_IN | G_IO_ERR);
  g_source_set_callback (mixer->src, gst_mixer_alsa_src_callback, mixer, NULL);
  g_source_attach (mixer->src, g_main_context_default ());
}


static int
gst_mixer_alsa_new (const char *name, GstMixer **mixer_ret)
{
  GstMixerAlsa *mixer;
  gchar *card_name;
  void *handle; /* snd_mixer_t */
  snd_ctl_card_info_t *info;

  snd_hctl_t *hctl;
  int err;

  err = snd_mixer_open ((snd_mixer_t **) &handle, 0);
  if (err < 0)
    goto error;

  err = snd_mixer_attach (handle, name);
  if (err < 0)
    goto error;

  err = snd_mixer_selem_register (handle, NULL, NULL);
  if (err < 0)
    goto error;

  err = snd_mixer_load (handle);
  if (err < 0)
    goto error;

  snd_mixer_get_hctl (handle, name, &hctl);

  snd_ctl_card_info_alloca (&info);
  snd_ctl_card_info (snd_hctl_ctl (hctl), info);
  card_name = g_strdup_printf ("%s (Alsa mixer)",
                               snd_ctl_card_info_get_name (info));

  mixer = (GstMixerAlsa *) g_object_new (GST_MIXER_TYPE_ALSA,
                                         "name", name,
                                         "card-name", card_name,
                                         NULL);
  g_free(card_name);

  mixer->handle = handle;
  snd_mixer_set_callback_private (mixer->handle, mixer);
  snd_mixer_set_callback (mixer->handle, gst_mixer_alsa_callback);

  gst_mixer_alsa_create_track_list (mixer);

  gst_mixer_alsa_src_attach (mixer);

  *mixer_ret = (GstMixer*)mixer;
  return 0;

error:
  return err;
}


GList *gst_mixer_alsa_probe (GList *card_list)
{
#ifdef __linux__
  int card = -1;
#endif

#ifdef __linux__
  while (snd_card_next(&card) >= 0 && card >= 0) {
    char name [16];
#endif
    GstMixer *mixer;
    int err;
#ifdef __linux__
    sprintf (name, "hw:%d", card);
    err = gst_mixer_alsa_new (name, &mixer);
#else
    err = gst_mixer_alsa_new ("default", &mixer);
#endif
    if (err < 0)
      return NULL;
    card_list = g_list_append (card_list, mixer);
#ifdef __linux__
  }
#endif

  return card_list;
}

/*
 * Copyright (c) 2004 Danny Milosavljevic <danny_milo@yahoo.com>
 * Copyright (c) 2004 Benedikt Meurer <benny@xfce.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_SUN

#define USE_THAT 1

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/audioio.h>
#include <sys/modctl.h>
#include <unistd.h>
#include <stropts.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#define VC_PLUGIN
#include "vc.h"

#define MAX_MIXERS 10
#define MIXER_BASE "/dev/audioctl"

static char mixer_device[PATH_MAX] = MIXER_BASE;
static int fd = -1;

static int init(void)
{
  if (g_getenv ("AUDIODEV") != NULL)
    g_strlcpy (mixer_device, g_getenv ("AUDIODEV"), PATH_MAX);
  else
    g_strlcpy (mixer_device, MIXER_BASE, PATH_MAX);

	vc_set_device(mixer_device);
	vc_reinit_device();
	return USE_THAT;
}

static void vc_close_device()
{
	if (fd != -1) {
		(void)close (fd);
		fd = -1;
	}
}

static int vc_reinit_device(void)
{
	vc_close_device ();
	if ((fd = open(mixer_device, O_RDONLY)) < 0)
		return -1;
		
	return 0;
}

static void vc_set_device(char const *name)
{
  /* Not supported */
}


static gboolean set_dev (const char *which, int *pport, gboolean *pinput)
{
  gboolean input = FALSE;
  gint port = 0;
  audio_info_t info;

  if (strcmp (which, "Headphone") == 0)
    {
      input = FALSE;
      port = AUDIO_HEADPHONE;
    }
  else if (strcmp (which, "Speaker") == 0)
    {
      input = FALSE;
      port = AUDIO_SPEAKER;
    }
  else if (strcmp (which, "Line Out") == 0)
    {
      input = FALSE;
      port = AUDIO_LINE_OUT;
    }
  else if (strcmp (which, "Microphone") == 0)
    {
      input = TRUE;
      port = AUDIO_MICROPHONE;
    }
  else if (strcmp (which, "Line In") == 0)
    {
      input = TRUE;
      port = AUDIO_LINE_IN;
    }
  else if (strcmp (which, "CD") == 0)
    {
      input = TRUE;
      port = AUDIO_CD;
    }
  else if (strcmp (which, "SPDIF In") == 0)
    {
      input = TRUE;
      port = AUDIO_SPDIF_IN;
    }
  else if (strcmp (which, "Aux1 In") == 0)
    {
      input = TRUE;
      port = AUDIO_AUX1_IN;
    }
  else if (strcmp (which, "Aux2 In") == 0)
    {
      input = TRUE;
      port = AUDIO_AUX2_IN;
    }
  else
    {
      return FALSE;
    }

  AUDIO_INITINFO (&info);

  if (ioctl (fd, AUDIO_GETINFO, &info) < 0)
    {
      g_warning ("Unable to query audio info from %s: %s", MIXER_BASE, g_strerror (errno));
      close (fd);
      fd = -1;
      return FALSE;
    }

  if (input)
    {
      if (info.record.port == port)
        return TRUE;
      info.record.port = port;
    }
  else
    {
      if (info.play.port == port)
        return TRUE;
      info.play.port = port;
    }

  if (ioctl (fd, AUDIO_SETINFO, &info) < 0)
    {
      g_warning ("Unable to set audio info on %s: %s", MIXER_BASE, g_strerror (errno));
      close (fd);
      fd = -1;
      return FALSE;
    }

  *pport = port;
  *pinput = input;

  return TRUE;
}

static int vc_get_volume(char const *which)
{
  gboolean input;
  gint port;
  audio_info_t info;
  gint gain;

  if (fd < 0)
    return 0;

  if (!set_dev (which, &port, &input))
    return 0;

  AUDIO_INITINFO (&info);

  if (ioctl (fd, AUDIO_GETINFO, &info) < 0)
    {
      g_warning ("Unable to query volume from %s: %s", MIXER_BASE, g_strerror (errno));
      close (fd);
      fd = -1;
      return 0;
    }

  if (input)
    gain = info.record.gain;
  else
    gain = info.play.gain;

  gain = ((gain - AUDIO_MIN_GAIN) * 100) / (AUDIO_MAX_GAIN - AUDIO_MIN_GAIN);

	return gain;
}

static void vc_set_volume(char const *which, int vol_p)
{
  gboolean input;
  gint port;
  audio_info_t info;
  gint gain;

  if (fd < 0)
    return;

  if (!set_dev (which, &port, &input))
    return;

  AUDIO_INITINFO (&info);

  if (ioctl (fd, AUDIO_GETINFO, &info) < 0)
    {
      g_warning ("Unable to query volume from %s: %s", MIXER_BASE, g_strerror (errno));
      close (fd);
      fd = -1;
      return;
    }

  gain = vol_p * (AUDIO_MAX_GAIN - AUDIO_MIN_GAIN) / 100;
  if (input)
    info.record.gain = gain;
  else
    info.play.gain = gain;

  if (ioctl (fd, AUDIO_SETINFO, &info) < 0)
    {
      g_warning ("Unable to set volumne of %s: %s", MIXER_BASE, g_strerror (errno));
      close (fd);
      fd = -1;
      return;
    }
}

static GList *vc_get_control_list()
{
  GList *lp = NULL;

  /* output ports */
  lp = g_list_append (lp, g_strdup ("Headphone"));
  lp = g_list_append (lp, g_strdup ("Speaker"));
  lp = g_list_append (lp, g_strdup ("Line Out"));

  /* input ports */
  lp = g_list_append (lp, g_strdup ("Microphone"));
  lp = g_list_append (lp, g_strdup ("Line In"));
  lp = g_list_append (lp, g_strdup ("CD"));
  lp = g_list_append (lp, g_strdup ("SPDIF In"));
  lp = g_list_append (lp, g_strdup ("Aux1 In"));
  lp = g_list_append (lp, g_strdup ("Aux2 In"));

	return lp;
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}


static GList *vc_get_device_list()
{
  return g_list_append (NULL, g_strdup (MIXER_BASE));
}

static void vc_set_select(char const *which, gchar const *v)
{
} 

static gchar *vc_get_select(char const *which)
{  
	return NULL;
}

static void vc_set_switch(char const *which, gboolean v)
{
}
  
static gboolean vc_get_switch(char const *which)
{
	return FALSE;
}

static char const *vc_get_device()
{
	return NULL;
}

static void vc_handle_events()
{
}

REGISTER_VC_PLUGIN(sun);

#endif /* !USE_SUN */

/*
 * Copyright (c) 2003 Danny Milosavljevic <danny_milo@yahoo.com>
 * Copyright (c) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
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
 */

/* sf is weird(tm) */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_OSS

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#elif defined(HAVE_MACHINE_SOUNDCARD_H)
#include <machine/soundcard.h>
#elif defined(HAVE_SOUNDCARD_H)
#include <soundcard.h>
#endif

#include <glib.h>

#include <libxfce4util/i18n.h>

#define VC_PLUGIN
#include "vc.h"

#define MAX_AMP 100

static char dev_name[PATH_MAX] = { "/dev/mixer" };

static int mixer_handle = -1;
static int devmask = 0;                /* Bitmask for supported mixer devices */
static int master_i = -1;

static char *label[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_LABELS;

static void
find_master(void)
{
	int i;
	
	g_return_if_fail(mixer_handle != -1);

	devmask = 0;
	master_i = -1;

	if (ioctl(mixer_handle, SOUND_MIXER_READ_DEVMASK, &devmask) == -1) {
		perror("Unable to get mixer device mask");
		(void)close(mixer_handle);
		mixer_handle = -1;
		return;
	}
	
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (devmask & (1 << i)) {
			/* if in doubt, choose the first */
			if (master_i == -1)
				master_i = 0;

			if ((!strcasecmp(label[i], "Master"))
		 	 || (!strncasecmp(label[i], "Vol", 3)))
				master_i = i;
		}
	}
}

static void
vc_set_device(const gchar *name)
{
	if (mixer_handle != -1) {
		(void)close(mixer_handle);
		mixer_handle = -1;
	}

	g_strlcpy(dev_name, name, sizeof(dev_name) - 1);
	mixer_handle = open(dev_name, O_RDWR, 0);
}

static int
vc_reinit_device()
{
	find_master();

	if (master_i == -1) {
		g_warning(_("oss: No master"));
		return -1;
	}
	
	return 0;
}

static int
init(void)
{
	vc_set_device(dev_name);
	vc_reinit_device();
	return 1;
}

/*
 * Purpose: Finds the given control by name
 * Returns: the index required for the MIXER_WRITE ioctl or -1 on error
 */
static int
find_control(char const *name)
{
	int	i;
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (devmask & (1 << i)) {
			if (!strcmp (label[i], name)) {
				return i;
			}
		}
	}
	
	return -1;
}


/*
 * Sets volume in percent
 */
static void
vc_set_volume(char const *which, int percent)
{
	int vol;
	int level;
	int i;

	g_return_if_fail(mixer_handle != -1);

	if (!which) {
		i = master_i;
	} else {
		i = find_control (which);
	}
	g_return_if_fail(i != -1);

	vol = (percent * MAX_AMP) / 100;
	level = (vol << 8) | vol;

	if (ioctl(mixer_handle, MIXER_WRITE(i), &level) < 0)
		perror("oss: Unable to set volume");
}

#define LEFT(lvl)	((lvl) & 0x7f)
#define RIGHT(lvl)	(((lvl) >> 8) & 0x7f)

/*
 * Returns volume in percent
 */
static int
vc_get_volume(char const *which)
{
	int level;
	int i;

	g_return_val_if_fail(mixer_handle != -1, 0);

	if (!which) {
		i = master_i;
	} else {
		i = find_control (which);
	}

	g_return_val_if_fail(i != -1, 0);

	level = 0;

	if (ioctl(mixer_handle, MIXER_READ(i), &level) == -1) {
		perror("oss: Unable to get volume");
		return(0);
	}

	return((((LEFT(level) + RIGHT(level)) >> 1) * 100) / MAX_AMP);
}

static GList *
vc_get_control_list(void)
{
	int 	i;
	volcontrol_t *c;

	GList *	g;
	
#if 0
	g = g_list_alloc ();
	if (!g) return; /* error */
#endif
	g = NULL;
		
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (devmask & (1 << i)) {
			/* if in doubt, choose the first */

			c = g_new0 (volcontrol_t, 1);
			if (c) {
				c->name = g_strdup(label[i]);
			
				g = g_list_append(g, c);
			}

		}
	}

	return g;	
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}

static void vc_close_device()
{
	if (mixer_handle != -1) {
		close (mixer_handle);
		mixer_handle = -1;
	}
}

static GList *vc_get_device_list()
{
	GList *l;
	GDir *dir;
	char const *d;
	
	l = NULL;
	
	dir = g_dir_open ("/dev/sound", 0, NULL);

	if (dir) {
		while ((d = g_dir_read_name (dir))) {
			if (!strncmp(d, "mixer", 5)) {
				l = g_list_append (l, g_strdup_printf("/dev/sound/%s", d));
			}
		}
		
		g_dir_close (dir);
	}
	
	dir = g_dir_open ("/dev", 0, NULL);

	if (dir) {
		while ((d = g_dir_read_name (dir))) {
			if (!strncmp(d, "mixer", 5)) {
				l = g_list_append (l, g_strdup_printf("/dev/%s", d));
			}
		}
		
		g_dir_close (dir);
	}
	
	return l;
}

REGISTER_VC_PLUGIN(oss);

#endif	/* !USE_OSS */


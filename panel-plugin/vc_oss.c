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

#define VC_PLUGIN
#include "vc.h"

#define MAX_AMP 100

/* TODO */

static char dev_name[PATH_MAX] = { "/dev/mixer" };

static int mixer_handle = -1;
static int devmask = 0;                /* Bitmask for supported mixer devices */
static int	master_i = -1;

/*static int level[SOUND_MIXER_NRDEVICES];*/
static char *label[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_LABELS;
/*
left = level[i] & 0x7f;
right = (level[i] >> 8) & 0x7f;
*/

static void find_master()
{
	int	i;
	devmask = 0;
	master_i = -1;
	
	if (mixer_handle == -1) return;

	g_print("find_master()\n");
	
	if (ioctl(mixer_handle, SOUND_MIXER_READ_DEVMASK, &devmask) == -1)
	{
		perror("SOUND_MIXER_READ_DEVMASK");
		close(mixer_handle); mixer_handle = -1;
	}
	
	for(i = 0; i < SOUND_MIXER_NRDEVICES; i++) if (devmask & (1 << i)) {
		if (master_i == -1) { master_i = 0; } /* if in doubt, choose the first */
		/*level[i] = 0;*/
		if ((!strcasecmp(label[i], "Master")) || (!strncasecmp(label[i], "Vol", 3))) {
			master_i = i;
		}
	}
}

static void set_device(char const *name)
{
	if (mixer_handle != -1) {
		close(mixer_handle);
		mixer_handle = -1;
	}
	strncpy(dev_name, name, sizeof(dev_name) - 1);
	dev_name[sizeof(dev_name) - 1] = 0;
	mixer_handle = open(dev_name, O_RDWR, 0);
	find_master();
}

static int reinit_device()
{
	find_master();
	if (master_i == -1) {
		g_warning("No master");
		return -1;
	}
	
	return 0;
}

static int init(void)
{
	set_device(dev_name);
	return 1;
}



static void set_master_volume(int vol_p)
{
	int	vol;
	int	level;
	if (mixer_handle == -1 || master_i == -1) return;

	vol = (vol_p * MAX_AMP) / 100;

	level = (vol << 8) | vol;

	ioctl(mixer_handle, MIXER_WRITE(master_i), &level);
}

static int get_master_volume(void)
{
	int	left;
	int	right;
	int	vol;
	int	vol_p; /* % */
	int	level;
	
	if (mixer_handle == -1 || master_i == -1) return 0;

	level = 0;
	if (ioctl(mixer_handle, MIXER_READ(master_i), &level) == -1) {
		perror("MIXER_READ");
		/* nm */
	}

	left = level & 0x7f;
	right = (level >> 8) & 0x7f;
	
	vol = (left + right) >> 1;
	vol_p = (vol * 100) / MAX_AMP; /* I guess */

	return vol_p;
}

REGISTER_VC_PLUGIN(oss);

#endif	/* !USE_OSS */


/*
 * Copyright (c) 2004 Danny Milosavljevic <danny_milo@yahoo.com>
 * Copyright (c) 2004 Benedikt Meurer
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
#include <sys/audioio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <limits.h>

#define VC_PLUGIN
#include "vc.h"

#define MAX_MIXERS 10
#define MIXER_BASE "/dev/mixer"

static char mixer_device[PATH_MAX] = "/dev/mixer";
static int fd = -1;

static int init(void)
{
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
	if ((fd = open(mixer_device, O_RDONLY | O_SHLOCK)) < 0)
		return -1;
		
	return 0;
}

static void vc_set_device(char const *name)
{
	if (!name)
		mixer_device[0] = 0;
	else
		(void)strlcpy(mixer_device, name, PATH_MAX);
}


static int vc_get_volume(char const *which)
{
	return 0;  
}

static void vc_set_volume(char const *which, int vol_p)
{
}

static GList *vc_get_control_list()
{
	return NULL;
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}


static GList *vc_get_device_list()
{
	int i;
	int fd;
	GList *g;
	char device[PATH_MAX];
	audio_device_t devinfo;

	g = NULL;
	for(i = 0; i < MAX_MIXERS; i++) {
		(void)snprintf(device, PATH_MAX, "%s%d", MIXER_BASE, n);

		if ((fd = open(device, O_RDONLY | O_SHLOCK)) < 0)
			continue;

		if (ioctl(fd, AUDIO_GETDEV, &devinfo) < 0) {
			(void)close(fd);
			continue;
		}
		
		/*
		   (void)strlcpy(mixers[n].name, devinfo.name, MAX_AUDIO_DEV_LEN);      #
	    (void)strlcpy(mixers[n].config, devinfo.config,                              MAX_AUDIO_DEV_LEN);
		*/
		
		(void)close(fd);
		g = g_list_append (g, g_strdup (device));
	}
	return g;
}

REGISTER_VC_PLUGIN(sun);

#endif /* !USE_SUN */

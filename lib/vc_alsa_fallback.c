/*
 * Copyright (c) 2003 Danny Milosavljevic <danny_milo@yahoo.com>
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
 * XXX - this is ugly. Enable on last resort. Really now. Change system() and
 *       popen() accordingly.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_ALSA

#define USE_THAT 0

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#define VC_PLUGIN
#include "vc.h"

static int init(void)
{
	return USE_THAT;
}

static int vc_reinit_device(void)
{
	/* TODO */
	return 0;
}

static void vc_set_device(char const *name)
{
	/* in your dreams ;) */
}


static int vc_get_volume(char const *which)
{
	/*char buf[1000];*/
	FILE *f;
	int     vol_p = 0;

	f = popen("LC_MESSAGES=\"C\" amixer sget Master "
		" |tail -1 |awk '{ print $5 }' "
		"|sed 's;\\[\\([0-9][0-9]*\\)%\\];\\1;'", "r");

	if (f && fscanf(f, "%d", &vol_p) == 1) {
		pclose(f);
	} else if (f) {
		pclose(f);
        }

	return vol_p;  
}

static void vc_set_volume(char const *which, int vol_p)
{
	char buf[1000];
	snprintf(buf,sizeof(buf), "amixer sset Master \"%d%%\" >/dev/null", vol_p);
	system(buf);
}

static GList *vc_get_control_list()
{
	GList *g;
	
	g = g_list_alloc ();
	g_list_append (g, g_strdup("Master")); 
	
	return g;
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}

static void vc_close_device()
{
	/* unsupported */
}

static GList *vc_get_device_list()
{
	return NULL;
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
         
REGISTER_VC_PLUGIN(alsa_fallback);

#endif /* !USE_ALSA */

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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_ALSA

#define USE_THAT 1

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <xfce4/libxfcegui4/dialogs.h>
#include <libxfce4util/i18n.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#define VC_PLUGIN
#include "vc.h"

#ifdef DEBUG
#define error printf
/*#define error xfce_info*/
#else
#define error printf
#endif

static char card[64] = "default";
static snd_mixer_t	*handle = NULL;
static snd_mixer_elem_t *elem = NULL;

static void find_master(void)
{
	int		err;
	snd_mixer_selem_id_t	*sid, *sid2;
	char buf[12] = "Master";
	char buf2[12] = "PCM";
	
	elem = NULL;

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, buf);

	snd_mixer_selem_id_alloca(&sid2);
	snd_mixer_selem_id_set_index(sid2, 0);
	snd_mixer_selem_id_set_name(sid2, buf2);

	if (handle) {
		snd_mixer_close(handle);
		handle = NULL;
	}
	
	if ((err = snd_mixer_open(&handle, 0)) < 0 || !handle) {
		error(_("alsa: Mixer %s open error: %s\n"), card, snd_strerror(err));
		return;
	}
	
	if ((err = snd_mixer_attach(handle, card)) < 0) {
		error(_("alsa: Mixer attach %s error: %s\n"), card, snd_strerror(err));
		snd_mixer_close(handle); /* <-- alsa 0.9.3a fails assert(hctl) if I do that... weeeeird... */
		handle = NULL;
		elem = NULL; /* just to be sure */
		return;
	}
	if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
#ifdef DEBUG
		xfce_info(_("alsa: Mixer register error: %s"), snd_strerror(err));  
#endif
		snd_mixer_close(handle);
		handle = NULL;
		return;
	}
        err = snd_mixer_load(handle);
	if (err < 0) {
#ifdef DEBUG
		xfce_info(_("alsa: Mixer load error: %s"), card, snd_strerror(err));
#endif
		snd_mixer_close(handle);
		return;
	}
	elem = snd_mixer_find_selem(handle, sid);
	if (!elem) {
		elem = snd_mixer_find_selem(handle, sid2);
		if (!elem) {
#ifdef DEBUG
			xfce_info(_("alsa: Unable to find simple control '%s',%i\n"),
			snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
#endif
			snd_mixer_close(handle);
			return;
		}
	}
}

static void vc_set_device(char const *name)
{
	strncpy(card, name, sizeof(card));
	card[sizeof(card) - 1] = 0;
	find_master();
}

static int vc_reinit_device(void)
{
	find_master();
	if (!elem) return -1;
	
	return 0;
}

static int init(void)
{
	find_master();
	return USE_THAT;
}

static snd_mixer_elem_t * find_control(char const *which)
{
	snd_mixer_elem_t *elem = NULL;

	snd_mixer_selem_id_t	*sid;

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, which);

	elem = snd_mixer_find_selem(handle, sid);
	
	return elem;
}

static int vc_get_volume(char const *which)
{
	long pmin,pmax;
	long lval;
	snd_mixer_selem_channel_id_t chn;
	snd_mixer_elem_t *xelem = NULL;

	if (!handle) return 0;

	if (which) {
		xelem = find_control (which);
	} else {
		xelem = elem;
	}

	if (!xelem) return 0;

	snd_mixer_selem_get_playback_volume_range(xelem, &pmin, &pmax);

	/* if (snd_mixer_selem_has_playback_volume(xelem)) { */
	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_mixer_selem_has_playback_channel(xelem, chn)) continue;
	
		snd_mixer_selem_get_playback_volume(xelem, chn, &lval); 

		/*xfce_info("%ld,%ld,%ld,%ld", pmin,pmax,lval,(lval - pmin) * 100 / (pmax-pmin));*/
		if (pmax > pmin) {
			return (lval - pmin) * 100 / (pmax-pmin);
		} else {
			return lval;
		}
	}
	return 0;
}

static void vc_set_volume(char const *which, int vol_p)
{
	long pmin,pmax;
	long lval;
	snd_mixer_selem_channel_id_t chn;
	snd_mixer_elem_t *xelem = NULL;
	
	if (!handle) return;
	
	if (which) {
		xelem = find_control (which);
	} else {
		xelem = elem;
	}
	
	if (!xelem) return;

	snd_mixer_selem_get_playback_volume_range(xelem, &pmin, &pmax);


	/*vol_p = (lval - pmin) * 100 / (pmax - pmin);*/
	lval = pmin + vol_p * (pmax - pmin) / 100;

	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_mixer_selem_has_playback_channel(xelem, chn)) continue;
	
		snd_mixer_selem_set_playback_volume(xelem, chn, lval);
		/* lol*/
		/*return;*/
	}
}

static GList *vc_get_control_list(void)
{
	volcontrol_t *c;
	GList *g;
	snd_mixer_elem_t *b; /* begin */
	snd_mixer_elem_t *e; /* end */
	snd_mixer_elem_t *i; /* item */
	char const *n; /* name */
	
	g = NULL;
#if 0
	g_list_alloc ();
	if (!g) return NULL;
#endif

	if (!handle) {
		return NULL;
	}

	b = snd_mixer_first_elem (handle);
	e = snd_mixer_last_elem (handle);
	
	i = b;
	while (i) {
		n = snd_mixer_selem_get_name (i);

		c = g_new0(volcontrol_t, 1);
		c->name = g_strdup(n);	

		g = g_list_append (g, c);
	
		if (i == e)
			break;

		i = snd_mixer_elem_next (i);
	}
	

	return g;
}

static volchanger_callback_t mycb = NULL;
static void *mydata = NULL;

static int alsa_cb(snd_mixer_t *ctl, unsigned int mask, snd_mixer_elem_t *elem)
{
	const char *which;
	if (elem) {
		which = snd_mixer_selem_get_name (elem);
	} else {
		which = NULL;
	}
	(*mycb) (which, mydata);
	return 0;
}


static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* supports 1 (ONE) callback */

	if (!handle) return;

	mycb = cb;
	mydata = data;
	
	snd_mixer_set_callback_private (handle, data);
	snd_mixer_set_callback (handle, alsa_cb);
}

REGISTER_VC_PLUGIN(alsa);

#endif /* USE_ALSA */

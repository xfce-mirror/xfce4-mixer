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

/*
snd_mixer_selem_is_active ?
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

#define _(x) (x)

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <alsa/control.h>

#define VC_PLUGIN
#include "vc.h"

#include "stringlist.inc"

#ifdef DEBUG
#define error printf
/*#define error xfce_info*/
#else
#define error printf
#endif

static snd_mixer_t	*handle = NULL;
static char card[64] = "default";

#define MAX_MASTERS 10

static char const* master_ids[MAX_MASTERS] = {
	"Master",
	"PCM",
	"Analog Front",
	"Speaker",
	NULL,
};

static void show_developer_hint(void)
{
	snd_mixer_elem_t*                first_control;
	snd_mixer_elem_t*                last_control;
	snd_mixer_elem_t*                current_control;
	gboolean                         has_playback;
	char const*                      control_name;
	int                              control_index;

	first_control = snd_mixer_first_elem (handle);
	last_control = snd_mixer_last_elem (handle);

	fprintf(stderr, _("alsa: error: master control not found. I even tried to guess wildly, but to no avail.\n"));
	if (first_control == NULL) {
		return;
	}
	
	fprintf(stderr, _("alsa: info: developer information follows: (send E-Mail to Developer with that)\n"));

	current_control = first_control;
	while (current_control != NULL) {
		control_name = snd_mixer_selem_get_name (current_control);
		control_index = snd_mixer_selem_get_index (current_control);

		has_playback = snd_mixer_selem_has_common_volume(current_control)
		             ||  snd_mixer_selem_has_playback_volume(current_control);

		fprintf(stderr, "alsa: * %s,%d", control_name, control_index);

		if (has_playback) {
			fprintf(stderr, "  <--- ");
			fprintf(stderr, _("hint hint"));
		}
		fprintf(stderr, "\n");

		current_control = snd_mixer_elem_next (current_control);
	}
	fprintf(stderr, _("alsa: info: end of developer information\n"));
}

static void find_master(void)
{
	int                     err;
	int                     i;

	snd_mixer_selem_id_t*   master_selectors[MAX_MASTERS] = { NULL };

	if (handle != NULL) {
		snd_mixer_close(handle);
		handle = NULL;
	}

	if ((err = snd_mixer_open(&handle, 0)) < 0 || handle == NULL) {
		error(_("alsa: Mixer %s open error: %s\n"), card, snd_strerror(err));
		return;
	}
	
	if ((err = snd_mixer_attach(handle, card)) < 0) {
		error(_("alsa: snd_mixer_attach(\"%s\"): error: %s\n"), card, snd_strerror(err));
		snd_mixer_close(handle); /* <-- alsa 0.9.3a fails assert(hctl) if I do that... weeeeird... */
		handle = NULL;
		return;
	}
	if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
#ifdef DEBUG
		error(_("alsa: snd_mixer_selem_register(...): error: %s\n"), snd_strerror(err));  
#endif
		snd_mixer_close(handle);
		handle = NULL;
		return;
	}
        err = snd_mixer_load(handle);
	if (err < 0) {
#ifdef DEBUG
		error(_("alsa: Mixer load error: %s: %s\n"), card, snd_strerror(err));
#endif
		snd_mixer_close(handle);
		handle = NULL;
		return;
	}
}

static void vc_set_device(char const *name)
{
	gchar	*colon;
	if (name && name[0] == '#') {
		card[0] = 'h'; card[1] = 'w'; card[2] = ':';
		strncpy(card + 3, &name[1], sizeof(card) - 3);
		card[sizeof(card) - 1] = 0;
		colon = g_utf8_strchr (card + 3, sizeof(card) - 4, ':');
		if (colon) *colon = 0;
		find_master();
	} else if (!strcmp (name, "default")) {
		strncpy(card, name, sizeof(card));
		card[sizeof(card) - 1] = 0;
	}
}

static int vc_reinit_device(void)
{
	find_master();
	if (handle == NULL) {
		return -1;
	}
	
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
	int		idx;
	gchar		**g;
	char const	*name;

	snd_mixer_selem_id_alloca(&sid);
	

	if (!which) {
		return NULL;
	}
	
	g = g_strsplit (which, ",", 2);
	if (g) {
		name = g[0];
		if (g[1] && *g[1]) {
			idx = atoi (g[1]);
		} else {
			idx = 0;
		}
	} else {
		name = which;
		idx = 0;
	}
	
	snd_mixer_selem_id_set_index(sid, idx);
	snd_mixer_selem_id_set_name(sid, name);

	elem = snd_mixer_find_selem(handle, sid);

	if (g) {
		g_strfreev (g);
	}
	
	return elem;
}

static double calculate_balance_ratio(long left, long right)
{
	return (left != right) ? (left > right ? (double)left / right : 0 -(double)right / left) : 0;
}

static long calculate_volume_delta(long left, long right, long new)
{
	return new - ((left + right) >> 1);
}

static int vc_get_volume(char const *which)
{
	int playback_enable;
	int capture_enable;
	long playback_min;
	long playback_max;
	long capture_min;
	long capture_max;
	long playback_value;
	long capture_value;

	snd_mixer_selem_channel_id_t chn;
	snd_mixer_elem_t *xelem;

	if (handle == NULL) {
		return 0;
	}

	xelem = (which != NULL) ? find_control (which) : NULL;

	if (xelem == NULL) {
		return 0;
	}

	snd_mixer_selem_get_playback_volume_range (xelem, &playback_min, &playback_max);
	snd_mixer_selem_get_capture_volume_range (xelem, &capture_min, &capture_max);

	playback_min = 0;
	capture_min = 0;

	/* if (snd_mixer_selem_has_playback_volume(xelem)) { */
	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_mixer_selem_has_playback_channel(xelem, chn)) {
			continue;
		}

		snd_mixer_selem_get_playback_volume(xelem, chn, &playback_value); 

		playback_enable = TRUE;
		snd_mixer_selem_get_playback_switch (xelem, chn, &playback_enable);
		if (playback_enable == 0) {
			return 0;
		}

		/*error("%ld,%ld,%ld,%ld", pmin,pmax,lval,(lval - pmin) * 100 / (pmax-pmin));*/
		if (playback_max > playback_min) {
			return (playback_value - playback_min) * 100 / (playback_max - playback_min);
		} else {
			return playback_value;
		}
	}

	/* if this point is reached, only capture setting remain, so use those */
	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_mixer_selem_has_capture_channel(xelem, chn)) {
			continue;
		}

		snd_mixer_selem_get_capture_volume(xelem, chn, &capture_value); 

		capture_enable = TRUE;
		snd_mixer_selem_get_capture_switch (xelem, chn, &capture_enable);
		if (capture_enable == 0) {
			return 0;
		}

		/*error("%ld,%ld,%ld,%ld", pmin,pmax,lval,(lval - pmin) * 100 / (pmax-pmin));*/
		if (capture_max > capture_min) {
			return (capture_value - capture_min) * 100 / (capture_max - capture_min);
		} else {
			return capture_value;
		}
	}

	return 0;
}

static void vc_set_volume(char const *which, int vol_p)
{
	long playback_min;
	long playback_max;
	long capture_min;
	long capture_max;
	long delta;
	long playback_value, playback_value_left, playback_value_right;
	long capture_value;
	long playback_value_current_left, playback_value_current_right;
	static double balance;

	/*snd_mixer_selem_channel_id_t chn;*/
	snd_mixer_elem_t *xelem;
	
	if (handle == NULL) {
		return;
	}

	xelem = (which != NULL) ? find_control (which) : NULL;
	
	if (xelem == NULL) {
		return;
	}

	snd_mixer_selem_get_playback_volume_range (xelem, &playback_min, &playback_max);
	snd_mixer_selem_get_capture_volume_range (xelem, &capture_min, &capture_max);
	snd_mixer_selem_get_playback_volume(xelem, SND_MIXER_SCHN_FRONT_LEFT, &playback_value_current_left);
	snd_mixer_selem_get_playback_volume(xelem, SND_MIXER_SCHN_FRONT_RIGHT, &playback_value_current_right);

	if (playback_value_current_left && playback_value_current_right) {
		balance = calculate_balance_ratio(playback_value_current_left, playback_value_current_right);
	}

	playback_min = 0;
	capture_min = 0;

	/*vol_p = (lval - pmin) * 100 / (pmax - pmin);*/
	playback_value = (long)(((double)(vol_p *playback_max) /100) +0.5);
	capture_value = (long)(((double)(vol_p *capture_max) /100) +0.5);

	delta = calculate_volume_delta(playback_value_current_left, playback_value_current_right, playback_value);

	/* FIXME
	 * This doesn't always work perfectly, due to rounding delta might endup being +1 instead of 0.
	 * Though not audiable, it might be visible on the slider.
	 */
	playback_value_left = playback_value_current_left +delta;
	playback_value_right = playback_value_current_right +delta;

	/* TODO
	 * Clipping top and bottom isn't respected yet.
	 */

	/*
	snd_mixer_selem_get_playback_switch(Ex_elem, SND_MIXER_SCHN_MONO, &status);
	*/
	
	snd_mixer_selem_set_playback_switch(xelem, SND_MIXER_SCHN_FRONT_LEFT, playback_value_left);
	snd_mixer_selem_set_playback_switch(xelem, SND_MIXER_SCHN_FRONT_RIGHT, playback_value_right);

	snd_mixer_selem_set_playback_volume(xelem, SND_MIXER_SCHN_FRONT_LEFT, playback_value_left);
	snd_mixer_selem_set_playback_volume(xelem, SND_MIXER_SCHN_FRONT_RIGHT, playback_value_right);
	snd_mixer_selem_set_capture_volume_all (xelem, capture_value);
#if 0		
	for (chn = 0; chn <= SND_MIXER_SCHN_LAST; chn++) {
		if (!snd_mixer_selem_has_playback_channel(xelem, chn)) continue;

		if (lval == pmin) { /* mute */
			snd_mixer_selem_set_playback_switch (xelem, chn, 0);
			return;
		} else { /* unmute, just in case. */
			snd_mixer_selem_set_playback_switch (xelem, chn, 1);
			snd_mixer_selem_set_playback_volume(xelem, chn, lval);
		}
	}
#endif
}

GList *alsa_enum_to_glist(snd_mixer_elem_t *i)
{
	GList * g;
	gint j;
	char tmp[257];
	gint cnt;
	volchoice_t* choice;
	
	g = NULL;
	if (!snd_mixer_selem_is_enumerated (i))
		return g;
			
	cnt = snd_mixer_selem_get_enum_items (i);
	for(j = 0; j < cnt; j++) {
		g_snprintf (tmp, 256, "%d", j);
		snd_mixer_selem_get_enum_item_name (i, j, 256, tmp);
		
		choice = g_new0 (volchoice_t, 1);
		choice->name = g_strdup (tmp);
		choice->displayname = g_strdup (tmp);
		
		g = g_list_append (g, choice);
	}
			
	return g;
	
}

/* returns list of volcontrol_t */
static GList *vc_get_control_list(void)
{
	volcontrol_t *c;
	GList *g;
	snd_mixer_elem_t *b; /* begin */
	snd_mixer_elem_t *e; /* end */
	snd_mixer_elem_t *i; /* item */
	char const *n; /* name */
	unsigned int id;
	/*snd_mixer_elem_type_t ty;*/
	
	g = NULL;
#if 0
	g_list_alloc ();
	if (!g) return NULL;
#endif

	if (handle == NULL) {
		return NULL;
	}

	b = snd_mixer_first_elem (handle);
	e = snd_mixer_last_elem (handle);
	
	i = b;
	while (i) {
		n = snd_mixer_selem_get_name (i);
		id = snd_mixer_selem_get_index (i);

		c = g_new0(volcontrol_t, 1);
		c->name = g_strdup_printf("%s,%u", n, id);
		c->type = CT_SLIDER;
		
		/*ty = snd_mixer_elem_get_type (i);*/
		
		if (snd_mixer_selem_is_enumerated (i)) {
			c->type = CT_SELECT;
			c->choices = alsa_enum_to_glist (i);
		} else {
			c->choices = NULL;
			
			
			if (!snd_mixer_selem_has_common_volume(i)
			&& !snd_mixer_selem_has_playback_volume(i)
			&& !snd_mixer_selem_has_capture_volume(i)
			&& (snd_mixer_selem_has_common_switch(i)
			 || snd_mixer_selem_has_playback_switch(i)
			 || snd_mixer_selem_has_capture_switch(i)
			))
			{
				c->type = CT_ONOFF;
			}
		}

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
	volchanger_callback_event_t event;

	g_warning ("alsa_cb\n");

	
	if (elem && snd_mixer_elem_get_type (elem) == SND_MIXER_ELEM_SIMPLE) {
		which = snd_mixer_selem_get_name (elem);
	} else {
		which = NULL;
	}

	if (mask == SND_CTL_EVENT_MASK_REMOVE) {
		event = VE_REMOVED;
	} else if (mask & (SND_CTL_EVENT_MASK_VALUE | SND_CTL_EVENT_MASK_INFO)) {
		event = VE_VALUE_CHANGED;
	} else {
		return 0;
	}

	if (mycb) 
		(*mycb) (which, event, mydata);
		
	return 0;
}


static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* supports 1 (ONE) callback */

	if (handle == NULL) {
		return;
	}

	mycb = cb;
	mydata = data;

	snd_mixer_set_callback_private (handle, data);
	snd_mixer_set_callback (handle, alsa_cb);
	/* snd_mixer_elem_set_callback(elem, melem_event); */
}

static void vc_close_device()
{
	if (handle == NULL) {
		return;
	}
	
	snd_mixer_set_callback_private (handle, NULL);
	snd_mixer_set_callback (handle, NULL);
	
	snd_mixer_close (handle); /* FIXME does this close all related stuff? */
	handle = NULL;
}

static GList *vc_get_device_list()
{
	GList *l;
	int card;
	int rc;
	gchar const *name;
	gchar *cname;
	snd_ctl_t *ctl_handle;
	snd_ctl_card_info_t	*card_info;
	snd_ctl_card_info_alloca (&card_info);
	
	card = -1;
	l = NULL;
	
	if (snd_card_next (&card) < 0) {
		return l;
	}

	/* FIXME remove this ? */
	l = g_list_append (l, g_strdup ("default")); /* this is just to make sure */

	do {
		cname = g_strdup_printf ("hw:%d", card);
		rc = snd_ctl_open(&ctl_handle, cname, 0);
		if (rc == 0) {
			if (snd_ctl_card_info (ctl_handle, card_info) >= 0) {
				name = snd_ctl_card_info_get_name (card_info);
			} else {
				name = "?";
			}

			l = g_list_append (l, g_strdup_printf("#%d: %s", card, name));
			snd_ctl_close (ctl_handle);

		}
		
			
		g_free (cname);

		card++;
	} while (rc == 0);		
		
#if 0
	/* SLOOOOW way: */
	while (card >= 0) {
		name = NULL;
		if (snd_card_get_name (card, &name) >= 0 && name) {
			l = g_list_append (l, g_strdup_printf("#%d: %s", card, name));
			g_free (name);
		}
	
		if (snd_card_next(&card) < 0) {
			g_warning("mixer: vc_alsa: snd_card_next failed\n");
			break;
		}
	}
#endif

	return l;
}

static GList *vc_get_choices (snd_mixer_elem_t *elem)
{
	return alsa_enum_to_glist (elem);
}

static void vc_set_select(char const *which, gchar const *v)
{
	GList *g;
	gint i;
	gchar *s;
	snd_mixer_elem_t *xelem = NULL;

	if (handle == NULL) {
		return;
	}
	
	xelem = (which != NULL) ? find_control (which) : NULL;
	
	if (xelem == NULL) {
		return;
	}
	
	g = vc_get_choices (xelem);
	if (!g || !v)
		return;
		
	for(i = 0; i < g_list_length (g); i++) {
		s = (gchar *) g_list_nth_data (g, i);
		if (g_str_equal (s, v)) {
			snd_mixer_selem_set_enum_item(
				xelem, SND_MIXER_SCHN_MONO, i);
			break;
		}
	}
	
	vc_free_choices (g);
} 

static gchar *vc_get_select(char const *which)
{  
	gint j;
	unsigned int jj;
	GList *g;
	volchoice_t* item;
	char* s;

	snd_mixer_elem_t *xelem = NULL;
	if (handle == NULL) {
		return NULL;
	}

	xelem = (which != NULL) ? find_control (which) : NULL;
	
	if (xelem == NULL) {
		return NULL;
	}

	if (snd_mixer_selem_get_enum_item(xelem, SND_MIXER_SCHN_MONO, &jj) < 0)
		return NULL;

	j = (gint)jj;
	
	g = vc_get_choices (xelem);
	if (!g)
		return NULL;

	item = (volchoice_t *) g_list_nth_data (g, j);
	
	if (!item) {
		vc_free_choices (g);
		return NULL;
	}
		
	s = g_strdup (item->name);
	vc_free_choices (g);
	return s;
}
        
static void vc_set_switch(char const *which, gboolean v)
{
	snd_mixer_elem_t *xelem = NULL;

	if (handle == NULL) {
		return;
	}

	xelem = (which != NULL) ? find_control (which) : NULL;
	
	if (xelem == NULL) {
		return;
	}

/*			&& (snd_mixer_selem_has_common_switch(i)
			 || snd_mixer_selem_has_playback_switch(i)
			 || snd_mixer_selem_has_capture_switch(i)
*/

	snd_mixer_selem_set_playback_switch_all (xelem, v);
}
  
static gboolean vc_get_switch(char const *which)
{
	snd_mixer_selem_channel_id_t chn;
	snd_mixer_elem_t *xelem = NULL;
	gint i;
	
	if (!handle) return FALSE;
	
	xelem = (which != NULL) ? find_control (which) : NULL;
	
	if (xelem == NULL) {
		return FALSE;
	}

	chn = SND_MIXER_SCHN_MONO;

	snd_mixer_selem_get_playback_switch (xelem, chn, &i);
	return (i != 0);
}

static char const *vc_get_device()
{
	return card;
}

static void vc_handle_events()
{
	if (handle)
		snd_mixer_handle_events (handle);
}
        
REGISTER_VC_PLUGIN(alsa);

#endif /* USE_ALSA */

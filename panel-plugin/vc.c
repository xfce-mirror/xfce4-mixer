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

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>

#include "vc.h"

#define MAX_VC 5
static volchanger_t *vcs[MAX_VC] = { NULL /* all */ };
static volchanger_t *sel = NULL;

void register_vc(volchanger_t *vc)
{
	int	i;

	for(i = 0; i < MAX_VC; i++) {
		if (vcs[i] == NULL) {
			vcs[i] = vc;
			if (!sel) { sel = vc; }
			break;
		}
	}
}

void unregister_vc(volchanger_t *vc)
{
	int	i;
	for(i = 0; i < MAX_VC; i++) {
		if (vcs[i] == vc) {
			vcs[i] = NULL;
			if (sel == vc) { sel = NULL; }
			break;
		}
	}
}

void unregister_all(void)
{
	int	i;
	for(i = 0; i < MAX_VC; i++) {
		vcs[i] = NULL;
	}
	sel = NULL;
}

void select_vc(char const *name)
{
	int	i;
	for(i = 0; i < MAX_VC; i++) {
		if (vcs[i] && !strcmp(vcs[i]->name, name)) {
			sel = vcs[i];
			return;
		}
	}
	sel = NULL;
}

void select_vc_direct(volchanger_t *v)
{
	int	i;
	for(i = 0; i < MAX_VC; i++) {
		if (vcs[i] && vcs[i] == v) {
			sel = vcs[i];
			return;
		}
	}
	sel = NULL;
}

volchanger_t *selected_vc()
{
	return sel;
}


int get_volume(char const *which)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->get_volume) {
		return 0;
	}
	
	return (*s->get_volume)(which);
}

void set_volume(char const *which, int v)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->set_volume) {
		return;
	}
	
	(*s->set_volume)(which, v);
}

GList *get_control_list()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->get_control_list) {
		return NULL;
	}
	
	return (*s->get_control_list)();
}

void free_control_list(GList *g)
{
/*	GList 			*f;*/
	volcontrol_t		*c;
	int			i = 0;
	
	while (i < 100) { /* safety */
		c = g_list_nth_data (g, i);
		if (c) {
			if (c->name) g_free (c->name);
			g_free (c);
		}
	
		++i;
	}

	g_list_free (g);
}


volchanger_t **first_vc()
{
	int	i;
	for(i = 0; i < MAX_VC; i++) {
		if (vcs[i]) {
			return &vcs[i];
		}
	}
	return NULL;
}

volchanger_t **next_vc(volchanger_t **v)
{
	int	i;
	
	++v;
	
	i = v - vcs;
	if (i < 0 || i >= MAX_VC) {
		return NULL;
	}
	
	while (i < MAX_VC) {
		if (vcs[i]) {
			return &vcs[i];
		}
	
		++i;
	}
	
	return NULL;
}


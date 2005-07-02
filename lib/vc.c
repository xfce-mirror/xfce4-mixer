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

void vc_set_device(char const *which)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_set_device) {
		return;
	}
	
	(*s->vc_set_device)(which);

	if (s->vc_reinit_device) {
		(*s->vc_reinit_device) ();
	} else {
		g_warning ("mixer: vc.c: Cannot reinit since driver did not tell how.\n");
	}
}

char const *vc_get_device()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_device) {
		return NULL;
	}
	
	return (*s->vc_get_device)();
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


int vc_get_volume(char const *which)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_volume) {
		return 0;
	}
	
	return (*s->vc_get_volume)(which);
}

void vc_set_volume_callback (volchanger_callback_t cb, void *data)
{
	volchanger_t *s = selected_vc();
	if (!s || !s->vc_set_volume_callback)
		return;
		
	s->vc_set_volume_callback (cb, data);
}


void vc_set_volume(char const *which, int v)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_set_volume) {
		return;
	}
	
	(*s->vc_set_volume)(which, v);
}

void vc_set_select(char const *which, gchar const *v)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_set_select) {
		return;
	}
	
	(*s->vc_set_select)(which, v);
}

gchar *vc_get_select(char const *which)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_select) {
		return NULL;
	}
	
	return (*s->vc_get_select)(which);
}

void vc_set_switch(char const *which, gboolean v)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_set_switch) {
		return;
	}
	
	(*s->vc_set_switch)(which, v);
}

gboolean vc_get_switch(char const *which)
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_switch) {
		return FALSE;
	}
	
	return (*s->vc_get_switch)(which);
}

/* returns list of volcontrol_t */
GList *vc_get_control_list()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_control_list) {
		return NULL;
	}
	
	return (*s->vc_get_control_list)();
}

void vc_close_device()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_close_device) {
		return;
	}
	
	(*s->vc_close_device)();
}

void vc_handle_events()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_handle_events) {
		return;
	}
	
	(*s->vc_handle_events)();
}

GList *vc_get_device_list()
{
	volchanger_t *s = selected_vc();
	if (!s  || !s->vc_get_device_list) {
		return NULL;
	}
	
	return (*s->vc_get_device_list)();
}

/* frees device list */
void vc_free_device_list(GList *device_list)
{
	if (device_list) {
		g_list_foreach (device_list, (GFunc)g_free, NULL);
		g_list_free (device_list);
	}
	
	/*g_list_foreach(list, (GFunc)g_free(), NULL); g_list_free(list);*/
}


void
vc_free_choices(GList* choices)
{
	GList*	item;
	volchoice_t* choice;
	item = choices;
	while (item) {
	  choice = (volchoice_t*) item->data;
	  if (choice->displayname) {
	    g_free (choice->displayname);
	    choice->displayname = NULL;
	  }
	  
	  if (choice->name) {
	    g_free (choice->name);
	    choice->name = NULL;
	  }
	  
	  item = g_list_next (item);
	}
	
	g_list_free (choices);
}

void vc_free_control_list(GList *g)
{
	GList 			*item;
	volcontrol_t		*c;
	
	item = g;
	while (item) {
		c = (volcontrol_t*) item->data;
		
		if (c) {
			if (c->choices) {
			  vc_free_choices (c->choices);
			  c->choices = NULL;
			}
			
			if (c->name) g_free (c->name);
			g_free (c);
		}
	
		item = g_list_next (item);
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

volchoice_t* vc_choice_dup(const volchoice_t *choice)
{
  volchoice_t* nchoice;
  nchoice = g_new0 (volchoice_t, 1);
  nchoice->name = g_strdup (choice->name); /* no name -> boom */
  nchoice->displayname = g_strdup (choice->displayname); /* no displayname -> boom */
  return nchoice;
}

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

/* some soundcards are rumoured to support multiple recording sources at once. 
   As I have no clue what use that should ever be, xfce4-mixer does not support that.
   One recording source to rule them all. */
   
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

#include <libxfce4util/libxfce4util.h>

#define VC_PLUGIN
#include "vc.h"

#define MAX_AMP 100

#define NAME_RECSELECTOR "RecSelect"

static char dev_name[PATH_MAX] = { "/dev/mixer" };

static int mixer_handle = -1;
static int devmask = 0;                /* Bitmask for supported mixer devices */
static int avail_recmask = 0;  /* available recording devices */
static int master_i = -1;
static int has_recselector = 0;

static char *label[SOUND_MIXER_NRDEVICES] = SOUND_DEVICE_LABELS;

static void
find_master(void)
{
	int i;
	
	/* TODO: cute error message
	if (mixer_handle == -1) {
		g_warning (_("No Volume Control found (oss)."));
		return;
	}
	*/
	
	g_return_if_fail(mixer_handle != -1);

	devmask = 0;
	master_i = -1;
	avail_recmask = 0;
	has_recselector = 0;

	if (ioctl(mixer_handle, SOUND_MIXER_READ_DEVMASK, &devmask) == -1) {
		perror("oss: Unable to get mixer device mask");
		/*(void)close(mixer_handle);
		mixer_handle = -1;*/
		return;
	}

	if (ioctl(mixer_handle, SOUND_MIXER_READ_RECMASK, &avail_recmask) == -1) {
	   // nevermind
		perror("oss: Unable to get possible recording channels");
		/*(void)close(mixer_handle);
		mixer_handle = -1;*/
		/*return;*/
	} else {
	  has_recselector = 1;
	}
	
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (devmask & (1 << i)) {
			/* if in doubt, choose the first */
			if (master_i == -1)
				master_i = i;

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
		g_warning(_("oss: No master volume"));
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
			if (!strcmp (label[i], name) ||
				(
					!strncmp (label[i], name, strlen (name))
				&&
					label[i][strlen (name)] == ' '
				)
			) {
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

/* reads recmask, returns choices list for recording device select control */
static GList* 
oss_recmask_to_choices(void)
{ 
	GList* res;
	int i;
	gchar* s;
	volchoice_t*	choice;
  
	res = NULL;
  
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (avail_recmask & (1 << i)) {
			s = g_strdup(label[i]);
			g_strchomp (s);
			
			choice = g_new0 (volchoice_t, 1);
			choice->name = s;
			choice->displayname = g_strdup (s);
		  
			res = g_list_append (res, choice);
		}
	}
  
	return res;
}

/* returns list of volcontrol_t */
static GList *
vc_get_control_list(void)
{
	int 	i;
	volcontrol_t *c;

	GList *	g;
	/*gchar *ss;*/
	gchar *s;
	
	g = NULL;
		
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if (devmask & (1 << i)) {
			c = g_new0 (volcontrol_t, 1);
			if (c) {
				s = g_strdup(label[i]);
				/* trim */
				g_strchomp (s);
				/*if (ss != s) {
					g_free (s);
					s = ss;
				}*/
				/*while (s && s[0] && s[strlen(s) - 1] == ' ') {
					s[strlen(s) - 1] = 0;
				}*/
				
				c->name = s;
			
				g = g_list_append(g, c);
			}

		}
	}

        if (has_recselector) {
          c = g_new0 (volcontrol_t, 1);
          c->type = CT_SELECT;
          c->choices = oss_recmask_to_choices();
          c->name = g_strdup (NAME_RECSELECTOR);
          g = g_list_append (g, c);
        }
        
	return g;	
}

static void vc_set_volume_callback(volchanger_callback_t cb, void *data)
{
	/* unsupported */
}

static void vc_close_device()
{
	master_i = -1;
	if (mixer_handle != -1) {
		close (mixer_handle);
		mixer_handle = -1;
	}
}

#define SUPPORT_LINK /* TODO: automake() this */
#ifdef SUPPORT_LINK
static gboolean my_file_is_link(gchar *filename)
{
	return g_file_test (filename, G_FILE_TEST_IS_SYMLINK);
}
#else
static gboolean my_file_is_link(gchar *filename)
{
	return FALSE;
}
#endif

static void scan_dir_for_mixers(GList **l, gchar const *base_dir)
{
	GDir *dir;
	gchar const *d;
	gchar *file;
	dir = g_dir_open (base_dir, 0, NULL);

	if (dir) {
		while ((d = g_dir_read_name (dir))) {
			if (!strncmp(d, "mixer", 5)) {
				file = g_strdup_printf ("%s/%s", base_dir, d);
				if (file) {
					if (!my_file_is_link (file)) {
						*l = g_list_append (*l, file);
					} else {
						g_free (file);
					}
				}
			}
		}
		
		g_dir_close (dir);
	}

}


static GList *vc_get_device_list()
{
	GList *l;
	
	l = NULL;
	
	scan_dir_for_mixers (&l, "/dev/sound");
	scan_dir_for_mixers (&l, "/dev");
	
	return l;
}

static void vc_set_select(char const *which, gchar const *v)
{
	int	i;
	int	recsrc;
	int	xrecsrc;
	
	if (has_recselector && g_str_equal (which, NAME_RECSELECTOR)) {
		i = find_control (v);
		if (i == -1) {
		  g_warning ("oss: could not find control that the new recording source refers to. Not setting it.");
		  return;
		}
		
		recsrc = 1 << i;
		 
		//SOUND_MIXER_READ_RECSRC
		if (ioctl(mixer_handle, SOUND_MIXER_WRITE_RECSRC, &recsrc) == -1) {
			perror("oss: Unable to set mixer recording source");
			return;
		}
		
		if (ioctl(mixer_handle, SOUND_MIXER_READ_RECSRC, &xrecsrc) == -1) {
			perror("oss: Unable to get mixer recording source back");  
			return;
		}
		
		if (xrecsrc != recsrc) {
			g_warning ("oss: sound card driver messed with the recording source given. Thus, it is not guaranteed that the correct one is set now.");
			return;
		}

		return;
	}
} 

static gchar *vc_get_select(char const *which)
{
	int	recsrc;
	gchar*	s;
	int	i;
	
	if (has_recselector && g_str_equal (which, NAME_RECSELECTOR)) {
		if (ioctl(mixer_handle, SOUND_MIXER_READ_RECSRC, &recsrc) == -1) {
			perror("oss: Unable to get mixer recording source");  
			return NULL;
		}
		
		for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
			if (recsrc & (1 << i)) {
				s = g_strdup (label[i]);
				g_strchomp (s);
				return s;
			}
		}
		
		/*
		g_warning ("oss: could not get associated recording source name. BAD!");
		
		if (master_i == -1) {
			g_warning ("oss: no master control found either. Giving up.");
			return NULL;
		}
		
		 this is most likely the wrong answer but better than crashing
		s = g_strdup (label[master_i]);
		g_strchomp (s);
		return s;
		*/
		
	}
	
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
	return dev_name;
}

static void vc_handle_events()
{
}
                
REGISTER_VC_PLUGIN(oss);

#endif	/* !USE_OSS */


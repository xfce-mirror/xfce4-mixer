/* 
 * A small mixer plugin for the panel.
 *
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

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*#define TEST_DEVICE_ENTRY*/

#include <libxfce4util/debug.h>
#include <libxfce4util/i18n.h>
#include <libxfcegui4/xfce_iconbutton.h>

#include <panel/global.h>
#include <panel/controls.h>
#include <panel/icons.h>
#include <panel/plugins.h>
#include <panel/xfce_support.h>
#include <panel/item_dialog.h>

#include "vc.h"
#include "mixer_window.h"
#include "mvisible_opts.h"

/* for xml: */
#define MIXER_ROOT "Mixer"

static GtkTooltips *tooltips = NULL;

/*  Mixer module
 *  ------------
*/
typedef struct
{
    char		*command;
    char		*device;
    gboolean		use_sn;
    gboolean		use_terminal;
    gboolean		use_internal;
/*    GList		*visible_ctrls;*/
    GList		*l_visible;
} MixerOptions;

typedef struct
{
    mixer_window_t	*mw;
    GtkWidget		*eventbox;
    GtkBox		*hbox;
    GtkWidget		*mixer;           /* our XfceMixer widget */
    GtkProgressBar	*status;
    int			timeout_id;
    gboolean		broken;
    
    /* cache */
    int			c_volume;

    /* settings */
    MixerOptions	options;
    GtkContainer	*settings_c; /* only a link */
    GtkSizeGroup	*sg; /* only a link */
    GtkScrolledWindow	*s_visible;
    mvisible_opts_t	*t_visible;
    
    int			settings_action; /* < 0: get; > 0: set control-used data */
    GtkWidget		*dialog; /* settings dialog, only a link */
        
}
t_mixer;

/* get translation from the panel's domain */
static gchar *
P_(gchar *s)
{
	return(dgettext("xfce4-panel", s));
}

static int find_working_sound(void)
{
#ifdef USE_OSS
	extern int register_oss(void);
#endif
#ifdef USE_ALSA
	extern int register_alsa(void);
	extern int register_alsa_fallback(void);
#endif

	volchanger_t	**v;

	/* register desired sound system */
#ifdef USE_OSS
	register_oss();
#endif
#ifdef USE_ALSA
	register_alsa();
#if 0
	register_alsa_fallback();
#endif
#endif

	v = first_vc();
	while (v) {
		if ((*v)->vc_reinit_device && (*(*v)->vc_reinit_device)() == 0) {
			/* works */
			select_vc_direct(*v);
			return 0;
		}
		v = next_vc(v);
	}

	g_warning(_("No working sound"));
	
	return -1;
}

/* utility functions */

static void 
swap_pixbuf_ptrs (GdkPixbuf **a, GdkPixbuf **b)
{
	GdkPixbuf *tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

static GdkPixbuf *
xfce_mixer_get_pixbuf_for(gboolean broken)
{
	GdkPixbuf	*pb;
	GdkPixbuf	*pb2;
	
	pb = get_pixbuf_by_id(SOUND_ICON);
	if (broken) {
		pb2 = gdk_pixbuf_copy(pb);
		gdk_pixbuf_saturate_and_pixelate(pb, pb2, 0, TRUE);

		/*saturation, pixelate)*/
		swap_pixbuf_ptrs(&pb, &pb2);

		g_object_unref(pb2);
	}
	return pb;
}

/* creation and destruction */

static GtkWidget *
xfce_mixer_new(gboolean *broken)
{
	XfceIconbutton *ib;
	GdkPixbuf *pb;
	
	*broken = FALSE;
	if (find_working_sound() == -1) {
	/*}{*/
		*broken = TRUE;
	}

	pb = xfce_mixer_get_pixbuf_for(*broken);

	ib = (XfceIconbutton *)xfce_iconbutton_new_from_pixbuf(pb);
	g_object_unref(pb);

	if (ib) {	
		gtk_button_set_relief(GTK_BUTTON(ib), GTK_RELIEF_NONE);
	}
	return (GtkWidget *)ib;
}

static void
mixer_do_options(t_mixer *mixer, int mode); /* 0: load; 1: store; 2: connect signals; 3: sensitivize/desensitivize */

static void
mixer_apply_options_cb(GtkWidget *button, t_mixer *mixer); /* verified: clicked */

static void use_internal_changed_cb(t_mixer *m)
{
	mixer_do_options(m, 3);
}

static t_mixer *
mixer_new (void)
{
    GtkRcStyle	*rc;
    GdkColor	color;
        
    t_mixer *mixer;
    
    mixer = g_new0 (t_mixer, 1);

    mixer->mw = NULL;
    mixer->broken = TRUE;
    
    mixer->options.command = NULL;
    mixer->options.device = NULL;
    mixer->options.use_sn = TRUE;
    mixer->options.use_internal = TRUE;
    mixer->options.use_terminal = FALSE;

    
    mixer->hbox = GTK_BOX(gtk_hbox_new(FALSE, 0));
    gtk_widget_set_name (GTK_WIDGET(mixer->hbox), "xfce_mixer");
    gtk_container_set_border_width(GTK_CONTAINER(mixer->hbox), border_width);

    gtk_widget_show(GTK_WIDGET(mixer->hbox));

    mixer->mixer = xfce_mixer_new (&mixer->broken);
    gtk_widget_show (mixer->mixer);
    gtk_box_pack_start (GTK_BOX(mixer->hbox), GTK_WIDGET(mixer->mixer), FALSE, FALSE, 0);

    mixer->status = GTK_PROGRESS_BAR(gtk_progress_bar_new());
    gtk_progress_bar_set_orientation(mixer->status, GTK_PROGRESS_BOTTOM_TO_TOP);
    
    rc = gtk_widget_get_modifier_style(GTK_WIDGET(mixer->status));
    if (!rc) { rc = gtk_rc_style_new(); }
    
    gdk_color_parse("#00c000", &color);
    
    if (rc) {
    	rc->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG; 
        rc->bg[GTK_STATE_PRELIGHT] = color;
    }
   
    gtk_widget_modify_style(GTK_WIDGET(mixer->status), rc); /* gone afterwards */
    
    gtk_widget_show(GTK_WIDGET(mixer->status));

    mixer->eventbox = gtk_event_box_new ();
    gtk_widget_show (mixer->eventbox);
    
    gtk_container_add (GTK_CONTAINER (mixer->eventbox),
		    GTK_WIDGET(mixer->status));

    gtk_box_pack_start (GTK_BOX(mixer->hbox), GTK_WIDGET(mixer->eventbox),
		    FALSE, FALSE, 0);

    mixer->options.l_visible = vc_get_control_list ();
    
    use_internal_changed_cb(mixer);
    
    return mixer;
}

static gboolean
update_volume_display(t_mixer *mixer)
{
	gchar caption[128];

	g_snprintf(caption, sizeof(caption), _("Volume: %d%%"),
			mixer->c_volume);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->hbox), caption,
			NULL);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->mixer), caption,
			NULL);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->eventbox), caption,
			NULL);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(mixer->status),
			mixer->c_volume / 100.0);
	
	return(FALSE);
}

static gboolean
check_volume (t_mixer *mixer)
{
	int volume;

	if (mixer->broken) {
		return TRUE; /* this is just in case */
	}
	
	/*	return FALSE;*/ /* stop */
	
	volume = vc_get_volume(NULL);
	if (volume != mixer->c_volume) {
		mixer->c_volume = volume; /* atomic? lol */
		update_volume_display(mixer);
	}

	/* keep timeout running */
	return(TRUE);
}

static gboolean
xfce_mixer_scroll_cb (GtkWidget *widget, GdkEventScroll *event, t_mixer *mixer) /* verified prototype */
{
	int	vol;	/* new volume */
	int	ovol;	/* old volume */ 
	int	tvol;	/* test volume */
	int	tries;
	
	vol = vc_get_volume(NULL);
	ovol = vol;

	if (event->type != GDK_SCROLL) {
		return FALSE;
	}
	
	tries = 0;
	do {
		if (event->direction == GDK_SCROLL_DOWN) {
			vol -= 7;
			if (vol < 0) { vol = 0; }
		} else if (event->direction == GDK_SCROLL_UP) {
			vol += 7;
			if (vol > 100) { vol = 100; }
		}
		if (ovol != vol) {
			vc_set_volume(NULL, vol);
		
			tvol = vc_get_volume(NULL);
			if (ovol != tvol) { /* worked. */
				break;
			}
		} else break; /* ouch */
		
		++tries;
	} while (ovol == vol && vol > 0 && vol < 100 && tries < 3); /* if still unchanged, loop */

	tvol = vc_get_volume(NULL);
	mixer->c_volume = tvol;

	update_volume_display(mixer);
	return TRUE;
}


static gboolean
xfce_mixer_status_button_cb (GtkWidget * widget, GdkEventButton *b, t_mixer *mixer) /* verified prototype */
{
	int	y; /* pos */
	int	sy; /* size */

	y = (int)b->y;

	if (b->button == 3 || b->button == 2) {
		if (b->type == GDK_BUTTON_PRESS) {
			y = 0;
		} else {
			return TRUE;
		}
	} else if (b->button == 1) {
		sy = widget->allocation.height;
		if (sy != 0) {
			/* this is a hack 'cause I dont know how to get the height 
			   of the border of the progressbar yet ;) */
			y = (sy + 2 - y) * 100 / sy;
			if (y <= 0) y = 0;
		} else y = 0;
	} else return TRUE;
	
	vc_set_volume(NULL, y);
		
	mixer->c_volume = vc_get_volume(NULL);
	update_volume_display(mixer);

	return TRUE;
}

static void
xfce_mixer_window_destroy_cb(GtkWindow *w, t_mixer *m)
{
	if ((GtkWindow *)(m->mw->window) == w) m->mw = NULL;
}

static void
xfce_mixer_launch_button_cb(GtkWidget *button, t_mixer *mixer) /* verified prototype: clicked */
{
	if (!mixer->options.command || !mixer->options.command[0] || mixer->options.use_internal) {
		/* empty: use internal */

		if (!mixer->mw && mixer->options.l_visible) {
			mixer->mw = mixer_window_new (TRUE, mixer->options.l_visible);
			g_signal_connect(GTK_WIDGET (mixer->mw->window), "destroy", G_CALLBACK(xfce_mixer_window_destroy_cb), mixer);
			gtk_widget_show (GTK_WIDGET (mixer->mw->window));
		}
		
		if (mixer->mw) {
			gtk_window_present (GTK_WINDOW (mixer->mw->window));
		}
	} else {
		exec_cmd(mixer->options.command, mixer->options.use_terminal,
			mixer->options.use_sn);
	}
}


static void
mixer_set_theme(Control * control, const char *theme)
{
	GdkPixbuf	*pb;
	t_mixer		*mixer;
	
	mixer = (t_mixer *)control->data;
	
	pb = xfce_mixer_get_pixbuf_for(mixer->broken);
	xfce_iconbutton_set_pixbuf(XFCE_ICONBUTTON(mixer->mixer), pb);
	g_object_unref(pb);
}


static void
free_optionsdialog(t_mixer *mixer)
{
	if (mixer->options.command) {
		g_free(mixer->options.command); 
		mixer->options.command = NULL;
	}

	if (mixer->options.device) {
	       g_free(mixer->options.device);
	       mixer->options.device = NULL;
	}
}

static void
mixer_free (Control * control)
{
    t_mixer *mixer = control->data;

    g_return_if_fail(mixer != NULL);
    
    if (mixer->timeout_id != 0) {
	    g_source_remove(mixer->timeout_id);
	    mixer->timeout_id = 0;
    }
    
    free_optionsdialog(mixer);

    if (mixer->options.l_visible) {
        vc_free_control_list (mixer->options.l_visible);
        mixer->options.l_visible = NULL;
    }

    g_free(mixer);
    
    vc_close_device ();
}

static void
mixer_attach_callback (Control * control, const char *signal,
                       GCallback callback, gpointer data)
{
    t_mixer *mixer = control->data;

    g_signal_connect (mixer->mixer, signal, callback, data);
}


G_MODULE_EXPORT void
mixer_set_size (Control *control, int size)
{
	t_mixer	*mixer = (t_mixer *)control->data;
	
	/* size: 0..3(HUGE), with 4- taken as huge too */

	gtk_widget_set_size_request(GTK_WIDGET(mixer->mixer), icon_size[size], icon_size[size]);
	gtk_widget_set_size_request(GTK_WIDGET(mixer->status), 6 + 2 * size, icon_size[size]);

	gtk_widget_queue_resize (GTK_WIDGET (mixer->status));
}


/*  Mixer panel control
 *  -------------------
*/
gboolean
create_mixer_control (Control * control)
{
	t_mixer *mixer = NULL;
	
	if (!tooltips) {
		tooltips = gtk_tooltips_new();
	}
	
	mixer = mixer_new ();
	gtk_container_add (GTK_CONTAINER (control->base), GTK_WIDGET(mixer->hbox));

	control->data = (gpointer) mixer;
	control->with_popup = FALSE;

	gtk_widget_set_size_request (control->base, -1, -1);

	g_signal_connect(mixer->eventbox, "scroll-event", G_CALLBACK(xfce_mixer_scroll_cb), mixer);

	g_signal_connect(mixer->hbox, "scroll-event", G_CALLBACK(xfce_mixer_scroll_cb), mixer);
	g_signal_connect(mixer->eventbox, "button-press-event", G_CALLBACK(xfce_mixer_status_button_cb), mixer);
	g_signal_connect(mixer->eventbox, "button-release-event", G_CALLBACK(xfce_mixer_status_button_cb), mixer);
	
	g_signal_connect(mixer->mixer, "clicked", G_CALLBACK(xfce_mixer_launch_button_cb), mixer);

	mixer->timeout_id = 0;
	if (!mixer->broken) {
		check_volume (mixer);
		/* timeout_id==0: failed */
		mixer->timeout_id = g_timeout_add(5 * 100, (GSourceFunc) check_volume, mixer);
	}

	return TRUE;
}

static GtkWidget *
mixer_options_get(GtkContainer *c, int index)
{
	GList	*list;
	GList	*iter;
	int	pos = 0;
	GtkWidget *w = NULL;
	
	if (!c) {
		return NULL;
	}
	
	list = gtk_container_get_children(GTK_CONTAINER(c));
	if (!list) {
		return NULL;
	}
	
	w = GTK_WIDGET(list->data);
	for(iter = list; pos <= index && iter; iter = g_list_next(iter), pos++) {
		w = GTK_WIDGET(iter->data);
	}
	g_list_free(list);
	return w;
}

static void 
mixer_stuff_toggled_cb(GtkToggleButton *tb, t_mixer *mixer) /* verified prototype: toggled */
{
	use_internal_changed_cb (mixer);
}
 
#if 0
static gboolean
mixer_command_entry_lost_focus_cb(GtkWidget *w, GdkEvent *event, t_mixer *mixer) /* verified prototype: focus-out-event */
{
	return TRUE;
	/* FALSE;*/ /* needed? */
}
#endif

static gboolean
mixer_device_entry_lost_focus_cb(GtkWidget *w, GdkEvent *event, t_mixer *mixer) /* verified prototype: focus-out-event */
{
	if (mixer->options.device) {
		mixer_apply_options_cb(NULL, mixer);
		vc_set_device (mixer->options.device);
	}
	return FALSE;
	/* FALSE;*/ /* needed? */
}

static void
mixer_do_options(t_mixer *mixer, int mode) /* 0: load; 1: store; 2: connect signals; 3: sensitivize/desensitivize */
{
	char const *temp;
	GtkContainer *c;
	GtkContainer *h1; /* hbox for entry */
	GtkContainer *h2; /* hbox for device */
	GtkContainer *h3; /* hbox for vbox2 */
	GtkContainer *v2; /* vbox for use_sn, use_terminal */
	GtkEntry	*e_command = NULL;	
	GtkEntry	*e_device = NULL;
	GtkCheckButton	*b_use_sn = NULL;
	GtkCheckButton	*b_use_term = NULL;
	GtkCheckButton	*b_use_internal = NULL;
	GtkButton	*b_dotdotdot = NULL;
	GtkTreeView	*tv = NULL;
	GList		*g;
	GList		*go;
	GList		*gn;
	volchanger_t	*vc;
	c = mixer->settings_c; /* vbox 1 */

	if (c) {
		h1 = GTK_CONTAINER(mixer_options_get(c, 0));
		e_command = GTK_ENTRY(mixer_options_get(GTK_CONTAINER(h1), 1));
		b_dotdotdot = GTK_BUTTON(mixer_options_get(GTK_CONTAINER(h1), 2));
		h2 = GTK_CONTAINER(mixer_options_get(c, 1));
		e_device = GTK_ENTRY(mixer_options_get(GTK_CONTAINER(h2), 1));
		h3 = GTK_CONTAINER(mixer_options_get(c, 2));
		v2 = GTK_CONTAINER(mixer_options_get(h3, 1));
		
		b_use_term = GTK_CHECK_BUTTON(mixer_options_get(v2, 0));
		b_use_sn = GTK_CHECK_BUTTON(mixer_options_get(v2, 1));
		b_use_internal = GTK_CHECK_BUTTON(mixer_options_get(v2, 2));
		
		tv = GTK_TREE_VIEW (mixer_options_get (
			GTK_CONTAINER(mixer_options_get (
				c,
			2)),
		0));
	}
	if (b_use_internal) {

		switch (mode) {
		case 1:
			mixer->options.use_internal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b_use_internal));
			break;
			
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b_use_internal), mixer->options.use_internal);
			break;
			
		case 2:
			g_signal_connect(GTK_WIDGET(b_use_internal), "toggled", G_CALLBACK(mixer_stuff_toggled_cb), mixer);
			break;
			
		}

		mixer->options.use_internal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b_use_internal));
	}

	if (mixer->t_visible) {
		switch (mode) {
		case 1:
			go = mvisible_opts_get_actives (mixer->t_visible);
			g = go;
			gn = NULL;
			while (g) {
				vc = g_new0 (volchanger_t, 1);
				vc->name = g_strdup ((gchar *)g->data);
				gn = g_list_append (gn, vc);
				g = g_list_next (g);
			}
			mvisible_opts_free_actives (go);
			mixer->options.l_visible = gn;
			break;
		case 0:
			go = mixer->options.l_visible;
			gn = NULL;
			g = go;
			
			while (g) {
				vc = (volchanger_t *)g->data;
				
				gn = g_list_append (gn, g_strdup (vc->name));
				g = g_list_next (g);
			}
				
			if (gn) {
				mvisible_opts_set_actives (mixer->t_visible, gn);
				mvisible_opts_free_actives (gn);
			}
			break;
		case 3:
			if (mixer->options.use_internal) {
				gtk_widget_show (GTK_WIDGET (mixer->s_visible));
			} else {
				gtk_widget_hide (GTK_WIDGET (mixer->s_visible));
			}
			break;
		}
	}
	
		
	if (e_command) {
		switch (mode) {
		case 1:
			temp = gtk_entry_get_text(GTK_ENTRY(e_command));
			if (temp && *temp) {
				mixer->options.command = g_strdup(temp);
			}
			break;
		case 0:
			if (mixer->options.command) {
				gtk_entry_set_text(GTK_ENTRY(e_command), g_strdup(mixer->options.command));
			}
			break;
		case 2:
			/*g_signal_connect (e_command, "insert-at-cursor", G_CALLBACK (mixer_revert_make_sensitive_cb), mixer->revert_b);
			g_signal_connect_swapped (e_command, "delete-from-cursor", G_CALLBACK (mixer_revert_make_sensitive_cb), mixer->revert_b);
			*/
			/*g_signal_connect (e_command, "focus-out-event", G_CALLBACK(mixer_command_entry_lost_focus_cb), mixer);*/
			break;
			
		case 3:
			gtk_widget_set_sensitive (GTK_WIDGET (e_command), !mixer->options.use_internal);
			break;
		}
	}
	if (e_device) {
		switch (mode) {
		case 1:
			temp = gtk_entry_get_text(GTK_ENTRY(e_device));
			if (temp && *temp) {
				mixer->options.device = g_strdup(temp);
			}
			break;
		case 0:
			if (mixer->options.device) {
				gtk_entry_set_text(GTK_ENTRY(e_device), g_strdup(mixer->options.device));
			}
			break;
		case 2:
			g_signal_connect (e_device, "focus-out-event", G_CALLBACK(mixer_device_entry_lost_focus_cb), mixer);
			break;
			
		case 3:
			/* maybe we could add a 'use default' device but only maybe!
			gtk_widget_set_sensitive (GTK_WIDGET (e_device), !mixer->options.use_default);
			*/
			break;
		}
	}
	
	if (b_use_sn) {
		switch (mode) {
		case 1:
			mixer->options.use_sn = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b_use_sn));
			break;
			
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b_use_sn), mixer->options.use_sn);
			break;
			
		case 2:
			g_signal_connect(GTK_WIDGET(b_use_sn), "toggled", G_CALLBACK(mixer_stuff_toggled_cb), mixer);
			break;
		case 3:
			gtk_widget_set_sensitive (GTK_WIDGET (b_use_sn), !mixer->options.use_internal);
			break;
		
		}
	}

	if (b_use_term) {
		switch (mode) {
		case 1:
			mixer->options.use_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b_use_term));
			break;
			
		case 0:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(b_use_term), mixer->options.use_terminal);
			break;
			
		case 2:
			g_signal_connect(GTK_WIDGET(b_use_term), "toggled", G_CALLBACK(mixer_stuff_toggled_cb), mixer);
			break;
			
		case 3:
			gtk_widget_set_sensitive (GTK_WIDGET (b_use_term), !mixer->options.use_internal);
			break;
		
		}
	}
}

static void
mixer_apply_options_cb(GtkWidget *button, t_mixer *mixer) /* verified: clicked */
{

	if (mixer->options.device) g_free(mixer->options.device);
	mixer->options.device = NULL;

	if (mixer->options.command) g_free(mixer->options.command);
	mixer->options.command = NULL;
	
	vc_free_control_list (mixer->options.l_visible);
	mixer->options.l_visible = NULL;

	mixer_do_options(mixer, 1);
}	

static void
mixer_fill_options(t_mixer *mixer)
{
	mixer_do_options(mixer, 0);
	use_internal_changed_cb(mixer);
}

#if 0
static GtkWidget *
my_create_command_option(GtkSizeGroup *sg)
{ /* nasty NASTY side effects ! */
	return create_command_option(w);
}
#else

/*  Change the command 
 *  ------------------
*/
static void
command_browse_cb (GtkWidget * b, GtkEntry * entry)
{
/*	GtkWidget	*h1 = NULL;
	GtkEntry	*entry = NULL;
	h1 = GTK_CONTAINER(mixer_options_get(mixer->settings_c, 0));
	entry = GTK_ENTRY(mixer_options_get(GTK_CONTAINER(h1), 1));
*/	
    char *file =
        select_file_name (P_("Select command"), gtk_entry_get_text (entry),
                          gtk_widget_get_toplevel(GTK_WIDGET(entry)));

    if (file)
    {
        gtk_entry_set_text (entry, file);
        g_free (file);
    }
}

/*  Change the device
 *  TODO: can be merged with command_browse_cb eventually (byT)
 *  ------------------
 */
static void
device_browse_cb (GtkWidget * b, GtkEntry * entry)
{
    char *file =
        select_file_name (P_("Select device"), gtk_entry_get_text (entry),
                          gtk_widget_get_toplevel(GTK_WIDGET(entry)));

    if (file)
    {
        gtk_entry_set_text (entry, file);
        g_free (file);
    }
}

static GtkWidget *
my_create_command_option(GtkSizeGroup *sg)
{
    GtkWidget *vbox;
    GtkWidget *vbox2;
    GtkWidget *hbox;
    GtkWidget *hbox2;
    GtkWidget *hbox3;
    GtkWidget *label;
    GtkWidget *image;
    
    GtkWidget *command_entry = NULL;
    GtkWidget *command_browse_button = NULL;
    GtkWidget *device_entry = NULL;
    GtkWidget *device_browse_button = NULL;
    GtkWidget *term_checkbutton = NULL;
    GtkWidget *sn_checkbutton = NULL;
    GtkWidget *internal_checkbutton = NULL;

    vbox = gtk_vbox_new (FALSE, 8);
    gtk_widget_show (vbox);

    /* entry */
    hbox = gtk_hbox_new (FALSE, 4);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new (P_("Command:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget (sg, label);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    command_entry = gtk_entry_new ();
    gtk_widget_show (command_entry);
    gtk_box_pack_start (GTK_BOX (hbox), command_entry, TRUE, TRUE, 0);

    command_browse_button = gtk_button_new ();
    gtk_widget_show (command_browse_button);
    gtk_box_pack_start (GTK_BOX (hbox), command_browse_button, FALSE, FALSE, 0);

    hbox2 = gtk_hbox_new (FALSE, 4);
#ifdef TEST_DEVICE_ENTRY
    gtk_widget_show (hbox2);
#endif

    gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, TRUE, 0);
    
    label = gtk_label_new (P_("Device:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_size_group_add_widget (sg, label);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

    device_entry = gtk_entry_new ();
    gtk_widget_show (device_entry);
    gtk_box_pack_start (GTK_BOX (hbox2), device_entry, TRUE, TRUE, 0);

    device_browse_button = gtk_button_new ();
    gtk_widget_show (device_browse_button);
    gtk_box_pack_start (GTK_BOX (hbox2), device_browse_button, FALSE, FALSE, 0);
    
    image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image);
    gtk_container_add(GTK_CONTAINER(device_browse_button), image);
    
    image = gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_BUTTON);
    gtk_widget_show(image);
    gtk_container_add(GTK_CONTAINER(command_browse_button), image);
    
    g_signal_connect (command_browse_button, "clicked",
                      G_CALLBACK (command_browse_cb), command_entry);
    g_signal_connect (device_browse_button, "clicked",
                      G_CALLBACK (device_browse_cb), device_entry);

    /* terminal */
    hbox3 = gtk_hbox_new (FALSE, 4);
    gtk_widget_show (hbox3);
    gtk_box_pack_start (GTK_BOX (vbox), hbox3, FALSE, TRUE, 0);

    label = gtk_label_new ("");
    gtk_size_group_add_widget (sg, label);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox2);
    gtk_box_pack_start (GTK_BOX (hbox3), vbox2, FALSE, TRUE, 0);

    term_checkbutton =
        gtk_check_button_new_with_mnemonic (P_("Run in _terminal"));
    gtk_widget_show (term_checkbutton);
    gtk_box_pack_start (GTK_BOX (vbox2), term_checkbutton, FALSE, FALSE, 0);

    sn_checkbutton =
        gtk_check_button_new_with_mnemonic (P_("Use startup _notification"));
    gtk_widget_show (sn_checkbutton);
    gtk_box_pack_start (GTK_BOX (vbox2), sn_checkbutton, FALSE, FALSE, 0);
#ifdef HAVE_LIBSTARTUP_NOTIFICATION
    gtk_widget_set_sensitive (sn_checkbutton, TRUE);
#else
    gtk_widget_set_sensitive (sn_checkbutton, FALSE);
#endif

    internal_checkbutton =
        gtk_check_button_new_with_mnemonic (_("Use _internal mixer"));
        
    gtk_widget_show (internal_checkbutton);
    gtk_box_pack_start (GTK_BOX (vbox2), internal_checkbutton, FALSE, FALSE, 0);

    return vbox;
}

#endif
static void
mixer_create_options(Control *control, GtkContainer *container, GtkWidget *done)
{
	t_mixer		*mixer;
	GtkWidget 	*vbox;
	GList		*names = NULL;
	GList		*src = NULL;
	GList		*snode = NULL;
	volchanger_t	*vc = NULL;
	
	mixer = (t_mixer *)control->data;

	mixer->dialog = gtk_widget_get_toplevel(done);
	
	mixer->sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = my_create_command_option(mixer->sg);
	gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(vbox));
	mixer->settings_c = GTK_CONTAINER(vbox);
	
	mixer->s_visible = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mixer->s_visible), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (GTK_WIDGET (mixer->s_visible));
	
	mixer->t_visible = mvisible_opts_new ();

	gtk_widget_set_size_request (GTK_WIDGET (mixer->s_visible), -1, 100);
	
	src = vc_get_control_list ();
	
	snode = src;
	names = NULL;
	while (snode) {
		vc = (volchanger_t *)snode->data;
		
		names = g_list_append (names, vc->name);
		snode = g_list_next (snode);
	}
	/*mixer->options.l_visible = names;*/
	
	mvisible_opts_fill (GTK_WIDGET (mixer->s_visible), mixer->t_visible, names);
	g_list_free (names);
	vc_free_control_list (src);
	
	/*gtk_box_pack_start (GTK_BOX(mixer->s_visible), GTK_WIDGET (mixer->t_visible->tv), 
		FALSE, TRUE, 0);*/

	gtk_container_add (GTK_CONTAINER (mixer->settings_c), GTK_WIDGET (mixer->s_visible));
	
	mixer_fill_options(mixer);
	mixer_do_options(mixer, 2);
/*	create_options_backup(mixer);*/
	
	g_signal_connect(GTK_WIDGET(mixer->dialog), "destroy-event", G_CALLBACK(free_optionsdialog), mixer);
	g_signal_connect(GTK_WIDGET(done), "clicked", G_CALLBACK(mixer_apply_options_cb), mixer);
}

extern xmlDocPtr xmlconfig;
#define MYDATA(node) xmlNodeListGetString(xmlconfig, node->children, 1)

static void
mixer_read_config(Control *control, xmlNodePtr node)
{
	int	n;
	t_mixer	*mixer;
	xmlChar	*value;
	xmlNodePtr node2;
	GList	*g;
	volchanger_t	*vc;

	mixer = (t_mixer *)control->data;
	
	if (!node || !node->children) 
		return;
		
	node = node->children;
	
	if (!xmlStrEqual(node->name, (const xmlChar *)MIXER_ROOT))
		return;
		
	value = xmlGetProp (node, "device");
	if (value) {
		mixer->options.device = g_strdup (value);
		g_free (value);
		TRACE("read device: '%s'", mixer->options.device);
	} else {
		TRACE("no device found in: '%s' :(", node->name);
	}
	
	for (node = node->children; node; node = node->next) {
		if (xmlStrEqual (node->name, (const xmlChar *)"Command")) {
			value = MYDATA (node);
			if (value) {
				if (mixer->options.command) g_free (mixer->options.command);
				mixer->options.command = (char *)value;
			}

			value = xmlGetProp (node, "term");
			if (value) {
				n = atoi (value);
				mixer->options.use_terminal = (n == 1) ? TRUE : FALSE;
				g_free(value);
			}

			value = xmlGetProp (node, "sn");
			if (value) {
				n = atoi (value);
				mixer->options.use_sn = (n == 1) ? TRUE: FALSE;
				g_free(value);
			}

			value = xmlGetProp (node, "internal");
			if (value) {
				n = atoi (value);
				mixer->options.use_internal = (n == 1) ? TRUE: FALSE;
				g_free(value);
			}
		} else 	if (xmlStrEqual (node->name, (const xmlChar *)"Controls")) {
			g = NULL;
			for (node2 = node->children; node2; node2 = node2->next) {
				if (xmlStrEqual (node2->name, (const xmlChar *)"Control")) {
					vc = g_new0 (volchanger_t, 1);
					vc->name = (gchar *)MYDATA(node2);
					g = g_list_append (g, vc);
				}
			}
			
			if (mixer->options.l_visible) {
				vc_free_control_list (mixer->options.l_visible);
				mixer->options.l_visible = NULL;
			}
			mixer->options.l_visible = g;
		}	
	}
}

static void
mixer_write_config(Control *control, xmlNodePtr parent)
{
	xmlNodePtr root, node, node2;
	char value[MAXSTRLEN + 1];
	GList *g;
	volchanger_t	*vc;

	t_mixer *mixer = (t_mixer *) control->data;

	g_return_if_fail(mixer != NULL);

	root = xmlNewTextChild (parent, NULL, MIXER_ROOT, NULL);
        
	if (mixer->options.device) {
	        xmlSetProp (root, "device", g_strdup(mixer->options.device));
	}
	    
	
	node = xmlNewTextChild (root, NULL, "Controls", NULL);
	if (mixer->options.l_visible) {
		g = mixer->options.l_visible;
		while (g) {
			vc = (volchanger_t *)g->data;
			node2 = xmlNewTextChild (node, NULL, "Control", (const xmlChar *)vc->name);
			xmlSetProp (node2, "id", (const xmlChar *)vc->name);

			g = g_list_next (g);
		}
	}
	
	node = NULL;

	if (mixer->options.command) {
		node = xmlNewTextChild (root, NULL, "Command", mixer->options.command);
	} else {
		return;
	}

	snprintf (value, 2, "%d", mixer->options.use_terminal);
	xmlSetProp (node, "term", value);
 
	snprintf (value, 2, "%d", mixer->options.use_sn);
	xmlSetProp (node, "sn", value);

	snprintf (value, 2, "%d", mixer->options.use_internal);
	xmlSetProp (node, "internal", value);
}

G_MODULE_EXPORT void
xfce_control_class_init (ControlClass * cc)
{
#if 0
#ifdef ENABLE_NLS
    /* This is required for UTF-8 at least - Please don't remove it */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
    textdomain (GETTEXT_PACKAGE);
#endif
#else
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
#endif

    cc->name = "mixer";
    cc->caption = _("Volume Control");

    cc->create_control = (CreateControlFunc) create_mixer_control;

    cc->free = mixer_free;
    cc->read_config = mixer_read_config;
    cc->write_config = mixer_write_config;
    cc->attach_callback = mixer_attach_callback;
    
    cc->create_options = (gpointer) mixer_create_options;

    cc->set_theme = mixer_set_theme;

    cc->set_size = mixer_set_size;
}

/* macro defined in plugins.h */
XFCE_PLUGIN_CHECK_INIT

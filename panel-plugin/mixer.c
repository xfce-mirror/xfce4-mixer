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

#include <libxfce4util/i18n.h>
#include <libxfcegui4/xfce_iconbutton.h>

#include <panel/global.h>
#include <panel/controls.h>
#include <panel/icons.h>
#include <panel/plugins.h>
#include <panel/xfce_support.h>
#include <panel/item_dialog.h>
/*#include <libxml/xml.h>*/
/*#include <panel/settings.h>*/

#include "vc.h"

/*#define BORDER 6*/
/* for xml: */
#define MIXER_ROOT "Mixer"

static GtkTooltips *tooltips = NULL;

/*  Mixer module
 *  ------------
*/
typedef struct
{
    char		*command;
    gboolean		use_sn;
    gboolean		use_terminal;
} MixerOptions;

typedef struct
{
    GtkWidget		*eventbox;
    GtkBox		*hbox;
    GtkWidget		*mixer;           /* our XfceClock widget */
    GtkProgressBar	*status;
    int			timeout_id;
    gboolean		broken;
    
    /* cache */
    int			c_volume;

    /* settings */
    MixerOptions	options;
    MixerOptions	revert;
    GtkContainer	*settings_c;
    GtkSizeGroup	*sg;
    GtkWidget		*revert_b; /* button */
    
    int			settings_action; /* < 0: get; > 0: set control-used data */
    GtkWidget		*dialog; /* settings dialog */
        
}
t_mixer;

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
	register_alsa_fallback();
#endif

	v = first_vc();
	while (v) {
		if ((*v)->reinit_device && (*(*v)->reinit_device)() == 0) {
			/* works */
			select_vc_direct(*v);
			return 0;
		}
		v = next_vc(v);
	}

	g_warning("no working sound");
	
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
		/*pb = gdk_pixbuf_add_alpha(pb, FALSE, 0,0,0);*/ /* copies too */
		gdk_pixbuf_saturate_and_pixelate(pb, pb2, 0, TRUE);
		/*pb = gdk_pixbuf_copy(pb2);
		composite_color(pb2, 0, 0, gdk_pixbuf_get_width(pb), 
		get_pixbuf_get_height(pb), 0,0, 1.0,1.0, INTERPOL, 
		255, 0,0, check_size, color1, color2); blah*/
		
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

static t_mixer *
mixer_new (void)
{
    GtkRcStyle	*rc;
    GdkColor	color;
    
    t_mixer *mixer;
    
    mixer = g_new (t_mixer, 1);

    mixer->broken = TRUE;
    
    mixer->options.command = g_strdup("aumix");
    mixer->options.use_sn = TRUE;
    mixer->options.use_terminal = FALSE;
    
    mixer->hbox = GTK_BOX(gtk_hbox_new(FALSE, 0));
    /*gtk_box_set_border(GTK_BOX(mixer->hbox), border_width);*/
    gtk_widget_set_name (GTK_WIDGET(mixer->hbox), "xfce_mixer");
    gtk_container_set_border_width(GTK_CONTAINER(mixer->hbox), border_width);

    gtk_widget_show(GTK_WIDGET(mixer->hbox));

    mixer->mixer = xfce_mixer_new (&mixer->broken);
    /*gtk_widget_set_name(mixer->mixer, "xfce_mixer");*/
    gtk_widget_show (mixer->mixer);
    gtk_box_pack_start (GTK_BOX(mixer->hbox), GTK_WIDGET(mixer->mixer), FALSE, FALSE, 0);

    mixer->status = GTK_PROGRESS_BAR(gtk_progress_bar_new());
    gtk_progress_bar_set_orientation(mixer->status, GTK_PROGRESS_BOTTOM_TO_TOP);
    
    rc = gtk_widget_get_modifier_style(GTK_WIDGET(mixer->status));
    if (!rc) { rc = gtk_rc_style_new(); }
    
    gdk_color_parse("#00c000", &color);
    
    if (rc) {
    	rc->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG; 
    	/*rc->color_flags[GTK_STATE_NORMAL] |= GTK_RC_BG; */
        rc->bg[GTK_STATE_PRELIGHT] = color;
        /*rc->bg[GTK_STATE_NORMAL] = color;*/
    }
   
    /*gtk_rc_parse_string("gtk-progressbar-style"style);*/
    /*bg[PRELIGHT]
    bg[NORMAL]*/
    
    gtk_widget_modify_style(GTK_WIDGET(mixer->status), rc); /* gone afterwards */
    
    /*gtk_widget_set_size_request(GTK_WIDGET(mixer->status), 8, 32);*/
    gtk_widget_show(GTK_WIDGET(mixer->status));

    mixer->eventbox = gtk_event_box_new ();
    /*gtk_widget_set_name (mixer->eventbox, "xfce_mixer");*/
    gtk_widget_show (mixer->eventbox);
    
    gtk_container_add (GTK_CONTAINER (mixer->eventbox), GTK_WIDGET(mixer->status));

    gtk_box_pack_start (GTK_BOX(mixer->hbox), GTK_WIDGET(mixer->eventbox), FALSE, FALSE, 0);
    
    return mixer;
}

static gboolean
update_volume_display(t_mixer *mixer)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(mixer->status), mixer->c_volume / 100.0);
	
	return FALSE;
}


static gboolean
check_volume (t_mixer *mixer)
{
	int volume;

	if (mixer->broken)
		return FALSE; /* stop */
	
	volume = get_master_volume();
	/*xfce_info("%d %d", volume, mixer->c_volume);*/
	if (volume != mixer->c_volume) {
		mixer->c_volume = volume; /* atomic? lol */
		g_idle_add((GSourceFunc) update_volume_display, mixer);
	}

	/* keep timeout running */
	return TRUE;
}

static gboolean
xfce_mixer_scroll_cb (GtkWidget *widget, GdkEventScroll *event, t_mixer *mixer)
{
	int	vol;	/* new volume */
	int	ovol;	/* old volume */ 
	int	tvol;	/* test volume */
	int	tries;
	
	vol = get_master_volume();
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
			set_master_volume(vol);
		
			tvol = get_master_volume();
			if (ovol != tvol) { /* worked. */
				break;
			}
		} else break; /* ouch */
		
		++tries;
	} while (ovol == vol && vol > 0 && vol < 100 && tries < 3); /* if still unchanged, loop */

	tvol = get_master_volume();
	mixer->c_volume = tvol;

	update_volume_display(mixer);
	return TRUE;
}


static gboolean
xfce_mixer_status_button_cb (GtkWidget * widget, GdkEventButton *b, t_mixer *mixer)
{
	int	y; /* pos */
	int	sy; /* size */
	/* b->type == GDK_BUTTON_PRESS|GDK_BUTTON_RELEASE|... */
	/* b->button = 1 or 2 or 3 (4,5)*/
	/* b->state = shift state (GdkModifierType) */
	/* b->x_root */
	/* b->y_root */
	/*if (b->type != GDK_BUTTON_PRESS) return FALSE;*/

	y = (int)b->y;
	/*gtk_widget_get_size(GTK_WIDGET(widget), &sx, &sy);*/
	
	sy = widget->allocation.height;
	/*xfce_info("%d/%d", y, sy);*/
	if (sy != 0) {
		/* this is a hack 'cause I dont know how to get the height 
		   of the border of the progressbar yet ;) */
		y = (sy + 2 - y) * 100 / sy;
	} else y = 0;

	set_master_volume(y);
		
	mixer->c_volume = get_master_volume();
	update_volume_display(mixer);

	return TRUE;
}

static gboolean
xfce_mixer_launch_button_cb(t_mixer *mixer)
{
	if (!mixer->options.command) return TRUE;
	
	exec_cmd(mixer->options.command, mixer->options.use_terminal, mixer->options.use_sn);
	/*exec_cmd("aumix", FALSE, TRUE);*/
	return TRUE;
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
mixer_free (Control * control)
{
    t_mixer *mixer = control->data;

    g_return_if_fail (mixer != NULL);

    g_free (mixer);
}

static void
mixer_attach_callback (Control * control, const char *signal,
                       GCallback callback, gpointer data)
{
    t_mixer *mixer = control->data;
#if 0
    g_signal_connect (mixer->eventbox, signal, callback, data);
#endif

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
	/*gtk_widget_queue_resize (GTK_WIDGET (mixer->hbox));*/
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
	/*mixer_set_size (control, settings.size);*/

	g_signal_connect(mixer->eventbox, "scroll-event", G_CALLBACK(xfce_mixer_scroll_cb), mixer);

	g_signal_connect(mixer->hbox, "scroll-event", G_CALLBACK(xfce_mixer_scroll_cb), mixer);
#if 0
	g_signal_connect(mixer->status, "scroll-event", G_CALLBACK(xfce_mixer_scroll_cb), mixer);
#endif
	g_signal_connect(mixer->eventbox, "button-press-event", G_CALLBACK(xfce_mixer_status_button_cb), mixer);
	g_signal_connect(mixer->eventbox, "button-release-event", G_CALLBACK(xfce_mixer_status_button_cb), mixer);
	
	g_signal_connect_swapped(mixer->mixer, "clicked", G_CALLBACK(xfce_mixer_launch_button_cb), mixer);

	if (!mixer->broken) {
		check_volume (mixer);
		/* timeout_id==0: failed */
		mixer->timeout_id = g_timeout_add(5 * 1000, (GSourceFunc) check_volume, mixer);
	}

	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->hbox), _("Volume Control"), NULL);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->mixer), _("Volume Control"), NULL);
	gtk_tooltips_set_tip(tooltips, GTK_WIDGET(mixer->eventbox), _("Volume Control"), NULL);

	return TRUE;
}

static
void create_options_backup(t_mixer *mixer)
{
	mixer->revert.command = g_strdup(mixer->options.command); 
	mixer->revert.use_sn = mixer->options.use_sn;
	mixer->revert.use_terminal = mixer->options.use_terminal;
}

static
GtkWidget *mixer_options_get(GtkContainer *c, int index)
{
	GList	*list;
	GList	*iter;
	int	pos = 0;
	GtkWidget *w = NULL;
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
free_optionsdialog(t_mixer *mixer)
{
	if (mixer->revert.command) g_free(mixer->revert.command); 
	mixer->revert.command = NULL;
	if (mixer->options.command) g_free(mixer->options.command); 
	mixer->options.command = NULL;
	/*g_free();*/
}

static void 
mixer_revert_make_sensitive(GtkWidget *w)
{
	gtk_widget_set_sensitive (w, TRUE);
}

static void 
mixer_stuff_toggled_cb(GtkToggleButton *tb, t_mixer *mixer)
{
	mixer_revert_make_sensitive(mixer->revert_b);
}
 
static void 
mixer_command_entry_lost_focus(t_mixer *mixer)
{
	mixer_revert_make_sensitive(mixer->revert_b);
}

static void
mixer_do_options(t_mixer *mixer, int mode) /* 0: load; 1: store; 2: connect revert */
{
	char const *temp;
	GtkContainer *c;
	GtkContainer *h1; /* hbox for entry */
	GtkContainer *h2; /* hbox for vbox2 */
	GtkContainer *v2; /* vbox for use_sn, use_terminal */
	GtkEntry	*e_command = NULL;	
	GtkCheckButton	*b_use_sn = NULL;
	GtkCheckButton	*b_use_term = NULL;
	GtkButton	*b_dotdotdot = NULL;
	c = mixer->settings_c; /* vbox 1 */
	/*c = GTK_CONTAINER(mixer_options_get(mixer->settings_c, 100));*/ /* vbox */

	if (c) {
		h1 = GTK_CONTAINER(mixer_options_get(c, 0));
		e_command = GTK_ENTRY(mixer_options_get(GTK_CONTAINER(h1), 1));
		b_dotdotdot = GTK_BUTTON(mixer_options_get(GTK_CONTAINER(h1), 2));
		h2 = GTK_CONTAINER(mixer_options_get(c, 1));
		v2 = GTK_CONTAINER(mixer_options_get(h2, 1));
		
		b_use_term = GTK_CHECK_BUTTON(mixer_options_get(v2, 0));
		b_use_sn = GTK_CHECK_BUTTON(mixer_options_get(v2, 1));
	}
	/*xfce_info("c: %X, h1: %X, h2: %X, v2: %X, e_command: %X, b_use_sn: %X, b_use_term: %X",
		c, h1, h2, v2, e_command, b_use_sn, b_use_term);
	*/
	if (b_dotdotdot && mode == 2) {
		g_signal_connect(GTK_WIDGET(b_dotdotdot), "clicked", G_CALLBACK(mixer_revert_make_sensitive), mixer->revert_b);
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
			gtk_entry_set_text(GTK_ENTRY(e_command), g_strdup(mixer->options.command));
			/*gtk_entry_set_text(GTK_ENTRY(e_command), g_strdup("test"));*/
			break;
		case 2:
			g_signal_connect_swapped (e_command, "insert-at-cursor", G_CALLBACK (mixer_revert_make_sensitive), mixer->revert_b);
			g_signal_connect_swapped (e_command, "delete-from-cursor", G_CALLBACK (mixer_revert_make_sensitive), mixer->revert_b);
			g_signal_connect_swapped (e_command, "focus-out-event", G_CALLBACK(mixer_command_entry_lost_focus), mixer);

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
		}
	}
}

static void
mixer_apply_options(t_mixer *mixer)
{
	if (mixer->options.command) g_free(mixer->options.command);
	mixer->options.command = NULL;

	mixer_do_options(mixer, 1);
}	

static void
mixer_fill_options(t_mixer *mixer)
{
	mixer_do_options(mixer, 0);
}

static void
mixer_revert_options(t_mixer *mixer)
{
	if (mixer->options.command) g_free(mixer->options.command);
	mixer->options.command = NULL;
	
	mixer->options.command = mixer->revert.command;
	mixer->options.use_sn = mixer->revert.use_sn;
	mixer->options.use_terminal = mixer->revert.use_terminal;
	mixer->revert.command = NULL;
	
	mixer_fill_options(mixer);
	create_options_backup(mixer);
	gtk_widget_set_sensitive(mixer->revert_b, FALSE);
}

static void
mixer_add_options(Control *control, GtkContainer *container, GtkWidget *revert, GtkWidget *done)
{
	t_mixer		*mixer;
	GtkWidget 	*vbox;
	
	mixer = (t_mixer *)control->data;

	/*revert*/
	mixer->dialog = gtk_widget_get_toplevel(revert);
	mixer->revert_b = revert;
	
	mixer->sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = create_command_option(mixer->sg);
	gtk_container_add(GTK_CONTAINER(container), GTK_WIDGET(vbox));
	mixer->settings_c = GTK_CONTAINER(vbox);
	/*g_signal_connect(GTK_WIDGET(vbox), "show-event", G_CALLBACK(mixer_fill_options), mixer);*/
	/*g_signal_connect(GTK_WIDGET(vbox), "hide-event", G_CALLBACK(mixer_apply_options), mixer);*/
	mixer_fill_options(mixer);
	mixer_do_options(mixer, 2);
	create_options_backup(mixer);
	
	g_signal_connect_swapped(GTK_WIDGET(mixer->dialog), "destroy-event", G_CALLBACK(free_optionsdialog), mixer);
	g_signal_connect_swapped(GTK_WIDGET(mixer->revert_b), "clicked", G_CALLBACK(mixer_revert_options), mixer);
	g_signal_connect_swapped(GTK_WIDGET(done), "clicked", G_CALLBACK(mixer_apply_options), mixer);
}

extern xmlDocPtr xmlconfig;
#define MYDATA(node) xmlNodeListGetString(xmlconfig, node->children, 1)

static void
mixer_read_config(Control *control, xmlNodePtr node)
{
	int	n;
	t_mixer	*mixer;
	xmlChar	*value;

	mixer = (t_mixer *)control->data;
	
	if (!node || !node->children) 
		return;
		
	node = node->children;
	
	if (!xmlStrEqual(node->name, (const xmlChar *)MIXER_ROOT))
		return;
	for (node = node->children; node; node = node->next) {
		if (xmlStrEqual (node->name, (const xmlChar *)"Command")) {
			value = MYDATA (node);
			if (value) {
				g_free (mixer->options.command);
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
		}
	}
}

static void
mixer_write_config(Control *control, xmlNodePtr parent)
{
	xmlNodePtr root, node;
	char value[MAXSTRLEN + 1];

	t_mixer *mixer = (t_mixer *) control->data;

	root = xmlNewTextChild (parent, NULL, MIXER_ROOT, NULL);

	node = xmlNewTextChild (root, NULL, "Command", mixer->options.command);

	snprintf (value, 2, "%d", mixer->options.use_terminal);
	xmlSetProp (node, "term", value);
 
	snprintf (value, 2, "%d", mixer->options.use_sn);
	xmlSetProp (node, "sn", value);
}

G_MODULE_EXPORT void
xfce_control_class_init (ControlClass * cc)
{
    cc->name = "mixer";
    cc->caption = _("Volume Control");

    cc->create_control = (CreateControlFunc) create_mixer_control;

    cc->free = mixer_free;
    cc->read_config = mixer_read_config;
    cc->write_config = mixer_write_config;
    cc->attach_callback = mixer_attach_callback;
    
    cc->add_options = (gpointer) mixer_add_options;

    cc->set_theme = mixer_set_theme;

    cc->set_size = mixer_set_size;
}

/* macro defined in plugins.h */
XFCE_PLUGIN_CHECK_INIT

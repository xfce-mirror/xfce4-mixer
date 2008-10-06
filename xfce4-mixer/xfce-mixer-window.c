/* $Id$ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/audio/mixerutils.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer.h"
#include "xfce-mixer-window.h"
#include "xfce-mixer-controls-dialog.h"



static void     xfce_mixer_window_class_init             (XfceMixerWindowClass *klass);
static void     xfce_mixer_window_init                   (XfceMixerWindow      *window);
static void     xfce_mixer_window_dispose                (GObject              *object);
static void     xfce_mixer_window_finalize               (GObject              *object);
static void     xfce_mixer_window_soundcard_changed      (XfceMixerCardCombo   *combo,
                                                          GstElement           *card,
                                                          XfceMixerWindow      *window);
static void     xfce_mixer_window_action_select_controls (GtkAction            *action,
                                                          XfceMixerWindow      *window);
static void     xfce_mixer_window_close                  (GtkAction            *action,
                                                          XfceMixerWindow      *window);
static gboolean xfce_mixer_window_closed                 (GtkWidget            *window,
                                                          GdkEvent             *event,
                                                          XfceMixerWindow      *mixer_window);
static void     xfce_mixer_window_update_contents        (XfceMixerWindow      *window);



struct _XfceMixerWindowClass
{
  GtkWindowClass __parent__;
};

struct _XfceMixerWindow
{
  GtkWindow __parent__;

  XfceMixerPreferences *preferences;

  GtkActionGroup       *action_group;

  GtkWidget            *soundcard_combo;

  /* Active mixer control set */
  GtkWidget            *mixer_frame;
  GtkWidget            *mixer;

  GtkWidget            *select_controls_button;
};



static const GtkActionEntry action_entries[] = 
{
  { "quit", GTK_STOCK_QUIT, N_ ("_Quit"), "<control>Q", N_ ("Exit the mixer"), 
    G_CALLBACK (xfce_mixer_window_close) },
  { "select-controls", NULL, N_ ("_Select Controls..."), "<control>S", N_ ("Select which controls are displayed"), 
    G_CALLBACK (xfce_mixer_window_action_select_controls) },
};

static const GtkToggleActionEntry toggle_action_entries[] =
{
};



static GObjectClass *xfce_mixer_window_parent_class = NULL;



GType
xfce_mixer_window_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerWindowClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_window_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerWindow),
          0,
          (GInstanceInitFunc) xfce_mixer_window_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_WINDOW, "XfceMixerWindow", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_window_class_init (XfceMixerWindowClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_window_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_window_dispose;
  gobject_class->finalize = xfce_mixer_window_finalize;
}



static void
xfce_mixer_window_init (XfceMixerWindow *window)
{
  GtkWidget       *heading;
  GtkWidget       *separator;
  GtkWidget       *label;
  GtkWidget       *button;
  GtkWidget       *vbox;
  GtkWidget       *hbox;
  GtkWidget       *bbox;
  gchar           *active_card = NULL;
  gchar           *title;
  gint             width;
  gint             height;

  window->preferences = xfce_mixer_preferences_get ();

  g_object_get (window->preferences, "window-width", &width, "window-height", &height, "sound-card", &active_card, NULL);

  /* Configure the main window */
  gtk_window_set_icon_name (GTK_WINDOW (window), "multimedia-volume-control");
  gtk_window_set_title (GTK_WINDOW (window), _("Mixer"));
  gtk_window_set_default_size (GTK_WINDOW (window), width, height);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  g_signal_connect (window, "delete-event", G_CALLBACK (xfce_mixer_window_closed), window);

  /* Quit mixer when the main window is closed */
  g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (gtk_main_quit), NULL);

  /* Create window action group */
  window->action_group = gtk_action_group_new ("XfceMixerWindow");
  gtk_action_group_set_translation_domain (window->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (window->action_group, action_entries, G_N_ELEMENTS (action_entries), GTK_WIDGET (window));

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  heading = xfce_heading_new ();
  xfce_heading_set_title (XFCE_HEADING (heading), _("Mixer"));
  xfce_heading_set_subtitle (XFCE_HEADING (heading), _("Configure your sound card(s) and control the volume of selected tracks"));
  xfce_heading_set_icon_name (XFCE_HEADING (heading), "multimedia-volume-control");
  gtk_box_pack_start (GTK_BOX (vbox), heading, FALSE, TRUE, 0);
  gtk_widget_show (heading);

  separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 0);
  gtk_widget_show (separator);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new (NULL);
  title = g_strdup_printf ("<span weight='bold' size='large'>%s</span>", _("Sound card:"));
  gtk_label_set_markup (GTK_LABEL (label), title);
  g_free (title);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  window->soundcard_combo = xfce_mixer_card_combo_new (xfce_mixer_get_card (active_card));
  g_signal_connect (window->soundcard_combo, "soundcard-changed", G_CALLBACK (xfce_mixer_window_soundcard_changed), window);
  gtk_container_add (GTK_CONTAINER (hbox), window->soundcard_combo);
  gtk_widget_show (window->soundcard_combo);

  window->mixer_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (window->mixer_frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (window->mixer_frame), 6);
  gtk_container_add (GTK_CONTAINER (vbox), window->mixer_frame);
  gtk_widget_show (window->mixer_frame);

  bbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_EDGE);
  gtk_container_set_border_width (GTK_CONTAINER (bbox), 6);
  gtk_box_pack_start (GTK_BOX (vbox), bbox, FALSE, TRUE, 0);
  gtk_widget_show (bbox);

  window->select_controls_button = gtk_button_new ();
  gtk_action_connect_proxy (gtk_action_group_get_action (window->action_group, "select-controls"), 
                            window->select_controls_button);
  gtk_button_set_image (GTK_BUTTON (window->select_controls_button), 
                        gtk_image_new_from_icon_name ("preferences-desktop", GTK_ICON_SIZE_BUTTON));
  gtk_widget_set_sensitive (window->select_controls_button, FALSE);
  gtk_box_pack_start (GTK_BOX (bbox), window->select_controls_button, FALSE, TRUE, 0);
  gtk_widget_show (window->select_controls_button);

  button = gtk_button_new ();
  gtk_action_connect_proxy (gtk_action_group_get_action (window->action_group, "quit"), button);
  gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_BUTTON));
  gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);

  /* Re-generate mixer controls for the active sound card */
  xfce_mixer_window_update_contents (window);

  g_free (active_card);
}



static void
xfce_mixer_window_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_window_parent_class)->dispose) (object);
}



static void
xfce_mixer_window_finalize (GObject *object)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (object);

  g_object_unref (G_OBJECT (window->preferences));

  (*G_OBJECT_CLASS (xfce_mixer_window_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_window_new (void)
{
  return g_object_new (TYPE_XFCE_MIXER_WINDOW, NULL);
}



static void
xfce_mixer_window_soundcard_changed (XfceMixerCardCombo *combo,
                                     GstElement         *card,
                                     XfceMixerWindow    *window)
{
  gchar *title;

  g_return_if_fail (IS_XFCE_MIXER_CARD_COMBO (combo));
  g_return_if_fail (IS_XFCE_MIXER_WINDOW (window));
  g_return_if_fail (GST_IS_MIXER (card));

  title = g_strdup_printf ("%s - %s", _("Mixer"), xfce_mixer_get_card_display_name (card));
  gtk_window_set_title (GTK_WINDOW (window), title);
  g_free (title);

  /* Destroy the current mixer */
  if (G_LIKELY (window->mixer != NULL))
    gtk_widget_destroy (gtk_bin_get_child (GTK_BIN (window->mixer_frame)));

  DBG ("card = %s", xfce_mixer_get_card_internal_name (card));

  xfce_mixer_select_card (card);

  /* Create a new XfceMixer for the active sound card */
  window->mixer = xfce_mixer_new (card);

  /* Add the XfceMixer to the window */
  gtk_container_add (GTK_CONTAINER (window->mixer_frame), window->mixer);
  gtk_widget_show (window->mixer);

  /* Make the "Select Controls..." button sensitive */
  gtk_widget_set_sensitive (window->select_controls_button, TRUE);

  /* Remember the card for next time */
  g_object_set (G_OBJECT (window->preferences), "sound-card", xfce_mixer_get_card_internal_name (card), NULL);
}



static void
xfce_mixer_window_action_select_controls (GtkAction       *action,
                                          XfceMixerWindow *window)
{
  GtkWidget     *dialog;

  dialog = xfce_mixer_controls_dialog_new (window);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  xfce_mixer_window_update_contents (window);
}



static void
xfce_mixer_window_close (GtkAction       *action,
                         XfceMixerWindow *window)
{
  /* This is a nasty hack to save the settings before the application quits */
  xfce_mixer_window_closed (GTK_WIDGET (window), NULL, window);

  gtk_main_quit ();
}



static gboolean 
xfce_mixer_window_closed (GtkWidget       *window,
                          GdkEvent        *event,
                          XfceMixerWindow *mixer_window)
{
  gint width;
  gint height;

  gtk_window_get_size (GTK_WINDOW (mixer_window), &width, &height);
  g_object_set (G_OBJECT (mixer_window->preferences), "window-width", width, "window-height", height, NULL);

  return FALSE;
}



GstElement *
xfce_mixer_window_get_active_card (XfceMixerWindow *window)
{
  g_return_val_if_fail (IS_XFCE_MIXER_WINDOW (window), NULL);
  return xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo));
}



static void
xfce_mixer_window_update_contents (XfceMixerWindow *window)
{
  GstElement *card;

  g_return_if_fail (IS_XFCE_MIXER_WINDOW (window));

  card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo));

  /* Kind of a hack to regenerate the mixer controls */
  if (G_LIKELY (GST_IS_MIXER (card)))
    xfce_mixer_window_soundcard_changed (XFCE_MIXER_CARD_COMBO (window->soundcard_combo), card, window);
}

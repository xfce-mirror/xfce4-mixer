/* $Id: xfce-menu.h 25273 2008-03-23 19:20:47Z jannis $ */
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

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <gst/gst.h>
#include <gst/audio/mixerutils.h>
#include <gst/interfaces/mixer.h>

#include "xfce-mixer-window.h"
#include "xfce-mixer.h"
#include "xfce-mixer-preferences.h"
#include "xfce-mixer-controls-dialog.h"



#define NAME_COLUMN    0
#define ELEMENT_COLUMN 1



static void     xfce_mixer_window_class_init             (XfceMixerWindowClass *klass);
static void     xfce_mixer_window_init                   (XfceMixerWindow      *window);
static void     xfce_mixer_window_dispose                (GObject              *object);
static void     xfce_mixer_window_finalize               (GObject              *object);
static void     xfce_mixer_window_load_soundcards        (XfceMixerWindow      *window);
static void     xfce_mixer_window_soundcard_changed      (GtkComboBox          *widget,
                                                          XfceMixerWindow      *window);
static void     xfce_mixer_window_action_select_controls (GtkAction            *action,
                                                          XfceMixerWindow      *window);
static gboolean xfce_mixer_window_filter_mixer           (GstMixer             *mixer,
                                                          gpointer              user_data);
static void     xfce_mixer_window_close                  (GtkAction            *action,
                                                          XfceMixerWindow      *window);
static gboolean xfce_mixer_window_closed                 (GtkWidget            *window,
                                                          GdkEvent             *event,
                                                          XfceMixerWindow      *mixer_window);



struct _XfceMixerWindowClass
{
  GtkWindowClass __parent__;
};

struct _XfceMixerWindow
{
  GtkWindow __parent__;

  XfceMixerPreferences *preferences;

  GtkActionGroup       *action_group;

  GtkListStore         *soundcard_model;
  GtkWidget            *soundcard_combo;

  /* List of available GstMixers */
  GList                *mixers;

  /* Active mixer control set */
  GtkWidget            *mixer_frame;
  GtkWidget            *mixer;

  GtkWidget            *select_controls_button;
};



static const GtkActionEntry action_entries[] = 
{
  { "quit", GTK_STOCK_QUIT, N_ ("_Quit"), "<control>Q", N_ ("Exit Xfce Mixer"), 
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
  GtkWidget       *image;
  GtkWidget       *vbox;
  GtkWidget       *hbox;
  GtkWidget       *bbox;
  GtkCellRenderer *renderer;
  GtkAction       *action;
  GtkListStore    *model;
  gchar           *title;
  gint             width;
  gint             height;

  window->preferences = xfce_mixer_preferences_get ();

  g_object_get (G_OBJECT (window->preferences), 
                "last-window-width", &width,
                "last-window-height", &height,
                NULL);

  /* Configure the main window */
  gtk_window_set_icon_name (GTK_WINDOW (window), "xfce4-mixer");
  gtk_window_set_title (GTK_WINDOW (window), _("Xfce Mixer"));
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
  xfce_heading_set_title (XFCE_HEADING (heading), _("Xfce Mixer"));
  xfce_heading_set_subtitle (XFCE_HEADING (heading), _("A reliable and comfortable mixer for your soundcard, finally!"));
  xfce_heading_set_icon_name (XFCE_HEADING (heading), "xfce4-mixer");
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

  window->soundcard_model = gtk_list_store_new (2, G_TYPE_STRING, GST_TYPE_ELEMENT);

  window->soundcard_combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (window->soundcard_model));
  g_signal_connect (G_OBJECT (window->soundcard_combo), "changed", G_CALLBACK (xfce_mixer_window_soundcard_changed), window);
  gtk_container_add (GTK_CONTAINER (hbox), window->soundcard_combo);
  gtk_widget_show (window->soundcard_combo);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (window->soundcard_combo), renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (window->soundcard_combo), renderer, "text", 0);

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

  /* Load available sound cards */
  xfce_mixer_window_load_soundcards (window);
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

  g_list_foreach (window->mixers, (GFunc) gst_object_unref, NULL);
  g_list_free (window->mixers);

  (*G_OBJECT_CLASS (xfce_mixer_window_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_window_new (void)
{
  return g_object_new (TYPE_XFCE_MIXER_WINDOW, NULL);
}



static void
xfce_mixer_window_load_soundcards (XfceMixerWindow *window)
{
  GtkTreeIter  iter;
  GList       *miter;
  guint        counter = 0;
  const gchar *name;
  gchar       *active_card;
  guint        active_index = 0;

  /* Get list of all available sound cards */
  window->mixers = gst_audio_default_registry_mixer_filter (xfce_mixer_window_filter_mixer, FALSE, &counter);

  if (G_UNLIKELY (g_list_length (window->mixers) == 0))
    xfce_err ("No sound cards could be found. You may have to install additional gstreamer packages for ALSA or OSS support.");

  /* Get sound card used last time */
  g_object_get (G_OBJECT (window->preferences), "sound-card", &active_card, NULL);

  for (miter = g_list_first (window->mixers), counter = 0; miter != NULL; miter = g_list_next (miter), ++counter)
    {
      name = g_object_get_data (G_OBJECT (miter->data), "xfce-mixer-control-name");
      g_debug ("Adding sound card: %s", name);
      gtk_list_store_append (window->soundcard_model, &iter);
      gtk_list_store_set (window->soundcard_model, &iter, NAME_COLUMN, name, ELEMENT_COLUMN, miter->data, -1);

      if (G_UNLIKELY (active_card != NULL && g_utf8_collate (name, active_card) == 0))
        active_index = counter;
    }

  if (G_LIKELY (active_card != NULL))
    g_free (active_card);

  gtk_combo_box_set_active (GTK_COMBO_BOX (window->soundcard_combo), active_index);
}



static void
remove_child (GtkWidget *widget)
{
  gtk_widget_destroy (widget);
}



static void
xfce_mixer_window_soundcard_changed (GtkComboBox     *widget,
                                     XfceMixerWindow *window)
{
  GtkTreeIter  iter;
  GstElement  *mixer;
  const gchar *name;
  gchar       *title;
  gchar       *mixer_name;

  /* Determine selected sound card */
  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget), &iter)))
    {
      /* Fetch index of the active sound card */
      gtk_tree_model_get (GTK_TREE_MODEL (window->soundcard_model), &iter, NAME_COLUMN, &mixer_name, ELEMENT_COLUMN, &mixer, -1);

      /* Update window title */
      if (G_LIKELY (mixer != NULL))
        {
          title = g_strdup_printf ("%s - %s", _("Xfce Mixer"), mixer_name);
          gtk_window_set_title (GTK_WINDOW (window), title);
          g_free (title);
        }
      else
        gtk_window_set_title (GTK_WINDOW (window), _("Xfce Mixer"));

      /* Destroy current mixer controls */
      if (G_LIKELY (window->mixer != NULL))
        gtk_widget_destroy (gtk_bin_get_child (GTK_BIN (window->mixer_frame)));

      gst_element_set_state (mixer, GST_STATE_READY);

      /* Create a new XfceMixer for the active GstMixer */
      window->mixer = xfce_mixer_new (mixer, mixer_name);

      /* Add the XfceMixer to the window */
      gtk_container_add (GTK_CONTAINER (window->mixer_frame), window->mixer);
      gtk_widget_show (window->mixer);

      /* Make the "Select Controls..." button sensitive */
      gtk_widget_set_sensitive (window->select_controls_button, TRUE);
    }
  else
    {
      /* Make the "Select Controls..." button insensitive */
      gtk_widget_set_sensitive (window->select_controls_button, FALSE);
    }
}



static void
xfce_mixer_window_action_select_controls (GtkAction       *action,
                                          XfceMixerWindow *window)
{
  GtkWidget *dialog;

  dialog = xfce_mixer_controls_dialog_new (window);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  /* Again, kind of a hack, this time to regenerate the mixer controls */
  xfce_mixer_window_soundcard_changed (GTK_COMBO_BOX (window->soundcard_combo), window);
}



static gboolean
xfce_mixer_window_filter_mixer (GstMixer *mixer,
                                gpointer user_data)
{
  GstElementFactory *factory;
  const gchar       *long_name;
  gchar             *device_name = NULL;
  gchar             *name;
  gchar             *title;
  guint             *counter = (guint *) user_data;

  /* Get long name of the mixer element */
  factory = gst_element_get_factory (GST_ELEMENT (mixer));
  long_name = gst_element_factory_get_longname (factory);

  /* Get device name of the mixer element */
  g_object_get (G_OBJECT (mixer), "device-name", &device_name, NULL);

  /* Build full name for this element */
  if (G_LIKELY (device_name != NULL))
    {
      name = g_strdup_printf ("%s (%s)", device_name, long_name);
      g_free (device_name);
    }
  else
    {
      title = g_strdup_printf (_("Unknown Volume Control %d"), (*counter)++);
      name = g_strdup_printf ("%s (%s)", title, long_name);
      g_free (title);
    }

  /* Set name to be used in xfce4-mixer */
  g_object_set_data_full (G_OBJECT (mixer), "xfce-mixer-control-name", name, (GDestroyNotify) g_free);

  gst_element_set_state (GST_ELEMENT (mixer), GST_STATE_NULL);

  return TRUE;
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
  gchar *active_card;
  gint   width;
  gint   height;

  gtk_window_get_size (GTK_WINDOW (mixer_window), &width, &height);

  active_card = gtk_combo_box_get_active_text (GTK_COMBO_BOX (mixer_window->soundcard_combo));

  g_object_set (G_OBJECT (mixer_window->preferences), 
                "last-window-width", width,
                "last-window-height", height,
                "sound-card", active_card,
                NULL);

  g_free (active_card);

  return FALSE;
}



GstMixer*
xfce_mixer_window_get_active_mixer (XfceMixerWindow *window)
{
  GstElement *element;
  GstMixer   *mixer = NULL;
  GtkTreeIter iter;

  if (G_LIKELY (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (window->soundcard_combo), &iter)))
    {
      gtk_tree_model_get (GTK_TREE_MODEL (window->soundcard_model), &iter, ELEMENT_COLUMN, &element, -1);
      
      if (G_LIKELY (element != NULL))
        mixer = GST_MIXER (element);
    }

  return mixer;
}



gchar*
xfce_mixer_window_get_active_card (XfceMixerWindow *window)
{
  return gtk_combo_box_get_active_text (GTK_COMBO_BOX (window->soundcard_combo));
}

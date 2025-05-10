/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 * Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_XFCE_REVISION_H
#include "xfce-revision.h"
#endif

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-container.h"
#include "xfce-mixer-window.h"
#include "xfce-mixer-controls-dialog.h"



static void     xfce_mixer_window_dispose                     (GObject              *object);
static void     xfce_mixer_window_finalize                    (GObject              *object);
static void     xfce_mixer_window_size_allocate               (GtkWidget            *widget,
                                                               GtkAllocation        *allocation);
static gboolean xfce_mixer_window_state_event                 (GtkWidget            *widget,
                                                               GdkEventWindowState  *event);
static void     xfce_mixer_window_destroy                     (GtkWidget            *widget);
static void     xfce_mixer_window_soundcard_changed           (XfceMixerCardCombo   *combo,
                                                               GstElement           *card,
                                                               XfceMixerWindow      *window);
static void     xfce_mixer_window_soundcard_property_changed  (XfceMixerWindow      *window,
                                                               GParamSpec           *pspec,
                                                               GObject              *object);
static void     xfce_mixer_window_action_select_controls      (GSimpleAction        *action,
                                                               GVariant             *parameter,
                                                               gpointer              user_data);
static void     xfce_mixer_window_controls_property_changed   (XfceMixerWindow      *window,
                                                               GParamSpec           *pspec,
                                                               GObject              *object);
static void     xfce_mixer_window_show_about_dialog           (GSimpleAction        *action,
                                                               GVariant             *parameter,
                                                               gpointer              user_data);
static void     xfce_mixer_window_close                       (GSimpleAction        *action,
                                                               GVariant             *parameter,
                                                               gpointer              user_data);
static void     xfce_mixer_window_update_contents             (XfceMixerWindow      *window);



struct _XfceMixerWindow
{
  XfceTitledDialog __parent__;

  XfceMixerPreferences *preferences;

  /* Current window state */
  gint                  current_width;
  gint                  current_height;
  gboolean              is_maximized;
  gboolean              is_fullscreen;

  GtkWidget            *soundcard_combo;

  /* Active mixer control set */
  GtkWidget            *mixer_frame;
  GtkWidget            *mixer_container;

  GtkWidget            *select_controls_button;

  GtkWidget            *controls_dialog;
};



static const GActionEntry action_entries[] =
{
  { "about", &xfce_mixer_window_show_about_dialog, NULL, NULL, NULL },
  { "quit", &xfce_mixer_window_close, NULL, NULL, NULL },
  { "select-controls", &xfce_mixer_window_action_select_controls, NULL, NULL, NULL },
};



G_DEFINE_TYPE (XfceMixerWindow, xfce_mixer_window, XFCE_TYPE_TITLED_DIALOG)



static void
xfce_mixer_window_class_init (XfceMixerWindowClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtk_widget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_window_dispose;
  gobject_class->finalize = xfce_mixer_window_finalize;

  gtk_widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class->size_allocate = xfce_mixer_window_size_allocate;
  gtk_widget_class->window_state_event = xfce_mixer_window_state_event;
  gtk_widget_class->destroy = xfce_mixer_window_destroy;
}



static void
xfce_mixer_window_init (XfceMixerWindow *window)
{
  GApplication  *app = g_application_get_default ();
  GtkWidget     *label;
  GtkWidget     *about_button;
  GtkWidget     *quit_button;
  GtkWidget     *vbox;
  GtkWidget     *hbox;
  GtkWidget     *bbox;
  gchar         *card_name;
  GstElement    *card;
  const gchar   *select_controls_accels[] = { "<Control>s", NULL };
  const gchar   *quit_accels[] = { "<Control>q", NULL };

  window->controls_dialog = NULL;

  window->preferences = xfce_mixer_preferences_get ();

  window->is_maximized = FALSE;
  window->is_fullscreen = FALSE;

  g_object_get (window->preferences, "window-width", &window->current_width, "window-height", &window->current_height,
                "sound-card", &card_name, NULL);
  if (card_name != NULL)
    card = xfce_mixer_get_card (card_name);
  else
    {
      card = xfce_mixer_get_default_card ();

      if (GST_IS_MIXER (card))
        g_object_set (window->preferences, "sound-card", xfce_mixer_get_card_internal_name (card), NULL);
    }
  g_free (card_name);

  /* Configure the main window */
  gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
  gtk_window_set_icon_name (GTK_WINDOW (window), "multimedia-volume-control");
  gtk_window_set_title (GTK_WINDOW (window), _("Audio Mixer"));
  gtk_window_set_default_size (GTK_WINDOW (window), window->current_width, window->current_height);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  /* Install actions */
  g_action_map_add_action_entries (G_ACTION_MAP (app), action_entries, G_N_ELEMENTS (action_entries), window);

  /* Install action accelerators for the mixer window */
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.select-controls", select_controls_accels);
  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", quit_accels);

  vbox = gtk_dialog_get_content_area (GTK_DIALOG (window));
  gtk_widget_show (vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Sound _card:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  window->soundcard_combo = xfce_mixer_card_combo_new (card);
  g_signal_connect (G_OBJECT (window->soundcard_combo), "soundcard-changed", G_CALLBACK (xfce_mixer_window_soundcard_changed), window);
  gtk_box_pack_start (GTK_BOX (hbox), window->soundcard_combo, TRUE, TRUE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), window->soundcard_combo);
  gtk_widget_show (window->soundcard_combo);

  window->mixer_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (window->mixer_frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (window->mixer_frame), 6);
  gtk_box_pack_start (GTK_BOX (vbox), window->mixer_frame, TRUE, TRUE, 0);
  gtk_widget_show (window->mixer_frame);

  window->mixer_container = xfce_mixer_container_new (NULL);
  gtk_container_add (GTK_CONTAINER (window->mixer_frame), window->mixer_container);
  gtk_widget_show (window->mixer_container);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  /* Single place still using deprecated API. GTK+ uses it internally as well, so why should we suffer?
     Suffice it to say, new API is quite limited. */
  bbox = gtk_dialog_get_action_area (GTK_DIALOG (window));
G_GNUC_END_IGNORE_DEPRECATIONS
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_END);
  gtk_container_set_border_width (GTK_CONTAINER (bbox), 6);

  window->select_controls_button = gtk_button_new_with_mnemonic (_("_Select Controls..."));
  gtk_button_set_image (GTK_BUTTON (window->select_controls_button), 
                        gtk_image_new_from_icon_name ("preferences-desktop", GTK_ICON_SIZE_BUTTON));
  gtk_actionable_set_action_name (GTK_ACTIONABLE (window->select_controls_button), "app.select-controls");
  gtk_widget_set_sensitive (window->select_controls_button, FALSE);
  gtk_box_pack_start (GTK_BOX (bbox), window->select_controls_button, FALSE, TRUE, 0);
  gtk_widget_show (window->select_controls_button);

  about_button = gtk_button_new_with_mnemonic (_("_About"));
  gtk_button_set_image (GTK_BUTTON (about_button),
                        gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_BUTTON));
  gtk_actionable_set_action_name (GTK_ACTIONABLE (about_button), "app.about");
  gtk_box_pack_start (GTK_BOX (bbox), about_button, FALSE, TRUE, 0);
  gtk_widget_show (about_button);

  quit_button = gtk_button_new_with_mnemonic (_("_Quit"));
  gtk_button_set_image (GTK_BUTTON (quit_button),
                        gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_BUTTON));
  gtk_actionable_set_action_name (GTK_ACTIONABLE (quit_button), "app.quit");
  gtk_box_pack_start (GTK_BOX (bbox), quit_button, FALSE, TRUE, 0);
  gtk_widget_show (quit_button);

  g_signal_connect_swapped (G_OBJECT (window->preferences), "notify::sound-card", G_CALLBACK (xfce_mixer_window_soundcard_property_changed), window);

  g_signal_connect_swapped (G_OBJECT (window->preferences), "notify::controls", G_CALLBACK (xfce_mixer_window_controls_property_changed), window);

  /* Update mixer controls for the active sound card */
  xfce_mixer_window_update_contents (window);
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



static void
xfce_mixer_window_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (widget);

  (*GTK_WIDGET_CLASS (xfce_mixer_window_parent_class)->size_allocate) (widget, allocation);

  if (!window->is_maximized && !window->is_fullscreen)
    gtk_window_get_size (GTK_WINDOW (window), &window->current_width, &window->current_height);
}



static gboolean
xfce_mixer_window_state_event (GtkWidget           *widget,
                               GdkEventWindowState *event)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (widget);
  gboolean         result = GDK_EVENT_PROPAGATE;

  if (GTK_WIDGET_CLASS (xfce_mixer_window_parent_class)->window_state_event != NULL)
    result = (*GTK_WIDGET_CLASS (xfce_mixer_window_parent_class)->window_state_event) (widget, event);

  window->is_maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
  window->is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

  return result;
}



static void
xfce_mixer_window_destroy (GtkWidget *widget)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (widget);

  g_object_set (G_OBJECT (window->preferences), "window-width", window->current_width,
                "window-height", window->current_height, NULL);

  (*GTK_WIDGET_CLASS (xfce_mixer_window_parent_class)->destroy) (widget);
}



GtkWidget*
xfce_mixer_window_new (GApplication *app)
{
  return g_object_new (XFCE_TYPE_MIXER_WINDOW, "application", app, NULL);
}



static void
xfce_mixer_window_soundcard_changed (XfceMixerCardCombo *combo,
                                     GstElement         *card,
                                     XfceMixerWindow    *window)
{
  g_return_if_fail (XFCE_IS_MIXER_CARD_COMBO (combo));
  g_return_if_fail (XFCE_IS_MIXER_WINDOW (window));
  g_return_if_fail (GST_IS_MIXER (card));

  /* Remember the card for next time */
  g_signal_handlers_block_by_func (G_OBJECT (window->preferences), xfce_mixer_window_soundcard_property_changed, window);
  g_object_set (G_OBJECT (window->preferences), "sound-card", xfce_mixer_get_card_internal_name (card), NULL);
  g_signal_handlers_unblock_by_func (G_OBJECT (window->preferences), xfce_mixer_window_soundcard_property_changed, window);

  /* Update mixer controls for the active sound card */
  xfce_mixer_window_update_contents (window);

  /* Update the controls dialog */
  if (window->controls_dialog != NULL)
    xfce_mixer_controls_dialog_set_soundcard (XFCE_MIXER_CONTROLS_DIALOG (window->controls_dialog), card);
}



static void
xfce_mixer_window_soundcard_property_changed (XfceMixerWindow *window,
                                              GParamSpec      *pspec,
                                              GObject         *object)
{
  gchar       *new_card_name;
  GstElement  *new_card = NULL;
  GstElement  *old_card;

  g_return_if_fail (XFCE_IS_MIXER_WINDOW (window));
  g_return_if_fail (G_IS_OBJECT (object));

  g_object_get (object, "sound-card", &new_card_name, NULL);
  if (new_card_name != NULL)
    new_card = xfce_mixer_get_card (new_card_name);
  g_free (new_card_name);

  /* If the new card is not valid reset it to the default */
  if (!GST_IS_MIXER (new_card))
    {
      new_card = xfce_mixer_get_default_card ();

      if (GST_IS_MIXER (new_card))
        g_object_set (object, "sound-card", xfce_mixer_get_card_internal_name (new_card), NULL);

      return;
    }

  old_card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo));

  /* Only update if different from the active card */
  if (new_card != old_card)
    {
      /* Update the combobox */
      g_signal_handlers_block_by_func (G_OBJECT (window->soundcard_combo), xfce_mixer_window_soundcard_changed, window);
      xfce_mixer_card_combo_set_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo), new_card);
      g_signal_handlers_unblock_by_func (G_OBJECT (window->soundcard_combo), xfce_mixer_window_soundcard_changed, window);

      /* Update mixer controls for the active sound card */
      xfce_mixer_window_update_contents (window);

      /* Update the controls dialog */
      if (window->controls_dialog != NULL)
        xfce_mixer_controls_dialog_set_soundcard (XFCE_MIXER_CONTROLS_DIALOG (window->controls_dialog), new_card);
    }
}



static void
xfce_mixer_window_action_select_controls (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (user_data);

  g_return_if_fail (window->controls_dialog == NULL);

  window->controls_dialog = xfce_mixer_controls_dialog_new (window);

  gtk_dialog_run (GTK_DIALOG (window->controls_dialog));
  gtk_widget_destroy (window->controls_dialog);
  window->controls_dialog = NULL;
}



static void
xfce_mixer_window_controls_property_changed (XfceMixerWindow *window,
                                             GParamSpec      *pspec,
                                             GObject         *object)
{
  xfce_mixer_container_update_contents (XFCE_MIXER_CONTAINER (window->mixer_container));

  /* Update the controls dialog */
  if (window->controls_dialog != NULL)
    xfce_mixer_controls_dialog_update_dialog (XFCE_MIXER_CONTROLS_DIALOG (window->controls_dialog));
}



static void
xfce_mixer_window_show_about_dialog (GSimpleAction *action,
                                     GVariant      *parameter,
                                     gpointer       user_data)
{
  const gchar *auth[] = {
    "Ali Abdallah <ali.abdallah@suse.com>",
    "Landry Breuil <landry@xfce.org>",
    "Jannis Pohlmann <jannis@xfce.org>",
    "Guido Berhoerster <guido+xfce@berhoerster.name>",
    NULL
  };

  gtk_show_about_dialog (NULL,
    "logo-icon-name", "multimedia-volume-control",
    "license", xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
    "version", VERSION_FULL,
    "program-name", PACKAGE_NAME,
    "comments", _("Adjust volume levels"),
    "website", PACKAGE_URL,
    "copyright", "Copyright \302\251 2003-" COPYRIGHT_YEAR " The Xfce development team",
    "authors", auth, NULL);
}



static void
xfce_mixer_window_close (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  XfceMixerWindow *window = XFCE_MIXER_WINDOW (user_data);

  gtk_widget_destroy (GTK_WIDGET (window));

  g_application_quit (g_application_get_default ());
}



GstElement *
xfce_mixer_window_get_active_card (XfceMixerWindow *window)
{
  g_return_val_if_fail (XFCE_IS_MIXER_WINDOW (window), NULL);
  return xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo));
}



static void
xfce_mixer_window_update_contents (XfceMixerWindow *window)
{
  gchar       *title;
  GstElement  *card;

  g_return_if_fail (XFCE_IS_MIXER_WINDOW (window));

  card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (window->soundcard_combo));
  if (G_LIKELY (GST_IS_MIXER (card)))
    {
      title = g_strdup_printf ("%s - %s", _("Audio Mixer"), xfce_mixer_get_card_display_name (card));
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title);

      xfce_mixer_select_card (card);

      /* Update the XfceMixerContainer containing the controls */
      g_object_set (window->mixer_container, "card", card, NULL);

      /* Make the "Select Controls..." button sensitive */
      gtk_widget_set_sensitive (window->select_controls_button, TRUE);
    }
  else
    {
      gtk_window_set_title (GTK_WINDOW (window), _("Audio Mixer"));

      /* Update the XfceMixerContainer containing the controls */
      g_object_set (window->mixer_container, "card", NULL, NULL);

      /* Make the "Select Controls..." button insensitive */
      gtk_widget_set_sensitive (window->select_controls_button, FALSE);
    }
}

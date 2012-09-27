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

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <gtk/gtk.h>

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include <xfconf/xfconf.h>

#ifdef HAVE_KEYBINDER
#include <keybinder.h>
#endif

#include "xfce-mixer-plugin.h"

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-volume-button.h"
#include "xfce-plugin-dialog.h"



enum
{
  PROP_0,
  PROP_SOUND_CARD,
  PROP_TRACK,
  PROP_COMMAND,
#ifdef HAVE_KEYBINDER
  PROP_ENABLE_KEYBOARD_SHORTCUTS,
#endif
  N_PROPERTIES,
};



#ifdef HAVE_KEYBINDER
#define XFCE_MIXER_PLUGIN_RAISE_VOLUME_KEY  "XF86AudioRaiseVolume"
#define XFCE_MIXER_PLUGIN_LOWER_VOLUME_KEY  "XF86AudioLowerVolume"
#define XFCE_MIXER_PLUGIN_MUTE_KEY          "XF86AudioMute"
#endif



static void     xfce_mixer_plugin_construct                 (XfcePanelPlugin    *plugin);
static void     xfce_mixer_plugin_set_property              (GObject            *object,
                                                             guint               prop_id,
                                                             const GValue       *value,
                                                             GParamSpec         *pspec);
static void     xfce_mixer_plugin_get_property              (GObject            *object,
                                                             guint               prop_id,
                                                             GValue             *value,
                                                             GParamSpec         *pspec);
static void     xfce_mixer_plugin_free_data                 (XfcePanelPlugin    *plugin);
static void     xfce_mixer_plugin_configure_plugin          (XfcePanelPlugin    *plugin);
static gboolean xfce_mixer_plugin_size_changed              (XfcePanelPlugin    *plugin,
                                                             gint                size);
static void     xfce_mixer_plugin_screen_position_changed   (XfcePanelPlugin    *plugin,
                                                             XfceScreenPosition  screen_position);
static void     xfce_mixer_plugin_button_toggled            (XfceMixerPlugin    *mixer_plugin,
                                                             GtkToggleButton    *togglebutton);
static void     xfce_mixer_plugin_volume_changed            (XfceMixerPlugin    *mixer_plugin,
                                                             gdouble             volume);
static void     xfce_mixer_plugin_mute_changed              (XfceMixerPlugin    *mixer_plugin,
                                                             gboolean            muted);
static void     xfce_mixer_plugin_mute_item_toggled         (XfceMixerPlugin    *mixer_plugin,
                                                             GtkCheckMenuItem   *mute_menu_item);
static void     xfce_mixer_plugin_command_item_activated    (XfceMixerPlugin    *mixer_plugin,
                                                             GtkMenuItem        *menuitem);
static void     xfce_mixer_plugin_is_muted_property_changed (XfceMixerPlugin    *mixer_plugin,
                                                             GParamSpec         *pspec,
                                                             GObject            *object);
static void     xfce_mixer_plugin_update_track              (XfceMixerPlugin    *mixer_plugin);
static void     xfce_mixer_plugin_bus_message               (GstBus             *bus,
                                                             GstMessage         *message,
                                                             XfceMixerPlugin    *mixer_plugin);
#ifdef HAVE_KEYBINDER
static void     xfce_mixer_plugin_volume_key_pressed        (const char         *keystring,
                                                             void               *user_data);
static void     xfce_mixer_plugin_mute_pressed              (const char         *keystring,
                                                             void               *user_data);
#endif




struct _XfceMixerPluginClass
{
  /* Parent class */
  XfcePanelPluginClass __parent__;
};



struct _XfceMixerPlugin
{
  /* Parent type */
  XfcePanelPlugin __parent__;

  /* Sound card being used */
  GstElement      *card;
  gchar           *card_name;

  /* Mixer track being used */
  GstMixerTrack   *track;
  gchar           *track_label;

  /* Mixer command */
  gchar           *command;

#ifdef HAVE_KEYBINDER
  /* Keyboard shortcuts */
  gboolean         enable_keyboard_shortcuts;
#endif

  /* Widgets */
  GtkWidget       *hvbox;
  GtkWidget       *button;
  GtkWidget       *mute_menu_item;

  /* Reference to the plugin private xfconf channel */
  XfconfChannel   *plugin_channel;

  /* Flag for ignoring messages from the GstBus */
  gboolean         ignore_bus_messages;

  /* GstBus connection id */
  gulong           message_handler_id;
};



/* Define the plugin */
XFCE_PANEL_DEFINE_PLUGIN (XfceMixerPlugin, xfce_mixer_plugin)



static void
xfce_mixer_plugin_class_init (XfceMixerPluginClass *klass)
{
  GObjectClass         *gobject_class;
  XfcePanelPluginClass *plugin_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = xfce_mixer_plugin_get_property;
  gobject_class->set_property = xfce_mixer_plugin_set_property;

  plugin_class = XFCE_PANEL_PLUGIN_CLASS (klass);
  plugin_class->construct = xfce_mixer_plugin_construct;
  plugin_class->free_data = xfce_mixer_plugin_free_data;
  plugin_class->size_changed = xfce_mixer_plugin_size_changed;
  plugin_class->screen_position_changed = xfce_mixer_plugin_screen_position_changed;
  plugin_class->configure_plugin = xfce_mixer_plugin_configure_plugin;

  g_object_class_install_property (gobject_class,
                                   PROP_SOUND_CARD,
                                   g_param_spec_string ("sound-card",
                                                        "sound-card",
                                                        "sound-card",
                                                        NULL,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_TRACK,
                                   g_param_spec_string ("track",
                                                        "track",
                                                        "track",
                                                        NULL,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_COMMAND,
                                   g_param_spec_string ("command",
                                                        "command",
                                                        "command",
                                                        NULL,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));

#ifdef HAVE_KEYBINDER
  g_object_class_install_property (gobject_class,
                                   PROP_ENABLE_KEYBOARD_SHORTCUTS,
                                   g_param_spec_boolean ("enable-keyboard-shortcuts",
                                                         "enable-keyboard-shortcuts",
                                                         "enable-keyboard-shortcuts",
                                                         TRUE,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));
#endif
}



static void
xfce_mixer_plugin_init (XfceMixerPlugin *mixer_plugin)
{
  gboolean     debug_mode = FALSE;
  const gchar *panel_debug_env;

  /* Initialize some of the plugin variables */
  mixer_plugin->card = NULL;
  mixer_plugin->track = NULL;
  mixer_plugin->track_label = NULL;
  mixer_plugin->command = NULL;
#ifdef HAVE_KEYBINDER
  mixer_plugin->enable_keyboard_shortcuts = FALSE;
#endif

  mixer_plugin->plugin_channel = NULL;

  mixer_plugin->ignore_bus_messages = FALSE;
  mixer_plugin->message_handler_id = 0;

  mixer_plugin->mute_menu_item = NULL;

  /* Initialize xfconf */
  xfconf_init (NULL);

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Initialize the mixer library */
  xfce_mixer_init ();

#ifdef HAVE_KEYBINDER
  /* Initialize libkeybinder */
  keybinder_init ();
#endif

  /* Enable debug level logging if PANEL_DEBUG contains G_LOG_DOMAIN */
  panel_debug_env = g_getenv ("PANEL_DEBUG");
  if (panel_debug_env != NULL && strstr (panel_debug_env, G_LOG_DOMAIN) != NULL)
    debug_mode = TRUE;
  xfce_mixer_debug_init (G_LOG_DOMAIN, debug_mode);

  xfce_mixer_debug ("mixer plugin version " VERSION " starting up");

  if (debug_mode)
    xfce_mixer_dump_gst_data ();

  /* Create container for the plugin */
  mixer_plugin->hvbox = GTK_WIDGET (xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0));
  xfce_panel_plugin_add_action_widget (XFCE_PANEL_PLUGIN (mixer_plugin), mixer_plugin->hvbox);
  gtk_container_add (GTK_CONTAINER (mixer_plugin), mixer_plugin->hvbox);
  gtk_widget_show (mixer_plugin->hvbox);

  /* Create volume button for the plugin */
  mixer_plugin->button = xfce_volume_button_new ();
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "volume-changed", G_CALLBACK (xfce_mixer_plugin_volume_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "notify::is-muted", G_CALLBACK (xfce_mixer_plugin_is_muted_property_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "toggled", G_CALLBACK (xfce_mixer_plugin_button_toggled), mixer_plugin);
  gtk_container_add (GTK_CONTAINER (mixer_plugin->hvbox), mixer_plugin->button);
  gtk_widget_show (mixer_plugin->button);

  /* Let the volume button receive mouse events */
  xfce_panel_plugin_add_action_widget (XFCE_PANEL_PLUGIN (mixer_plugin), mixer_plugin->button);
}



static void
xfce_mixer_plugin_construct (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);
  GtkWidget       *command_menu_item;
  GtkWidget       *command_image;

  xfce_panel_plugin_menu_show_configure (plugin);

  /* Add menu item for muting */
  mixer_plugin->mute_menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Mute"));
  xfce_panel_plugin_menu_insert_item (plugin, GTK_MENU_ITEM (mixer_plugin->mute_menu_item));
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->mute_menu_item), "toggled", G_CALLBACK (xfce_mixer_plugin_mute_item_toggled), mixer_plugin);
  gtk_widget_show (mixer_plugin->mute_menu_item);

  /* Add menu item for running the user-defined command */
  command_image = gtk_image_new_from_icon_name ("multimedia-volume-control", GTK_ICON_SIZE_MENU);
  gtk_widget_show (command_image);
  command_menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Run Audio Mixer"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (command_menu_item), command_image);
  xfce_panel_plugin_menu_insert_item (plugin, GTK_MENU_ITEM (command_menu_item));
  g_signal_connect_swapped (G_OBJECT (command_menu_item), "activate", G_CALLBACK (xfce_mixer_plugin_command_item_activated), mixer_plugin);
  gtk_widget_show (command_menu_item);

  /* Only occupy a single row in deskbar mode */
  xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (mixer_plugin), TRUE);

  /* Set up xfconf channel */
  mixer_plugin->plugin_channel = xfconf_channel_new_with_property_base (xfce_panel_get_channel_name (), xfce_panel_plugin_get_property_base (plugin));

  /* Try to set properties to defaults */
  g_object_set (G_OBJECT (mixer_plugin), "sound-card", NULL, "command", NULL,
#ifdef HAVE_KEYBINDER
                "enable-keyboard-shortcuts", TRUE,
#endif
                NULL);

  /* Set up xfconf property bindings */
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/sound-card", G_TYPE_STRING, mixer_plugin, "sound-card");
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/track", G_TYPE_STRING, mixer_plugin, "track");
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/command", G_TYPE_STRING, mixer_plugin, "command");
#ifdef HAVE_KEYBINDER
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/enable-keyboard-shortcuts", G_TYPE_BOOLEAN, mixer_plugin, "enable-keyboard-shortcuts");
#endif

  /* Make sure the properties are in xfconf */
  g_object_notify (G_OBJECT (mixer_plugin), "sound-card");
  g_object_notify (G_OBJECT (mixer_plugin), "track");
  g_object_notify (G_OBJECT (mixer_plugin), "command");
#ifdef HAVE_KEYBINDER
  g_object_notify (G_OBJECT (mixer_plugin), "enable-keyboard-shortcuts");
#endif
}



static void
xfce_mixer_plugin_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  XfceMixerPlugin    *mixer_plugin = XFCE_MIXER_PLUGIN (object);
  const gchar        *card_name;
  GstElement         *card = NULL;
  gchar              *track_label = NULL;
  GstMixerTrack      *track = NULL;
  XfceMixerTrackType  track_type = G_TYPE_INVALID;
  gboolean            enable_keyboard_shortcuts;

  switch(prop_id)
    {
      case PROP_SOUND_CARD:
        /* Freeze "notify" signals since the "track" property is manipulated */
        g_object_freeze_notify (object);

        g_free (mixer_plugin->card_name);
        mixer_plugin->card_name = NULL;
        mixer_plugin->card = NULL;

        card_name = g_value_get_string (value);
        if (card_name != NULL)
            card = xfce_mixer_get_card (card_name);

        /* If the given card name is invalid resort to the default */
        if (!GST_IS_MIXER (card))
          {
            card = xfce_mixer_get_default_card ();
            if GST_IS_MIXER (card)
              card_name = xfce_mixer_get_card_internal_name (card);
            else
              card_name = NULL;
          }

        if (GST_IS_MIXER (card))
          {
            mixer_plugin->card = card;
            mixer_plugin->card_name = g_strdup (card_name);
            xfce_mixer_select_card (mixer_plugin->card);
            mixer_plugin->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_plugin_bus_message), mixer_plugin);
            track_label = xfconf_channel_get_string (mixer_plugin->plugin_channel, "/track", NULL);
          }
        else
          {
            track_label = NULL;
            xfce_mixer_bus_disconnect (mixer_plugin->message_handler_id);
          }
        g_object_set (object, "track", track_label, NULL);

        g_free (track_label);

        g_object_thaw_notify (object);
        break;
      case PROP_TRACK:
        g_free (mixer_plugin->track_label);
        mixer_plugin->track_label = NULL;
        mixer_plugin->track = NULL;

        if (GST_IS_MIXER (mixer_plugin->card))
          {
            track_label = g_value_dup_string (value);
            if (track_label != NULL)
              track = xfce_mixer_get_track (mixer_plugin->card, track_label);

            if (GST_IS_MIXER_TRACK (track))
              track_type = xfce_mixer_track_type_new (track);

            /*
             * If the given track is invalid or not a playback or capture track
             * or read-only resort to the default
             */
            if (!GST_IS_MIXER_TRACK (track) ||
                (track_type != XFCE_MIXER_TRACK_TYPE_PLAYBACK &&
                 track_type != XFCE_MIXER_TRACK_TYPE_CAPTURE) ||
                GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_READONLY))
              {
                g_free (track_label);
                track = xfce_mixer_get_default_track (mixer_plugin->card);
                if (GST_IS_MIXER_TRACK (track))
                  track_label = g_strdup (xfce_mixer_get_track_label (track));
                else
                  track_label = NULL;
              }

            if (GST_IS_MIXER_TRACK (track))
              {
                mixer_plugin->track = track;
                mixer_plugin->track_label = g_strdup (track_label);
              }

            g_free (track_label);
          }

        xfce_mixer_plugin_update_track (mixer_plugin);
        break;
      case PROP_COMMAND:
        g_free (mixer_plugin->command);

        mixer_plugin->command = g_value_dup_string (value);
        if (mixer_plugin->command == NULL)
          mixer_plugin->command = g_strdup (XFCE_MIXER_PLUGIN_DEFAULT_COMMAND);
        break;
#ifdef HAVE_KEYBINDER
      case PROP_ENABLE_KEYBOARD_SHORTCUTS:
        enable_keyboard_shortcuts = g_value_get_boolean (value);

        if (mixer_plugin->enable_keyboard_shortcuts != enable_keyboard_shortcuts)
          {
            if (enable_keyboard_shortcuts)
              {
                /* Set up global keyboard shortcuts */
                keybinder_bind(XFCE_MIXER_PLUGIN_LOWER_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed, mixer_plugin);
                keybinder_bind(XFCE_MIXER_PLUGIN_RAISE_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed, mixer_plugin);
                keybinder_bind(XFCE_MIXER_PLUGIN_MUTE_KEY, xfce_mixer_plugin_mute_pressed, mixer_plugin);
              }
            else
              {
                /* Remove global keyboard shortcuts */
                keybinder_unbind(XFCE_MIXER_PLUGIN_LOWER_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed);
                keybinder_unbind(XFCE_MIXER_PLUGIN_RAISE_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed);
                keybinder_unbind(XFCE_MIXER_PLUGIN_MUTE_KEY, xfce_mixer_plugin_mute_pressed);
              }
            mixer_plugin->enable_keyboard_shortcuts = enable_keyboard_shortcuts;
          }
        break;
#endif
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



static void
xfce_mixer_plugin_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (object);

  switch(prop_id)
    {
      case PROP_SOUND_CARD:
        g_value_set_string (value, mixer_plugin->card_name);
        break;
      case PROP_TRACK:
        g_value_set_string (value, mixer_plugin->track_label);
        break;
      case PROP_COMMAND:
        g_value_set_string (value, mixer_plugin->command);
        break;
#ifdef HAVE_KEYBINDER
      case PROP_ENABLE_KEYBOARD_SHORTCUTS:
        g_value_set_boolean (value, mixer_plugin->enable_keyboard_shortcuts);
        break;
#endif
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



static void
xfce_mixer_plugin_free_data (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);

#ifdef HAVE_KEYBINDER
  if (mixer_plugin->enable_keyboard_shortcuts)
    {
      /* Remove global keyboard shortcuts */
      keybinder_unbind(XFCE_MIXER_PLUGIN_LOWER_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed);
      keybinder_unbind(XFCE_MIXER_PLUGIN_RAISE_VOLUME_KEY, xfce_mixer_plugin_volume_key_pressed);
      keybinder_unbind(XFCE_MIXER_PLUGIN_MUTE_KEY, xfce_mixer_plugin_mute_pressed);
    }
#endif

  /* Shutdown xfconf */
  g_object_unref (mixer_plugin->plugin_channel);
  xfconf_shutdown ();

  /* Free card and track names */
  g_free (mixer_plugin->command);
  g_free (mixer_plugin->card_name);
  g_free (mixer_plugin->track_label);

  /* Disconnect from GstBus */
  xfce_mixer_bus_disconnect (mixer_plugin->message_handler_id);

  /* Shutdown the mixer library */
  xfce_mixer_shutdown ();
}



static void
xfce_mixer_plugin_configure_plugin (XfcePanelPlugin *plugin)
 {
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);
  GtkWidget       *dialog;

  g_return_if_fail (mixer_plugin != NULL);

  /* Block the panel menu as long as the config dialog is shown */
  xfce_panel_plugin_block_menu (plugin);

  /* Warn user if no sound cards are available */
  if (G_UNLIKELY (g_list_length (xfce_mixer_get_cards ()) <= 0))
    {
      xfce_dialog_show_error (NULL,
                              NULL,
                              _("GStreamer was unable to detect any sound devices. "
                              "Some sound system specific GStreamer packages may "
                              "be missing. It may also be a permissions problem.")); 

    }
  else
    {
      /* Create and run the config dialog */
      dialog = xfce_plugin_dialog_new (plugin);
      gtk_dialog_run (GTK_DIALOG (dialog));

      /* Destroy the config dialog */
      gtk_widget_destroy (dialog);
    }

  /* Make the plugin menu accessable again */
  xfce_panel_plugin_unblock_menu (plugin);
}



static gboolean
xfce_mixer_plugin_size_changed (XfcePanelPlugin *plugin,
                                gint             size)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);

  g_return_val_if_fail (mixer_plugin != NULL, FALSE);

  /* The plugin only occupies a single row */
  size /= xfce_panel_plugin_get_nrows (XFCE_PANEL_PLUGIN (mixer_plugin));

  /* Determine size for the volume button icons */
  size -= 2 + 2 * MAX (mixer_plugin->button->style->xthickness, mixer_plugin->button->style->ythickness);

  /* Set volume button icon size and update the volume button */
  xfce_volume_button_set_icon_size (XFCE_VOLUME_BUTTON (mixer_plugin->button), size);
  xfce_volume_button_update (XFCE_VOLUME_BUTTON (mixer_plugin->button));

  return TRUE;
}



static void
xfce_mixer_plugin_screen_position_changed (XfcePanelPlugin    *plugin,
                                           XfceScreenPosition  screen_position)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);

  g_return_if_fail (IS_XFCE_MIXER_PLUGIN (mixer_plugin));
  g_return_if_fail (GTK_IS_WIDGET (mixer_plugin->button));

  xfce_volume_button_set_screen_position (XFCE_VOLUME_BUTTON (mixer_plugin->button), screen_position);
}



static void
xfce_mixer_plugin_button_toggled (XfceMixerPlugin *mixer_plugin,
                                  GtkToggleButton *togglebutton)
{
  gboolean active;

  g_object_get (G_OBJECT (togglebutton), "active", &active, NULL);

  /* Block autohide while the dock is shown */
  xfce_panel_plugin_block_autohide (XFCE_PANEL_PLUGIN (mixer_plugin), active);
}



static void
xfce_mixer_plugin_volume_changed (XfceMixerPlugin  *mixer_plugin,
                                  gdouble           volume)
{
  gint  *volumes;
  gint   volume_range;
  gint   new_volume;
  gint   i;

  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  mixer_plugin->ignore_bus_messages = TRUE;

  /* Allocate array for track volumes */
  volumes = g_new (gint, mixer_plugin->track->num_channels);

  /* Determine difference between max and min volume */
  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Determine new volume */
  new_volume = (gint) round (mixer_plugin->track->min_volume + (volume * volume_range));

  /* Set all channel volumes to the new volume */
  for (i = 0; i < mixer_plugin->track->num_channels; ++i)
    volumes[i] = new_volume;

  /* Apply volume change to the sound card */
  gst_mixer_set_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

  /* Free volume array */
  g_free (volumes);

  mixer_plugin->ignore_bus_messages = FALSE;
}



static void
xfce_mixer_plugin_mute_changed (XfceMixerPlugin *mixer_plugin,
                                gboolean         muted)
{
  XfceMixerTrackType track_type;

  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  track_type = xfce_mixer_track_type_new (mixer_plugin->track);

  if ((track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK &&
       GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_NO_MUTE)) ||
      (track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE &&
       GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_NO_RECORD)))
    return;

  mixer_plugin->ignore_bus_messages = TRUE;

  if (G_LIKELY (track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK))
    {
      /* Apply mute change to the sound card */
      gst_mixer_set_mute (GST_MIXER (mixer_plugin->card), mixer_plugin->track, muted);
    }
  else
    {
      /* Toggle capture */
      gst_mixer_set_record (GST_MIXER (mixer_plugin->card), mixer_plugin->track, !muted);
    }

  /* Update mute menu item */
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item)) != muted)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), muted);

  mixer_plugin->ignore_bus_messages = FALSE;
}



static void
xfce_mixer_plugin_mute_item_toggled (XfceMixerPlugin  *mixer_plugin,
                                     GtkCheckMenuItem *mute_menu_item)
{
  gboolean muted = gtk_check_menu_item_get_active (mute_menu_item);

  /* Update the volume button */
  xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), muted);

  xfce_mixer_plugin_mute_changed (mixer_plugin, muted);
}



static void
xfce_mixer_plugin_command_item_activated (XfceMixerPlugin *mixer_plugin,
                                          GtkMenuItem     *menuitem)
{
  gchar *message;

  g_return_if_fail (mixer_plugin != NULL);

  if (G_UNLIKELY (mixer_plugin->command == NULL || strlen (mixer_plugin->command) == 0))
    {
      xfce_dialog_show_error (NULL, NULL, _("No command defined"));
      return;
    }

  /* Try to start the mixer command */
  if (G_UNLIKELY (!g_spawn_command_line_async (mixer_plugin->command, NULL)))
    {
      /* Generate error message and insert the current command */
      message = g_strdup_printf (_("Could not execute the command \"%s\". "
                                   "Ensure that either the location of the command "
                                   "is included in the PATH environment variable or "
                                   "that you are providing the full path to the "
                                   "command."), 
                                 mixer_plugin->command);

      /* Display error */
      xfce_dialog_show_error (NULL, NULL, "%s", message); 

      /* Free error message */
      g_free (message);
    }
}



static void
xfce_mixer_plugin_is_muted_property_changed (XfceMixerPlugin *mixer_plugin,
                                             GParamSpec      *pspec,
                                             GObject         *object)
{
  gboolean muted;

  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  g_object_get (object, "is-muted", &muted, NULL);

  xfce_mixer_plugin_mute_changed (mixer_plugin, muted);
}



static void
xfce_mixer_plugin_update_track (XfceMixerPlugin *mixer_plugin)
{
  XfceMixerTrackType track_type;
  gboolean           muted = FALSE;
  gint               volume_range;
  gdouble            volume;
  gint              *volumes;

  g_return_if_fail (IS_XFCE_MIXER_PLUGIN (mixer_plugin));

  /* Set the volume button to invalid state and return if the card or track is invalid */
  if (!GST_IS_MIXER (mixer_plugin->card) || !GST_IS_MIXER_TRACK (mixer_plugin->track))
    {
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), FALSE);
      xfce_volume_button_set_is_configured (XFCE_VOLUME_BUTTON (mixer_plugin->button), FALSE);
      return;
    }

  /* Get volumes of the mixer track */
  volumes = g_new (gint, mixer_plugin->track->num_channels);
  gst_mixer_get_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

  /* Determine difference between max and min volume */
  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Determine maximum value as double between 0.0 and 1.0 */
  volume = ((gdouble) xfce_mixer_get_max_volume (volumes, mixer_plugin->track->num_channels) - mixer_plugin->track->min_volume) / volume_range;

  /* Determine track type */
  track_type = xfce_mixer_track_type_new (mixer_plugin->track);

  if (G_LIKELY (track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK))
    muted = GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_MUTE);
  else if (track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
    muted = !GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_RECORD);

  /* Update the volume button and menu */
  xfce_volume_button_set_is_configured (XFCE_VOLUME_BUTTON (mixer_plugin->button), TRUE);
  xfce_volume_button_set_track_label (XFCE_VOLUME_BUTTON (mixer_plugin->button), xfce_mixer_get_track_label (mixer_plugin->track));
  xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), volume);

  /*
   * If the track does not support mute/record, disable the corresponding menu
   * item and button functionality
   */
  if ((track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK &&
       GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_NO_MUTE)) ||
      (track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE &&
       GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_NO_RECORD)))
    {
      xfce_volume_button_set_no_mute (XFCE_VOLUME_BUTTON (mixer_plugin->button), TRUE);
      xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), FALSE);
      gtk_widget_set_sensitive (mixer_plugin->mute_menu_item, FALSE);
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), FALSE);
    }
  else
    {
      xfce_volume_button_set_no_mute (XFCE_VOLUME_BUTTON (mixer_plugin->button), FALSE);
      xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), muted);
      gtk_widget_set_sensitive (mixer_plugin->mute_menu_item, TRUE);
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), muted);
    }

  /* Free volume array */
  g_free (volumes);
}



static void
xfce_mixer_plugin_bus_message (GstBus          *bus,
                               GstMessage      *message,
                               XfceMixerPlugin *mixer_plugin)
{
  GstMixerTrack *track = NULL;
  gboolean      mute;
  gboolean      record;
  const gchar   *label;

  /* Don't do anything if GstBus messages are to be ignored */
  if (G_UNLIKELY (mixer_plugin->ignore_bus_messages))
    return;

  if (G_UNLIKELY (!GST_IS_MIXER (mixer_plugin->card) || !GST_IS_MIXER_TRACK (mixer_plugin->track) || mixer_plugin->track_label == NULL))
    return;

  if (G_UNLIKELY (GST_MESSAGE_SRC (message) != GST_OBJECT (mixer_plugin->card)))
    return;

  switch (gst_mixer_message_get_type (message))
    {
      case GST_MIXER_MESSAGE_VOLUME_CHANGED:
        /* Get the track of the volume changed message */
        gst_mixer_message_parse_volume_changed (message, &track, NULL, NULL);
        label = xfce_mixer_get_track_label (track);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          xfce_mixer_plugin_update_track (mixer_plugin);

        break;
      case GST_MIXER_MESSAGE_MUTE_TOGGLED:
        /* Parse the mute message */
        gst_mixer_message_parse_mute_toggled (message, &track, &mute);
        label = xfce_mixer_get_track_label (track);

        /* Update the volume button and mute menu item if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          {
            xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), mute);
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), mute);
          }

        break;
      case GST_MIXER_MESSAGE_RECORD_TOGGLED:
        /* Parse the record message */
        gst_mixer_message_parse_record_toggled (message, &track, &record);
        label = xfce_mixer_get_track_label (track);

        /* Update the volume button and mute menu item if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          {
            xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), !record);
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (mixer_plugin->mute_menu_item), !record);
          }

        break;
      case GST_MIXER_MESSAGE_MIXER_CHANGED:
        /*
         * If the mixer tracks have changed, try to keep the current track by
         * selecting a track based on the current track name; if there is no
         * track with such name any more, the property setter will handle the
         * situation in a sane way
         */
        g_object_set (mixer_plugin, "track", mixer_plugin->track_label, NULL);
        break;
      default:
        break;
    }
}



#ifdef HAVE_KEYBINDER
static void
xfce_mixer_plugin_volume_key_pressed (const char *keystring,
                                      void       *user_data)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (user_data);
  gint             volume_range;
  gint             interval;
  gint             i;
  gint            *old_volumes;
  gint            *new_volumes;
  gint             old_volume;
  gint             new_volume;
  gdouble          button_volume;

  if (G_UNLIKELY (!GST_IS_MIXER (mixer_plugin->card) || !GST_IS_MIXER_TRACK (mixer_plugin->track) || mixer_plugin->track_label == NULL))
    return;

  /* Get volumes of the mixer track */
  old_volumes = g_new (gint, mixer_plugin->track->num_channels);
  gst_mixer_get_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, old_volumes);
  old_volume = xfce_mixer_get_max_volume (old_volumes, mixer_plugin->track->num_channels);
  g_free (old_volumes);

  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Increase/Decrease in intervals of 5% of the volume range but at least 1 */
  interval = (gint) round (volume_range * 0.05);
  if (interval == 0)
    interval = 1;

  if (strcmp (keystring, XFCE_MIXER_PLUGIN_RAISE_VOLUME_KEY) == 0)
    {
      /* Determine new volume */
      new_volume = old_volume + interval;
      if (new_volume > mixer_plugin->track->max_volume)
        new_volume = mixer_plugin->track->max_volume;
    }
  else if (strcmp (keystring, XFCE_MIXER_PLUGIN_LOWER_VOLUME_KEY) == 0)
    {
      /* Determine new volume */
      new_volume = old_volume - interval;
      if (new_volume < mixer_plugin->track->min_volume)
        new_volume = mixer_plugin->track->min_volume;
    }
  else
    return;

  if (new_volume != old_volume)
    {
      mixer_plugin->ignore_bus_messages = TRUE;

      /* Calculate volume as double between 0.0 and 1.0 for the button */
      button_volume = ((gdouble) new_volume - mixer_plugin->track->min_volume) / volume_range;
      xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), button_volume);

      /* Apply volume change to the sound card */
      new_volumes = g_new (gint, mixer_plugin->track->num_channels);
      for (i = 0; i < mixer_plugin->track->num_channels; ++i)
        new_volumes[i] = new_volume;
      gst_mixer_set_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, new_volumes);
      g_free (new_volumes);

      /* Mute when volume reaches 0%, unmute if volume is raised from 0% */
      if (old_volume > 0 && new_volume == 0)
        {
          xfce_mixer_plugin_mute_changed (mixer_plugin, TRUE);
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), TRUE);
        }
      else if (old_volume == 0 && new_volume > 0)
        {
          xfce_mixer_plugin_mute_changed (mixer_plugin, FALSE);
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), FALSE);
        }

      mixer_plugin->ignore_bus_messages = FALSE;
    }
}



static void
xfce_mixer_plugin_mute_pressed (const char *keystring,
                                void       *user_data)
{
  XfceMixerPlugin    *mixer_plugin = XFCE_MIXER_PLUGIN (user_data);
  XfceMixerTrackType  track_type;
  gboolean            muted = TRUE;

  if (G_UNLIKELY (!GST_IS_MIXER (mixer_plugin->card) || !GST_IS_MIXER_TRACK (mixer_plugin->track) || mixer_plugin->track_label == NULL))
    return;

  /* Determine track type */
  track_type = xfce_mixer_track_type_new (mixer_plugin->track);

  /* Determine current mute state */
  if (G_LIKELY (track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK))
    muted = GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_MUTE);
  else if (track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
    muted = !GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_RECORD);

  /* Set the new mute state */
  xfce_mixer_plugin_mute_changed (mixer_plugin, !muted);

  /* Update the volume button */
  xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), !muted);
}
#endif

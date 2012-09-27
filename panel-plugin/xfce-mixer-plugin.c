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

#include <gtk/gtk.h>

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include <xfconf/xfconf.h>

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
  N_PROPERTIES,
};



static void     xfce_mixer_plugin_construct                   (XfcePanelPlugin  *plugin);
static void     xfce_mixer_plugin_set_property                (GObject          *object,
                                                               guint             prop_id,
                                                               const GValue     *value,
                                                               GParamSpec       *pspec);
static void     xfce_mixer_plugin_get_property                (GObject          *object,
                                                               guint             prop_id,
                                                               GValue           *value,
                                                               GParamSpec       *pspec);
static void     xfce_mixer_plugin_free_data                   (XfcePanelPlugin  *plugin);
static void     xfce_mixer_plugin_configure_plugin            (XfcePanelPlugin  *plugin);
static gboolean xfce_mixer_plugin_size_changed                (XfcePanelPlugin  *plugin,
                                                               gint              size);
static void     xfce_mixer_plugin_clicked                     (XfceMixerPlugin  *mixer_plugin);
static void     xfce_mixer_plugin_volume_changed              (XfceMixerPlugin  *mixer_plugin,
                                                               gdouble           volume);
static void     xfce_mixer_plugin_mute_toggled                (XfceMixerPlugin  *mixer_plugin,
                                                               gboolean          mute);
static void     xfce_mixer_plugin_update_track                (XfceMixerPlugin  *mixer_plugin);
#ifdef HAVE_GST_MIXER_NOTIFICATION
static void     xfce_mixer_plugin_bus_message                 (GstBus           *bus,
                                                               GstMessage       *message,
                                                               XfceMixerPlugin  *mixer_plugin);
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

  /* Tooltips structure */
  GtkTooltips     *tooltips;

  /* Sound card being used */
  GstElement      *card;
  gchar           *card_name;

  /* Mixer track being used */
  GstMixerTrack   *track;
  gchar           *track_label;

  /* Mixer command */
  gchar           *command;

  /* Widgets */
  GtkWidget       *hvbox;
  GtkWidget       *button;

  /* Reference to the plugin private xfconf channel */
  XfconfChannel   *plugin_channel;

#ifdef HAVE_GST_MIXER_NOTIFICATION
  /* Flag for ignoring messages from the GstBus */
  gboolean         ignore_bus_messages;

  /* GstBus connection id */
  guint            message_handler_id;
#endif
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
}



static void
xfce_mixer_plugin_init (XfceMixerPlugin *mixer_plugin)
{
  /* Initialize some of the plugin variables */
  mixer_plugin->card = NULL;
  mixer_plugin->track = NULL;
  mixer_plugin->track_label = NULL;
  mixer_plugin->command = NULL;

  mixer_plugin->plugin_channel = NULL;

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = FALSE;
  mixer_plugin->message_handler_id = 0;
#endif

  /* Initialize xfconf */
  xfconf_init (NULL);

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Initialize the mixer library */
  xfce_mixer_init ();

  /* Allocate a tooltips structure */
  mixer_plugin->tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_delay (mixer_plugin->tooltips, 10);

  /* Create container for the plugin */
  mixer_plugin->hvbox = GTK_WIDGET (xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0));
  xfce_panel_plugin_add_action_widget (XFCE_PANEL_PLUGIN (mixer_plugin), mixer_plugin->hvbox);
  gtk_container_add (GTK_CONTAINER (mixer_plugin), mixer_plugin->hvbox);
  gtk_widget_show (mixer_plugin->hvbox);

  /* Create volume button for the plugin */
  mixer_plugin->button = xfce_volume_button_new ();
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "volume-changed", G_CALLBACK (xfce_mixer_plugin_volume_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "mute-toggled", G_CALLBACK (xfce_mixer_plugin_mute_toggled), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "clicked", G_CALLBACK (xfce_mixer_plugin_clicked), mixer_plugin);
  gtk_container_add (GTK_CONTAINER (mixer_plugin->hvbox), mixer_plugin->button);
  gtk_widget_show (mixer_plugin->button);

  /* Let the volume button receive mouse events */
  xfce_panel_plugin_add_action_widget (XFCE_PANEL_PLUGIN (mixer_plugin), mixer_plugin->button);
}



static void
xfce_mixer_plugin_construct (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);

  xfce_panel_plugin_menu_show_configure (plugin);

  /* Only occupy a single row in deskbar mode */
  xfce_panel_plugin_set_small (XFCE_PANEL_PLUGIN (mixer_plugin), TRUE);

  /* Set up xfconf property bindings */
  mixer_plugin->plugin_channel = xfconf_channel_new_with_property_base (xfce_panel_get_channel_name (), xfce_panel_plugin_get_property_base (plugin));
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/sound-card", G_TYPE_STRING, mixer_plugin, "sound-card");
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/track", G_TYPE_STRING, mixer_plugin, "track");
  xfconf_g_property_bind (mixer_plugin->plugin_channel, "/command", G_TYPE_STRING, mixer_plugin, "command");
}



static void
xfce_mixer_plugin_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (object);
  const gchar     *card_name;
  GstElement      *card = NULL;
  const gchar     *track_label;
  gchar           *new_track_label;
  GstMixerTrack   *track = NULL;

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

        if (GST_IS_MIXER (card))
          {
            mixer_plugin->card = card;
            mixer_plugin->card_name = g_strdup (card_name);
            xfce_mixer_select_card (mixer_plugin->card);
#ifdef HAVE_GST_MIXER_NOTIFICATION
            mixer_plugin->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_plugin_bus_message), mixer_plugin);
#endif
            new_track_label = xfconf_channel_get_string (mixer_plugin->plugin_channel, "/track", NULL);
          }
        else
          {
            new_track_label = NULL;
#ifdef HAVE_GST_MIXER_NOTIFICATION
            xfce_mixer_bus_disconnect (mixer_plugin->message_handler_id);
#endif
          }
        g_object_set (object, "track", new_track_label, NULL);

        g_free (new_track_label);

        g_object_thaw_notify (object);
        break;
      case PROP_TRACK:
        g_free (mixer_plugin->track_label);
        mixer_plugin->track_label = NULL;
        mixer_plugin->track = NULL;

        if (GST_IS_MIXER (mixer_plugin->card))
          {
            track_label = g_value_get_string (value);
            if (track_label != NULL)
              track = xfce_mixer_get_track (mixer_plugin->card, track_label);

            if (GST_IS_MIXER_TRACK (track))
              {
                mixer_plugin->track = track;
                mixer_plugin->track_label = g_strdup (track_label);
              }
          }

        xfce_mixer_plugin_update_track (mixer_plugin);
        break;
      case PROP_COMMAND:
        g_free (mixer_plugin->command);
        mixer_plugin->command = g_value_dup_string (value);
        if (mixer_plugin->command == NULL)
          mixer_plugin->command = g_strdup (XFCE_MIXER_PLUGIN_DEFAULT_COMMAND);
        break;
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
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}



static void
xfce_mixer_plugin_free_data (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin = XFCE_MIXER_PLUGIN (plugin);

  /* Shutdown xfconf */
  g_object_unref (mixer_plugin->plugin_channel);
  xfconf_shutdown ();

  /* Free card and track names */
  g_free (mixer_plugin->command);
  g_free (mixer_plugin->card_name);
  g_free (mixer_plugin->track_label);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  /* Disconnect from GstBus */
  xfce_mixer_bus_disconnect (mixer_plugin->message_handler_id);
#endif

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
xfce_mixer_plugin_clicked (XfceMixerPlugin *mixer_plugin)
{
  gchar *message;
  gint   response;

  g_return_if_fail (mixer_plugin != NULL);

  if (G_UNLIKELY (mixer_plugin->command == NULL || strlen (mixer_plugin->command) == 0))
    {
      /* Run error message dialog */
      response = xfce_message_dialog (NULL,
                                      _("No left-click command defined"),
                                      GTK_STOCK_DIALOG_ERROR,
                                      NULL,
                                      _("No left-click command defined yet. You can change this in the plugin properties."),
                                      XFCE_BUTTON_TYPE_MIXED, _("Properties"), GTK_STOCK_PREFERENCES, GTK_RESPONSE_ACCEPT,
                                      GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT,
                                      NULL);

      /* Configure the plugin if requested by the user */
      if (G_LIKELY (response == GTK_RESPONSE_ACCEPT))
        xfce_mixer_plugin_configure_plugin (XFCE_PANEL_PLUGIN (mixer_plugin));

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
xfce_mixer_plugin_volume_changed (XfceMixerPlugin  *mixer_plugin,
                                  gdouble           volume)
{
  gchar *tip_text;
  gint  *volumes;
  gint   volume_range;
  gint   new_volume;
  gint   i;

  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = TRUE;
#endif

  /* Set tooltip (e.g. 'Master: 50%') */
  tip_text = g_strdup_printf (_("%s: %i%%"), mixer_plugin->track_label, (gint) (volume * 100));
  gtk_tooltips_set_tip (mixer_plugin->tooltips, mixer_plugin->button, tip_text, "test");
  g_free (tip_text);

  /* Allocate array for track volumes */
  volumes = g_new (gint, mixer_plugin->track->num_channels);

  /* Determine difference between max and min volume */
  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Determine new volume */
  new_volume = mixer_plugin->track->min_volume + (volume * volume_range);

  /* Set all channel volumes to the new volume */
  for (i = 0; i < mixer_plugin->track->num_channels; ++i)
    volumes[i] = new_volume;

  /* Apply volume change to the sound card */
  gst_mixer_set_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

  /* Free volume array */
  g_free (volumes);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = FALSE;
#endif
}



static void
xfce_mixer_plugin_mute_toggled (XfceMixerPlugin *mixer_plugin,
                                gboolean         mute)
{
  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = TRUE;
#endif

  if (G_LIKELY (xfce_mixer_track_type_new (mixer_plugin->track) == XFCE_MIXER_TRACK_TYPE_PLAYBACK))
    {
      /* Apply mute change to the sound card */
      gst_mixer_set_mute (GST_MIXER (mixer_plugin->card), mixer_plugin->track, mute);
    }
  else
    {
      /* Toggle capture */
      gst_mixer_set_record (GST_MIXER (mixer_plugin->card), mixer_plugin->track, !mute);
    }

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = FALSE;
#endif
}



static void
xfce_mixer_plugin_update_track (XfceMixerPlugin *mixer_plugin)
{
  XfceMixerTrackType track_type;
  gboolean           muted = FALSE;
  gint               volume_range;
  gdouble            volume;
  gint              *volumes;
  gchar             *tip_text;

  g_return_if_fail (IS_XFCE_MIXER_PLUGIN (mixer_plugin));

  /* Reset tooltip and return if the card or track is invalid */
  if (!GST_IS_MIXER (mixer_plugin->card) || !GST_IS_MIXER_TRACK (mixer_plugin->track))
    {
      gtk_tooltips_set_tip (mixer_plugin->tooltips, mixer_plugin->button, NULL, NULL);
      return;
    }

  /* Get volumes of the mixer track */
  volumes = g_new (gint, mixer_plugin->track->num_channels);
  gst_mixer_get_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

  /* Determine difference between max and min volume */
  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Determine maximum value as double between 0.0 and 1.0 */
  volume = ((gdouble) xfce_mixer_get_max_volume (volumes, mixer_plugin->track->num_channels) - mixer_plugin->track->min_volume) / volume_range;

  /* Set tooltip (e.g. 'Master: 50%') */
  tip_text = g_strdup_printf (_("%s: %i%%"), mixer_plugin->track_label, (gint) (volume * 100));
  gtk_tooltips_set_tip (mixer_plugin->tooltips, mixer_plugin->button, tip_text, "test");
  g_free (tip_text);

  /* Determine track type */
  track_type = xfce_mixer_track_type_new (mixer_plugin->track);

  if (G_LIKELY (track_type == XFCE_MIXER_TRACK_TYPE_PLAYBACK))
    muted = GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_MUTE);
  else if (track_type == XFCE_MIXER_TRACK_TYPE_CAPTURE)
    muted = !GST_MIXER_TRACK_HAS_FLAG (mixer_plugin->track, GST_MIXER_TRACK_RECORD);

  /* Update the volume button */
  xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), volume);
  xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), muted);

  /* Free volume array */
  g_free (volumes);
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
static void
xfce_mixer_plugin_bus_message (GstBus          *bus,
                               GstMessage      *message,
                               XfceMixerPlugin *mixer_plugin)
{
  GstMixerTrack      *track = NULL;
  gboolean            mute;
  gboolean            record;
  gchar              *label;

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
        g_object_get (track, "label", &label, NULL);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          xfce_mixer_plugin_update_track (mixer_plugin);

        g_free (label);
        break;
      case GST_MIXER_MESSAGE_MUTE_TOGGLED:
        /* Parse the mute message */
        gst_mixer_message_parse_mute_toggled (message, &track, &mute);
        g_object_get (track, "label", &label, NULL);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), mute);

        g_free (label);
        break;
      case GST_MIXER_MESSAGE_RECORD_TOGGLED:
        /* Parse the record message */
        gst_mixer_message_parse_record_toggled (message, &track, &record);
        g_object_get (track, "label", &label, NULL);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (label, mixer_plugin->track_label) == 0))
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), !record);

        g_free (label);
      default:
        break;
    }
}
#endif

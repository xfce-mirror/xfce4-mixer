/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-volume-button.h"
#include "xfce-plugin-dialog.h"



#define DEFAULT_COMMAND "xfce4-mixer"



typedef struct _XfceMixerPlugin XfceMixerPlugin;



/* Plugin structure */
struct _XfceMixerPlugin
{
  XfcePanelPlugin *plugin;

  /* Tooltips structure */
  GtkTooltips     *tooltips;

  /* Sound card being used */
  GstElement      *card;

  /* Mixer track being used */
  GstMixerTrack   *track;

  /* Mixer command */
  gchar           *command;

  /* Widgets */
  GtkWidget       *hvbox;
  GtkWidget       *button;

#ifdef HAVE_GST_MIXER_NOTIFICATION
  /* Flag for ignoring messages from the GstBus */
  gboolean         ignore_bus_messages;

  /* GstBus connection id */
  guint            message_handler_id;
#endif
};



/* Function prototypes */
static void             xfce_mixer_plugin_construct         (XfcePanelPlugin  *plugin);
static XfceMixerPlugin *xfce_mixer_plugin_new               (XfcePanelPlugin  *plugin);
static void             xfce_mixer_plugin_free              (XfcePanelPlugin  *plugin,
                                                             XfceMixerPlugin  *mixer_plugin);
static gboolean         xfce_mixer_plugin_size_changed      (XfceMixerPlugin  *mixer_plugin,
                                                             gint              size);
static void             xfce_mixer_plugin_volume_changed    (XfceMixerPlugin  *mixer_plugin,
                                                             gdouble           volume);
static void             xfce_mixer_plugin_mute_toggled      (XfceMixerPlugin  *mixer_plugin,
                                                             gboolean          mute);
static void             xfce_mixer_plugin_configure         (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_clicked           (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_read_config       (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_write_config      (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_update_track      (XfceMixerPlugin  *mixer_plugin);
#ifdef HAVE_GST_MIXER_NOTIFICATION
static void             xfce_mixer_plugin_bus_message       (GstBus           *bus,
                                                             GstMessage       *message,
                                                             XfceMixerPlugin  *mixer_plugin);
#endif
static void             xfce_mixer_plugin_set_card          (XfceMixerPlugin  *mixer_plugin,
                                                             GstElement       *card);
static void             xfce_mixer_plugin_set_track         (XfceMixerPlugin  *mixer_plugin,
                                                             GstMixerTrack    *track);
static void             xfce_mixer_plugin_set_command       (XfceMixerPlugin  *mixer_plugin,
                                                             const gchar      *command);



/* Register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (xfce_mixer_plugin_construct);



static XfceMixerPlugin*
xfce_mixer_plugin_new (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;

  /* Allocate memory for the plugin structure */
  mixer_plugin = panel_slice_new0 (XfceMixerPlugin);

  /* Store pointer to the panel plugin */
  mixer_plugin->plugin = plugin;

  /* Initialize some of the plugin variables */
  mixer_plugin->card = NULL;
  mixer_plugin->track = NULL;
  mixer_plugin->command = g_strdup (DEFAULT_COMMAND);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = FALSE;
  mixer_plugin->message_handler_id = 0;
#endif

  /* Allocate a tooltips structure */
  mixer_plugin->tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_delay (mixer_plugin->tooltips, 10);

  /* Create container for the plugin */
  mixer_plugin->hvbox = GTK_WIDGET (xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0));
  xfce_panel_plugin_add_action_widget (plugin, mixer_plugin->hvbox);
  gtk_container_add (GTK_CONTAINER (plugin), mixer_plugin->hvbox);
  gtk_widget_show (mixer_plugin->hvbox);

  /* Create volume button for the plugin */
  mixer_plugin->button = xfce_volume_button_new ();
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "volume-changed", G_CALLBACK (xfce_mixer_plugin_volume_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "mute-toggled", G_CALLBACK (xfce_mixer_plugin_mute_toggled), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "clicked", G_CALLBACK (xfce_mixer_plugin_clicked), mixer_plugin);
  gtk_container_add (GTK_CONTAINER (mixer_plugin->hvbox), mixer_plugin->button);
  gtk_widget_show (mixer_plugin->button);

  /* Let the volume button receive mouse events */
  xfce_panel_plugin_add_action_widget (plugin, mixer_plugin->button);

  return mixer_plugin;
}



static void
xfce_mixer_plugin_free (XfcePanelPlugin *plugin,
                        XfceMixerPlugin *mixer_plugin)
{
  /* Free card and track names */
  g_free (mixer_plugin->command);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  /* Disconnect from GstBus */
  xfce_mixer_bus_disconnect (mixer_plugin->message_handler_id);
#endif

  /* Shutdown the mixer library */
  xfce_mixer_shutdown ();

  /* Free memory of the mixer plugin */
  panel_slice_free (XfceMixerPlugin, mixer_plugin);
}



static void
xfce_mixer_plugin_construct (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;

  /* Set up translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* Initialize the thread system */
  if (!g_thread_supported ())
    g_thread_init (NULL);

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Initialize the mixer library */
  xfce_mixer_init ();

  /* Create the plugin */
  mixer_plugin = xfce_mixer_plugin_new (plugin);

  xfce_panel_plugin_menu_show_configure (plugin);

  /* Connect to plugin signals */
  g_signal_connect_swapped (G_OBJECT (plugin), "free-data", G_CALLBACK (xfce_mixer_plugin_free), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (plugin), "size-changed", G_CALLBACK (xfce_mixer_plugin_size_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (plugin), "configure-plugin", G_CALLBACK (xfce_mixer_plugin_configure), mixer_plugin);

  /* Read config file */
  xfce_mixer_plugin_read_config (mixer_plugin);

  /* Update the plugin if it was already set up */
  if (G_LIKELY (GST_IS_MIXER (mixer_plugin->card) && GST_IS_MIXER_TRACK (mixer_plugin->track)))
    xfce_mixer_plugin_update_track (mixer_plugin);
}



static gboolean
xfce_mixer_plugin_size_changed (XfceMixerPlugin *mixer_plugin,
                                gint             size)
{
  GtkOrientation orientation;

  g_return_val_if_fail (mixer_plugin != NULL, FALSE);

  /* Get the orientation of the panel */
  orientation = xfce_panel_plugin_get_orientation (mixer_plugin->plugin);

  /* Resize the panel plugin */
  gtk_widget_set_size_request (GTK_WIDGET (mixer_plugin->plugin), size, size);

  /* Determine size for the volume button icons */
  size -= 2 + 2 * MAX (mixer_plugin->button->style->xthickness, mixer_plugin->button->style->ythickness);

  /* Set volume button icon size and update the volume button */
  xfce_volume_button_set_icon_size (XFCE_VOLUME_BUTTON (mixer_plugin->button), size);
  xfce_volume_button_update (XFCE_VOLUME_BUTTON (mixer_plugin->button));

  return TRUE;
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
  tip_text = g_strdup_printf (_("%s: %i%%"), GST_MIXER_TRACK (mixer_plugin->track)->label, (gint) (volume * 100));
  gtk_tooltips_set_tip (mixer_plugin->tooltips, mixer_plugin->button, tip_text, "test");
  g_free (tip_text);

  /* Allocate array for track volumes */
  volumes = g_new (gint, mixer_plugin->track->num_channels);

  /* Determine difference between max and min volume */
  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;

  /* Determine new volume */
  new_volume = mixer_plugin->track->min_volume + (volume * volume_range);

#if 0
  g_message ("Volume changed for %s (%s):", xfce_mixer_card_get_display_name (mixer_plugin->card), GST_MIXER_TRACK (mixer_plugin->track)->label);
  g_message ("  min_volume = %i, max_volume = %i, volume_range = %i, new_volume = %i", 
             mixer_plugin->track->min_volume, mixer_plugin->track->max_volume, volume_range, new_volume);
#endif

  /* Set all channel volumes to the new volume */
  for (i = 0; i < mixer_plugin->track->num_channels; ++i)
    volumes[i] = new_volume;

  /* Apply volume change to the sound card */
  gst_mixer_set_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

#if 0
  xfce_mixer_card_get_track_volume (mixer_plugin->card, mixer_plugin->track, volumes);
  g_message ("After the change: volume = %i", volumes[0]);
#endif

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

  /* Apply mute change to the sound card */
  gst_mixer_set_mute (GST_MIXER (mixer_plugin->card), mixer_plugin->track, mute);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->ignore_bus_messages = FALSE;
#endif
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
                                      XFCE_CUSTOM_STOCK_BUTTON, _("Properties"), GTK_STOCK_PREFERENCES, GTK_RESPONSE_ACCEPT,
                                      GTK_STOCK_CLOSE, GTK_RESPONSE_REJECT,
                                      NULL);

      /* Configure the plugin if requested by the user */
      if (G_LIKELY (response == GTK_RESPONSE_ACCEPT))
        xfce_mixer_plugin_configure (mixer_plugin);

      return;
    }

  /* Try to start the mixer command */
  if (G_UNLIKELY (!g_spawn_command_line_async (mixer_plugin->command, NULL)))
    {
      /* Generate error message and insert the current command */
      message = g_strdup_printf (_("Could not execute the command %s. "
                                   "Ensure that either the location of the command "
                                   "is included in the PATH environment variable or "
                                   "the full path of the command is used."), 
                                 mixer_plugin->command);

      /* Display error */
      xfce_err (message);

      /* Free error message */
      g_free (message);
    }
}



static void
xfce_mixer_plugin_configure (XfceMixerPlugin *mixer_plugin)
{
  GstElement    *card = NULL;
  GstMixerTrack *track = NULL;
  GtkWidget     *dialog;
  gchar         *command;

  g_return_if_fail (mixer_plugin != NULL);

  /* Block the panel menu as long as the config dialog is shown */
  xfce_panel_plugin_block_menu (mixer_plugin->plugin);

  /* Warn user if no sound cards are available */
  if (G_UNLIKELY (g_list_length (xfce_mixer_get_cards ()) <= 0))
    {
      xfce_err (_("GStreamer was unable to detect any sound devices. "
                  "Some sound system specific GStreamer packages may "
                  "be missing. It may also be a permissions problem."));
    }
  else
    {
      /* Create and run the config dialog */
      dialog = xfce_plugin_dialog_new (mixer_plugin->card, mixer_plugin->track, mixer_plugin->command);
      gtk_dialog_run (GTK_DIALOG (dialog));

      /* Determine which card and mixer track were selected */
      xfce_plugin_dialog_get_data (XFCE_PLUGIN_DIALOG (dialog), &card, &track, &command);

      /* Check if they are valid */
      if (G_LIKELY (GST_IS_MIXER (card) && GST_IS_MIXER_TRACK (track)))
        {
          /* Set card and track of the plugin */
          xfce_mixer_plugin_set_card (mixer_plugin, card);
          xfce_mixer_plugin_set_track (mixer_plugin, track);

          /* Update the volume button */
          xfce_mixer_plugin_update_track (mixer_plugin);
        }

      /* Set mixer command */
      xfce_mixer_plugin_set_command (mixer_plugin, command);

      /* Save the plugin configuration */
      xfce_mixer_plugin_write_config (mixer_plugin);

      /* Free mixer command */
      g_free (command);

      /* Destroy the config dialog */
      gtk_widget_destroy (dialog);
    }

  /* Make the plugin menu accessable again */
  xfce_panel_plugin_unblock_menu (mixer_plugin->plugin);
}



static void
xfce_mixer_plugin_read_config (XfceMixerPlugin *mixer_plugin)
{
  XfceRc        *rc;
  GstElement    *card;
  GstMixerTrack *track;
  const gchar   *card_name = NULL;
  const gchar   *track_name = NULL;
  const gchar   *command = DEFAULT_COMMAND;
  gchar         *filename;

  g_return_if_fail (mixer_plugin != NULL);

  /* Search for the config file */
  filename = xfce_panel_plugin_lookup_rc_file (mixer_plugin->plugin);

  /* Abort if file cannot be found */
  if (G_UNLIKELY (filename == NULL))
    {
      xfce_mixer_plugin_set_command (mixer_plugin, DEFAULT_COMMAND);
      return;
    }

  /* Open rc handle */
  rc = xfce_rc_simple_open (filename, TRUE);

  /* Only read config if rc handle could be opened */
  if (G_LIKELY (rc != NULL))
    {
      /* Read config values */
      card_name = xfce_rc_read_entry (rc, "card", card_name);
      track_name = xfce_rc_read_entry (rc, "track", track_name);
      command = xfce_rc_read_entry (rc, "command", command);

      /* Get card and track */
      card = xfce_mixer_get_card (card_name);
      track = xfce_mixer_get_track (card, track_name);

      /* Update plugin values */
      xfce_mixer_plugin_set_card (mixer_plugin, card);
      xfce_mixer_plugin_set_track (mixer_plugin, track);
      xfce_mixer_plugin_set_command (mixer_plugin, command);

      /* Close rc handle */
      xfce_rc_close (rc);
    }

  /* Free filename string */
  g_free (filename);
}



static void
xfce_mixer_plugin_write_config (XfceMixerPlugin *mixer_plugin)
{
  XfceRc *rc;
  gchar  *filename;

  g_return_if_fail (mixer_plugin != NULL);

  /* Search for the config file */
  filename = xfce_panel_plugin_save_location (mixer_plugin->plugin, TRUE);

  /* Abort saving if the file does not exist and could not be created */
  if (G_UNLIKELY (filename == NULL))
    return;

  /* Open rc handle */
  rc = xfce_rc_simple_open (filename, FALSE);

  if (G_LIKELY (rc != NULL))
    {
      /* Write plugin values to the config file */
      xfce_rc_write_entry (rc, "card", xfce_mixer_get_card_internal_name (mixer_plugin->card));
      xfce_rc_write_entry (rc, "track", GST_MIXER_TRACK (mixer_plugin->track)->label);
      xfce_rc_write_entry (rc, "command", mixer_plugin->command);

      /* Close the rc handle */
      xfce_rc_close (rc);
    }

  /* Free filename string */
  g_free (filename);
}



static void
xfce_mixer_plugin_update_track (XfceMixerPlugin *mixer_plugin)
{
  gint   *volumes;
  gdouble volume;
  gchar  *tip_text;

  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  /* Get volumes of the mixer track */
  volumes = g_new (gint, mixer_plugin->track->num_channels);
  gst_mixer_get_volume (GST_MIXER (mixer_plugin->card), mixer_plugin->track, volumes);

  /* Determine maximum value as double between 0.0 and 1.0 */
  volume = ((gdouble) xfce_mixer_get_max_volume (volumes, mixer_plugin->track->num_channels)) / mixer_plugin->track->max_volume;

  /* Set tooltip (e.g. 'Master: 50%') */
  tip_text = g_strdup_printf (_("%s: %i%%"), GST_MIXER_TRACK (mixer_plugin->track)->label, (gint) (volume * 100));
  gtk_tooltips_set_tip (mixer_plugin->tooltips, mixer_plugin->button, tip_text, "test");
  g_free (tip_text);

  /* Update the volume button */
  xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), volume);

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
  gdouble             volume;
  gint               *volumes;
  gint                num_channels;
  gboolean            mute;

  /* Don't do anything if GstBus messages are to be ignored */
  if (G_UNLIKELY (mixer_plugin->ignore_bus_messages))
    return;

  if (G_UNLIKELY (GST_MESSAGE_SRC (message) != GST_OBJECT (mixer_plugin->card)))
    return;

  switch (gst_mixer_message_get_type (message))
    {
      case GST_MIXER_MESSAGE_VOLUME_CHANGED:
        /* Get the track of the volume changed message */
        gst_mixer_message_parse_volume_changed (message, &track, NULL, NULL);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (track->label, mixer_plugin->track->label) == 0))
          xfce_mixer_plugin_update_track (mixer_plugin);
        break;
      case GST_MIXER_MESSAGE_MUTE_TOGGLED:
        /* Parse the mute message */
        gst_mixer_message_parse_mute_toggled (message, &track, &mute);

        /* Update the volume button if the message belongs to the current mixer track */
        if (G_UNLIKELY (g_utf8_collate (track->label, mixer_plugin->track->label) == 0))
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), mute);
        break;
      default:
        break;
    }
}
#endif



static void
xfce_mixer_plugin_set_card (XfceMixerPlugin *mixer_plugin,
                            GstElement      *card)
{
  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER (card));

  mixer_plugin->card = card;

  xfce_mixer_select_card (mixer_plugin->card);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer_plugin->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_plugin_bus_message), mixer_plugin);
#endif
}



static void
xfce_mixer_plugin_set_track (XfceMixerPlugin *mixer_plugin,
                             GstMixerTrack   *track)
{
  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (GST_IS_MIXER_TRACK (track));

  mixer_plugin->track = track;
}



static void
xfce_mixer_plugin_set_command (XfceMixerPlugin *mixer_plugin,
                               const gchar     *command)
{
  g_return_if_fail (mixer_plugin != NULL);

  if (G_LIKELY (mixer_plugin->command != NULL))
    {
      if (G_UNLIKELY (g_utf8_collate (mixer_plugin->command, command) == 0))
        return;

      g_free (mixer_plugin->command);
    }

  mixer_plugin->command = g_strdup (command);
}

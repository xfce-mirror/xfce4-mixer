/* $Id$ */
/* vim:set et ai sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "libxfce4mixer/xfce-mixer-stock.h"
#include "libxfce4mixer/xfce-mixer-card.h"

#include "xfce-volume-button.h"
#include "xfce-plugin-dialog.h"



typedef struct _XfceMixerPlugin XfceMixerPlugin;



/* Plugin structure */
struct _XfceMixerPlugin
{
  XfcePanelPlugin *plugin;

  gchar           *card_name;
  gchar           *track_name;

  XfceMixerCard   *card;
  GstMixerTrack   *track;

  GtkWidget       *hvbox;
  GtkWidget       *button;
};



/* Function prototypes */
static void             xfce_mixer_plugin_construct      (XfcePanelPlugin  *plugin);
static XfceMixerPlugin *xfce_mixer_plugin_new            (XfcePanelPlugin  *plugin);
static void             xfce_mixer_plugin_free           (XfcePanelPlugin  *plugin,
                                                          XfceMixerPlugin  *mixer_plugin);
static gboolean         xfce_mixer_plugin_size_changed   (XfceMixerPlugin  *mixer_plugin,
                                                          gint              size);
static void             xfce_mixer_plugin_volume_changed (XfceMixerPlugin  *mixer_plugin,
                                                          gdouble           volume);
static void             xfce_mixer_plugin_mute_toggled   (XfceMixerPlugin  *mixer_plugin,
                                                          gboolean          mute);
static void             xfce_mixer_plugin_configure      (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_clicked        (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_read_config    (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_write_config   (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_update_track   (XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_replace_values (XfceMixerPlugin  *mixer_plugin, 
                                                          const gchar      *card_name,
                                                          const gchar      *track_name);
#ifdef HAVE_GST_MIXER_NOTIFICATION
static gboolean         xfce_mixer_plugin_bus_message    (GstBus           *bus,
                                                          GstMessage       *message,
                                                          XfceMixerPlugin  *mixer_plugin);
#endif



/* Register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (xfce_mixer_plugin_construct);



static XfceMixerPlugin*
xfce_mixer_plugin_new (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;
  GtkWidget       *button;

  /* Allocate memory for the plugin structure */
  mixer_plugin = panel_slice_new0 (XfceMixerPlugin);

  /* Store pointer to the panel plugin */
  mixer_plugin->plugin = plugin;

  mixer_plugin->card = NULL;
  mixer_plugin->track = NULL;

  mixer_plugin->hvbox = GTK_WIDGET (xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0));
  xfce_panel_plugin_add_action_widget (plugin, mixer_plugin->hvbox);
  gtk_container_add (GTK_CONTAINER (plugin), mixer_plugin->hvbox);
  gtk_widget_show (mixer_plugin->hvbox);

  mixer_plugin->button = xfce_volume_button_new ();
  xfce_panel_plugin_add_action_widget (plugin, mixer_plugin->button);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "volume-changed", G_CALLBACK (xfce_mixer_plugin_volume_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "mute-toggled", G_CALLBACK (xfce_mixer_plugin_mute_toggled), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (mixer_plugin->button), "clicked", G_CALLBACK (xfce_mixer_plugin_clicked), mixer_plugin);
  gtk_container_add (GTK_CONTAINER (mixer_plugin->hvbox), mixer_plugin->button);
  gtk_widget_show (mixer_plugin->button);

  return mixer_plugin;
}



static void
xfce_mixer_plugin_free (XfcePanelPlugin *plugin,
                        XfceMixerPlugin *mixer_plugin)
{
  g_free (mixer_plugin->card_name);
  g_free (mixer_plugin->track_name);

  panel_slice_free (XfceMixerPlugin, mixer_plugin);
}



static void
xfce_mixer_plugin_construct (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;

  /* Set up translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  if (!g_thread_supported ())
    g_thread_init (NULL);

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Initialize our own stock icon set */
  xfce_mixer_stock_init ();

  /* Create the plugin */
  mixer_plugin = xfce_mixer_plugin_new (plugin);

  xfce_panel_plugin_menu_show_configure (plugin);

  /* Connect to plugin signals */
  g_signal_connect_swapped (G_OBJECT (plugin), "free-data", G_CALLBACK (xfce_mixer_plugin_free), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (plugin), "size-changed", G_CALLBACK (xfce_mixer_plugin_size_changed), mixer_plugin);
  g_signal_connect_swapped (G_OBJECT (plugin), "configure-plugin", G_CALLBACK (xfce_mixer_plugin_configure), mixer_plugin);

  /* Read config file */
  xfce_mixer_plugin_read_config (mixer_plugin);

  /* Update the plugin */
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

  gtk_widget_set_size_request (GTK_WIDGET (mixer_plugin->plugin), size, size);

  size -= 2 + 2 * MAX (mixer_plugin->button->style->xthickness, mixer_plugin->button->style->ythickness);

  xfce_volume_button_set_icon_size (XFCE_VOLUME_BUTTON (mixer_plugin->button), size);
  xfce_volume_button_update (XFCE_VOLUME_BUTTON (mixer_plugin->button));

  /* TODO: Handle it */

  return TRUE;
}



static void
xfce_mixer_plugin_volume_changed (XfceMixerPlugin  *mixer_plugin,
                                  gdouble           volume)
{
  gint *volumes;
  gint  volume_range;
  gint  new_volume;
  gint  i;

  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (IS_XFCE_MIXER_CARD (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  volumes = g_new (gint, mixer_plugin->track->num_channels);

  volume_range = mixer_plugin->track->max_volume - mixer_plugin->track->min_volume;
  new_volume = mixer_plugin->track->min_volume + (volume * volume_range);

  for (i = 0; i < mixer_plugin->track->num_channels; ++i)
    volumes[i] = new_volume;

  xfce_mixer_card_set_track_volume (mixer_plugin->card, mixer_plugin->track, volumes);

  g_free (volumes);
}



static void
xfce_mixer_plugin_mute_toggled (XfceMixerPlugin *mixer_plugin,
                                gboolean         mute)
{
  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (IS_XFCE_MIXER_CARD (mixer_plugin->card));
  g_return_if_fail (GST_IS_MIXER_TRACK (mixer_plugin->track));

  xfce_mixer_card_set_track_muted (mixer_plugin->card, mixer_plugin->track, mute);
}



static void
xfce_mixer_plugin_clicked (XfceMixerPlugin *mixer_plugin)
{
  g_return_if_fail (mixer_plugin != NULL);

  if (G_UNLIKELY (!g_spawn_command_line_async ("xfce4-mixer", NULL)))
    xfce_err (_("Could not find xfce4-mixer in PATH."));
}



static void
xfce_mixer_plugin_configure (XfceMixerPlugin *mixer_plugin)
{
  XfceMixerCard *card;
  GstMixerTrack *track;
  GtkWidget     *dialog;

  g_return_if_fail (mixer_plugin != NULL);

  xfce_panel_plugin_block_menu (mixer_plugin->plugin);

  dialog = xfce_plugin_dialog_new (mixer_plugin->card_name, mixer_plugin->track_name);

  gtk_dialog_run (GTK_DIALOG (dialog));

  xfce_plugin_dialog_get_data (XFCE_PLUGIN_DIALOG (dialog), &card, &track);

  gtk_widget_destroy (dialog);

  if (G_LIKELY (IS_XFCE_MIXER_CARD (card) && GST_IS_MIXER_TRACK (track)))
    {
      g_message ("Changed to card %s and track %s", xfce_mixer_card_get_display_name (card), GST_MIXER_TRACK (track)->label);
  
      xfce_mixer_plugin_replace_values (mixer_plugin, xfce_mixer_card_get_display_name (card), GST_MIXER_TRACK (track)->label);

      xfce_mixer_plugin_write_config (mixer_plugin);

      xfce_mixer_plugin_update_track (mixer_plugin);
    }

  xfce_panel_plugin_unblock_menu (mixer_plugin->plugin);
}



static void
xfce_mixer_plugin_read_config (XfceMixerPlugin *mixer_plugin)
{
  XfceRc      *rc;
  const gchar *card = NULL;
  const gchar *track = NULL;
  gchar       *filename;

  g_return_if_fail (mixer_plugin != NULL);

  /* Search for the config file */
  filename = xfce_panel_plugin_lookup_rc_file (mixer_plugin->plugin);

  /* Abort if file cannot be found */
  if (G_UNLIKELY (filename == NULL))
    {
      mixer_plugin->card_name = g_strdup (card);
      mixer_plugin->track_name = g_strdup (track);
      return;
    }

  /* Open rc handle */
  rc = xfce_rc_simple_open (filename, TRUE);

  /* Only read config if rc handle could be opened */
  if (G_LIKELY (rc != NULL))
    {
      /* Read card value */
      card = xfce_rc_read_entry (rc, "card", card);
      
      /* Read track value */
      track = xfce_rc_read_entry (rc, "track", track);

      /* Update plugin values */
      xfce_mixer_plugin_replace_values (mixer_plugin, card, track);

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
      xfce_rc_write_entry (rc, "card", mixer_plugin->card_name);
      xfce_rc_write_entry (rc, "track", mixer_plugin->track_name);

      xfce_rc_close (rc);
    }

  g_free (filename);
}



static void
xfce_mixer_plugin_update_track (XfceMixerPlugin *mixer_plugin)
{
  gint   *volumes;
  gdouble volume;

  g_return_if_fail (mixer_plugin != NULL);

  if (mixer_plugin->card_name == NULL || mixer_plugin->track_name == NULL)
    return;

  if (IS_XFCE_MIXER_CARD (mixer_plugin->card))
    g_object_unref (G_OBJECT (mixer_plugin->card));

  if (GST_IS_MIXER_TRACK (mixer_plugin->track))
    g_object_unref (G_OBJECT (mixer_plugin->track));

  mixer_plugin->card = XFCE_MIXER_CARD (xfce_mixer_utilities_get_card_by_name (mixer_plugin->card_name));
#ifdef HAVE_GST_MIXER_NOTIFICATION
  xfce_mixer_card_connect (mixer_plugin->card, G_CALLBACK (xfce_mixer_plugin_bus_message), mixer_plugin);
#endif

  mixer_plugin->track = xfce_mixer_card_get_track_by_name (mixer_plugin->card, mixer_plugin->track_name);

  volumes = g_new (gint, mixer_plugin->track->num_channels);
  xfce_mixer_card_get_track_volume (mixer_plugin->card, mixer_plugin->track, volumes);
  volume = ((gdouble) volumes[0]) / mixer_plugin->track->max_volume;

  g_message ("volume (int) = %i, volume (double) = %f", volumes[0], volume);

  xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), volume);

  g_free (volumes);
}



static void
xfce_mixer_plugin_replace_values (XfceMixerPlugin *mixer_plugin, 
                                  const gchar     *card_name,
                                  const gchar     *track_name)
{
  g_return_if_fail (mixer_plugin != NULL);
  g_return_if_fail (card_name != NULL);
  g_return_if_fail (track_name != NULL);

  g_free (mixer_plugin->card_name);
  g_free (mixer_plugin->track_name);

  mixer_plugin->card_name = g_strdup (card_name);
  mixer_plugin->track_name = g_strdup (track_name);
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
static gboolean
xfce_mixer_plugin_bus_message (GstBus          *bus,
                               GstMessage      *message,
                               XfceMixerPlugin *mixer_plugin)
{
  GstMixerTrack      *track = NULL;
  gdouble             volume;
  gint               *volumes;
  gint                num_channels;
  gboolean            mute;

  if (G_UNLIKELY (!xfce_mixer_card_get_message_owner (mixer_plugin->card, message)))
    return TRUE;

  g_debug ("Message from card received: %s", GST_MESSAGE_TYPE_NAME (message));

  switch (gst_mixer_message_get_type (message))
    {
      case GST_MIXER_MESSAGE_VOLUME_CHANGED:
        gst_mixer_message_parse_volume_changed (message, &track, &volumes, &num_channels);

        if (G_UNLIKELY (g_utf8_collate (track->label, mixer_plugin->track->label) == 0))
          {
            volume = ((gdouble) volumes[0]) / mixer_plugin->track->max_volume;
            xfce_volume_button_set_volume (XFCE_VOLUME_BUTTON (mixer_plugin->button), volume);
          }
        break;
      case GST_MIXER_MESSAGE_MUTE_TOGGLED:
        gst_mixer_message_parse_mute_toggled (message, &track, &mute);

        if (G_UNLIKELY (g_utf8_collate (track->label, mixer_plugin->track->label) == 0))
          xfce_volume_button_set_muted (XFCE_VOLUME_BUTTON (mixer_plugin->button), mute);
        break;
    }
}
#endif

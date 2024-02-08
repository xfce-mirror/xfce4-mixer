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

#include <libxfce4panel/libxfce4panel.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-plugin.h"

#include "xfce-plugin-dialog.h"



static void xfce_plugin_dialog_dispose                    (GObject               *object);
static void xfce_plugin_dialog_finalize                   (GObject               *object);
static void xfce_plugin_dialog_create_contents            (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_soundcard_changed          (XfcePluginDialog      *dialog,
                                                           GstElement            *card,
                                                           XfceMixerCardCombo    *combo);
static void xfce_plugin_dialog_track_changed              (XfcePluginDialog      *dialog,
                                                           GstMixerTrack         *track,
                                                           XfceMixerTrackCombo   *combo);
static void xfce_plugin_dialog_soundcard_property_changed (XfcePluginDialog      *dialog,
                                                           GParamSpec            *pspec,
                                                           GObject               *object);
static void xfce_plugin_dialog_track_property_changed     (XfcePluginDialog      *dialog,
                                                           GParamSpec            *pspec,
                                                           GObject               *object);



struct _XfcePluginDialog
{
  XfceTitledDialog  __parent__;

  XfceMixerPlugin  *plugin;

  GtkWidget        *card_combo;
  GtkWidget        *track_combo;
};



G_DEFINE_TYPE (XfcePluginDialog, xfce_plugin_dialog, XFCE_TYPE_TITLED_DIALOG)



static void
xfce_plugin_dialog_class_init (XfcePluginDialogClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_plugin_dialog_dispose;
  gobject_class->finalize = xfce_plugin_dialog_finalize;
}



static void
xfce_plugin_dialog_init (XfcePluginDialog *dialog)
{
  dialog->card_combo = NULL;
  dialog->track_combo = NULL;
  dialog->plugin = NULL;
}



static void
xfce_plugin_dialog_dispose (GObject *object)
{
  XfcePluginDialog *dialog = XFCE_PLUGIN_DIALOG (object);

  g_signal_handlers_disconnect_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_soundcard_property_changed, dialog);
  g_signal_handlers_disconnect_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);

  (*G_OBJECT_CLASS (xfce_plugin_dialog_parent_class)->dispose) (object);
}



static void
xfce_plugin_dialog_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_plugin_dialog_parent_class)->finalize) (object);
}



GtkWidget*
xfce_plugin_dialog_new (XfcePanelPlugin *plugin)
{
  XfcePluginDialog *dialog;

  dialog = XFCE_PLUGIN_DIALOG (g_object_new (XFCE_TYPE_PLUGIN_DIALOG, NULL));
  dialog->plugin = XFCE_MIXER_PLUGIN (plugin);

  xfce_plugin_dialog_create_contents (dialog);

  return GTK_WIDGET (dialog);
}



static void
xfce_plugin_dialog_create_contents (XfcePluginDialog *dialog)
{
  GtkWidget     *grid;
  GtkWidget     *button;
  GtkWidget     *label;

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "multimedia-volume-control");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Audio Mixer Plugin"));
  
  button = gtk_button_new_with_mnemonic (_("_Close"));
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_BUTTON));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CLOSE);
  gtk_widget_show (button);

  grid = gtk_grid_new ();
  g_object_set (G_OBJECT (grid), "row-spacing", 6, "column-spacing", 12, "margin-top", 6, "margin-bottom", 6, NULL);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), grid);
  gtk_widget_show (grid);

  label = gtk_label_new_with_mnemonic (_("Sound _card:"));
  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
  gtk_widget_show (label);

  dialog->card_combo = xfce_mixer_card_combo_new (NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->card_combo);
  g_object_set (G_OBJECT (dialog->card_combo), "halign", GTK_ALIGN_FILL, "hexpand", TRUE, NULL);
  gtk_grid_attach (GTK_GRID (grid), dialog->card_combo, 1, 0, 1, 1);
  gtk_widget_show (dialog->card_combo);

  label = gtk_label_new_with_mnemonic (_("Mixer _track:"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), dialog->track_combo);
  gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
  gtk_widget_show (label);

  dialog->track_combo = xfce_mixer_track_combo_new (NULL, NULL);
  g_object_set (G_OBJECT (dialog->track_combo), "halign", GTK_ALIGN_FILL, "hexpand", TRUE, NULL);
  gtk_grid_attach (GTK_GRID (grid), dialog->track_combo, 1, 1, 1, 1);
  gtk_widget_show (dialog->track_combo);

  /* Hack to initialize the widget state */
  xfce_plugin_dialog_soundcard_property_changed (dialog, g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (dialog->plugin)), "sound-card"), G_OBJECT (dialog->plugin));
  xfce_plugin_dialog_track_property_changed (dialog, g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (dialog->plugin)), "track"), G_OBJECT (dialog->plugin));

  g_signal_connect_swapped (G_OBJECT (dialog->card_combo), "soundcard-changed", G_CALLBACK (xfce_plugin_dialog_soundcard_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->track_combo), "track-changed", G_CALLBACK (xfce_plugin_dialog_track_changed), dialog);

  g_signal_connect_swapped (G_OBJECT (dialog->plugin), "notify::sound-card", G_CALLBACK (xfce_plugin_dialog_soundcard_property_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->plugin), "notify::track", G_CALLBACK (xfce_plugin_dialog_track_property_changed), dialog);
}



static void 
xfce_plugin_dialog_soundcard_changed (XfcePluginDialog   *dialog,
                                      GstElement         *card,
                                      XfceMixerCardCombo *combo)
{
  const gchar *card_name;

  card_name = xfce_mixer_get_card_internal_name (card);

  g_signal_handlers_block_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_soundcard_property_changed, dialog);
  g_signal_handlers_block_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
  g_object_set (G_OBJECT (dialog->plugin), "sound-card", card_name, NULL);
  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_soundcard_property_changed, dialog);

  xfce_mixer_track_combo_set_soundcard (XFCE_MIXER_TRACK_COMBO (dialog->track_combo), card);
}



static void
xfce_plugin_dialog_track_changed (XfcePluginDialog    *dialog,
                                  GstMixerTrack       *track,
                                  XfceMixerTrackCombo *combo)
{
  const gchar *track_label;

  track_label = xfce_mixer_get_track_label (track);

  g_signal_handlers_block_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
  g_object_set (G_OBJECT (dialog->plugin), "track", track_label, NULL);
  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
}



static void
xfce_plugin_dialog_soundcard_property_changed (XfcePluginDialog *dialog,
                                               GParamSpec       *pspec,
                                               GObject          *object)
{
  GstElement  *old_card;
  GstElement  *new_card = NULL;
  gchar       *new_card_name;
  const gchar *old_card_name = NULL;

  g_return_if_fail (XFCE_IS_PLUGIN_DIALOG (dialog));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (XFCE_IS_MIXER_CARD_COMBO (dialog->card_combo));
  g_return_if_fail (XFCE_IS_MIXER_TRACK_COMBO (dialog->track_combo));

  g_object_get (object, "sound-card", &new_card_name, NULL);
  if (new_card_name != NULL)
    new_card = xfce_mixer_get_card (new_card_name);

  old_card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo));
  if (GST_IS_MIXER (old_card))
    old_card_name = xfce_mixer_get_card_internal_name (old_card);

  if (xfce_mixer_utf8_cmp (old_card_name, new_card_name) != 0)
    {
      g_signal_handlers_block_by_func (object, xfce_plugin_dialog_soundcard_changed, dialog);
      g_signal_handlers_block_by_func (object, xfce_plugin_dialog_track_changed, dialog);
      /* If "sound-card" is NULL, this will take care of resetting it to the default */
      xfce_mixer_card_combo_set_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo), new_card);
      xfce_mixer_track_combo_set_soundcard (XFCE_MIXER_TRACK_COMBO (dialog->track_combo), new_card);
      g_signal_handlers_unblock_by_func (object, xfce_plugin_dialog_track_changed, dialog);
      g_signal_handlers_unblock_by_func (object, xfce_plugin_dialog_soundcard_changed, dialog);
    }

  g_free (new_card_name);
}



static void
xfce_plugin_dialog_track_property_changed (XfcePluginDialog *dialog,
                                           GParamSpec       *pspec,
                                           GObject          *object)
{
  GstElement    *card;
  GstMixerTrack *old_track;
  GstMixerTrack *new_track = NULL;
  const gchar   *old_track_label = NULL;
  gchar         *new_track_label;

  g_return_if_fail (XFCE_IS_PLUGIN_DIALOG (dialog));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (XFCE_IS_MIXER_CARD_COMBO (dialog->card_combo));
  g_return_if_fail (XFCE_IS_MIXER_TRACK_COMBO (dialog->track_combo));

  g_object_get (object, "track", &new_track_label, NULL);

  card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo));
  if (new_track_label != NULL && GST_IS_MIXER (card))
    new_track = xfce_mixer_get_track (card, new_track_label);

  old_track = xfce_mixer_track_combo_get_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo));
  if (GST_IS_MIXER_TRACK (old_track))
    old_track_label = xfce_mixer_get_track_label (old_track);

  if (xfce_mixer_utf8_cmp (old_track_label, new_track_label) != 0)
    {
      g_signal_handlers_block_by_func (object, xfce_plugin_dialog_track_changed, dialog);
      /* If track has been removed, this will take care of resetting it to the default */
      xfce_mixer_track_combo_set_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo), new_track);
      g_signal_handlers_unblock_by_func (object, xfce_plugin_dialog_track_changed, dialog);
    }

  g_free (new_track_label);
}


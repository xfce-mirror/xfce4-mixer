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
#include <gst/interfaces/mixer.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-plugin.h"

#include "xfce-plugin-dialog.h"



static void xfce_plugin_dialog_class_init                 (XfcePluginDialogClass *klass);
static void xfce_plugin_dialog_init                       (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_dispose                    (GObject               *object);
static void xfce_plugin_dialog_finalize                   (GObject               *object);
static void xfce_plugin_dialog_create_contents            (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_command_button_clicked     (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_soundcard_changed          (XfcePluginDialog      *dialog,
                                                           GstElement            *card,
                                                           XfceMixerCardCombo    *combo);
static void xfce_plugin_dialog_track_changed              (XfcePluginDialog      *dialog,
                                                           GstMixerTrack         *track,
                                                           XfceMixerTrackCombo   *combo);
static void xfce_plugin_dialog_command_entry_changed      (XfcePluginDialog      *dialog,
                                                           GtkEditable           *editable);
static void xfce_plugin_dialog_soundcard_property_changed (XfcePluginDialog      *dialog,
                                                           GParamSpec            *pspec,
                                                           GObject               *object);
static void xfce_plugin_dialog_track_property_changed     (XfcePluginDialog      *dialog,
                                                           GParamSpec            *pspec,
                                                           GObject               *object);
static void xfce_plugin_dialog_command_property_changed   (XfcePluginDialog      *dialog,
                                                           GParamSpec            *pspec,
                                                           GObject               *object);



struct _XfcePluginDialogClass
{
  XfceTitledDialogClass __parent__;
};



struct _XfcePluginDialog
{
  XfceTitledDialog  __parent__;

  XfceMixerPlugin  *plugin;

  GtkWidget        *card_combo;
  GtkWidget        *track_combo;
  GtkWidget        *command_entry;
};



static GObjectClass *xfce_plugin_dialog_parent_class = NULL;



GType
xfce_plugin_dialog_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfcePluginDialogClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_plugin_dialog_class_init,
          NULL,
          NULL,
          sizeof (XfcePluginDialog),
          0,
          (GInstanceInitFunc) xfce_plugin_dialog_init,
          NULL,
        };

      type = g_type_register_static (XFCE_TYPE_TITLED_DIALOG, "XfcePluginDialog", &info, 0);
    }
  
  return type;
}



static void
xfce_plugin_dialog_class_init (XfcePluginDialogClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_plugin_dialog_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_plugin_dialog_dispose;
  gobject_class->finalize = xfce_plugin_dialog_finalize;
}



static void
xfce_plugin_dialog_init (XfcePluginDialog *dialog)
{
  dialog->card_combo = NULL;
  dialog->track_combo = NULL;
  dialog->command_entry = NULL;
  dialog->plugin = NULL;
}



static void
xfce_plugin_dialog_dispose (GObject *object)
{
  XfcePluginDialog *dialog = XFCE_PLUGIN_DIALOG (object);

  g_signal_handlers_disconnect_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_soundcard_property_changed, dialog);
  g_signal_handlers_disconnect_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
  g_signal_handlers_disconnect_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_command_property_changed, dialog);

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

  dialog = XFCE_PLUGIN_DIALOG (g_object_new (TYPE_XFCE_PLUGIN_DIALOG, NULL));
  dialog->plugin = XFCE_MIXER_PLUGIN (plugin);

  xfce_plugin_dialog_create_contents (dialog);

  return GTK_WIDGET (dialog);
}



static void
xfce_plugin_dialog_create_contents (XfcePluginDialog *dialog)
{
  GtkWidget     *alignment;
  GtkWidget     *vbox;
  GtkWidget     *hbox;
  GtkWidget     *button;
  GtkWidget     *label;
  gchar         *title;

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "multimedia-volume-control");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Mixer Plugin"));

  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), _("Configure the mixer track and left-click command"));
  
  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CLOSE);
  gtk_widget_show (button);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), vbox);
  gtk_widget_show (vbox);

  label = gtk_label_new (NULL);
  title = g_strdup_printf ("<span weight='bold'>%s</span>", _("Sound card"));
  gtk_label_set_markup (GTK_LABEL (label), title);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (title);

  alignment = gtk_alignment_new (0.0, 0.0, 1, 1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 6, 12, 0);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, TRUE, 0);
  gtk_widget_show (alignment);

  dialog->card_combo = xfce_mixer_card_combo_new (NULL);
  gtk_container_add (GTK_CONTAINER (alignment), dialog->card_combo);
  gtk_widget_show (dialog->card_combo);

  label = gtk_label_new (NULL);
  title = g_strdup_printf ("<span weight='bold'>%s</span>", _("Mixer track"));
  gtk_label_set_markup (GTK_LABEL (label), title);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (title);

  alignment = gtk_alignment_new (0.0, 0.0, 1, 1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 6, 12, 0);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, TRUE, 0);
  gtk_widget_show (alignment);

  dialog->track_combo = xfce_mixer_track_combo_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (alignment), dialog->track_combo);
  gtk_widget_show (dialog->track_combo);

  label = gtk_label_new (NULL);
  title = g_strdup_printf ("<span weight='bold'>%s</span>", _("Left-click command"));
  gtk_label_set_markup (GTK_LABEL (label), title);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (title);

  alignment = gtk_alignment_new (0.0, 0.0, 1, 1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 6, 12, 0);
  gtk_box_pack_start (GTK_BOX (vbox), alignment, FALSE, TRUE, 0);
  gtk_widget_show (alignment);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (alignment), hbox);
  gtk_widget_show (hbox);

  dialog->command_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), dialog->command_entry, TRUE, TRUE, 0);
  gtk_widget_show (dialog->command_entry);

  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (xfce_plugin_dialog_command_button_clicked), dialog);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);

  /* Hack to initialize the widget state */
  xfce_plugin_dialog_soundcard_property_changed (dialog, g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (dialog->plugin)), "sound-card"), G_OBJECT (dialog->plugin));
  xfce_plugin_dialog_track_property_changed (dialog, g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (dialog->plugin)), "track"), G_OBJECT (dialog->plugin));
  xfce_plugin_dialog_command_property_changed (dialog, g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (dialog->plugin)), "command"), G_OBJECT (dialog->plugin));

  g_signal_connect_swapped (G_OBJECT (dialog->card_combo), "soundcard-changed", G_CALLBACK (xfce_plugin_dialog_soundcard_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->track_combo), "track-changed", G_CALLBACK (xfce_plugin_dialog_track_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->command_entry), "changed", G_CALLBACK (xfce_plugin_dialog_command_entry_changed), dialog);

  g_signal_connect_swapped (G_OBJECT (dialog->plugin), "notify::sound-card", G_CALLBACK (xfce_plugin_dialog_soundcard_property_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->plugin), "notify::track", G_CALLBACK (xfce_plugin_dialog_track_property_changed), dialog);
  g_signal_connect_swapped (G_OBJECT (dialog->plugin), "notify::command", G_CALLBACK (xfce_plugin_dialog_command_property_changed), dialog);
}



static void
xfce_plugin_dialog_command_button_clicked (XfcePluginDialog *dialog)
{
  GtkWidget     *chooser;
  GtkFileFilter *filter;
  gchar         *filename;

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));

  chooser = gtk_file_chooser_dialog_new (_("Select command"), 
                                         GTK_WINDOW (dialog), 
                                         GTK_FILE_CHOOSER_ACTION_OPEN, 
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

  /* Add file chooser filters */
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All Files"));
  gtk_file_filter_add_pattern (filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Executable Files"));
  gtk_file_filter_add_mime_type (filter, "application/x-csh");
  gtk_file_filter_add_mime_type (filter, "application/x-executable");
  gtk_file_filter_add_mime_type (filter, "application/x-perl");
  gtk_file_filter_add_mime_type (filter, "application/x-python");
  gtk_file_filter_add_mime_type (filter, "application/x-ruby");
  gtk_file_filter_add_mime_type (filter, "application/x-shellscript");
  gtk_file_filter_add_pattern (filter, "*.pl");
  gtk_file_filter_add_pattern (filter, "*.py");
  gtk_file_filter_add_pattern (filter, "*.rb");
  gtk_file_filter_add_pattern (filter, "*.sh");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Perl Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-perl");
  gtk_file_filter_add_pattern (filter, "*.pl");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Python Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-python");
  gtk_file_filter_add_pattern (filter, "*.py");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Ruby Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-ruby");
  gtk_file_filter_add_pattern (filter, "*.rb");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Shell Scripts"));
  gtk_file_filter_add_mime_type (filter, "application/x-csh");
  gtk_file_filter_add_mime_type (filter, "application/x-shellscript");
  gtk_file_filter_add_pattern (filter, "*.sh");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

#if 0
  /* Use bindir as default folder */
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), BINDIR);
#endif
  
  /* Use previous command as a starting point */
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (chooser), gtk_entry_get_text (GTK_ENTRY (dialog->command_entry)));

  /* Run the file chooser */
  if (G_LIKELY (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK))
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
      gtk_entry_set_text (GTK_ENTRY (dialog->command_entry), filename);
      g_free (filename);
    }

  /* Destroy the dialog */
  gtk_widget_destroy (chooser);
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
  gchar *track_label;

  g_object_get (G_OBJECT (track), "label", &track_label, NULL);

  g_signal_handlers_block_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);
  g_object_set (G_OBJECT (dialog->plugin), "track", track_label, NULL);
  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_track_property_changed, dialog);

  g_free (track_label);
}



static void
xfce_plugin_dialog_command_entry_changed (XfcePluginDialog *dialog,
                                          GtkEditable      *editable)
{
  gchar *command;

  command = gtk_editable_get_chars (editable, 0, -1);

  g_signal_handlers_block_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_command_property_changed, dialog);
  g_object_set (G_OBJECT (dialog->plugin), "command", command, NULL);
  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->plugin), xfce_plugin_dialog_command_property_changed, dialog);

  g_free (command);
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

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (IS_XFCE_MIXER_CARD_COMBO (dialog->card_combo));
  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (dialog->track_combo));

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
  gchar         *old_track_label = NULL;
  gchar         *new_track_label = NULL;

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (IS_XFCE_MIXER_CARD_COMBO (dialog->card_combo));
  g_return_if_fail (IS_XFCE_MIXER_TRACK_COMBO (dialog->track_combo));

  g_object_get (object, "track", &new_track_label, NULL);

  card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo));
  if (new_track_label != NULL && GST_IS_MIXER (card))
    new_track = xfce_mixer_get_track (card, new_track_label);

  old_track = xfce_mixer_track_combo_get_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo));
  if (GST_IS_MIXER_TRACK (old_track))
    g_object_get (G_OBJECT (old_track), "label", &old_track_label, NULL);

  if (xfce_mixer_utf8_cmp (old_track_label, new_track_label) != 0)
    {
      g_signal_handlers_block_by_func (object, xfce_plugin_dialog_track_changed, dialog);
      /* If track has been removed, this will take care of resetting it to the default */
      xfce_mixer_track_combo_set_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo), new_track);
      g_signal_handlers_unblock_by_func (object, xfce_plugin_dialog_track_changed, dialog);
    }

  g_free (new_track_label);
  g_free (old_track_label);
}



static void
xfce_plugin_dialog_command_property_changed (XfcePluginDialog *dialog,
                                             GParamSpec       *pspec,
                                             GObject          *object)
{
  gchar *command;

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));
  g_return_if_fail (G_IS_OBJECT (object));

  g_object_get (object, "command", &command, NULL);

  if (xfce_mixer_utf8_cmp (gtk_entry_get_text (GTK_ENTRY (dialog->command_entry)), command) != 0)
    {
      g_signal_handlers_block_by_func (object, xfce_plugin_dialog_command_entry_changed, dialog);
      if (command != NULL)
        gtk_entry_set_text (GTK_ENTRY (dialog->command_entry), command);
      else
        gtk_entry_set_text (GTK_ENTRY (dialog->command_entry), "");
      g_signal_handlers_unblock_by_func (object, xfce_plugin_dialog_command_entry_changed, dialog);
    }

  g_free (command);
}


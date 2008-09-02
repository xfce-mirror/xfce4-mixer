/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/xfce-mixer-card.h"
#include "libxfce4mixer/xfce-mixer-card-combo.h"
#include "libxfce4mixer/xfce-mixer-track-combo.h"
#include "libxfce4mixer/xfce-mixer-utilities.h"
#include "libxfce4mixer/xfce-mixer-track-type.h"

#include "xfce-plugin-dialog.h"



static void xfce_plugin_dialog_class_init             (XfcePluginDialogClass *klass);
static void xfce_plugin_dialog_init                   (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_dispose                (GObject               *object);
static void xfce_plugin_dialog_finalize               (GObject               *object);
static void xfce_plugin_dialog_create_contents        (XfcePluginDialog      *dialog,
                                                       const gchar           *card_name,
                                                       const gchar           *track_name,
                                                       const gchar           *command);
static void xfce_plugin_dialog_soundcard_changed      (XfceMixerCardCombo    *combo,
                                                       XfceMixerCard         *card,
                                                       XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_update_tracks          (XfcePluginDialog      *dialog,
                                                       XfceMixerCard         *card);
static void xfce_plugin_dialog_track_changed          (XfceMixerTrackCombo   *combo,
                                                       GstMixerTrack         *track,
                                                       XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_command_button_clicked (XfcePluginDialog      *dialog);



struct _XfcePluginDialogClass
{
  XfceTitledDialogClass __parent__;
};

struct _XfcePluginDialog
{
  XfceTitledDialog __parent__;

  GtkWidget    *card_combo;
  GtkWidget    *track_combo;
  GtkWidget    *command_entry;
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
}



static void
xfce_plugin_dialog_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_plugin_dialog_parent_class)->dispose) (object);
}



static void
xfce_plugin_dialog_finalize (GObject *object)
{
  XfcePluginDialog *dialog = XFCE_PLUGIN_DIALOG (object);

  (*G_OBJECT_CLASS (xfce_plugin_dialog_parent_class)->finalize) (object);
}



GtkWidget*
xfce_plugin_dialog_new (const gchar *card_name,
                        const gchar *track_name,
                        const gchar *command)
{
  XfcePluginDialog *dialog;
  
  dialog = XFCE_PLUGIN_DIALOG (g_object_new (TYPE_XFCE_PLUGIN_DIALOG, NULL));

  xfce_plugin_dialog_create_contents (dialog, card_name, track_name, command);

  return GTK_WIDGET (dialog);
}



static void 
xfce_plugin_dialog_soundcard_changed (XfceMixerCardCombo *combo,
                                      XfceMixerCard      *card,
                                      XfcePluginDialog *dialog)
{
  xfce_plugin_dialog_update_tracks (dialog, card);
}



static void 
xfce_plugin_dialog_update_tracks (XfcePluginDialog *dialog,
                                  XfceMixerCard    *card)
{
  xfce_mixer_track_combo_set_soundcard (XFCE_MIXER_TRACK_COMBO (dialog->track_combo), card);
}



void
xfce_plugin_dialog_get_data (XfcePluginDialog *dialog,
                             XfceMixerCard   **card,
                             GstMixerTrack   **track,
                             gchar           **command)
{
  XfceMixerCard *active_card;
  GstMixerTrack *active_track;

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));

  active_card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo));

  if (G_LIKELY (IS_XFCE_MIXER_CARD (active_card)))
    *card = XFCE_MIXER_CARD (g_object_ref (G_OBJECT (active_card)));

  active_track = xfce_mixer_track_combo_get_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo));

  if (G_LIKELY (GST_IS_MIXER_TRACK (active_track)))
    *track = active_track;

  *command = g_strdup (gtk_entry_get_text (GTK_ENTRY (dialog->command_entry)));
}



static void 
xfce_plugin_dialog_track_changed (XfceMixerTrackCombo *combo,
                                  GstMixerTrack       *track,
                                  XfcePluginDialog    *dialog)
{
  g_debug ("track changed to: %s", GST_MIXER_TRACK (track)->label);
}



static void
xfce_plugin_dialog_create_contents (XfcePluginDialog *dialog,
                                    const gchar      *card_name,
                                    const gchar      *track_name,
                                    const gchar      *command)
{
  GtkCellRenderer *renderer;
  XfceMixerCard   *card = NULL;
  GtkWidget       *alignment;
  GtkWidget       *vbox;
  GtkWidget       *hbox;
  GtkWidget       *button;
  GtkWidget       *label;
  GtkWidget       *card_frame;
  GtkWidget       *track_frame;
  gchar           *title;

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-mixer");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Mixer Plugin"));

  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), _("Please select which mixer track should be used by the plugin"));
  
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

  dialog->card_combo = xfce_mixer_card_combo_new (card_name);
  g_signal_connect (G_OBJECT (dialog->card_combo), "soundcard-changed", G_CALLBACK (xfce_plugin_dialog_soundcard_changed), dialog);
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

  card = card_name == NULL ? NULL : xfce_mixer_utilities_get_card_by_name (card_name);

  dialog->track_combo = xfce_mixer_track_combo_new (card, track_name);
  g_signal_connect (G_OBJECT (dialog->track_combo), "track-changed", G_CALLBACK (xfce_plugin_dialog_track_changed), dialog);
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
  gtk_entry_set_text (GTK_ENTRY (dialog->command_entry), command);
  gtk_box_pack_start (GTK_BOX (hbox), dialog->command_entry, TRUE, TRUE, 0);
  gtk_widget_show (dialog->command_entry);

  button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (xfce_plugin_dialog_command_button_clicked), dialog);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 0);
  gtk_widget_show (button);
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

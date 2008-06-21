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

#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/xfce-mixer-card.h"
#include "libxfce4mixer/xfce-mixer-card-combo.h"
#include "libxfce4mixer/xfce-mixer-track-combo.h"
#include "libxfce4mixer/xfce-mixer-utilities.h"
#include "libxfce4mixer/xfce-mixer-track-type.h"

#include "xfce-plugin-dialog.h"



static void xfce_plugin_dialog_class_init        (XfcePluginDialogClass *klass);
static void xfce_plugin_dialog_init              (XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_dispose           (GObject               *object);
static void xfce_plugin_dialog_finalize          (GObject               *object);
static void xfce_plugin_dialog_create_contents   (XfcePluginDialog      *dialog,
                                                  const gchar           *card_name,
                                                  const gchar           *track_name);
static void xfce_plugin_dialog_soundcard_changed (XfceMixerCardCombo    *combo,
                                                  XfceMixerCard         *card,
                                                  XfcePluginDialog      *dialog);
static void xfce_plugin_dialog_update_tracks     (XfcePluginDialog      *dialog,
                                                  XfceMixerCard         *card);
static void xfce_plugin_dialog_track_changed     (XfceMixerTrackCombo   *combo,
                                                  GstMixerTrack         *track,
                                                  XfcePluginDialog      *dialog);



struct _XfcePluginDialogClass
{
  XfceTitledDialogClass __parent__;
};

struct _XfcePluginDialog
{
  XfceTitledDialog __parent__;

  GtkWidget    *card_combo;
  GtkWidget    *track_combo;
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
                        const gchar *track_name)
{
  XfcePluginDialog *dialog;
  
  dialog = XFCE_PLUGIN_DIALOG (g_object_new (TYPE_XFCE_PLUGIN_DIALOG, NULL));

  xfce_plugin_dialog_create_contents (dialog, card_name, track_name);

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
                             GstMixerTrack   **track)
{
  XfceMixerCard *active_card;
  GstMixerTrack *active_track;

  g_return_if_fail (IS_XFCE_PLUGIN_DIALOG (dialog));

  active_card = xfce_mixer_card_combo_get_active_card (XFCE_MIXER_CARD_COMBO (dialog->card_combo));

  if (G_LIKELY (IS_XFCE_MIXER_CARD (active_card)))
    *card = XFCE_MIXER_CARD (g_object_ref (G_OBJECT (active_card)));

  active_track = xfce_mixer_track_combo_get_active_track (XFCE_MIXER_TRACK_COMBO (dialog->track_combo));

  if (G_LIKELY (GST_IS_MIXER_TRACK (active_track)))
    *track = GST_MIXER_TRACK (g_object_ref (G_OBJECT (active_track)));
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
                                    const gchar      *track_name)
{
  GtkCellRenderer *renderer;
  XfceMixerCard   *card = NULL;
  GtkWidget       *vbox;
  GtkWidget       *button;
  GtkWidget       *label;
  GtkWidget       *card_frame;
  GtkWidget       *track_frame;
  gchar           *title;

  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-mixer");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Mixer Plugin Settings"));

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

  card_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (card_frame), GTK_SHADOW_NONE);
  gtk_box_pack_start (GTK_BOX (vbox), card_frame, FALSE, TRUE, 0);
  gtk_widget_show (card_frame);

  dialog->card_combo = xfce_mixer_card_combo_new (card_name);
  g_signal_connect (G_OBJECT (dialog->card_combo), "soundcard-changed", G_CALLBACK (xfce_plugin_dialog_soundcard_changed), dialog);
  gtk_container_add (GTK_CONTAINER (card_frame), dialog->card_combo);
  gtk_widget_show (dialog->card_combo);

  label = gtk_label_new (NULL);
  title = g_strdup_printf ("<span weight='bold'>%s</span>", _("Mixer track"));
  gtk_label_set_markup (GTK_LABEL (label), title);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (title);

  track_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (track_frame), GTK_SHADOW_NONE);
  gtk_box_pack_start (GTK_BOX (vbox), track_frame, FALSE, TRUE, 0);
  gtk_widget_show (track_frame);

  card = card_name == NULL ? NULL : xfce_mixer_utilities_get_card_by_name (card_name);

  dialog->track_combo = xfce_mixer_track_combo_new (card, track_name);
  g_signal_connect (G_OBJECT (dialog->track_combo), "track-changed", G_CALLBACK (xfce_plugin_dialog_track_changed), dialog);
  gtk_container_add (GTK_CONTAINER (track_frame), dialog->track_combo);
  gtk_widget_show (dialog->track_combo);
}

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

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-window.h"
#include "xfce-mixer-controls-dialog.h"



enum
{
  VISIBLE_COLUMN,
  NAME_COLUMN
};



static void   xfce_mixer_controls_dialog_dispose              (GObject                      *object);
static void   xfce_mixer_controls_dialog_finalize             (GObject                      *object);
static void   xfce_mixer_controls_dialog_response             (GtkDialog                    *dialog,
                                                               gint                          response_id);
static void   xfce_mixer_controls_dialog_create_contents      (XfceMixerControlsDialog      *dialog);
static void   xfce_mixer_controls_dialog_update_preferences   (XfceMixerControlsDialog      *dialog);
static void   xfce_mixer_controls_dialog_control_toggled      (GtkCellRendererToggle        *renderer,
                                                               gchar                        *path,
                                                               XfceMixerControlsDialog      *dialog);
static void  xfce_mixer_controls_dialog_bus_message           (GstBus                       *bus,
                                                               GstMessage                   *message,
                                                               XfceMixerControlsDialog      *dialog);



struct _XfceMixerControlsDialog
{
  XfceTitledDialog __parent__;

  XfceMixerWindow      *parent;
  XfceMixerPreferences *preferences;
  GstElement           *card;
  gulong                signal_handler_id;

  GtkWidget            *frame;
  GtkListStore         *store;
};



G_DEFINE_TYPE (XfceMixerControlsDialog, xfce_mixer_controls_dialog, XFCE_TYPE_TITLED_DIALOG)



static void
xfce_mixer_controls_dialog_class_init (XfceMixerControlsDialogClass *klass)
{
  GObjectClass   *gobject_class;
  GtkDialogClass *dialog_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_controls_dialog_dispose;
  gobject_class->finalize = xfce_mixer_controls_dialog_finalize;

  dialog_class = GTK_DIALOG_CLASS (klass);
  dialog_class->response = xfce_mixer_controls_dialog_response;
}



static void
xfce_mixer_controls_dialog_init (XfceMixerControlsDialog *dialog)
{
  GtkWidget *button;

  dialog->preferences = xfce_mixer_preferences_get ();

  dialog->signal_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_controls_dialog_bus_message), dialog);

  dialog->card = NULL;

  dialog->frame = NULL;

  gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 350);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "preferences-desktop");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Select Controls"));

  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), _("Select which controls should be visible"));

  button = gtk_button_new_with_mnemonic (_("_Close"));
  gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_BUTTON));
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CLOSE);
  gtk_widget_show (button);
}



static void
xfce_mixer_controls_dialog_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_controls_dialog_parent_class)->dispose) (object);
}



static void
xfce_mixer_controls_dialog_finalize (GObject *object)
{
  XfceMixerControlsDialog *dialog = XFCE_MIXER_CONTROLS_DIALOG (object);

  if (dialog->signal_handler_id > 0)
    {
      xfce_mixer_bus_disconnect (dialog->signal_handler_id);
      dialog->signal_handler_id = 0;
    }

  g_object_unref (G_OBJECT (dialog->preferences));

  (*G_OBJECT_CLASS (xfce_mixer_controls_dialog_parent_class)->finalize) (object);
}



static void
xfce_mixer_controls_dialog_response (GtkDialog *dialog,
                                     gint       response_id)
{
  xfce_mixer_controls_dialog_update_preferences (XFCE_MIXER_CONTROLS_DIALOG (dialog));
}



GtkWidget*
xfce_mixer_controls_dialog_new (XfceMixerWindow *window)
{
  XfceMixerControlsDialog *dialog;

  dialog = g_object_new (XFCE_TYPE_MIXER_CONTROLS_DIALOG, NULL);
  dialog->parent = window;

  dialog->card = xfce_mixer_window_get_active_card (window);

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (dialog->parent));

  xfce_mixer_controls_dialog_create_contents (dialog);

  return GTK_WIDGET (dialog);
}



static void
xfce_mixer_controls_dialog_create_contents (XfceMixerControlsDialog *dialog)
{
  GtkTreeViewColumn    *column;
  GtkCellRenderer      *renderer;
  GtkWidget            *view;
  GtkWidget            *scrollwin;

  dialog->store = gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);

  dialog->frame = gtk_frame_new (NULL);
  g_object_set (G_OBJECT (dialog->frame), "margin-top", 6, "margin-bottom", 6, NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (dialog->frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), dialog->frame);
  gtk_widget_show (dialog->frame);

  scrollwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_IN);
  g_object_set (G_OBJECT (scrollwin), "halign", GTK_ALIGN_FILL, "hexpand", TRUE, "valign", GTK_ALIGN_FILL, "vexpand", TRUE, NULL);
  gtk_container_add (GTK_CONTAINER (dialog->frame), scrollwin);
  gtk_widget_show (scrollwin);

  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (dialog->store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);
  gtk_container_add (GTK_CONTAINER (scrollwin), view);
  gtk_widget_show (view);

  renderer = gtk_cell_renderer_toggle_new ();
  g_object_set (G_OBJECT (renderer), "activatable", TRUE, NULL);
  g_signal_connect (renderer, "toggled", G_CALLBACK (xfce_mixer_controls_dialog_control_toggled), dialog);
  column = gtk_tree_view_column_new_with_attributes ("Visible", renderer, "active", VISIBLE_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Control", renderer, "text", NAME_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (view), column);

  xfce_mixer_controls_dialog_update_dialog (dialog);
}



static void
xfce_mixer_controls_dialog_update_contents (XfceMixerControlsDialog *dialog)
{
  g_return_if_fail (XFCE_IS_MIXER_CONTROLS_DIALOG (dialog));

  gtk_widget_destroy (dialog->frame);
  xfce_mixer_controls_dialog_create_contents (dialog);
}



void
xfce_mixer_controls_dialog_update_dialog (XfceMixerControlsDialog *dialog)
{
  XfceMixerPreferences *preferences;
  const GList          *iter;
  GstMixerTrack        *track;
  const gchar          *track_label;
  gboolean              visible;
  GtkTreeIter           tree_iter;

  /* Clear the list before recreating it below */
  gtk_list_store_clear (dialog->store);

  if (G_UNLIKELY (!GST_IS_MIXER (dialog->card)))
    return;

  preferences = xfce_mixer_preferences_get ();

  for (iter = gst_mixer_list_tracks (GST_MIXER (dialog->card));
       iter != NULL;
       iter = g_list_next (iter))
    {
      track = GST_MIXER_TRACK (iter->data);
      track_label = xfce_mixer_get_track_label (track);

      visible = xfce_mixer_preferences_get_control_visible (preferences, track_label);

      gtk_list_store_append (dialog->store, &tree_iter);
      gtk_list_store_set (dialog->store, &tree_iter,
                        VISIBLE_COLUMN, visible,
                        NAME_COLUMN, track_label,
                        -1);
    }

  g_object_unref (preferences);
}



static void
xfce_mixer_controls_dialog_update_preferences (XfceMixerControlsDialog *dialog)
{
  XfceMixerPreferences  *preferences;
  GPtrArray             *controls;
  GtkTreeModel          *model;
  GtkTreeIter            iter;
  gboolean               visible;
  GValue                *name;

  preferences = xfce_mixer_preferences_get ();

  controls = g_ptr_array_new ();

  model = GTK_TREE_MODEL (dialog->store);
  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      do
        {
          gtk_tree_model_get (model, &iter, VISIBLE_COLUMN, &visible, -1);

          if (visible)
            {
              name = g_new0 (GValue, 1);
              gtk_tree_model_get_value (model, &iter, NAME_COLUMN, name);
              g_ptr_array_add (controls, name);
            }
        }
      while (gtk_tree_model_iter_next(model, &iter));
    }

  xfce_mixer_preferences_set_controls (preferences, controls);

  xfconf_array_free (controls);

  g_object_unref (preferences);
}



static void
xfce_mixer_controls_dialog_control_toggled (GtkCellRendererToggle   *renderer,
                                            gchar                   *path,
                                            XfceMixerControlsDialog *dialog)
{
  GtkTreeIter  iter;

  if (G_UNLIKELY (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (dialog->store), &iter, path)))
    return;

  if (!gtk_cell_renderer_toggle_get_active (renderer))
    gtk_list_store_set (dialog->store, &iter, VISIBLE_COLUMN, TRUE, -1);
  else
    gtk_list_store_set (dialog->store, &iter, VISIBLE_COLUMN, FALSE, -1);

  xfce_mixer_controls_dialog_update_preferences (dialog);
}



void
xfce_mixer_controls_dialog_set_soundcard (XfceMixerControlsDialog *dialog,
                                          GstElement              *card)
{
  g_return_if_fail (XFCE_IS_MIXER_CONTROLS_DIALOG (dialog));
  g_return_if_fail (GST_IS_MIXER (card));

  dialog->card = card;

  /* Recreate contents corresponding to the new card */
  xfce_mixer_controls_dialog_update_contents (dialog);
}



static void
xfce_mixer_controls_dialog_bus_message (GstBus                  *bus,
                                        GstMessage              *message,
                                        XfceMixerControlsDialog *dialog)
{
  if (!GST_IS_MIXER (dialog->card) || GST_MESSAGE_SRC (message) != GST_OBJECT (dialog->card))
    return;

  /* Rebuild track list if the tracks for this card have changed */
  if (gst_mixer_message_get_type (message) == GST_MIXER_MESSAGE_MIXER_CHANGED)
    xfce_mixer_controls_dialog_update_contents (dialog);
}

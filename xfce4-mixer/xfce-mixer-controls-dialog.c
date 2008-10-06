/* $Id$ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your dialog) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include "xfce-mixer-window.h"
#include "xfce-mixer-controls-dialog.h"
#include "libxfce4mixer/xfce-mixer-preferences.h"



#define VISIBLE_COLUMN 0
#define NAME_COLUMN    1



static void   xfce_mixer_controls_dialog_class_init           (XfceMixerControlsDialogClass *klass);
static void   xfce_mixer_controls_dialog_init                 (XfceMixerControlsDialog      *dialog);
static void   xfce_mixer_controls_dialog_dispose              (GObject                      *object);
static void   xfce_mixer_controls_dialog_finalize             (GObject                      *object);
static void   xfce_mixer_controls_dialog_response             (GtkDialog                    *dialog,
                                                               gint                          response_id);
static void   xfce_mixer_controls_dialog_create_contents      (XfceMixerControlsDialog      *dialog);
static void   xfce_mixer_controls_dialog_control_toggled      (GtkCellRendererToggle        *renderer,
                                                               gchar                        *path,
                                                               XfceMixerControlsDialog      *dialog);



struct _XfceMixerControlsDialogClass
{
  XfceTitledDialogClass __parent__;
};

struct _XfceMixerControlsDialog
{
  XfceTitledDialog __parent__;

  XfceMixerWindow      *parent;
  XfceMixerPreferences *preferences;
  GstElement           *card;

  GtkListStore         *store;
};



static GObjectClass *xfce_mixer_controls_dialog_parent_class = NULL;



GType
xfce_mixer_controls_dialog_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerControlsDialogClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_controls_dialog_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerControlsDialog),
          0,
          (GInstanceInitFunc) xfce_mixer_controls_dialog_init,
          NULL,
        };

      type = g_type_register_static (XFCE_TYPE_TITLED_DIALOG, "XfceMixerControlsDialog", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_controls_dialog_class_init (XfceMixerControlsDialogClass *klass)
{
  GObjectClass   *gobject_class;
  GtkDialogClass *dialog_class;

  /* Determine parent type class */
  xfce_mixer_controls_dialog_parent_class = g_type_class_peek_parent (klass);

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

  gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 350);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "preferences-desktop");
  gtk_window_set_title (GTK_WINDOW (dialog), _("Select Controls"));

  xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dialog), _("Please select which controls you want to be visible"));

  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
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

  g_object_unref (G_OBJECT (dialog->preferences));

  (*G_OBJECT_CLASS (xfce_mixer_controls_dialog_parent_class)->finalize) (object);
}



static gboolean
xfce_mixer_controls_dialog_collect_visible_controls (GtkTreeModel *model,
                                                     GtkTreePath  *path,
                                                     GtkTreeIter  *iter,
                                                     GList       **controls)
{
  gboolean visible;
  gchar   *name;

  gtk_tree_model_get (model, iter, VISIBLE_COLUMN, &visible, NAME_COLUMN, &name, -1);

  if (G_LIKELY (visible && name != NULL))
    *controls = g_list_append (*controls, name);

  return FALSE;
}



static void
xfce_mixer_controls_dialog_response (GtkDialog *dialog,
                                     gint       response_id)
{
  XfceMixerControlsDialog *mixer_dialog = XFCE_MIXER_CONTROLS_DIALOG (dialog);
  XfceMixerPreferences    *preferences;
  GList                   *visible_controls = NULL;
  gchar                  **controls;
  gint                     i;

  gtk_tree_model_foreach (GTK_TREE_MODEL (mixer_dialog->store), 
                          (GtkTreeModelForeachFunc) xfce_mixer_controls_dialog_collect_visible_controls,
                          &visible_controls);

  preferences = xfce_mixer_preferences_get ();

  if (G_LIKELY (g_list_length (visible_controls) > 0))
    {
      controls = g_new0 (gchar *, g_list_length (visible_controls) + 1);
      controls[g_list_length (visible_controls)] = NULL;

      for (i = 0; i < g_list_length (visible_controls); ++i)
        controls[i] = (gchar *) g_list_nth (visible_controls, i)->data;

      xfce_mixer_preferences_set_visible_controls (preferences, mixer_dialog->card, controls);

      g_strfreev (controls);
    }
  else
    xfce_mixer_preferences_set_visible_controls (preferences, mixer_dialog->card, NULL);

  g_object_unref (preferences);

  g_list_free (visible_controls);
}



GtkWidget*
xfce_mixer_controls_dialog_new (XfceMixerWindow *window)
{
  XfceMixerControlsDialog *dialog;

  dialog = g_object_new (TYPE_XFCE_MIXER_CONTROLS_DIALOG, NULL);
  dialog->parent = window;
  dialog->card = xfce_mixer_window_get_active_card (window);

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (dialog->parent));

  xfce_mixer_controls_dialog_create_contents (dialog);

  return GTK_WIDGET (dialog);
}



static void
xfce_mixer_controls_dialog_create_contents (XfceMixerControlsDialog *dialog)
{
  XfceMixerPreferences *preferences;
  GtkTreeViewColumn    *column;
  GtkCellRenderer      *renderer;
  GstElement           *card;
  const GList          *iter;
  GtkTreeIter           tree_iter;
  GtkWidget            *view;
  GtkWidget            *frame;
  GtkWidget            *scrollwin;
  GList                *item;
  gboolean              visible;

  dialog->store = gtk_list_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);
  gtk_widget_show (frame);

  scrollwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (frame), scrollwin);
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

  preferences = xfce_mixer_preferences_get ();
  card = xfce_mixer_window_get_active_card (dialog->parent);

  if (G_LIKELY (card != NULL))
    {
      for (iter = gst_mixer_list_tracks (GST_MIXER (dialog->card)); 
           iter != NULL; 
           iter = g_list_next (iter))
        {
          visible = xfce_mixer_preferences_get_control_visible (preferences, dialog->card, iter->data);

          gtk_list_store_append (dialog->store, &tree_iter);
          gtk_list_store_set (dialog->store, &tree_iter, 
                              VISIBLE_COLUMN, visible, 
                              NAME_COLUMN, GST_MIXER_TRACK (iter->data)->label, 
                              -1);
        }
    }

  g_object_unref (preferences);
}



static void
xfce_mixer_controls_dialog_control_toggled (GtkCellRendererToggle   *renderer,
                                            gchar                   *path,
                                            XfceMixerControlsDialog *dialog)
{
  GtkTreeIter  iter;
  gchar       *name;

  if (G_UNLIKELY (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (dialog->store), &iter, path)))
    return;

  gtk_tree_model_get (GTK_TREE_MODEL (dialog->store), &iter, NAME_COLUMN, &name, -1);

  if (!gtk_cell_renderer_toggle_get_active (renderer))
    gtk_list_store_set (dialog->store, &iter, VISIBLE_COLUMN, TRUE, -1);
  else
    gtk_list_store_set (dialog->store, &iter, VISIBLE_COLUMN, FALSE, -1);

  g_free (name);
}

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
#include <libxfce4ui/libxfce4ui.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-container.h"
#include "xfce-mixer-track.h"
#include "xfce-mixer-switch.h"
#include "xfce-mixer-option.h"



enum
{
  PROP_0,
  PROP_CARD,
};



static void xfce_mixer_container_constructed     (GObject                 *object);
static void xfce_mixer_container_finalize        (GObject                 *object);
static void xfce_mixer_container_get_property    (GObject                 *object,
                                                  guint                    prop_id,
                                                  GValue                  *value,
                                                  GParamSpec              *pspec);
static void xfce_mixer_container_set_property    (GObject                 *object,
                                                  guint                    prop_id,
                                                  const GValue            *value,
                                                  GParamSpec              *pspec);
static void xfce_mixer_container_create_contents (XfceMixerContainer      *mixer_container);
static void xfce_mixer_container_bus_message     (GstBus                  *bus,
                                                  GstMessage              *message,
                                                  XfceMixerContainer      *mixer_container);



struct _XfceMixerContainerClass
{
  GtkNotebookClass __parent__;
};

struct _XfceMixerContainer
{
  GtkNotebook  __parent__;

  GstElement  *card;

  GHashTable  *widgets;

  gulong       message_handler_id;
};



G_DEFINE_TYPE (XfceMixerContainer, xfce_mixer_container, GTK_TYPE_NOTEBOOK)



static void
xfce_mixer_container_class_init (XfceMixerContainerClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructed = xfce_mixer_container_constructed;
  gobject_class->finalize = xfce_mixer_container_finalize;
  gobject_class->get_property = xfce_mixer_container_get_property;
  gobject_class->set_property = xfce_mixer_container_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_CARD,
                                   g_param_spec_object ("card",
                                                        "card",
                                                        "card",
                                                        GST_TYPE_ELEMENT,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE));
}



static void
xfce_mixer_container_init (XfceMixerContainer *mixer_container)
{
  mixer_container->card = NULL;
  mixer_container->widgets = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  mixer_container->message_handler_id = 0;
}



static void
xfce_mixer_container_constructed (GObject *object)
{
  XfceMixerContainer *mixer_container = XFCE_MIXER_CONTAINER (object);

  /* Create the content */
  xfce_mixer_container_create_contents (XFCE_MIXER_CONTAINER (mixer_container));

  mixer_container->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_container_bus_message), mixer_container);

  (*G_OBJECT_CLASS (xfce_mixer_container_parent_class)->constructed) (object);
}



static void
xfce_mixer_container_finalize (GObject *object)
{
  XfceMixerContainer *mixer_container = XFCE_MIXER_CONTAINER (object);

  xfce_mixer_bus_disconnect (mixer_container->message_handler_id);

  g_object_unref (mixer_container->card);
  g_hash_table_unref (mixer_container->widgets);

  (*G_OBJECT_CLASS (xfce_mixer_container_parent_class)->finalize) (object);
}



static void
xfce_mixer_container_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  XfceMixerContainer *mixer_container = XFCE_MIXER_CONTAINER (object);

  switch (prop_id)
    {
    case PROP_CARD:
      g_value_set_object (value, mixer_container->card);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_mixer_container_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  XfceMixerContainer *mixer_container = XFCE_MIXER_CONTAINER (object);

  switch (prop_id)
    {
    case PROP_CARD:
      if (mixer_container->message_handler_id != 0)
        xfce_mixer_bus_disconnect (mixer_container->message_handler_id);
      mixer_container->card = g_value_dup_object (value);
      xfce_mixer_container_update_contents (mixer_container);
      if (GST_IS_MIXER (mixer_container->card))
        mixer_container->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_container_bus_message), mixer_container);
      else
        mixer_container->message_handler_id = 0;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GtkWidget *
xfce_mixer_container_new (GstElement *card)
{
  GObject *object = NULL;

  object = g_object_new (TYPE_XFCE_MIXER_CONTAINER, "card", card, NULL);

  return GTK_WIDGET (object);
}



static void
xfce_mixer_container_create_contents (XfceMixerContainer *mixer_container)
{
  XfceMixerPreferences *preferences;
  XfceMixerTrackType    type;
  GstMixerTrack        *track;
  const GList          *iter;
  const gchar          *titles[4] = { N_("_Playback"), N_("C_apture"), N_("S_witches"), N_("_Options") };
  GtkWidget            *track_widget;
  GtkWidget            *track_label_widget;
  GtkWidget            *labels[4];
  GtkWidget            *scrollwins[4];
  GtkWidget            *views[4];
  GtkWidget            *last_separator[4] = { NULL, NULL, NULL, NULL };
  GtkWidget            *vbox;
  GtkWidget            *label1;
  GtkWidget            *label2;
  GtkWidget            *label3;
  const gchar          *track_label;
  gchar                *option_track_label;
  guint                 num_children[4] = { 0, 0, 0, 0 };
  gboolean              no_controls_visible = TRUE;
  gint                  i;

  g_return_if_fail (IS_XFCE_MIXER_CONTAINER (mixer_container));

  preferences = xfce_mixer_preferences_get ();

  /* Create widgets for all four tabs */
  for (i = 0; i < 4; ++i)
    {
      labels[i] = gtk_label_new_with_mnemonic (_(titles[i]));
      scrollwins[i] = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwins[i]), GTK_SHADOW_IN);
      gtk_container_set_border_width (GTK_CONTAINER (scrollwins[i]), 6);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwins[i]), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

      views[i] = gtk_grid_new ();
      g_object_set (G_OBJECT (views[i]), "row-spacing", 6, "column-spacing", 12, "border-width", 6, NULL);
      gtk_container_add (GTK_CONTAINER (scrollwins[i]), views[i]);
      gtk_viewport_set_shadow_type (GTK_VIEWPORT (gtk_bin_get_child (GTK_BIN (scrollwins[i]))), GTK_SHADOW_NONE);
      gtk_widget_show (views[i]);
      gtk_widget_show (scrollwins[i]);
    }

  /* Create controls for all mixer tracks */
  if (GST_IS_MIXER (mixer_container->card))
    {
      for (iter = gst_mixer_list_tracks (GST_MIXER (mixer_container->card)); iter != NULL; iter = g_list_next (iter))
        {
          track = GST_MIXER_TRACK (iter->data);

          track_label = xfce_mixer_get_track_label (track);

          if (!xfce_mixer_preferences_get_control_visible (preferences, track_label))
            continue;

          /* Determine the type of the mixer track */
          type = xfce_mixer_track_type_new (track);

          switch (type) 
            {
            case XFCE_MIXER_TRACK_TYPE_PLAYBACK:
              /* Create a regular volume control for this track */
              track_label_widget = gtk_label_new (track_label);
              gtk_grid_attach (GTK_GRID (views[0]), track_label_widget, num_children[0], 0, 1, 1);
              gtk_widget_show (track_label_widget);
              track_widget = xfce_mixer_track_new (mixer_container->card, track);
              g_object_set (G_OBJECT (track_widget), "valign", GTK_ALIGN_FILL, "vexpand", TRUE, NULL);
              gtk_grid_attach (GTK_GRID (views[0]), track_widget, num_children[0], 1, 1, 1);
              gtk_widget_show (track_widget);
              num_children[0]++;

              /* Append a separator. The last one will be destroyed later */
              last_separator[0] = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
              gtk_grid_attach (GTK_GRID (views[0]), last_separator[0], num_children[0], 0, 1, 2);
              gtk_widget_show (last_separator[0]);
              num_children[0]++;

              /* Add the track to the hash table */
              g_hash_table_insert (mixer_container->widgets, g_strdup (track_label), track_widget);
              break;

            case XFCE_MIXER_TRACK_TYPE_CAPTURE:
              /* Create a regular volume control for this track */
              track_label_widget = gtk_label_new (track_label);
              gtk_grid_attach (GTK_GRID (views[1]), track_label_widget, num_children[1], 0, 1, 1);
              gtk_widget_show (track_label_widget);
              track_widget = xfce_mixer_track_new (mixer_container->card, track);
              g_object_set (G_OBJECT (track_widget), "valign", GTK_ALIGN_FILL, "vexpand", TRUE, NULL);
              gtk_grid_attach (GTK_GRID (views[1]), track_widget, num_children[1], 1, 1, 1);
              gtk_widget_show (track_widget);
              num_children[1]++;

              /* Append a separator. The last one will be destroyed later */
              last_separator[1] = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
              gtk_grid_attach (GTK_GRID (views[1]), last_separator[1], num_children[1], 0, 1, 2);
              gtk_widget_show (last_separator[1]);
              num_children[1]++;

              /* Add the track to the hash table */
              g_hash_table_insert (mixer_container->widgets, g_strdup (track_label), track_widget);
              break;

            case XFCE_MIXER_TRACK_TYPE_SWITCH:
              track_widget = xfce_mixer_switch_new (mixer_container->card, track);
              g_object_set (G_OBJECT (track_widget), "halign", GTK_ALIGN_FILL, "hexpand", TRUE, NULL);
              gtk_grid_attach (GTK_GRID (views[2]), track_widget, 0, num_children[2], 1, 1);
              gtk_widget_show (track_widget);

              num_children[2]++;

              /* Add the track to the hash table */
              g_hash_table_insert (mixer_container->widgets, g_strdup (track_label), track_widget);
              break;

            case XFCE_MIXER_TRACK_TYPE_OPTIONS:
              option_track_label = g_strdup_printf ("%s:", track_label);
              track_label_widget = gtk_label_new (option_track_label);
              g_object_set (G_OBJECT (track_label_widget), "halign", GTK_ALIGN_FILL, NULL);
              gtk_grid_attach (GTK_GRID (views[3]), track_label_widget, 0, num_children[3], 1, 1);
              gtk_widget_show (track_label_widget);
              g_free (option_track_label);

              track_widget = xfce_mixer_option_new (mixer_container->card, track);
              g_object_set (G_OBJECT (track_widget), "halign", GTK_ALIGN_FILL, "hexpand", TRUE, NULL);
              gtk_grid_attach (GTK_GRID (views[3]), track_widget, 1, num_children[3], 1, 1);
              gtk_widget_show (track_widget);

              num_children[3]++;

              /* Add the track to the hash table */
              g_hash_table_insert (mixer_container->widgets, g_strdup (track_label), track_widget);
              break;
            }
        }
    }

  /* Append tab or destroy all its widgets - depending on the contents of each tab */
  for (i = 0; i < 4; ++i)
    {
      /* Destroy the last separator in the tab */
      if (G_LIKELY (last_separator[i] != NULL))
        gtk_widget_destroy (last_separator[i]);

      gtk_notebook_append_page (GTK_NOTEBOOK (mixer_container), scrollwins[i], labels[i]);

      /* Hide tabs with no visible controls */
      if (num_children[i] > 0)
        no_controls_visible = FALSE;
      else
        gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (mixer_container), i));
    }

  /* Show informational message if no controls are visible */
  if (G_UNLIKELY (no_controls_visible))
    {
      label1 = gtk_label_new (_("No controls visible"));
      gtk_widget_show (label1);

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      g_object_set (G_OBJECT (vbox), "halign", GTK_ALIGN_CENTER, "hexpand", TRUE, "valign", GTK_ALIGN_CENTER, "vexpand", TRUE,
                    "border-width", 6, NULL);
      gtk_widget_show (vbox);

      label2 = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label2), _("<span weight=\"bold\" size=\"larger\">No controls visible</span>"));
      g_object_set (G_OBJECT (label2), "max-width-chars", 80, "xalign", 0.0, "wrap", TRUE, NULL);
      gtk_box_pack_start (GTK_BOX (vbox), label2, FALSE, TRUE, 0);
      gtk_widget_show (label2);

      label3 = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label3), _("In order to toggle the visibility of mixer controls, open the <b>\"Select Controls\"</b> dialog."));
      g_object_set (G_OBJECT (label3), "max-width-chars", 80, "xalign", 0.0, "wrap", TRUE, NULL);
      gtk_box_pack_start (GTK_BOX (vbox), label3, FALSE, TRUE, 0);
      gtk_widget_show (label3);

      gtk_notebook_append_page (GTK_NOTEBOOK (mixer_container), vbox, label1);
    }

  g_object_unref (preferences);
}



void
xfce_mixer_container_update_contents (XfceMixerContainer *mixer_container)
{
  gint current_tab;
  gint i;

  g_return_if_fail (IS_XFCE_MIXER_CONTAINER (mixer_container));
  g_return_if_fail (mixer_container->widgets != NULL);

  g_hash_table_remove_all (mixer_container->widgets);

  /* Remember active tab */
  current_tab = gtk_notebook_get_current_page (GTK_NOTEBOOK (mixer_container));

  /* Destroy all tabs */
  for (i = gtk_notebook_get_n_pages (GTK_NOTEBOOK (mixer_container)); i >= 0; i--)
    gtk_notebook_remove_page (GTK_NOTEBOOK (mixer_container), i);

  /* Re-create contents */
  xfce_mixer_container_create_contents (mixer_container);

  /* Restore previously active tab if possible */
  if (current_tab > 0 && current_tab < 4)
    gtk_notebook_set_current_page (GTK_NOTEBOOK (mixer_container), current_tab);
}



static void
xfce_mixer_container_bus_message (GstBus             *bus,
                                  GstMessage         *message,
                                  XfceMixerContainer *mixer_container)
{
  GstMixerOptions    *options = NULL;
  GstMixerTrack      *track = NULL;
  GtkWidget          *widget;
  gboolean            muted;
  gboolean            record;
  const gchar        *option;
  const gchar        *label;
  gint               *volumes;
  gint                num_channels;

  g_return_if_fail (IS_XFCE_MIXER_CONTAINER (mixer_container));

  if (G_UNLIKELY (GST_MESSAGE_SRC (message) != GST_OBJECT (mixer_container->card)))
    return;

  switch (gst_mixer_message_get_type (message))
    {
      case GST_MIXER_MESSAGE_MUTE_TOGGLED:
        gst_mixer_message_parse_mute_toggled (message, &track, &muted);
        label = xfce_mixer_get_track_label (track);
        xfce_mixer_debug ("Track '%s' was %s", label, muted ? "muted" : "unmuted");
        widget = g_hash_table_lookup (mixer_container->widgets, label);

        if (IS_XFCE_MIXER_TRACK (widget))
          xfce_mixer_track_update_mute (XFCE_MIXER_TRACK (widget));
        else if (IS_XFCE_MIXER_SWITCH (widget))
          xfce_mixer_switch_update (XFCE_MIXER_SWITCH (widget));
        break;
      case GST_MIXER_MESSAGE_RECORD_TOGGLED:
        gst_mixer_message_parse_record_toggled (message, &track, &record);
        label = xfce_mixer_get_track_label (track);
        xfce_mixer_debug ("Recording on track '%s' was %s", label, record ? "turned on" : "turned off");
        widget = g_hash_table_lookup (mixer_container->widgets, label);

        if (IS_XFCE_MIXER_TRACK (widget))
          xfce_mixer_track_update_record (XFCE_MIXER_TRACK (widget));
        else if (IS_XFCE_MIXER_SWITCH (widget))
          xfce_mixer_switch_update (XFCE_MIXER_SWITCH (widget));
        break;
      case GST_MIXER_MESSAGE_VOLUME_CHANGED:
        gst_mixer_message_parse_volume_changed (message, &track, &volumes, &num_channels);
        label = xfce_mixer_get_track_label (track);
        xfce_mixer_debug ("Volume on track '%s' changed to %i", label, volumes[0]);
        widget = g_hash_table_lookup (mixer_container->widgets, label);

        if (IS_XFCE_MIXER_TRACK (widget))
          xfce_mixer_track_update_volume (XFCE_MIXER_TRACK (widget));
        break;
      case GST_MIXER_MESSAGE_OPTION_CHANGED:
        gst_mixer_message_parse_option_changed (message, &options, &option);
        label = xfce_mixer_get_track_label (GST_MIXER_TRACK (options));
        xfce_mixer_debug ("Option '%s' was set to '%s'", label, option);
        widget = g_hash_table_lookup (mixer_container->widgets, label);

        if (IS_XFCE_MIXER_OPTION (widget))
          xfce_mixer_option_update (XFCE_MIXER_OPTION (widget));
        break;
      case GST_MIXER_MESSAGE_MIXER_CHANGED:
        xfce_mixer_debug ("Mixer tracks have changed");
        xfce_mixer_container_update_contents (mixer_container);
        break;
      default:
        break;
    }
}

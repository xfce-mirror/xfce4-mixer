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

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer.h"
#include "xfce-mixer-track.h"
#include "xfce-mixer-switch.h"
#include "xfce-mixer-option.h"



enum
{
  PROP_0,
  PROP_CARD,
};



static void xfce_mixer_class_init      (XfceMixerClass *klass);
static void xfce_mixer_instance_init   (XfceMixer      *mixer);
static void xfce_mixer_constructed     (GObject        *object);
static void xfce_mixer_finalize        (GObject        *object);
static void xfce_mixer_get_property    (GObject        *object,
                                        guint           prop_id,
                                        GValue         *value,
                                        GParamSpec     *pspec);
static void xfce_mixer_set_property    (GObject        *object,
                                        guint           prop_id,
                                        const GValue   *value,
                                        GParamSpec     *pspec);
#ifdef HAVE_GST_MIXER_NOTIFICATION
static void xfce_mixer_bus_message     (GstBus         *bus,
                                        GstMessage     *message,
                                        XfceMixer      *mixer);
#endif



struct _XfceMixerClass
{
  GtkNotebookClass __parent__;
};

struct _XfceMixer
{
  GtkNotebook __parent__;

  GstElement *card;

  GHashTable *widgets;

  guint       message_handler_id;
};



static GObjectClass *xfce_mixer_parent_class = NULL;



GType
xfce_mixer_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_class_init,
          NULL,
          NULL,
          sizeof (XfceMixer),
          0,
          (GInstanceInitFunc) xfce_mixer_instance_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_NOTEBOOK, "XfceMixer", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_class_init (XfceMixerClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
#if GLIB_CHECK_VERSION (2,14,0)
  gobject_class->constructed = xfce_mixer_constructed;
#endif
  gobject_class->finalize = xfce_mixer_finalize;
  gobject_class->get_property = xfce_mixer_get_property;
  gobject_class->set_property = xfce_mixer_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_CARD,
                                   g_param_spec_object ("card",
                                                        "card",
                                                        "card",
                                                        GST_TYPE_ELEMENT,
                                                        G_PARAM_CONSTRUCT_ONLY | 
                                                        G_PARAM_READWRITE));
}



static void
xfce_mixer_instance_init (XfceMixer *mixer)
{
  mixer->widgets = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
  mixer->message_handler_id = 0;
}



static void
xfce_mixer_constructed (GObject *object)
{
  XfceMixer            *mixer = XFCE_MIXER (object);
  XfceMixerPreferences *preferences;
  XfceMixerTrackType    type;
  GstMixerTrack        *track;
  const GList          *iter;
  const gchar          *titles[4] = { N_("Playback"), N_("Capture"), N_("Switches"), N_("Options") };
  GtkWidget            *track_widget;
  GtkWidget            *labels[4];
  GtkWidget            *scrollwins[4];
  GtkWidget            *views[4];
  GtkWidget            *last_separator[4] = { NULL, NULL, NULL, NULL };
  GtkWidget            *label1;
  GtkWidget            *label2;
  GList                *visible_controls;
  gchar                *label;
  guint                 num_children[4] = { 0, 0, 0, 0 };
  gint                  i;

  preferences = xfce_mixer_preferences_get ();

  /* Create widgets for all four tabs */
  for (i = 0; i < 4; ++i)
    {
      labels[i] = gtk_label_new (_(titles[i]));
      scrollwins[i] = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwins[i]), GTK_SHADOW_IN);
      gtk_container_set_border_width (GTK_CONTAINER (scrollwins[i]), 6);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwins[i]), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

      if (i < 2) 
        {
          views[i] = gtk_table_new (1, 1, FALSE);
          gtk_table_set_col_spacings (GTK_TABLE (views[i]), 12);
        }
      else
        views[i] = gtk_vbox_new (FALSE, 6);

      gtk_container_set_border_width (GTK_CONTAINER (views[i]), 6);
      gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollwins[i]), views[i]);
      gtk_viewport_set_shadow_type (GTK_VIEWPORT (gtk_bin_get_child (GTK_BIN (scrollwins[i]))), GTK_SHADOW_NONE);
      gtk_widget_show (views[i]);
      gtk_widget_show (scrollwins[i]);
    }

  /* Create controls for all mixer tracks */
  for (iter = gst_mixer_list_tracks (GST_MIXER (mixer->card)); iter != NULL; iter = g_list_next (iter))
    {
      track = iter->data;

      if (!xfce_mixer_preferences_get_control_visible (preferences, mixer->card, track))
        continue;

      /* Determine the type of the mixer track */
      type = xfce_mixer_track_type_new (track);

      g_object_get (GST_MIXER_TRACK (track), "label", &label, NULL);

      switch (type) 
        {
        case XFCE_MIXER_TRACK_TYPE_PLAYBACK:
          /* Create a regular volume control for this track */
          track_widget = xfce_mixer_track_new (mixer->card, track);
          gtk_table_attach (GTK_TABLE (views[0]), track_widget, 
                            num_children[0], num_children[0] + 1, 0, 1, GTK_SHRINK, GTK_FILL|GTK_EXPAND, 0, 0);
          gtk_widget_show (track_widget);
          num_children[0]++;

          /* Append a separator. The last one will be destroyed later */
          last_separator[0] = gtk_vseparator_new ();
          gtk_table_attach (GTK_TABLE (views[0]), last_separator[0], 
                            num_children[0], num_children[0] + 1, 0, 1, GTK_SHRINK, GTK_FILL|GTK_EXPAND, 0, 0);
          gtk_widget_show (last_separator[0]);
          num_children[0]++;

          /* Add the track to the hash table */
          g_hash_table_insert (mixer->widgets, label, track_widget);
          break;

        case XFCE_MIXER_TRACK_TYPE_CAPTURE:
          /* Create a regular volume control for this track */
          track_widget = xfce_mixer_track_new (mixer->card, track);
          gtk_table_attach (GTK_TABLE (views[1]), track_widget, 
                            num_children[1], num_children[1] + 1, 0, 1, GTK_SHRINK, GTK_FILL|GTK_EXPAND, 0, 0);
          gtk_widget_show (track_widget);
          num_children[1]++;

          /* Append a separator. The last one will be destroyed later */
          last_separator[1] = gtk_vseparator_new ();
          gtk_table_attach (GTK_TABLE (views[1]), last_separator[1], 
                            num_children[1], num_children[1] + 1, 0, 1, GTK_SHRINK, GTK_FILL|GTK_EXPAND, 0, 0);
          gtk_widget_show (last_separator[1]);
          num_children[1]++;

          /* Add the track to the hash table */
          g_hash_table_insert (mixer->widgets, label, track_widget);
          break;

        case XFCE_MIXER_TRACK_TYPE_SWITCH:
          track_widget = xfce_mixer_switch_new (mixer->card, track);
          gtk_box_pack_start (GTK_BOX (views[2]), track_widget, FALSE, FALSE, 0);
          gtk_widget_show (track_widget);
          num_children[2]++;

          /* Add the track to the hash table */
          g_hash_table_insert (mixer->widgets, label, track_widget);
          break;

        case XFCE_MIXER_TRACK_TYPE_OPTIONS:
          track_widget = xfce_mixer_option_new (mixer->card, track);
          gtk_box_pack_start (GTK_BOX (views[3]), track_widget, FALSE, FALSE, 0);
          gtk_widget_show (track_widget);
          num_children[3]++;

          /* Add the track to the hash table */
          g_hash_table_insert (mixer->widgets, label, track_widget);
          break;
        }

      g_free (label);
    }

  /* Append tab or destroy all its widgets - depending on the contents of each tab */
  for (i = 0; i < 4; ++i)
    {
      /* Destroy the last separator in the tab */
      if (G_LIKELY (last_separator[i] != NULL))
        gtk_widget_destroy (last_separator[i]);

      /* Check if there are controls at all for this tab */
      if (G_LIKELY (num_children[i] > 0))
        {
          /* If there are controls, create the notebook tab */
          gtk_notebook_append_page (GTK_NOTEBOOK (mixer), scrollwins[i], labels[i]);
        }
      else
        {
          /* Otherwise, destroy all created widgets */
          gtk_widget_destroy (labels[i]);
          gtk_widget_destroy (scrollwins[i]);
        }
    }

  if (G_UNLIKELY (gtk_notebook_get_n_pages (GTK_NOTEBOOK (mixer)) == 0))
    {
      label1 = gtk_label_new (_("No controls visible"));
      gtk_widget_show (label1);

      label2 = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label2), _("No controls are marked as visible. Please open the <span size='large'><b>Select Controls</b></span> dialog to select some."));
      gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
      gtk_widget_show (label2);

      gtk_notebook_append_page (GTK_NOTEBOOK (mixer), label2, label1);
    }

#ifdef HAVE_GST_MIXER_NOTIFICATION
  mixer->message_handler_id = xfce_mixer_bus_connect (G_CALLBACK (xfce_mixer_bus_message), mixer);
#endif

  g_object_unref (preferences);
}



static void
xfce_mixer_finalize (GObject *object)
{
  XfceMixer *mixer = XFCE_MIXER (object);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  xfce_mixer_bus_disconnect (mixer->message_handler_id);
#endif

  g_object_unref (mixer->card);
  g_hash_table_unref (mixer->widgets);

  (*G_OBJECT_CLASS (xfce_mixer_parent_class)->finalize) (object);
}



static void
xfce_mixer_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  XfceMixer *mixer = XFCE_MIXER (object);

  switch (prop_id)
    {
    case PROP_CARD:
      g_value_set_object (value, g_object_ref (mixer->card));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_mixer_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  XfceMixer *mixer = XFCE_MIXER (object);

  switch (prop_id)
    {
    case PROP_CARD:
      mixer->card = g_object_ref (g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GtkWidget *
xfce_mixer_new (GstElement *card)
{
  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  GObject *object = g_object_new (TYPE_XFCE_MIXER, "card", card, NULL);
#if !GLIB_CHECK_VERSION (2,14,0)
  xfce_mixer_constructed (object);
#endif
  return GTK_WIDGET (object);
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
static void
xfce_mixer_bus_message (GstBus *bus,
                        GstMessage *message,
                        XfceMixer  *mixer)
{
  GstMixerMessageType type;
  GstMixerOptions    *options = NULL;
  GstMixerTrack      *track = NULL;
  GtkWidget          *widget;
  gboolean            muted;
  gboolean            record;
  const gchar        *option;
  gint               *volumes;
  gint                num_channels;

  g_return_if_fail (IS_XFCE_MIXER (mixer));

  if (G_UNLIKELY (GST_MESSAGE_SRC (message) != GST_OBJECT (mixer->card)))
    return;

  type = gst_mixer_message_get_type (message);

  if (type == GST_MIXER_MESSAGE_MUTE_TOGGLED)
    {
      gst_mixer_message_parse_mute_toggled (message, &track, &muted);
      DBG ("Track '%s' was %s", track->label, muted ? "muted" : "unmuted");
  
      widget = g_hash_table_lookup (mixer->widgets, track->label);

      if (IS_XFCE_MIXER_TRACK (widget))
        xfce_mixer_track_update_mute (XFCE_MIXER_TRACK (widget));
      else if (IS_XFCE_MIXER_SWITCH (widget))
        xfce_mixer_switch_update (XFCE_MIXER_SWITCH (widget));
    }
  else if (type == GST_MIXER_MESSAGE_RECORD_TOGGLED)
    {
      gst_mixer_message_parse_record_toggled (message, &track, &record);
      DBG ("Recording on track '%s' was %s", track->label, record ? "turned on" : "turned off");

      widget = g_hash_table_lookup (mixer->widgets, track->label);

      if (IS_XFCE_MIXER_TRACK (widget))
        xfce_mixer_track_update_record (XFCE_MIXER_TRACK (widget));
      else if (IS_XFCE_MIXER_SWITCH (widget))
        xfce_mixer_switch_update (XFCE_MIXER_SWITCH (widget));
    }
  else if (type == GST_MIXER_MESSAGE_VOLUME_CHANGED)
    {
      gst_mixer_message_parse_volume_changed (message, &track, &volumes, &num_channels);
      DBG ("Volume on track '%s' changed to %i", track->label, volumes[0]);

      widget = g_hash_table_lookup (mixer->widgets, track->label);

      if (IS_XFCE_MIXER_TRACK (widget))
        xfce_mixer_track_update_volume (XFCE_MIXER_TRACK (widget));
    }
  else if (type == GST_MIXER_MESSAGE_OPTION_CHANGED)
    {
      gst_mixer_message_parse_option_changed (message, &options, &option);
      DBG ("Option '%s' was set to '%s'", GST_MIXER_TRACK (options)->label, option);

      widget = g_hash_table_lookup (mixer->widgets, GST_MIXER_TRACK (options)->label);

      if (IS_XFCE_MIXER_OPTION (widget))
        xfce_mixer_option_update (XFCE_MIXER_OPTION (widget));
    }
}
#endif

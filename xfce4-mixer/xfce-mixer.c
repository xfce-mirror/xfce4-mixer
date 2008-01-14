/* $Id$ */
/* vim:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "xfce-mixer.h"
#include "xfce-mixer-track-type.h"
#include "xfce-mixer-track.h"
#include "xfce-mixer-switch.h"
#include "xfce-mixer-option.h"
#include "xfce-mixer-preferences.h"



static void       xfce_mixer_class_init            (XfceMixerClass *klass);
static void       xfce_mixer_init                  (XfceMixer      *mixer);
static void       xfce_mixer_dispose               (GObject        *object);
static void       xfce_mixer_finalize              (GObject        *object);
static void       xfce_mixer_create_contents       (XfceMixer      *mixer);
#ifdef HAVE_GST_MIXER_NOTIFICATION
static gboolean   xfce_mixer_bus_message           (GstBus         *bus,
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

  XfceMixerCard *card;
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
          (GInstanceInitFunc) xfce_mixer_init,
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
  gobject_class->dispose = xfce_mixer_dispose;
  gobject_class->finalize = xfce_mixer_finalize;
}



static void
xfce_mixer_init (XfceMixer *mixer)
{
}



static void
xfce_mixer_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_parent_class)->dispose) (object);
}



static void
xfce_mixer_finalize (GObject *object)
{
  XfceMixer *mixer = XFCE_MIXER (object);

  g_object_unref (G_OBJECT (mixer->card));

  (*G_OBJECT_CLASS (xfce_mixer_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_new (XfceMixerCard *card)
{
  XfceMixer *mixer;

  g_return_val_if_fail (IS_XFCE_MIXER_CARD (card), NULL);
  
  mixer = g_object_new (TYPE_XFCE_MIXER, NULL);
  mixer->card = XFCE_MIXER_CARD (g_object_ref (card));

  xfce_mixer_create_contents (mixer);

  return GTK_WIDGET (mixer);
}



static void
xfce_mixer_create_contents (XfceMixer *mixer)
{
  XfceMixerPreferences *preferences;
  XfceMixerTrackType    type;
  GstMixerTrack        *track;
  const GList          *iter;
  const gchar          *titles[4] = { _("Playback"), _("Capture"), _("Switches"), _("Options") };
  GtkWidget            *track_widget;
  GtkWidget            *labels[4];
  GtkWidget            *scrollwins[4];
  GtkWidget            *views[4];
  GtkWidget            *last_separator[4] = { NULL, NULL, NULL, NULL };
  GtkWidget            *label1;
  GtkWidget            *label2;
  GList                *visible_controls;
  guint                 num_children[4] = { 0, 0, 0, 0 };
  gint                  i;

  xfce_mixer_card_set_ready (mixer->card);

#ifdef HAVE_GST_MIXER_NOTIFICATION
  xfce_mixer_card_connect (mixer->card, G_CALLBACK (xfce_mixer_bus_message), mixer);
#endif

  /* Create widgets for all four tabs */
  for (i = 0; i < 4; ++i)
    {
      labels[i] = gtk_label_new (titles[i]);
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

  visible_controls = xfce_mixer_card_get_visible_controls (mixer->card);

  /* Create controls for all mixer tracks */
  for (iter = xfce_mixer_card_get_tracks (mixer->card); iter != NULL; iter = g_list_next (iter))
    {
      track = iter->data;

      if (g_list_find_custom (visible_controls, track->label, (GCompareFunc) g_utf8_collate) == NULL)
        continue;

      /* Determine the type of the mixer track */
      type = xfce_mixer_track_type_new (track);

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
          break;

        case XFCE_MIXER_TRACK_TYPE_SWITCH:
          track_widget = xfce_mixer_switch_new (mixer->card, track);
          gtk_box_pack_start (GTK_BOX (views[2]), track_widget, FALSE, FALSE, 0);
          gtk_widget_show (track_widget);
          num_children[2]++;
          break;

        case XFCE_MIXER_TRACK_TYPE_OPTIONS:
          track_widget = xfce_mixer_option_new (mixer->card, track);
          gtk_box_pack_start (GTK_BOX (views[3]), track_widget, FALSE, FALSE, 0);
          gtk_widget_show (track_widget);
          num_children[3]++;
          break;
        }
    }

  g_list_foreach (visible_controls, (GFunc) g_free, NULL);
  g_list_free (visible_controls);

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
      label1 = gtk_label_new (_("No Controls Visible"));
      gtk_widget_show (label1);

      label2 = gtk_label_new (NULL);
      gtk_label_set_markup (GTK_LABEL (label2), _("No controls are marked as visible. Please open the <span size='large'><b>Select Controls</b></span> dialog to select some."));
      gtk_label_set_line_wrap (GTK_LABEL (label2), TRUE);
      gtk_widget_show (label2);

      gtk_notebook_append_page (GTK_NOTEBOOK (mixer), label2, label1);
    }
}



#ifdef HAVE_GST_MIXER_NOTIFICATION
static gboolean
xfce_mixer_bus_message (GstBus     *bus,
                        GstMessage *message,
                        XfceMixer  *mixer)
{
  GstMixerMessageType type;
  GstMixerOptions    *options = NULL;
  GstMixerTrack      *track = NULL;
  gboolean            muted;
  gboolean            record;
  const gchar        *option;

  if (G_UNLIKELY (!xfce_mixer_card_get_message_owner (mixer->card, message)))
    return TRUE;

  g_debug ("Message from card received: %s", GST_MESSAGE_TYPE_NAME (message));

  type = gst_mixer_message_get_type (message);

  if (type == GST_MIXER_MESSAGE_MUTE_TOGGLED)
    {
      gst_mixer_message_parse_mute_toggled (message, &track, &muted);
      g_debug ("Track '%s' was %s", track->label, muted ? "muted" : "unmuted");
    }
  else if (type == GST_MIXER_MESSAGE_RECORD_TOGGLED)
    {
      gst_mixer_message_parse_record_toggled (message, &track, &record);
      g_debug ("Recording on track '%s' was %s", track->label, record ? "turned on" : "turned off");
    }
  else if (type == GST_MIXER_MESSAGE_VOLUME_CHANGED)
    {
    }
  else if (type == GST_MIXER_MESSAGE_OPTION_CHANGED)
    {
      gst_mixer_message_parse_option_changed (message, &options, &option);
      g_debug ("Option '%s' was set to '%s'", GST_MIXER_TRACK (options)->label, option);
    }

  return TRUE;
}
#endif

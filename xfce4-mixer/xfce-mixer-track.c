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

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-track.h"



#define XFCE_MIXER_ICON_SIZE GTK_ICON_SIZE_MENU



static void     xfce_mixer_track_dispose                       (GObject             *object);
static void     xfce_mixer_track_finalize                      (GObject             *object);
static gboolean xfce_mixer_track_lock_button_line_expose_event (GtkWidget           *widget,
                                                                GdkEventExpose      *event,
                                                                gpointer             user_data);
static void     xfce_mixer_track_create_contents               (XfceMixerTrack      *track);
static void     xfce_mixer_track_fader_changed                 (GtkRange            *range,
                                                                XfceMixerTrack      *track);
static void     xfce_mixer_track_mute_toggled                  (GtkToggleButton     *button,
                                                                XfceMixerTrack      *track);
static void     xfce_mixer_track_lock_toggled                  (GtkToggleButton     *button,
                                                                XfceMixerTrack      *track);
static void     xfce_mixer_track_record_toggled                (GtkToggleButton     *button,
                                                                XfceMixerTrack      *track);



struct _XfceMixerTrackClass
{
  GtkVBoxClass __parent__;
};

struct _XfceMixerTrack
{
  GtkVBox        __parent__;

  GtkWidget     *mute_button;
  GtkWidget     *lock_button;
  GtkWidget     *record_button;

  GList         *channel_faders;

  GstElement    *card;
  GstMixerTrack *gst_track;

  gboolean       ignore_signals;
};



G_DEFINE_TYPE (XfceMixerTrack, xfce_mixer_track, GTK_TYPE_VBOX)



static void
xfce_mixer_track_class_init (XfceMixerTrackClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_track_dispose;
  gobject_class->finalize = xfce_mixer_track_finalize;
}



static void
xfce_mixer_track_init (XfceMixerTrack *track)
{
  track->mute_button = NULL;
  track->lock_button = NULL;
  track->record_button = NULL;

  track->ignore_signals = FALSE;

  track->channel_faders = NULL;
}



static void
xfce_mixer_track_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_track_parent_class)->dispose) (object);
}



static void
xfce_mixer_track_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_track_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_track_new (GstElement    *card,
                      GstMixerTrack *gst_track)
{
  XfceMixerTrack *track;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (gst_track), NULL);
  
  track = g_object_new (TYPE_XFCE_MIXER_TRACK, NULL);
  track->card = card;
  track->gst_track = gst_track;

  xfce_mixer_track_create_contents (track);

  return GTK_WIDGET (track);
}



static gboolean
xfce_mixer_track_lock_button_line_expose_event (GtkWidget       *widget,
                                                GdkEventExpose  *event,
                                                gpointer         user_data)
{
  GtkPositionType    position = GPOINTER_TO_INT (user_data);
  GtkAllocation      allocation;
  cairo_t           *cr;
  GtkStyle          *style = gtk_widget_get_style (widget);
  GdkPoint           points[3];
  double             line_width = 2.0;

  cr = gdk_cairo_create (gtk_widget_get_window (widget));

  gtk_widget_get_allocation (widget, &allocation);
  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    position = (position == GTK_POS_LEFT) ? GTK_POS_RIGHT : GTK_POS_LEFT;

  /* Only refresh regions where it is necessary */
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  /*
   * Draw an L-shaped line from the right/left center to the top middle of the
   * allocation
   */
  gdk_cairo_set_source_color (cr, &style->fg[GTK_STATE_NORMAL]);
  cairo_set_line_width(cr, line_width);
  if (position == GTK_POS_RIGHT)
    {
      points[0].x = 0;
      points[0].y = (gint) round ((allocation.height - line_width) / 2.0);
      points[1].x = (gint) round ((allocation.width - line_width) / 2.0);
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = 0;
    }
  else
    {
      points[0].x = allocation.width;
      points[0].y = (gint) round ((allocation.height - line_width) / 2.0);
      points[1].x = (gint) round ((allocation.width + line_width) / 2.0);
      points[1].y = points[0].y;
      points[2].x = points[1].x;
      points[2].y = 0;
    }
  cairo_move_to (cr, points[0].x, points[0].y);
  cairo_line_to (cr, points[1].x, points[1].y);
  cairo_line_to (cr, points[2].x, points[2].y);
  cairo_stroke (cr);

  cairo_destroy (cr);

  return TRUE;
}



static void
xfce_mixer_track_create_contents (XfceMixerTrack *track)
{
  gint            *volumes;
  gboolean         volume_locked = TRUE;
  gint             channel;
  const gchar     *track_label;
  gchar           *tooltip_text;
  gdouble          step;
  GtkWidget       *faders_alignment;
  GtkWidget       *faders_vbox;
  GtkWidget       *faders_hbox;
  GtkWidget       *lock_button_hbox;
  GtkWidget       *fader;
  GtkWidget       *lock_button_line1;
  GtkWidget       *lock_button_line2;
  GtkWidget       *image;
  GtkWidget       *buttons_alignment;
  GtkWidget       *buttons_hbox;
  GtkRequisition   lock_button_hbox_requisition;

  /* Get volumes of all channels of the track */
  volumes = g_new0 (gint, track->gst_track->num_channels);
  gst_mixer_get_volume (GST_MIXER (track->card), track->gst_track, volumes);

  track_label = xfce_mixer_get_track_label (track->gst_track);

  step = (gdouble) (track->gst_track->max_volume - track->gst_track->min_volume) / (gdouble) 20;

  gtk_box_set_spacing (GTK_BOX (track), 6);

  /* Center and do not expand faders and lock button */
  faders_alignment = gtk_alignment_new (0.5, 1.0, 0, 1.0);
  gtk_box_pack_start (GTK_BOX (track), faders_alignment, TRUE, TRUE, 0);
  gtk_widget_show (faders_alignment);

  faders_vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (faders_alignment), faders_vbox);
  gtk_widget_show (faders_vbox);

  faders_hbox = gtk_hbox_new (TRUE, 6);
  gtk_box_pack_start (GTK_BOX (faders_vbox), faders_hbox, TRUE, TRUE, 0);
  gtk_widget_show (faders_hbox);

  /* Create a fader for each channel */
  for (channel = 0; channel < track->gst_track->num_channels; ++channel)
    {
      tooltip_text = g_strdup_printf (_("Volume of channel %d on %s"), channel, track_label);

      fader = gtk_vscale_new_with_range (track->gst_track->min_volume, track->gst_track->max_volume, step);
      gtk_scale_set_draw_value (GTK_SCALE (fader), FALSE);
      gtk_range_set_inverted (GTK_RANGE (fader), TRUE);
      gtk_range_set_value (GTK_RANGE (fader), volumes[channel]);
      gtk_widget_set_tooltip_text (fader, tooltip_text);
      /* Make read-only tracks insensitive */
      if (GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_READONLY))
        gtk_widget_set_sensitive (fader, FALSE);
      gtk_box_pack_start (GTK_BOX (faders_hbox), fader, TRUE, TRUE, 0);
      g_signal_connect (fader, "value-changed", G_CALLBACK (xfce_mixer_track_fader_changed), track);
      gtk_widget_show (fader);

      track->channel_faders = g_list_append (track->channel_faders, fader);

      /* Equal volume across all channels means the track is locked */
      if (volume_locked && channel > 0 && volumes[channel] != volumes[channel - 1])
        volume_locked = FALSE;

      g_free (tooltip_text);
    }

  /* Create lock button with lines */
  lock_button_hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (faders_vbox), lock_button_hbox, FALSE, FALSE, 0);
  gtk_widget_show (lock_button_hbox);

  /* Left L-shaped line */
  lock_button_line1 =  gtk_drawing_area_new ();
  gtk_widget_set_size_request (lock_button_line1, 12, 8);
  gtk_box_pack_start (GTK_BOX (lock_button_hbox), lock_button_line1, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (lock_button_line1), "expose-event", G_CALLBACK (xfce_mixer_track_lock_button_line_expose_event), GINT_TO_POINTER (GTK_POS_LEFT));
  gtk_widget_show (lock_button_line1);

  /* Lock button */
  tooltip_text = g_strdup_printf (_("Lock channels for %s together"), track_label);
  track->lock_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_file (DATADIR "/pixmaps/xfce4-mixer/chain.png");
  gtk_button_set_image (GTK_BUTTON (track->lock_button), image);
  gtk_widget_set_tooltip_text (track->lock_button, tooltip_text);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->lock_button), TRUE);
  /* Make button insensitive for read-only tracks */
  if (GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_READONLY))
    gtk_widget_set_sensitive (track->lock_button, FALSE);
  g_signal_connect (track->lock_button, "toggled", G_CALLBACK (xfce_mixer_track_lock_toggled), track);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->lock_button), volume_locked);
  gtk_box_pack_start (GTK_BOX (lock_button_hbox), track->lock_button, FALSE, FALSE, 0);
  gtk_widget_show (track->lock_button);
  g_free (tooltip_text);

  /* Right L-shaped line */
  lock_button_line2 =  gtk_drawing_area_new ();
  gtk_widget_set_size_request (lock_button_line2, 12, 8);
  gtk_box_pack_start (GTK_BOX (lock_button_hbox), lock_button_line2, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (lock_button_line2), "expose-event", G_CALLBACK (xfce_mixer_track_lock_button_line_expose_event), GINT_TO_POINTER (GTK_POS_RIGHT));
  gtk_widget_show (lock_button_line2);

  /*
   * Destroy the chain button and lines and replace them with an equally sized
   * placeholder if there is only one fader
   */
  if (track->gst_track->num_channels < 2)
    {
      gtk_widget_size_request (lock_button_hbox, &lock_button_hbox_requisition);
      gtk_widget_destroy (lock_button_hbox);
      lock_button_hbox = gtk_hbox_new (FALSE, 0);
      gtk_widget_set_size_request (lock_button_hbox, lock_button_hbox_requisition.width, lock_button_hbox_requisition.height);
      gtk_box_pack_start (GTK_BOX (faders_vbox), lock_button_hbox, FALSE, FALSE, 0);
      gtk_widget_show (lock_button_hbox);
    }

  buttons_alignment = gtk_alignment_new (0.5, 1.0, 0, 0);
  gtk_box_pack_start (GTK_BOX (track), buttons_alignment, FALSE, FALSE, 0);
  gtk_widget_show (buttons_alignment);

  buttons_hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (buttons_alignment), buttons_hbox);
  gtk_widget_show (buttons_hbox);

  /* Mute button for playback tracks */
  if (xfce_mixer_track_type_new (track->gst_track) == XFCE_MIXER_TRACK_TYPE_PLAYBACK)
    {
      tooltip_text = g_strdup_printf (_("Mute/unmute %s"), track_label);

      track->mute_button = gtk_toggle_button_new ();
      image = gtk_image_new_from_icon_name ("audio-volume-high", XFCE_MIXER_ICON_SIZE);
      gtk_button_set_image (GTK_BUTTON (track->mute_button), image);
      gtk_widget_set_tooltip_text (track->mute_button, tooltip_text);
      /* Make button insensitive for tracks without mute or read-only tracks */
      if (GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_READONLY) ||
          GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_NO_MUTE))
        gtk_widget_set_sensitive (track->mute_button, FALSE);
      g_signal_connect (track->mute_button, "toggled", G_CALLBACK (xfce_mixer_track_mute_toggled), track);
      gtk_box_pack_start (GTK_BOX (buttons_hbox), track->mute_button, FALSE, FALSE, 0);
      gtk_widget_show (track->mute_button);

      g_free (tooltip_text);
    }

  /* Record button for capture tracks */
  if (G_UNLIKELY (xfce_mixer_track_type_new (track->gst_track) == XFCE_MIXER_TRACK_TYPE_CAPTURE))
    {
      tooltip_text = g_strdup_printf (_("Enable/disable audible input from %s in output"), track_label);

      track->record_button = gtk_toggle_button_new ();
      image = gtk_image_new_from_icon_name ("audio-input-microphone-muted", XFCE_MIXER_ICON_SIZE);
      gtk_button_set_image (GTK_BUTTON (track->record_button), image);
      gtk_widget_set_tooltip_text (track->record_button, tooltip_text);
      /* Make button insensitive for tracks without record or read-only tracks */
      if (GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_READONLY) ||
          GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_NO_RECORD))
        gtk_widget_set_sensitive (track->record_button, FALSE);
      g_signal_connect (track->record_button, "toggled", G_CALLBACK (xfce_mixer_track_record_toggled), track);
      gtk_box_pack_start (GTK_BOX (buttons_hbox), track->record_button, FALSE, FALSE, 0);
      gtk_widget_show (track->record_button);

      g_free (tooltip_text);
    }

  /* Some of the mixer controls need to be updated before they can be used */
  xfce_mixer_track_update_mute (track);
  xfce_mixer_track_update_record (track);

  /* Free volume array */
  g_free (volumes);
}



static void 
xfce_mixer_track_fader_changed (GtkRange       *range,
                                XfceMixerTrack *track)
{
  GList          *iter;
  gint           *old_volumes;
  gint           *new_volumes;
  gint            channel;
  gint            old_max_volume;
  gint            new_max_volume;

  /* Locking mechanism: If locked, the volume change should be applied to all
   * channels, but only one should deliver the change to GStreamer. */
  static gboolean locked = FALSE;

  /* Do nothing if signals are to be ignored */
  if (G_UNLIKELY (track->ignore_signals))
    return;

  /* Do nothing if another fader already takes care of everything */
  if (G_UNLIKELY (locked))
    return;

  /* Otherwise, block the other faders */
  locked = TRUE;

  /* Get current volumes of the track */
  old_volumes = g_new (gint, track->gst_track->num_channels);
  gst_mixer_get_volume (GST_MIXER (track->card), track->gst_track, old_volumes);

  /* Determine maximum value of all channels */
  old_max_volume = xfce_mixer_get_max_volume (old_volumes, track->gst_track->num_channels);

  /* Allocate array for the volumes to be sent to GStreamer */
  new_volumes = g_new (gint, track->gst_track->num_channels);

  /* Collect volumes of all channels */
  for (iter = track->channel_faders, channel = 0; iter != NULL; iter = g_list_next (iter), ++channel)
    {
      /* If the channels are bound together, apply the volume to all the others */
      if (G_LIKELY (track->gst_track->num_channels >= 2 && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (track->lock_button))))
          gtk_range_set_value (GTK_RANGE (iter->data), gtk_range_get_value (range));

      new_volumes[channel] = (gint) gtk_range_get_value (GTK_RANGE (iter->data));
    }

  /* Deliver the volume update to GStreamer */
  gst_mixer_set_volume (GST_MIXER (track->card), track->gst_track, new_volumes);

  /* Determine new maximum value of all channels */
  new_max_volume = xfce_mixer_get_max_volume (new_volumes, track->gst_track->num_channels);

  /* Mute when volume reaches the minimum, unmute if volume is raised from the minimum */
  if (old_max_volume > track->gst_track->min_volume && new_max_volume == track->gst_track->min_volume)
    {
      if (xfce_mixer_track_type_new (track->gst_track) != XFCE_MIXER_TRACK_TYPE_CAPTURE)
        {
          gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, TRUE);
          xfce_mixer_track_update_mute (track);
        }
      else
        {
          gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, TRUE);
          xfce_mixer_track_update_record (track);
        }
    }
  else if (old_max_volume == track->gst_track->min_volume && new_max_volume > track->gst_track->min_volume)
    {
      if (xfce_mixer_track_type_new (track->gst_track) != XFCE_MIXER_TRACK_TYPE_CAPTURE)
        {
          gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, FALSE);
          xfce_mixer_track_update_mute (track);
        }
      else
        {
          gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, FALSE);
          xfce_mixer_track_update_record (track);
        }
    }

  /* Free volume arrays */
  g_free (old_volumes);
  g_free (new_volumes);

  /* We're done, unlock this function */
  locked = FALSE;
}



static void 
xfce_mixer_track_mute_toggled (GtkToggleButton *button,
                               XfceMixerTrack  *track)
{
  GtkWidget   *image;
  const gchar *stock;

  if (gtk_toggle_button_get_active (button))
    {
      stock = "audio-volume-muted";
      gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, TRUE);
    }
  else
    {
      stock = "audio-volume-high";
      gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, FALSE);
    }

  image = gtk_image_new_from_icon_name (stock, XFCE_MIXER_ICON_SIZE);
  gtk_button_set_image (GTK_BUTTON (button), image);
}



static void 
xfce_mixer_track_lock_toggled (GtkToggleButton *button,
                               XfceMixerTrack  *track)
{
  GtkWidget      *image;
  GtkWidget      *first_fader = NULL;
  GList          *iter;
  gint           *volumes;
  gint            channel;

  /* Do nothing if the channels were unlocked */
  if (G_UNLIKELY (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (track->lock_button))))
    {
      image = gtk_image_new_from_file (DATADIR "/pixmaps/xfce4-mixer/chain-broken.png");
      gtk_button_set_image (GTK_BUTTON (track->lock_button), image);
      return;
    }

  image = gtk_image_new_from_file (DATADIR "/pixmaps/xfce4-mixer/chain.png");
  gtk_button_set_image (GTK_BUTTON (track->lock_button), image);

  /* Allocate array for volumes of all channels */
  volumes = g_new (gint, track->gst_track->num_channels);

  /* Collect channel volumes */
  for (iter = track->channel_faders, channel = 0; iter != NULL; iter = g_list_next (iter), ++channel)
    {
      /* Remember the first fader - its volume is going to be applied to all faders */
      if (G_UNLIKELY (first_fader == NULL))
        first_fader = iter->data;
      else
        {
          /* Apply the volume of the first fader to the others */
          gtk_range_set_value (GTK_RANGE (iter->data), gtk_range_get_value (GTK_RANGE (first_fader)));
        }

      volumes[channel] = (gint) gtk_range_get_value (GTK_RANGE (iter->data));
    }

  /* Deliver the volume update to GStreamer */
  gst_mixer_set_volume (GST_MIXER (track->card), track->gst_track, volumes);

  /* Free the volume array */
  g_free (volumes);
}



static void 
xfce_mixer_track_record_toggled (GtkToggleButton *button,
                                 XfceMixerTrack  *track)
{
  GtkWidget   *image;
  const gchar *stock;

  if (gtk_toggle_button_get_active (button))
    {
      stock = "audio-input-microphone";
      gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, TRUE);
    }
  else
    {
      stock = "audio-input-microphone-muted";
      gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, FALSE);
    }

  image = gtk_image_new_from_icon_name (stock, XFCE_MIXER_ICON_SIZE);
  gtk_button_set_image (GTK_BUTTON (button), image);
}



void 
xfce_mixer_track_update_mute (XfceMixerTrack *track)
{
  gboolean muted;

  g_return_if_fail (IS_XFCE_MIXER_TRACK (track));

  if (G_LIKELY (xfce_mixer_track_type_new (track->gst_track) != XFCE_MIXER_TRACK_TYPE_CAPTURE))
    {
      muted = GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_MUTE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->mute_button), muted);
    }
}



void 
xfce_mixer_track_update_record (XfceMixerTrack *track)
{
  gboolean record;

  g_return_if_fail (IS_XFCE_MIXER_TRACK (track));

  if (G_UNLIKELY (xfce_mixer_track_type_new (track->gst_track) == XFCE_MIXER_TRACK_TYPE_CAPTURE))
    {
      record = GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_RECORD);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->record_button), record);
    }
}



void
xfce_mixer_track_update_volume (XfceMixerTrack *track)
{
  GList *iter;
  gint  *volumes;
  gint   channel;

  g_return_if_fail (IS_XFCE_MIXER_TRACK (track));

  volumes = g_new0 (gint, track->gst_track->num_channels);
  gst_mixer_get_volume (GST_MIXER (track->card), track->gst_track, volumes);

  track->ignore_signals = TRUE;

  for (iter = track->channel_faders, channel = 0; iter != NULL; iter = g_list_next (iter), ++channel)
    gtk_range_set_value (GTK_RANGE (iter->data), volumes[channel]);

  track->ignore_signals = FALSE;

  g_free (volumes);
}

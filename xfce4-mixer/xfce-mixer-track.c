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

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-track.h"



#define XFCE_MIXER_ICON_SIZE GTK_ICON_SIZE_MENU



static void xfce_mixer_track_class_init      (XfceMixerTrackClass *klass);
static void xfce_mixer_track_init            (XfceMixerTrack      *track);
static void xfce_mixer_track_dispose         (GObject             *object);
static void xfce_mixer_track_finalize        (GObject             *object);
static void xfce_mixer_track_create_contents (XfceMixerTrack      *track);
static void xfce_mixer_track_fader_changed   (GtkRange            *range,
                                              XfceMixerTrack      *track);
static void xfce_mixer_track_mute_toggled    (GtkToggleButton     *button,
                                              XfceMixerTrack      *track);
static void xfce_mixer_track_lock_toggled    (GtkToggleButton     *button,
                                              XfceMixerTrack      *track);
static void xfce_mixer_track_record_toggled  (GtkToggleButton     *button,
                                              XfceMixerTrack      *track);



struct _XfceMixerTrackClass
{
  GtkTableClass __parent__;
};

struct _XfceMixerTrack
{
  GtkTable __parent__;

  GtkWidget     *mute_button;
  GtkWidget     *lock_button;
  GtkWidget     *record_button;

  GList         *channel_faders;

  GstElement    *card;
  GstMixerTrack *gst_track;

  gboolean       ignore_signals;
};



static GObjectClass *xfce_mixer_track_parent_class = NULL;



GType
xfce_mixer_track_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerTrackClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_track_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerTrack),
          0,
          (GInstanceInitFunc) xfce_mixer_track_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_TABLE, "XfceMixerTrack", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_track_class_init (XfceMixerTrackClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_track_parent_class = g_type_class_peek_parent (klass);

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
  XfceMixerTrack *track = XFCE_MIXER_TRACK (object);

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



static void
xfce_mixer_track_create_contents (XfceMixerTrack *track)
{
  GtkWidget   *label;
  GtkWidget   *image;
  GtkWidget   *button_box;
  GtkWidget   *fader;
  gdouble      step;
  gint         channel;
  gint         columns;
  gint        *volumes;
  
  /* Calculate the number of columns required for the GtkTable */
  columns = MAX (track->gst_track->num_channels, 1);

  /* Get volumes of all channels of the track */
  volumes = g_new (gint, track->gst_track->num_channels);
  gst_mixer_get_volume (GST_MIXER (track->card), track->gst_track, volumes);

  /* Put some space between channel faders */
  gtk_table_set_col_spacings (GTK_TABLE (track), 6);

  /* Put the name of the track on top of the other elements */
  label = gtk_label_new (track->gst_track->label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5f, 0.5f);
  gtk_table_attach (GTK_TABLE (track), label, 0, columns, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);
  gtk_widget_show (label);

  step = (gdouble) (track->gst_track->max_volume - track->gst_track->min_volume) / (gdouble) 20;

  /* Create a fader for each channel */
  for (channel = 0; channel < track->gst_track->num_channels; ++channel)
    {
      fader = gtk_vscale_new_with_range (track->gst_track->min_volume, track->gst_track->max_volume, step);
      gtk_scale_set_draw_value (GTK_SCALE (fader), FALSE);
      gtk_range_set_inverted (GTK_RANGE (fader), TRUE);
      gtk_range_set_value (GTK_RANGE (fader), volumes[channel]);
      g_signal_connect (fader, "value-changed", G_CALLBACK (xfce_mixer_track_fader_changed), track);
      gtk_table_attach (GTK_TABLE (track), fader, channel, channel + 1, 1, 2, GTK_SHRINK, GTK_FILL|GTK_EXPAND, 0, 0);
      gtk_widget_show (fader);

      track->channel_faders = g_list_append (track->channel_faders, fader);
    }

  /* Create a horizontal for the control buttons */
  button_box = gtk_hbox_new (TRUE, 6);
  gtk_table_attach (GTK_TABLE (track), button_box, 0, columns, 2, 3, GTK_FILL, GTK_SHRINK, 0, 0);
  gtk_widget_show (button_box);

  /* Create the control buttons. Depending on the type (playback/capture) these may differ */
  track->mute_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_stock (XFCE_MIXER_STOCK_NO_MUTED, XFCE_MIXER_ICON_SIZE); 
  gtk_button_set_image (GTK_BUTTON (track->mute_button), image);
  g_signal_connect (track->mute_button, "toggled", G_CALLBACK (xfce_mixer_track_mute_toggled), track);
  gtk_box_pack_start (GTK_BOX (button_box), track->mute_button, FALSE, FALSE, 0);
  gtk_widget_show (track->mute_button);

  if (G_LIKELY (track->gst_track->num_channels >= 2))
    {
      track->lock_button = gtk_toggle_button_new ();
      image = gtk_image_new_from_file (DATADIR "/pixmaps/xfce4-mixer/chain.png");
      gtk_button_set_image (GTK_BUTTON (track->lock_button), image);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->lock_button), TRUE);
      g_signal_connect (track->lock_button, "toggled", G_CALLBACK (xfce_mixer_track_lock_toggled), track);
      gtk_box_pack_start (GTK_BOX (button_box), track->lock_button, FALSE, FALSE, 0);
      gtk_widget_show (track->lock_button);
    }

  if (G_UNLIKELY (xfce_mixer_track_type_new (track->gst_track) == XFCE_MIXER_TRACK_TYPE_CAPTURE))
    {
      track->record_button = gtk_toggle_button_new ();
      image = gtk_image_new_from_stock (XFCE_MIXER_STOCK_NO_RECORD, XFCE_MIXER_ICON_SIZE);
      gtk_button_set_image (GTK_BUTTON (track->record_button), image);
      g_signal_connect (track->record_button, "toggled", G_CALLBACK (xfce_mixer_track_record_toggled), track);
      gtk_box_pack_start (GTK_BOX (button_box), track->record_button, FALSE, FALSE, 0);
      gtk_widget_show (track->record_button);
    }

  /* Some of the mixer controls need to be updated before they can be used */
  xfce_mixer_track_update_mute (track);
  if (G_UNLIKELY (xfce_mixer_track_type_new (track->gst_track) == XFCE_MIXER_TRACK_TYPE_CAPTURE))
    xfce_mixer_track_update_record (track);

  /* Free volume array */
  g_free (volumes);
}



static void 
xfce_mixer_track_fader_changed (GtkRange       *range,
                                XfceMixerTrack *track)
{
  GList          *iter;
  gint           *volumes;
  gint            channel;

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

  /* Allocate array for the volumes to be sent to GStreamer */
  volumes = g_new (gint, track->gst_track->num_channels);

  /* Collect volumes of all channels */
  for (iter = track->channel_faders, channel = 0; iter != NULL; iter = g_list_next (iter), ++channel)
    {
      /* If the channels are bound together, apply the volume to all the others */
      if (G_LIKELY (track->gst_track->num_channels >= 2 && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (track->lock_button))))
          gtk_range_set_value (GTK_RANGE (iter->data), gtk_range_get_value (range));

      volumes[channel] = (gint) gtk_range_get_value (GTK_RANGE (iter->data));
    }

  /* Deliver the volume update to GStreamer */
  gst_mixer_set_volume (GST_MIXER (track->card), track->gst_track, volumes);

  /* Free volume array */
  g_free (volumes);

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
      stock = XFCE_MIXER_STOCK_MUTED;
      gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, TRUE);
    }
  else
    {
      stock = XFCE_MIXER_STOCK_NO_MUTED;
      gst_mixer_set_mute (GST_MIXER (track->card), track->gst_track, FALSE);
    }

  image = gtk_image_new_from_stock (stock, XFCE_MIXER_ICON_SIZE);
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
      stock = XFCE_MIXER_STOCK_RECORD;
      gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, TRUE);
    }
  else
    {
      stock = XFCE_MIXER_STOCK_NO_RECORD;
      gst_mixer_set_record (GST_MIXER (track->card), track->gst_track, FALSE);
    }

  image = gtk_image_new_from_stock (stock, XFCE_MIXER_ICON_SIZE);
  gtk_button_set_image (GTK_BUTTON (button), image);
}



void 
xfce_mixer_track_update_mute (XfceMixerTrack *track)
{
  gboolean muted;

  g_return_if_fail (IS_XFCE_MIXER_TRACK (track));

  muted = GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_MUTE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->mute_button), muted);
}



void 
xfce_mixer_track_update_record (XfceMixerTrack *track)
{
  gboolean record;

  g_return_if_fail (IS_XFCE_MIXER_TRACK (track));

  record = GST_MIXER_TRACK_HAS_FLAG (track->gst_track, GST_MIXER_TRACK_RECORD);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track->record_button), record);
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

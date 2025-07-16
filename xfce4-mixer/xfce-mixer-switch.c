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

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-switch.h"



static void xfce_mixer_switch_dispose         (GObject              *object);
static void xfce_mixer_switch_finalize        (GObject              *object);
static void xfce_mixer_switch_create_contents (XfceMixerSwitch      *mixer_switch);
static void xfce_mixer_switch_toggled         (GtkToggleButton      *button);



struct _XfceMixerSwitch
{
  GtkCheckButton  __parent__;

  GstElement    *card;
  GstMixerTrack *track;

  gboolean       ignore_signals;
};



G_DEFINE_TYPE (XfceMixerSwitch, xfce_mixer_switch, GTK_TYPE_CHECK_BUTTON)



static void
xfce_mixer_switch_class_init (XfceMixerSwitchClass *klass)
{
  GObjectClass         *gobject_class;
  GtkToggleButtonClass *gtk_toggle_button_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_switch_dispose;
  gobject_class->finalize = xfce_mixer_switch_finalize;

  gtk_toggle_button_class = GTK_TOGGLE_BUTTON_CLASS (klass);
  gtk_toggle_button_class->toggled = xfce_mixer_switch_toggled;
}



static void
xfce_mixer_switch_init (XfceMixerSwitch *mixer_switch)
{
  mixer_switch->ignore_signals = FALSE;
}



static void
xfce_mixer_switch_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_switch_parent_class)->dispose) (object);
}



static void
xfce_mixer_switch_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_switch_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_switch_new (GstElement    *card,
                       GstMixerTrack *track)
{
  XfceMixerSwitch *mixer_switch;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), NULL);
  
  mixer_switch = g_object_new (XFCE_TYPE_MIXER_SWITCH, NULL);
  mixer_switch->card = card;
  mixer_switch->track = track;

  xfce_mixer_switch_create_contents (mixer_switch);

  return GTK_WIDGET (mixer_switch);
}



static void
xfce_mixer_switch_create_contents (XfceMixerSwitch *mixer_switch)
{
  const gchar *label;

  label = xfce_mixer_get_track_label (mixer_switch->track);
  gtk_button_set_label (GTK_BUTTON (mixer_switch), label);
  xfce_mixer_switch_update (mixer_switch);

  /* Make read-only switches insensitive */
  if ((GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_INPUT) &&
       (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_NO_RECORD) ||
        GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_READONLY))) ||
      (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_OUTPUT) &&
       (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_NO_MUTE) ||
        GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_READONLY))))
    gtk_widget_set_sensitive (GTK_WIDGET (mixer_switch), FALSE);
}



static void 
xfce_mixer_switch_toggled (GtkToggleButton *button)
{
  XfceMixerSwitch *mixer_switch = XFCE_MIXER_SWITCH (button);

  if (G_UNLIKELY (mixer_switch->ignore_signals))
    return;

  if (G_LIKELY (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_INPUT)))
    gst_mixer_set_record (GST_MIXER (mixer_switch->card), mixer_switch->track, gtk_toggle_button_get_active (button));
  else
    gst_mixer_set_mute (GST_MIXER (mixer_switch->card), mixer_switch->track, !gtk_toggle_button_get_active (button));
}



void 
xfce_mixer_switch_update (XfceMixerSwitch *mixer_switch)
{
  g_return_if_fail (XFCE_IS_MIXER_SWITCH (mixer_switch));

  mixer_switch->ignore_signals = TRUE;

  if (G_LIKELY (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_INPUT)))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mixer_switch),
                                  GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_RECORD));
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mixer_switch),
                                  !GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_MUTE));

  mixer_switch->ignore_signals = FALSE;
}

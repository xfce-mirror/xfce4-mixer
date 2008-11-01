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

#include "xfce-mixer-switch.h"



static void xfce_mixer_switch_class_init      (XfceMixerSwitchClass *klass);
static void xfce_mixer_switch_init            (XfceMixerSwitch      *mixer_switch);
static void xfce_mixer_switch_dispose         (GObject              *object);
static void xfce_mixer_switch_finalize        (GObject              *object);
static void xfce_mixer_switch_create_contents (XfceMixerSwitch      *mixer_switch);
static void xfce_mixer_switch_toggled         (GtkToggleButton      *button,
                                               XfceMixerSwitch      *mixer_switch);



struct _XfceMixerSwitchClass
{
  GtkHBoxClass __parent__;
};

struct _XfceMixerSwitch
{
  GtkHBox __parent__;

  GstElement    *card;
  GstMixerTrack *track;

  GtkWidget     *check_button;

  gboolean       ignore_signals;
};



static GObjectClass *xfce_mixer_switch_parent_class = NULL;



GType
xfce_mixer_switch_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info = 
        {
          sizeof (XfceMixerSwitchClass),
          NULL,
          NULL,
          (GClassInitFunc) xfce_mixer_switch_class_init,
          NULL,
          NULL,
          sizeof (XfceMixerSwitch),
          0,
          (GInstanceInitFunc) xfce_mixer_switch_init,
          NULL,
        };

      type = g_type_register_static (GTK_TYPE_HBOX, "XfceMixerSwitch", &info, 0);
    }
  
  return type;
}



static void
xfce_mixer_switch_class_init (XfceMixerSwitchClass *klass)
{
  GObjectClass *gobject_class;

  /* Determine parent type class */
  xfce_mixer_switch_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_switch_dispose;
  gobject_class->finalize = xfce_mixer_switch_finalize;
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
  XfceMixerSwitch *mixer_switch = XFCE_MIXER_SWITCH (object);

  (*G_OBJECT_CLASS (xfce_mixer_switch_parent_class)->finalize) (object);
}



GtkWidget*
xfce_mixer_switch_new (GstElement    *card,
                       GstMixerTrack *track)
{
  XfceMixerSwitch *mixer_switch;

  g_return_val_if_fail (GST_IS_MIXER (card), NULL);
  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), NULL);
  
  mixer_switch = g_object_new (TYPE_XFCE_MIXER_SWITCH, NULL);
  mixer_switch->card = card;
  mixer_switch->track = track;

  xfce_mixer_switch_create_contents (mixer_switch);

  return GTK_WIDGET (mixer_switch);
}



static void
xfce_mixer_switch_create_contents (XfceMixerSwitch *mixer_switch)
{
  gtk_box_set_homogeneous (GTK_BOX (mixer_switch), FALSE);
  gtk_box_set_spacing (GTK_BOX (mixer_switch), 12);

  mixer_switch->check_button = gtk_check_button_new_with_mnemonic (mixer_switch->track->label);
  gtk_box_pack_start (GTK_BOX (mixer_switch), mixer_switch->check_button, FALSE, FALSE, 0);
  gtk_widget_show (mixer_switch->check_button);

  xfce_mixer_switch_update (mixer_switch);

  g_signal_connect (mixer_switch->check_button, "toggled", G_CALLBACK (xfce_mixer_switch_toggled), mixer_switch);
}



static void 
xfce_mixer_switch_toggled (GtkToggleButton *button,
                           XfceMixerSwitch *mixer_switch)
{
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
  g_return_if_fail (IS_XFCE_MIXER_SWITCH (mixer_switch));

  mixer_switch->ignore_signals = TRUE;

  if (G_LIKELY (GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_INPUT)))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mixer_switch->check_button), 
                                  GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_RECORD));
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mixer_switch->check_button), 
                                  !GST_MIXER_TRACK_HAS_FLAG (mixer_switch->track, GST_MIXER_TRACK_MUTE));

  mixer_switch->ignore_signals = FALSE;
}

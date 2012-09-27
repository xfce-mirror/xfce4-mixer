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

#ifndef __XFCE_MIXER_TRACK_COMBO_H__
#define __XFCE_MIXER_TRACK_COMBO_H__

#include <gtk/gtk.h>

#include <gst/gst.h>
#include <gst/interfaces/mixer.h>

G_BEGIN_DECLS;

typedef struct _XfceMixerTrackComboClass XfceMixerTrackComboClass;
typedef struct _XfceMixerTrackCombo      XfceMixerTrackCombo;

#define TYPE_XFCE_MIXER_TRACK_COMBO            (xfce_mixer_track_combo_get_type ())
#define XFCE_MIXER_TRACK_COMBO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_TRACK_COMBO, XfceMixerTrackCombo))
#define XFCE_MIXER_TRACK_COMBO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_TRACK_COMBO, XfceMixerTrackComboClass))
#define IS_XFCE_MIXER_TRACK_COMBO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_TRACK_COMBO))
#define IS_XFCE_MIXER_TRACK_COMBO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_TRACK_COMBO))
#define XFCE_MIXER_TRACK_COMBO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_TRACK_COMBO, XfceMixerTrackComboClass))

GType          xfce_mixer_track_combo_get_type         (void) G_GNUC_CONST;

GtkWidget     *xfce_mixer_track_combo_new              (GstElement          *card,
                                                        GstMixerTrack       *track);

void           xfce_mixer_track_combo_set_soundcard    (XfceMixerTrackCombo *combo,
                                                        GstElement          *card);
GstMixerTrack *xfce_mixer_track_combo_get_active_track (XfceMixerTrackCombo *combo);
void           xfce_mixer_track_combo_set_active_track (XfceMixerTrackCombo *combo,
                                                        GstMixerTrack       *track);

G_END_DECLS;

#endif /* !__XFCE_MIXER_TRACK_COMBO_H__ */

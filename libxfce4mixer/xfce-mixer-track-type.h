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

#ifndef __XFCE_TRACK_TYPE_H__
#define __XFCE_TRACK_TYPE_H__

#include <glib-object.h>
#include <gst/interfaces/mixer.h>

G_BEGIN_DECLS

typedef enum
{
  XFCE_MIXER_TRACK_TYPE_PLAYBACK,
  XFCE_MIXER_TRACK_TYPE_CAPTURE,
  XFCE_MIXER_TRACK_TYPE_SWITCH,
  XFCE_MIXER_TRACK_TYPE_OPTIONS
} XfceMixerTrackType;

#define TYPE_XFCE_MIXER_TRACK_TYPE  (xfce_mixer_track_type_get_type ())

GType              xfce_mixer_track_type_get_type (void) G_GNUC_CONST G_GNUC_INTERNAL;

XfceMixerTrackType xfce_mixer_track_type_new      (GstMixerTrack *track);

G_END_DECLS

#endif /* !__XFCE_TRACK_TYPE_H__ */

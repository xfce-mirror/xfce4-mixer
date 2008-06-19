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

#include <glib.h>
#include <glib-object.h>

#include "xfce-mixer-track-type.h"



GType
xfce_mixer_track_type_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] = 
        {
          { XFCE_MIXER_TRACK_TYPE_PLAYBACK, "XFCE_MIXER_TRACK_TYPE_PLAYBACK", "playback" },
          { XFCE_MIXER_TRACK_TYPE_CAPTURE, "XFCE_MIXER_TRACK_TYPE_CAPTURE", "capture" },
          { XFCE_MIXER_TRACK_TYPE_SWITCH, "XFCE_MIXER_TRACK_TYPE_SWITCH", "switch" },
          { XFCE_MIXER_TRACK_TYPE_OPTIONS, "XFCE_MIXER_TRACK_TYPE_OPTIONS", "options" },
          { 0, NULL, NULL }
        };

      type = g_enum_register_static (g_intern_static_string ("XfceMixerTrackType"), values);
    }

  return type;
}



XfceMixerTrackType
xfce_mixer_track_type_new (GstMixerTrack *track)
{
  XfceMixerTrackType type = XFCE_MIXER_TRACK_TYPE_CAPTURE;
  
  if (G_UNLIKELY (GST_IS_MIXER_OPTIONS (track)))
    type = XFCE_MIXER_TRACK_TYPE_OPTIONS;
  else
    {
      if (G_UNLIKELY (track->num_channels == 0))
        type = XFCE_MIXER_TRACK_TYPE_SWITCH;
      else
        {
          if (GST_MIXER_TRACK_HAS_FLAG (track, GST_MIXER_TRACK_INPUT))
            type = XFCE_MIXER_TRACK_TYPE_CAPTURE;
          else
            type = XFCE_MIXER_TRACK_TYPE_PLAYBACK;
        }
    }

  return type;
}

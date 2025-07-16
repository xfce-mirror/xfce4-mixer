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

#include <glib.h>
#include <glib-object.h>

#include "xfce-mixer-track-type.h"
#include "gst-mixer-options.h"



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

  g_return_val_if_fail (GST_IS_MIXER_TRACK (track), G_TYPE_INVALID);

  if (G_UNLIKELY (GST_IS_MIXER_OPTIONS (track)))
    type = XFCE_MIXER_TRACK_TYPE_OPTIONS;
  else
    {
      if (G_UNLIKELY (gst_mixer_track_get_num_channels (track)) == 0 )
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

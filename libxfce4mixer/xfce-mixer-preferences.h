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

#ifndef __XFCE_MIXER_PREFERENCES_H__
#define __XFCE_MIXER_PREFERENCES_H__

#include <glib-object.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define XFCE_TYPE_MIXER_PREFERENCES (xfce_mixer_preferences_get_type ())
G_DECLARE_FINAL_TYPE (XfceMixerPreferences, xfce_mixer_preferences, XFCE, MIXER_PREFERENCES, GObject)

XfceMixerPreferences *xfce_mixer_preferences_get                  (void);
void                  xfce_mixer_preferences_set_controls         (XfceMixerPreferences *preferences,
                                                                   GPtrArray            *controls);
GPtrArray            *xfce_mixer_preferences_get_controls         (XfceMixerPreferences *preferences);
gboolean              xfce_mixer_preferences_get_control_visible  (XfceMixerPreferences *preferences,
                                                                   const gchar          *track_label);

G_END_DECLS

#endif /* !__XFCE_MIXER_PREFERENCES_H__ */

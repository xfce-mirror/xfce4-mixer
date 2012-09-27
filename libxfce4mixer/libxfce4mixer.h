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

#ifndef __LIBXFCE4MIXER_H__
#define __LIBXFCE4MIXER_H__

#include <glib.h>

#include <gst/interfaces/mixer.h>

#include "xfce-mixer-preferences.h"
#include "xfce-mixer-card-combo.h"
#include "xfce-mixer-track-combo.h"
#include "xfce-mixer-track-type.h"

G_BEGIN_DECLS;

void           xfce_mixer_init                   (void);
void           xfce_mixer_shutdown               (void);

GList         *xfce_mixer_get_cards              (void);
GstElement    *xfce_mixer_get_card               (const gchar   *name);
GstElement    *xfce_mixer_get_default_card       (void);
const gchar   *xfce_mixer_get_card_internal_name (GstElement    *card);
const gchar   *xfce_mixer_get_card_display_name  (GstElement    *card);
void           xfce_mixer_select_card            (GstElement    *card);
GstMixerTrack *xfce_mixer_get_track              (GstElement    *card,
                                                  const gchar   *track_name);
GstMixerTrack *xfce_mixer_get_default_track      (GstElement    *card);

#ifdef HAVE_GST_MIXER_NOTIFICATION
guint          xfce_mixer_bus_connect            (GCallback      callback,
                                                  gpointer       user_data);
void           xfce_mixer_bus_disconnect         (guint          signal_handler_id);
#endif

gint           xfce_mixer_get_max_volume         (gint          *volumes,
                                                  gint           num_channels);
int            xfce_mixer_utf8_cmp               (const gchar   *s1,
                                                  const gchar   *s2);

G_END_DECLS;

#endif /* !__LIBXFCE4MIXER_H__ */

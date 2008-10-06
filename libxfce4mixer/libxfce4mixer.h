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

#ifndef __LIBXFCE4MIXER_H__
#define __LIBXFCE4MIXER_H__

#include <glib.h>

#include <gst/interfaces/mixer.h>

G_BEGIN_DECLS;

void           xfce_mixer_init                   (void);
void           xfce_mixer_shutdown               (void);

GList         *xfce_mixer_get_cards              (void);
GstElement    *xfce_mixer_get_card               (const gchar   *name);
const gchar   *xfce_mixer_get_card_internal_name (GstElement    *card);
const gchar   *xfce_mixer_get_card_display_name  (GstElement    *card);
void           xfce_mixer_select_card            (GstElement    *card);
GstMixerTrack *xfce_mixer_get_track              (GstElement    *card,
                                                  const gchar   *track_name);

#ifdef HAVE_GST_MIXER_NOTIFICATION
guint          xfce_mixer_bus_connect            (GCallback      callback,
                                                  gpointer       user_data);
void           xfce_mixer_bus_disconnect         (guint          signal_handler_id);
#endif

gint           xfce_mixer_get_max_volume         (gint          *volumes,
                                                  gint           num_channels);

G_END_DECLS;

#endif /* !__LIBXFCE4MIXER_H__ */

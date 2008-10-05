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

#ifndef __XFCE_MIXER_CARD_H__
#define __XFCE_MIXER_CARD_H__

#include <gtk/gtk.h>
#include <gst/interfaces/mixer.h>

G_BEGIN_DECLS;

typedef struct _XfceMixerCardClass XfceMixerCardClass;
typedef struct _XfceMixerCard      XfceMixerCard;

#define TYPE_XFCE_MIXER_CARD            (xfce_mixer_card_get_type ())
#define XFCE_MIXER_CARD(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_CARD, XfceMixerCard))
#define XFCE_MIXER_CARD_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_CARD, XfceMixerCardClass))
#define IS_XFCE_MIXER_CARD(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_CARD))
#define IS_XFCE_MIXER_CARD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_CARD))
#define XFCE_MIXER_CARD_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_CARD, XfceMixerCardClass))

GType          xfce_mixer_card_get_type             (void) G_GNUC_CONST;

XfceMixerCard *xfce_mixer_card_new                  (GstElement    *element);
const gchar   *xfce_mixer_card_get_display_name     (XfceMixerCard *card);
const gchar   *xfce_mixer_card_get_name             (XfceMixerCard *card);
void           xfce_mixer_card_set_ready            (XfceMixerCard *card);
gchar * const *xfce_mixer_card_get_visible_controls (XfceMixerCard *card);
const GList   *xfce_mixer_card_get_tracks           (XfceMixerCard *card);
GstMixerTrack *xfce_mixer_card_get_track_by_name    (XfceMixerCard *card,
                                                     const gchar   *track_name);
void           xfce_mixer_card_set_visible_controls (XfceMixerCard *card,
                                                     gchar * const *controls);
void           xfce_mixer_card_get_track_volume     (XfceMixerCard *card,
                                                     GstMixerTrack *track,
                                                     gint          *volumes);
void          xfce_mixer_card_set_track_volume      (XfceMixerCard *card,
                                                     GstMixerTrack *track,
                                                     gint          *volumes);
void           xfce_mixer_card_set_track_muted      (XfceMixerCard *card,
                                                     GstMixerTrack *track,
                                                     gboolean       muted);
void           xfce_mixer_card_set_track_record     (XfceMixerCard *card,
                                                     GstMixerTrack *track,
                                                     gboolean       record);
const gchar   *xfce_mixer_card_get_track_option     (XfceMixerCard *card,
                                                     GstMixerTrack *track);
void           xfce_mixer_card_set_track_option     (XfceMixerCard *card,
                                                     GstMixerTrack *track,
                                                     gchar         *option);
gboolean       xfce_mixer_card_control_is_visible   (XfceMixerCard *card,
                                                     const gchar   *control);


G_END_DECLS;

#endif /* !__XFCE_MIXER_CARD_H__ */

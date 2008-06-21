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

#ifndef __XFCE_MIXER_STOCK_H__
#define __XFCE_MIXER_STOCK_H__

G_BEGIN_DECLS;

#define XFCE_MIXER_STOCK_RECORD    "xfce-mixer-record"
#define XFCE_MIXER_STOCK_NO_RECORD "xfce-mixer-no-record"
#define XFCE_MIXER_STOCK_MUTED     "xfce-mixer-muted"
#define XFCE_MIXER_STOCK_NO_MUTED  "xfce-mixer-no-muted"
#define AUDIO_VOLUME_00            "audio-volume-00"
#define AUDIO_VOLUME_01            "audio-volume-01"
#define AUDIO_VOLUME_02            "audio-volume-02"
#define AUDIO_VOLUME_03            "audio-volume-03"
#define AUDIO_VOLUME_04            "audio-volume-04"
#define AUDIO_VOLUME_05            "audio-volume-05"

void xfce_mixer_stock_init (void) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__XFCE_MIXER_STOCK_H__ */

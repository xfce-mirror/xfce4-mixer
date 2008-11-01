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
#define AUDIO_VOLUME_06            "audio-volume-06"

void xfce_mixer_stock_init (void) G_GNUC_INTERNAL;

G_END_DECLS;

#endif /* !__XFCE_MIXER_STOCK_H__ */

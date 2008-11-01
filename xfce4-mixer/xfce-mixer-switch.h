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

#ifndef __XFCE_MIXER_SWITCH_H__
#define __XFCE_MIXER_SWITCH_H__

#include <gtk/gtk.h>
#include <gst/gst.h>

G_BEGIN_DECLS;

typedef struct _XfceMixerSwitchClass XfceMixerSwitchClass;
typedef struct _XfceMixerSwitch      XfceMixerSwitch;

#define TYPE_XFCE_MIXER_SWITCH            (xfce_mixer_switch_get_type ())
#define XFCE_MIXER_SWITCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_SWITCH, XfceMixerSwitch))
#define XFCE_MIXER_SWITCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_SWITCH, XfceMixerSwitchClass))
#define IS_XFCE_MIXER_SWITCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_SWITCH))
#define IS_XFCE_MIXER_SWITCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_SWITCH))
#define XFCE_MIXER_SWITCH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_SWITCH, XfceMixerSwitchClass))

GType      xfce_mixer_switch_get_type (void) G_GNUC_CONST;

GtkWidget *xfce_mixer_switch_new      (GstElement      *card,
                                       GstMixerTrack   *track);
void       xfce_mixer_switch_update   (XfceMixerSwitch *mixer_switch);

G_END_DECLS;

#endif /* !__XFCE_MIXER_SWITCH_H__ */

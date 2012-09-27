/* vi:set expandtab sw=2 sts=2: */
/*-
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

#ifndef __XFCE_MIXER_PLUGIN_H__
#define __XFCE_MIXER_PLUGIN_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XfceMixerPluginClass XfceMixerPluginClass;
typedef struct _XfceMixerPlugin      XfceMixerPlugin;

#define TYPE_XFCE_MIXER_PLUGIN            (xfce_mixer_plugin_get_type ())
#define XFCE_MIXER_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_PLUGIN, XfceMixerPlugin))
#define XFCE_MIXER_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_PLUGIN, XfceMixerPluginClass))
#define IS_XFCE_MIXER_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_PLUGIN))
#define IS_XFCE_MIXER_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_PLUGIN))
#define XFCE_MIXER_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_PLUGIN, XfceMixerPluginClass))

GType     xfce_mixer_plugin_get_type       (void) G_GNUC_CONST;

void  xfce_mixer_plugin_register_type (XfcePanelTypeModule *type_module);

G_END_DECLS

#endif /* !__XFCE_MIXER_PLUGIN_H__ */

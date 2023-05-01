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

#include <libxfce4panel/libxfce4panel.h>

G_BEGIN_DECLS

#define XFCE_TYPE_MIXER_PLUGIN (xfce_mixer_plugin_get_type ())
#if !LIBXFCE4PANEL_CHECK_VERSION (4, 19, 1)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XfcePanelPlugin, g_object_unref)
#endif
G_DECLARE_FINAL_TYPE (XfceMixerPlugin, xfce_mixer_plugin, XFCE, MIXER_PLUGIN, XfcePanelPlugin)

void  xfce_mixer_plugin_register_type (XfcePanelTypeModule *type_module);

G_END_DECLS

#endif /* !__XFCE_MIXER_PLUGIN_H__ */

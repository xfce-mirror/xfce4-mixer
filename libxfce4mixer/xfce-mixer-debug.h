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

#ifndef __XFCE_MIXER_DEBUG_H__
#define __XFCE_MIXER_DEBUG_H__

#include <glib.h>
#include <stdarg.h>

G_BEGIN_DECLS

#define xfce_mixer_debug(...) xfce_mixer_debug_real (G_LOG_DOMAIN, __FILE__, __func__, __LINE__, __VA_ARGS__)

void xfce_mixer_debug_init        (const gchar    *log_domain,
                                   gboolean        debug_mode);
void xfce_mixer_debug_real        (const gchar    *log_domain,
                                   const gchar    *file,
                                   const gchar    *func,
                                   gint            line,
                                   const gchar    *format, ...);
void xfce_mixer_dump_gst_data     (void);



G_END_DECLS

#endif /* !__XFCE_MIXER_DEBUG_H__ */


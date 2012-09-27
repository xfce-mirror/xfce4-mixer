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

#ifndef __XFCE_VOLUME_BUTTON_H__
#define __XFCE_VOLUME_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _XfceVolumeButtonClass XfceVolumeButtonClass;
typedef struct _XfceVolumeButton      XfceVolumeButton;

#define TYPE_XFCE_VOLUME_BUTTON            (xfce_volume_button_get_type ())
#define XFCE_VOLUME_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_VOLUME_BUTTON, XfceVolumeButton))
#define XFCE_VOLUME_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_VOLUME_BUTTON, XfceVolumeButtonClass))
#define IS_XFCE_VOLUME_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_VOLUME_BUTTON))
#define IS_XFCE_VOLUME_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_VOLUME_BUTTON))
#define XFCE_VOLUME_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_VOLUME_BUTTON, XfceVolumeButtonClass))

GType     xfce_volume_button_get_type           (void) G_GNUC_CONST;

GtkWidget *xfce_volume_button_new               (void);

void       xfce_volume_button_set_muted         (XfceVolumeButton *button,
                                                 gboolean          is_muted);
void       xfce_volume_button_set_volume        (XfceVolumeButton *button,
                                                 gdouble           volume);
gboolean   xfce_volume_button_get_muted         (XfceVolumeButton *button);
void       xfce_volume_button_update            (XfceVolumeButton *button);
void       xfce_volume_button_set_icon_size     (XfceVolumeButton *button,
                                                 gint              size);
void       xfce_volume_button_set_track_label   (XfceVolumeButton *button,
                                                 const gchar      *track_label);
gchar     *xfce_volume_button_get_track_label   (XfceVolumeButton *button);
void       xfce_volume_button_set_is_configured (XfceVolumeButton *button,
                                                 gboolean          is_configured);
gboolean   xfce_volume_button_get_is_configured (XfceVolumeButton *button);

G_END_DECLS;

#endif /* !__XFCE_VOLUME_BUTTON_H__ */

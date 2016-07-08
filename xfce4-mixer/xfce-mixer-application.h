/* vi:set expandtab sw=2 sts=2: */
/*-
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

#ifndef __XFCE_MIXER_APPLICATION_H__
#define __XFCE_MIXER_APPLICATION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XfceMixerApplicationClass XfceMixerApplicationClass;
typedef struct _XfceMixerApplication      XfceMixerApplication;

#define TYPE_XFCE_MIXER_APPLICATION            (xfce_mixer_application_get_type ())
#define XFCE_MIXER_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_APPLICATION, XfceMixerApplication))
#define XFCE_MIXER_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_APPLICATION, XfceMixerApplicationClass))
#define IS_XFCE_MIXER_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_APPLICATION))
#define IS_XFCE_MIXER_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_APPLICATION))
#define XFCE_MIXER_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_APPLICATION, XfceMixerApplicationClass))

GType         xfce_mixer_application_get_type        (void) G_GNUC_CONST;

GApplication *xfce_mixer_application_new             (void);

G_END_DECLS

#endif /* !__XFCE_MIXER_APPLICATION_H__ */

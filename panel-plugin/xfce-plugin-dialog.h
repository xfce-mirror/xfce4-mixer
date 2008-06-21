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

#ifndef __XFCE_PLUGIN_DIALOG_H__
#define __XFCE_PLUGIN_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _XfcePluginDialogClass XfcePluginDialogClass;
typedef struct _XfcePluginDialog      XfcePluginDialog;

#define TYPE_XFCE_PLUGIN_DIALOG            (xfce_plugin_dialog_get_type ())
#define XFCE_PLUGIN_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_PLUGIN_DIALOG, XfcePluginDialog))
#define XFCE_PLUGIN_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_PLUGIN_DIALOG, XfcePluginDialogClass))
#define IS_XFCE_PLUGIN_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_PLUGIN_DIALOG))
#define IS_XFCE_PLUGIN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_PLUGIN_DIALOG))
#define XFCE_PLUGIN_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_PLUGIN_DIALOG, XfcePluginDialogClass))

GType     xfce_plugin_dialog_get_type (void) G_GNUC_CONST;

GtkWidget *xfce_plugin_dialog_new     (const gchar *active_card,
                                       const gchar *active_track);

void       xce_plugin_dialog_get_data (XfcePluginDialog *dialog,
                                       XfceMixerCard   **card,
                                       GstMixerTrack   **track);

G_END_DECLS;

#endif /* !__XFCE_PLUGIN_DIALOG_H__ */

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

#ifndef __XFCE_MIXER_CONTROLS_DIALOG_H__
#define __XFCE_MIXER_CONTROLS_DIALOG_H__

/* includes libxfce4ui and the definition of glib_autoptr_clear_XfceTitledDialog */
#include "xfce-mixer-window.h"

G_BEGIN_DECLS

#define XFCE_TYPE_MIXER_CONTROLS_DIALOG (xfce_mixer_controls_dialog_get_type ())
G_DECLARE_FINAL_TYPE (XfceMixerControlsDialog, xfce_mixer_controls_dialog, XFCE, MIXER_CONTROLS_DIALOG, XfceTitledDialog)

GtkWidget *xfce_mixer_controls_dialog_new           (XfceMixerWindow         *parent);
void       xfce_mixer_controls_dialog_set_soundcard (XfceMixerControlsDialog *dialog,
                                                     GstElement              *card);
void       xfce_mixer_controls_dialog_update_dialog (XfceMixerControlsDialog *dialog);

G_END_DECLS

#endif /* !__XFCE_MIXER_CONTROLS_DIALOG_H__ */

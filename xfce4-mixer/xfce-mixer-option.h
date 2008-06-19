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

#ifndef __XFCE_MIXER_OPTION_H__
#define __XFCE_MIXER_OPTION_H__

#include <gtk/gtk.h>
#include <gst/interfaces/mixer.h>

#include "libxfce4mixer/xfce-mixer-card.h"

G_BEGIN_DECLS;

typedef struct _XfceMixerOptionClass XfceMixerOptionClass;
typedef struct _XfceMixerOption      XfceMixerOption;

#define TYPE_XFCE_MIXER_OPTION            (xfce_mixer_option_get_type ())
#define XFCE_MIXER_OPTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_XFCE_MIXER_OPTION, XfceMixerOption))
#define XFCE_MIXER_OPTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_XFCE_MIXER_OPTION, XfceMixerOptionClass))
#define IS_XFCE_MIXER_OPTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_XFCE_MIXER_OPTION))
#define IS_XFCE_MIXER_OPTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_XFCE_MIXER_OPTION))
#define XFCE_MIXER_OPTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_XFCE_MIXER_OPTION, XfceMixerOptionClass))

GType      xfce_mixer_option_get_type (void) G_GNUC_CONST;

GtkWidget *xfce_mixer_option_new      (XfceMixerCard *card,
                                       GstMixerTrack *track);

G_END_DECLS;

#endif /* !__XFCE_MIXER_OPTION_H__ */

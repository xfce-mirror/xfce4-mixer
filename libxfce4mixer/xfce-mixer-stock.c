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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "xfce-mixer-stock.h"



typedef struct
{
  const gchar *name;
  const gchar *icon;
} XfceMixerStockIcon;



static const XfceMixerStockIcon xfce_mixer_stock_icons[] =
{
  { XFCE_MIXER_STOCK_RECORD,    "stock_xfce4-mixer-record",    },
  { XFCE_MIXER_STOCK_NO_RECORD, "stock_xfce4-mixer-no-record", },
  { XFCE_MIXER_STOCK_MUTED,     "stock_xfce4-mixer-muted",     },
  { XFCE_MIXER_STOCK_NO_MUTED,  "stock_xfce4-mixer-no-muted",  },
  { AUDIO_VOLUME_00,            "audio-volume-00", },
  { AUDIO_VOLUME_01,            "audio-volume-01", },
  { AUDIO_VOLUME_02,            "audio-volume-02", },
  { AUDIO_VOLUME_03,            "audio-volume-03", },
  { AUDIO_VOLUME_04,            "audio-volume-04", },
  { AUDIO_VOLUME_05,            "audio-volume-05", },
  { AUDIO_VOLUME_06,            "audio-volume-06", },
};



void
xfce_mixer_stock_init (void)
{
  GtkIconFactory *icon_factory;
  GtkIconSource  *icon_source;
  GtkIconSet     *icon_set;
  guint           n;

  /* Allocate a new icon factory for the mixer stock icons */
  icon_factory = gtk_icon_factory_new ();

  /* Allocate an icon source */
  icon_source = gtk_icon_source_new ();

  /* Register the stock icons */
  for (n = 0; n < G_N_ELEMENTS (xfce_mixer_stock_icons); ++n)
    {
      /* Setup the icon set */
      icon_set = gtk_icon_set_new ();
      gtk_icon_source_set_icon_name (icon_source, xfce_mixer_stock_icons[n].icon);
      gtk_icon_set_add_source (icon_set, icon_source);
      gtk_icon_factory_add (icon_factory, xfce_mixer_stock_icons[n].name, icon_set);
      gtk_icon_set_unref (icon_set);
    }

  /* Register our icon factory as default */
  gtk_icon_factory_add_default (icon_factory);

  /* Clean up */
  g_object_unref (G_OBJECT (icon_factory));
  gtk_icon_source_free (icon_source);
}

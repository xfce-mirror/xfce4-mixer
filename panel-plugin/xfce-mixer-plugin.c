/* $Id$ */
/* vim:set et ai sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "xfce-volume-button.h"



typedef struct _XfceMixerPlugin XfceMixerPlugin;



/* Plugin structure */
struct _XfceMixerPlugin
{
  XfcePanelPlugin *plugin;

  GtkWidget       *hvbox;
  GtkWidget       *button;
};



/* Function prototypes */
static void             xfce_mixer_plugin_construct      (XfcePanelPlugin  *plugin);
static XfceMixerPlugin *xfce_mixer_plugin_new            (XfcePanelPlugin  *plugin);
static void             xfce_mixer_plugin_free           (XfcePanelPlugin  *plugin,
                                                          XfceMixerPlugin  *mixer_plugin);
static gboolean         xfce_mixer_plugin_size_changed   (XfcePanelPlugin  *plugin,
                                                          gint              size,
                                                          XfceMixerPlugin  *mixer_plugin);
static void             xfce_mixer_plugin_volume_changed (XfceVolumeButton *button,
                                                          gdouble           volume,
                                                          XfceMixerPlugin  *mixer_plugin);



/* Register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (xfce_mixer_plugin_construct);



static XfceMixerPlugin*
xfce_mixer_plugin_new (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;
  GtkWidget       *button;

  /* Allocate memory for the plugin structure */
  mixer_plugin = panel_slice_new0 (XfceMixerPlugin);

  /* Store pointer to the panel plugin */
  mixer_plugin->plugin = plugin;

  mixer_plugin->hvbox = xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (plugin), mixer_plugin->hvbox);
  gtk_widget_show (mixer_plugin->hvbox);

  mixer_plugin->button = xfce_volume_button_new ();
  g_signal_connect (G_OBJECT (mixer_plugin->button), "volume-changed", G_CALLBACK (xfce_mixer_plugin_volume_changed), mixer_plugin);
  gtk_container_add (GTK_CONTAINER (mixer_plugin->hvbox), mixer_plugin->button);
  gtk_widget_show (mixer_plugin->button);

  /* Connect to plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data", G_CALLBACK (xfce_mixer_plugin_free), mixer_plugin);
  g_signal_connect (G_OBJECT (plugin), "size-changed", G_CALLBACK (xfce_mixer_plugin_size_changed), mixer_plugin);
//  g_signal_connect (G_OBJECT (plugin), "configure", G_CALLBACK (xfce_mixer_plugin_configure), mixer_plugin);

  return mixer_plugin;
}



static void
xfce_mixer_plugin_free (XfcePanelPlugin *plugin,
                        XfceMixerPlugin *mixer_plugin)
{
  panel_slice_free (XfceMixerPlugin, mixer_plugin);
}



static void
xfce_mixer_plugin_construct (XfcePanelPlugin *plugin)
{
  XfceMixerPlugin *mixer_plugin;

  /* Set up translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* Create the plugin */
  mixer_plugin = xfce_mixer_plugin_new (plugin);
}



static gboolean
xfce_mixer_plugin_size_changed (XfcePanelPlugin *plugin,
                                gint             size,
                                XfceMixerPlugin *mixer_plugin)
{
  GtkOrientation orientation;

  g_return_val_if_fail (plugin != NULL, FALSE);
  g_return_val_if_fail (mixer_plugin != NULL, FALSE);

  /* Determine the icon size for the volume button */
  size -= 4 * MAX (mixer_plugin->button->style->xthickness, mixer_plugin->button->style->ythickness);

  /* Update the volume button icon size */
  xfce_volume_button_set_icon_size (XFCE_VOLUME_BUTTON (mixer_plugin->button), size);

  /* Get the orientation of the panel */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* TODO: Handle it */

  return TRUE;
}



static void
xfce_mixer_plugin_volume_changed (XfceVolumeButton *button,
                                  gdouble           volume,
                                  XfceMixerPlugin  *mixer_plugin)
{
  g_message ("volume changed to %f", volume);
}

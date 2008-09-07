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

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <libxfce4util/libxfce4util.h>

#include <gst/gst.h>

#include "libxfce4mixer/xfce-mixer-stock.h"
#include "xfce-mixer-window.h"



static void transform_string_to_int (const GValue *src,
                                     GValue       *dest)
{
  g_value_set_int (dest, (gint) strtol (g_value_get_string (src), NULL, 10));
}



static void transform_string_to_boolean (const GValue *src,
                                         GValue       *dest)
{
  g_value_set_boolean (dest, strcmp (g_value_get_string (src), "FALSE") != 0);
}



int 
main (int    argc,
      char **argv)
{
  GtkWidget *window;

  /* Setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* Initialize the threading system */
  if (G_LIKELY (!g_thread_supported ()))
    g_thread_init (NULL);

  /* Set debug level */
#ifdef G_ENABLE_DEBUG
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  /* Set application name */
  g_set_application_name (_("Xfce Mixer"));

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Register special transform functions for GValue types */
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_INT, transform_string_to_int);
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_BOOLEAN, transform_string_to_boolean);

  /* Initialize our own stock icon set */
  xfce_mixer_stock_init ();

  /* Use volume control icon for all mixer windows */
  gtk_window_set_default_icon_name ("xfce4-mixer");

  /* Warn users if there were no sound cards detected by GStreamer */
  if (G_UNLIKELY (xfce_mixer_utilities_get_n_cards () <= 0))
    {
      xfce_err (_("GStreamer was unable to detect any sound cards on your system. "
                  "You might be missing sound system specific GStreamer packages. "
                  "It might as well be a permission problem."));

      return EXIT_FAILURE;
    }

  /* Create the mixer window */
  window = xfce_mixer_window_new ();

  /* Display the mixer window */
  gtk_widget_show (window);

  /* Enter the GTK+ main loop */
  gtk_main ();

  /* Destroy the window */
  gtk_widget_destroy (window);

  return EXIT_SUCCESS;
}

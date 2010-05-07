/* vi:set expandtab sw=2 sts=2: */
/*-
 * Copyright (c) 2008 Jannis Pohlmann <jannis@xfce.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gst/gst.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-window.h"



int 
main (int    argc,
      char **argv)
{
  GtkWidget *window;
  GError    *error = NULL;

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
  g_set_application_name (_("Mixer"));

  /* Initialize GTK+ */
  gtk_init (&argc, &argv);

  /* Initialize Xfconf */
  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_print (_("Failed to initialize xfconf: %s"), error->message);
          g_error_free (error);
        }

      return EXIT_FAILURE;
    }

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Initialize the mixer library */
  xfce_mixer_init ();

  /* Use volume control icon for all mixer windows */
  gtk_window_set_default_icon_name ("multimedia-volume-control");

  /* Warn users if there were no sound cards detected by GStreamer */
  if (G_UNLIKELY (g_list_length (xfce_mixer_get_cards ()) <= 0))
    {
      xfce_dialog_show_error (NULL,
                              NULL,
                              _("GStreamer was unable to detect any sound devices. "
                              "Some sound system specific GStreamer packages may "
                              "be missing. It may also be a permissions problem."));

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

  /* Shutdown the mixer library */
  xfce_mixer_shutdown ();

  /* Shutdown Xfconf */
  xfconf_shutdown ();

  return EXIT_SUCCESS;
}

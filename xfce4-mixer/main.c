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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <gst/gst.h>

#include <gtk/gtk.h>
#include <unique/unique.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-window.h"



static UniqueResponse
message_received (UniqueApp         *app,
                  UniqueCommand      command,
                  UniqueMessageData *message,
                  guint              time_,
                  GtkWidget         *window)
{
  UniqueResponse response;

  switch (command)
    {
      case UNIQUE_ACTIVATE:
        /* Move window to the screen the command was started on */
        gtk_window_set_screen (GTK_WINDOW (window), unique_message_data_get_screen (message));
        /* Bring window to the foreground */
        gtk_window_present_with_time (GTK_WINDOW (window), time_);
        response = UNIQUE_RESPONSE_OK;
        break;
      default:
        /* Invalid command */
        response = UNIQUE_RESPONSE_FAIL;
        break;
    }

  return response;
}



int
main (int    argc,
      char **argv)
{
  UniqueApp          *app;
  GtkWidget          *window;
  GError             *error = NULL;
  gboolean            debug_mode = FALSE;
  gboolean            show_version = FALSE;
  GOptionContext     *option_context;
  GOptionEntry        option_entries[] =
  {
    { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug_mode, N_("Enable debugging output"), NULL },
    { "version", 'V', 0, G_OPTION_ARG_NONE, &show_version, N_("Show version and exit"), NULL },
    { NULL, 0, 0, 0, NULL, NULL, NULL }
  };

  /* Setup translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

#if !GLIB_CHECK_VERSION(2, 32, 0)
  /* Initialize the threading system */
  if (G_LIKELY (!g_thread_supported ()))
    g_thread_init (NULL);
#endif

  /* Set application name */
  g_set_application_name (_("Audio Mixer"));

  /* Parse commandline options */
  option_context = g_option_context_new ("- Adjust volume levels");
  g_option_context_add_main_entries (option_context, option_entries, GETTEXT_PACKAGE);
  g_option_context_add_group (option_context, gtk_get_option_group (FALSE));
  g_option_context_add_group (option_context, gst_init_get_option_group ());
  g_option_context_parse (option_context, &argc, &argv, &error);
  g_option_context_free (option_context);
  if (error != NULL)
    {
      g_printerr ("xfce4-mixer: %s\n", error->message);

      return EXIT_FAILURE;
    }

  if (show_version)
    {
      g_print ("xfce4-mixer " VERSION "\n");

      return EXIT_SUCCESS;
    }

  /* Initialize GTK+ fully */
  gtk_init (NULL, NULL);

  /* Initialize Xfconf */
  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_printerr (_("xfce4-mixer: Failed to initialize xfconf: %s\n"), error->message);
          g_error_free (error);
        }

      return EXIT_FAILURE;
    }

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

  /* Initialize debugging code */
  xfce_mixer_debug_init (G_LOG_DOMAIN, debug_mode);

  xfce_mixer_debug ("xfce4-mixer version " VERSION " starting up");

  if (debug_mode)
    xfce_mixer_dump_gst_data ();

  /* Create unique application */
  app = unique_app_new ("org.xfce.xfce4-mixer", NULL);
  if (unique_app_is_running (app))
    {
      unique_app_send_message (app, UNIQUE_ACTIVATE, NULL);

      g_object_unref (app);
    }
  else
    {
      /* Create the mixer window */
      window = xfce_mixer_window_new ();

      /* Display the mixer window */
      gtk_widget_show (window);

      /* Watch mixer window */
      unique_app_watch_window (app, GTK_WINDOW (window));

      /* Handle messages */
      g_signal_connect (app, "message-received", G_CALLBACK (message_received), window);

      /* Enter the GTK+ main loop */
      gtk_main ();

      g_object_unref (app);

      /* Destroy the window */
      gtk_widget_destroy (window);
    }

  /* Shutdown the mixer library */
  xfce_mixer_shutdown ();

  /* Shutdown Xfconf */
  xfconf_shutdown ();

  return EXIT_SUCCESS;
}

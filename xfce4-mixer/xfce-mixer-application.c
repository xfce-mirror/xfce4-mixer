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

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <xfconf/xfconf.h>

#include "libxfce4mixer/libxfce4mixer.h"

#include "xfce-mixer-application.h"
#include "xfce-mixer-window.h"



static void xfce_mixer_application_dispose              (GObject      *object);
static void xfce_mixer_application_finalize             (GObject      *object);
static void xfce_mixer_application_startup              (GApplication *app);
static void xfce_mixer_application_activate             (GApplication *app);
static void xfce_mixer_application_shutdown             (GApplication *app);
static gint xfce_mixer_application_handle_local_options (GApplication *app,
                                                         GVariantDict *options,
                                                         gpointer      user_data);



struct _XfceMixerApplication
{
  GtkApplication  __parent__;

  GtkWidget    *main_window;
};



G_DEFINE_TYPE (XfceMixerApplication, xfce_mixer_application, GTK_TYPE_APPLICATION)



static gboolean debug_mode = FALSE;
static gboolean show_version = FALSE;

static GOptionEntry option_entries[] =
{
  { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug_mode, N_("Enable debugging output"), NULL },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &show_version, N_("Show version and exit"), NULL },
  { NULL, 0, 0, 0, NULL, NULL, NULL }
};



static void
xfce_mixer_application_class_init (XfceMixerApplicationClass *klass)
{
  GObjectClass      *gobject_class;
  GApplicationClass *g_application_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = xfce_mixer_application_dispose;
  gobject_class->finalize = xfce_mixer_application_finalize;

  g_application_class = G_APPLICATION_CLASS (klass);
  g_application_class->startup = xfce_mixer_application_startup;
  g_application_class->activate = xfce_mixer_application_activate;
  g_application_class->shutdown = xfce_mixer_application_shutdown;
}



static void
xfce_mixer_application_init (XfceMixerApplication *app)
{
  app->main_window = NULL;

  g_application_add_main_option_entries (G_APPLICATION (app), option_entries);
  g_application_add_option_group (G_APPLICATION (app), gst_init_get_option_group ());

  g_signal_connect (app, "handle-local-options", G_CALLBACK (xfce_mixer_application_handle_local_options), NULL);
}



static void
xfce_mixer_application_dispose (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_application_parent_class)->dispose) (object);
}



static void
xfce_mixer_application_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (xfce_mixer_application_parent_class)->finalize) (object);
}



static void
xfce_mixer_application_startup (GApplication *app)
{
  GError      *error = NULL;

  (*G_APPLICATION_CLASS (xfce_mixer_application_parent_class)->startup) (app);

  /* Initialize Xfconf */
  if (G_UNLIKELY (!xfconf_init (&error)))
    {
      if (G_LIKELY (error != NULL))
        {
          g_critical ("Failed to initialize xfconf: %s", error->message);
          g_error_free (error);
        }

      exit (EXIT_FAILURE);
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
      exit (EXIT_FAILURE);
    }

  /* Initialize debugging code */
  xfce_mixer_debug_init (G_LOG_DOMAIN, debug_mode);

  xfce_mixer_debug ("xfce4-mixer version " VERSION " starting up");

  if (debug_mode)
    xfce_mixer_dump_gst_data ();
}



static void
xfce_mixer_application_activate (GApplication *app_)
{
  XfceMixerApplication *app = XFCE_MIXER_APPLICATION (app_);

  /* Create the mixer window */
  if (app->main_window == NULL)
    app->main_window = xfce_mixer_window_new (app_);

  /* Display the mixer window */
  gtk_window_present (GTK_WINDOW (app->main_window));
}



static void
xfce_mixer_application_shutdown (GApplication *app)
{
  /* Shutdown the mixer library */
  xfce_mixer_shutdown ();

  /* Shutdown Xfconf */
  xfconf_shutdown ();

  (*G_APPLICATION_CLASS (xfce_mixer_application_parent_class)->shutdown) (app);
}



static gint
xfce_mixer_application_handle_local_options (GApplication *app,
                                             GVariantDict *options,
                                             gpointer      user_data)
{
  if (show_version)
    {
      g_print ("xfce4-mixer " VERSION "\n");
      return EXIT_SUCCESS;
    }

  return -1;
}



GApplication*
xfce_mixer_application_new (void)
{
  XfceMixerApplication *app;

  app = g_object_new (XFCE_TYPE_MIXER_APPLICATION, "application-id", "org.xfce.xfce4-mixer", NULL);

  return G_APPLICATION (app);
}

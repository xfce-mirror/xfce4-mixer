static void
appexit_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	gtk_main_quit ();
}

static void
info_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	GtkWidget *i;
	GtkWidget *w;
	gchar *s;
	i = xfce_mixer_info_new ();
	w = gtk_widget_get_toplevel (cmd);
	gtk_window_set_transient_for (GTK_WINDOW (i), GTK_WINDOW (w));
	s = g_strdup_printf (_("%s - Info"), gtk_window_get_title (GTK_WINDOW (w)));
	gtk_window_set_title (GTK_WINDOW (i), s);
	g_free (s);
	gtk_dialog_run (GTK_DIALOG (i));
	gtk_widget_destroy (i);
}

static void
options_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	g_spawn_command_line_async ("xfce-setting-show sound", NULL);
}

static void
profile_mgr_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	XfceMixerProfiledlg *dlg;
	GtkWidget *w;
	gchar *s;
	w = gtk_widget_get_toplevel (cmd);
	dlg = xfce_mixer_profiledlg_new ();
	gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (w));
	s = g_strdup_printf (_("%s - Profiles"), gtk_window_get_title (GTK_WINDOW (w)));
	gtk_window_set_title (GTK_WINDOW (dlg), s);
	g_free (s);
	gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (GTK_WIDGET (dlg));
}


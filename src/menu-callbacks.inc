static void
appexit_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	gtk_main_quit ();
}

static void
info_activate_cb(GtkWidget *cmd, gpointer user_data)
{
	GtkWidget *i;
	i = xfce_mixer_info_new ();
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
}


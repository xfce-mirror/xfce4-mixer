
static GtkResponseType
annoy_user (
	GtkWindow *parent, 
	GtkMessageType t, GtkButtonsType b, 
	gchar const *fmt, ...
)
{
	GtkResponseType	rt;
	GtkWidget *w;
	gchar *s;
	va_list	ap;
	va_start (ap, fmt);
	s = g_strdup_vprintf (fmt, ap);
	w = gtk_message_dialog_new (parent, 
		GTK_DIALOG_MODAL |GTK_DIALOG_DESTROY_WITH_PARENT,
		t, b, "%s", s); 

	rt = gtk_dialog_run (GTK_DIALOG (w));

	gtk_widget_destroy (GTK_WIDGET (w));

	g_free (s);
	va_end (ap);

	return rt;
}


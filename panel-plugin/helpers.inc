
#ifdef FHELPERS1
static GtkWidget *headline(gchar const *title, GtkWidget **box_o)
{
	GtkWidget *w;
	GtkWidget *f;

	w = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (GTK_WIDGET (w));

	f = xfce_framebox_new (title, TRUE);
	xfce_framebox_add (XFCE_FRAMEBOX (f), w);
	gtk_widget_show (GTK_WIDGET (f));

	*box_o = w;

	return f;
}

static GtkWidget *twocol_label(gchar const *n, GtkWidget *c)
{
	GtkWidget *l;
	GtkWidget *b;
	l = gtk_label_new (n);
	gtk_widget_show (l);

	gtk_widget_show (c);

	b = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (b), GTK_WIDGET (l), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (b), GTK_WIDGET (c), TRUE, TRUE, 0);

	gtk_widget_show (b);

	return b;
}
#endif

#ifdef FHELPERS1
static gchar *nvl(gchar *s, gchar *n)
{
	/*assert(n);*/
	if (s)
		return s;
	else
		return n;
}
#endif

static void master_enum_cb(gpointer data, gpointer user_data)
{
	GList **lst;
	volcontrol_t *vci;

	vci = (volcontrol_t *) data;
	lst = (GList **) user_data;

	if (vci && vci->name && (xfce_mixer_cache_vc_get_type (vci->name) == 'S')) {
		(*lst) = g_list_append ((*lst), g_strdup (vci->name));
	}
}

#ifdef FHELPERS1
#include "delayer.inc"

#if 0
/* get translation from the panel's domain */
static gchar *P_(gchar *s)
{
	return(dgettext("xfce4-panel", s));
}
#endif
#endif

#define emit

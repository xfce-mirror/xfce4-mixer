
#ifndef __stringlist_free
#define __stringlist_free
static void stringlist_free(GList *lst)
{
	GList *g;
	gchar *s;
	g = lst;
	while (g) {
		s = (gchar *)g->data;
		if (s)
			g_free (s);

		g = g_list_next (g);
	}
	if (lst)
		g_list_free (lst);
}

#endif

static void delayer_free (delayer_t idsave)
{
	if (idsave)
		g_source_remove (idsave);
}

static void delayer_restart (delayer_t *idsave, guint interval, GSourceFunc func, gpointer user_data)
{
	if (!idsave) {
		/* dangerous... */
		g_timeout_add (interval, func, user_data);
		return;
	}

	delayer_free (*idsave);

	*idsave = g_timeout_add (interval, func, user_data);
}

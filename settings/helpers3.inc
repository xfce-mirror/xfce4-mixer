static GtkTreeViewColumn *
tree_new_check_column (gchar const *title, gint datalink)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *cell;

	cell = gtk_cell_renderer_toggle_new ();

	col = gtk_tree_view_column_new_with_attributes
                                            (title,
                                             cell,
						"active", datalink,
                                             NULL);

	return col;
}

static GtkTreeViewColumn *
tree_new_text_column (gchar const *title, gint datalink)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *cell;

	cell = gtk_cell_renderer_text_new ();

	col = gtk_tree_view_column_new_with_attributes
                                            (title,
                                             cell,
						"text", datalink, 
                                             NULL);

	return col;
}


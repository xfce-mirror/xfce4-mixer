#ifndef NO_TREE_CHECK_COLUMN
static void
tree_check_column_toggled_cb (
	GtkCellRendererToggle *cell,
	gchar *path_str, gpointer data
)
{
	GtkTreeModel *model = (GtkTreeModel *)data;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeIter iter;
	gboolean toggle_item;
	
	gint *column;
	column = g_object_get_data (G_OBJECT (cell), "column"); 

	/* get toggled iter */
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, column, &toggle_item, -1);
     
	/* do something with the value */
	toggle_item ^= 1;
         	
	/* set new value */
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,
                        toggle_item, -1);
                        
	/* clean up */
	gtk_tree_path_free (path);
}

static GtkTreeViewColumn *
tree_new_check_column (gchar const *title, gint datalink, GtkTreeModel *model)
{
	GtkTreeViewColumn *col;
	GtkCellRenderer *cell;

	cell = gtk_cell_renderer_toggle_new ();

	col = gtk_tree_view_column_new_with_attributes
                                            (title,
                                             cell,
						"active", datalink,
                                             NULL);

	g_signal_connect (G_OBJECT (cell), 
		"toggled", G_CALLBACK (tree_check_column_toggled_cb), 
		model
	);

	return col;
}
#endif

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


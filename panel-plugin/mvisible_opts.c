#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include "mvisible_opts.h"

mvisible_opts_t *mvisible_opts_new()
{
	mvisible_opts_t	*o = g_new0 (mvisible_opts_t, 1);
	
	return o;
}

static void
mvisible_toggled_cb(
	GtkCellRendererToggle *cell,
	gchar *path_str,
	gpointer data)
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
                            

void mvisible_opts_fill(GtkWidget *parent, mvisible_opts_t *o, GList *source)
{
	GList		*item;
	char		*txt;
	GtkTreeIter	*iter = g_new0(GtkTreeIter, 1);
	GtkTreeViewColumn *col0;
	GtkTreeViewColumn *col1;
	GtkCellRenderer	*cell0;
	GtkCellRenderer	*cell1;
	

	if (!o->treestore) {
		o->treestore = GTK_TREE_STORE (gtk_tree_store_new (2, G_TYPE_BOOLEAN, G_TYPE_STRING));
	}
	
	if (!o->tv) {
		o->tv = GTK_TREE_VIEW (gtk_tree_view_new ());
		gtk_tree_view_set_model (o->tv, GTK_TREE_MODEL (o->treestore));

		cell0 = gtk_cell_renderer_toggle_new();
		col0 = gtk_tree_view_column_new_with_attributes (
			_("Visible"), cell0, "active", 0, NULL);
					
		gtk_tree_view_append_column (o->tv, col0);
		
		g_signal_connect (cell0, "toggled", G_CALLBACK (mvisible_toggled_cb), GTK_TREE_MODEL(o->treestore));

		cell1 = gtk_cell_renderer_text_new();
		col1 = gtk_tree_view_column_new_with_attributes (
			_("Control"), cell1, "text", 1, NULL);
					
		gtk_tree_view_append_column (o->tv, col1);
		
		gtk_widget_show (GTK_WIDGET (o->tv));		
		
		gtk_container_add (GTK_CONTAINER (parent), GTK_WIDGET (o->tv));
	}
	
	gtk_tree_store_clear (o->treestore);


	item = source;
	while (item) {
		txt = (char *)item->data;
		
		/*gtk_tree_store_get_root_iter (&iter);*/
		gtk_tree_store_append (o->treestore, iter, NULL);
		/*gtk_tree_store_set_value (o->treestore, );*/

#if 0
		v = NULL;		
		v = g_value_init(v, G_TYPE_STRING);
		
		printf("-> %s\n", txt);
		gtk_tree_store_set_value (o->treestore, iter, 1, v);

		g_warning(txt);
		g_warning("\n");
#endif

		gtk_tree_store_set (o->treestore, iter, 0, TRUE, -1);
		gtk_tree_store_set (o->treestore, iter, 1, txt, -1);
	
		item = g_list_next (item);
	}	
	
}

static
GList *mvisible_opts_get_da(mvisible_opts_t *o, gboolean active)
{
	GtkTreeIter	iter;
	GList		*l = NULL;
	gchar		*txt;
	gboolean	iactive;
	
	if (gtk_tree_model_get_iter_root (GTK_TREE_MODEL (o->treestore), &iter)) {
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (o->treestore), &iter, 0, &iactive, 1, &txt, -1);
			
			if (active == iactive && txt) {
				l = g_list_append (l, txt);
			}
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (o->treestore), &iter));
	}
	return l;
}

static
void mvisible_opts_set_da(mvisible_opts_t *o, gboolean active, GList *l)
{
	GtkTreeIter	iter;
	gchar		*txt;
	gboolean	iactive;
	GList		*i;
	
	if (gtk_tree_model_get_iter_root (GTK_TREE_MODEL (o->treestore), &iter)) {
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (o->treestore), &iter, 0, &iactive, 1, &txt, -1);
			
			if (txt) {
				iactive = FALSE;
				
				i = l;
				while (i) {
					if (!strcmp ((char *)i->data, txt)) {
						iactive = TRUE;
						break;
					}
					i = g_list_next (i);
				}
				
				gtk_tree_store_set (GTK_TREE_STORE (o->treestore), &iter, 0, iactive, -1);

				g_free (txt);
			}
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (o->treestore), &iter));
	}
}

void mvisible_opts_free_actives(GList *l)
{
	GList *g = l;
	gchar *vc;
	while (g) {
		vc = (gchar *)g->data;
		if ( vc ) g_free (vc);
		g = g_list_next (g);
	}
	
	g_list_free (l);
}

void mvisible_opts_set_actives(mvisible_opts_t *o, GList *l)
{
	mvisible_opts_set_da (o, TRUE, l);
}

GList *mvisible_opts_get_actives(mvisible_opts_t *o)
{
	return mvisible_opts_get_da (o, TRUE);
}

#if 0
static
GList *mvisible_opts_get_deactives(mvisible_opts_t *o)
{
	return mvisible_opts_get_da (o, FALSE);
}
#endif

void mvisible_opts_free(mvisible_opts_t *o)
{
	if (o) {
		if (o->treestore) {
			/*o->treestore auto-freed*/
			o->treestore = NULL;
		}
		if (o->tv) {
			/*o->tv auto-freed*/
			o->tv = NULL;
		}
		free(o);
	}
}

#ifdef MAIN
int main(int argc, char *argv[])
{
	GtkWindow *w;
	mvisible_opts_t *o;
	GList *s;
	
	gtk_init (&argc, &argv);
	
	w = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_widget_show (GTK_WIDGET (w));

	o = mvisible_opts_new ();
	
	s = NULL; /*g_list_alloc ();*/
	s = g_list_append(s, "test");
	s = g_list_append(s, "test2");
	
	mvisible_opts_fill (GTK_WIDGET (w), o, s);
	
	gtk_main ();
	
	return 0;
}
#endif

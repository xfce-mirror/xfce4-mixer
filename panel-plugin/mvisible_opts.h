#ifndef __MVISIBLE_OPTS_H
#define __MVISIBLE_OPTS_H
#include <glib.h>
#include <gtk/gtk.h>

typedef struct
{
	GtkTreeView *tv;
	GtkTreeStore *treestore;
} mvisible_opts_t;

mvisible_opts_t *mvisible_opts_new();
void mvisible_opts_fill(GtkWidget *parent, mvisible_opts_t *o, GList *source);
GList *mvisible_opts_get_actives(mvisible_opts_t *o);
/*GList *mvisible_opts_get_deactives(mvisible_opts_t *o);*/
void mvisible_opts_set_actives(mvisible_opts_t *o, GList *l);
void mvisible_opts_free(mvisible_opts_t *o);
void mvisible_opts_free_actives(GList *l);

#endif /* ndef __MVISIBLE_OPTS_H */

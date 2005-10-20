#include "launcher-entry.h"

/* copied from xfce4-panel launcher plugin as-is */

LauncherEntry *
launcher_entry_new (void)
{
    return g_new0 (LauncherEntry, 1);
}

void
launcher_entry_free (LauncherEntry *e)
{
    g_free (e->name);
    g_free (e->comment);
    if (e->icon.type == LAUNCHER_ICON_TYPE_NAME)
        g_free (e->icon.icon.name);
    g_free (e->exec);

    g_free (e);
}

static void
launcher_entry_exec (LauncherEntry *entry)
{
    GError *error = NULL;
    
    if (!entry->exec || !strlen (entry->exec))
        return;
    
    xfce_exec (entry->exec, entry->terminal, entry->startup, &error);
        
    if (error)
    {
        char *first = 
            g_strdup_printf (_("Could not run \"%s\""), entry->name);
    
        xfce_message_dialog (NULL, _("Xfce Panel"), 
                             GTK_STOCK_DIALOG_ERROR, first, error->message,
                             GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

        g_free (first);
                                    
        g_error_free (error);
    }
}

static void
launcher_entry_drop_cb (GdkScreen *screen, LauncherEntry *entry, 
                        GPtrArray *files)
{
    char **argv;
    int i, n;
    GError *error = NULL;

    n = files->len;
    
    if (entry->terminal)
    {
        argv = g_new (char *, n + 4);
        argv[0] = "xfterm4";
        argv[1] = "-e";
        argv[2] = entry->exec;
        n = 3;
    }
    else
    {
        argv = g_new (char *, n + 2);
        argv[0] = entry->exec;
        n = 1;
    }

    for (i = 0; i < files->len; ++i)
        argv[n+i] = g_ptr_array_index (files, i);

    argv[n+i] = NULL;
    
    if (!xfce_exec_argv (argv, entry->terminal, entry->startup, &error))
    {
        char *first = 
            g_strdup_printf (_("Could not run \"%s\""), entry->name);
    
        xfce_message_dialog (NULL, _("Xfce Panel"), 
                             GTK_STOCK_DIALOG_ERROR, first, error->message,
                             GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

        g_free (first);
                                    
        g_error_free (error);
    }

    g_free (argv);
}

static void
launcher_entry_data_received (GtkWidget *widget, GdkDragContext *context, 
                              gint x, gint y, GtkSelectionData *data, 
                              guint info, guint time, LauncherEntry *entry)
{
    GPtrArray *files;
    
    if (!data || data->length < 1)
        return;
    
    files = launcher_get_file_list_from_selection_data (data);

    if (files)
    {
        launcher_entry_drop_cb (gtk_widget_get_screen (widget), entry, files);
        g_ptr_array_free (files, TRUE);
    }

    if (open_launcher)
    {
        gtk_widget_hide (GTK_MENU (open_launcher->menu)->toplevel);
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (open_launcher->arrowbutton), FALSE);
        open_launcher = NULL;
    }
}


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <assert.h>
#include <ctype.h>
#include "launcher-entry.h"

/* copied from xfce4-panel launcher plugin as-is, added laucher_entry_set_command, ... */

struct _LauncherEntry
{
    GtkWidget* widget;
    GtkEntry* exec_widget;
    GtkCheckButton* terminal_widget;
    GtkCheckButton* startup_widget;
    
    /*char *name;
    char *comment;*/
    
    char *exec;

    /*LauncherIcon icon; */

    unsigned int terminal:1;
    unsigned int startup:1;
};


static void
launcher_entry_update_data_from_gui(LauncherEntry* e)
{
  gchar const* exec = gtk_entry_get_text(GTK_ENTRY(e->exec_widget));

  e->terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(e->terminal_widget));
  e->startup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(e->startup_widget));
  
  if (e->exec != NULL) {
    g_free (e->exec);
    e->exec = NULL;
  }
  
  if (exec != NULL) {
    e->exec = g_strdup (exec);
  }
  
}

LauncherEntry *
launcher_entry_new (void)
{
    LauncherEntry* launcher_entry;
    launcher_entry = g_new0 (LauncherEntry, 1);
    
    launcher_entry->widget = gtk_vbox_new (FALSE, 5);
    
    gtk_container_set_border_width (GTK_CONTAINER (launcher_entry->widget), 7);
    
    launcher_entry->exec_widget = GTK_ENTRY (gtk_entry_new ());
    gtk_widget_show (GTK_WIDGET (launcher_entry->exec_widget));
    
    launcher_entry->terminal_widget = GTK_CHECK_BUTTON (
      gtk_check_button_new_with_label (_("Run in Terminal"))
    );
    gtk_widget_show (GTK_WIDGET (launcher_entry->terminal_widget));
    
    launcher_entry->startup_widget = GTK_CHECK_BUTTON (
      gtk_check_button_new_with_label (_("Use Startup Notification"))
    );
    gtk_widget_show (GTK_WIDGET (launcher_entry->startup_widget));
    
    gtk_box_pack_start (GTK_BOX (launcher_entry->widget), GTK_WIDGET (launcher_entry->exec_widget), FALSE, TRUE, 5);
    gtk_box_pack_start (GTK_BOX (launcher_entry->widget), GTK_WIDGET (launcher_entry->terminal_widget), FALSE, FALSE, 5);
    gtk_box_pack_start (GTK_BOX (launcher_entry->widget), GTK_WIDGET (launcher_entry->startup_widget), FALSE, FALSE, 5);
    /* TODO launcher_entry_data_received */
    
    return launcher_entry;
}

void
launcher_entry_set_command (LauncherEntry *e, gchar const* command, gboolean terminal, gboolean startupnotification)
{
    if (e->exec) {
      g_free (e->exec);
      e->exec = NULL;
    }
    
    assert (command != NULL);
    e->exec = g_strdup (command);
    
    e->terminal = terminal != 0;
    e->startup = startupnotification != 0;
}

void
launcher_entry_free (LauncherEntry *e)
{
    if (e->widget) {
      g_object_ref (G_OBJECT (e->widget));
      gtk_object_sink (GTK_OBJECT (e->widget));
      g_object_unref (G_OBJECT (e->widget));
    }
      
/*    g_free (e->name);
    g_free (e->comment);
    if (e->icon.type == LAUNCHER_ICON_TYPE_NAME)
        g_free (e->icon.icon.name);
*/
    g_free (e->exec);

    g_free (e);
}

/*
void
launcher_entry_execute (LauncherEntry *entry)
{
    GError *error;
    
    launcher_entry_update_data_from_gui(entry);
    if (!entry->exec || !entry->exec[0])
        return;
        
    error = NULL;
    
    g_warning ("terminal %d", entry->terminal);
    
    xfce_exec (entry->exec, entry->terminal, entry->startup, &error);
        
    if (error)
    {
        char *first = 
            g_strdup_printf (_("Could not run \"%s\""), entry->exec);
    
        xfce_message_dialog (NULL, _("Xfce Panel"), 
                             GTK_STOCK_DIALOG_ERROR, first, error->message,
                             GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

        g_free (first);
                                    
        g_error_free (error);
    }
}
*/

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
            g_strdup_printf (_("Could not run \"%s\""), entry->exec);
    
        xfce_message_dialog (NULL, _("Xfce Panel"), 
                             GTK_STOCK_DIALOG_ERROR, first, error->message,
                             GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);

        g_free (first);
                                    
        g_error_free (error);
    }

    g_free (argv);
}

GPtrArray *
launcher_get_file_list_from_selection_data (GtkSelectionData *data)
{
    GPtrArray *files;
    const char *s1, *s2;

    if (data->length < 1)
        return NULL;

    files = g_ptr_array_new ();

    /* Assume text/uri-list (RFC2483):
     * - Commented lines are allowed; they start with #.
     * - Lines are separated by CRLF (\r\n)
     * - We also allow LF (\n) as separator
     * - We strip "file:" and multiple slashes ("/") at the start
     */
    for (s1 = (const char *)data->data; s1 != NULL && strlen(s1); ++s1)
    {
        if (*s1 != '#')
        {
            while (isspace ((int)*s1))
                ++s1;
            
            if (!strncmp (s1, "file:", 5))
            {
                s1 += 5;
                while (*(s1 + 1) == '/')
                    ++s1;
            }
            
            for (s2 = s1; *s2 != '\0' && *s2 != '\r' && *s2 != '\n'; ++s2)
                /* */;
            
            if (s2 > s1)
            {
                while (isspace ((int)*(s2-1)))
                    --s2;
                
                if (s2 > s1)
                {
                    int len, i;
                    char *file;
                    
                    len = s2 - s1;
                    file = g_new (char, len + 1);
                    
                    /* decode % escaped characters */
                    for (i = 0, s2 = s1; s2 - s1 <= len; ++i, ++s2)
                    {
                        if (*s2 != '%' || s2 + 3 -s1 > len)
                        {
                            file[i] = *s2;
                        }
                        else
                        {
                            guint c;
                            
                            if (sscanf (s2+1, "%2x", &c) == 1)
                                file[i] = (char)c;

                            s2 += 2;
                        }
                    }

                    file[i-1] = '\0';
                    g_ptr_array_add (files, file);
                }
            }
        }
        
        if (!(s1 = strchr (s1, '\n')))
            break;
    }
    
    if (files->len > 0)
    {
        return files;
    }

    g_ptr_array_free (files, TRUE);

    return NULL;    
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

/*    if (open_launcher)
    {
        gtk_widget_hide (GTK_MENU (open_launcher->menu)->toplevel);
        gtk_toggle_button_set_active (
                GTK_TOGGLE_BUTTON (open_launcher->arrowbutton), FALSE);
        open_launcher = NULL;
    }*/
}

GtkWidget* launcher_entry_get_widget (LauncherEntry* e)
{
  return e->widget;
}

void launcher_entry_get_command (LauncherEntry *e, gchar** command, gboolean* terminal, gboolean* startupnotification)
{
  launcher_entry_update_data_from_gui(e);

  if (e->exec) {
    *command = g_strdup (e->exec);
  } else {
    *command = NULL;
  }
  
  *terminal = e->terminal;
  *startupnotification = e->startup;
}


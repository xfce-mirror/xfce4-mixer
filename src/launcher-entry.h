#ifndef __PANEL_LAUNCHER_ENTRY_H
#define __PANEL_LAUNCHER_ENTRY_H

/* copied from xfce4-panel launcher plugin as-is */

struct _LauncherEntry
{
    char *name;
    char *comment;
    char *exec;

    /*LauncherIcon icon; */

    unsigned int terminal:1;
    unsigned int startup:1;
};

#endif

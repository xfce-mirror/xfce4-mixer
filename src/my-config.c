#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>

#include "my-config.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
/*#ifdef HAVE_SYS_FILE_H not present */
#include <sys/file.h>
/*#endif*/
#include <errno.h>

static void 
migrate_errno_str(gchar const *oldpath, gchar const *newpath, int errno, gboolean newfile_culprit)
{
	char tmp[2049];
	gchar *whichfile;
	if (strerror_r (errno, tmp, sizeof(tmp)) != -1) {
		if (!newfile_culprit)
			whichfile = "old file";
		else
			whichfile = "new file";
			
		g_warning("could not migrate \"%s\" to \"%s\" because: %s (%s)", oldpath, newpath, tmp, whichfile);
	} else {
		g_warning("could not migrate \"%s\" to \"%s\" because something went wrong (%s)", oldpath, newpath, whichfile);
	}
}

static void
copy_from_old_config(gchar const *oldpath, gchar const *filename)
{
	gchar *abspath;
	int newfile;
	int oldfile;
	char buf[20000];
	ssize_t rcnt;
	ssize_t wcnt;

	if (!oldpath)
		return; /* no old file: nothing to migrate from */

	abspath = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, filename, TRUE);
			
	if (!abspath) /* no new file: nothing to migrate to */
		return;
		
	oldfile = -1;
	newfile = -1;
	
	oldfile = open(oldpath, O_RDONLY);
	if (oldfile == -1) { /* no old file: nothing to migrate from */
		migrate_errno_str (oldpath, abspath, errno, FALSE);
		goto endme;
	}
	
	flock(oldfile, LOCK_EX);

	newfile = open(abspath, O_CREAT|O_WRONLY, 0644);
	if (newfile == -1) { /* new file couldnt be created, probably already exists */
		migrate_errno_str (oldpath, abspath, errno, TRUE);
		goto endme;
	}
	flock(newfile, LOCK_EX);
	
	/* at this point both files are open and to be copied */
	while ((rcnt = read (oldfile, buf, sizeof(buf))) > 0) {
		wcnt = write (newfile, buf, rcnt);
		if (wcnt <= 0) { /* something went wrong */
			if (wcnt < 0) /* normal error */
				migrate_errno_str (oldpath, abspath, errno, TRUE);
			else /* impossible error */
				migrate_errno_str (oldpath, abspath, EIO, TRUE);

			close (newfile); /* -1: dont care since its being deleted anyways */
			newfile = -1;
			unlink (abspath);
			goto endme;
		}
	}
	
	if (rcnt < 0) {
		migrate_errno_str (oldpath, abspath, errno, FALSE);
		goto endme;
	}
	
endme:
	if (newfile != -1) { /* open: close down */
		flock(newfile, LOCK_UN);
		if (close(newfile) == -1) { /* didnt work, something went wrong when flushing, probably, so revert. */
			migrate_errno_str (oldpath, abspath, errno, TRUE);
			
			newfile = -1;
			unlink (abspath);
		}
	}
		
	if (oldfile != -1) {
		flock(oldfile, LOCK_UN);
		close(oldfile); /* -1: dont care since we already read everything */
	}
		
	g_free (abspath);
}

gchar *
get_config_path(gchar const *relpath, ConfigAction save)
{
	gchar *filename;
	gchar *abspath;
	abspath = NULL;
	if (!relpath)
		return NULL;
	
	filename = g_build_filename (MIXER_CONFIG_SUBPATH, relpath, NULL);
	
	if (filename) {
		if (save)
			abspath = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, filename, TRUE);
		else {
			abspath = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, filename);
			if (!abspath) {
				/* look for config in old location */
				abspath = xfce_get_userfile ("xfce4-mixer", relpath, NULL);
				if (abspath && access (abspath, R_OK)) {
					copy_from_old_config (abspath, filename);
					g_free (abspath);
					abspath = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, filename);
				}
			}
		}
	
		g_free (filename);
	}
	return abspath;
}

#endif

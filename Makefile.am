# $Id$

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST =								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in						\
	xfce4-mixer.spec 						\
	NOTES

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update


if BUILD_BIG_MIXER
SUBDIR_BIG_MIXER = src settings
endif

if BUILD_MIGHTY_MOUSE_PANEL_PLUGIN
if HAVE_MIGHTY_MOUSE
SUBDIR_MIGHTY_MOUSE_PANEL_PLUGIN = panel-plugin
endif
endif

SUBDIRS = lib $(SUBDIR_MIGHTY_MOUSE_PANEL_PLUGIN) $(SUBDIR_BIG_MIXER) po
DIST_SUBDIRS = include lib panel-plugin src settings po

AUTOMAKE_OPTIONS =							\
	1.8								\
	dist-bzip2

distclean-local:
	rm -rf *.cache *~ *.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

# vi:set ts=8 sw=8 noet ai nocindent:

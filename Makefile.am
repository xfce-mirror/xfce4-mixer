# $Id$
#
# vi:set ts=8 sw=8 noet ai nocindent:

@SET_MAKE@

SUBDIRS =								\
	icons								\
	pixmaps								\
	xfce4-mixer							\
	po

AUTOMAKE_OPTIONS =							\
	dist-bzip2

distclean-local:
	rm -rf *.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

EXTRA_DIST =								\
	AUTHORS								\
	ChangeLog							\
	COPYING								\
	HACKING								\
	INSTALL								\
	NEWS								\
	README								\
	THANKS								\
	TODO								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update

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

SUBDIRS = lib panel-plugin $(SUBDIR_BIG_MIXER) po
DIST_SUBDIRS = include lib panel-plugin src settings po

distclean-local:
	rm -rf *.cache *~ *.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

dist-bz2: dist
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

distcheck-bz2: distcheck
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

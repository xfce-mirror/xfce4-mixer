@SET_MAKE@

EXTRA_DIST = config.rpath  \
	xfce4-mixer.spec \
	autogen.sh \
	NOTES

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

ACLOCAL_AMFLAGS = -I m4


dist-bz2: dist
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

distcheck-bz2: distcheck
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

@SET_MAKE@

EXTRA_DIST = config.rpath  \
	xfce4-mixer.spec \
	autogen.sh \
	NOTES

SUBDIRS = lib src panel-plugin settings po

distclean-local:
	rm -rf *.cache *~ *.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

ACLOCAL_AMFLAGS = -I m4


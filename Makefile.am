@SET_MAKE@

EXTRA_DIST = \
	xfce4-mixer.spec \
	autogen.sh \
	NOTES

SUBDIRS = lib src panel-plugin settings

distclean-local:
	rm -rf *.cache *~ *.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

ACLOCAL_AMFLAGS = -I m4


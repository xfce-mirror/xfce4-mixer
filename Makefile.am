@SET_MAKE@

EXTRA_DIST = \
	xfce4-mixer.spec \
	NOTES

SUBDIRS = src panel-plugin settings

distclean-local:
	rm -rf *.cache *~ *.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

ACLOCAL_AMFLAGS = -I m4


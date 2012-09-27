# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
# -
# Copyright (c) 2011 Jannis Pohlmann <jannis@xfce.org>
# 
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of 
# the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public 
# License along with this program; if not, write to the Free 
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

@SET_MAKE@

SUBDIRS =								\
	icons								\
	pixmaps								\
	libxfce4mixer							\
	panel-plugin							\
	xfce4-mixer							\
	po

AUTOMAKE_OPTIONS =							\
	dist-bzip2

distclean-local:
	rm -rf *.cache *~ ChangeLog

.PHONY: ChangeLog

ChangeLog: Makefile
	$(AM_V_GEN) if GIT_DIR=$(top_srcdir)/.git git log >$@.tmp; then \
	    mv $@.tmp $@; \
	else \
	    rm -f $@.tmp; \
	    touch $@; \
	    echo 'Could not generate ChangeLog from git: installing possibly empty changelog.' >&2; \
	fi

dist-hook: ChangeLog

EXTRA_DIST =								\
	AUTHORS								\
	ChangeLog							\
	COPYING								\
	INSTALL								\
	NEWS								\
	README								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update

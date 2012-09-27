#!/bin/sh
#
# vi:set et ai sw=2 sts=2 ts=2: */
#-
# Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
# Copyright (c) 2012 Guido Berhoerster <guido+xfce@berhoerster.name>
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

# finds the given command in $PATH
findpath () {
    if [ $# -ne 1 ] || [ -z "$1" ]; then
        return 1
    fi

    _findpath_cmd="$1"
    oIFS="${IFS}"
    IFS=:
    set -- ${PATH}
    IFS="${oIFS}"

    while [ $# -gt 0 ]; do
        if [ -x "$1/${_findpath_cmd}" ]; then
            printf "%s\n" "$1/${_findpath_cmd}"
            unset _findpath_cmd oIFS
            return 0
        fi
        shift
    done

    unset _findpath_cmd oIFS
    return 1
}

xdt_autogen="$(findpath xdt-autogen)" || {
  cat >&2 <<EOF
autogen.sh: You don't seem to have the Xfce development tools installed on
            your system, which are required to build this software.
            Please install the xfce4-dev-tools package first, it is available
            from http://www.xfce.org/.
EOF
  exit 1
}

XDT_AUTOGEN_REQUIRED_VERSION="4.7.2" exec "${xdt_autogen}" "$@"

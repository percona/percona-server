#!/bin/sh

# Copyright (c) 2009, 2021, Oracle and/or its affiliates.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301, USA

# Ensure cmake and perl are there
cmake -P cmake/check_minimal_version.cmake >/dev/null 2>&1 || HAVE_CMAKE=no
perl --version >/dev/null 2>&1 || HAVE_PERL=no
scriptdir=`dirname $0`
if test "$HAVE_CMAKE" = "no"
then
  echo "CMake is required to build MySQL."
  exit 1
elif test "$HAVE_PERL" = "no"
then
  echo "Perl is required to build MySQL using the configure to CMake translator."
  exit 1
else
  perl $scriptdir/cmake/configure.pl "$@"
fi


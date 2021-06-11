#!/bin/sh

# Copyright (c) 2003, 2021, Oracle and/or its affiliates.
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

set -e

mkdir -p result
cd result
rm -rf *

if [ `uname | grep -ic cygwin || true` -ne 0 ]
then
  while [ $# -gt 0 ]
  do
    SAVE_IFS=$IFS
    IFS=":"
    declare -a ARR="($1)"
    IFS=$SAVE_IFS
    DIR=`dirname "${ARR[1]}"`
    REMOTE_DIR=`cygpath -u $DIR`
    HOST="${ARR[0]}"
    rsync -a --exclude='BACKUP' --exclude='ndb_*_fs' "$HOST:$REMOTE_DIR" .
    shift
  done
else
  while [ $# -gt 0 ]
  do
#
# The below commented out lines can be used if we want to keep the file
# as part of the result from a faulty test in autotest. The first line
# also keeps the BACKUP files as part of a faulty test case. These lines
# can be used in special autotest runs when a the file contents are
# needed to debug issues in test cases.
#
#    rsync -a "$1" .
#    rsync -a --exclude='BACKUP' "$1" .
    rsync -a --exclude='BACKUP' --exclude='ndb_*_fs' "$1" .
    shift
  done

  #
  # clean tables...not to make results too large
  #
  lst=`find . -name '*.frm'`
  if [ "$lst" ]
  then
 	  basename=`echo $i | sed 's!\.frm!!'`
	  if [ "$basename" ]
	  then
	    rm -f $basename.*
	  fi
  fi

fi

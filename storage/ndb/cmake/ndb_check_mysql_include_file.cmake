# Copyright (c) 2012, 2021, Oracle and/or its affiliates.
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

MACRO(NDB_CHECK_MYSQL_INCLUDE_FILE INCLUDE VARIABLE)
  IF("${${VARIABLE}}" MATCHES "^${${VARIABLE}}$")
    SET(_msg "Looking for MySQL include file ${INCLUDE}")
    MESSAGE(STATUS "${_msg}")
    IF(EXISTS "${CMAKE_SOURCE_DIR}/include/${INCLUDE}")
      MESSAGE(STATUS "${_msg} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have MySQL include ${INCLUDE}")
    ELSE()
      MESSAGE(STATUS "${_msg} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have MySQL include ${INCLUDE}")
    ENDIF()
  ENDIF()
ENDMACRO()


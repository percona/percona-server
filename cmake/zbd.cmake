# Copyright (c) 2021, Percona and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

# cmake -DWITH_ZBD=system|bundled
# bundled is the default

MACRO (FIND_SYSTEM_ZBD)
  FIND_PATH(ZBD_INCLUDE_DIR
    NAMES libzbd/zbd.h)
  FIND_LIBRARY(ZBD_SYSTEM_LIBRARY
    NAMES zbd)
  IF (ZBD_INCLUDE_DIR AND ZBD_SYSTEM_LIBRARY)
    SET(SYSTEM_ZBD_FOUND 1)
    SET(ZBD_LIBRARY ${ZBD_SYSTEM_LIBRARY})
  ENDIF()
ENDMACRO()

SET(BUNDLED_ZBD_PATH "${CMAKE_SOURCE_DIR}/extra/libzbd")

MACRO (MYSQL_USE_BUNDLED_ZBD)
  SET(WITH_ZBD "bundled" CACHE STRING "Bundled zbd library")
  SET(BUILD_BUNDLED_ZBD 1)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM ${BUNDLED_ZBD_PATH}/include)
  SET(ZBD_LIBRARY zbd_lib)
  ADD_LIBRARY(zbd_lib STATIC
    ${BUNDLED_ZBD_PATH}/lib/zbd.c
    ${BUNDLED_ZBD_PATH}/lib/zbd_utils.c
  )
  TARGET_COMPILE_OPTIONS(zbd_lib PRIVATE "-U_GNU_SOURCE")
ENDMACRO()

MACRO (MYSQL_CHECK_ZBD)
  IF(NOT WITH_ZBD)
    SET(WITH_ZBD "bundled" CACHE STRING "By default use bundled zbd library")
  ENDIF()

  IF(WITH_ZBD STREQUAL "bundled")
    MYSQL_USE_BUNDLED_ZBD()
  ELSEIF(WITH_ZBD STREQUAL "system")
    FIND_SYSTEM_ZBD()
    IF (NOT SYSTEM_ZBD_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find system zbd libraries.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_ZBD must be bundled or system")
  ENDIF()
ENDMACRO()

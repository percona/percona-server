# Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.
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

# cmake -DWITH_ZSTD=system|bundled
# bundled is the default

<<<<<<< HEAD
# zstd 1.0.0 is required for: ZSTD_CStream, ZSTD_DStream, ZSTD_createCStream, ZSTD_createDStream, ZSTD_inBuffer, ZSTD_outBuffer
SET(LIBZSTD_VERSION_REQUIRED "1.0.0")

MACRO (CHECK_ZSTD_VERSION)
  SET(PATH_TO_ZSTD_H "${ARGV0}/zstd.h")
  IF (NOT EXISTS ${PATH_TO_ZSTD_H})
    MESSAGE(FATAL_ERROR "File ${PATH_TO_ZSTD_H} not found")
  ENDIF()
  FILE(STRINGS "${PATH_TO_ZSTD_H}" LIBZSTD_HEADER_CONTENT REGEX "#define ZSTD_VERSION_[A-Z]+ +[0-9]+")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_MAJOR +([0-9]+).*" "\\1" LIBZSTD_VERSION_MAJOR "${LIBZSTD_HEADER_CONTENT}")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_MINOR +([0-9]+).*" "\\1" LIBZSTD_VERSION_MINOR "${LIBZSTD_HEADER_CONTENT}")
  STRING(REGEX REPLACE ".*#define ZSTD_VERSION_RELEASE +([0-9]+).*" "\\1" LIBZSTD_VERSION_RELEASE "${LIBZSTD_HEADER_CONTENT}")
  SET(LIBZSTD_VERSION_STRING "${LIBZSTD_VERSION_MAJOR}.${LIBZSTD_VERSION_MINOR}.${LIBZSTD_VERSION_RELEASE}")
  UNSET(LIBZSTD_HEADER_CONTENT)
  IF (${LIBZSTD_VERSION_STRING} VERSION_LESS ${LIBZSTD_VERSION_REQUIRED})
    MESSAGE(FATAL_ERROR "Required libzstd ${LIBZSTD_VERSION_REQUIRED} and installed version is ${LIBZSTD_VERSION_STRING}")
  ELSE()
    MESSAGE(STATUS "Found libzstd version ${LIBZSTD_VERSION_STRING}")
  ENDIF()
ENDMACRO()

||||||| merged common ancestors
=======
# With earier versions, several compression tests fail.
# With version < 1.0.0 our source code does not build.
SET(MIN_ZSTD_VERSION_REQUIRED "1.2.0")

MACRO (FIND_ZSTD_VERSION)
  FOREACH(version_part
      ZSTD_VERSION_MAJOR
      ZSTD_VERSION_MINOR
      ZSTD_VERSION_RELEASE
      )
    FILE(STRINGS "${ZSTD_INCLUDE_DIR}/zstd.h" ${version_part}
      REGEX "^#[\t ]*define[\t ]+${version_part}[\t ]+([0-9]+).*")
    STRING(REGEX REPLACE
      "^.*${version_part}[\t ]+([0-9]+).*" "\\1"
      ${version_part} "${${version_part}}")
  ENDFOREACH()
  SET(ZSTD_VERSION
    "${ZSTD_VERSION_MAJOR}.${ZSTD_VERSION_MINOR}.${ZSTD_VERSION_RELEASE}")
  SET(ZSTD_VERSION "${ZSTD_VERSION}" CACHE INTERNAL "ZSTD major.minor.step")
  MESSAGE(STATUS "ZSTD_VERSION (${WITH_ZSTD}) is ${ZSTD_VERSION}")
ENDMACRO()

>>>>>>> upstream/mysql-8.0.22
MACRO (FIND_SYSTEM_ZSTD)
  FIND_PATH(ZSTD_INCLUDE_DIR
    NAMES zstd.h
    PATH_SUFFIXES include)
  FIND_LIBRARY(ZSTD_SYSTEM_LIBRARY
    NAMES zstd
    PATH_SUFFIXES lib)
<<<<<<< HEAD
  IF (PATH_TO_ZSTD AND ZSTD_SYSTEM_LIBRARY)
    CHECK_ZSTD_VERSION(${PATH_TO_ZSTD})
||||||| merged common ancestors
  IF (PATH_TO_ZSTD AND ZSTD_SYSTEM_LIBRARY)
=======
  IF (ZSTD_INCLUDE_DIR AND ZSTD_SYSTEM_LIBRARY)
>>>>>>> upstream/mysql-8.0.22
    SET(SYSTEM_ZSTD_FOUND 1)
    SET(ZSTD_LIBRARY ${ZSTD_SYSTEM_LIBRARY})
<<<<<<< HEAD
    MESSAGE(STATUS "ZSTD_LIBRARY(system) ${ZSTD_LIBRARY}")
||||||| merged common ancestors
    MESSAGE(STATUS "ZSTD_LIBRARY ${ZSTD_LIBRARY}")
=======
    IF(NOT ZSTD_INCLUDE_DIR STREQUAL "/usr/include")
      # In case of -DCMAKE_PREFIX_PATH=</path/to/custom/zstd>
      INCLUDE_DIRECTORIES(BEFORE SYSTEM ${ZSTD_INCLUDE_DIR})
    ENDIF()
>>>>>>> upstream/mysql-8.0.22
  ENDIF()
ENDMACRO()

MACRO (MYSQL_USE_BUNDLED_ZSTD)
<<<<<<< HEAD
  SET(WITH_ZSTD "bundled" CACHE STRING "By default use bundled zstd library")
  CHECK_ZSTD_VERSION(${CMAKE_SOURCE_DIR}/extra/zstd/lib)
  SET(BUILD_BUNDLED_ZSTD 1)
||||||| merged common ancestors
  SET(WITH_ZSTD "bundled" CACHE STRING "By default use bundled zstd library")
  SET(BUILD_BUNDLED_ZSTD 1)
=======
>>>>>>> upstream/mysql-8.0.22
  SET(ZSTD_LIBRARY zstd CACHE INTERNAL "Bundled zlib library")
  SET(WITH_ZSTD "bundled" CACHE STRING "Use bundled zstd")
  SET(ZSTD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extra/zstd/lib)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM ${ZSTD_INCLUDE_DIR})
  ADD_SUBDIRECTORY(extra/zstd)
ENDMACRO()

MACRO (MYSQL_CHECK_ZSTD)
  IF(NOT WITH_ZSTD)
    SET(WITH_ZSTD "bundled" CACHE STRING "By default use bundled zstd library")
  ENDIF()

  IF(WITH_ZSTD STREQUAL "bundled")
    MYSQL_USE_BUNDLED_ZSTD()
  ELSEIF(WITH_ZSTD STREQUAL "system")
    FIND_SYSTEM_ZSTD()
    IF (NOT SYSTEM_ZSTD_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find system zstd libraries.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_ZSTD must be bundled or system")
  ENDIF()
  FIND_ZSTD_VERSION()
  IF(ZSTD_VERSION VERSION_LESS MIN_ZSTD_VERSION_REQUIRED)
    MESSAGE(FATAL_ERROR
      "ZSTD version must be at least ${MIN_ZSTD_VERSION_REQUIRED}, "
      "found ${ZSTD_VERSION}.\nPlease use -DWITH_ZSTD=bundled")
  ENDIF()
ENDMACRO()

# Copyright (c) 2017 Oracle and/or its affiliates. All rights reserved.
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

MACRO(MYSQL_CHECK_CURL)
  IF(NOT WIN32)
   IF(WITH_CURL STREQUAL "system")
     #  FindCURL.cmake will set
     #  CURL_INCLUDE_DIRS   - where to find curl/curl.h, etc.
     #  CURL_LIBRARIES      - List of libraries when using curl.
     #  CURL_FOUND          - True if curl found.
     #  CURL_VERSION_STRING - the version of curl found (since CMake 2.8.8)
     FIND_PACKAGE(CURL)
     IF(CURL_FOUND)
       SET(CURL_LIBRARY ${CURL_LIBRARIES} CACHE PATH "Curl library")
     ENDIF()
     MESSAGE(STATUS "CURL_LIBRARY = ${CURL_LIBRARY}")
   ELSEIF(WITH_CURL)
    LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    FIND_LIBRARY(CURL_LIBRARY
      NAMES curl
      PATHS ${WITH_CURL}/lib
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
    )
    CHECK_INCLUDE_FILE_CXX(${WITH_CURL}/include/curl/curl.h HAVE_CURL_HEADERS)
    IF (CURL_LIBRARY AND HAVE_CURL_HEADERS)
      SET(CURL_FOUND TRUE)
      SET(CURL_INCLUDE_DIRS ${WITH_CURL}/include)
    ELSE()  
      SET(CURL_FOUND FALSE)
    ENDIF()
    LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    MESSAGE(STATUS "CURL_LIBRARY = ${CURL_LIBRARY}")
   ELSE()
     MESSAGE(STATUS
       "You need to set WITH_CURL. This"
       " variable needs to point to curl library.")
   ENDIF()
  ENDIF()
ENDMACRO()

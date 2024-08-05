# Copyright (c) 2024, Percona and/or its affiliates. All rights reserved.
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

# Simplify Google V8 usage by defining ext::v8 interface library.

MACRO(MYSQL_CHECK_V8)
  IF(NOT DEFINED V8_INCLUDE_DIR OR NOT DEFINED V8_LIB_DIR)
    MESSAGE(FATAL_ERROR "No path to V8 headers and libraries. Please set V8_INCLUDE_DIR and V8_LIB_DIR.")
  ENDIF()

  # Check that V8_INCLUDE_DIR contains necessary headers.
  IF(NOT EXISTS ${V8_INCLUDE_DIR}/v8.h)
    MESSAGE(FATAL_ERROR "Bad path to V8 headers. No v8.h in V8_INCLUDE_DIR.")
  ENDIF()

  # Also check that V8_LIB_DIR contains necessary libraries.
  FIND_LIBRARY(V8_LIB v8_monolith PATHS ${V8_LIB_DIR} NO_DEFAULT_PATH)
  IF(NOT V8_LIB)
    MESSAGE(FATAL_ERROR "Bad path to V8 libraries. No v8_monolith library in V8_LIB_DIR.")
  ENDIF()

  ADD_LIBRARY(v8_interface INTERFACE)

  TARGET_INCLUDE_DIRECTORIES(v8_interface SYSTEM INTERFACE ${V8_INCLUDE_DIR})

  TARGET_LINK_LIBRARIES(v8_interface INTERFACE ${V8_LIB})

  # Uncomment below if you are playing with debug build of V8 library.
  # TODO: Add check which will determine if we need to use this option automagically?
  # TARGET_COMPILE_DEFINITIONS(v8_interface INTERFACE V8_ENABLE_CHECKS)

  ADD_LIBRARY(ext::v8 ALIAS v8_interface)
ENDMACRO()

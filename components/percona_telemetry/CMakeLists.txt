# Copyright (c) 2017, 2021, Oracle and/or its affiliates.
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

DISABLE_MISSING_PROFILE_WARNING()

IF(WITH_PERCONA_TELEMETRY)
  message(STATUS "Building Percona Telemetry component")
ELSE()
  message(STATUS "Not building Percona Telemetry component")
  RETURN()
ENDIF()

MYSQL_ADD_COMPONENT(percona_telemetry
  percona_telemetry_component.cc
  config.cc
  data_provider.cc
  storage.cc
  worker.cc
  component.cc
  logger.cc
  MODULE_ONLY
  TEST_ONLY
  )

IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
  TARGET_LINK_LIBRARIES(component_percona_telemetry stdc++fs)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(component_percona_telemetry SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(component_percona_telemetry extra::rapidjson)

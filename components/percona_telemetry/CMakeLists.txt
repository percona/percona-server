# Copyright (c) 2024 Percona LLC and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2 of
# the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


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

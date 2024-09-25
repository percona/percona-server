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
   
MY_CHECK_CXX_COMPILER_WARNING("-Wno-unused-local-typedefs" HAS_WARN_FLAG)
IF(HAS_WARN_FLAG)
  STRING_APPEND(CMAKE_CXX_FLAGS " ${HAS_WARN_FLAG}")
ENDIF()

INCLUDE_DIRECTORIES(SYSTEM
  ${GMOCK_INCLUDE_DIRS}
)

SET(PERCONA_TELEMETRY_COMPONENT_SRC
  ${CMAKE_SOURCE_DIR}/components/percona_telemetry/data_provider.cc
)

SET(LOCAL_MOCK_SRC
  logger.cc
)

# Add tests
SET(TESTS
  data_provider
  )

FOREACH(test ${TESTS})
  MYSQL_ADD_EXECUTABLE(${test}-t ${PERCONA_TELEMETRY_COMPONENT_SRC} ${LOCAL_MOCK_SRC} ${test}-t.cc ADD_TEST ${test} LINK_LIBRARIES ${PERCONA_TELEMETRY_COMPONENT_LIBS} DEPENDENCIES component_percona_telemetry)
  TARGET_LINK_LIBRARIES(${test}-t mysys gunit_small)
ENDFOREACH()


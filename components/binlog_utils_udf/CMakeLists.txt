# Copyright (c) 2023 Percona LLC and/or its affiliates. All rights reserved.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2 of
# the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

option(WITH_BINLOG_UTILS_UDF "Build Binlog Utils component" ON)

if(WITH_BINLOG_UTILS_UDF)
  message(STATUS "Building Binlog Utils UDF component")
else()
  message(STATUS "Not building Binlog Utils UDF component")
  return()
endif()

# We are not interesting in profiling tests.
DISABLE_MISSING_PROFILE_WARNING()

MYSQL_ADD_COMPONENT(binlog_utils_udf
  binlog_utils_udf.cc
  LINK_LIBRARIES extra::rapidjson ext::zlib
  MODULE_ONLY
)

target_compile_definitions(component_binlog_utils_udf PRIVATE MYSQL_SERVER)
target_include_directories(component_binlog_utils_udf SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

IF(APPLE)
  SET_TARGET_PROPERTIES(component_binlog_utils_udf PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
ENDIF()
